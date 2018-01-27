#include <iostream>
#include <string>
#include <typeinfo>

#include "r0.h"

using namespace std;

void uniquify(P p)
{
}

bool compile(P p)
{
    uniquify(p);
    return true;
}

int compile_run(P p)
{
//    system("gcc file.s -o file")
//    TODO
    return 1;
}

bool test_compile(P p, int expect)
{
    int actual = compile_run(p);
    return expect == actual;
}
