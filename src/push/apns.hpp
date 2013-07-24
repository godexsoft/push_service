//
//  apns.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _APNS_PROVIDER_HPP_
#define _APNS_PROVIDER_HPP_

#include <push/push_provider.hpp>

#include <push/detail/apns_connection.hpp>
#include <push/detail/apns_feedback_connection.hpp>
#include <push/detail/connection_pool.hpp>

#include <boost/function.hpp>
#include <string>

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
        uint16_t badge;
        std::string sound;
        std::string action_loc_key;
        std::string loc_key;
        std::vector<std::string> loc_args;
        std::string launch_image;
        
        // TODO: add support for arrays and custom hierarchies.
        // this can be done if we implement add(ptree tree)
        // and a wrapper: add(vector<> arr)
        void add(const std::string& k, const std::string& v);
        
        std::string to_json() const;        
        
    private:
        typedef std::map<std::string, std::string> custom_map_type;
        std::map<std::string, std::string> custom_;
    };
    
    /**
     * @brief APNS push provider
     */
    class apns : public provider
    {
    public:
        
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
            std::string     certificate;
            std::string     private_key;
            uint16_t        pool_size;
            callback_type   callback;
            
            config(const std::string& h, const std::string& p,
                   const std::string& crt, const std::string& pk)
            : host(h)
            , port(p)
            , certificate(crt)
            , private_key(pk)
            , pool_size(4)
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
        };        
        
        static const char* key;
        
        apns(push_service& ps, const config& cfg);
        
        bool validate_device(const device& dev) const;
        
        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t ident);
        
        uint32_t post(const device& dev, const push_message& msg,
                      const uint32_t expiry, const uint32_t ident);

    private:
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
        typedef push::detail
            ::apns_feedback_connection::callback_type
                callback_type;

        /**
         * @brief configuration helper
         */
        struct config
        {
            std::string     host;
            std::string     port;
            std::string     certificate;
            std::string     private_key;
            callback_type   callback;
            
            config(const std::string& h, const std::string& p,
                   const std::string& crt, const std::string& pk)
            : host(h)
            , port(p)
            , certificate(crt)
            , private_key(pk)
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
        };
        
        apns_feedback(push_service& ps, const config& cfg);

        /// Start getting the feed
        void start();
        
        /// Stop fetching and processing the feed
        void stop();
        
    private:
        push_service&  push_service_;
        const std::string host_;
        const std::string port_;
        callback_type  on_feedback_;
        
        boost::asio::ssl::context ssl_ctx_;
        boost::shared_ptr<push::detail::apns_feedback_connection> connection_;
    };
    
} // namespace push

#endif // _APNS_PROVIDER_HPP_
