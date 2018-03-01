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
    return { new x0s::ICall(CALLQ, "_lang_read_num"),
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


list<x0s::I*> If::select()
{
    x0s::Dst* var = new x0s::Reg("Rax"); // TODO?
    x0s::Var* tv = new x0s::Var(this->v);
    list<x0s::I*> l = {
        new x0s::ISrcDst(MOVQ, this->conde->l->to_arg(), var),
        new x0s::ISrcDst(CMPQ, this->conde->r->to_arg(), var) };
    string thenlabel = this->v + "_then";
    string donelabel = this->v + "_done";
    switch(this->conde->op)
    {
        case B_EQ:
            l.push_back(new x0s::ICall(JE, thenlabel));
            break;
        case B_LT:
            l.push_back(new x0s::ICall(JL, thenlabel));
            break;
        case B_GT:
            l.push_back(new x0s::ICall(JG, thenlabel));
            break;
        case B_LE:
            l.push_back(new x0s::ICall(JLE, thenlabel));
            break;
        case B_GE:
            l.push_back(new x0s::ICall(JGE, thenlabel));
            break;
        default:
            std::cout << "\tc0If: WARN: unknown binary operator: " << this->conde->op << "\n";
            return { };
    }
    list<x0s::I*> elsei;
    for (auto s : elses)
    {
        list<x0s::I*> is = s->select();
        elsei.splice(elsei.end(), is);
    }
    elsei.push_back(new x0s::ISrcDst(MOVQ, elsev->to_arg(), tv));
    elsei.push_back(new x0s::ICall(CALLQ, donelabel));
    elsei.push_back(new x0s::ILabel(thenlabel));

    list<x0s::I*> theni;
    for (auto s : thens)
    {
        list<x0s::I*> is = s->select();
        theni.splice(theni.end(), is);
    }
    theni.push_back(new x0s::ISrcDst(MOVQ, thenv->to_arg(), tv));
    theni.push_back(new x0s::ILabel(donelabel));
    return { new x0s::IIf(l, theni, elsei) };
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

