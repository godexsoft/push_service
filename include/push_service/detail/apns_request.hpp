//
//  apns_request.hpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_APNS_REQUEST_HPP_
#define _PUSH_SERVICE_APNS_REQUEST_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace push {
    
    class device;
    
namespace detail {

    class apns_request
    {
    public:
        friend class apns_connection;

        apns_request();
        
        apns_request(const apns_request& r);
        
        apns_request(const device& dev, const std::string& payload,
                     const uint32_t expiry, const uint32_t ident);
        
        uint32_t get_identity() const;
        
        boost::posix_time::ptime get_time() const;

        static const uint32_t invalid_ident = 0;

    private:
        uint32_t ident_;
        boost::posix_time::ptime time_;
        typedef std::vector<char> request_buffer;
        boost::shared_ptr<request_buffer> body_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_APNS_REQUEST_HPP_
