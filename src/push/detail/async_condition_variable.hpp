//
//  async_condition_variable.hpp
//  push_service
//
//  Created by Alexander Kremer on 18/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_ASYNC_COND_VAR_HPP_
#define _PUSH_SERVICE_ASYNC_COND_VAR_HPP_

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <list>

namespace push {
namespace detail {
    
    class async_condition_variable
    {
    public:
        
        class handle
        {
        public:
            friend class async_condition_variable;
            
            ~handle()
            {
                ptr_->disconnect(ident_);
            }
            
        private:
            handle();
            
            handle(async_condition_variable* p, const uint64_t& ident)
            : ptr_(p)
            , ident_(ident)
            {
            }
            
            async_condition_variable* const  ptr_;
            const uint64_t                   ident_;
        };
        
        friend class handle;
        
        typedef boost::function<void()> handler_type;        
        typedef boost::shared_ptr<handle> handle_type;
        
        async_condition_variable(boost::asio::io_service& io)
        : io_service_(io)
        , counter_(0)
        {
        }

        handle_type async_wait(const handler_type& cb)
        {
            boost::lock_guard<boost::mutex> l(mutex_);

            if(counter_ >= UINT64_MAX) // safe because synchronized
            {
                counter_ = 0;
            }
            
            waiters_.push_back(std::make_pair(counter_, cb));            
            return handle_type( new handle(this, counter_++) );
        }
        
        void notify_one()
        {
            boost::lock_guard<boost::mutex> l(mutex_);
            if(! waiters_.empty())
            {
                handler_type cb = waiters_.front().second;
                waiters_.pop_front();
                
                io_service_.post(cb);
            }
        }

        void notify_all()
        {
            boost::lock_guard<boost::mutex> l(mutex_);
            while(! waiters_.empty())
            {
                handler_type cb = waiters_.front().second;
                waiters_.pop_front();
                
                io_service_.post(cb);
            }
        }        
        
    private:
        
        void disconnect(const uint64_t& ident)
        {
            boost::lock_guard<boost::mutex> l(mutex_);
            
            // FIXME: there must be a much more efficient way to do this
            waiters_.erase(
                std::remove_if(waiters_.begin(), waiters_.end(),
                boost::bind(&std::pair<uint64_t,handler_type>::first, _1) == ident),
                    waiters_.end() );
        }
        
        boost::asio::io_service&    io_service_;
        boost::mutex                mutex_;
        
        std::list<std::pair<uint64_t, handler_type> > waiters_;
        uint64_t                    counter_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_ASYNC_COND_VAR_HPP_
