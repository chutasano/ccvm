#include <iostream>
#include <string>
#include <typeinfo>

#include "r0.h"

using namespace std;

int eval(E e, map<string, int> vmap)
{
    auto type = typeid(e);
    if (type == typeid(Num))
    {
        return static_cast<Num>(e).value;
    }
    else if (type == typeid(Read))
    {
        int val;
        cin >> val;
        return val;
    }
    else if (type == typeid(Binop))
    {
        auto b = static_cast<Binop>(e);
        int l = eval(b.l, vmap);
        int r = eval(b.r, vmap);
        int result;
        switch b.op
        {
            case B_PLUS:
                result = lval + rval;
                break;
            default:
                std::cout << "WARN: unknown binary operator: " << op << "\n";
                break;
        }
        return result;
    }
    else if (type == typeid(Unop))
    {
        auto u = static_cast<Unop>(e);
        int val = eval(u.v, vmap);
        int result;
        switch b.op
        {
            case U_NEG:
                result = -val;
                break;
            default:
                std::cout << "WARN: unknown unary operator: " << op << "\n";
                break;
        }
        return result;
    }
    else if (type == typeid(Var))
    {
        auto v = static_cast<Var>(e);
        return vmap.at(v.name);
    }
    else if (type == typeid(Let))
    {
        auto l = static_cast<Let>(e);
        vmap[l.name] = eval(l.ve, vmap);
        eval(be, vmap);
    }
    else
    {
        cerr << "ERROR: expression type invalid?";
        exit (1);
    }
}

int interp(P p)
{
    E e = p.e;
    map<string, int> map;
    return eval(e, map) ;
}

bool compile(P p)
{
    uniquify(p);
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
