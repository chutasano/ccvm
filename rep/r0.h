#pragma once

#include <map>
#include <string>
#include <vector>

#include "c0.h"
#include "operators.h"
#include "type.h"

namespace r0
{
    enum bool_val 
    {
        BV_FALSE = 0,
        BV_TRUE = 1
    };

    struct E
    {
        virtual ~E() { }
        virtual std::vector<E*> get_childs() = 0;
        virtual void uniquify(std::map<std::string, std::string>) = 0;
        virtual type t_check(std::map<std::string, type>) = 0;
        virtual c0::Arg* to_c0() = 0;
        virtual E* clone() const = 0;
        virtual void deep_delete() = 0;
    };

    struct P
    {
        P(E* ee) : e(ee) { }
        P(const P &obj);
        E* e;
        void deep_delete();
        bool is_unique();
        void uniquify();
        type type_check();
        c0::P flatten();
    };

    struct Num : E
    {
        Num(int64_t v) { value = v; }
        int64_t value;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Num* clone() const;
        void deep_delete() { }
    };

    struct Bool : E
    {
        Bool(bool_val v) { value = v; }
        bool_val value;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Bool* clone() const;
        void deep_delete() { }
    };

    struct Read : E
    {
        Read() { }
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Read* clone() const;
        void deep_delete() { }
    };

    struct Binop : E
    {
        Binop(b_ops oper, E* left, E* right) : op(oper), l(left), r(right) { }
        b_ops op;
        E* l;
        E* r;
        std::vector<E*> get_childs() { return {l, r}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Binop* clone() const;
        void deep_delete() { this->l->deep_delete(); this->r->deep_delete(); delete this->l; delete this->r; }
    };

    struct Unop : E
    {
        Unop(u_ops oper, E* value) : op(oper), v(value) { }
        u_ops op;
        E* v;
        std::vector<E*> get_childs() { return {v}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Unop* clone() const;
        void deep_delete() { this->v->deep_delete(); delete this->v; }
    };

    struct Var : E
    {
        Var(std::string varname) { name = varname; }
        std::string name;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Var* clone() const;
        void deep_delete() { }
    };

    struct Let : E
    {
        Let(std::string varname, E* vexp, E* bexp) : name(varname), ve(vexp), be(bexp) { }
        std::string name;
        E* ve;
        E* be;
        std::vector<E*> get_childs() { return {ve, be}; }
        void uniquify(std::map<std::string, std::string>);
        type t_check(std::map<std::string, type>);
        c0::Arg* to_c0();
        Let* clone() const;
        void deep_delete() { this->ve->deep_delete(); this->be->deep_delete(); delete this->ve; delete this->be; }
    };
}

