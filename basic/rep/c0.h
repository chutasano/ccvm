#pragma once

#include <map>
#include <string>
#include <vector>

#include "operators.h"
#include "x0.h"

namespace c0
{

    struct E
    {
        virtual ~E() { }
        virtual std::vector<x0::I*> select() = 0;
    };

    struct Arg : E
    {
        std::vector<x0::I*> select();
    };

    struct Var : Arg
    {
        Var(std::string varname) { name = varname; }
        std::string name;
    };

    struct Num : Arg
    {
        Num(int v) { value = v; }
        int value;
        std::vector<x0::I*> select();
    };

    //stmt
    struct S
    {
        Var* v;
        E* e;
        std::vector<x0::I*> select();
    };

    struct Read : E
    {
        Read() { }
        std::vector<x0::I*> select();
    };

    struct Binop : E
    {
        Binop(b_ops oper, Arg* left, Arg* right) : op(oper), l(left), r(right) { }
        b_ops op;
        Arg* l;
        Arg* r;
        std::vector<x0::I*> select();
    };

    struct Unop : E
    {
        Unop(u_ops oper, Arg* value) : op(oper), v(value) { }
        u_ops op;
        Arg* v;
        std::vector<x0::I*> select();
    };

    struct P
    {
        P(std::vector<Var*> v, std::vector<S*> s, Arg* a) : vars(v), stmts(s), arg(a) { }
        std::vector<Var*> vars;
        std::vector<S*> stmts;
        Arg* arg; //ret
    };
}
