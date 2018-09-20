#include "Context.h"
#include "DbgMsgr.h"

#include <iostream>
#include <stdexcept>

#define SUBSYSC // remove this if you're not on MSVC SUBSYSTEM/CONSOLE

// separate function for running the app to ensure 
// that the app object is destroyed before we exit
int RunApp();

int main()
{
	int mySuccess = RunApp();

#if defined(PPR_DEBUG) && defined(_MSC_VER) && !defined(SUBSYSC)
	std::cout << "Press ENTER to exit.";
	std::cin.get();
#endif

	return mySuccess;
}

int RunApp()
{
#if !defined(PPR_VERBOSE)
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

    #if defined(PPR_DEBUG) && defined(_MSC_VER) && !defined(SUBSYSC)
		std::cin.get();
	#endif

        // if we're not debugging, we still want to return the relevant error code
        return myError.GetCode();
	}

	return EXIT_SUCCESS;
}