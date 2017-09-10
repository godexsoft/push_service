//
//  apns_feedback_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 12/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push_service/detail/apns_feedback_connection.hpp>
#include <push_service/detail/apns_response.hpp>
#include <push_service/exception/apns_exception.hpp>

namespace push {
namespace detail {

    using namespace boost::asio;

    apns_feedback_connection::apns_feedback_connection(pool_type& /*pool*/,
            const boost::posix_time::time_duration& /*confirmation_delay*/, const log_callback_type& log_callback)
        : work_(io_service_)
        , strand_(io_service_)
        , log_callback_(log_callback)
    {
    }

    void apns_feedback_connection::handle_read(const boost::system::error_code& error,
                                               size_t bytes_transferred)
    {
        if (!error)
        {
            boost::system::error_code ec(
                push::error::no_apns_error, push::error::apns_error_category);

            for(uint32_t i=0; i<bytes_transferred/38; ++i)
            {
                // transfer and parse packet
                boost::asio::streambuf::const_buffers_type bufs = response_.data();
                std::string packet_data(boost::asio::buffers_begin(bufs),
                                        boost::asio::buffers_begin(bufs) + 38);

                response_.commit(38);

                push::detail::apns_feedback_response resp(packet_data, log_callback_);
                on_feedback_(ec, resp.get_token(), resp.get_time());
            }

            async_read(*socket_, response_,
                strand_.wrap(boost::bind(&apns_feedback_connection::handle_read, this,
                    placeholders::error, placeholders::bytes_transferred)));
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
            if(on_feedback_)
            {
                on_feedback_(error, std::string(), boost::posix_time::ptime());
            }

            throw push::exception::apns_exception(
                "feedback read error: " + error.message());
        }
    }

    void apns_feedback_connection::start(ssl::context* const context,
                                         ip::tcp::resolver::iterator iterator,
                                         const callback_type& cb,
                                         const std::string& ca_certificate_path)
    {
        PUSH_LOG("Connection start", LogLevel::INFO);
        ssl_ctx_ = context; // can't copy so has to be pointer
        resolved_iterator_ = iterator; // copy is fine
        on_feedback_ = cb;
        ca_certificate_path_ = ca_certificate_path;

        socket_ = boost::shared_ptr<ssl_socket_t>( new ssl_socket_t(io_service_, *context) );

        if (!ca_certificate_path_.empty())
        {
            context->load_verify_file(ca_certificate_path_);
            socket_->set_verify_mode(boost::asio::ssl::verify_peer);
        }

        async_connect(socket_->lowest_layer(), iterator,
            strand_.wrap(boost::bind(&apns_feedback_connection::handle_connect, this,
                placeholders::error)));
    }

    void apns_feedback_connection::restart()
    {
        close_socket();
        start(ssl_ctx_, resolved_iterator_, on_feedback_, ca_certificate_path_);
    }

    void apns_feedback_connection::close_socket()
    {
        if (socket_)
            socket_->lowest_layer().cancel();
         socket_.reset();
    }

    void apns_feedback_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(ssl::stream_base::client,
                strand_.wrap(boost::bind(&apns_feedback_connection::handle_handshake, this,
                    placeholders::error)));
        }
        else
        {
            if(on_feedback_)
            {
                on_feedback_(error, std::string(), boost::posix_time::ptime());
            }

            throw push::exception::apns_exception(
                "apns feedback connection failed: " + error.message());
        }
    }

    void apns_feedback_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (error)
        {
            if(on_feedback_)
            {
                on_feedback_(error, std::string(), boost::posix_time::ptime());
            }

            throw push::exception::apns_exception(
                "apns feedback handshake failed: " + error.message());
        }

        PUSH_LOG("Connection established", LogLevel::INFO);
        async_read(*socket_, response_,
           strand_.wrap(boost::bind(&apns_feedback_connection::handle_read, this,
                placeholders::error, placeholders::bytes_transferred)));
    }

    boost::asio::io_service& apns_feedback_connection::get_io_service()
    {
        return io_service_;
    }

    void apns_feedback_connection::stop()
    {
        io_service_.stop();
    }

} // namespace detail
} // namespace push
