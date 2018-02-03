#include <iostream>
#include <string>
#include <vector>

#include "c0.h"

using namespace std;
using namespace c0;

static map<string, unsigned int> count;

x0s::Arg* Var::to_arg()
{
    return new x0s::Var(this->name);
}

x0s::Arg* Num::to_arg()
{
    return new x0s::Con(this->value);
}

vector<x0s::I*> Arg::select(x0s::Dst* var)
{
    x0s::TwoArg* movq = new x0s::TwoArg(MOVQ,
            this->to_arg(),
            var);
    return { movq };
}

vector<x0s::I*> Read::select(x0s::Dst* var)
{
    x0s::Call* callq = new x0s::Call("_lang_read");
    x0s::TwoArg* movq = new x0s::TwoArg(MOVQ,
            new x0s::Reg("rax"),
            var);
    return { callq, movq };
}

vector<x0s::I*> Binop::select(x0s::Dst* var)
{
    switch(this->op)
    {
        case B_PLUS:
            {
                x0s::TwoArg* movq = new x0s::TwoArg(MOVQ,
                        this->l->to_arg(),
                        var);
                x0s::TwoArg* addq = new x0s::TwoArg(ADDQ,
                        this->r->to_arg(),
                        var);
                return { movq, addq };
            }
        default:
            std::cout << "WARN: unknown binary operator: " << this->op << "\n";
            return { };
    }
}

vector<x0s::I*> Unop::select(x0s::Dst* var)
{
    switch(this->op)
    {
        case U_NEG:
            {
                x0s::TwoArg* movq = new x0s::TwoArg(MOVQ,
                        this->v->to_arg(),
                        var);
                x0s::OneDst* negq = new x0s::OneDst(NEGQ, var);
                return { movq, negq };
            }
        default:
            std::cout << "WARN: unknown unary operator: " << this->op << "\n";
            return { };
    }
}

vector<x0s::I*> S::select()
{
    return this->e->select(new x0s::Var(this->v));
}

x0s::P P::select()
{
    std::vector<x0s::I*> instrs;
    for (auto s : this->stmts)
    {
        auto is = s.select();
        instrs.insert(end(instrs), begin(is), end(is));
    }
    instrs.push_back(new x0s::Ret(this->arg->to_arg()));
    return x0s::P(instrs, this->vars);
}

