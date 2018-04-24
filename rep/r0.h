#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "c0.h"
#include "operators.h"
#include "type.h"

namespace r0
{

    struct E
    {
        E() : t(TUNKNOWN) { }
        virtual ~E() { }
        virtual std::list<E*> get_childs() = 0;
        virtual std::list<std::string> get_vars();
        virtual void uniquify(std::unordered_map<std::string, std::string>);
        virtual int t_check(std::unordered_map<std::string, int>&) = 0;
        virtual E* lambda_lift() { return this; } //TODO
        virtual c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                               std::vector<c0::AS*> &stmts,
                               std::vector<c0::F> &c0fs) const = 0;
        virtual E* clone() const = 0;
        void deep_delete();
        virtual E* desugar() = 0;
        virtual inline void fix_trustme(int t, std::unordered_map<std::string, int>&);
        int t;
    };

    struct Num : E
    {
        Num(int64_t v) { value = v; }
        int64_t value;
        std::list<E*> get_childs() { return {}; }
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Num* clone() const;
        E* desugar() { return this; }
    };

    struct Bool : E
    {
        Bool(tbool v) { value = v; }
        tbool value;
        std::list<E*> get_childs() { return {}; }
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Bool* clone() const;
        E* desugar() { return this; }
    };

    struct Read : E
    {
        Read() { }
        std::list<E*> get_childs() { return {}; }
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Read* clone() const;
        E* desugar() { return this; }
    };

    struct Binop : E
    {
        Binop(b_ops oper, E* left, E* right) : op(oper), l(left), r(right) { }
        b_ops op;
        E* l;
        E* r;
        std::list<E*> get_childs() { return {l, r}; }
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Binop* clone() const;
        E* desugar() { l = l->desugar(); r = r->desugar(); return this; }
    };

    struct Unop : E
    {
        Unop(u_ops oper, E* value) : op(oper), v(value) { }
        u_ops op;
        E* v;
        std::list<E*> get_childs() { return {v}; }
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Unop* clone() const;
        E* desugar() { v = v->desugar(); return this; }
    };

    struct Var : E
    {
        Var(std::string varname) : name(varname) { }
        Var(std::string varname, int t) : name(varname) { this->t = t; } 
        std::string name;
        std::list<E*> get_childs() { return {}; }
        std::list<std::string> get_vars() override { return {name}; }
        void uniquify(std::unordered_map<std::string, std::string>) override;
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Var* clone() const;
        E* desugar() { return this; }
        inline void fix_trustme(int t, std::unordered_map<std::string, int>&);
    };

    struct GlobalVar : E
    {
        GlobalVar(std::string varname) { name = varname; }
        std::string name;
        std::list<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>) override;
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        GlobalVar* clone() const;
        E* desugar() { return this; }
    };

    struct Call : E
    {
        Call(std::string fname, std::list<E*> ee) :
            func(new GlobalVar(fname)), args(ee), t_tentative(TUNKNOWN) { }
        Call(std::string fname, std::list<E*> ee, type t) :
            func(new GlobalVar(fname)), args(ee), t_tentative(t) { }
        Call(E* func, std::list<E*> ee) :
            func(func), args(ee), t_tentative(TUNKNOWN) { }
        Call(E* func, std::list<E*> ee, type t) :
            func(func), args(ee), t_tentative(t) { }
        E* func;
        std::list<E*> args;
        std::list<E*> get_childs()
        {
            std::list<E*> args_cpy = args;
            args_cpy.push_back(func);
            return args_cpy;
        }
        // an extra t is necessary because we use existing t to check for first
        // initialization on type_check, so t has to remain TUNKNOWN until
        // type_check runs at least once
        type t_tentative;
        int t_check(std::unordered_map<std::string, int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Call* clone() const;
        E* desugar();
    };

    struct Let : E
    {
        Let(std::string varname, E* vexp, E* bexp) : name(varname), ve(vexp), be(bexp) { }
        std::string name;
        E* ve;
        E* be;
        std::list<E*> get_childs() { return {ve, be}; }
        void uniquify(std::unordered_map<std::string, std::string>) override;
        int t_check(std::unordered_map<std::string , int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Let* clone() const;
        E* desugar() { ve = ve->desugar(); be = be->desugar(); return this; }
    };

    struct If : E
    {
        If(E* cexp, E* thenexp, E* elseexp) : conde(cexp), thene(thenexp), elsee(elseexp) { }
        E* conde;
        E* thene;
        E* elsee;
        std::list<E*> get_childs() { return { conde, thene, elsee}; }
        int t_check(std::unordered_map<std::string, int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        If* clone() const;
        E* desugar() { conde = conde->desugar(); thene = thene->desugar(); elsee = elsee->desugar(); return this;}
    };

    struct Vector : E
    {
        Vector(std::list<E*> ee) : elist(ee) { }
        std::list<E*> elist;
        std::list<E*> get_childs() { return elist; }
        int t_check(std::unordered_map<std::string, int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Vector* clone() const;
        E* desugar();
    };

    struct VectorRef : E
    {
        VectorRef(E* v, int index) : vec(v), index(index) { }
        // potentially misleading name, vec can be a variable pointing to a vec as well
        // (it can even be a let or if that returns a vec)
        E* vec;
        int index;
        std::list<E*> get_childs() { return { vec }; }
        int t_check(std::unordered_map<std::string, int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        VectorRef* clone() const { return new VectorRef(vec->clone(), index); }
        E* desugar() { vec = vec->desugar(); return this; }
    };

    struct VectorSet : E
    {
        VectorSet(E* v, int index, E* asg) : vec(v), index(index), asg(asg) { }
        E* vec;
        int index;
        E* asg;
        std::list<E*> get_childs() { return { vec, asg }; }
        int t_check(std::unordered_map<std::string, int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        VectorSet* clone() const { return new VectorSet(vec->clone(), index, asg->clone()); }
        E* desugar() { vec = vec->desugar(); asg = asg->desugar(); return this; };
    };

    struct Lambda : E
    {
        Lambda(std::vector<std::string> args, E* body) : args(args), body(body) { }
        std::vector<std::string> args;
        E* body;
        std::list<E*> get_childs() { return { body }; }
        void uniquify(std::unordered_map<std::string, std::string> m) override;
        int t_check(std::unordered_map<std::string, int>&);
        E* lambda_lift() override;
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
        Lambda* clone() const { return new Lambda(args, body->clone()); }
        E* desugar() { body = body->desugar(); return this; };
    };


    /********* Below are syntactic sugars *********/
    struct Sugar : E
    {
        void uniquify(std::unordered_map<std::string, std::string>) override;
        int t_check(std::unordered_map<std::string, int>&);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars,
                       std::vector<c0::AS*> &stmts,
                       std::vector<c0::F> &c0fs) const;
    };

    struct Begin : Sugar
    {
        Begin(std::list<E*> exps) : elist(exps) { }
        std::list<E*> elist;
        std::list<E*> get_childs();
        Begin* clone() const;
        E* desugar();
    };

    struct F
    {
        F(std::string name, std::vector<Var> args, E* e) : name(name), args(args), t(TUNKNOWN), e(e) { }
        F(std::string name, std::vector<Var> args, int t, E* e) : name(name), args(args), t(t), e(e) { }
        F(const F &obj);
        std::string name;
        std::vector<Var> args;
        int t;
        E* e;
        F clone() const;
        void deep_delete() { this->e->deep_delete(); delete this->e; }
        bool is_unique() const;
        void uniquify(std::unordered_map<std::string, std::string>);
        void type_check(std::unordered_map<std::string, int> vars);
        void generate_fun_type(std::unordered_map<std::string, int> &vars);
        void flatten(std::vector<c0::F> &c0fs) const;
        void desugar();
    };

    struct P
    {
        P(std::vector<F> fs, std::string to_run, int heap_s);
        P(E* ee, int heap_s);
        P(const P &obj);
        std::vector<F> funcs;
        int heap_size;
        int t;
        std::string to_run;
        void deep_delete();
        bool is_unique() const;
        void uniquify();
        void type_check();
        c0::P flatten() const;
        void desugar();
    };
}

