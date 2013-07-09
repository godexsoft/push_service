//
//  apns.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _APNS_PROVIDER_HPP_
#define _APNS_PROVIDER_HPP_

#include <push/push_provider.hpp>

#include <push/detail/apns_connection.hpp>
#include <push/detail/connection_pool.hpp>

#include <string>

namespace push {

    class apns : public provider
    {
    public:
        static const char* key;
        
        apns(push_service& ps, const std::string& host, const std::string& port,
             const std::string& cert, const std::string& priv_key);
        
        bool validate_device(const device& dev) const;
        
        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t ident);

    private:
        push::detail::connection_pool<push::detail::apns_connection> pool_;
    };
    
} // namespace push

#endif // _APNS_PROVIDER_HPP_
