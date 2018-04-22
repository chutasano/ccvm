#pragma once

#include <list>
#include <string>
#include <vector>

#include "operators.h"
#include "type.h"
#include "x0s.h"

namespace c0
{
    struct E
    {
        virtual ~E() { }
        virtual std::list<x0s::I*> select(x0s::Var*) = 0;
    };

    struct Arg : E
    {
        virtual ~Arg() { }
        std::list<x0s::I*> select(x0s::Var*);
        virtual x0s::Arg* to_arg() const = 0;
    };

    struct Var : Arg
    {
        Var(std::string varname) { name = varname; }
        std::string name;
        x0s::Arg* to_arg() const;
    };

    struct GlobalVar : Arg
    {
        GlobalVar(std::string varname) { name = varname; }
        std::string name;
        x0s::Arg* to_arg() const;
    };

    struct Num : Arg
    {
        Num(int64_t v) { value = v; }
        int64_t value;
        x0s::Arg* to_arg() const;
    };

    struct Global : Arg
    {
        Global(std::string varname) : name(varname) { }
        std::string name;
        x0s::Arg* to_arg() const;
    };

    // abstract statement either contains an if or a statement
    struct AS
    {
        virtual ~AS() {  }
        virtual std::list<x0s::I*> select() = 0;
    };


    struct Read : E
    {
        Read() { }
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct Binop : E
    {
        Binop(b_ops oper, Arg* left, Arg* right) : op(oper), l(left), r(right) { }
        ~Binop() { delete l; delete r; }
        b_ops op;
        Arg* l;
        Arg* r;
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct Unop : E
    {
        Unop(u_ops oper, Arg* value) : op(oper), v(value) { }
        ~Unop() { delete v; }
        u_ops op;
        Arg* v;
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct FunCall : E
    {
        FunCall(Arg* func, std::vector<Arg*> args) : func(func), args(args) {  }
        ~FunCall() { for (auto a : args) delete a; }
        Arg* func;
        std::vector<Arg*> args;
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct Alloc : E
    {
        Alloc(int size, int tag) : size(size), tag(tag) { }
        int size;
        int tag;
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct VecRef : E
    {
        VecRef(Var* vec, int index) : vec(vec), index(index) { }
        ~VecRef() { delete vec; }
        Var* vec;
        int index;
        std::list<x0s::I*> select(x0s::Var*);
    };

    struct VecSet : E
    {
        VecSet(Var* vec, int index, Arg* asg) : vec(vec), index(index), asg(asg) { }
        ~VecSet() { delete vec; delete asg; }
        Var* vec;
        int index;
        Arg* asg;
        std::list<x0s::I*> select(x0s::Var*);
    };

    //stmt
    struct S : AS
    {
        S(std::string s, E* ee) : v(s), e(ee) { }
        ~S() { delete e; }
        std::string v;
        E* e;
        std::list<x0s::I*> select();
    };

    struct If : AS
    {
        If(std::string v, Binop* bo, Arg* thenvar, std::vector<AS*> thenstmts, Arg* elsevar, std::vector<AS*> elsestmts) :
            v(v), conde(bo), thens(thenstmts), elses(elsestmts), thenv(thenvar), elsev(elsevar) { }
        ~If() { delete conde; for (auto s : thens) delete s; for (auto s: elses) delete s; }
        std::string v;
        Binop* conde;
        std::vector<AS*> thens;
        std::vector<AS*> elses;
        Arg* thenv;
        Arg* elsev;
        std::list<x0s::I*> select();
    };

    struct F
    {
        F(std::string name,
          std::unordered_map<std::string, int> v,
          std::vector<std::string> as,
          std::vector<AS*> s,
          Arg* a, int t) :
            name(name), vars(v), args(as), stmts(s), arg(a), t(t)  { }
        std::string name;
        std::unordered_map<std::string, int> vars;
        std::vector<std::string> args;
        std::vector<AS*> stmts;
        Arg* arg; //ret
        int t;
        x0s::F select() const;
   };

    struct P
    {
        P(std::vector<F> funcs, std::string to_run, int heap_size) :
            funcs(funcs), to_run(to_run), heap_size(heap_size) { }
        ~P() { for (F &f : funcs) for (AS* s : f.stmts) delete s;
        }
        std::vector<F> funcs;
        std::string to_run;
        int heap_size;
        x0s::P select() const;
    };
}

