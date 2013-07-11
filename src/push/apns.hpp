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

#include <boost/function.hpp>
#include <string>

namespace push {

    class apns : public provider
    {
    public:
        typedef push::detail::connection_pool<
            detail::apns_connection,detail::apns_request>
                ::error_callback_type
                    error_callback_type;
        
        static const char* key;
        
        apns(push_service& ps, const std::string& host, const std::string& port,
             const std::string& cert, const std::string& priv_key,
             const error_callback_type& cb = error_callback_type());
        
        bool validate_device(const device& dev) const;
        
        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t ident);

    private:
        push::detail::connection_pool<
            push::detail::apns_connection,
            push::detail::apns_request> pool_;
        boost::asio::ssl::context ssl_ctx_;
        
        /// error callback
        error_callback_type on_error_;
    };
    
} // namespace push

#endif // _APNS_PROVIDER_HPP_
