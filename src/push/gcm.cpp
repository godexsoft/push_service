//
//  gcm.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/gcm.hpp>
#include <push/push_service.hpp>

#include <push/detail/gcm_connection.hpp>
#include <push/detail/gcm_request.hpp>

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace push {

    using namespace boost::asio;

    gcm::gcm(push_service& ps, const std::string& api_url,
             const std::string& project_id,
             const std::string& api_key,
             const callback_type& cb)
    : provider(ps, gcm::key)
    , api_key_(api_key)
    , pool_(ps.get_io_service(), 4) // TODO: hardcoded for now. remove
    , ssl_ctx_(ssl::context::sslv23)
    , callback_(cb)
    {
        ip::tcp::resolver resolver(ps.get_io_service());
        ip::tcp::resolver::query query(api_url, "https");
        ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        
        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);
        
        pool_.start(ssl_ctx_, iterator, callback_);
    }
    
    bool gcm::validate_device(const device& dev) const
    {
        std::cout << "Validate GCM device with token " << dev.token << "\n";
        return true;
    }

    uint32_t gcm::post(const device& dev, const std::string& payload,
                       const uint32_t expiry, const uint32_t ident)
    {
        pool_.post( detail::gcm_request(dev, api_key_, payload) );
        
        return ident;
    }

    
    const char* gcm::key = "gcm";
    
} // namespace push