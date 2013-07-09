//
//  gcm.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/gcm.hpp>
#include <iostream>

namespace push {

    gcm::gcm(push_service& ps, const std::string& host, const std::string& port,
             const std::string& project_id, const std::string& api_key)
    : provider(ps, gcm::key)
    {
    }
    
    bool gcm::validate_device(const device& dev) const
    {
        std::cout << "Validate GCM device with token " << dev.token << "\n";
        return true;
    }

    uint32_t gcm::post(const device& dev, const std::string& payload,
                       const uint32_t expiry, const uint32_t ident)
    {
        return ident;
    }

    
    const char* gcm::key = "gcm";
    
} // namespace push