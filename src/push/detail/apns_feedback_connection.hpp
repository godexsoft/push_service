//
//  apns_feedback_connection.hpp
//  push_service
//
//  Created by Alexander Kremer on 12/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_APNS_FEEDBACK_CONNECTION_HPP_
#define _PUSH_SERVICE_APNS_FEEDBACK_CONNECTION_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace push {
namespace detail {

    class apns_feedback_connection
    {
    public:
        typedef boost::function<
            void(boost::system::error_code&,
                 const std::string&,
                 const boost::posix_time::ptime&)>
            callback_type;
        
        typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_t;
        
        apns_feedback_connection(boost::asio::io_service& io,
                                 const callback_type& cb);

        void start(boost::asio::ssl::context& context,
                   boost::asio::ip::tcp::resolver::iterator iterator);
        
    private:
        void stop();
        
        void handle_connect(const boost::system::error_code& error);        
        void handle_handshake(const boost::system::error_code& error);
        void handle_read(const boost::system::error_code& err, size_t bytes_transferred);
        
        boost::asio::io_service&  io_service_;
        boost::shared_ptr<ssl_socket_t> socket_;
        
        boost::asio::streambuf response_;
        callback_type on_feedback_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_APNS_FEEDBACK_CONNECTION_HPP_
