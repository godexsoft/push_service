//
//  apns_response.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/apns_response.hpp>
#include <push/apns_errors.hpp>
#include <boost/asio/streambuf.hpp>

namespace push {
namespace detail {

    apns_response::apns_response(boost::asio::streambuf& data)
    : status_(push::error::NO_ERROR)
    , identity_(0) // unset
    {
        boost::asio::streambuf::const_buffers_type bufs = data.data();
        std::string s(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + 6);
        
        char* p = &s.at(0);
        
        char cmd;
        cmd = *p;
        BOOST_ASSERT(cmd == 8); // should always be 8
        ++p;
        
        status_ = static_cast<push::error::apns_err_code_t>(*p);
        ++p;
        
        uint32_t ident;
        memcpy(&ident, p, sizeof(uint32_t));
        
        identity_ = ntohl(ident);
    }
    
    const boost::system::error_code apns_response::to_error_code() const
    {
        return boost::system::error_code(status_, apns_cat_);
    }

} // namespace detail
} // namespace push