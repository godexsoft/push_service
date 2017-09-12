//
//  apns.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _APNS_PROVIDER_HPP_
#define _APNS_PROVIDER_HPP_

#include <string>
#include <boost/function.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <push_service/json_spirit/json_spirit_writer_template.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <push_service/push_provider.hpp>
#include <push_service/detail/apns_connection.hpp>
#include <push_service/detail/apns_feedback_connection.hpp>
#include <push_service/detail/connection_pool.hpp>

namespace push {

    /**
     * @brief APNS message wrapper
     */
    class apns_message : public push_message
    {
    public:
        apns_message();
        
        // properties
        std::string alert;
        std::string title;
        std::string title_loc_key;
        std::vector<std::string> title_loc_args;
        std::string body;
        std::string loc_key;
        std::vector<std::string> loc_args;
        std::string action_loc_key;
        std::string launch_image;
        uint16_t badge;
        std::string sound;
        
        // TODO: add support for arrays and custom hierarchies.
        // this can be done if we implement add(ptree tree)
        // and a wrapper: add(vector<> arr)
        void add(const std::string& k, const std::string& v);
        
        std::string to_json() const;        
        
    private:
        json_spirit::Object alert_to_json() const;

        std::map<std::string, std::string> custom_;
    };
    
    /**
     * @brief APNS push provider
     */
    class apns : public provider
    {
    public:
        static const uint32_t invalid_ident = detail::apns_request::invalid_ident;
        
        typedef push::detail::connection_pool<
            detail::apns_connection,detail::apns_request>
                ::callback_type
                    callback_type;
        
        /**
         * @brief configuration helper
         */
        struct config
        {
            std::string     host;
            std::string     port;
            std::string     certificate; // pem
            std::string     private_key; // pem
            std::string     p12_cert_key; // .p12 with both cert and key
            std::string     p12_pass; // .p12 password for the above
            std::string     ca_certificate_path;  // CA certificate path
            uint16_t        pool_size;
            boost::posix_time::time_duration restart_delay;
            boost::posix_time::time_duration confirmation_delay;
            callback_type   callback;
            log_callback_type log_callback;

            static const uint16_t sc_default_pool_size;
            static const boost::posix_time::time_duration sc_default_restart_delay;
            static const boost::posix_time::time_duration sc_default_confirmation_delay;

            config()
            : pool_size(sc_default_pool_size)
            , restart_delay(sc_default_restart_delay)
            , confirmation_delay(sc_default_confirmation_delay)
            {
            }

            config(const std::string& h, const std::string& p,
                   const std::string& crt, const std::string& pk)
            : host(h)
            , port(p)
            , certificate(crt)
            , private_key(pk)
            , pool_size(sc_default_pool_size)
            , restart_delay(sc_default_restart_delay)
            , confirmation_delay(sc_default_confirmation_delay)
            {
            }
            
            config(const std::string& h, const std::string& p,
                   const std::string& p12_ck)
            : host(h)
            , port(p)
            , p12_cert_key(p12_ck)
            , pool_size(sc_default_pool_size)
            , restart_delay(sc_default_restart_delay)
            , confirmation_delay(sc_default_confirmation_delay)
            {
            }
            
            static config sandbox(const std::string& crt,
                                  const std::string& pk)
            {
                return config(
                    "gateway.sandbox.push.apple.com",  
                    "2195",
                    crt,
                    pk
                );
            }
            
            static config production(const std::string& crt,
                                     const std::string& pk)
            {
                return config(
                    "gateway.push.apple.com",
                    "2195",
                    crt,
                    pk
                );
            }
            
            static config sandbox(const std::string& p12_ck)
            {
                return config(
                    "gateway.sandbox.push.apple.com",
                    "2195",
                    p12_ck
                );
            }
            
            static config production(const std::string& p12_ck)
            {
                return config(
                    "gateway.push.apple.com",
                    "2195",
                    p12_ck
                );
            }
        };

        static const char* key;

        apns(push_service& ps, const config& cfg);

        bool validate_device(const device& dev) const;

        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t ident);

        uint32_t post(const device& dev, const push_message& msg,
                      const uint32_t expiry, const uint32_t ident);

    private:
        const std::string give_p12_pass(const std::string& pass);
        std::pair<std::string, std::string> cert_key_;
        
        push::detail::connection_pool<
            push::detail::apns_connection,
            push::detail::apns_request> pool_;
        boost::asio::ssl::context ssl_ctx_;
        
        /// callback
        callback_type callback_;
    };
    
    /**
     * @brief APNS feedback reader
     */
    class apns_feedback
    {
    public:
        typedef push::detail::connection_pool<
            detail::apns_feedback_connection, detail::apns_request>
                ::callback_type
                    callback_type;

        /**
         * @brief configuration helper
         */
        struct config
        {
            std::string     host;
            std::string     port;
            std::string     certificate; // pem
            std::string     private_key; // pem
            std::string     p12_cert_key; // .p12 with both cert and key
            std::string     p12_pass; // .p12 password for the above
            std::string     ca_certificate_path;  // CA certificate path
            uint16_t        pool_size;
            boost::posix_time::time_duration restart_delay;
            callback_type   callback;
            log_callback_type log_callback;

            static const uint16_t sc_default_pool_size;
            static const boost::posix_time::time_duration sc_default_restart_delay;

            config()
            : pool_size(sc_default_pool_size)
            , restart_delay(sc_default_restart_delay)
            {
            }

            config(const std::string& h, const std::string& p,
                   const std::string& crt, const std::string& pk)
            : host(h)
            , port(p)
            , certificate(crt)
            , private_key(pk)
            , pool_size(sc_default_pool_size)
            , restart_delay(sc_default_restart_delay)
            {
            }

            config(const std::string& h, const std::string& p,
                   const std::string& p12_ck)
            : host(h)
            , port(p)
            , p12_cert_key(p12_ck)
            , pool_size(sc_default_pool_size)
            , restart_delay(sc_default_restart_delay)
            {
            }
            
            static config sandbox(const std::string& crt,
                                  const std::string& pk)
            {
                return config(
                    "feedback.sandbox.push.apple.com",
                    "2196",
                    crt,
                    pk
                );
            }
            
            static config production(const std::string& crt,
                                     const std::string& pk)
            {
                return config(
                    "feedback.push.apple.com",
                    "2196",
                    crt,
                    pk
                );
            }
            
            static config sandbox(const std::string& p12_ck)
            {
                return config(
                    "feedback.sandbox.push.apple.com",
                    "2196",
                    p12_ck
                );
            }
            
            static config production(const std::string& p12_ck)
            {
                return config(
                    "feedback.push.apple.com",
                    "2196",
                    p12_ck
                );
            }

        };
        
        apns_feedback(push_service& ps, const config& cfg);

        /// Start getting the feed
        void start();
        
        /// Stop fetching and processing the feed
        void stop();

    private:
        const std::string give_p12_pass(const std::string& pass);

        push_service&  push_service_;
        const std::string host_;
        const std::string port_;
        std::pair<std::string, std::string> cert_key_;
        callback_type  on_feedback_;
        const std::string ca_certificate_path_;

        push::detail::connection_pool<
            push::detail::apns_feedback_connection,
            push::detail::apns_request> pool_;
        boost::asio::ssl::context ssl_ctx_;
    };

} // namespace push

#endif // _APNS_PROVIDER_HPP_
