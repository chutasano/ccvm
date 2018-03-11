#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

#include "rep/r0.h"
#include "rep/type.h"
#include "test.h"

using namespace std;
using namespace r0;

// interpreter only maybe vector
struct mv
{
    mv() { }
    mv(int a) : val(a), is_vector(false) { }
    mv(int a, bool b) : val(a), is_vector(b) { }
    int val;
    bool is_vector;
};

vector<vector<mv> > vecs;

// I kept eval separate from the r0 classes because I felt that the interpreter
// should be its own separate entity. Hence, I have to use some nasty casting.
mv eval(const E* e, map<string, mv> vmap)
{
    if (typeid(*e) == typeid(Num))
    {
        return mv(static_cast<const Num*>(e)->value);
    }
    else if (typeid(*e) == typeid(Bool))
    {
        return mv(static_cast<const Bool*>(e)->value);
    }
    else if (typeid(*e) == typeid(Read))
    {
        int val;
        cin >> val;
        return mv(val);
    }
    else if (typeid(*e) == typeid(Binop))
    {
        auto b = static_cast<const Binop*>(e);
        int l = eval(b->l, vmap).val;
        int r = eval(b->r, vmap).val;
        int result;
        switch (b->op)
        {
            case B_PLUS:
                result = l + r;
                break;
            case B_EQ:
                result = (l == r) ? TB_TRUE : TB_FALSE;
                break;
            case B_LT:
                result = (l < r) ? TB_TRUE : TB_FALSE;
                break;
            case B_GT:
                result = (l > r) ? TB_TRUE : TB_FALSE;
                break;
            case B_LE:
                result = (l <= r) ? TB_TRUE : TB_FALSE;
                break;
            case B_GE:
                result = (l >= r) ? TB_TRUE : TB_FALSE;
                break;
            default:
                cout << "WARN: unknown binary operator: " << b->op << "\n";
                break;
        }
        return mv(result);
    }
    else if (typeid(*e) == typeid(Unop))
    {
        auto u = static_cast<const Unop*>(e);
        int val = eval(u->v, vmap).val;
        int result;
        switch (u->op)
        {
            case U_NEG:
                result = -val;
                break;
            case U_NOT:
                result = (val == TB_TRUE) ? TB_FALSE : TB_TRUE;
                break;
            default:
                cout << "WARN: unknown unary operator: " << u->op << "\n";
                break;
        }
        return mv(result);
    }
    else if (typeid(*e) == typeid(Var))
    {
        auto v = static_cast<const Var*>(e);
        return vmap.at(v->name);
    }
    else if (typeid(*e) == typeid(Let))
    {
        auto l = static_cast<const Let*>(e);
        vmap[l->name] = eval(l->ve, vmap);
        return eval(l->be, vmap);
    }
    else if (typeid(*e) == typeid(If))
    {
        auto i = static_cast<const If*>(e);
        return (eval(i->conde, vmap).val == TB_TRUE) ? eval(i->thene, vmap) : eval(i->elsee, vmap);
    }
    else if (typeid(*e) == typeid(Vector))
    {
        auto v = static_cast<const Vector*>(e);
        vector<mv> thisvec;
        for (auto e : v->elist)
        {
            thisvec.push_back(eval(e, vmap));
        }
        vecs.push_back(thisvec);
        return mv(vecs.size()-1, true);
    }
    else if (typeid(*e) == typeid(VectorRef))
    {
        auto vref = static_cast<const VectorRef*>(e);
        // we can assume this is an index b/c we assume type is correct
        mv i = eval(vref->vec, vmap);
        if (!i.is_vector)
        {
            cerr << "WTF, type error?\n";
            return mv(0);
        }
        return vecs[i.val][vref->index];
    }
    else if (typeid(*e) == typeid(VectorSet))
    {
        auto vset = static_cast<const VectorSet*>(e);
        mv i = eval(vset->vec, vmap);
        if (!i.is_vector)
        {
            cerr << "WTF, type error?\n";
            return mv(0);
        }
        mv v = eval(vset->asg, vmap);
        vecs[i.val][vset->index] = v;
        return TV_VOID;
    }
    else
    {
        if (typeid(e) == typeid(const E*))
        {
            cerr << "HAHA";
        }
        cerr << "ERROR: expression type invalid?";
        return 0;
    }
}

mv interp(const P &p)
{
    vecs.clear();
    map<string, mv> map;
    return eval(p.e, map);
}

static bool veceq(mv actual, vec_t expect[], int where)
{
    if (actual.is_vector && expect[where].t == TVEC)
    {
        bool status = true;
        for (int i = 0; i < expect[where].val; i++)
        {
            vector<mv> cur = vecs.at(actual.val);
            if (cur.at(i).is_vector)
            {
                status &= veceq(cur.at(i), expect, where+i+1);
            }
            else
            {
                status &= cur.at(i).val == expect[where+i+1].val;
            }
        }
        return status;
    }
    else
    {
        cerr << "ERROR: got a vector, expected non-vector";
        return false;
    }
}

bool test_interp(const P &p, vec_t expect[])
{
    mv actual = interp(p);
    if (!actual.is_vector)
    {
        return expect[0].val == actual.val;
    }
    else
    {
        return veceq(actual, expect, 0);
    }
}
