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
    x0s::TwoArg* movq = new x0s::TwoArg(MOVQ,
            this->to_arg(),
            var);
    return { movq };
}

list<x0s::I*> Read::select(x0s::Dst* var)
{
    x0s::Call* callq = new x0s::Call("_lang_read_num");
    x0s::TwoArg* movq = new x0s::TwoArg(MOVQ,
            new x0s::Reg("rax"),
            var);
    return { callq, movq };
}

list<x0s::I*> Binop::select(x0s::Dst* var)
{
    switch(this->op)
    {
        case B_PLUS:
            return { new x0s::TwoArg(MOVQ, this->l->to_arg(), var),
                     new x0s::TwoArg(ADDQ, this->r->to_arg(), var) };
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
            return { new x0s::TwoArg(MOVQ, this->v->to_arg(), var),
                     new x0s::OneDst(NEGQ, var) };
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
    instrs.push_back(new x0s::Ret(this->arg->to_arg()));
    return x0s::P(instrs, this->vars, this->t);
}

