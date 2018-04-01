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

x0s::Arg* GlobalVar::to_arg()
{
    return new x0s::Global(this->name);
}

x0s::Arg* Num::to_arg()
{
    return new x0s::Con(this->value);
}

list<x0s::I*> Arg::select(x0s::Var* var)
{
    return { new x0s::ISrcDst(MOVQ, this->to_arg(), var) };
}

list<x0s::I*> Read::select(x0s::Var* var)
{
    return { new x0s::ICall("_lang_read_num", { }, var) };
}

list<x0s::I*> Binop::select(x0s::Var* var)
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

list<x0s::I*> Unop::select(x0s::Var* var)
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
    };
}

list<x0s::I*> FunCall::select(x0s::Var* var)
{
    list<x0s::Arg*> x0sargs;
    for (auto a : args)
    {
        x0sargs.push_back(a->to_arg());
    }
    return { new x0s::ICall(name, x0sargs, var) };
}

list<x0s::I*> Alloc::select(x0s::Var* var)
{
    int total_size = 8*(1+size);
    // TODO abstractify this a bit more to reuse code
    x0s::Reg* tmp = new x0s::Reg("rax");
    return { new x0s::ISrcDst(MOVQ, new x0s::Global("_LANG_HEAP_END"), tmp),
             new x0s::ISrcDst(SUBQ, new x0s::Con(total_size), tmp),
             new x0s::ISrcDst(CMPQ, new x0s::Reg("r15"), tmp),
             new x0s::IJmp(JG, var->var + "_call_collect_end"),
             new x0s::ICollect(),
             new x0s::ILabel(var->var + "_call_collect_end"),
             new x0s::ISrcDst(LEAQ, new x0s::Global(type2name(tag)),
                      new x0s::Deref(new x0s::Reg("r15"), 0)),
             new x0s::ISrcDst(MOVQ, new x0s::Reg("r15"), var),
             new x0s::ISrcDst(ADDQ, new x0s::Con(total_size), new x0s::Reg("r15")) };
}

list<x0s::I*> VecRef::select(x0s::Var* var)
{
    return { new x0s::ISrcDst(MOVQ, vec->to_arg(), new x0s::Reg("r8")),
             new x0s::ISrcDst(MOVQ, new x0s::Deref(new x0s::Reg("r8"), 8*(1+index)), var) };
}

list<x0s::I*> VecSet::select(x0s::Var* var)
{
    return { new x0s::ISrcDst(MOVQ, vec->to_arg(), new x0s::Reg("r8")),
             new x0s::ISrcDst(MOVQ, asg->to_arg(),
                      new x0s::Deref(new x0s::Reg("r8"), 8*(1+index))),
             new x0s::ISrcDst(MOVQ, new x0s::Con(TV_VOID), var) };
}

list<x0s::I*> S::select()
{
    return this->e->select(new x0s::Var(this->v));
}


list<x0s::I*> If::select()
{
    x0s::Dst* var = new x0s::Reg("rax"); // TODO?
    x0s::Var* tv = new x0s::Var(this->v);
    list<x0s::I*> l = {
        new x0s::ISrcDst(MOVQ, this->conde->l->to_arg(), var),
        new x0s::ISrcDst(CMPQ, this->conde->r->to_arg(), var) };
    string thenlabel = this->v + "_then";
    string donelabel = this->v + "_done";
    switch(this->conde->op)
    {
        case B_EQ:
            l.push_back(new x0s::IJmp(JE, thenlabel));
            break;
        case B_LT:
            l.push_back(new x0s::IJmp(JL, thenlabel));
            break;
        case B_GT:
            l.push_back(new x0s::IJmp(JG, thenlabel));
            break;
        case B_LE:
            l.push_back(new x0s::IJmp(JLE, thenlabel));
            break;
        case B_GE:
            l.push_back(new x0s::IJmp(JGE, thenlabel));
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
    elsei.push_back(new x0s::IJmp(JMP, donelabel));
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

x0s::F F::select()
{
    list<x0s::I*> instrs;
    for (auto s : this->stmts)
    {
        list<x0s::I*> is = s->select();
        instrs.splice(instrs.end(), is);
    }
    instrs.push_back(new x0s::IRet(this->arg->to_arg()));
    return x0s::F(name, instrs, vars, t);
}

x0s::P P::select()
{
    vector<x0s::F> x0sfs;
    for (F f : funcs)
    {
        x0sfs.push_back(f.select());
    }
    return x0s::P(x0sfs, to_run, heap_size);
}

