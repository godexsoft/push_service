//
//  apns.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push_service/apns.hpp>
#include <push_service/detail/push_service.hpp>
#include <push_service/detail/apns_connection.hpp>
#include <push_service/detail/apns_request.hpp>
#include <push_service/detail/p12.hpp>

#include <iostream>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace push {

    using namespace boost::asio;

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
        json_spirit::Object top_obj, aps_obj;
        
        if(!title.empty() || !body.empty() || !loc_key.empty() || !title_loc_key.empty())
        {
            // alert is json object
            aps_obj.emplace_back("alert", alert_to_json());
        }
        else if(!alert.empty())
        {
            // simple alert value
            aps_obj.emplace_back("alert", alert);
        }
        
        if(badge != UINT16_MAX)
        {
            aps_obj.emplace_back("badge", badge);
        }
        
        if(!sound.empty())
        {
            aps_obj.emplace_back("sound", sound);
        }
        
        if(!action_loc_key.empty())
        {
            aps_obj.emplace_back("action-loc-key", action_loc_key);
        }

        if(!launch_image.empty())
        {
            aps_obj.emplace_back("launch_image", launch_image);
        }
        
        // add custom key-values
        for (const auto& p : custom_)
        {
            top_obj.emplace_back(p.first, p.second);
        }
        
        top_obj.emplace_back("aps", aps_obj);
        std::string str = json_spirit::write_string( json_spirit::Value(top_obj), false );
        return str;
    }

    json_spirit::Object apns_message::alert_to_json() const
    {
       json_spirit::Object alert_obj;

       if (!title.empty())
       {
          alert_obj.emplace_back("title", title);
       }

       if (!body.empty())
       {
          alert_obj.emplace_back("body", body);
       }

       if (!title_loc_key.empty())
       {
          alert_obj.emplace_back("title-loc-key", title_loc_key);

          if (!title_loc_args.empty())
          {
             // title-log-args is array
             json_spirit::Array arr;

             for(const auto& title_loc_arg : title_loc_args)
             {
                 arr.emplace_back(title_loc_arg);
             }

             alert_obj.emplace_back("title-loc-args", arr);
          }
       }

       if (!loc_key.empty())
       {
          alert_obj.emplace_back("loc-key", loc_key);

          if (!loc_args.empty())
          {
              // loc-args is array
              json_spirit::Array arr;

              for(const auto& loc_arg : loc_args)
              {
                  arr.emplace_back(loc_arg);
              }

              alert_obj.emplace_back("loc-args", arr);
          }
       }

       return alert_obj;
    }

    apns::apns(push_service& ps, const config& cfg)
    : provider(ps, apns::key, cfg.log_callback)
    , pool_(ps.get_io_service(), cfg.pool_size, cfg.restart_delay, cfg.confirmation_delay, cfg.log_callback)
    , ssl_ctx_(ssl::context::sslv23)
    , callback_(cfg.callback)
    {
        ip::tcp::resolver resolver(ps.get_io_service());
        ip::tcp::resolver::query query(cfg.host, cfg.port);
        ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);

        if(cfg.p12_cert_key.empty())
        {
            if(!cfg.p12_pass.empty())
            {
                ssl_ctx_.set_password_callback(boost::bind(&apns::give_p12_pass, this, cfg.p12_pass));
            }

            ssl_ctx_.use_private_key_file(cfg.private_key, ssl::context::pem);
            ssl_ctx_.use_certificate_file(cfg.certificate, ssl::context::pem);
        }
        else
        {
            cert_key_ = detail::p12::extract_cert_key_pem(cfg.p12_cert_key, cfg.p12_pass);
            
            ssl_ctx_.use_private_key(boost::asio::buffer(cert_key_.second.c_str(), cert_key_.second.size()), ssl::context::pem);
            ssl_ctx_.use_certificate(boost::asio::buffer(cert_key_.first.c_str(), cert_key_.first.size()), ssl::context::pem);
        }

        pool_.start(ssl_ctx_, iterator, callback_, cfg.ca_certificate_path);
    }

    const std::string apns::give_p12_pass(const std::string& pass)
    {
        return pass;
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
        const std::string json_msg = msg.to_json();
        PUSH_LOG("Post " + json_msg, LogLevel::INFO);
        return post(dev, json_msg, expiry, ident);
    }
    
    const char* apns::key = "apns";
    const uint16_t apns::config::sc_default_pool_size = 4;
    const boost::posix_time::time_duration apns::config::sc_default_restart_delay = boost::posix_time::seconds(1);
    const boost::posix_time::time_duration apns::config::sc_default_confirmation_delay = boost::posix_time::seconds(1);

    const uint16_t apns_feedback::config::sc_default_pool_size = 1;
    const boost::posix_time::time_duration apns_feedback::config::sc_default_restart_delay = boost::posix_time::seconds(1);

    apns_feedback::apns_feedback(push_service& ps, const config& cfg)
    : push_service_(ps)
    , host_(cfg.host)
    , port_(cfg.port)
    , pool_(ps.get_io_service(), cfg.pool_size, cfg.restart_delay, boost::posix_time::seconds(0) /*not used*/, cfg.log_callback)
    , ssl_ctx_(ssl::context::sslv23)
    , on_feedback_(cfg.callback)
    , ca_certificate_path_(cfg.ca_certificate_path)
    {
        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);
        
        if(cfg.p12_cert_key.empty())
        {
            if(!cfg.p12_pass.empty())
            {
                ssl_ctx_.set_password_callback(boost::bind(&apns_feedback::give_p12_pass, this, cfg.p12_pass));
            }

            ssl_ctx_.use_private_key_file(cfg.private_key, ssl::context::pem);
            ssl_ctx_.use_certificate_file(cfg.certificate, ssl::context::pem);
        }
        else
        {
            cert_key_ = detail::p12::extract_cert_key_pem(cfg.p12_cert_key, cfg.p12_pass);
            
            ssl_ctx_.use_private_key(boost::asio::buffer(cert_key_.second.c_str(), cert_key_.second.size()), ssl::context::pem);
            ssl_ctx_.use_certificate(boost::asio::buffer(cert_key_.first.c_str(), cert_key_.first.size()), ssl::context::pem);
        }
    }

    const std::string apns_feedback::give_p12_pass(const std::string& pass)
    {
        return pass;
    }

    void apns_feedback::start()
    {
        ip::tcp::resolver resolver(push_service_.get_io_service());
        ip::tcp::resolver::query query(host_, port_);
        boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        pool_.start(ssl_ctx_, iterator, on_feedback_, ca_certificate_path_);
    }

    void apns_feedback::stop()
    {
        pool_.stop();
    }

} // namespace push
