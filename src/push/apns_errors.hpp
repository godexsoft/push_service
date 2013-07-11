//
//  apns_errors.hpp
//  push_service
//
//  Created by Alexander Kremer on 11/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_APNS_ERRORS_HPP_
#define _PUSH_SERVICE_APNS_ERRORS_HPP_

#include <boost/system/error_code.hpp>

namespace push {
namespace error {

    typedef enum apns_err_code_t
    {
        NO_ERROR = 0,
        PROCESSING_ERROR = 1,
        MISSING_DEV_TOKEN = 2,
        MISSING_TOPIC = 3,
        MISSING_PAYLOAD = 4,
        INVALID_TOKEN_SIZE = 5,
        INVALID_TOPIC_SIZE = 6,
        INVALID_PAYLOAD_SIZE = 7,
        INVALID_TOKEN = 8,
        SHUTDOWN = 10,
        UNKNOWN = 255
    } apns_err_code;
    
    class apns_category : public boost::system::error_category
    {
    public:
        const char *name() const;        
        std::string message(int ev) const;
    };

} // namespace error
} // namespace push

#endif // _PUSH_SERVICE_APNS_ERRORS_HPP_
