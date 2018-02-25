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
        virtual ~E() { }
        virtual std::vector<E*> get_childs() = 0;
        virtual void uniquify(std::unordered_map<std::string, std::string>) = 0;
        virtual type t_check(std::unordered_map<std::string, type>) const = 0;
        virtual c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const = 0;
        virtual E* clone() const = 0;
        virtual void deep_delete() = 0;
    };

    struct P
    {
        P(E* ee) : e(ee) { }
        P(const P &obj);
        E* e;
        void deep_delete();
        bool is_unique() const;
        void uniquify();
        type type_check() const;
        c0::P flatten() const;
    };

    struct Num : E
    {
        Num(int64_t v) { value = v; }
        int64_t value;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
        Num* clone() const;
        void deep_delete() { }
    };

    struct Bool : E
    {
        Bool(tbool v) { value = v; }
        tbool value;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
        Bool* clone() const;
        void deep_delete() { }
    };

    struct Read : E
    {
        Read() { }
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
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
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
        Binop* clone() const;
        void deep_delete() { this->l->deep_delete(); this->r->deep_delete(); delete this->l; delete this->r; }
    };

    struct Unop : E
    {
        Unop(u_ops oper, E* value) : op(oper), v(value) { }
        u_ops op;
        E* v;
        std::vector<E*> get_childs() { return {v}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
        Unop* clone() const;
        void deep_delete() { this->v->deep_delete(); delete this->v; }
    };

    struct Var : E
    {
        Var(std::string varname) { name = varname; }
        std::string name;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
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
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string , type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
        Let* clone() const;
        void deep_delete() { this->ve->deep_delete(); this->be->deep_delete(); delete this->ve; delete this->be; }
    };

    struct If : E
    {
        If(E* cexp, E* thenexp, E* elseexp) : conde(cexp), thene(thenexp), elsee(elseexp) { }
        E* conde;
        E* thene;
        E* elsee;
        std::vector<E*> get_childs() { return { conde, thene, elsee}; }
        void uniquify(std::unordered_map<std::string, std::string>);
        type t_check(std::unordered_map<std::string, type>) const;
        c0::Arg* to_c0(std::vector<std::string> &vars, std::vector<c0::AS*> &stmts) const;
        If* clone() const;
        void deep_delete() { this->conde->deep_delete(); this->thene->deep_delete(); this->elsee->deep_delete();
            delete this->conde; delete this->thene; delete this->elsee; }
    };
}

