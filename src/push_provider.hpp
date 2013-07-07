//
//  push_provider.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_PROVIDER_HPP_
#define _PUSH_PROVIDER_HPP_

#include <string>

namespace push {
    
    class push_service;
    
    struct device
    {
        explicit device(const std::string& pc, const std::string& t);
        const std::string provider_class;
        const std::string token;
    };
    
    class provider
    {
    public:
        provider(push_service& ps, const std::string& provider_key);
        ~provider();
        
        virtual bool validate_device(const device& dev) const = 0;
        
    private:
        push_service& push_service_;
        const std::string provider_key_;
    };
    
} // namespace push

#endif // _PUSH_PROVIDER_HPP_
