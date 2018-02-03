#include <functional>
#include <iostream>
#include <utility>

#include "compile.h"
#include "interp.h"
#include "rep/r0.h"
#include "test.h"


// not really unique_ptr, but it effetively gives RAII for the testings
// implementation is very simple and is not meant to actually emulate a
// unique_ptr, I just ran out on ideas for good names
#define UPTR(type, name, ...)            \
    type name ## _LOCAL(__VA_ARGS__);    \
    type * name = & name ## _LOCAL 

// positive num is a bit easier. It will figure out the name by simply
// using the value of the initialized num
// NUM(1) -> n1
#define NUM(a) UPTR(r0::Num, n ## a, a)

// I had to separate out negatives because - signs cannot be used for var
// names. a has to be a positive number
// NNUM(1) -> nn1
#define NNUM(a) UPTR(r0::Num, nn ## a, -a)

// VAR(x) -> vx
#define VAR(name) UPTR(r0::Var, v ## name, #name)

// LET(name, var, vexp, bexp) -> name (nothing special or cool)
#define LET(name, var, ...) UPTR(r0::Let, name, #var, __VA_ARGS__)

// BINOP(name, operator, lexp, rexp) -> name (nothing special)
#define BINOP(...) UPTR(r0::Binop, __VA_ARGS__)

// UNOP
#define UNOP(...) UPTR(r0::Unop, __VA_ARGS__)

// operation specifics, might be too much work to maintain if we add more operators

#define PLUS(lexp, rexp) BINOP(bplus ## lexp ## _ ## rexp, B_PLUS, lexp, rexp)
#define NEG(exp) UNOP(uneg ## exp, U_NEG, exp)
    
    

using namespace std;

std::function<bool(r0::P, int)> testfunc;

void ts(string name)
{
    cout << endl << "Test suite: " << name << endl;
}

void t(r0::E* e, int expect)
{
    r0::P p(e);
    if (testfunc(p, expect))
    {
        cout << "  Test passed\n";
    }
    else
    {
        cout << "  Test failed!!!!!!!!!!!!!!!!1\n";
    }
}

// test for uniqueness and uniquify
void tu(r0::E* e, bool unique)
{
    r0::P p(e);
    if (p.is_unique() && unique)
    {
        cout << "  Test passed, both unique\n";
    }
    else if (p.is_unique() && !unique)
    {
        cout << "  Test failed!!! Expected not unique, somehow got unique\n";
    }
    else if (!p.is_unique() && !unique)
    {
        cout << "  Test passed, both not unique, attempting to uniquify\n";
        p.uniquify();
        if (p.is_unique())
        {
            cout << "    Uniquify success\n";
        }
        else
        {
            cout << "    Uniquify failed!!!!!!!!!!\n";
        }
    }
    else
    {
        cout << "  Test failed!!! Expected unique, somehow got not unique\n";
    }
}

void test_all()
{
    // TODO switch to test both once compiler is implemented
    //testfunc = test_interp;
    testfunc = test_compile;

    cout << "Start Tests\n";

    // Useful expressions are pre-defined here for future use
    NUM(10);
    NUM(23);
    NNUM(1);

    VAR(x);
    VAR(y);

    ts("Num");
    {
        t(n10, 10);
        t(nn1, -1);
    }

    ts("Addition operator");
    {
        PLUS(n10, n10);
        PLUS(nn1, n23);
        t(bplusn10_n10, 20);
        t(bplusnn1_n23, 22);
    }

    ts("Negation operator");
    {
        NEG(n10);
        NEG(nn1);
        t(unegn10, -10);
        t(unegnn1, 1);
    }

    ts("Variable lookup");
    {
        LET(onevar, x, n10, vx);
        t(onevar, 10);
        PLUS(vx, vy);
        LET(twovaradd2, y, n23, bplusvx_vy);
        LET(twovaradd, x, n10, twovaradd2);
        t(twovaradd, 33);
    }

    ts("Shadowing and Uniquify");
    {
        LET(shadowb, x, n23, vx);
        LET(shadow, x, n10, shadowb);
        t(shadow, 23);
        LET(shadowmore, x, nn1, shadow);
        t(shadowmore, 23);

        tu(shadowmore, false);
        tu(shadowb, true);
    }
}


