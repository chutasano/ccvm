#include "rep/r0.h"
#include "test.h"
#include <cstring>

int main(int argc, char* argv[])
{
    bool last_test_only = false;
    for (int i=argc; i>1; i--)
    {
        if (!strcmp("--last_test_only", argv[i-1]))
        {
            last_test_only = true;
        }
    }
    test_all(last_test_only);
}
