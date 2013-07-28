//
//  apns_connection.cpp
//  push_service
//
//  Created by Alexander Kremer on 08/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#include <push/detail/apns_connection.hpp>
#include <push/detail/connection_pool.hpp>
#include <push/exception/apns_exception.hpp>

namespace push {
namespace detail {

    using namespace boost::asio;
    
    apns_connection::apns_connection(pool_type& pool)
    : work_(io_service_)
    , cache_check_timer_(io_service_)
    , pool_(pool)
    {
    }

    void apns_connection::wait_for_job()
    {
        // schedule async job retrieval.
        // note: the old handle, if any, will be destroyed
        // and the async_condition_variable waiter will be disconnected.
        wait_handle_ =
            pool_.async_wait_for_job(
                boost::bind(&apns_connection::handle_job_available, this));
    }

    void apns_connection::handle_job_available()
    {
        boost::optional<apns_request> job = pool_.get_next_job();
        
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
            // got new job. add it to cache.. request time resets automatically.
            current_req_ = *job;
            cache_.push_back(current_req_);
            
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
        if (error) // ssl error
        {
            // this happens when socket is closed
            // FIXME: safe to ignore?
            return;
        }
        
        wait_for_job(); // get next job
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
            cache_.erase(cache_.begin(), it);
            
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
            throw push::exception::apns_exception("read error: " + error.message());
        }
                
        // reconnect because Apple closes the socket on error
        start(ssl_ctx_, resolved_iterator_, callback_);
    }
    
    void apns_connection::start(ssl::context* const context,
                                ip::tcp::resolver::iterator iterator,
                                const callback_type& cb)
    {
        ssl_ctx_ = context; // can't copy so has to be pointer
        resolved_iterator_ = iterator; // copy is fine
        callback_ = cb;
        
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
            throw push::exception::apns_exception("apns connection failed: " + error.message());
        }
    }
    
    void apns_connection::handle_handshake(const boost::system::error_code& error)
    {
        if (error)
        {
            throw push::exception::apns_exception("apns handshake failed: " + error.message());
        }
        
        // ok. connection is established. lets sit and try to read to handle any errors.
        // note: this async_read will not block the connection and client's possibility
        // to push messages over this pipe.
        async_read(*socket_, response_,
            transfer_exactly(6), // exactly six bytes must be set for a valid reply
            boost::bind(&apns_connection::handle_read_err, this,
                placeholders::error));
        
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
            boost::bind(&apns_connection::on_check_cache,
                this, boost::asio::placeholders::error));
    }
    
    void apns_connection::on_check_cache(const boost::system::error_code& err)
    {
        if(err != boost::asio::error::operation_aborted)
        {
            boost::posix_time::ptime now( boost::posix_time::microsec_clock::local_time());
            boost::posix_time::ptime check_time = now - boost::posix_time::seconds(120); // TODO: configurable amount
            
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