//
//  gcm_response.cpp
//  push_service
//
//  Created by Alexander Kremer on 29/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/gcm_response.hpp>
#include <push/gcm_errors.hpp>

#include <sstream>

namespace push {
namespace detail {

    using namespace json_spirit;
    
    gcm_response_entry::gcm_response_entry(const push::error::gcm_entry_err_code& code)
    : status_(code)
    {        
    }
    
    gcm_response_entry::gcm_response_entry(const Object& obj)
    : status_(push::error::successful)
    {
        for(Object::size_type i = 0; i != obj.size(); ++i)
        {
            const Pair& pair = obj[i];
            const std::string& name  = pair.name_;
            const Value& value = pair.value_;
            
            if(name == "message_id")
            {
                message_id = value.get_str();
            }
            else if(name == "error")
            {
                status_ = status_from_error(value.get_str());
            }
            else if(name == "registration_id")
            {
                registration_id = value.get_str();
            }
        }
    }
    
    const push::error::gcm_entry_err_code gcm_response_entry::status_from_error(const std::string& err) const
    {
        if(err == "MissingRegistration")
        {
            return push::error::missing_registration;
        }
        else if(err == "InvalidRegistration")
        {
            return push::error::invalid_registration;
        }
        else if(err == "MismatchSenderId")
        {
            return push::error::mismatch_sender_id;
        }
        else if(err == "NotRegistered")
        {
            return push::error::not_registered;
        }
        else if(err == "MessageTooBig")
        {
            return push::error::message_too_big;
        }
        else if(err == "InvalidDataKey")
        {
            return push::error::invalid_data_key;
        }
        else if(err == "InvalidTtl")
        {
            return push::error::invalid_ttl;
        }
        else if(err == "Unavailable")
        {
            return push::error::unavailable;
        }
        else if(err == "InternalServerError")
        {
            return push::error::internal_server_error;
        }
        else if(err == "InvalidPackageName")
        {
            return push::error::invalid_package_name;
        }
        
        return push::error::unknown_entry_error;
    }
    
    const boost::system::error_code gcm_response_entry::to_error_code() const
    {
        return boost::system::error_code(status_, push::error::gcm_entry_error_category);
    }
    
    const push::error::gcm_entry_err_code gcm_response_entry::get_status() const
    {
        return status_;
    }
    
    gcm_response::gcm_response(const push::error::gcm_err_code& code)
    : multicast_id(-1)
    , success(0)
    , failure(0)
    , canonical_ids(0)
    , status_(code)
    {
    }
    
    gcm_response::gcm_response(const std::string& json)
    : multicast_id(-1)
    , success(0)
    , failure(0)
    , canonical_ids(0)
    , status_(push::error::no_gcm_error)
    {   
        Value val;
        if(!read_string(json, val))
        {
            std::cout << "FAILED ON JSON: " << json << "\n";
            status_ = push::error::json_parsing_error;
            return;
        }
        
        const Object& obj = val.get_obj();
        for(Object::size_type i = 0; i != obj.size(); ++i)
        {
            const Pair& pair = obj[i];
            const std::string& name  = pair.name_;
            const Value& value = pair.value_;
            
            if(name == "multicast_id")
            {
                multicast_id = value.get_int64();
            }
            else if(name == "success")
            {
                success = value.get_uint64();
            }
            else if(name == "failure")
            {
                failure = value.get_uint64();
            }
            else if(name == "canonical_ids")
            {
                canonical_ids = value.get_uint64();
            }
            else if(name == "results")
            {
                const Array& res_arr = value.get_array();
                
                for(Array::size_type j = 0; j != res_arr.size(); ++j)
                {
                    results.push_back( gcm_response_entry(res_arr[j].get_obj()) );
                }
            }
        }
    }
    
    const boost::system::error_code gcm_response::to_error_code() const
    {
        return boost::system::error_code(status_, push::error::gcm_error_category);
    }
    
    const push::error::gcm_err_code gcm_response::get_status() const
    {
        return status_;
    }
    
} // namespace detail
} // namespace push