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
    : status_(push::error::no_error)
    , identity_(0) // unset
    {
        boost::asio::streambuf::const_buffers_type bufs = data.data();
        std::string s(boost::asio::buffers_begin(bufs), boost::asio::buffers_begin(bufs) + 6);

        // consume the 6 bytes
        data.consume(6);
        
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
    
    apns_feedback_response::apns_feedback_response(std::string& data)
    {
        char* p = &data.at(0);
        
        time_t t;
        memcpy(&t, p, sizeof(time_t));
        p += sizeof(time_t);
        
        time_ = boost::posix_time::from_time_t(t);
        
        uint16_t token_len;
        memcpy(&token_len, p, sizeof(uint16_t));
        p += sizeof(uint16_t);

        token_len = ntohs(token_len);
        
        std::cout << "token len is " << token_len << std::endl;
        std::vector<char> token_bytes(token_len, 0);

        memcpy(&token_bytes.at(0), p, token_len);
        token_ = std::string(token_bytes.begin(), token_bytes.end());
        
        std::cout << "token: " << token_ << std::endl;
    }
    
    const boost::system::error_code apns_response::to_error_code() const
    {
        return boost::system::error_code(status_, push::error::apns_error_category);
    }
    
    const push::error::apns_err_code apns_response::get_status() const
    {
        return status_;
    }
    
    const uint32_t apns_response::get_identity() const
    {
        return identity_;
    }

    const boost::posix_time::ptime apns_feedback_response::get_time() const
    {
        return time_;
    }
    
    const std::string apns_feedback_response::get_token() const
    {
        return token_;
    }

} // namespace detail
} // namespace push