#include <iterator>
#include <list>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "asm.h"
#include "graph.h"
#include "type.h"
#include "x0s.h"

using namespace std;
using namespace x0s;

//#define DEBUG
//#define DEBUG_VERB

// FIXME global
unordered_map<string, unsigned int> vmap;

static const vector<string> regs
{
    "rbx",
    "r12",
    "r13",
    "r14",
};

const assign_mode a_mode = ASG_SMART;

x0::P P::assign()
{
    Graph::NodeList in;
    // generate nodes
    for (auto s : vars)
    {
        in.add_node(s.first);
    }
    // get lifetime of all vars
    unordered_map<string, pair<int, int> > lifetime;
        
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
        int min = it->second.first;
        int max = it->second.second;
        list<string> local_interf;
        for (auto it_search = next(it); it_search != lifetime.end(); ++it_search)
        {
            int lmin = it_search->second.first;
            int lmax = it_search->second.second;
#ifdef DEBUG_VERB
            cout << endl
                 << it->first << ": " << min << " to " << max << endl
                 << it_search->first << ": " << lmin << " to " << lmax << endl;
#endif
            if ((lmin <= min && lmax >= min) ||
                (lmin <= max && lmax >= max) ||
                (lmin >= min && lmax <= max))
            {
                local_interf.push_back(it_search->first);
#ifdef DEBUG_VERB
                cout << "Interferes!\n";
#endif
            }
        }
        in.add_edges(it->first, local_interf);
    }

    in.assign(regs.size(), a_mode);
    vmap = in.get_mapping();
#ifdef DEBUG
    for (auto p : vmap)
    {
        cout << p.first << " : " << p.second << endl;
    }
#endif
    unsigned int worst = 0;
    for (auto p : vmap)
    {
        if (p.second > worst)
        {
            worst = p.second;
        }
    }
    list<x0::I*> ins;
    ins.push_back(new x0::ILabel("main"));
    int total_offset;
    bool need_stack = worst >= regs.size();
    ins.push_back(new x0::ICall("_lang_debug"));
    if (need_stack)
    {
        total_offset = 8*(worst - regs.size() + 1);
        ins.push_back(new x0::ISrcDst(SUBQ, new x0::Con(total_offset), new x0::Reg("rsp")));
    }
    for (auto iptr : this->instr)
    {
        // temp hack FIXME
        if (typeid(*iptr) == typeid(IRet))
        {
            auto r = static_cast<IRet*>(iptr);
            ins.push_back(new x0::ISrcDst(MOVQ, r->arg->assign(), new x0::Reg("rax")));
            if (need_stack)
            {
                ins.push_back(new x0::ISrcDst(ADDQ, new x0::Con(total_offset), new x0::Reg("rsp")));
            }
            // can't use map to get type because simple programs may optimize
            // the ret part such that it's returning a non-variable (ie: constant)
            ins.push_back(new x0::IRet(this->t));
        }
        else
        {
            ins.splice(ins.end(), iptr->assign());
        }
    }
    list<x0::Tag> tags;
    tags.push_back(x0::Tag(type2name(TNUM), TNUM));
    tags.push_back(x0::Tag(type2name(TBOOL), TBOOL));
    tags.push_back(x0::Tag(type2name(TVOID), TVOID));
    tags.push_back(x0::Tag(type2name(TVEC), TVEC));
    // type -> list of types (for vectors)
    vec_type.erase(vec_type.begin());
    for (auto vpair : vec_type)
    {
        auto v = vpair.second;
        v.insert(v.begin(), v.size());
        tags.push_back(x0::Tag(type2name(vpair.first), v));
    }
    return x0::P(ins, tags);
}

x0::Arg* Reg::assign()
{
    return new x0::Reg(this->name);
}

x0::Arg* Reg8::assign()
{
    return new x0::Reg8(this->name);
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

x0::Arg* Deref::assign()
{
    return new x0::Mem(reg->name, offset);
}

x0::Arg* Global::assign()
{
    return new x0::Global(name) ;
}

x0::Arg* Con::assign()
{
    return new x0::Con(this->val);
}

list<x0::I*> INoArg::assign()
{
    return { new x0::INoArg(this->instr) };
}

list<x0::I*> ISrc::assign()
{
    return { new x0::ISrc(this->instr, this->src->assign())};
}
list<x0::I*> IDst::assign()
{
    x0::Dst* d = static_cast<x0::Dst*>(this->dst->assign());
    return { new x0::IDst(this->instr, d) };
}
list<x0::I*> ISrcDst::assign()
{
    x0::Dst* d = static_cast<x0::Dst*>(this->dst->assign());
    return { new x0::ISrcDst(this->instr, this->src->assign(), d) };
}
list<x0::I*> ISrcSrc::assign()
{
    return { new x0::ISrcSrc(this->instr, this->src->assign(), this->src2->assign()) };
}

list<x0::I*> IIf::assign()
{
    list<x0::I*> ins;
    for (I* i : ifi)
    {
        ins.splice(ins.end(), i->assign());
    }
    for (I* i : elsei)
    {
        ins.splice(ins.end(), i->assign());
    }
    for (I* i : theni)
    {
        ins.splice(ins.end(), i->assign());
    }
    return ins;
}

list<x0::I*> IJmp::assign()
{
    return { new x0::IJmp(this->instr, this->label) };
}

list<x0::I*> ICall::assign()
{
    static const list<string> available_regs = 
    {
        "rdi", "rsi", "rdx", "rcx", "r8", "r9"
    };
    if (args.size() > 6)
    {
        cerr << "ERROR: more than 6 args for function not supported atm.\n";
        exit(1);
    }
    list<x0::I*> a;
    for(auto it_pair = make_pair(args.begin(), available_regs.begin());
            it_pair.first != args.end();
            ++it_pair.first, ++it_pair.second)
    {
        a.push_back(new x0::ISrcDst(MOVQ, (*it_pair.first)->assign(), new x0::Reg(*it_pair.second)));
    }
    a.push_back(new x0::ICall(this->label));
    a.push_back(new x0::ISrcDst(MOVQ, new x0::Reg("rax"), static_cast<x0::Dst*>(dst->assign())));
    return a;
}

list<x0::I*> ILabel::assign()
{
    return { new x0::ILabel(this->name) };
}

list<x0::I*> IRet::assign()
{
    // TODO fix
    return { new x0::IRet(TBOOL) };
}

list<string> INoArg::get_vars()
{
    return { };
}

list<string> ISrc::get_vars()
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

list<string> IDst::get_vars()
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

list<string> ISrcDst::get_vars()
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

list<string> ISrcSrc::get_vars()
{
    list<string> a;
    if (typeid(*(this->src)) == typeid(Var))
    {
        a.push_back(static_cast<Var*>(this->src)->var);
    }
    if (typeid(*(this->src2)) == typeid(Var))
    {
        a.push_back(static_cast<Var*>(this->src2)->var);
    }
    return a;
}

list<string> IIf::get_vars()
{
    list<string> a;
    for (auto i : ifi)
    {
        a.splice(a.end(), i->get_vars());
    }
    for (auto i: elsei)
    {
        a.splice(a.end(), i->get_vars());
    }
    for (auto i: theni)
    {
        a.splice(a.end(), i->get_vars());
    }
    return a;
}

list<string> ICall::get_vars()
{
    if (typeid(dst) == typeid(Var*))
    {
        return { static_cast<Var*>(dst)->var };
    }
    else
    {
        return { };
    }
}

list<string> IJmp::get_vars()
{
    return { };
}

list<string> ILabel::get_vars()
{
    return { };
}

list<string> IRet::get_vars()
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

