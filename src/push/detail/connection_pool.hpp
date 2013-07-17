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

#include <push/exception/push_exception.hpp>

namespace push {
namespace detail {

    template<typename T, typename J>
    class connection_pool
    {
    public:
        typedef typename T::callback_type
            callback_type;
        
        connection_pool(boost::asio::io_service& io, const uint32_t cnt)
        : callback_io_(io)
        {
            if(cnt < 1)
            {
                throw push::exception::push_exception("connection_pool must specify 1 or more as connections count.");
            }
            
            for(uint32_t i = 0; i<cnt; ++i)
            {
                boost::shared_ptr<T> con = boost::shared_ptr<T> ( new T(*this) );
                
                threads_.create_thread(
                    boost::bind(&boost::asio::io_service::run, boost::ref(con->get_io_service())) );

                connections_.push_back(con);
            }
        }
        
        ~connection_pool()
        {
            // stop all connections
            std::for_each(connections_.begin(), connections_.end(),
                boost::bind(&T::stop, _1) );

            threads_.join_all();
        }

        void start(boost::asio::ssl::context& context,
                   boost::asio::ip::tcp::resolver::iterator iterator,
                   const callback_type& cb)
        {
            std::for_each(connections_.begin(), connections_.end(),
                boost::bind(&T::start, _1, &context, iterator, callback_io_.wrap(cb) ) );
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

        
        boost::optional<J> get_next_job(const boost::posix_time::time_duration&
                                        timeout = boost::posix_time::not_a_date_time)
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            while(jobs_.empty())
            {
                if(timeout == boost::posix_time::not_a_date_time)
                {
                    condition_.wait(lock);
                }
                else
                {
                    if(!condition_.timed_wait(lock, timeout))
                    {
                        return boost::optional<J>();
                    }
                }
            }
            
            boost::optional<J> ret(jobs_.front());
            jobs_.pop();
            
            return ret;
        }
        
    private:
        /// io_service which we will use for the callbacks
        boost::asio::io_service&    callback_io_;

        /// Connection pool's own threads
        boost::thread_group         threads_;

        /// All available connections
        std::vector<boost::shared_ptr<T> > connections_;

        boost::mutex                mutex_;
        boost::condition_variable   condition_;
        
        /// Requests to process
        std::queue<J> jobs_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_CONNECTION_POOL_HPP_
