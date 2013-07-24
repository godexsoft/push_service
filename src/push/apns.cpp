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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/foreach.hpp>

namespace push {

    using namespace boost::asio;
    using boost::property_tree::ptree;
    using boost::property_tree::read_json;
    using boost::property_tree::write_json;

    apns_message::apns_message()
    : badge(UINT16_MAX)
    {
    }
    
    void apns_message::add(const std::string& k, const std::string& v)
    {
        custom_.insert(std::make_pair(k,v));
    }
    
    std::string apns_message::to_json() const
    {
        ptree pt;
        
        if(!loc_key.empty())
        {
            // alert is json object
            pt.put("aps.alert.loc-key", loc_key);
            
            // TODO: there must be a better way to do this
            ptree args_array;
            BOOST_FOREACH(const std::string &arg, loc_args)
            {
                ptree a;
                a.put_value(arg);
                
                args_array.push_back(std::make_pair("", a));
            }
            
            pt.add_child("aps.alert.loc-args", args_array);
        }
        else if(!alert.empty())
        {
            // simple alert value
            pt.put("aps.alert", alert);
        }
        
        if(badge != UINT16_MAX)
        {
            pt.put("aps.badge", badge);
        }
        
        if(!sound.empty())
        {
            pt.put("aps.sound", sound);
        }
        
        if(!action_loc_key.empty())
        {
            pt.put("aps.action-loc-key", action_loc_key);
        }

        if(!launch_image.empty())
        {
            pt.put("aps.launch_image", launch_image);
        }
        
        // add custom key-values
        BOOST_FOREACH(const custom_map_type::value_type& p, custom_)
        {
            pt.put(p.first, p.second);
        }
        
        std::ostringstream buf;
        write_json (buf, pt, false);
        return buf.str();
    }
    
    
    apns::apns(push_service& ps, const config& cfg)
    : provider(ps, apns::key)
    , pool_(ps.get_io_service(), cfg.pool_size)
    , ssl_ctx_(ssl::context::sslv23)
    , callback_(cfg.callback)
    {
        ip::tcp::resolver resolver(ps.get_io_service());
        ip::tcp::resolver::query query(cfg.host, cfg.port);
        ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        
        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);
        
        ssl_ctx_.use_private_key_file(cfg.private_key, ssl::context::pem);
        ssl_ctx_.use_certificate_file(cfg.certificate, ssl::context::pem);
        
        pool_.start(ssl_ctx_, iterator, callback_);
    }
    
    bool apns::validate_device(const device& dev) const
    {
        return dev.token.size() == 32; // must be exactly 32 bytes
    }

    uint32_t apns::post(const device& dev, const std::string& payload,
                        const uint32_t expiry, const uint32_t ident)
    {
        pool_.post( detail::apns_request(dev, payload, expiry, ident) );        
        return ident;
    }
    
    uint32_t apns::post(const device& dev, const push_message& msg,
                        const uint32_t expiry, const uint32_t ident)
    {
        return post(dev, msg.to_json(), expiry, ident);
    }
    
    const char* apns::key = "apns";
    
    
    apns_feedback::apns_feedback(push_service& ps, const config& cfg)
    : push_service_(ps)
    , host_(cfg.host)
    , port_(cfg.port)
    , ssl_ctx_(ssl::context::sslv23)
    , on_feedback_(cfg.callback)
    {
        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);
        
        ssl_ctx_.use_private_key_file(cfg.private_key, ssl::context::pem);
        ssl_ctx_.use_certificate_file(cfg.certificate, ssl::context::pem);
    }
    
    void apns_feedback::start()
    {
        ip::tcp::resolver resolver(push_service_.get_io_service());
        ip::tcp::resolver::query query(host_, port_);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
        
        connection_ = boost::shared_ptr<detail::apns_feedback_connection>(
            new detail::apns_feedback_connection(
                push_service_.get_io_service(), on_feedback_) );
        
        connection_->start(ssl_ctx_, iterator);
    }
    
    void apns_feedback::stop()
    {
        connection_ = boost::shared_ptr<detail::apns_feedback_connection>();
    }
    
} // namespace push