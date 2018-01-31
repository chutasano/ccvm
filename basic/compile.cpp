#include <iostream>
#include <string>
#include <typeinfo>

#include "rep/r0.h"

using namespace std;



bool compile(r0::P p)
{
    p.uniquify();
    // p->something else
    //
    return true;
}

int compile_run(r0::P p)
{
//    system("gcc file.s -o file")
//    TODO
    return 1;
}

bool test_compile(r0::P p, int expect)
{
    int actual = compile_run(p);
    return expect == actual;
}
