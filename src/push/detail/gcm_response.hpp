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

#include <push/gcm_errors.hpp>

namespace push {
namespace detail {
        
    class gcm_response
    {
    public:
        friend class gcm_connection;
        gcm_response(const push::error::gcm_err_code& code = push::error::no_gcm_error);
        
        const boost::system::error_code to_error_code() const;
        
        const push::error::gcm_err_code get_status() const;
        
    private:
        template<typename Iter>
        void append_json(const Iter& begin, const Iter& end)
        {
            json_.append(begin, end);
        }
        
        push::error::gcm_err_code  status_;
        std::string json_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_GCM_RESPONSE_HPP_
