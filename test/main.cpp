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

void on_apns_error(const uint32_t& ident, const boost::system::error_code& err)
{
    cout << "Error for identifier " << ident << ": "
        << err.category().message(err.value()) << endl;
}

int main(int argc, const char * argv[])
{
    io_service       io;
    
    push_service     ps(io);
    io_service::work w(io); // don't you dare die on me
    
    device dev1(apns::key, base64_decode("9pfzxPgLrG/8DM8zYXcwUEId2lH0G8dq+jlkh72HXMQ="));
    
    try
    {
        apns apple_push(ps,
            "gateway.sandbox.push.apple.com",      // Host for APNS
            "2195",                                // Port for APNS
            "/tmp/cert.pem",                       // Certificate path (PEM format)
            "/tmp/key.pem",                        // Private key path (PEM format)
            boost::bind(&on_apns_error, _1, _2) ); // Error reporting callback (optional)

        ps.post(dev1, "{\"aps\" : {\"alert\":\"Foo bar\"}}", 200, 12320); // success
        ps.post(dev1, "", 200, 11111); // fail and report into callback
        
        io.run();
    }
    catch(std::exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    
    return 0;
}

