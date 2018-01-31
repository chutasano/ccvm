#pragma once

#include <map>
#include <string>
#include <vector>

#include "operators.h"

namespace r0
{

    struct E
    {
        virtual ~E() { }
        virtual std::vector<E*> get_childs() = 0;
        virtual void uniquify(std::map<std::string, std::string>) = 0;
    };

    struct P
    {
        P(E* ee) : e(ee) { }
        E* e;

        void uniquify();
        bool is_unique();
    };

    struct Num : E
    {
        Num(int v) { value = v; }
        int value;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
    };

    struct Read : E
    {
        Read() { }
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
    };

    struct Binop : E
    {
        Binop(b_ops oper, E* left, E* right) : op(oper), l(left), r(right) { }
        b_ops op;
        E* l;
        E* r;
        std::vector<E*> get_childs() { return {l, r}; }
        void uniquify(std::map<std::string, std::string>);
    };

    struct Unop : E
    {
        Unop(u_ops oper, E* value) : op(oper), v(value) { }
        u_ops op;
        E* v;
        std::vector<E*> get_childs() { return {v}; }
        void uniquify(std::map<std::string, std::string>);
    };

    struct Var : E
    {
        Var(std::string varname) { name = varname; }
        std::string name;
        std::vector<E*> get_childs() { return {}; }
        void uniquify(std::map<std::string, std::string>);
    };

    struct Let : E
    {
        Let(std::string varname, E* vexp, E* bexp) : name(varname), ve(vexp), be(bexp) { }
        std::string name;
        E* ve;
        E* be;
        std::vector<E*> get_childs() { return {ve, be}; }
        void uniquify(std::map<std::string, std::string>);
    };

}
