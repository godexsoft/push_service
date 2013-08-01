//
//  apns_errors.cpp
//  push_service
//
//  Created by Alexander Kremer on 11/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/apns_errors.hpp>

namespace push {
namespace error {
    
    const char * apns_category::name() const
    {
        return "apns";
    }
    
    std::string apns_category::message(int ev) const
    {        
        switch(ev)
        {
            case no_apns_error:
                return "no error";
            case processing_error:
                return "processing error";
            case missing_dev_token:
                return "dev token is missing";
            case missing_topic:
                return "topic is missing";
            case missing_payload:
                return "payload is missing";
            case invalid_token_size:
                return "invalid token size";
            case invalid_topic_size:
                return "invalid topic size";
            case invalid_payload_size:
                return "invalid payload size";
            case invalid_token:
                return "invalid token";
            case shutdown:
                return "connection shutdown by remote host";            
            case unknown:
            default:
                return "unknown error";
        }
    }
    
} // namespace error
} // namespace push