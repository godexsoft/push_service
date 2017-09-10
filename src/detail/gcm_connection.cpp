//
//  gcm_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push_service/detail/gcm_connection.hpp>
#include <push_service/detail/connection_pool.hpp>

#include <sstream>

namespace push {
namespace detail {

    class match_exact_size
    {
    public:
        explicit match_exact_size(size_t sz) : size_(sz)
        {
        }

        template <typename Iterator>
        std::pair<Iterator, bool> operator()(
            Iterator begin, Iterator end) const
        {
            size_t cnt = 0;

            Iterator i = begin;
            while (i != end)
            {
                if(++cnt >= size_)
                {
                    return std::make_pair(i, true);
                }
            }

            return std::make_pair(i, false);
        }

    private:
        size_t size_;
    };

} // namespace detail
} // namespace push


namespace boost {
namespace asio {

    template <> struct is_match_condition<push::detail::match_exact_size>
        : public boost::true_type {};

} // namespace asio
} // namespace boost


namespace push {
namespace detail {

    using namespace boost::asio;

    gcm_connection::gcm_connection(pool_type& pool, const boost::posix_time::time_duration /*confirmation_delay*/, const log_callback_type& log_callback)
    : work_(io_service_)
    , strand_(io_service_)
    , pool_(pool)
    , new_request_(true)
    , log_callback_(log_callback)
    {
    }

    void gcm_connection::wait_for_job()
    {
        // schedule async job retrieval.
        // note: the old handle, if any, will be destroyed
        // and the async_condition_variable waiter will be disconnected.
        wait_handle_ =
            pool_.async_wait_for_job(
                strand_.wrap(boost::bind(&gcm_connection::handle_job_available, this) ) );
    }

    void gcm_connection::handle_job_available()
    {
        boost::optional<gcm_request> job = pool_.get_next_job();

        // there is always chance that we will not get a job
        // because another connection worker was faster.
        // using notify_one this should be a minimal chance however.
        if(!job)
        {
            // schedule again if no job available
            wait_for_job();
        }
        else
        {
            current_req_ = *job;
            current_res_json_.resize(0);

            async_write(*socket_,
                buffer(current_req_.request_.data()),
                    strand_.wrap(boost::bind(&gcm_connection::handle_write, this,
                        placeholders::error,
                        placeholders::bytes_transferred) ) );
        }
    }

    void gcm_connection::handle_write(const boost::system::error_code& error,
                                       size_t /*bytes_transferred*/ )
    {
        if (error)
        {
            throw std::runtime_error("gcm write error");
        }

        if(new_request_)
        {
            // read status line
            async_read_until(*socket_, response_, "\r\n",
                strand_.wrap( boost::bind(&gcm_connection::handle_read_statusline, this,
                    placeholders::error) ) );
        }
        else
        {
            // read the headers
            async_read_until(*socket_, response_, "\r\n\r\n",
                strand_.wrap(boost::bind(&gcm_connection::handle_read_headers, this,
                    placeholders::error) ) );
        }
    }

    void gcm_connection::handle_read_statusline(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::istream response_stream(&response_);

            std::string http_version;
            response_stream >> http_version;

            unsigned int status_code;
            response_stream >> status_code;

            std::string status_message;
            std::getline(response_stream, status_message);

            if (!response_stream || http_version.substr(0, 5) != "HTTP/")
            {
                current_res_ = push::detail::gcm_response(push::error::invalid_response);

                if(callback_)
                {
                    callback_(current_res_.to_error_code(), current_req_.ident_);
                }

                // copmpletely restart connection
                restart();

                return;
            }

            if (status_code != 200)
            {
                current_res_ = push::detail::gcm_response(
                    static_cast<push::error::gcm_err_code>(status_code) );

                if(callback_)
                {
                    callback_(current_res_.to_error_code(), current_req_.ident_);
                }

                // copmpletely restart connection
                restart();

                return;
            }

            // read the headers
            async_read_until(*socket_, response_, "\r\n\r\n",
                strand_.wrap(boost::bind(&gcm_connection::handle_read_headers, this,
                    placeholders::error) ) );
        }
        else
        {
            throw std::runtime_error("gcm error while reading status line: " + error.message());
        }
    }

    void gcm_connection::handle_read_headers(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::istream response_stream(&response_);
            std::string header;

            // read and ignore all headers
            while (std::getline(response_stream, header) && header != "\r");

            // done with status lines and headers for this connection.
            new_request_ = false;

            // read the hex size of first chunk
            async_read_until(*socket_, response_, "\r\n",
                strand_.wrap(boost::bind(&gcm_connection::handle_read_chunk_size, this,
                    placeholders::error) ) );
        }
        else
        {
            throw std::runtime_error("gcm error while reading headers: " + error.message());
        }
    }

    void gcm_connection::handle_read_chunk_size(const boost::system::error_code& error)
    {
        if (!error)
        {
            std::istream response_stream(&response_);

            std::string chunk_size_line;
            std::getline(response_stream, chunk_size_line);

            // remove comment if any
            size_t comment_start = chunk_size_line.find(';');
            if(comment_start != std::string::npos)
            {
                // get rid of everything after and including ';'
                chunk_size_line.resize(comment_start);
            }

            std::stringstream ss;

            ss << chunk_size_line;
            ss >> std::hex >> cur_chunk_size_;

            if(cur_chunk_size_ == 0)
            {
                // end of chunked response. check footers.
                async_read_until(*socket_, response_, "\r\n",
                    strand_.wrap(boost::bind(&gcm_connection::handle_skip_footers, this,
                        placeholders::error) ) );
            }
            else
            {
                // read the chunk body.
                async_read_until(*socket_, response_,
                    match_exact_size(cur_chunk_size_), // don't add 2 for \r\n
                    strand_.wrap(boost::bind(&gcm_connection::handle_read_body, this,
                        placeholders::error) ) );
            }
        }
        else
        {
            throw std::runtime_error("read error: " + error.message());
        }
    }

    void gcm_connection::handle_read_body(const boost::system::error_code& error)
    {
        if (!error)
        {
            // append the data off the streambuf into current_res_json_
            std::istream response_stream(&response_);
            std::vector<char> chunk(cur_chunk_size_, 0);
            response_stream.read(&chunk.at(0), cur_chunk_size_);

            response_.consume(2); // +2 for \r\n
            current_res_json_.append(chunk.begin(), chunk.end());

            // read the hex size of next chunk.
            async_read_until(*socket_, response_, "\r\n",
                strand_.wrap(boost::bind(&gcm_connection::handle_read_chunk_size, this,
                    placeholders::error) ) );
        }
        else
        {
            throw std::runtime_error("read error: " + error.message());
        }
    }

    void gcm_connection::handle_skip_footers(const boost::system::error_code& error)
    {
        if (!error)
        {
            // check if there are any footers
            std::istream response_stream(&response_);
            std::string footer;

            while (std::getline(response_stream, footer) && footer != "\r")
            {
                std::cout << "Footer: " << footer << "\n";
            }

            std::stringstream response_message;
            response_message << "response: " << current_res_json_;
            PUSH_LOG(response_message.str(), LogLevel::INFO);
            current_res_ = push::detail::gcm_response(current_res_json_, log_callback_);

            if(callback_)
            {
                callback_(current_res_.to_error_code(), current_req_.ident_);
            }

            // reuse open connection
            wait_for_job();
        }
        else
        {
            throw std::runtime_error("read error: " + error.message());
        }
    }


    void gcm_connection::start(ssl::context* const context,
                               ip::tcp::resolver::iterator iterator,
                               const callback_type& cb,
                               const std::string& ca_certificate_path)
    {
        ssl_ctx_ = context; // can't copy so has to be pointer
        resolved_iterator_ = iterator; // copy is fine
        callback_ = cb;
        ca_certificate_path_ = ca_certificate_path;

        socket_ = boost::shared_ptr<ssl_socket_t>( new ssl_socket_t(io_service_, *context) );

        if (!ca_certificate_path_.empty())
        {
            context->load_verify_file(ca_certificate_path_);
            socket_->set_verify_mode(boost::asio::ssl::verify_peer);
        }

        async_connect(socket_->lowest_layer(), iterator,
            strand_.wrap(boost::bind(&gcm_connection::handle_connect, this,
                placeholders::error) ) );
    }

    void gcm_connection::restart()
    {
        start(ssl_ctx_, resolved_iterator_, callback_, ca_certificate_path_);
    }

    void gcm_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(ssl::stream_base::client,
                strand_.wrap(boost::bind(&gcm_connection::handle_handshake, this,
                    placeholders::error) ) );
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
        new_request_ = true;
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
