#pragma once

#include <iostream>
#include <string>
#include <typeinfo>

#include "operators.h"


#if 0
typeid(*this)
typeid(class)
#endif


struct E
{
    virtual ~E() { }
    virtual int eval() = 0;
};

struct P
{
    P(E& ee) : e(ee){ }
    E& e;
};

struct Num : E
{
    Num(int v) { value = v; }
    int value;
    int eval() { return value; }
};

struct Read : E
{
    Read() { }
    int eval() { return 0;}
};

struct Binop : E
{
    Binop(b_ops oper, E& left, E& right) : op(oper), l(left), r(right) { }
    b_ops op;
    E& l;
    E& r;
    int eval() {return 0; }
};

struct Unop : E
{
    Unop(u_ops oper, E& value) : op(oper), v(value) { }
    u_ops op;
    E& v;
    int eval() {return 0; }
};

struct Var : E
{
    Var(std::string varname) { name = varname; }
    std::string name;
    int eval() {return 0; }
};

struct Let : E
{
    Let(std::string varname, E& vexp, E& bexp) : name(varname), ve(vexp), be(bexp) { }
    std::string name;
    E& ve;
    E& be;
    int eval() { return 0; }
};

