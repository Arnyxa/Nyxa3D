#include "Context.h"

#include <iostream>
#include <stdexcept>

// separate function for running the app to ensure 
// that the app object is destroyed before we exit
int RunApp();

int main()
{
	int mySuccess = RunApp();

#ifdef NXDEBUG
	std::cout << "Press ENTER to close console.";
	std::cin.get();
#endif

	return mySuccess;
}

int RunApp()
{
#ifdef NXDEBUG
	std::cout << "---------- DEBUG MODE ----------" << "\n" << std::endl;
#endif

	nx::Context myContext;

	try
	{
		myContext.Run();
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;

		std::cout << "Press ENTER to close console.";
		std::cin.get();

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}