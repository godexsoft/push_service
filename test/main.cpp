//
//  main.cpp
//  test
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <iostream>
#include <boost/thread.hpp>
#include <push_service.hpp>

using namespace boost::asio;
using namespace boost;
using namespace std;

using namespace push;

int main(int argc, const char * argv[])
{
    io_service      io;
    thread_group    threads;
    
    push_service    ps(io);
    
    try
    {
        apns apple_push(ps,
            "gateway.sandbox.push.apple.com",  // Host for APNS
            "2195",                            // Port for APNS
            "/tmp/cert.pem",                   // Certificate path (PEM format)
            "/tmp/key.pem");                   // Private key path (PEM format)
    
        // spawn some threads to handle our stuff
        for(int i=0; i<3; ++i)
        {
            threads.create_thread(
                boost::bind(&io_service::run, &io) );
        }

        // post test
        device dev1(apns::key, "12345");
        ps.post(dev1, "{\"aps\" : {\"alert\":\"Test message\"}}");
        
        // run on main thread too
        io.run();
        
        // wait for all threads to finish spinning
        threads.join_all();
    }
    catch(std::exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    
    return 0;
}

