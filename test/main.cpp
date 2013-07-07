//
//  main.cpp
//  test
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <iostream>
#include "push_service.hpp"

// include used providers
#include "apns.hpp"

using namespace boost::asio;
using namespace push;

int main(int argc, const char * argv[])
{
    
    io_service io;
    push_service ps(io);
    
    // enable apple push with settings
    apns apple_push(ps, "host", "port", "cert", "key");
    
    // create an ios push device
    device dev1(apns::key, "mega token");
    
    // validate token
    bool flag = ps.validate_device(dev1);
    std::cout << "Validator says " << (flag?"all good":"invalid") << "\n";
    
    long ident = ps.post(dev1, "{...}");
    std::cout << "push message sent with id " << ident << std::endl;
    
    return 0;
}

