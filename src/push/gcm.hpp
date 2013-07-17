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
#include <push/detail/gcm_connection.hpp>
#include <push/detail/connection_pool.hpp>

#include <boost/function.hpp>
#include <string>

namespace push {
    
    class gcm : public provider
    {
    public:        
        typedef push::detail::connection_pool<
            detail::gcm_connection, detail::gcm_request>
                ::callback_type
                    callback_type;

        static const char* key;
        
        gcm(push_service& ps, const std::string& api_url,
            const std::string& project_id, const std::string& api_key,
            const callback_type& cb = callback_type());
        
        bool validate_device(const device& dev) const;

        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t ident);
        
    private:
        const std::string api_key_;
        
        push::detail::connection_pool<
            push::detail::gcm_connection,
            push::detail::gcm_request> pool_;
        boost::asio::ssl::context ssl_ctx_;
        
        /// callback
        callback_type callback_;
    };
    
} // namespace push

#endif // _GCM_PROVIDER_HPP_
