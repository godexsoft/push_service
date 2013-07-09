//
//  apns_connection.hpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _PUSH_SERVICE_APNS_CONNECTION_HPP_
#define _PUSH_SERVICE_APNS_CONNECTION_HPP_

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

#include <push/detail/apns_request.hpp>

namespace push {
namespace detail {

    class apns_connection
    {
    public:
        typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_t;
        
        apns_connection(boost::asio::io_service& io);

        void start(boost::asio::ssl::context& context,
                   boost::asio::ip::tcp::resolver::iterator iterator);

        const bool is_available() const;

        /// Performs a request over this connection
        void perform(const apns_request& req);
        
    private:
        
        bool verify_cert(bool accept_any, boost::asio::ssl::verify_context& ctx);
        void handle_connect(const boost::system::error_code& error);        
        void handle_handshake(const boost::system::error_code& error);
        
        void reset()
        {
            current_req_ = boost::shared_ptr<apns_request>();
        }
        
        boost::asio::io_service& io_service_;
        boost::shared_ptr<ssl_socket_t> socket_;
        
        boost::shared_ptr<apns_request> current_req_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_APNS_CONNECTION_HPP_
