//
//  apns_feedback_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 12/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/apns_feedback_connection.hpp>
#include <push/detail/apns_response.hpp>
#include <push/exception/apns_exception.hpp>

namespace push {
namespace detail {
    
    using namespace boost::asio;

    apns_feedback_connection::apns_feedback_connection(boost::asio::io_service& io,
                                                       const callback_type& cb)
    : io_service_(io)
    , on_feedback_(cb)
    {
    }

    void apns_feedback_connection::handle_read(const boost::system::error_code& error,
                                               size_t bytes_transferred)
    {
        if (!error)
        {
            boost::system::error_code ec(
                push::error::no_apns_error, push::error::apns_error_category);
                    
            for(uint32_t i=0; i<bytes_transferred%38; ++i)
            {
                // transfer and parse packet
                boost::asio::streambuf::const_buffers_type bufs = response_.data();
                std::string packet_data(boost::asio::buffers_begin(bufs),
                                        boost::asio::buffers_begin(bufs) + 38);

                response_.commit(38);
                
                push::detail::apns_feedback_response resp(packet_data);
                on_feedback_(ec, resp.get_token(), resp.get_time());
            }
            
            async_read(*socket_, response_,
                boost::bind(&apns_feedback_connection::handle_read, this,
                    placeholders::error, placeholders::bytes_transferred));
        }
        else if (error == boost::asio::error::eof)
        {
            // socket was closed. report back to the client
            boost::system::error_code ec(
                push::error::shutdown, push::error::apns_error_category);
            
            on_feedback_(ec, std::string(), boost::posix_time::ptime());
        }
        else
        {
            throw push::exception::apns_exception(
                "feedback read error: " + error.message());
        }
    }
    
    void apns_feedback_connection::start(ssl::context& context,
                                         ip::tcp::resolver::iterator iterator)
    {
        context.set_default_verify_paths();
        socket_ = boost::shared_ptr<ssl_socket_t>( new ssl_socket_t(io_service_, context) );
        socket_->set_verify_mode(boost::asio::ssl::verify_peer);
        
        async_connect(socket_->lowest_layer(), iterator,
            boost::bind(&apns_feedback_connection::handle_connect, this,
                placeholders::error));
    }
    
    void apns_feedback_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(ssl::stream_base::client,
                boost::bind(&apns_feedback_connection::handle_handshake, this,
                    placeholders::error));
        }
        else
        {
            throw push::exception::apns_exception(
                "apns feedback connection failed: " + error.message());
        }
    }
    
    void apns_feedback_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (error)
        {
            throw push::exception::apns_exception(
                "apns feedback handshake failed: " + error.message());
        }
        
        async_read(*socket_, response_,
            boost::bind(&apns_feedback_connection::handle_read, this,
                placeholders::error, placeholders::bytes_transferred));
    }
    
    void apns_feedback_connection::stop()
    {
        socket_ = boost::shared_ptr<ssl_socket_t>();
    }

} // namespace detail
} // namespace push