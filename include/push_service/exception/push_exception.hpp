//
//  push_exception.hpp
//  push_service
//
//  Created by Alexander Kremer on 12/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_EXCEPTION_HPP_
#define _PUSH_EXCEPTION_HPP_

#include <exception>

namespace push {
namespace exception {

    class push_exception
    : public std::runtime_error
    {
    public:
        explicit push_exception(const std::string& m)
        : std::runtime_error(m)
        {
        }
        
        explicit push_exception(const char* m)
        : std::runtime_error(m)
        {            
        }
    };
    
} // namespace exception
} // namespace push

#endif // _PUSH_EXCEPTION_HPP_
