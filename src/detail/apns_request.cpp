//
//  apns_request.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push_service/detail/apns_request.hpp>
#include <push_service/push_provider.hpp>
#include <algorithm>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

namespace push {
namespace detail {
    
    apns_request::apns_request()
    : ident_(invalid_ident)
    {
    }
    
    apns_request::apns_request(const apns_request& r)
    : ident_(r.ident_)
    , time_(boost::posix_time::microsec_clock::local_time()) // set to now
    , body_(r.body_)
    {
    }
    
    apns_request::apns_request(const device& dev, const std::string& payload,
                               const uint32_t expiry, const uint32_t ident)
    : ident_(ident)
    , body_( boost::make_shared<request_buffer>( payload.size() + 32 + 2*sizeof(int32_t) + 2*sizeof(int16_t) + sizeof(char) ) )
    {
        char cmd = 1;
        int16_t token_len = htons(32);
        int32_t identity = htonl(ident);
        int32_t expiration = htonl(expiry);
        int16_t payload_len = htons( payload.size() );
                
        char *p = body_->data();
        
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

        assert( p <= body_->data() + body_->size() );
    }

    uint32_t apns_request::get_identity() const
    {
        return ident_;
    }

    boost::posix_time::ptime apns_request::get_time() const
    {
        return time_;
    }
        
} // namespace detail
} // namespace push
