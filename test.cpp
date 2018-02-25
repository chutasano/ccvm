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

static bool both(r0::P p, int expect)
{
    bool woof = test_interp(p, expect);
    bool meow = test_compile(p, expect);
    if (!woof)
    {
        cerr << "Interpreter failed\n";
    }
    if (!meow)
    {
        cerr << "Compiler failed\n";
    }
    return woof && meow;
}

static r0::E* power(int exp)
{
    if (exp == 0)
    {
        return new r0::Num(1);
    }
    else
    {
        return new r0::Binop(B_PLUS, power(exp-1), power(exp-1));
    }
}

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
    //testfunc = both;

    cout << "Start Tests\n";

    // Useful expressions are pre-defined here for future use
    NUM(10);
    NUM(23);
    NNUM(1);


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
        VAR(x);
        LET(onevar, x, n10, vx);
        t(onevar, 10);
    }
    {
        VAR(x);
        VAR(y);
        PLUS(vx, vy);
        LET(twovaradd2, y, n23, bplusvx_vy);
        LET(twovaradd, x, n10, twovaradd2);
        t(twovaradd, 33);
    }

    ts("Shadowing and Uniquify");
    {
        VAR(x);
        LET(shadowb, x, n23, vx);
        LET(shadow, x, n10, shadowb);
        t(shadow, 23);
        LET(shadowmore, x, nn1, shadow);
        t(shadowmore, 23);
    }
    {
        VAR(x);
        LET(shadowb, x, n23, vx);
        LET(shadow, x, n10, shadowb);
        LET(shadowmore, x, nn1, shadow);
        tu(shadowmore, false);
    }
    {
        VAR(x);
        LET(shadowb, x, n23, vx);
        tu(shadowb, true);
    }
    const int64_t exponent = 13;
    ts("Power");
    {
        // god why doesn't C have int powers
        int64_t ans = 1;
        for (int i = 0; i < exponent; i++)
        {
            ans*=2;
        }
        r0::E* twopower = power(exponent);
        t(twopower, ans);
    }
}


