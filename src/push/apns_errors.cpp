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
            case NO_ERROR:
                return "no error";
            case PROCESSING_ERROR:
                return "processing error";
            case MISSING_DEV_TOKEN:
                return "dev token is missing";
            case MISSING_TOPIC:
                return "topic is missing";
            case MISSING_PAYLOAD:
                return "payload is missing";
            case INVALID_TOKEN_SIZE:
                return "invalid token size";
            case INVALID_TOPIC_SIZE:
                return "invalid topic size";
            case INVALID_PAYLOAD_SIZE:
                return "invalid payload size";
            case INVALID_TOKEN:
                return "invalid token";
            case SHUTDOWN:
                return "connection shutdown by remote host";            
            case UNKNOWN:
            default:
                return "unknown error";
        }
    }
    
} // namespace error
} // namespace push