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
    c0::Arg* a = this->e->to_c0(vars, stmts);
    type t = this->type_check();
    if (t == TERROR)
    {
        cerr << "Type check failed";
        exit(1);
    }
    return c0::P(vars, stmts, a, t);
}

type P::type_check() const
{
    unordered_map<string, type> vars;
    return this->e->t_check(vars);
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

type Num::t_check(unordered_map<string, type> vmap) const
{
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

type Bool::t_check(unordered_map<string, type> vmap) const
{
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

type Read::t_check(unordered_map<string, type> vmap) const
{
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

type Binop::t_check(unordered_map<string, type> vmap) const
{
    type lt = this->l->t_check(vmap);
    type rt = this->r->t_check(vmap);
    if (lt == TERROR || rt == TERROR)
    {
        cerr << "Binop: args have unresolvable types\n";
        return TERROR;
    }
    if (this->op == B_PLUS) // num,num -> num
    {
        if (lt == TNUM && rt == TNUM)
        {
            return TNUM;
        }
        else
        {
            cerr << "Expected num,num got " << lt << ", " << rt << endl;
            return TERROR;
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
            return TBOOL;
        }
        else
        {
            cerr << "Expected num,num got " << lt << ", " << rt << endl;
            return TERROR;
        }
    }
    else
    {
        cerr << "Unsupported operator: " << this->op << endl;
        return TERROR;
    }
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

type Unop::t_check(unordered_map<string, type> vmap) const
{
    type vt = this->v->t_check(vmap);
    if (vt == TERROR)
    {
        cerr << "Unop: child has unresolved type\n";
        return TERROR;
    }
    if (this->op == U_NEG) // num -> num
    {
        if (vt == TNUM)
        {
            return TNUM;
        }
        else
        {
            cerr << "U_NEG expects a TNUM\n";
            return TERROR;
        }
    }
    else if (this->op == U_NOT) // bool -> bool
    {
        if (vt == TBOOL)
        {
            return TBOOL;
        }
        else
        {
            cerr << "U_NOT expects a TBOOL\n";
            return TERROR;
        }
    }
    else
    {
        cerr << "Bad unary op " << this->op << endl;
        return TERROR;
    }
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

type Var::t_check(unordered_map<string, type> vmap) const
{
    return vmap.at(this->name);
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

type Let::t_check(unordered_map<string, type> vmap) const
{
    vmap[this->name] = this->ve->t_check(vmap);
    return this->be->t_check(vmap);
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
    // TODO
    string s = gensym("r0If");
    vars.push_back(s);
    stmts.push_back(new c0::S(s, new c0::Num(10)));
    return new c0::Var(s);
}

type If::t_check(unordered_map<string, type> vmap) const
{
    if (conde->t_check(vmap) != TBOOL)
    {
        cerr << "If: cond expression does not have type bool\n";
        return TERROR;
    }
    type thent = thene->t_check(vmap);
    type elset = elsee->t_check(vmap);
    if (thent == elset)
    {
        return thent; // if both are TERROR, we'll catch it from the ret anyway
    }
    else
    {
        cerr << "If: then and else expression types not matching\n";
        return TERROR;
    }
}


