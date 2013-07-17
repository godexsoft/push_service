//
//  gcm_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/gcm_connection.hpp>
#include <push/detail/connection_pool.hpp>

namespace push {
namespace detail {

    using namespace boost::asio;
    
    gcm_connection::gcm_connection(pool_type& pool)
    : work_(io_service_)
    , pool_(pool)
    {
    }

    void gcm_connection::wait_for_job()
    {
        boost::optional<gcm_request> job =
            pool_.get_next_job();
        
        // got new job
        current_req_ = *job;
            
        async_write(*socket_,
            buffer(current_req_.request_.data()),
            boost::bind(&gcm_connection::handle_write, this,
                placeholders::error,
                placeholders::bytes_transferred));
    }
    
    void gcm_connection::handle_write(const boost::system::error_code& error,
                                       size_t bytes_transferred)
    {
        if (error)
        {
            throw std::runtime_error("gcm write error");
        }
        
        // read response
        async_read_until(*socket_, response_, "\r\n\r\n",
            boost::bind(&gcm_connection::handle_read, this,
                placeholders::error));
    }
    
    void gcm_connection::handle_read(const boost::system::error_code& error)
    {
        if (!error)
        {
            // push::detail::gcm_response resp(response_);
        }
        else if (error != boost::asio::error::eof)
        {
            throw std::runtime_error("read error: " + error.message());
        }

        // prepare and warm up a new connection
        start(ssl_ctx_, resolved_iterator_, callback_);
    }
    
    void gcm_connection::start(ssl::context* const context,
                               ip::tcp::resolver::iterator iterator,
                               const callback_type& cb)
    {
        ssl_ctx_ = context; // can't copy so has to be pointer
        resolved_iterator_ = iterator; // copy is fine
        callback_ = cb;
        
        socket_ = boost::shared_ptr<ssl_socket_t>( new ssl_socket_t(io_service_, *context) );
        
        socket_->set_verify_mode(boost::asio::ssl::verify_peer);
        socket_->set_verify_callback(boost::bind(&gcm_connection::verify_cert, this, _1, _2));
        
        async_connect(socket_->lowest_layer(), iterator,
            boost::bind(&gcm_connection::handle_connect, this,
                placeholders::error));
    }
    
    bool gcm_connection::verify_cert(bool accept_any,
                                     boost::asio::ssl::verify_context& ctx)
    {
        // TODO: some real verification?
        return accept_any;
    }
    
    void gcm_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(ssl::stream_base::client,
                boost::bind(&gcm_connection::handle_handshake, this,
                    placeholders::error));
        }
        else
        {
            throw std::runtime_error("gcm connection failed: " + error.message());
        }
    }
    
    void gcm_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (error)
        {
            throw std::runtime_error("gcm handshake failed: " + error.message());
        }
        
        // finally, allow clients to (re)use this connection.
        wait_for_job();
    }
    
    boost::asio::io_service& gcm_connection::get_io_service()
    {
        return io_service_;
    }
    
    void gcm_connection::stop()
    {
        io_service_.stop();
    }

} // namespace detail
} // namespace push