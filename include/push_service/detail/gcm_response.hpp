//
//  gcm_response.hpp
//  push_service
//
//  Created by Alexander Kremer on 29/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_GCM_RESPONSE_HPP_
#define _PUSH_SERVICE_GCM_RESPONSE_HPP_

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <json_spirit/json_spirit_reader_template.h>

#include <push/gcm_errors.hpp>

namespace push {
namespace detail {
    
    /**
     * @brief This class represents a single entry (either success or failure per device) of the gcm response.
     */
    class gcm_response_entry
    {
    public:
        gcm_response_entry(const push::error::gcm_entry_err_code& code = push::error::successful);
        gcm_response_entry(const json_spirit::Object& obj);
        
        const boost::system::error_code to_error_code() const;

        const push::error::gcm_entry_err_code get_status() const;
        
        // properties
        std::string message_id;
        std::string registration_id;
        
    private:
        const push::error::gcm_entry_err_code status_from_error(const std::string& err) const;
        
        push::error::gcm_entry_err_code  status_;        
    };
    
    /**
     * @brief This class represents a complete gcm response.
     */
    class gcm_response
    {
    public:
        friend class gcm_connection;
        explicit gcm_response(const push::error::gcm_err_code& code = push::error::no_gcm_error);
        explicit gcm_response(const std::string& json);
        
        const boost::system::error_code to_error_code() const;
        
        const push::error::gcm_err_code get_status() const;
        
        // properties
        int64_t multicast_id;
        uint64_t success;
        uint64_t failure;
        uint64_t canonical_ids;
        
        std::vector<gcm_response_entry> results;
        
    private:
        push::error::gcm_err_code  status_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_GCM_RESPONSE_HPP_
