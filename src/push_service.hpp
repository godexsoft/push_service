//
//  push_service.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_HPP_
#define _PUSH_SERVICE_HPP_

#include <boost/asio.hpp>
#include <map>

#include "push_provider.hpp"

namespace push {

    class push_service
    {
    public:
        friend class provider;
        explicit push_service(boost::asio::io_service& io);
        
        long post(const device& dev, const std::string& raw_payload,
                  const long expiration = 0, const long ident = 123456); // todo: random ident
        
        bool validate_device(const device& dev) const;
        
    private:
        void add_provider(provider* p, const std::string& provider_key);
        void remove_provider(const std::string& provider_key);
        
        boost::asio::io_service& io_;
        std::map<std::string, provider*> providers_;
    };
    
} // namespace push

#endif // _PUSH_SERVICE_HPP_
