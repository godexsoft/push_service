//
//  push_service.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include "push_service.hpp"

namespace push {
    
    push_service::push_service(boost::asio::io_service& io)
    : io_(io)
    {
    }
    
    long push_service::post(const device& dev, const std::string& raw_payload,
                            const long expiration, const long ident)
    {
        if( ! providers_.count(dev.provider_class))
        {
            throw std::runtime_error(dev.provider_class + " was not registered. Can't post push notification.");
        }
        
        return ident;
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
    
} // namespace push