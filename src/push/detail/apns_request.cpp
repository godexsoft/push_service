//
//  apns_request.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/apns_request.hpp>
#include <push/push_provider.hpp>
#include <algorithm>

namespace push {
namespace detail {

    apns_request::apns_request()
    : len_(0)
    {
    }
    
    apns_request::apns_request(const apns_request& r)
    : ident_(r.ident_)
    , time_(boost::posix_time::microsec_clock::local_time()) // set to now
    , body_(r.body_)
    , len_(r.len_)
    {        
    }
    
    apns_request::apns_request(const device& dev, const std::string& payload,
                               const uint32_t expiry, const uint32_t ident)
    : ident_(ident)
    {
        char cmd = 1;
        int16_t token_len = htons(32);
        int32_t identity = htonl(ident);
        int32_t expiration = htonl(expiry);
        int16_t payload_len = htons( payload.size() );
                
        char buf[1024] = {0,};
        char *p = buf;
        
        memcpy(p, &cmd, sizeof(char));
        ++p;
        
        memcpy(p, &identity, sizeof(int32_t));
        p += sizeof(int32_t);
        
        memcpy(p, &expiration, sizeof(int32_t));
        p += sizeof(int32_t);
        
        memcpy(p, &token_len, sizeof(int16_t));
        p += sizeof(int16_t);
        
        memcpy(p, dev.token.data(), 32);
        p += 32;
        
        memcpy(p, &payload_len, sizeof(int16_t));
        p += sizeof(int16_t);
        
        memcpy(p, payload.data(), payload.size());
        p += payload.size();
        
        len_ = p - buf;
        memcpy(body_.elems, buf, len_);
    }

    const uint32_t apns_request::get_identity() const
    {
        return ident_;
    }

    const boost::posix_time::ptime apns_request::get_time() const
    {
        return time_;
    }
        
} // namespace detail
} // namespace push