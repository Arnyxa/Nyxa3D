#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>

#include <string>

namespace ppr
{
    enum class log_level : unsigned
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERR,
        CRITICAL,
        OFF
    };

#if defined(PPR_VERBOSE)
    constexpr log_level DEFAULT_LOG_LVL = log_level::TRACE;
    constexpr const char* DEFAULT_LOGGER_S = "pepper-verbose";
#elif defined(PPR_DEBUG)
    constexpr log_level DEFAULT_LOG_LVL = log_level::DEBUG;
    constexpr const char* DEFAULT_LOGGER_S = "pepper-debug";
#else
    constexpr log_level DEFAULT_LOG_LVL = log_level::OFF;
    constexpr const char* DEFAULT_LOGGER_S = "pepper-release";
#endif

    class logger
    {
    public:
        using spd_logger = std::shared_ptr<spdlog::logger>;

    public:
        ~logger();

        static const logger& get_instance();
        static const spd_logger& get();

    private:
        auto convert_lvl(log_level a_level);
        logger(log_level a_level = DEFAULT_LOG_LVL);

    private:
        spd_logger m_logger;
    };

#define log logger::get()
}