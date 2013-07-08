//
//  apns.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include "apns.hpp"
#include <iostream>

namespace push {

    apns::apns(push_service& ps, const std::string& host, const std::string& port,
               const std::string& cert, const std::string& priv_key)
    : provider(ps, apns::key)
    {
    }
    
    bool apns::validate_device(const device& dev) const
    {
        std::cout << "Validate APNS device with token " << dev.token << "\n";
        return true;
    }
    
    const char* apns::key = "APNS provider";
    
} // namespace push