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

#define BTRUE UPTR(r0::Bool, bt, TB_TRUE)
#define BFALSE UPTR(r0::Bool, bf, TB_FALSE)

// VAR(x) -> vx
#define VAR(name) UPTR(r0::Var, v ## name, #name)

// LET(name, var, vexp, bexp) -> name (nothing special or cool)
#define LET(name, var, ...) UPTR(r0::Let, name, #var, __VA_ARGS__)

// BINOP(name, operator, lexp, rexp) -> name (nothing special)
#define BINOP(...) UPTR(r0::Binop, __VA_ARGS__)

// UNOP
#define UNOP(...) UPTR(r0::Unop, __VA_ARGS__)

// UNOP
#define IF(...) UPTR(r0::If, __VA_ARGS__)

// operation specifics, might be too much work to maintain if we add more operators

#define PLUS(lexp, rexp) BINOP(bplus_ ## lexp ## _ ## rexp, B_PLUS, lexp, rexp)
#define EQ(lexp, rexp) BINOP(beq_ ## lexp ## _ ## rexp, B_EQ, lexp, rexp)
#define LT(lexp, rexp) BINOP(blt_ ## lexp ## _ ## rexp, B_LT, lexp, rexp)
#define GT(lexp, rexp) BINOP(bgt_ ## lexp ## _ ## rexp, B_GT, lexp, rexp)
#define LE(lexp, rexp) BINOP(ble_ ## lexp ## _ ## rexp, B_LE, lexp, rexp)
#define GE(lexp, rexp) BINOP(bge_ ## lexp ## _ ## rexp, B_GE, lexp, rexp)

#define NEG(exp) UNOP(uneg_ ## exp, U_NEG, exp)
#define NOT(exp) UNOP(unot_ ## exp, U_NOT, exp)
    
    

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

static int fails = 0;

static void t(r0::E* e, int expect)
{
    r0::P p(e);
    p.desugar();
    if (testfunc(p, expect))
    {
        cout << "  Test passed\n";
    }
    else
    {
        cout << "  Test failed!!!!!!!!!!!!!!!!1\n";
        fails++;
    }
}

// test for uniqueness and uniquify
static void tu(r0::E* e, bool unique)
{
    r0::P a(e);
    r0::P p(a); 
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
static void tt(r0::E* e, int expect)
{
    r0::P a(e);
    r0::P p(a); // copy P because typecheck modifies type field
    p.type_check();
    int ty = p.t;
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
        t(bplus_n10_n10, 20);
        t(bplus_nn1_n23, 22);
    }

    ts("Negation operator");
    {
        NEG(n10);
        NEG(nn1);
        t(uneg_n10, -10);
        t(uneg_nn1, 1);
    }

    ts("Variable lookup");
    {
        LET(onevar, x, n10, vx);
        t(onevar, 10);
        PLUS(vx, vy);
        LET(twovaradd2, y, n23, bplus_vx_vy);
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

    const int64_t exponent = 10;
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
        t(bt, TB_TRUE);
        t(bf, TB_FALSE);
        tt(bt, TBOOL);
        tt(bf, TBOOL);
    }

    ts("Not");
    {
        NOT(bt);
        NOT(bf);
        NOT(unot_bt);
        NOT(unot_bf);
        t(unot_bt, TB_FALSE);
        t(unot_bf, TB_TRUE);
        t(unot_unot_bt, TB_TRUE);
        t(unot_unot_bf, TB_FALSE);
        tt(unot_bt, TBOOL);
        tt(unot_bf, TBOOL);
        tt(unot_unot_bt, TBOOL);
        tt(unot_unot_bf, TBOOL);
    }
    ts("Equal");
    {
        EQ(n10, nn1);
        EQ(n10, n10);
        t(beq_n10_nn1, TB_FALSE);
        t(beq_n10_n10, TB_TRUE);
        tt(beq_n10_nn1, TBOOL);
        tt(beq_n10_n10, TBOOL);
    }

    ts("LT");
    {
        LT(n10, nn1); // 10 < -1, false
        LT(nn1, n10); // -1 < 10, true
        LT(n23, n23); // 23 < 23, false
        t(blt_n10_nn1, TB_FALSE);
        t(blt_nn1_n10, TB_TRUE);
        t(blt_n23_n23, TB_FALSE);
        tt(blt_n10_nn1, TBOOL);
        tt(blt_nn1_n10, TBOOL);
        tt(blt_n23_n23, TBOOL);
    }

    ts("GT");
    {
        GT(n10, nn1); // 10 > -1, true
        GT(nn1, n10); // -1 > 10, false
        GT(n23, n23); // 23 > 23, false
        t(bgt_n10_nn1, TB_TRUE);
        t(bgt_nn1_n10, TB_FALSE);
        t(bgt_n23_n23, TB_FALSE);
        tt(bgt_n10_nn1, TBOOL);
        tt(bgt_nn1_n10, TBOOL);
        tt(bgt_n23_n23, TBOOL);
    }

    ts("LE");
    {
        LE(n10, nn1); // 10 <= -1, false
        LE(nn1, n10); // -1 <= 10, true
        LE(n23, n23); // 23 <= 23, true
        t(ble_n10_nn1, TB_FALSE);
        t(ble_nn1_n10, TB_TRUE);
        t(ble_n23_n23, TB_TRUE);
        tt(ble_n10_nn1, TBOOL);
        tt(ble_nn1_n10, TBOOL);
        tt(ble_n23_n23, TBOOL);
    }

    ts("GE");
    {
        GE(n10, nn1); // 10 >= -1, true
        GE(nn1, n10); // -1 >= 10, false
        GE(n23, n23); // 23 >= 23, true
        t(bge_n10_nn1, TB_TRUE);
        t(bge_nn1_n10, TB_FALSE);
        t(bge_n23_n23, TB_TRUE);
        tt(bge_n10_nn1, TBOOL);
        tt(bge_nn1_n10, TBOOL);
        tt(bge_n23_n23, TBOOL);
    }

    ts("If simple");
    {
        IF(if10, bt, n10, nn1);
        IF(ifnn1, bf, n10, nn1);
        t(if10, 10);
        t(ifnn1, -1);
    }

    ts("If complicated");
    {
        GE(n10, nn1); // true
        GE(nn1, n10); // false
        IF(if10, bge_n10_nn1, n10, nn1); // n10
        IF(ifnn1, bge_nn1_n10, n10, nn1); // nn1
        t(if10, 10);
        t(ifnn1, -1);

    }

    ts("Begin");
    {
        NUM(123);
        NUM(234);
        NUM(345);
        NUM(456);
        UPTR(r0::Begin, beg, {n123, n234, n345, n456});
        t(beg, 456);
    }

    ts("Vector");
    {
        UPTR(r0::Vector, v, { n10, n23, nn1, bt, bf} );
        UPTR(r0::VectorRef, vref0, v, 0);
        UPTR(r0::VectorRef, vref4, v, 4);
        // todo accept vector as return value
        t(vref0, 10);
        t(vref4, TB_FALSE);
        tt(v, TVEC+1); // maybe bad test
        tt(vref0, TNUM);
        tt(vref4, TBOOL);
        UPTR(r0::VectorSet, vset0, v, 0, n23);
        UPTR(r0::VectorSet, vset0_fail, v, 0, bt);
        UPTR(r0::VectorSet, vset4, v, 4, bt);
        t(vset0, TV_VOID);
        tt(vset0, TVOID);
        tt(vset0_fail, TERROR);
        tt(vset4, TVOID);
    }

    cout << "Total tests failed: " << fails << endl;
}

