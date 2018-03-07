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
        virtual std::list<x0s::I*> select(x0s::Var*) = 0;
    };

    struct Arg : E
    {
        virtual ~Arg() { }
        std::list<x0s::I*> select(x0s::Var*);
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


    struct Read : E
    {
        Read() { }
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct Binop : E
    {
        Binop(b_ops oper, Arg* left, Arg* right) : op(oper), l(left), r(right) { }
        b_ops op;
        Arg* l;
        Arg* r;
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct Unop : E
    {
        Unop(u_ops oper, Arg* value) : op(oper), v(value) { }
        u_ops op;
        Arg* v;
        std::list<x0s::I*> select(x0s::Var*);
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
        If(std::string v, Binop* bo, Arg* thenvar, std::vector<AS*> thenstmts, Arg* elsevar, std::vector<AS*> elsestmts) :
            v(v), conde(bo), thens(thenstmts), elses(elsestmts), thenv(thenvar), elsev(elsevar) { }
        std::string v;
        Binop* conde;
        std::vector<AS*> thens;
        std::vector<AS*> elses;
        Arg* thenv;
        Arg* elsev;
        std::list<x0s::I*> select();
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

