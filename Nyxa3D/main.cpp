#include "Context.h"
#include "DbgMsgr.h"

#include <iostream>
#include <stdexcept>

// separate function for running the app to ensure 
// that the app object is destroyed before we exit
int RunApp();

int main()
{
	int mySuccess = RunApp();

	return mySuccess;
}

int RunApp()
{
#if !defined(NXDEEP)
	nx::DbgPrint("---------- DEBUG MODE ----------\n\n");
#elif defined(NXDEEP)
	nx::DeepPrint("---------- DEEP DEBUG ----------\n\n");
#endif

	nx::Context myContext;

	try
	{
		myContext.Run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}