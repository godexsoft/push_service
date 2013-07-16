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
#include <boost/function.hpp>
#include <deque>

#include <push/detail/apns_request.hpp>
#include <push/detail/apns_response.hpp>

namespace push {
namespace detail {

    template <typename T, typename J> class connection_pool;
    
    class apns_connection
    {
    public:
        typedef boost::function<void(const boost::system::error_code&,
            const uint32_t&)> callback_type;
        
        typedef connection_pool<apns_connection, apns_request>
            pool_type;
        
        friend class connection_pool<apns_connection, apns_request>;
        typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_t;
        
        apns_connection(connection_pool<apns_connection, apns_request>& pool);

        void start(boost::asio::ssl::context* const context,
                   boost::asio::ip::tcp::resolver::iterator iterator,
                   const callback_type& cb);
        
    private:
        void sort_cache_on_error(const push::detail::apns_response& resp);
        void reset_cache_checker();
        void on_check_cache(const boost::system::error_code& err);
        
        boost::asio::io_service& get_io_service();
        void stop();    
        
        void wait_for_job();        
        bool verify_cert(bool accept_any, boost::asio::ssl::verify_context& ctx);
        void handle_connect(const boost::system::error_code& error);        
        void handle_handshake(const boost::system::error_code& error);
        void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
        void handle_read_err(const boost::system::error_code& err);        
        
        boost::asio::io_service         io_service_;
        boost::asio::io_service::work   work_;
        boost::asio::deadline_timer     cache_check_timer_;

        boost::shared_ptr<ssl_socket_t> socket_;
        pool_type& pool_;
        
        // FIXME: pointers are not very nice. however, the connection object MUST
        // outlive the connection_pool and the holder of connection pool by design.
        // We just must be sure that the ssl context is never destroyed before this instance.
        boost::asio::ssl::context* ssl_ctx_;
        boost::asio::ip::tcp::resolver::iterator resolved_iterator_;
        
        apns_request current_req_;
        boost::asio::streambuf response_;
        std::deque<apns_request> cache_;
        
        callback_type callback_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_APNS_CONNECTION_HPP_
