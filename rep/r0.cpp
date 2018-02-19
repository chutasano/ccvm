#include <iostream>
#include <map>
#include <queue>
#include <typeinfo>
#include <set>
#include <string>
#include <vector>

#include "c0.h"
#include "r0.h"

using namespace std;
using namespace r0;

//#define DEBUG

static map<string, unsigned int> count;

string gensym(string sym)
{
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

void P::uniquify()
{
    count.clear();
    map<string, string> varmap;
    this->e->uniquify(varmap);
}

bool P::is_unique()
{
    vector<string> varnames; // FIXME is vector overkill for finding duplicates?
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


void Num::uniquify(map<string, string> m)
{
    return;
}

void Read::uniquify(map<string, string> m)
{
    return;
}

void Binop::uniquify(map<string, string> m)
{
    this->l->uniquify(m);
    this->r->uniquify(m);
}

void Unop::uniquify(map<string, string> m)
{
    this->v->uniquify(m);
}

void Var::uniquify(map<string, string> m)
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

void Let::uniquify(map<string, string> m)
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

vector<string> vars;
vector<c0::S> stmts;

c0::P P::flatten()
{
    c0::Arg* a = this->e->to_c0();
    return c0::P(vars, stmts, a);
}


c0::Arg* Num::to_c0()
{
     return new c0::Num(this->value);
}

c0::Arg* Read::to_c0()
{
    string s = gensym("r0Read");
    vars.push_back(s);
    stmts.push_back(c0::S(s, new c0::Read()));
    return new c0::Var(s);
}

c0::Arg* Binop::to_c0()
{
    string s = gensym("r0Binop");
    vars.push_back(s);
    stmts.push_back(c0::S(s, new c0::Binop(this->op,
                    this->l->to_c0(),
                    this->r->to_c0())));
    return new c0::Var(s);
}

c0::Arg* Unop::to_c0()
{
    string s = gensym("r0Unop");
    vars.push_back(s);
    stmts.push_back(c0::S(s, new c0::Unop(this->op, this->v->to_c0())));
    return new c0::Var(s);
}

c0::Arg* Var::to_c0()
{
    return new c0::Var(this->name);
}

c0::Arg* Let::to_c0()
{
    stmts.push_back(c0::S(this->name, this->ve->to_c0()));
    vars.push_back(this->name);
    return this->be->to_c0();
}


