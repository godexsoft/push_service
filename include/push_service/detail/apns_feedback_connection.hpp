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

#include <push_service/log.hpp>
#include <push_service/detail/apns_request.hpp>
#include <push_service/detail/connection_pool.hpp>

namespace push {
namespace detail {

    class apns_feedback_connection
    {
    public:
        typedef boost::function<
            void(const boost::system::error_code&,
                 const std::string&,
                 const boost::posix_time::ptime&)>
            callback_type;

        typedef connection_pool<apns_feedback_connection, apns_request>
            pool_type;

        friend class connection_pool<apns_feedback_connection, apns_request>;
        typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_t;

        apns_feedback_connection(pool_type& pool, const boost::posix_time::time_duration& confirmation_delay,
                                 const log_callback_type& log_callback);

        ~apns_feedback_connection() { close_socket(); }

        void start(boost::asio::ssl::context* const context,
                   boost::asio::ip::tcp::resolver::iterator iterator,
                   const callback_type& cb,
                   const std::string& ca_certificate_path);
        void restart();
        
    private:
        void close_socket();
        boost::asio::io_service& get_io_service();
        void stop();

        void handle_connect(const boost::system::error_code& error);
        void handle_handshake(const boost::system::error_code& error);
        void handle_read(const boost::system::error_code& err, size_t bytes_transferred);

        boost::asio::io_service         io_service_;
        boost::asio::io_service::work   work_;
        boost::asio::strand             strand_;

        boost::shared_ptr<ssl_socket_t> socket_;

        // FIXME: pointers are not very nice. however, the connection object MUST
        // outlive the connection_pool and the holder of connection pool by design.
        // We just must be sure that the ssl context is never destroyed before this instance.
        boost::asio::ssl::context* ssl_ctx_;
        boost::asio::ip::tcp::resolver::iterator resolved_iterator_;

        boost::asio::streambuf response_;
        callback_type on_feedback_;
        std::string ca_certificate_path_;
        log_callback_type log_callback_;
    };

} // namespace detail
} // namespace push

#endif // _PUSH_SERVICE_APNS_FEEDBACK_CONNECTION_HPP_
