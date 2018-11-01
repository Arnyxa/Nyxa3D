#include "logger.hpp"
#include "globals.hpp"
#include "util.hpp"

#include <iostream>

namespace ppr
{
    const ppr::logger& ppr::logger::get_instance()
    {
        static logger s_logger;

        return s_logger;
    }

    const ppr::logger::spd_logger& ppr::logger::get()
    {
        return get_instance().m_logger;
    }

    auto ppr::logger::convert_lvl(log_level a_level)
    {
        return static_cast<spdlog::level::level_enum>(a_level);
    }

    ppr::logger::logger(log_level a_level)
        : m_logger(spdlog::stderr_color_st(to_lower_copy(PROJECT_TITLE)))
    {
#ifdef PPR_TRACE
        m_logger->set_level(spdlog::level::trace);

        m_logger->trace("Testing trace");
        m_logger->debug("Testing debug");
        m_logger->info("Testing info");
        m_logger->warn("Testing warn");
        m_logger->error("Testing error");
        m_logger->critical("Testing crit\n");
#endif

        m_logger->set_level(convert_lvl(a_level));
    }

}