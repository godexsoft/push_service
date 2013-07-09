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
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace push {
namespace detail {

    template<typename T>
    class connection_pool
    {
    public:
        connection_pool(boost::asio::io_service& io, const uint32_t cnt)
        {
            for(uint32_t i = 0; i<cnt; ++i)
            {
                connections_.push_back( boost::shared_ptr<T>( new T(io) ) );
            }
        }

        void start(boost::asio::ssl::context& context,
                   boost::asio::ip::tcp::resolver::iterator iterator)
        {
            boost::lock_guard<boost::mutex> locked(mutex_); // just to be extra sure
            
            std::for_each(connections_.begin(), connections_.end(),
                boost::bind(&T::start, _1, boost::ref(context), iterator) );

            elect_leader();
        }

        boost::shared_ptr<T> get_connection()
        {
            boost::lock_guard<boost::mutex> locked(mutex_);
            boost::shared_ptr<T> tmp = leader_;

            elect_leader();

            return tmp;
        }

    private:
        void elect_leader()
        {
            // TODO: employ a smarter strategy than picking the next available
            for(int i=0; i<connections_.size(); ++i)
            {
                if(connections_.at(i)->is_available() && connections_.at(i) != leader_)
                {
                    leader_ = connections_.at(i);
                    return;
                }
            }

            // self-DoS atack prevented :D
            throw std::runtime_error("Ran out of available connections while electing new leader");
        }

        /// All available connections
        std::vector<boost::shared_ptr<T> > connections_;

        /// Current leader of the pool
        boost::shared_ptr<T> leader_;
        
        /// Mutex for elections
        boost::mutex mutex_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_CONNECTION_POOL_HPP_
