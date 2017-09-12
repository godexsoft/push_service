//
//  gcm.cpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push_service/gcm.hpp>
#include <push_service/detail/push_service.hpp>
#include <push_service/detail/gcm_connection.hpp>
#include <push_service/detail/gcm_request.hpp>

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <push_service/json_spirit/json_spirit_writer_template.h>
#include <boost/foreach.hpp>

namespace push {

    using namespace boost::asio;

    gcm_message::gcm_message(const uint32_t& ident)
    : time_to_live(UINT64_MAX)
    , delay_while_idle(false)
    , dry_run(false)
    , ident_(ident)
    {
    }

    void gcm_message::add(const std::string& k, const std::string& v)
    {
        custom_.insert(std::make_pair(k,v));
    }

    void gcm_message::add_reg_id(const std::string& reg_id)
    {
        registration_ids_.push_back(reg_id);
    }

    void gcm_message::clear_reg_ids()
    {
        registration_ids_.clear();
    }

    std::string gcm_message::to_json() const
    {
        json_spirit::Object top_obj;

        // registration_ids is array
        json_spirit::Array reg_ids;

        for(auto it = registration_ids_.begin(); it != registration_ids_.end(); ++it)
        {
            reg_ids.push_back(*it);
        }

        top_obj.push_back( json_spirit::Pair("registration_ids", reg_ids) );

        if(!notification_key.empty())
        {
            top_obj.push_back( json_spirit::Pair("notification_key", notification_key) );
        }

        if(!notification_key_name.empty())
        {
            top_obj.push_back( json_spirit::Pair("notification_key_name", notification_key_name) );
        }

        if(!collapse_key.empty())
        {
            top_obj.push_back( json_spirit::Pair("collapse_key", collapse_key) );
        }

        if(time_to_live != UINT64_MAX)
        {
            top_obj.push_back( json_spirit::Pair("time_to_live", time_to_live) );
        }

        // by default it's false in GCM
        if(delay_while_idle)
        {
            top_obj.push_back( json_spirit::Pair("delay_while_idle", delay_while_idle) );
        }

        if(!restricted_package_name.empty())
        {
            top_obj.push_back( json_spirit::Pair("restricted_package_name", restricted_package_name) );
        }

        // by default it's false in GCM
        if(dry_run)
        {
            top_obj.push_back( json_spirit::Pair("dry_run", dry_run) );
        }

        if(!custom_.empty())
        {
            json_spirit::Object data_obj;

            // add custom key-values
            BOOST_FOREACH(const custom_map_type::value_type& p, custom_)
            {
                data_obj.push_back( json_spirit::Pair(p.first, p.second) );
            }

            top_obj.push_back( json_spirit::Pair("data", data_obj) );
        }

        std::string str = json_spirit::write_string( json_spirit::Value(top_obj), false );

        return str;
    }

    gcm::gcm(push_service& ps,
             const std::string& /*project_id*/,
             const std::string& api_key,
             const log_callback_type& log_callback,
             const uint32_t& poolsize,
             const boost::posix_time::time_duration restart_delay,
             const boost::posix_time::time_duration confirmation_delay,
             const callback_type& cb)
    : provider(ps, gcm::key, log_callback)
    , api_key_(api_key)
    , pool_(ps.get_io_service(), poolsize, restart_delay, confirmation_delay, log_callback)
    , ssl_ctx_(ssl::context::sslv23)
    , callback_(cb)
    {
        ip::tcp::resolver resolver(ps.get_io_service());
        ip::tcp::resolver::query query("android.googleapis.com", "https");
        ip::tcp::resolver::iterator iterator = resolver.resolve(query);

        ssl_ctx_.set_options(ssl::context::default_workarounds
                             | ssl::context::no_sslv2
                             | ssl::context::single_dh_use);

        pool_.start(ssl_ctx_, iterator, callback_, std::string()/*not used*/);
    }

    bool gcm::validate_device(const device& dev) const
    {
        PUSH_LOG("Validate GCM device with token " + dev.token, LogLevel::INFO);
        return true;
    }

    uint32_t gcm::post(const device& dev, const std::string& payload,
                       const uint32_t /*expiry*/, const uint32_t& ident)
    {
        pool_.post( detail::gcm_request(dev, api_key_, payload, ident) );

        return ident;
    }

    uint32_t gcm::post(const device& dev, const gcm_message& msg,
                       const uint32_t expiry)
    {
        gcm_message tmp(msg);
        tmp.add_reg_id(dev.token);
        std::string json_msg = tmp.to_json();
        PUSH_LOG("post " + json_msg, LogLevel::INFO);

        return post(dev, json_msg, expiry, msg.ident_);
    }


    const char* gcm::key = "gcm";

} // namespace push
