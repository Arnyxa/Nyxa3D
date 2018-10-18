#include "context.hpp"
#include "globals.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace krn
{
    namespace log = spdlog;
}

void test()
{
    auto console = spdlog::stdout_color_mt("penis");

    console->set_level(krn::log::level::trace);
    console->info("Penis {} {}", 69, "dicks");
    console->trace("Sex {} {}", "butt", 420);
    console->critical("oh no it's a big oopsie");
    console->debug("Just testing stuff");
    console->error("fucksy wucksy");
    console->warn("u better watch out");
}

int main()
{
    test();
    {
        ppr::context context;
        context.run();
    }

    return EXIT_SUCCESS;
}