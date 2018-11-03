#include "context.hpp"

int main()
{
    {
        ppr::context context;
        context.run();
    }

    return EXIT_SUCCESS;
}