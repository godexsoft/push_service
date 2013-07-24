//
//  push_service.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _IMPL_PUSH_SERVICE_HPP_
#define _IMPL_PUSH_SERVICE_HPP_

#include <boost/asio.hpp>
#include <map>

#include <push/push_provider.hpp>

namespace push {

    class push_service
    {
    public:
        friend class provider;
        explicit push_service(boost::asio::io_service& io);
        
        // TODO: remove ident generation, move expiration to ptime
        uint32_t post(const device& dev, const std::string& raw_payload,
                      const uint32_t expiration = 0, const uint32_t ident = 123456); // todo: random ident

        uint32_t post(const device& dev, const push_message& msg,
                      const uint32_t expiration = 0, const uint32_t ident = 123456); // todo: random ident
        
        bool validate_device(const device& dev) const;

        boost::asio::io_service& get_io_service();

    private:
        void add_provider(provider* p, const std::string& provider_key);
        void remove_provider(const std::string& provider_key);
        
        boost::asio::io_service& io_;
        std::map<std::string, provider*> providers_;
    };
    
} // namespace push

#endif // _IMPL_PUSH_SERVICE_HPP_
