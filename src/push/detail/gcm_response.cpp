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

    gcm_response::gcm_response(const push::error::gcm_err_code& code)
    : status_(code)
    {
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