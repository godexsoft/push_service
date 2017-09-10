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
        no_apns_error = 0,
        processing_error = 1,
        missing_dev_token = 2,
        missing_topic = 3,
        missing_payload = 4,
        invalid_token_size = 5,
        invalid_topic_size = 6,
        invalid_payload_size = 7,
        invalid_token = 8,
        shutdown = 10,
        unknown = 255
    } apns_err_code;
    
    class apns_category : public boost::system::error_category
    {
    public:
        const char *name() const BOOST_SYSTEM_NOEXCEPT;
        std::string message(int ev) const;
    };
    
    static const boost::system::error_category& apns_error_category
        = apns_category();
    
    
    inline boost::system::error_code make_error_code(apns_err_code_t e)
    {
        return boost::system::error_code(
            static_cast<int>(e), apns_error_category);
    }

} // namespace error
} // namespace push

namespace boost {
namespace system {
        
    template<> struct is_error_code_enum<push::error::apns_err_code_t>
    {
        static const bool value = true;
    };
            
} // namespace system
} // namespace boost

#endif // _PUSH_SERVICE_APNS_ERRORS_HPP_
