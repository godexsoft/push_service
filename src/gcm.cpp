//
//  gcm.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include "gcm.hpp"
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
    
    const char* gcm::key = "GCM provider";
    
} // namespace push