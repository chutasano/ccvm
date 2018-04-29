#include <algorithm>
#include <cassert>
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
//'[z] vector -- zvector?????????????
//'[s]ugar

map<int, vector<int> > vec_type;
map<int, vector<int> > fun_type;

static string gensym(const string &sym, bool reset = false)
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

static int add_vtype(const vector<int> &types)
{
    auto it = vec_type.begin();
    int t = TUNKNOWN;
    for (; it != vec_type.end(); ++it)
    {
        // no need to worry about checking for nested vectors because
        // the above for loop should take care of that and prevent
        // duplicate definition from occuring
        if (it->second == types)
        {
            t = it->first;
            break;
        }
    }
    if (it == vec_type.end())
    {
        // make new entry in vec_type
        int next_index = (--it)->first + 1;
        vec_type[next_index] = types;
        t = next_index;
    }
    return t;
}

static int add_ftype(const vector<int> &ftype)
{
    // expensive... oh well maybe optimize todo?
    auto it = fun_type.begin();
    int t = TUNKNOWN;
    for (; it != fun_type.end(); ++it)
    {
        // no need to worry about checking for nested vectors because
        // the above for loop should take care of that and prevent
        // duplicate definition from occuring
        if (it->second == ftype)
        {
            t = it->first;
            break;
        }
    }
    if (it == fun_type.end())
    {
        int next_index = (--it)->first + 1;
        fun_type[next_index] = ftype;
        t = next_index;
    }
    return t;
}

void E::uniquify(unordered_map<string, string> m)
{
    for (E* e : this->get_childs())
    {
        e->uniquify(m);
    }
}

E* E::lambda_lift(const unordered_map<string, int> &vars)
{
    for (E* &e : this->get_childs())
    {
        e = e->lambda_lift(vars);
    }
    return this;
}

list<string> E::get_vars()
{
    list<string> vars;
    for (E* e : this->get_childs())
    {
        list<string> evars = e->get_vars();
        vars.splice(vars.end(), evars);
    }
    return vars;
}

void E::deep_delete()
{
    for (E* e : this->get_childs())
    {
        e->deep_delete();
        delete e;
    }
}

E* E::desugar()
{
    for (E* &e : this->get_childs())
    {
        e = e->desugar();
    }
    return this;
}

inline void E::fix_trustme(int t, unordered_map<string, int> &vmap)
{
    assert(this->t == TTRUSTME);
    this->t = t;
}


P::P(std::vector<F> fs, string to_run, int heap_s = 2048) : funcs(fs), to_run(to_run)
{
    if (heap_s%8 != 0 || heap_s < 0)
    {
        cerr << "ERROR: invalid heap_size: " << heap_s << endl;
        cerr << "using default heapsize of 2048\n";
        heap_size = 2048;
    }
    else
    {
        heap_size = heap_s;
    }
    t = TUNKNOWN;
}

P::P(E* ee, int heap_s = 2048)
{
    funcs.push_back(F("main", { }, TUNKNOWN, ee));
    to_run = "main";
    if (heap_s%8 != 0 || heap_s < 0)
    {
        cerr << "ERROR: invalid heap_size: " << heap_s << endl;
        cerr << "using default heapsize of 2048\n";
        heap_size = 2048;
    }
    else
    {
        heap_size = heap_s;
    }
    t = TUNKNOWN;
}

P::P(const P &obj)
{
    for (const F &f : obj.funcs)
    {
        this->funcs.push_back(f.clone());
    }
    this->to_run = obj.to_run;
    this->heap_size = obj.heap_size;
    this->t = obj.t;
}

void P::deep_delete()
{
    for (F &f : funcs)
    {
        f.deep_delete();
    }
}

void P::uniquify()
{
    gensym("", true);
    unordered_map<string, string> varmap;
    for (F &f : funcs)
    {
        if (to_run != f.name)
        {
            varmap[f.name] = gensym(f.name);
            f.name = varmap[f.name];
        }
    }
    
    for (F &f : funcs)
    {
        f.uniquify(varmap);
    }
}

bool P::is_unique() const
{
    for (const F &f : funcs)
    {
        if (!f.is_unique())
        {
            return false;
        }
    }
    return true;
}

void P::lambda_lift()
{
    for (F &f : funcs)
    {
        f.lambda_lift();
    }
}

c0::P P::flatten() const
{
    vector<c0::F> cfs;
    for (const F &f : funcs)
    {
        vector<c0::F> c0f;
        f.flatten(c0f);
        cfs.insert(end(cfs), begin(c0f), end(c0f));
    }
    return c0::P(cfs, to_run, heap_size);
}

void P::type_check(bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        if (t == TUNKNOWN)
        {
            vec_type.clear();
            fun_type.clear();
            vec_type[TVEC] = { };
            fun_type[TFUN] = { };
        }
        unordered_map<string, int> vmap;
        for (F &f : funcs)
        {
            f.generate_fun_type(vmap);
        }
        for (F &f : funcs)
        {
            f.type_check(vmap, force);
            if (f.name == to_run)
            {
                t = fun_type.at(f.t).back();
            }
        }
    }
}

void P::desugar()
{
    for (F &f : funcs)
    {
        f.desugar();
    }
}

F::F(const F &obj)
{
    name = obj.name;
    args = obj.args;
    t = obj.t;
    e = obj.e;
}

F F::clone() const
{
    E* e_copied = e->clone();
    return F(name, args, t, e_copied);
}

void F::uniquify(unordered_map<string, string> varmap)
{
    for (Var &v : args)
    {
        varmap[v.name] = gensym(v.name);
        v.name = varmap[v.name];
    }
    this->e->uniquify(varmap);
}

bool F::is_unique() const
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

void F::lambda_lift()
{
    e = e->lambda_lift(v2type);
}

// P is expected to feed in an empty vector
void F::flatten(vector<c0::F> &c0fs) const
{
    unordered_map<string, int> vars;
    vector<string> args_names;
    for (const Var &v : args)
    {
        vars[v.name] = v.t;
        args_names.push_back(v.name);
    }
    vector<c0::AS*> stmts;
    c0::Arg* a = e->to_c0(vars, stmts, c0fs);
    if (t == TERROR)
    {
        cerr << "Type check failed\n";
        exit(1);
    }
    c0fs.push_back(c0::F(name, vars, args_names, stmts, a, t));
}

void F::generate_fun_type(unordered_map<string, int> &vars)
{
    vector<int> ftype;
    if (t < TFUN)
    {
        for (const Var &v : args)
        {
            if (v.t == TUNKNOWN)
            {
                cerr << "F::generate_fun_type: arguments must have known type";
                exit(2);
            }
            else
            {
                ftype.push_back(v.t);
            }
        }
        ftype.push_back(t);
        t = add_ftype(ftype);
    }
    vars[name] = t;
}

void F::type_check(unordered_map<string, int> vars, bool force)
{
    vars[name] = t;
    for (const Var &v : args)
    {
        vars[v.name] = v.t;
    }
    int t_expected = e->t_check(vars, force);
    if (fun_type.at(t).back() == TUNKNOWN || force)
    {
        vector<int> ftype_cpy = fun_type.at(t);
        ftype_cpy.back() = t_expected;
        t = add_ftype(ftype_cpy);
    }
    if (fun_type.at(t).back() != t_expected)
    {
        cerr << "F::type_check: expected return type mismatch with calculated type\n";
        exit(1);
    }
    v2type = vars;
}

void F::desugar()
{
    e = e->desugar();
}

Num* Num::clone() const
{
    return new Num(this->value);
}

c0::Arg* Num::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
     return new c0::Num(this->value);
}

int Num::t_check(unordered_map<string, int> &vmap, bool force)
{
    t = TNUM;
    return TNUM;
}

Bool* Bool::clone() const
{
    return new Bool(this->value);
}

c0::Arg* Bool::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
     return new c0::Num(static_cast<int>(this->value));
}

int Bool::t_check(unordered_map<string, int> &vmap, bool force)
{
    t = TBOOL;
    return TBOOL;
}

Read* Read::clone() const
{
    return new Read();
}

c0::Arg* Read::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0Read");
    vars[s] = t;
    stmts.push_back(new c0::S(s, new c0::Read()));
    return new c0::Var(s);
}

int Read::t_check(unordered_map<string, int> &vmap, bool force)
{
    t = TNUM;
    return TNUM;
}

Binop* Binop::clone() const
{
    return new Binop(this->op, this->l->clone(), this->r->clone());
}

c0::Arg* Binop::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0Binop");
    vars[s] = t;
    stmts.push_back(new c0::S(s, new c0::Binop(this->op,
                    this->l->to_c0(vars, stmts, c0fs),
                    this->r->to_c0(vars, stmts, c0fs))));
    return new c0::Var(s);
}

int Binop::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        int lt = this->l->t_check(vmap, force);
        int rt = this->r->t_check(vmap, force);
        if (lt == TERROR || lt == TUNKNOWN ||
            rt == TERROR || rt == TUNKNOWN)
        {
            cerr << "Binop: args have unresolvable types\n";
            t = TERROR;
        }
        int lt_expected, rt_expected;
        if (this->op == B_PLUS) // num,num -> num
        {
            lt_expected = TNUM;
            rt_expected = TNUM;
            t = TNUM;
        }
        else if ((this->op == B_EQ) || // num, num -> bool
                 (this->op == B_LT) ||
                 (this->op == B_GT) ||
                 (this->op == B_LE) ||
                 (this->op == B_GE))
        {
            lt_expected = TNUM;
            rt_expected = TNUM;
            t = TBOOL;
        }
        else
        {
            cerr << "Unsupported operator: " << this->op << endl;
            t = TERROR;
        }
        if (lt == TTRUSTME)
        {
            this->l->fix_trustme(lt_expected, vmap);
            lt = lt_expected;
        }
        if (rt == TTRUSTME)
        {
            this->r->fix_trustme(rt_expected, vmap);
            rt = rt_expected;
        }
        if (lt != lt_expected || rt != rt_expected)
        {
                cerr << "Expected " << type2name(lt_expected) <<  ", "
                     << type2name(rt_expected) << "... got "
                     << type2name(lt) << ", " << type2name(rt) << endl;
                t = TERROR;
        }
    }
    return t;
}

Unop* Unop::clone() const
{
    return new Unop(this->op, this->v->clone());
}

c0::Arg* Unop::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0Unop");
    vars[s] = t;
    stmts.push_back(new c0::S(s, new c0::Unop(this->op, this->v->to_c0(vars, stmts, c0fs))));
    return new c0::Var(s);
}

int Unop::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        int vt = this->v->t_check(vmap, force);
        int vt_expected;
        if (vt == TERROR || vt ==TUNKNOWN)
        {
            cerr << "Unop: child has unresolved type\n";
            t = TERROR;
        }
        if (this->op == U_NEG) // num -> num
        {
            vt_expected = TNUM;
            t = TNUM;
        }
        else if (this->op == U_NOT) // bool -> bool
        {
            vt_expected = TBOOL;
            t = TBOOL;
        }
        else
        {
            cerr << "Bad unary op " << this->op << endl;
            t = TERROR;
        }
        if (vt == TTRUSTME)
        {
            this->v->fix_trustme(vt_expected, vmap);
            vt = vt_expected;
        }
        if (vt != vt_expected)
        {
                cerr << "Expected " << type2name(vt_expected) <<  ", "
                     << "... got "  << type2name(vt) << endl;
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
    const auto &it = m.find(this->name);
    if (it != m.end())
    {
#ifdef DEBUG
        cout << "Uniquify var: changing " << this->name << " to " << it->second << endl;
#endif
        this->name = it->second;
    }
    else
    {
        cerr << "Uniquify var: var ref DNE: " << this->name << "\n";
        assert(0);
    }
}

c0::Arg* Var::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    return new c0::Var(this->name);
}

int Var::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        t = vmap.at(this->name);
    }
    return t;
}

inline void Var::fix_trustme(int t, unordered_map<string, int> &vmap)
{
    assert(this->t == TTRUSTME);
    this->t = t;
    vmap.at(name) = t;
}

GlobalVar* GlobalVar::clone() const
{
    return new GlobalVar(this->name);
}

void GlobalVar::uniquify(unordered_map<string, string> m)
{
    // global functions need to be uniquified, check if it exists in map
    if (m.find(name) != m.end())
    {
        name = m.at(name);
    }
}

c0::Arg* GlobalVar::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    return new c0::GlobalVar(this->name);
}

int GlobalVar::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        if (vmap.find(name) != vmap.end())
        {
            t = vmap.at(name);
        }
        else
        {
            cerr << "GlobalVar without known type. Investigate this\n";
            assert(0);
        }
    }
    return t;
}

Call* Call::clone() const
{
    list<E*> ecopy;
    for (const E* e : args)
    {
        ecopy.push_back(e->clone());
    }
    return new Call(func->clone(), ecopy, static_cast<type>(t));
}

c0::Arg* Call::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0Call");
    vars[s] = t;
    vector<c0::Arg*> c0args;
    for (E* e : args)
    {
        c0args.push_back(e->to_c0(vars, stmts, c0fs));
    }
    int ft = func->t_check(vars);
    if (type_is_func(ft))
    {
        stmts.push_back(new c0::S(s, new c0::FunCall(func->to_c0(vars, stmts, c0fs), c0args)));
    }
    else if (type_is_vec(ft))
    {
        // TODO hacky way, let lambda_lift magically handle this case
        // TODO super hack to make c0 destructors not double delete. fix properly
        c0::Arg* vec = func->to_c0(vars, stmts, c0fs);
        c0::Arg* vec2 = func->to_c0(vars, stmts, c0fs);
        c0::Arg* vec3 = func->to_c0(vars, stmts, c0fs);
        c0args.insert(c0args.begin(), vec);
        string svec = gensym("r0CallVec");
        string sfref = gensym("r0CallVecFunRef");
        vars[svec] = func->t;
        vars[sfref] = vec_type.at(ft).front();
        stmts.push_back(new c0::S(svec, vec2));
        stmts.push_back(new c0::S(sfref, new c0::VecRef(static_cast<c0::Var*>(vec3), 0)));
        stmts.push_back(new c0::S(s, new c0::FunCall(new c0::Var(sfref), c0args)));
    }
    else
    {
        assert(0);
    }
    return new c0::Var(s);
}

int Call::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        // no type specifier means that function must exist in our context
        // (ie: not a function from somewhere else like the runtime library)
        // that means we can find it in vmap, so we can do a stricter check
        if (t_tentative == TUNKNOWN)
        {
            int i = 0;
            //auto t_vec = fun_type.at(vmap.at(name));
            int ft = func->t_check(vmap, force);
            bool is_vec = type_is_vec(ft);
            if (is_vec) // vecs are OK
            {
                ft = vec_type.at(func->t_check(vmap, force)).front();
            }
            const auto &t_vec = fun_type.at(ft);
            assert(args.size() == t_vec.size()-1 - (is_vec? 1 : 0));
            for (E* e : args)
            {
                int t_arg = e->t_check(vmap, force);
                assert(t_arg == t_vec.at(i + (is_vec? 1 : 0)));
                i++;
            }
            t = t_vec.back();
        }
        else
        {
            for (E* e : args)
            {
                e->t_check(vmap, force);
            }
            t = t_tentative;
        }
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

c0::Arg* Let::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    stmts.push_back(new c0::S(this->name, this->ve->to_c0(vars, stmts, c0fs)));
    vars[this->name] = this->ve->t;
    return this->be->to_c0(vars, stmts, c0fs);
}

int Let::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        vmap[this->name] = this->ve->t_check(vmap, force);
        t = this->be->t_check(vmap, force);
    }
    return t;
}

If* If::clone() const
{
    return new If(conde->clone(), thene->clone(), elsee->clone());
}

c0::Arg* If::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0If");
    vars[s] = t;
    c0::Binop* c0bin; // TODO fix optimization, will not optimize
                      // a Let(Binop()) case
    if (typeid(*conde) == typeid(Binop))
    {
        Binop* cb = static_cast<Binop*>(conde);
        c0::Arg* l = cb->l->to_c0(vars, stmts, c0fs);
        c0::Arg* r = cb->r->to_c0(vars, stmts, c0fs);
        c0bin = new c0::Binop(cb->op, l, r);
    }
    else
    {
        c0::Arg* condv = conde->to_c0(vars, stmts, c0fs);
        c0bin = new c0::Binop(B_EQ, new c0::Num(1), condv);
    }
    vector<c0::AS*> thenstmts;
    vector<c0::AS*> elsestmts;
    c0::Arg* thenv = thene->to_c0(vars, thenstmts, c0fs);
    c0::Arg* elsev = elsee->to_c0(vars, elsestmts, c0fs);
    stmts.push_back(new c0::If(s, c0bin , thenv, thenstmts, elsev, elsestmts));
    return new c0::Var(s);
}

int If::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        if (conde->t_check(vmap, force) != TBOOL)
        {
            cerr << "If: cond expression does not have type bool\n";
            t = TERROR;
        }
        int thent = thene->t_check(vmap, force);
        int elset = elsee->t_check(vmap, force);
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

Vector* Vector::clone() const
{
    list<E*> ecopy;
    for (E* e : elist)
    {
        ecopy.push_back(e->clone());
    }
    return new Vector(ecopy);
}

c0::Arg* Vector::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0Vector");
    vars[s] = t;
    stmts.push_back(new c0::S(s, new c0::Alloc(elist.size(), t)));
    string schild = s + "Children";
    vars[schild] = TNUM;
    int i = 0;
    for (E* e : elist)
    {
        stmts.push_back(new c0::S(schild, new c0::VecSet(new c0::Var(s), i, e->to_c0(vars, stmts, c0fs))));
        i++;
    }
    return new c0::Var(s);
}

int Vector::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        vector<int> types;
        for (E* e : elist)
        {
            types.push_back(e->t_check(vmap, force));
        }
        t = add_vtype(types);
    }
    return t;
}

int VectorRef::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        int t_vec = vec->t_check(vmap, force);
        vector<int> types = vec_type[t_vec];
        t = types[index];
    }
    return t;
}

c0::Arg* VectorRef::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0VecRef");
    vars[s] = t;
    stmts.push_back(new c0::S(s, new c0::VecRef(static_cast<c0::Var*>(vec->to_c0(vars, stmts, c0fs)),
                                                index)));
    return new c0::Var(s);
}

int VectorSet::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        int t_vec = vec->t_check(vmap, force);
        vector<int> types = vec_type[t_vec];
        if(types[index] == asg->t_check(vmap, force))
        {
            t = TVOID;
        }
        else
        {
            t = TERROR;
        }
    }
    return t;
}

c0::Arg* VectorSet::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    string s = gensym("r0VecSet");
    vars[s] = t;
    stmts.push_back(new c0::S(s,
                new c0::VecSet(static_cast<c0::Var*>(vec->to_c0(vars, stmts, c0fs)),
                               index, asg->to_c0(vars, stmts, c0fs))));
    return new c0::Var(s);
}

void Lambda::uniquify(unordered_map<string, string> m)
{
    // manually uniquify function args
    for (Var &v : args)
    {
        // overwrite if exists
        m[v.name] = gensym(v.name);
        v.name = m[v.name];
    }
    body->uniquify(m);
}

int Lambda::t_check(unordered_map<string, int> &vmap, bool force)
{
    if ((t == TUNKNOWN) || force)
    {
        for (const Var &v : args)
        {
            vmap[v.name] = (v.t == TUNKNOWN) ? TTRUSTME : v.t;
        }
        int ret_t = body->t_check(vmap, force);
        vector<int> this_ftype;
        for (const Var &v : args)
        {
            this_ftype.push_back(vmap.at(v.name));
        }
        this_ftype.push_back(ret_t);
        t = add_ftype(this_ftype);
        for (Var &v : args)
        {
            v.t = vmap.at(v.name);
        }
    }
    return t;
}

E* Lambda::lambda_lift(const unordered_map<string, int> &vars)
{
    // TODO maybe use set in get_vars?
    body = body->lambda_lift(vars);
    list<string> all_vars = body->get_vars();
    vector<string> arg_names;
    for(const Var &v : args)
    {
        arg_names.push_back(v.name);
    }
    all_vars.sort();
    all_vars.erase(unique(all_vars.begin(), all_vars.end()), all_vars.end());
    all_vars.erase(remove_if(all_vars.begin(), all_vars.end(),
                             [arg_names](string s) {
                             return find(arg_names.begin(), arg_names.end(), s)
                             != arg_names.end(); }),
                   all_vars.end());
    bool enable_closure = all_vars.size() > 0;
    //all_vars contain all variables that need to be captured in f
    if (enable_closure)
    {
        vector<int> ft = fun_type.at(t);
        // placeholder for vec arg
        ft.insert(ft.begin(), TTRUSTME);
        int new_t = add_ftype(ft); // reserve a spot in ftype
        // to_c0 is post type check so TTRUSTME must be unique
        // vector should consist of (Func, capture1, capture2, ...)
        vector<int> vt = {new_t};
        for (const string &s : all_vars)
        {
            vt.push_back(vars.at(s));
        }
        int new_vec_t = add_vtype(vt);
        fun_type.at(new_t).front() = new_vec_t;
        t = new_t;
        {
            // types are now complete, we need to now modify the current function
            // and wrap it with lets to capture the variables
            // arbitrary new function arg name
            string vector_name = gensym("r0Lambdaclosure_vec");
            Var* vec_var = new Var(vector_name, new_vec_t);
            args.insert(args.begin(), Var(vector_name, new_vec_t));
            E* prev_ref = body;
            int i=1;
            for (const string &s : all_vars)
            {
                // note: the vector_refs will be in reverse order, but
                // it shouldn't matter?? maybe revisit this
                prev_ref = new Let(s, new VectorRef(vec_var, i), prev_ref);
                i++;
            }
            body = prev_ref;
        }
        {
            // lambda is called after uniquify, so we can't have conflicting names
            list<E*> vec_elements = { this };
            for (const string &s : all_vars)
            {
                vec_elements.push_back(new Var(s, vars.at(s)));
            }
            Vector* v = new Vector(vec_elements);
            v->t = new_vec_t;
            return v;
        }
    }
    return this;
}

c0::Arg* Lambda::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{

    string s = gensym("r0Lambda");
    vars[s] = t;
    std::vector<Var> fargs;
    const vector<int> &ft = fun_type.at(t);

    int i=0;
    for (const Var &v : args)
    {
        fargs.emplace_back(v.name, ft.at(i));
        i++;
    }
    F f(s, fargs, t, body);
    // type check is already complete
    f.flatten(c0fs);
    return new c0::GlobalVar(s);
}

void Sugar::uniquify(unordered_map<string, string> a)
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

int Sugar::t_check(unordered_map<string, int> &a, bool force)
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

E* Sugar::lambda_lift(const unordered_map<string, int> &a)
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}



c0::Arg* Sugar::to_c0(unordered_map<string, int> &vars, vector<c0::AS*> &stmts, vector<c0::F> &c0fs) const
{
    cerr << "Call desugar before using any r0->c0 functionality\n";
    exit(10);
}

list<reference_wrapper<E*> > Begin::get_childs()
{
    return list<reference_wrapper<E*> >(elist.begin(), elist.end());
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
