//
//  gcm.hpp
//  push_service
//
//  Created by Alexander Kremer on 06/07/2013.
//  Copyright (c) 2013 godexsoft. All rights reserved.
//

#ifndef _GCM_PROVIDER_HPP_
#define _GCM_PROVIDER_HPP_

#include <push_service/push_provider.hpp>
#include <push_service/detail/gcm_connection.hpp>
#include <push_service/detail/connection_pool.hpp>

#include <boost/function.hpp>
#include <string>

namespace push {
    
    class gcm;
    
    /**
     * @brief GCM message wrapper
     */
    class gcm_message : public push_message
    {
    public:
        friend class gcm;
        gcm_message(const uint32_t& ident);
        
        // properties
        std::string notification_key;
        std::string notification_key_name;
        std::string collapse_key;
        uint64_t time_to_live;
        bool delay_while_idle;
        std::string restricted_package_name;
        bool dry_run;
        
        void add(const std::string& k, const std::string& v);
        
        std::string to_json() const;
        
        void add_reg_id(const std::string& reg_id);
        void clear_reg_ids();
        
    private:
        
        const uint32_t ident_; // not really passed to GCM, used internally
        std::vector<std::string> registration_ids_;
        
        typedef std::map<std::string, std::string> custom_map_type;
        std::map<std::string, std::string> custom_;
    };
    
    /**
     * @brief GCM push provider
     */
    class gcm : public provider
    {
    public:        
        typedef push::detail::connection_pool<
            detail::gcm_connection, detail::gcm_request>
                ::callback_type
                    callback_type;

        static const char* key;
        
        gcm(push_service& ps,
            const std::string& project_id,
            const std::string& api_key,
            const log_callback_type& log_callback_ = log_callback_type(),
            const uint32_t& poolsize = 1,
            const boost::posix_time::time_duration restart_delay = boost::posix_time::seconds(1),
            const boost::posix_time::time_duration confirmation_delay = boost::posix_time::seconds(1),
            const callback_type& cb = callback_type());
        
        bool validate_device(const device& dev) const;

        uint32_t post(const device& dev, const std::string& payload,
                      const uint32_t expiry, const uint32_t& ident);

        uint32_t post(const device& dev, const gcm_message& msg,
                      const uint32_t expiry);

    private:
        const std::string api_key_;
        
        push::detail::connection_pool<
            push::detail::gcm_connection,
            push::detail::gcm_request> pool_;
        boost::asio::ssl::context ssl_ctx_;
        
        /// callback
        callback_type callback_;
    };
    
} // namespace push

#endif // _GCM_PROVIDER_HPP_
