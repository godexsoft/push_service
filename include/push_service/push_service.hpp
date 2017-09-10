//
//  push_service.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_HPP_
#define _PUSH_SERVICE_HPP_

#include <push_service/detail/push_service.hpp>
#include <push_service/log.hpp>

// known providers automatically included
#include <push_service/apns.hpp>
#include <push_service/gcm.hpp>

// error and exception handling
#include <push_service/apns_errors.hpp>
#include <push_service/exception/push_exception.hpp>
#include <push_service/exception/apns_exception.hpp>

#endif // _PUSH_SERVICE_HPP_
