//
//  gcm_errors.cpp
//  push_service
//
//  Created by Alexander Kremer on 29/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/gcm_errors.hpp>

namespace push {
namespace error {
    
    const char * gcm_entry_category::name() const BOOST_SYSTEM_NOEXCEPT
    {
        return "gcm result entry";
    }
    
    std::string gcm_entry_category::message(int ev) const BOOST_SYSTEM_NOEXCEPT
    {        
        switch(ev)
        {
            case successful:
                return "successful";
            case missing_registration:
                return "missing registration";
            case invalid_registration:
                return "invalid registration";
            case mismatch_sender_id:
                return "mismatch sender id";
            case not_registered:
                return "not registered";
            case message_too_big:
                return "message too big";
            case invalid_data_key:
                return "invalid data key";
            case invalid_ttl:
                return "invalid time to live";
            case unavailable:
                return "unavailable";
            case internal_server_error:
                return "internal server error";
            case invalid_package_name:
                return "invalid package name";
            case unknown_entry_error:
            default:
                return "unknown error";
        }
    }
      
    const char * gcm_category::name() const BOOST_SYSTEM_NOEXCEPT
    {
        return "gcm";
    }
    
    std::string gcm_category::message(int ev) const BOOST_SYSTEM_NOEXCEPT
    {
        switch(ev)
        {
            case no_gcm_error:
                return "no error";
            case invalid_response:
                return "invalid response";
            case json_parsing_error:
                return "couldn't parse json response";
            case parsing_error:
                return "json parsing failed or contains invalid fields";
            case authentication_error:
                return "error authenticating sender account";
            case internal_error:
                return "5xx internal error";
            default:
                return "unknown error";
        }
    }
} // namespace error
} // namespace push
