#include <iterator>
#include <list>
#include <map>
#include <string>
#include <typeinfo>
#include <vector>

#include "asm.h"
#include "graph.h"
#include "x0s.h"

using namespace std;
using namespace x0s;

#define _DEBUG

// FIXME global
map<string, unsigned int> vmap;

static const vector<string> regs
{
    "rbx",
    "r12",
    "r13",
    "r14",
    "r15"
};

const assign_mode a_mode = ASG_SMART;

x0::P P::assign()
{
    Graph::NodeList in;
    // generate nodes
    for (string s : vars)
    {
        in.add_node(s);
    }
    // get lifetime of all vars
    map<string, pair<int, int> > lifetime;
    int i = 1; // let i start at 1 to exploit the default constructor
               // of std::pair to check for initial existance
    for (auto iptr : this->instr)
    {
        const auto &slist = iptr->get_vars();
        for (string s : slist)
        {
            auto entry = lifetime[s];
            if (entry.first == 0) // must be new
            {
                lifetime[s] = make_pair(i, i);
            }
            else
            {
                lifetime[s] = make_pair(entry.first, i);
            }
        }
        i++;
    }
    //use lifetime to make edges
    for (auto it = lifetime.begin(); it != lifetime.end(); ++it)
    {
#ifdef DEBUG
        cout << it->first << " lives from " << it->second.first << " to " << it->second.second << endl;
#endif
        int min = it->second.first;
        int max = it->second.second;
        list<string> local_interf;
        for (auto it_search = next(it); it_search != lifetime.end(); ++it_search)
        {
            int lmin = it_search->second.first;
            int lmax = it_search->second.second;
#ifdef DEBUG
            cout << "Var: " << min << " to " << max << endl
                 << "  t: " << lmin << " to " << lmax << endl;
#endif
            if ((lmin <= min && lmax >= min) ||
                (lmin <= max && lmax >= max) ||
                (lmin >= min && lmax <= max))
            {
                local_interf.push_back(it_search->first);
            }
        }
        in.add_edges(it->first, local_interf);
    }

    in.assign(regs.size(), a_mode);
    vmap = in.get_mapping();
    unsigned int worst = 0;
    for (auto p : vmap)
    {
        if (p.second > worst)
        {
            worst = p.second;
        }
    }
    int total_offset = 8*(worst - regs.size() + 1);
    list<x0::I*> ins;
    ins.push_back(new x0::TwoArg(SUBQ, new x0::Con(total_offset), new x0::Reg("rsp")));
    for (auto iptr : this->instr)
    {
        // temp hack FIXME
        if (typeid(*iptr) == typeid(Ret))
        {
            auto r = static_cast<Ret*>(iptr);
            ins.push_back(new x0::TwoArg(MOVQ, r->arg->assign(), new x0::Reg("rax")));
            ins.push_back(new x0::TwoArg(ADDQ, new x0::Con(total_offset), new x0::Reg("rsp")));
            ins.push_back(new x0::Ret());
        }
        else
        {
            ins.push_back(iptr->assign());
        }
    }
    return x0::P(ins);
}

x0::Arg* Reg::assign()
{
    return new x0::Reg(this->name);
}

x0::Arg* Var::assign()
{
    string v = this->var;
    if (vmap[v] < regs.size())
    {
        return new x0::Reg(regs.at(vmap[v]));
    }
    else
    {
        return new x0::Mem("rsp", -8*(vmap[v]-regs.size()));
    }

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
    // TODO fix
    return new x0::Ret();
}

list<string> NoArg::get_vars()
{
    return { };
}

list<string> OneSrc::get_vars()
{
    if (typeid(*(this->src)) == typeid(Var))
    {
        return { static_cast<Var*>(this->src)->var };
    }
    else
    {
        return { };
    }
}

list<string> OneDst::get_vars()
{
    if (typeid(*(this->dst)) == typeid(Var))
    {
        return { static_cast<Var*>(this->dst)->var };
    }
    else
    {
        return { };
    }
}

list<string> TwoArg::get_vars()
{
    list<string> a;
    if (typeid(*(this->src)) == typeid(Var))
    {
        a.push_back(static_cast<Var*>(this->src)->var);
    }
    if (typeid(*(this->dst)) == typeid(Var))
    {
        a.push_back(static_cast<Var*>(this->dst)->var);
    }
    return a;
}

list<string> Call::get_vars()
{
    return { };
}

list<string> Ret::get_vars()
{
    if (typeid(*(this->arg)) == typeid(Var))
    {
        return { static_cast<Var*>(this->arg)->var };
    }
    else
    {
        return { };
    }
}

