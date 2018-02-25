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

#define BTRUE UPTR(r0::Bool, bt, r0::TB_TRUE)
#define BFALSE UPTR(r0::Bool, bf, r0::TB_FALSE)

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

function<bool(r0::P, int)> testfunc;

static bool both(const r0::P &p, int expect)
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

// chain of lets to stress test register allocation
static r0::E* letchain(int chaincount)
{
    if (chaincount == 0)
    {
        return new r0::Num(0);
    }
    else
    {
        return new r0::Let("chaincount" + to_string(chaincount), 
                           new r0::Num(0),
                           new r0::Binop(B_PLUS, new r0::Var("chaincount" + to_string(chaincount)),
                               letchain(chaincount-1)));
    }
}

static void ts(string name)
{
    cout << endl << "Test suite: " << name << endl;
}

static void t(r0::E* e, int expect)
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
static void tu(r0::E* e, bool unique)
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

// test for typechecker
static void tt(r0::E* e, type expect)
{
    r0::P p(e);
    type ty = p.type_check();
    if (ty == expect)
    {
        cout << "  Test passed: type check\n";
    }
    else
    {
        cout << "  Test failed: type check, expected "
             << expect << " got " << ty << endl;
    }
}

void test_all()
{
    // TODO switch to test both once compiler is implemented
    //testfunc = test_interp;
    //testfunc = test_compile;
    testfunc = both;

    cout << "Start Tests\n";

    // Useful expressions are pre-defined here for future use
    NUM(10); // n10
    NUM(23); // n23
    NNUM(1); // nn1
    VAR(x);  // vx
    VAR(y);  // vy
    BTRUE;   // bt
    BFALSE;  // bf

    ts("Num");
    {
        t(n10, 10);
        tt(n10, TNUM);
        t(nn1, -1);
        tt(nn1, TNUM);
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
        tt(twopower, TNUM);
    }

    const int chainc = 129;
    ts("Let Chain");
    {
        r0::E* lets = letchain(chainc);
        t(lets, 0);
        tt(lets, TNUM);
    }

    ts("Bool");
    {
        t(bt, r0::TB_TRUE);
        tt(bt, TBOOL);
        t(bf, r0::TB_FALSE);
        tt(bf, TBOOL);
    }
}


