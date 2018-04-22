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
#define VAR_T(name, type) UPTR(r0::Var, v ## name, #name, type)

// LET(name, var, vexp, bexp) -> name (nothing special or cool)
#define LET(name, var, ...) UPTR(r0::Let, name, #var, __VA_ARGS__)

// BINOP(name, operator, lexp, rexp) -> name (nothing special)
#define BINOP(...) UPTR(r0::Binop, __VA_ARGS__)

// UNOP
#define UNOP(...) UPTR(r0::Unop, __VA_ARGS__)

#define IF(...) UPTR(r0::If, __VA_ARGS__)

#define LAMBDA(...) UPTR(r0::Lambda, __VA_ARGS__)

// operation specifics, might be too much work to maintain if we add more operators

#define PLUS(lexp, rexp) BINOP(bplus_ ## lexp ## _ ## rexp, B_PLUS, lexp, rexp)
#define PLUSN(name, lexp, rexp) BINOP(name, B_PLUS, lexp, rexp)
#define EQ(lexp, rexp) BINOP(beq_ ## lexp ## _ ## rexp, B_EQ, lexp, rexp)
#define LT(lexp, rexp) BINOP(blt_ ## lexp ## _ ## rexp, B_LT, lexp, rexp)
#define GT(lexp, rexp) BINOP(bgt_ ## lexp ## _ ## rexp, B_GT, lexp, rexp)
#define LE(lexp, rexp) BINOP(ble_ ## lexp ## _ ## rexp, B_LE, lexp, rexp)
#define GE(lexp, rexp) BINOP(bge_ ## lexp ## _ ## rexp, B_GE, lexp, rexp)

#define NEG(exp) UNOP(uneg_ ## exp, U_NEG, exp)
#define NOT(exp) UNOP(unot_ ## exp, U_NOT, exp)



using namespace std;

function<bool(r0::P, vec_t[])> testfunc;

static bool both(const r0::P &p, vec_t expect[])
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

static void t(r0::E* e, vec_t expect[], int heap_size = 2048)
{
    r0::P p(e, heap_size);
    p.desugar();
    if (testfunc(p, expect))
    {
        cout << "  Test passed\n";
    }
    else
    {
        cout << "  Test failed!!!!!!!!!!!!\n";
        fails++;
    }
}

static void t(r0::P &p, vec_t expect[])
{
    p.desugar();
    if (testfunc(p, expect))
    {
        cout << "  Test passed\n";
    }
    else
    {
        cout << "  Test failed!!!!!!!!!!!!\n";
        fails++;
    }
}
static void t(r0::E* e, int expect, int heap_size=2048)
{
    vec_t woof[] = { vec_t(TNUM, expect) };
    t(e, woof, heap_size);
}

static void t(r0::P &p, int expect)
{
    vec_t woof[] = { vec_t(TNUM, expect) };
    t(p, woof);
}

// test for uniqueness and uniquify
static void tu(r0::E* e, bool unique, int heap_size=2048)
{
    r0::P a(e, heap_size);
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
static void tt(r0::E* e, int expect, int heap_size=2048)
{
    r0::P a(e, heap_size);
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

void test_all(bool run_only_last = false)
{
    //testfunc = test_interp;
    testfunc = test_compile;
    //testfunc = both;

    cout << "Start Tests\n";

    // Useful expressions are pre-defined here for future use
    NUM(10); // n10
    NUM(23); // n23
    NNUM(1); // nn1
    VAR(x);  // vx
    VAR(y);  // vy
    BTRUE;   // bt
    BFALSE;  // bf

    if (!run_only_last)
    {
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

        ts("Lots of Vectors");
        {
            UPTR(r0::Vector, v1, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, v2, { n23, nn1, bt, bf} );
            UPTR(r0::Vector, v3, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, v4, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, v5, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, v6, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, v7, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, v8, { n10, nn1, bf} );
            UPTR(r0::Begin, b, {v1,v2,v3,v4,v5,v6,v7,v8, n10 });
            t(b, 10);
            UPTR(r0::Begin, vs, {v1,v2,v3,v4,v5,v6,v7,v8});
            vec_t vs_expect[] = 
            {
                vec_t(TVEC, 3),
                vec_t(TNUM, 10),
                vec_t(TNUM, -1),
                vec_t(TBOOL, TB_FALSE)
            };
            t(vs, vs_expect, 128);
        }

        ts("Lots of active vectors");
        {
            VAR(a);
            VAR(b);
            VAR(c);
            UPTR(r0::Vector, _v1, { n10, n23, nn1, bt, bf} );
            UPTR(r0::Vector, _v2, { n23, nn1, bt, bf} );
            UPTR(r0::Vector, _v3, { nn1, n23, n10, n10, n10, n10} );
            UPTR(r0::VectorRef, vr1, va, 0);
            UPTR(r0::VectorRef, vr2, vb, 0);
            UPTR(r0::VectorRef, vr3, vc, 0);
            UPTR(r0::VectorRef, vr4, va, 0);
            UPTR(r0::VectorRef, vr5, vb, 0);
            UPTR(r0::VectorRef, vr6, vc, 0);
            PLUS(vr1, vr2);
            PLUS(bplus_vr1_vr2, vr3);
            PLUS(vr4, vr5);
            PLUS(bplus_vr4_vr5, vr6);
            PLUS(bplus_bplus_vr1_vr2_vr3, bplus_bplus_vr4_vr5_vr6);
            LET(v3let, c, _v3, bplus_bplus_bplus_vr1_vr2_vr3_bplus_bplus_vr4_vr5_vr6 );
            LET(v2let, b, _v2, v3let);
            LET(v1let, a, _v1, v2let);
            t(v1let, 64, 128);
        }

    }

    ts("Simple Fibonacci w/ Recursion");
    {
        NUM(0);
        NUM(1);
        NNUM(2);
        r0::Var n = r0::Var("n", TNUM);
        auto nref = &n;
        EQ(nref, n0);
        EQ(nref, n1);
        PLUS(nref, nn2);
        PLUS(nref, nn1);
        UPTR(r0::Call, fibsub2, "simple_fib", { bplus_nref_nn2 });
        UPTR(r0::Call, fibsub1, "simple_fib", { bplus_nref_nn1 });
        PLUS(fibsub2, fibsub1);
        IF(fib_eq1, beq_nref_n1, n1, bplus_fibsub2_fibsub1);
        IF(fib_body, beq_nref_n0, n0, fib_eq1);
        r0::F fib = r0::F("simple_fib", {n}, TNUM, fib_body);
        NUM(10);
        UPTR (r0::Call, callfib, "simple_fib", { n10 });
        r0::F main = r0::F("main", { }, callfib);
        r0::P prog = r0::P({fib, main }, "main", 2048);
        t(prog, 55);
    }

    ts("Function with lots of args");
    {
        r0::Var n0 = r0::Var("n0", TNUM); r0::Var n1 = r0::Var("n1", TNUM);
        r0::Var n2 = r0::Var("n2", TNUM); r0::Var n3 = r0::Var("n3", TNUM);
        r0::Var n4 = r0::Var("n4", TNUM); r0::Var n5 = r0::Var("n5", TNUM);
        r0::Var n6 = r0::Var("n6", TNUM); r0::Var n7 = r0::Var("n7", TNUM);
        r0::Var n8 = r0::Var("n8", TNUM); r0::Var n9 = r0::Var("n9", TNUM);
        r0::Var n10 = r0::Var("n10", TNUM); r0::Var n11 = r0::Var("n11", TNUM);
        r0::Var n12 = r0::Var("n12", TNUM);
        auto rn0 = &n0; auto rn1 = &n1; auto rn2 = &n2; auto rn3 = &n3;
        auto rn4 = &n4; auto rn5 = &n5; auto rn6 = &n6; auto rn7 = &n7;
        auto rn8 = &n8; auto rn9 = &n9; auto rn10 = &n10; auto rn11 = &n11;
        auto rn12 = &n12;
        PLUSN(p01, rn0, rn1); PLUSN(p02, p01, rn2); PLUSN(p03, p02, rn3);
        PLUSN(p04, p03, rn4); PLUSN(p05, p04, rn5); PLUSN(p06, p05, rn6);
        PLUSN(p07, p06, rn7); PLUSN(p08, p07, rn8); PLUSN(p09, p08, rn9);
        PLUSN(p010, p09, rn10); PLUSN(p011, p010, rn11);  PLUSN(p012, p011, rn12);
        r0::F add_12_nums = r0::F("add_12_nums",
                {n0,n1,n2,n3,n4,n5,n6,n7,n8,n9,n10,n11,n12},
                TNUM, p012);
        NUM(20);
        UPTR(r0::Call, call_12_nums, "add_12_nums", {n20,n20,n20,n20,n20,n20,n20,n20,n20,n20,n20,n20, n20});
        r0::F main = r0::F("main", { }, call_12_nums);
        r0::P prog = r0::P({add_12_nums, main }, "main", 2048);
        t(prog, 20*13);
    }

    ts("Lambda (no closure)");
    {
        NUM(123); NUM(321); NUM(111);
        VAR(x1); VAR(x2); VAR(x3);
        PLUS(vx2, vx3);
        PLUSN(sum3, vx1, bplus_vx2_vx3);
        LAMBDA(add3num, {"x1","x2","x3"}, sum3);
        VAR(r0add3num);
        UPTR(r0::Call, calladd3, vr0add3num, {n123, n321, n111});
        LET(main_test, r0add3num, add3num, calladd3);
        t(main_test, 123+321+111);
    }

    cout << "Total tests failed: " << fails << endl;
}

