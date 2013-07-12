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

void on_apns_error(const boost::system::error_code& err, const uint32_t& ident)
{
    cout << "Error for identifier " << ident << ": "
        << err.category().message(err.value()) << endl;
}

void on_feed(const boost::system::error_code& err,
             const std::string& token,
             const posix_time::ptime& time)
{
    if(!err)
    {
        cout << "Feedback item: " << time << "\n";
    }
    else if(err == push::error::shutdown)
    {
        cout << "Socket of feedback channel was shutdown by remote host.\n";
    }
}

int main(int argc, const char * argv[])
{
    io_service       io;
    push_service     ps(io);

    io_service::work w(io); // don't you dare die on me
    
    try
    {
        apns apple_push(ps,
            "gateway.sandbox.push.apple.com",      // Host for APNS
            "2195",                                // Port for APNS
            "/tmp/cert.pem",                       // Certificate path (PEM format)
            "/tmp/key.pem",                        // Private key path (PEM format)
            boost::bind(&on_apns_error, _1, _2) ); // Error reporting callback (optional)

        apns_feedback apple_feedback(ps,
            "feedback.sandbox.push.apple.com",     // Host for APNS feedback
            "2196",                                // Port for APNS feedback
            "/tmp/cert.pem",                       // Certificate path (PEM format)
            "/tmp/key.pem",                        // Private key path (PEM format)
            boost::bind(&on_feed, _1, _2, _3) );   // Callback
        
        apple_feedback.start(); // start getting feedback
        
        device dev1(apns::key, base64_decode("9pfzxPgLrG/8DM8zYXcwUEId2lH0G8dq+jlkh72HXMQ="));
        ps.post(dev1, "{\"aps\" : {\"alert\":\"Foo bar\"}}", 0, 12320); // success
        ps.post(dev1, "", 0, 11111); // error to callback
        
        io.run();
    }
    catch(std::exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    
    return 0;
}

