//
//  apns.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/apns.hpp>
#include <push/push_service.hpp>

#include <push/detail/apns_connection.hpp>
#include <push/detail/apns_request.hpp>

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace push {

    using namespace boost::asio;
    
    apns::apns(push_service& ps, const std::string& host, const std::string& port,
               const std::string& cert, const std::string& priv_key,
               const error_callback_type& cb)
    : provider(ps, apns::key)
    , pool_(ps.get_io_service(), 4) // TODO: hardcoded for now. remove
    , ssl_ctx_(ssl::context::sslv23)
    , on_error_(cb)
    {
        ip::tcp::resolver resolver(ps.get_io_service());
        ip::tcp::resolver::query query(host, port);
        ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        
        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);
        
        ssl_ctx_.use_private_key_file(priv_key, ssl::context::pem);
        ssl_ctx_.use_certificate_file(cert, ssl::context::pem);
        
        pool_.start(ssl_ctx_, iterator, on_error_);
    }
    
    bool apns::validate_device(const device& dev) const
    {
        return dev.token.size() == 32; // must be exactly 32 bytes
    }

    uint32_t apns::post(const device& dev, const std::string& payload,
                        const uint32_t expiry, const uint32_t ident)
    {
        /*
        boost::shared_ptr<detail::apns_connection> con =
            pool_.get_connection();

        con->perform( boost::shared_ptr<detail::apns_request>(
            new detail::apns_request(dev, payload, expiry, ident) ) );
         */
        
        pool_.post( detail::apns_request(dev, payload, expiry, ident) );
        
        return ident;
    }
    
    const char* apns::key = "apns";
    
} // namespace push