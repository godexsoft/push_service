//
//  apns_exception.hpp
//  push_service
//
//  Created by Alexander Kremer on 12/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _APNS_EXCEPTION_HPP_
#define _APNS_EXCEPTION_HPP_

#include <push/exception/push_exception.hpp>

namespace push {
namespace exception {

    class apns_exception
    : public push_exception
    {
    public:
        explicit apns_exception(const std::string& m)
        : push_exception(m)
        {
        }
        
        explicit apns_exception(const char* m)
        : push_exception(m)
        {            
        }
    };
    
} // namespace exception
} // namespace push

#endif // _APNS_EXCEPTION_HPP_
