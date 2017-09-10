#pragma once

#include <functional>
#include <string>

namespace push
{
    enum class LogLevel
    {
        ERROR,
        WARNING,
        INFO,
        DEBUG
    };

    typedef std::function<void(const std::string&, LogLevel level)> log_callback_type;
}

#define PUSH_LOG(msg, level) if (log_callback_) { log_callback_(msg, level); }
