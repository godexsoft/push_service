//
//  apns_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/apns_connection.hpp>
#include <push/detail/apns_response.hpp>
#include <push/detail/connection_pool.hpp>

namespace push {
namespace detail {

    using namespace boost::asio;
    
    apns_connection::apns_connection(connection_pool<apns_connection, apns_request>& pool)
    : work_(io_service_)
    , pool_(pool)
    {
    }

    void apns_connection::wait_for_job()
    {
        // wait on condition with timeout
        if(!pool_.get_next_job(current_req_, boost::posix_time::milliseconds(1000)))
        {
            // try again.
            // note: this technique helps to keep the async_read going.
            io_service_.post(boost::bind(&apns_connection::wait_for_job, this));
        }
        else
        {
            async_write(*socket_,
                buffer(current_req_.body_, current_req_.len_),
                boost::bind(&apns_connection::handle_write, this,
                    placeholders::error,
                    placeholders::bytes_transferred));
        }
    }
    
    void apns_connection::handle_write(const boost::system::error_code& error,
                                       size_t bytes_transferred)
    {
        if (error)
        {
            std::cerr << "Write failed: " + error.message() + "\n";
            // TODO: report
        }
        
        wait_for_job(); // get next job
    }
    
    void apns_connection::handle_read_err(const boost::system::error_code& err)
    {
        // if we got here the connection is not usable anymore.
        // note: !err means we got some data but according to apple
        // they only send data when something went wrong.
        if (!err)
        {
            if(on_error_)
            {
                push::detail::apns_response resp(response_);
                on_error_(resp.identity_, resp.to_error_code());
            }
        }
        else if (err != boost::asio::error::eof)
        {
            std::cout << "Error: " << err << "\n";
        }

        // reconnect because Apple closes the socket on error
        start(ssl_ctx_, resolved_iterator_, on_error_);
    }
    
    void apns_connection::start(ssl::context* const context,
                                ip::tcp::resolver::iterator iterator,
                                const error_callback_type& cb)
    {
        ssl_ctx_ = context; // can't copy so has to be pointer
        resolved_iterator_ = iterator; // copy is fine
        on_error_ = cb;
        
        socket_ = boost::shared_ptr<ssl_socket_t>( new ssl_socket_t(io_service_, *context) );
        
        socket_->set_verify_mode(boost::asio::ssl::verify_peer);
        socket_->set_verify_callback(boost::bind(&apns_connection::verify_cert, this, _1, _2));
        
        async_connect(socket_->lowest_layer(), iterator,
            boost::bind(&apns_connection::handle_connect, this,
                placeholders::error));
    }
    
    bool apns_connection::verify_cert(bool accept_any,
                                      boost::asio::ssl::verify_context& ctx)
    {
        // TODO: some real verification?
        return accept_any;
    }
    
    void apns_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(ssl::stream_base::client,
                boost::bind(&apns_connection::handle_handshake, this,
                    placeholders::error));
        }
        else
        {
            throw std::runtime_error("APNS connection failed: " + error.message());
        }
    }
    
    void apns_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (error)
        {
            throw std::runtime_error("APNS handshake failed: " + error.message());
        }
        
        // ok. connection is established. lets sit and try to read to handle any errors.
        // note: this async_read will not block the connection and client's possibility
        // to push messages over this pipe.
        // read the answer if any
        async_read(*socket_, response_,
            transfer_at_least(6), // exactly six bytes must be set for a valid reply
            boost::bind(&apns_connection::handle_read_err, this,
                placeholders::error));
        
        // finally, allow clients to (re)use this connection.
        wait_for_job();
    }
    
    boost::asio::io_service& apns_connection::get_io_service()
    {
        return io_service_;
    }
    
    void apns_connection::stop()
    {
        io_service_.stop();
    }

} // namespace detail
} // namespace push