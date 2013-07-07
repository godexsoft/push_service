//
//  apns.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _APNS_PROVIDER_HPP_
#define _APNS_PROVIDER_HPP_

#include "push_provider.hpp"

#include <string>

namespace push {
    
    class apns : public provider
    {
    public:
        static const char* key;
        
        apns(push_service& ps, const std::string& host, const std::string& port, const std::string& cert, const std::string& key);
        
        bool validate_device(const device& dev) const;
    };
    
} // namespace push

#endif // _APNS_PROVIDER_HPP_
