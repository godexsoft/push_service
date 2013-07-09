//
//  gcm.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _GCM_PROVIDER_HPP_
#define _GCM_PROVIDER_HPP_

#include <push/push_provider.hpp>

#include <string>

namespace push {
    
    class gcm : public provider
    {
    public:
        static const char* key;
        
        gcm(push_service& ps, const std::string& host, const std::string& port,
            const std::string& project_id, const std::string& api_key);
        
        bool validate_device(const device& dev) const;

        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t ident);
    };
    
} // namespace push

#endif // _GCM_PROVIDER_HPP_
