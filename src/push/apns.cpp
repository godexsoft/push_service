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

namespace bip = boost::asio::ip;
namespace bssl = boost::asio::ssl;

namespace push {
    
    apns::apns(push_service& ps, const std::string& host, const std::string& port,
               const std::string& cert, const std::string& priv_key)
    : provider(ps, apns::key)
    , pool_(ps.get_io_service(), 4) // TODO: hardcoded for now. remove
    {
        bip::tcp::resolver resolver(ps.get_io_service());
        bip::tcp::resolver::query query(host, port);
        bip::tcp::resolver::iterator iterator = resolver.resolve(query);
        
        bssl::context ctx(bssl::context::sslv23);
        ctx.set_options(bssl::context::default_workarounds
                            | bssl::context::no_sslv2
                            | bssl::context::single_dh_use);
        
        ctx.use_private_key_file(priv_key, boost::asio::ssl::context::pem);
        ctx.use_certificate_file(cert, boost::asio::ssl::context::pem);
        
        pool_.start(ctx, iterator);
    }
    
    bool apns::validate_device(const device& dev) const
    {
        // TODO: perform actual validation
        return true;
    }

    uint32_t apns::post(const device& dev, const std::string& payload,
                          const uint32_t expiry, const uint32_t ident)
    {
        boost::shared_ptr<detail::apns_connection> con =
            pool_.get_connection();

        con->perform( detail::apns_request(dev, payload, expiry, ident) );

        return ident;
    }
    
    const char* apns::key = "apns";
    
} // namespace push