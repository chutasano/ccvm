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
        virtual void uniquify(std::unordered_map<std::string, std::string>) = 0;
        virtual int t_check(std::unordered_map<std::string, int>) = 0;
        virtual c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const = 0;
        virtual E* clone() const = 0;
        virtual void deep_delete() = 0;
        virtual E* desugar() = 0;
        int t;
    };

    struct P
    {
        P(E* ee) : e(ee) { }
        P(const P &obj);
        E* e;
        int t;
        void deep_delete();
        bool is_unique() const;
        void uniquify();
        void type_check();
        c0::P flatten() const;
        void desugar();
    };

    struct Num : E
    {
        Num(int64_t v) { value = v; }
        int64_t value;
        std::list<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Num* clone() const;
        void deep_delete() { }
        E* desugar() { return this; }
    };

    struct Bool : E
    {
        Bool(tbool v) { value = v; }
        tbool value;
        std::list<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Bool* clone() const;
        void deep_delete() { }
        E* desugar() { return this; }
    };

    struct Read : E
    {
        Read() { }
        std::list<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Read* clone() const;
        void deep_delete() { }
        E* desugar() { return this; }
    };

    struct Binop : E
    {
        Binop(b_ops oper, E* left, E* right) : op(oper), l(left), r(right) { }
        b_ops op;
        E* l;
        E* r;
        std::list<E*> get_childs() { return {l, r}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Binop* clone() const;
        void deep_delete() { l->deep_delete(); r->deep_delete(); delete l; delete r; }
        E* desugar() { l = l->desugar(); r = r->desugar(); return this; }
    };

    struct Unop : E
    {
        Unop(u_ops oper, E* value) : op(oper), v(value) { }
        u_ops op;
        E* v;
        std::list<E*> get_childs() { return {v}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Unop* clone() const;
        void deep_delete() { v->deep_delete(); delete v; }
        E* desugar() { v = v->desugar(); return this; }
    };

    struct Var : E
    {
        Var(std::string varname) { name = varname; }
        std::string name;
        std::list<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Var* clone() const;
        void deep_delete() { }
        E* desugar() { return this; }
    };

    struct Let : E
    {
        Let(std::string varname, E* vexp, E* bexp) : name(varname), ve(vexp), be(bexp) { }
        std::string name;
        E* ve;
        E* be;
        std::list<E*> get_childs() { return {ve, be}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string , int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Let* clone() const;
        void deep_delete() { ve->deep_delete(); be->deep_delete(); delete ve; delete be; }
        E* desugar() { ve = ve->desugar(); be = be->desugar(); return this; }
    };

    struct If : E
    {
        If(E* cexp, E* thenexp, E* elseexp) : conde(cexp), thene(thenexp), elsee(elseexp) { }
        E* conde;
        E* thene;
        E* elsee;
        std::list<E*> get_childs() { return { conde, thene, elsee}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string, int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        If* clone() const;
        void deep_delete() { conde->deep_delete(); thene->deep_delete(); elsee->deep_delete();
            delete conde; delete thene; delete elsee; }
        E* desugar() { conde = conde->desugar(); thene = thene->desugar(); elsee = elsee->desugar(); return this;}
    };

    struct Vector : E
    {
        Vector(std::list<E*> ee) : elist(ee) { }
        std::list<E*> elist;
        std::list<E*> get_childs() { return elist; }
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string, int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        Vector* clone() const;
        void deep_delete();
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
        void uniquify(std::unordered_map<std::string, std::string> m) { vec->uniquify(m); }
        int t_check(std::unordered_map<std::string, int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        VectorRef* clone() const { return new VectorRef(vec->clone(), index); }
        void deep_delete() { vec->deep_delete(); delete vec; }
        E* desugar() { vec = vec->desugar(); return this; }
    };

    struct VectorSet : E
    {
        VectorSet(E* v, int index, E* asg) : vec(v), index(index), asg(asg) { }
        E* vec;
        int index;
        E* asg;
        std::list<E*> get_childs() { return { vec, asg }; }
        void uniquify(std::unordered_map<std::string, std::string> m) { vec->uniquify(m); asg->uniquify(m); }
        int t_check(std::unordered_map<std::string, int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
        VectorSet* clone() const { return new VectorSet(vec->clone(), index, asg->clone()); }
        void deep_delete() { vec->deep_delete(); asg->deep_delete(); delete vec; delete asg; }
        E* desugar() { vec = vec->desugar(); asg = asg->desugar(); return this; };
    };

    /********* Below are syntactic sugars *********/
    struct Sugar : E
    {
        std::list<E*> get_childs();
        void uniquify(std::unordered_map<std::string, std::string>);
        int t_check(std::unordered_map<std::string, int>);
        c0::Arg* to_c0(std::unordered_map<std::string, int> &vars, std::vector<c0::AS*> &stmts) const;
    };

    struct Begin : Sugar
    {
        Begin(std::list<E*> exps) : elist(exps) { }
        std::list<E*> elist;
        Begin* clone() const;
        void deep_delete();
        E* desugar();
    };
}

