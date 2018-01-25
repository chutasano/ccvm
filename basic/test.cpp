#include <functional>
#include <iostream>

#include "compile.h"
#include "interp.h"
#include "r0.h"
#include "test.h"

using namespace std;

std::function<bool(P, int)> testfunc;

void t(E& e, int expect)
{
    P p = P(e);
    if(testfunc(p, expect))
    {
        cout << "Test passed\n";
    }
    else
    {
        cout << "Test failed\n";
    }
}

void test_all()
{
    testfunc = test_interp;
    Num n10 = Num(10);
    Num n23 = Num(23);
    Num nn1 = Num(-1);

    // Num
    t(n10, 10);
    t(nn1, -1);

    // Simple addition
    Binop b10p10 = Binop(B_PLUS, n10, n10);
    Binop bn1p23 = Binop(B_PLUS, nn1, n23);
    t(b10p10, 20);
    t(bn1p23, 22);

    // Simple negation
    Unop un10 = Unop(U_NEG, n10);
    Unop unn1 = Unop(U_NEG, nn1);
    t(un10, -10);
    t(unn1, 1);
}


