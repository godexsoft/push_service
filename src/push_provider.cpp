//
//  push_provider.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/push_provider.hpp>
#include <push/push_service.hpp>

namespace push {
    device::device(const std::string& pc, const std::string& t)
    : provider_class(pc)
    , token(t)
    {
    }
    
    provider::provider(push_service& ps, const std::string& k)
    : push_service_(ps)
    , provider_key_(k)
    {
        // register with the push_service
        push_service_.add_provider(this, provider_key_);
    }
    
    provider::~provider()
    {
        // unregister
        push_service_.remove_provider(provider_key_);
    }
}