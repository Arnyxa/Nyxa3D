#include "Context.h"
#include "DbgMsgr.h"

#include "Globals.h"

#include <iostream>

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

}

// separate function for running the app to ensure 
// that the app object is destroyed before we exit
int RunApp();

int main()
{
    test();
	int mySuccess = RunApp();

	return mySuccess;
}

int RunApp()
{
#if defined(PPR_DEBUG) && !defined(PPR_VERBOSE)
	ppr::DbgPrint("---------- DEBUG MODE ----------\n\n");
#elif defined(PPR_VERBOSE)
	ppr::VerbosePrint("---------- DEEP DEBUG ----------\n\n");
#endif

	ppr::Context myContext;

	try
	{
		myContext.Run();
	}
	catch (const ppr::Error& myError)
	{
        // create a separator that matches our error's length
        std::string separator("\n");

        for (unsigned i = 0; i < myError.What().size(); ++i)
            separator.push_back('-');
        separator.push_back('\n');

        std::cerr << separator << myError.What() << separator << "\n";

        // if we're not debugging, we still want to return the relevant error code
        return myError.GetCode();
	}

	return EXIT_SUCCESS;
}