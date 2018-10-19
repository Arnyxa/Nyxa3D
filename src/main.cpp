#include "context.hpp"
#include "globals.hpp"

int main()
{
    {
        ppr::context context;
        context.run();
    }

    return EXIT_SUCCESS;
}