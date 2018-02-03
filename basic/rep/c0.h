#pragma once

#include <map>
#include <string>
#include <vector>

#include "operators.h"
#include "x0s.h"

namespace c0
{

    struct E
    {
        virtual ~E() { }
        virtual std::vector<x0s::I*> select(x0s::Dst*) = 0;
    };

    struct Arg : E
    {
        virtual ~Arg() { }
        std::vector<x0s::I*> select(x0s::Dst*);
        virtual x0s::Arg* to_arg() = 0;
    };

    struct Var : Arg
    {
        Var(std::string varname) { name = varname; }
        std::string name;
        x0s::Arg* to_arg();
    };

    struct Num : Arg
    {
        Num(int v) { value = v; }
        int value;
        x0s::Arg* to_arg();
    };

    //stmt
    struct S
    {
        S(std::string s, E* ee) : v(s), e(ee) { }
        std::string v;
        E* e;
        std::vector<x0s::I*> select();
    };

    struct Read : E
    {
        Read() { }
        std::vector<x0s::I*> select(x0s::Dst*);
    };

    struct Binop : E
    {
        Binop(b_ops oper, Arg* left, Arg* right) : op(oper), l(left), r(right) { }
        b_ops op;
        Arg* l;
        Arg* r;
        std::vector<x0s::I*> select(x0s::Dst*);
    };

    struct Unop : E
    {
        Unop(u_ops oper, Arg* value) : op(oper), v(value) { }
        u_ops op;
        Arg* v;
        std::vector<x0s::I*> select(x0s::Dst*);
    };

    struct P
    {
        P(std::vector<std::string> v, std::vector<S> s, Arg* a) : vars(v), stmts(s), arg(a) { }
        x0s::P select();
        std::vector<std::string> vars;
        std::vector<S> stmts;
        Arg* arg; //ret
    };
}

