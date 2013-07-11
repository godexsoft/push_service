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

namespace push {
namespace detail {

    template<typename T, typename J>
    class connection_pool
    {
    public:
        typedef apns_connection::error_callback_type
            error_callback_type;
        
        connection_pool(boost::asio::io_service& io, const uint32_t cnt)
        : callback_io_(io)
        {
            if(cnt < 1)
            {
                throw std::runtime_error("connection_pool must specify 1 or more as connections count.");
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
                   const error_callback_type& cb)
        {
            std::for_each(connections_.begin(), connections_.end(),
                boost::bind(&T::start, _1, &context, iterator, callback_io_.wrap(cb) ) );
        }
        
        void post(const J& job)
        {
            {
                boost::lock_guard<boost::mutex> lock(mutex_);
                jobs_.push(job);
            }

            condition_.notify_one();
        }

        bool get_next_job(J& req, const boost::posix_time::time_duration& timeout)
        {
            boost::unique_lock<boost::mutex> lock(mutex_);
            while(jobs_.empty())
            {
                if(!condition_.timed_wait(lock, timeout))
                {
                    return false;
                }
            }
            
            req = jobs_.front();
            jobs_.pop();
            
            return true;
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
