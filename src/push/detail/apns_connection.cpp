//
//  apns_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/apns_connection.hpp>

namespace push {
namespace detail {

    apns_connection::apns_connection(boost::asio::io_service& io)
    : io_service_(io)
    {
    }

    void apns_connection::perform(const apns_request& req)
    {
        
    }
    
    void apns_connection::start(boost::asio::ssl::context& context, boost::asio::ip::tcp::resolver::iterator iterator)
    {
        socket_ = boost::shared_ptr<ssl_socket_t>( new ssl_socket_t(io_service_, context) );
        
        socket_->set_verify_mode(boost::asio::ssl::verify_peer);
        socket_->set_verify_callback(boost::bind(&apns_connection::verify_cert, this, _1, _2));
        
        boost::asio::async_connect(socket_->lowest_layer(), iterator,
                                   boost::bind(&apns_connection::handle_connect, this,
                                               boost::asio::placeholders::error));
    }
    
    bool apns_connection::verify_cert(bool accept_any,
                                      boost::asio::ssl::verify_context& ctx)
    {
        return accept_any;
    }
    
    void apns_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(boost::asio::ssl::stream_base::client,
                                    boost::bind(&apns_connection::handle_handshake, this,
                                                boost::asio::placeholders::error));
        }
        else
        {
            throw std::runtime_error("APNS connection failed: " + error.message());
        }
    }
    
    void apns_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (!error)
        {
            /*
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(request_, req_len_),
                                     boost::bind(&apns_connection::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
             */
        }
        else
        {
            throw std::runtime_error("APNS handshake failed: " + error.message());
        }
    }

    const bool apns_connection::is_available() const
    {
        return !current_req_;
    }
    
} // namespace detail
} // namespace push