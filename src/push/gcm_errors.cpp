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
    
    const char * gcm_category::name() const
    {
        return "gcm";
    }
    
    std::string gcm_category::message(int ev) const
    {        
        switch(ev)
        {
            case no_gcm_error:
                return "no error";
            case invalid_response:
                return "invalid response";
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