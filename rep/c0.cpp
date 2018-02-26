#include <iostream>
#include <list>
#include <string>
#include <unordered_map>

#include "c0.h"

using namespace std;
using namespace c0;

static unordered_map<string, unsigned int> count;

x0s::Arg* Var::to_arg()
{
    return new x0s::Var(this->name);
}

x0s::Arg* Num::to_arg()
{
    return new x0s::Con(this->value);
}

list<x0s::I*> Arg::select(x0s::Dst* var)
{
    return { new x0s::ISrcDst(MOVQ, this->to_arg(), var) };
}

list<x0s::I*> Read::select(x0s::Dst* var)
{
    return { new x0s::ICall("_lang_read_num"),
             new x0s::ISrcDst(MOVQ, new x0s::Reg("rax"), var) };
}

list<x0s::I*> Binop::select(x0s::Dst* var)
{
    switch(this->op)
    {
        case B_PLUS:
            return { new x0s::ISrcDst(MOVQ, this->l->to_arg(), var),
                     new x0s::ISrcDst(ADDQ, this->r->to_arg(), var) };
        case B_EQ:
            return { new x0s::ISrcDst(MOVQ, this->l->to_arg(), var),
                     new x0s::ISrcDst(CMPQ, this->r->to_arg(), var),
                     new x0s::IDst(SETE, new x0s::Reg8("Al")),
                     new x0s::ISrcDst(MOVZBQ, new x0s::Reg8("Al"), var) };
        case B_LT:
            return { new x0s::ISrcDst(MOVQ, this->l->to_arg(), var),
                     new x0s::ISrcDst(CMPQ, this->r->to_arg(), var),
                     new x0s::IDst(SETL, new x0s::Reg8("Al")),
                     new x0s::ISrcDst(MOVZBQ, new x0s::Reg8("Al"), var) };
        case B_GT:
            return { new x0s::ISrcDst(MOVQ, this->l->to_arg(), var),
                     new x0s::ISrcDst(CMPQ, this->r->to_arg(), var),
                     new x0s::IDst(SETG, new x0s::Reg8("Al")),
                     new x0s::ISrcDst(MOVZBQ, new x0s::Reg8("Al"), var) };
        case B_LE:
            return { new x0s::ISrcDst(MOVQ, this->l->to_arg(), var),
                     new x0s::ISrcDst(CMPQ, this->r->to_arg(), var),
                     new x0s::IDst(SETLE, new x0s::Reg8("Al")),
                     new x0s::ISrcDst(MOVZBQ, new x0s::Reg8("Al"), var) };
        case B_GE:
            return { new x0s::ISrcDst(MOVQ, this->l->to_arg(), var),
                     new x0s::ISrcDst(CMPQ, this->r->to_arg(), var),
                     new x0s::IDst(SETGE, new x0s::Reg8("Al")),
                     new x0s::ISrcDst(MOVZBQ, new x0s::Reg8("Al"), var) };
        default:
            std::cout << "WARN: unknown binary operator: " << this->op << "\n";
            return { };
    }
}

list<x0s::I*> Unop::select(x0s::Dst* var)
{
    switch(this->op)
    {
        case U_NEG:
            return { new x0s::ISrcDst(MOVQ, this->v->to_arg(), var),
                     new x0s::IDst(NEGQ, var) };
        case U_NOT:
            return { new x0s::ISrcDst(MOVQ, this->v->to_arg(), var),
                     new x0s::ISrcDst(XORQ, new x0s::Con(1), var) };
        default:
            std::cout << "WARN: unknown unary operator: " << this->op << "\n";
            return { };
    }
}

list<x0s::I*> S::select()
{
    return this->e->select(new x0s::Var(this->v));
}

x0s::P P::select()
{
    list<x0s::I*> instrs;
    for (auto s : this->stmts)
    {
        list<x0s::I*> is = s->select();
        instrs.splice(instrs.end(), is);
    }
    instrs.push_back(new x0s::IRet(this->arg->to_arg()));
    return x0s::P(instrs, this->vars, this->t);
}

