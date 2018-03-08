#include <iostream>
#include <queue>
#include <typeinfo>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "c0.h"
#include "r0.h"

using namespace std;
using namespace r0;

//#define DEBUG

//MARKS
//'p
//'[n]um
//'[b]ool
//'[r]ead
//'[t]wo args (Binop)
//'[u]nop
//'[v]ar
//'[l]et
//'[i]f
//'[s]ugar

unordered_map<type, list<type>, hash<int> > vec_type;

string gensym(string sym, bool reset = false)
{
    static unordered_map<string, unsigned int> count;
    if (reset && sym == "")
    {
        count.clear();
        return "";
    }
    unsigned int id = 0;
    auto it = count.find(sym);
    if (it != count.end())
    {
        id = ++(it->second);
    }
    else
    {
        count[sym] = 0;
    }
    return sym + "_" + to_string(id);
}

P::P(const P &obj)
{
    this->e = obj.e->clone();
}

void P::deep_delete()
{
    this->e->deep_delete();
    delete this->e;
}

void P::uniquify()
{
    gensym("", true); // resets gensym count
    unordered_map<string, string> varmap;
    this->e->uniquify(varmap);
}

bool P::is_unique() const
{
    list<string> varnames;
    queue<E*> nodes;
    nodes.push(e);
    while (!nodes.empty())
    {
        E* node = nodes.front();
        nodes.pop();
        if (typeid(*node) == typeid(Let)) // track all declared vars
        {
            varnames.push_back((static_cast<Let*>(node)->name));
        }
        auto newnodes = node->get_childs();
        for (auto n : newnodes)
        {
            nodes.push(n);
        }
    }
    set<string> uniquenames(varnames.begin(), varnames.end());
    return varnames.size() == uniquenames.size();
}

c0::P P::flatten() const
{
    vector<string> vars;
    vector<c0::AS*> stmts;
    c0::Arg* a = e->to_c0(vars, stmts);
    if (t == TERROR)
    {
        cerr << "Type check failed";
        exit(1);
    }
    return c0::P(vars, stmts, a, t);
}

void P::type_check()
{
    unordered_map<string, type> vars;
    t = e->t_check(vars);
}

void P::desugar()
{
    e = e->desugar();
}

Num* Num::clone() const
{
    return new Num(this->value);
}

void Num::uniquify(unordered_map<string, string> m)
{
    return;
}

c0::Arg* Num::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
     return new c0::Num(this->value);
}

type Num::t_check(unordered_map<string, type> vmap)
{
    t = TNUM;
    return TNUM;
}

Bool* Bool::clone() const
{
    return new Bool(this->value);
}

void Bool::uniquify(unordered_map<string, string> m)
{
    return;
}

c0::Arg* Bool::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
     return new c0::Num(static_cast<int>(this->value));
}

type Bool::t_check(unordered_map<string, type> vmap)
{
    t = TBOOL;
    return TBOOL;
}

Read* Read::clone() const
{
    return new Read();
}

void Read::uniquify(unordered_map<string, string> m)
{
    return;
}

c0::Arg* Read::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    string s = gensym("r0Read");
    vars.push_back(s);
    stmts.push_back(new c0::S(s, new c0::Read()));
    return new c0::Var(s);
}

type Read::t_check(unordered_map<string, type> vmap)
{
    t = TNUM;
    return TNUM;
}

Binop* Binop::clone() const
{
    return new Binop(this->op, this->l->clone(), this->r->clone());
}

void Binop::uniquify(unordered_map<string, string> m)
{
    this->l->uniquify(m);
    this->r->uniquify(m);
}

c0::Arg* Binop::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    string s = gensym("r0Binop");
    vars.push_back(s);
    stmts.push_back(new c0::S(s, new c0::Binop(this->op,
                    this->l->to_c0(vars, stmts),
                    this->r->to_c0(vars, stmts))));
    return new c0::Var(s);
}

type Binop::t_check(unordered_map<string, type> vmap)
{
    if (t == TUNKNOWN)
    {
        type lt = this->l->t_check(vmap);
        type rt = this->r->t_check(vmap);
        if (lt == TERROR || rt == TERROR)
        {
            cerr << "Binop: args have unresolvable types\n";
            t = TERROR;
        }
        if (this->op == B_PLUS) // num,num -> num
        {
            if (lt == TNUM && rt == TNUM)
            {
                t = TNUM;
            }
            else
            {
                cerr << "Expected num,num got " << lt << ", " << rt << endl;
                t = TERROR;
            }
        }
        else if ((this->op == B_EQ) || // num, num -> bool
                (this->op == B_LT) ||
                (this->op == B_GT) ||
                (this->op == B_LE) ||
                (this->op == B_GE))
        {
            if (lt == TNUM && rt == TNUM)
            {
                t = TBOOL;
            }
            else
            {
                cerr << "Expected num,num got " << lt << ", " << rt << endl;
                t = TERROR;
            }
        }
        else
        {
            cerr << "Unsupported operator: " << this->op << endl;
            t = TERROR;
        }
    }
    return t;
}

Unop* Unop::clone() const
{
    return new Unop(this->op, this->v->clone());
}

void Unop::uniquify(unordered_map<string, string> m)
{
    this->v->uniquify(m);
}

c0::Arg* Unop::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    string s = gensym("r0Unop");
    vars.push_back(s);
    stmts.push_back(new c0::S(s, new c0::Unop(this->op, this->v->to_c0(vars, stmts))));
    return new c0::Var(s);
}

type Unop::t_check(unordered_map<string, type> vmap)
{
    if (t == TUNKNOWN)
    {
        type vt = this->v->t_check(vmap);
        if (vt == TERROR)
        {
            cerr << "Unop: child has unresolved type\n";
            t = TERROR;
        }
        if (this->op == U_NEG) // num -> num
        {
            if (vt == TNUM)
            {
                t = TNUM;
            }
            else
            {
                cerr << "U_NEG expects a TNUM\n";
                t = TERROR;
            }
        }
        else if (this->op == U_NOT) // bool -> bool
        {
            if (vt == TBOOL)
            {
                t = TBOOL;
            }
            else
            {
                cerr << "U_NOT expects a TBOOL\n";
                t = TERROR;
            }
        }
        else
        {
            cerr << "Bad unary op " << this->op << endl;
            t = TERROR;
        }
    }
    return t;
}

Var* Var::clone() const
{
    return new Var(this->name);
}

void Var::uniquify(unordered_map<string, string> m)
{
    auto it = m.find(this->name); // FIXME const iterator will be the "right"
                                  // thing to do instead of auto
    if (it != m.end())
    {
#ifdef DEBUG
        cout << "Uniquify var: changing " << this->name << " to " << it->second << endl;
#endif
        this->name = it->second;
    }
}

c0::Arg* Var::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    return new c0::Var(this->name);
}

type Var::t_check(unordered_map<string, type> vmap)
{
    if (t == TUNKNOWN)
    {
        t = vmap.at(this->name);
    }
    return t;
}

Let* Let::clone() const
{
    return new Let(this->name, this->ve->clone(), this->be->clone());
}

void Let::uniquify(unordered_map<string, string> m)
{
    // uniquify the var expression first because the var expression should not
    // have access to the var it's trying to define
    this->ve->uniquify(m);

    m[this->name] = gensym(this->name);
#ifdef DEBUG
    cout << "Uniquify let: changing " << this->name << " to " << m[this->name] << endl;
#endif
    this->name = m[this->name];
    this->be->uniquify(m);
}

c0::Arg* Let::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    stmts.push_back(new c0::S(this->name, this->ve->to_c0(vars, stmts)));
    vars.push_back(this->name);
    return this->be->to_c0(vars, stmts);
}

type Let::t_check(unordered_map<string, type> vmap)
{
    if (t == TUNKNOWN)
    {
        vmap[this->name] = this->ve->t_check(vmap);
        t = this->be->t_check(vmap);
    }
    return t;
}

If* If::clone() const
{
    return new If(conde->clone(), thene->clone(), elsee->clone());
}

void If::uniquify(unordered_map<string, string> m)
{
    conde->uniquify(m);
    thene->uniquify(m);
    elsee->uniquify(m);
}

c0::Arg* If::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    string s = gensym("r0If");
    vars.push_back(s);
    c0::Binop* c0bin; // TODO fix optimization, will not optimize
                      // a Let(Binop()) case
    if (typeid(*conde) == typeid(Binop))
    {
        Binop* cb = static_cast<Binop*>(conde);
        c0::Arg* l = cb->l->to_c0(vars, stmts);
        c0::Arg* r = cb->r->to_c0(vars, stmts);
        c0bin = new c0::Binop(cb->op, l, r);
    }
    else
    {
        c0::Arg* condv = conde->to_c0(vars, stmts);
        c0bin = new c0::Binop(B_EQ, new c0::Num(1), condv);
    }
    vector<c0::AS*> thenstmts;
    vector<c0::AS*> elsestmts;
    c0::Arg* thenv = thene->to_c0(vars, thenstmts);
    c0::Arg* elsev = elsee->to_c0(vars, elsestmts);
    stmts.push_back(new c0::If(s, c0bin , thenv, thenstmts, elsev, elsestmts));
    return new c0::Var(s);
}

type If::t_check(unordered_map<string, type> vmap)
{
    if (t == TUNKNOWN)
    {
        if (conde->t_check(vmap) != TBOOL)
        {
            cerr << "If: cond expression does not have type bool\n";
            t = TERROR;
        }
        type thent = thene->t_check(vmap);
        type elset = elsee->t_check(vmap);
        if (thent == elset)
        {
            t = thent; // if both are TERROR, we'll catch it from the ret anyway
        }
        else
        {
            cerr << "If: then and else expression types not matching\n";
            t = TERROR;
        }
    }
    return t;
}

vector<E*> Sugar::get_childs()
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

void Sugar::uniquify(unordered_map<string, string> a)
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

type Sugar::t_check(unordered_map<string, type> a)
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

c0::Arg* Sugar::to_c0(vector<string> &vars, vector<c0::AS*> &stmts) const
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

Begin* Begin::clone() const
{
    list<E*> newlist;
    for (E* e : elist)
    {
        newlist.push_back(e->clone());
    }
    return new Begin(newlist);
}

void Begin::deep_delete()
{
    for (E* e : elist)
    {
        e->deep_delete();
        delete e;
    }
}

E* Begin::desugar()
{
    auto i = elist.rbegin();
    E* curr = *i;
    for (++i; i != elist.rend(); ++i)
    {
        curr = new Let(gensym("unusedvar"), *i, curr);
    }
    return curr;
}
