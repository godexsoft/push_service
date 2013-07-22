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
#include <push/detail/async_condition_variable.hpp>
#include <boost/thread/condition_variable.hpp>

#include "util.hpp"

using namespace boost::asio;
using namespace boost;
using namespace std;

using namespace push;

void on_apns(const boost::system::error_code& err, const uint32_t& ident)
{
    if(!err)
    {
        cout << "Item with identifier " << ident
            << " is sent successfully\n";
    }
    else
    {
        cout << "Error for identifier " << ident << ": "
            << err.message() << endl;
    }
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

void on_timer_good(deadline_timer& t, push_service& ps)
{
    static uint32_t count = 10000;
    
    device dev1(apns::key, base64_decode("9pfzxPgLrG/8DM8zYXcwUEId2lH0G8dq+jlkh72HXMQ="));
    ps.post(dev1, "{\"aps\" : {\"alert\":\"Valid\"}}", 1, count++);
    
    // reset timer
    t.expires_from_now(posix_time::millisec(1));
    t.async_wait(boost::bind(&on_timer_good, boost::ref(t), boost::ref(ps)));
}

void on_timer_bad(deadline_timer& t, push_service& ps)
{
    static uint32_t count = 1;
    
    device dev1(apns::key, base64_decode("9pfzxPgLrG/8DM8zYXcwUEId2lH0G8dq+jlkh72HXMQ="));
    ps.post(dev1, "", 1, count++);
    
    // reset timer
    t.expires_from_now(posix_time::millisec(10));
    t.async_wait(boost::bind(&on_timer_bad, boost::ref(t), boost::ref(ps)));
}

int main(int argc, const char * argv[])
{
    try
    {
        io_service       io;
        push_service     ps(io);        
        io_service::work w(io); // don't you dare die on me
        
        // configure apns for sandbox mode
        apns::config apple_dev(
           apns::config::sandbox(
               "/tmp/cert.pem",                       // Certificate path (PEM format)
               "/tmp/key.pem"                         // Private key path (PEM format)
           )
        );
        
        apple_dev.callback =
            boost::bind(&on_apns, _1, _2);            // Optional. defaults to no callback        
        apple_dev.pool_size = 8;                      // Optional. defaults to 4
        
        // create the apns push service runner
        apns apple_push(ps, apple_dev);
 
        // configure apns feedback for sandbox mode
        apns_feedback::config apple_dev_feed(
            apns_feedback::config::sandbox(
                "/tmp/cert.pem",                      // Certificate path (PEM format)
                "/tmp/key.pem"                        // Private key path (PEM format)
            )
        );
        
        apple_dev_feed.callback =
            boost::bind(&on_feed, _1, _2, _3);
        
        // create the apns feedback listener
        apns_feedback apple_feedback(ps, apple_dev_feed);

        // and start getting the feed
        apple_feedback.start();
        
        // test some message dispatching
        deadline_timer t1(io);
        deadline_timer t2(io);
  
        // normal message every 1 ms
        t1.expires_from_now(posix_time::millisec(1));
        t1.async_wait(boost::bind(&on_timer_good, boost::ref(t1), boost::ref(ps)));
        
        
        // bad message every 10 ms
        t2.expires_from_now(posix_time::millisec(10));
        t2.async_wait(boost::bind(&on_timer_bad, boost::ref(t2), boost::ref(ps)));
        
        io.run();
    }
    catch(std::exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    
    return 0;
}

