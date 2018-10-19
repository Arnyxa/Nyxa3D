#include "context.hpp"
#include "globals.hpp"
#include "logger.hpp"

int main()
{
    ppr::log->trace("Testing trace");
    ppr::log->debug("Testing debug");
    ppr::log->info("Testing info");
    ppr::log->warn("Testing warn");
    ppr::log->error("Testing error");
    ppr::log->critical("Testing crit");

    {
        ppr::context context;
        context.run();
    }

    return EXIT_SUCCESS;
}