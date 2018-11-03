#include "logger.hpp"
#include "globals.hpp"
#include "util.hpp"

namespace ppr
{
    const logger& logger::get_instance()
    {
        static logger s_logger;

        return s_logger;
    }

    const logger::spd_logger& logger::get()
    {
        return get_instance().m_logger;
    }

    auto logger::convert_lvl(log_level a_level)
    {
        return static_cast<spdlog::level::level_enum>(a_level);
    }

    logger::logger(log_level a_level)
        : m_logger(spdlog::stderr_color_st("ppr"))
    {
        m_logger->set_pattern("%v");
        m_logger->set_level(spdlog::level::trace);
        m_logger->trace("--- Pepper logging error codes ---");

        m_logger->set_pattern("%^[%L]%$ %v");
        m_logger->trace("TRACE");
        m_logger->info("INFO");
        m_logger->debug("DEBUG");
        m_logger->warn("WARN");
        m_logger->error("ERROR");
        m_logger->critical("CRITICAL\n"); 

        m_logger->set_pattern("[%M:%S:%e] [%n] %^[%L]%$ %v");
        m_logger->set_level(convert_lvl(a_level));
    }

}