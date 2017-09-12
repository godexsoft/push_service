//
//  apns_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push_service/detail/apns_connection.hpp>
#include <push_service/detail/connection_pool.hpp>
#include <push_service/exception/apns_exception.hpp>

namespace push {
namespace detail {

    using namespace boost::asio;

    apns_connection::apns_connection(pool_type& pool, boost::posix_time::time_duration confirmation_delay, const log_callback_type& log_callback)
    : work_(io_service_)
    , strand_(io_service_)
    , cache_check_timer_(io_service_)
    , pool_(pool)
    , confirmation_delay_(confirmation_delay)
    , log_callback_(log_callback)
    {
    }

    void apns_connection::wait_for_job()
    {
        // schedule async job retrieval.
        // note: the old handle, if any, will be destroyed
        // and the async_condition_variable waiter will be disconnected.
        wait_handle_ =
            pool_.async_wait_for_job(
                strand_.wrap(boost::bind(&apns_connection::handle_job_available, this) ) );
    }

    void apns_connection::handle_job_available()
    {
        std::vector<const_buffer> bufs;
        for (int i = 0; i < 16; ++i)
        {
            boost::optional<apns_request> job = pool_.get_next_job();
            if (!job)
               break;
            else
            {
                // got new job. add it to cache.. request time resets automatically.
                cache_.push_back( *job );
                if (bufs.empty())
                {
                   bufs.reserve(16);
                   current_req_ = *job;
                }
                bufs.emplace_back( job->body_->data(), job->body_->size() );
             }
        }

        if (bufs.empty()) // schedule again if no jobs available
            wait_for_job();
        else
            async_write( *socket_, bufs,
                strand_.wrap( boost::bind(&apns_connection::handle_write, this,
                placeholders::error,
                placeholders::bytes_transferred, socket_) ) );
    }

    void apns_connection::handle_write(const boost::system::error_code& error,
                                       size_t /*bytes_transferred*/, boost::shared_ptr<ssl_socket_t> socket)
    {
        if (error) // ssl error
        {
            // this happens when socket is closed
            // FIXME: safe to ignore?
            return;
        }

        handle_job_available(); // get next job
    }

    void apns_connection::sort_cache_on_error(const push::detail::apns_response& resp)
    {
        std::deque<apns_request>::iterator it =
            std::find_if(cache_.begin(), cache_.end(),
                boost::bind(&apns_request::get_identity, _1)
                    == resp.get_identity() );

        if(it != cache_.end())
        {
            // consider all before this one - success
            if(callback_)
            {
                std::for_each(cache_.begin(), it,
                    boost::bind(callback_, boost::system::error_code(),
                        boost::bind(&apns_request::get_identity, _1)) );
            }

            // and now remove them
            it = cache_.erase(cache_.begin(), it);

            // report error
            if(callback_)
            {
                callback_(resp.to_error_code(), resp.get_identity());
            }

            // remove the errorneous item
            it = cache_.erase(it);

            // now for all the rest - enque them back into the pool
            pool_.post(it, cache_.end());

            // and finally clear the cache
            cache_.clear();
        }
        else
        {
            if(callback_)
            {
                callback_(resp.to_error_code(), resp.get_identity());
            }

            // should not happen often or at all if the check_time is big enough
            throw push::exception::apns_exception("lost failed APNS message in cache");
        }
    }

    void apns_connection::sort_cache_on_shutdown(const push::detail::apns_response& resp)
    {
        std::deque<apns_request>::iterator it =
            std::find_if(cache_.begin(), cache_.end(),
                boost::bind(&apns_request::get_identity, _1)
                    == resp.get_identity() );

        // handle if found in the cache.
        if(it != cache_.end())
        {
            // consider all before and including this one - success
            if(callback_)
            {
                std::for_each(cache_.begin(), it,
                    boost::bind(callback_, boost::system::error_code(),
                        boost::bind(&apns_request::get_identity, _1)) );

                // and report the last successful too
                callback_(boost::system::error_code(), it->get_identity());
            }

            // and remove all successful
            cache_.erase(cache_.begin(), it);
            it = cache_.erase(it);

            // now for all the rest - enque them back into the pool
            pool_.post(it, cache_.end());

            // and finally clear the cache
            cache_.clear();
        }
        else
        {
            // it's not in the cache already and
            // it probably means that the message was already
            // processed successfully with callback invoked.
            // however we still need to enque all messages currently in the cache.
            it = cache_.begin();
            pool_.post(it, cache_.end());

            // and clear the cache
            cache_.clear();
        }
    }

    void apns_connection::handle_read_err(const boost::system::error_code& error)
    {
        cache_check_timer_.cancel();

        // if we got here the connection is not usable anymore.
        // note: !error means we got some data but according to apple
        // they only send data when something went wrong.
        if (!error)
        {
            push::detail::apns_response resp(response_);

            if(resp.to_error_code() == push::error::shutdown)
            {
                sort_cache_on_shutdown(resp);
            }
            else
            {
                // sort out the cache and error
                sort_cache_on_error(resp);
            }
        }
        else if (error != boost::asio::error::eof)
        {
            if(callback_)
            {
                callback_(error, current_req_.get_identity());
            }

            throw push::exception::apns_exception("read error: " + error.message());
        }

        // reconnect because Apple closes the socket on error
        restart();
    }

    void apns_connection::start(ssl::context* const context,
                                ip::tcp::resolver::iterator iterator,
                                const callback_type& cb,
                                const std::string& ca_certificate_path)
    {
        PUSH_LOG("Connection start", LogLevel::INFO);
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
            strand_.wrap( boost::bind(&apns_connection::handle_connect, this,
                placeholders::error) ) );
    }

    void apns_connection::restart()
    {
        close_socket();
        start(ssl_ctx_, resolved_iterator_, callback_, ca_certificate_path_);
    }

    void apns_connection::close_socket()
    {
        if (socket_)
            socket_->lowest_layer().cancel();
         socket_.reset();
    }

    void apns_connection::handle_connect(const boost::system::error_code& error)
    {
        if (!error)
        {
            socket_->async_handshake(ssl::stream_base::client,
                strand_.wrap( boost::bind(&apns_connection::handle_handshake, this,
                    placeholders::error) ) );
        }
        else
        {
            if(callback_)
            {
                callback_(error, current_req_.get_identity());
            }

            throw push::exception::apns_exception("apns connection failed: " + error.message());
        }
    }

    void apns_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (error)
        {
            if(callback_)
            {
                callback_(error, current_req_.get_identity());
            }

            throw push::exception::apns_exception("apns handshake failed: " + error.message());
        }

        // ok. connection is established. lets sit and try to read to handle any errors.
        // note: this async_read will not block the connection and client's possibility
        // to push messages over this pipe.
        PUSH_LOG("Connection established", LogLevel::INFO);
        async_read(*socket_, response_,
            transfer_exactly(6), // exactly six bytes must be set for a valid reply
                strand_.wrap( boost::bind(&apns_connection::handle_read_err, this,
                    placeholders::error) ) );

        // finally, allow clients to (re)use this connection.
        reset_cache_checker();
        wait_for_job();
    }

    boost::asio::io_service& apns_connection::get_io_service()
    {
        return io_service_;
    }

    void apns_connection::reset_cache_checker()
    {
        cache_check_timer_.expires_from_now(boost::posix_time::seconds(1));
        cache_check_timer_.async_wait(
            strand_.wrap( boost::bind(&apns_connection::on_check_cache,
                this, boost::asio::placeholders::error) ) );
    }

    void apns_connection::on_check_cache(const boost::system::error_code& err)
    {
        if(err != boost::asio::error::operation_aborted)
        {
            boost::posix_time::ptime now( boost::posix_time::microsec_clock::local_time());
            boost::posix_time::ptime check_time = now - confirmation_delay_;

            std::deque<apns_request>::iterator it = cache_.begin();

            while(! cache_.empty())
            {
                if(it->get_time() < check_time)
                {
                    if(callback_)
                    {
                        callback_(boost::system::error_code(), it->get_identity());
                    }

                    // remove this item
                    it = cache_.erase(it);
                }
                else
                {
                    // leave the rest of the cache for now
                    break;
                }
            }

            // in a normal situation we want this to go on
            reset_cache_checker();
        }
    }

    void apns_connection::stop()
    {
        io_service_.stop();
    }

} // namespace detail
} // namespace push
