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
//#define DEBUG_BUILD
//#define DEBUG_VERB


static const vector<string> regs
{
    "rbx",
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
        in.add_node(s.first, (s.second > TVEC) ? ROOTSTACK : STACK);
    }
    // get lifetime of all vars
    unordered_map<string, pair<int, int> > lifetime;
        
    int i = 1; // let i start at 1 to exploit the default constructor
               // of std::pair to check for initial existance
    vector<pair<ICollect*, int> > collects;
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
        if (typeid(*iptr) == typeid(ICollect))
        {
            collects.push_back(make_pair(static_cast<ICollect*>(iptr), i));
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
        if (vars.at(it->first) > TVEC)
        {
            for (pair<ICollect*, int> p : collects)
            {
                if (p.second <= max && p.second >= min)
                {
#ifdef DEBUG
                    cout << "Vec: " << it->first << " is live at collect call at " << p.second << endl;
#endif
                    p.first->live_references.push_back(new Var(it->first));
                }
            }
        }
    }

    in.assign(regs.size(), a_mode);
    s2vmap vmap = in.get_mapping();
#ifdef DEBUG
    for (auto p : vmap)
    {
        cout << p.first << ", ";
        if (p.second.second == STACK)
        {
            cout << "stack";
        }
        else if (p.second.second == ROOTSTACK)
        {
            cout << "rootstack";
        }
        else
        {
            cout << "WTF?";
            exit(1);
        }
        cout << " : " << p.second.first << endl;
    }
#endif
    unsigned int worst_stack = 0;
    unsigned int worst_rootstack = 0;
    for (auto p : vmap)
    {
        if (vars.at(p.first) < TVEC)
        {
            if (p.second.first > worst_stack)
            {
                worst_stack = p.second.first;
            }
        }
        else if (vars.at(p.first) > TVEC)
        {
            if (p.second.first > worst_rootstack)
            {
                worst_rootstack = p.second.first;
            }
        }
    }
    list<x0::I*> ins;
    ins.push_back(new x0::ILabel("main"));
    int total_offset;
    bool need_stack = worst_stack >= regs.size();
    ins.push_back(new x0::ICall("_lang_debug"));
    if (need_stack)
    {
        total_offset = 8*(worst_stack - regs.size() + 1);
        ins.push_back(new x0::ISrcDst(SUBQ, new x0::Con(total_offset), new x0::Reg("rsp")));
    }
    if (worst_rootstack >= regs.size())
    {
        ICall a("_lang_init_rootstack", { new Con((worst_rootstack - regs.size() + 1))}, new Reg("r12"));
        ins.splice(ins.end(), a.assign(vmap));
        //ins.push_back(new x0::ISrcDst(SUBQ, new x0::Con(8*(worst_rootstack - regs.size() + 1)), new x0::Reg("r12")));
    }
    for (auto iptr : this->instr)
    {
        // temp hack FIXME
        if (typeid(*iptr) == typeid(IRet))
        {
            auto r = static_cast<IRet*>(iptr);
            ins.push_back(new x0::ISrcDst(MOVQ, r->arg->assign(vmap), new x0::Reg("rax")));
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
            ins.splice(ins.end(), iptr->assign(vmap));
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

x0::Arg* Reg::assign(const s2vmap &vmap)
{
    return new x0::Reg(this->name);
}

x0::Arg* Reg8::assign(const s2vmap &vmap)
{
    return new x0::Reg8(this->name);
}

x0::Arg* Var::assign(const s2vmap &vmap)
{
    string v = this->var;
    if (vmap.at(v).first < regs.size())
    {
        return new x0::Reg(regs.at(vmap.at(v).first));
    }
    else
    {
        if (vmap.at(v).second == STACK)
        {
            return new x0::Mem("rsp", -8*(vmap.at(v).first-regs.size()));
        }
        else if (vmap.at(v).second == ROOTSTACK)
        {
            return new x0::Mem("r12", 8*(vmap.at(v).first-regs.size()));
        }
        else
        {
            cerr << "Var::assign, WTF?";
            exit(1);
        }
    }
}

x0::Arg* Deref::assign(const s2vmap &vmap)
{
    return new x0::Mem(reg->name, offset);
}

x0::Arg* Global::assign(const s2vmap &vmap)
{
    return new x0::Global(name);
}

x0::Arg* Con::assign(const s2vmap &vmap)
{
    return new x0::Con(this->val);
}

list<x0::I*> INoArg::assign(const s2vmap &vmap)
{
    return { new x0::INoArg(this->instr) };
}

list<x0::I*> ISrc::assign(const s2vmap &vmap)
{
    return { new x0::ISrc(this->instr, this->src->assign(vmap))};
}
list<x0::I*> IDst::assign(const s2vmap &vmap)
{
    x0::Dst* d = static_cast<x0::Dst*>(this->dst->assign(vmap));
    return { new x0::IDst(this->instr, d) };
}
list<x0::I*> ISrcDst::assign(const s2vmap &vmap)
{
    x0::Dst* d = static_cast<x0::Dst*>(this->dst->assign(vmap));
    return { new x0::ISrcDst(this->instr, this->src->assign(vmap), d) };
}
list<x0::I*> ISrcSrc::assign(const s2vmap &vmap)
{
    return { new x0::ISrcSrc(this->instr, this->src->assign(vmap), this->src2->assign(vmap)) };
}

list<x0::I*> IIf::assign(const s2vmap &vmap)
{
    list<x0::I*> ins;
    for (I* i : ifi)
    {
        ins.splice(ins.end(), i->assign(vmap));
    }
    for (I* i : elsei)
    {
        ins.splice(ins.end(), i->assign(vmap));
    }
    for (I* i : theni)
    {
        ins.splice(ins.end(), i->assign(vmap));
    }
    return ins;
}

list<x0::I*> IJmp::assign(const s2vmap &vmap)
{
    return { new x0::IJmp(this->instr, this->label) };
}

list <x0::I*> ICollect::assign(const s2vmap &vmap)
{
    vector<x0::Dst*> move_refs;
    int worst_offset = 0;
    for (Dst* r : live_references)
    {
        if (typeid(*r) != typeid(Var))
        {
            Var* v = static_cast<Var*>(r);
            int offs = vmap.at(v->var).first - regs.size() + 1;
            if (offs < 0)
            {
                move_refs.push_back(static_cast<x0::Dst*>(v->assign(vmap)));
            }
            else
            {
                // get highest offset -> push registers on that
                if (worst_offset < offs)
                {
                    worst_offset = offs;
                }
            }
        }
        else
        {
            cerr << "HMMMM. x0s::ICollect does not support non-var vectors for now\n";
            exit(4);
        }
    }
    list<x0::I*> instrs;
    for (int i = 0; i < (int)move_refs.size(); i++)
    {
        instrs.push_back(new x0::ISrcDst(MOVQ, move_refs.at(i), new x0::Mem("r11", worst_offset+i)));
    }
    ICall call_collect("_lang_collect", { new Reg("r12"), new Con(worst_offset+move_refs.size()-1)}, new Reg("r15"));
    instrs.splice(instrs.end(), call_collect.assign(vmap));
    for (int i = 0; i < (int)move_refs.size(); i++)
    {
        instrs.push_back(new x0::ISrcDst(MOVQ, new x0::Mem("r11", worst_offset+i), move_refs.at(i)));
    }

#ifdef DEBUG_BUILD
    ICall call_dbg_1("_lang_print_num", { new Reg("rax") }, nullptr);
    ICall call_dbg_2("_lang_print_num", { new Reg("r15") }, nullptr);
    auto dbg1 = call_dbg_1.assign(vmap);
    dbg1.splice(dbg1.end(), call_dbg_2.assign(vmap));
    dbg1.splice(dbg1.end(), instrs);
    return dbg1;
#else
    return instrs;
#endif
}

list<x0::I*> ICall::assign(const s2vmap &vmap)
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
        a.push_back(new x0::ISrcDst(MOVQ, (*it_pair.first)->assign(vmap), new x0::Reg(*it_pair.second)));
    }
    a.push_back(new x0::ICall(this->label));
    if (dst != nullptr)
    {
        a.push_back(new x0::ISrcDst(MOVQ, new x0::Reg("rax"), static_cast<x0::Dst*>(dst->assign(vmap))));
    }
    return a;
}

list<x0::I*> ILabel::assign(const s2vmap &vmap)
{
    return { new x0::ILabel(this->name) };
}

list<x0::I*> IRet::assign(const s2vmap &vmap)
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

list <string> ICollect::get_vars()
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

