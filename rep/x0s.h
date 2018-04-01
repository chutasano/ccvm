#pragma once

#include <list>
#include <string>
#include <unordered_map>

#include "asm.h"
#include "graph.h"
#include "x0.h"

namespace x0s
{
    typedef std::unordered_map<std::string, std::pair<unsigned int, stack_e> > s2vmap;
    struct Arg
    {
        virtual ~Arg() {  }
        virtual x0::Arg* assign(const s2vmap&) = 0;
    };

    struct Dst : Arg
    {
    };

    struct Reg : Dst
    {
        Reg(std::string n) : name(n) { }
        // the register name without the %
        // example: rax
        std::string name;
        x0::Arg* assign(const s2vmap&);
    };

    struct Reg8 : Dst
    {
        Reg8(std::string n) : name(n) { }
        // the register name without the %
        // example: rax
        std::string name;
        x0::Arg* assign(const s2vmap&);
    };

    struct Var : Dst
    {
        Var(std::string n) : var(n) { }
        std::string var;
        x0::Arg* assign(const s2vmap&);
    };

    struct Deref : Dst
    {
        Deref(Reg* r, int offset) : reg(r), offset(offset) { }
        ~Deref() { delete reg; }
        Reg* reg;
        int offset;
        x0::Arg* assign(const s2vmap&);
    };

    struct Global : Dst
    {
        Global(std::string s) : name(s) { }
        std::string name;
        x0::Arg* assign(const s2vmap&);
    };


    struct Con : Arg
    {
        Con(int64_t c) : val(c) { }
        int64_t val;
        x0::Arg* assign(const s2vmap&);
    };
    // base class for X0 instructions
    struct I
    {
        virtual ~I() { }
        virtual std::list<x0::I*> assign(const s2vmap&) = 0;
        virtual std::list<std::string> get_vars() = 0;
    };

    struct INoArg : I
    {
        INoArg(no_arg_instr i) : instr(i) { }
        no_arg_instr instr;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct ISrc : I
    {
        ISrc(src_instr i, Arg* s) : instr(i), src(s) { }
        ~ISrc() { delete src; }
        src_instr instr;
        Arg* src;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct IDst : I
    {
        IDst(dst_instr i, Dst* d) : instr(i), dst(d) { }
        ~IDst() { delete dst; }
        dst_instr instr;
        Dst* dst;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct ISrcDst : I
    {
        ISrcDst(src_dst_instr i, Arg* s, Dst* d) : instr(i), src(s), dst(d) { }
        // FIXME (try a maybe delete implementation)
        //~ISrcDst() { delete src; delete dst; }
        src_dst_instr instr;
        Arg* src;
        Dst* dst;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct ISrcSrc : I
    {
        ISrcSrc(src_src_instr i, Arg* s, Arg* s2) : instr(i), src(s), src2(s2) { }
        ~ISrcSrc() { delete src; delete src2; }
        src_src_instr instr;
        Arg* src;
        Arg* src2;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    // group if statements for var liveliness analysis
    struct IIf : I
    {
        IIf(std::list<I*> iif, std::list<I*> ithen, std::list<I*> ielse) : ifi(iif), theni(ithen), elsei(ielse) { }
        ~IIf() { for (auto i : ifi) delete i; for (auto i : theni) delete i; for (auto i : elsei) delete i; }
        // don't need to abstract conditions here
        std::list<I*> ifi;
        std::list<I*> theni;
        std::list<I*> elsei;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct ICollect : I
    {
        ICollect() { }
        std::list<Dst*> live_references;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct IJmp : I
    {
        IJmp(jmp_instr ca, std::string l) : instr(ca), label(l) { }
        jmp_instr instr;
        std::string label;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    // abstraction of function calls
    struct ICall : I
    {
        ICall(std::string l, std::list<Arg*> args, Dst* dst) : label(l), args(args), dst(dst) { }
        ~ICall() { for (auto a : args) {delete a;} delete dst; }
        std::string label;
        std::list<Arg*> args;
        Dst* dst;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct ILabel : I
    {
        ILabel(std::string n) : name(n) { }
        std::string name;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };
    struct IRet : I
    {
        IRet(Arg* a) : arg(a) { }
        ~IRet() { delete arg; }
        Arg* arg;
        std::list<x0::I*> assign(const s2vmap&);
        std::list<std::string> get_vars();
    };

    struct F
    {
        F(std::string name, std::list<I*> i, std::unordered_map<std::string, int> v, int t) : name(name), instr(i), vars(v), t(t) { }
        std::string name;
        std::list<I*> instr;
        std::unordered_map<std::string, int> vars;
        int t;
        std::list<x0::I*> assign(bool is_default, int heap_size);
    };

    struct P
    {
        P(std::vector<F> funcs, std::string to_run, int heap_size) : funcs(funcs), to_run(to_run), heap_size(heap_size) {  }
        ~P() { for (F &f : funcs) for (auto i : f.instr) delete i; }
        std::vector<F> funcs;
        std::string to_run;
        int heap_size;
        x0::P assign();
    };

}


