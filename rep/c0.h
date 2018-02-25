#pragma once

#include <list>
#include <string>
#include <vector>

#include "operators.h"
#include "type.h"
#include "x0s.h"

namespace c0
{
    struct E
    {
        virtual ~E() { }
        virtual std::list<x0s::I*> select(x0s::Dst*) = 0;
    };

    struct Arg : E
    {
        virtual ~Arg() { }
        std::list<x0s::I*> select(x0s::Dst*);
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
        Num(int64_t v) { value = v; }
        int64_t value;
        x0s::Arg* to_arg();
    };

    // abstract statement either contains an if or a statement
    struct AS
    {
        virtual std::list<x0s::I*> select() = 0;
    };

    //stmt
    struct S : AS
    {
        S(std::string s, E* ee) : v(s), e(ee) { }
        std::string v;
        E* e;
        std::list<x0s::I*> select();
    };

    struct If : AS
    {
        If(E* ee, S thenstmt, S elsestmt) : conde(ee), thens(thenstmt), elses(elsestmt) { }
        E* conde;
        S thens;
        S elses;
        std::list<x0s::I*> select();
    };

    struct Read : E
    {
        Read() { }
        std::list<x0s::I*> select(x0s::Dst*);
    };

    struct Binop : E
    {
        Binop(b_ops oper, Arg* left, Arg* right) : op(oper), l(left), r(right) { }
        b_ops op;
        Arg* l;
        Arg* r;
        std::list<x0s::I*> select(x0s::Dst*);
    };

    struct Unop : E
    {
        Unop(u_ops oper, Arg* value) : op(oper), v(value) { }
        u_ops op;
        Arg* v;
        std::list<x0s::I*> select(x0s::Dst*);
    };

    struct P
    {
        P(std::vector<std::string> v, std::vector<AS*> s, Arg* a, type ty) : vars(v), stmts(s), arg(a), t(ty) { }
        std::vector<std::string> vars;
        std::vector<AS*> stmts;
        Arg* arg; //ret
        type t;
        x0s::P select();
    };
}

