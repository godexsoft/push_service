//
//  push_service.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/push_service.hpp>

namespace push {
    
    push_service::push_service(boost::asio::io_service& io)
    : io_(io)
    , work_(io_)
    {
    }
    
    uint32_t push_service::post(const device& dev, const std::string& raw_payload,
                            const uint32_t expiration, const uint32_t
                                ident)
    {
        if( ! providers_.count(dev.provider_class))
        {
            throw std::runtime_error(dev.provider_class + " was not registered. Can't post push notification.");
        }

        return providers_.at(dev.provider_class)
                ->post(dev, raw_payload, expiration, ident);
    }
    
    bool push_service::validate_device(const device& dev) const
    {
        if( ! providers_.count(dev.provider_class))
        {
            throw std::runtime_error(dev.provider_class + " was not registered. Can't validate device.");
        }
        
        return providers_.at(dev.provider_class)->validate_device(dev);
    }
    
    void push_service::add_provider(provider* p, const std::string& provider_key)
    {
        if(providers_.count(provider_key))
        {
            throw std::runtime_error(provider_key + " is already registered.");
        }
        
        providers_.insert( std::make_pair(provider_key, p) );
    }
    
    void push_service::remove_provider(const std::string& provider_key)
    {
        providers_.erase( provider_key );
    }

    boost::asio::io_service& push_service::get_io_service()
    {
        return io_;
    }

} // namespace push