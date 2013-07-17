//
//  gcm_request.hpp
//  push_service
//
//  Created by Alexander Kremer on 17/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_GCM_REQUEST_HPP_
#define _PUSH_SERVICE_GCM_REQUEST_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace push {
    
    class device;
    
namespace detail {
    
    class gcm_request
    {
    public:
        friend class gcm_connection;

        gcm_request();
        
        gcm_request(const gcm_request& r);
        
        gcm_request& operator=(const gcm_request & other);
        
        gcm_request(const device& dev,
                    const std::string& api_key,
                    const std::string& payload);
        
    private:
        boost::asio::streambuf request_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_GCM_REQUEST_HPP_
