#include <map>
#include <queue>
#include <typeinfo>
#include <set>
#include <string>
#include <vector>

#include "r0.h"

using namespace std;
using namespace r0;

//#define DEBUG

map<string, unsigned int> count;

void P::uniquify()
{
    count.clear();
    map<string, string> varmap;
    e->uniquify(varmap);
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
    unsigned int id = 0;
    auto it = count.find(this->name);
    if (it != count.end())
    {
        id = ++(it->second);
    }
    else
    {
        count[this->name] = 0;
    }
#ifdef DEBUG
    cout << "Uniquify let: changing " << this->name << " to id " << id << endl;
#endif
    m[this->name] = this->name + "_" + to_string(id);
    this->name = m[this->name];
    this->be->uniquify(m);
}
