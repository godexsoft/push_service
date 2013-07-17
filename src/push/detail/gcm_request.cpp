//
//  gcm_request.cpp
//  push_service
//
//  Created by Alexander Kremer on 17/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/gcm_request.hpp>
#include <push/push_provider.hpp>

#include <algorithm>
#include <ostream>

namespace push {
namespace detail {

    gcm_request::gcm_request()
    {
    }
    
    gcm_request::gcm_request(const gcm_request& r)
    {
        *this = r;
    }
    
    gcm_request& gcm_request::operator= (const gcm_request& other)
    {
        if (this != &other)
        {
            // TODO: dirty.. find a better way to copy data
            std::ostream request_stream(&request_);
            
            boost::asio::streambuf::const_buffers_type bufs = other.request_.data();
            std::string s(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + other.request_.size());
            
            request_stream << s;
        }
        
        return *this;
    }
    
    gcm_request::gcm_request(const device& dev,
                             const std::string& api_key,
                             const std::string& payload)
    {
        std::ostream request_stream(&request_);
        
        request_stream << "POST /gcm/send HTTP/1.0\r\n";
        request_stream << "User-Agent: push_service\r\n";
        request_stream << "Host: android.googleapis.com\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Content-Type: application/json\r\n";
        request_stream << "Authorization: key=" << api_key << "\r\n";
        request_stream << "Connection: close\r\n";
        request_stream << "Content-Length:" << payload.size() << "\r\n\r\n";
        request_stream << payload;
    }
        
} // namespace detail
} // namespace push