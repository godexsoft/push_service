//
//  connection_pool.hpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_CONNECTION_POOL_HPP_
#define _PUSH_SERVICE_CONNECTION_POOL_HPP_

#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/optional.hpp>

#include <push_service/log.hpp>
#include <push_service/exception/push_exception.hpp>
#include <push_service/detail/async_condition_variable.hpp>

namespace push {
namespace detail {

    template<typename T, typename J>
    class connection_pool
    {
    public:
        typedef typename T::callback_type
            callback_type;
        
        connection_pool(boost::asio::io_service& io, const uint32_t cnt, boost::posix_time::time_duration restart_delay,
                        boost::posix_time::time_duration confirmation_delay, const log_callback_type& log_callback)
        : callback_io_(io)
        , condition_(io)
        , restart_delay_(restart_delay)
        , confirmation_delay_(confirmation_delay)
        , log_callback_(log_callback)
        {
            if(cnt < 1)
            {
                throw push::exception::push_exception("connection_pool must specify 1 or more as connections count.");
            }
            
            for(uint32_t i = 0; i<cnt; ++i)
            {
                spawn_connection();
            }
        }

        ~connection_pool()
        {
            stop();
            threads_.join_all();
        }

        void start(boost::asio::ssl::context& context,
                   boost::asio::ip::tcp::resolver::iterator iterator,
                   const callback_type& cb,
                   const std::string& ca_certificate_path)
        {
            std::for_each(connections_.begin(), connections_.end(),
                callback_io_.wrap(
                    boost::bind(&T::start, _1, &context, iterator, callback_io_.wrap(cb), ca_certificate_path) ) );
        }

        void stop()
        {
            // stop all connections
            std::for_each(connections_.begin(), connections_.end(),
                boost::bind(&T::stop, _1) );
        }

        void post(const J& job)
        {
            {
                boost::unique_lock<boost::mutex> lock(mutex_);
                jobs_.push(job);
            }

            condition_.notify_one();
        }

        template<typename I>
        void post(I& begin, const I& end)
        {
            boost::unique_lock<boost::mutex> lock(mutex_);

            for(; begin != end; ++begin)
            {
                jobs_.push(*begin);
            }
            
            condition_.notify_all();
        }

        async_condition_variable::handle_type
        async_wait_for_job(const boost::function<void()>& cb)
        {
            return condition_.async_wait(cb);
        }
        
        boost::optional<J> get_next_job()
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            if(!jobs_.empty())
            {
                boost::optional<J> ret(jobs_.front());
                jobs_.pop();
                
                return ret;
            }
            
            return boost::optional<J>();
        }
        
    private:
        
        void run_one(boost::shared_ptr<T>& con)
        {
            boost::asio::deadline_timer restart_timer(con->get_io_service());
            for (;;)
            {
               try
               {
                   con->get_io_service().run();
                   break;
               }
               catch (const std::exception& ex)
               {
                   PUSH_LOG(ex.what(), LogLevel::ERROR);
                   // restart the connection after delay
                   restart_timer.cancel();
                   restart_timer.expires_from_now(restart_delay_);
                   boost::weak_ptr<T> weak_con = con;
                   restart_timer.async_wait([weak_con](boost::system::error_code) {
                      if (auto con = weak_con.lock())
                        con->restart();
                   });
                   continue;
               }
            }
        }
        
        void spawn_connection()
        {
            boost::shared_ptr<T> con = boost::shared_ptr<T> ( new T(*this, confirmation_delay_, log_callback_) );
            connections_.push_back(con);
            
            threads_.create_thread(
                boost::bind(&connection_pool::run_one, this, con) );
        }

        /// io_service which we will use for the callbacks
        boost::asio::io_service&    callback_io_;

        /// delay for restart after error
        const boost::posix_time::time_duration restart_delay_;

        /// period of checking requests
        const boost::posix_time::time_duration confirmation_delay_;

        /// Connection pool's own threads
        boost::thread_group         threads_;

        boost::mutex                       mutex_;
        detail::async_condition_variable   condition_;
        
        /// All available connections
        std::vector<boost::shared_ptr<T> > connections_;

        /// Requests to process
        std::queue<J> jobs_;

        log_callback_type log_callback_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_CONNECTION_POOL_HPP_
