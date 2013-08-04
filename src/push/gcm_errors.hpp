//
//  gcm_errors.hpp
//  push_service
//
//  Created by Alexander Kremer on 29/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_GCM_ERRORS_HPP_
#define _PUSH_SERVICE_GCM_ERRORS_HPP_

#include <boost/system/error_code.hpp>

namespace push {
namespace error {

    typedef enum gcm_err_code_t
    {
        no_gcm_error = 0,
        invalid_response = 1,
        json_parsing_error = 2,
        parsing_error = 400,
        authentication_error = 401,
        internal_error = 500
    } gcm_err_code;
    
    typedef enum gcm_entry_err_code_t
    {
        successful = 0,
        missing_registration,
        invalid_registration,
        mismatch_sender_id,
        not_registered,
        message_too_big,
        invalid_data_key,
        invalid_ttl,
        unavailable,
        internal_server_error,
        invalid_package_name,
        unknown_entry_error
    } gcm_entry_err_code;
    
    class gcm_category : public boost::system::error_category
    {
    public:
        const char *name() const;        
        std::string message(int ev) const;
    };
    
    static const boost::system::error_category& gcm_error_category
        = gcm_category();
    
    inline boost::system::error_code make_error_code(gcm_err_code_t e)
    {
        return boost::system::error_code(
            static_cast<int>(e), gcm_error_category);
    }

    
    class gcm_entry_category : public boost::system::error_category
    {
    public:
        const char *name() const;
        std::string message(int ev) const;
    };
    
    static const boost::system::error_category& gcm_entry_error_category
        = gcm_entry_category();
    
    inline boost::system::error_code make_error_code(gcm_entry_err_code_t e)
    {
        return boost::system::error_code(
            static_cast<int>(e), gcm_entry_error_category);
    }

} // namespace error
} // namespace push

namespace boost {
namespace system {
        
    template<> struct is_error_code_enum<push::error::gcm_err_code_t>
    {
        static const bool value = true;
    };

    template<> struct is_error_code_enum<push::error::gcm_entry_err_code_t>
    {
        static const bool value = true;
    };
    
} // namespace system
} // namespace boost

#endif // _PUSH_SERVICE_GCM_ERRORS_HPP_
