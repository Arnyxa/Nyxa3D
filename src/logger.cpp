#include "logger.hpp"

ppr::logger::~logger()
{

}

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

ppr::logger::logger(log_level a_level /*= DEFAULT_LOG_LVL*/) 
    : m_logger(spdlog::stderr_color_st(DEFAULT_LOGGER_S))
{
    m_logger->set_level(convert_lvl(a_level));
}

