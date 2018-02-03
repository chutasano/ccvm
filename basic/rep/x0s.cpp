#include <map>
#include <string>
#include <vector>

#include "asm.h"
#include "x0s.h"

using namespace std;
using namespace x0s;

int total_offset;

int get_offset(string v)
{
    static int worst = 8;
    static map<string, int> m;
    auto it = m.find(v);
    int offset;
    if (it != m.end())
    {
        offset = it->second;
    }
    else
    {
        offset = worst - 8;
        worst = offset;
        m[v] = worst;
    }
    return offset;
}

x0::P P::assign()
{
    std::vector<x0::I*> ins;
    total_offset = 8*this->vars.size();
    ins.push_back(new x0::TwoArg(SUBQ, new x0::Con(total_offset), new x0::Reg("rsp")));
    for (auto iptr : this->instr)
    {
        ins.push_back(iptr->assign());
    }
    auto it = ins.end() - 1;
    // evil hack assuming ret happens at the end, FIXME?
    ins.insert(it, new x0::TwoArg(ADDQ, new x0::Con(total_offset), new x0::Reg("rsp")));
    return x0::P(ins);
}

x0::Arg* Reg::assign()
{
    return new x0::Reg(this->name);
}

x0::Arg* Var::assign()
{
    return new x0::Mem("rsp", get_offset(this->var));
}

x0::Arg* Con::assign()
{
    return new x0::Con(this->val);
}

x0::I* NoArg::assign()
{
    return new x0::NoArg(this->instr);
}

x0::I* OneSrc::assign()
{
    return new x0::OneSrc(this->instr, this->src->assign());
}
x0::I* OneDst::assign()
{
    // evil hax
    x0::Dst* d = static_cast<x0::Dst*>(this->dst->assign());
    return new x0::OneDst(this->instr, d);
}
x0::I* TwoArg::assign()
{
    x0::Dst* d = static_cast<x0::Dst*>(this->dst->assign());
    return new x0::TwoArg(this->instr, this->src->assign(), d);
}

x0::I* Call::assign()
{
    return new x0::Call(this->label);
}

x0::I* Ret::assign()
{
    return new x0::Ret(this->arg->assign());
}

