#pragma once

#include <list>
#include <string>

#include "asm.h"
#include "x0.h"

namespace x0s
{
    struct Arg
    {
        virtual x0::Arg* assign() = 0;
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
        x0::Arg* assign();
    };

    struct Reg8 : Dst
    {
        Reg8(std::string n) : name(n) { }
        // the register name without the %
        // example: rax
        std::string name;
        x0::Arg* assign();
    };

    struct Var : Dst
    {
        Var(std::string n) : var(n) { }
        std::string var;
        x0::Arg* assign();
    };

    struct Con : Arg
    {
        Con(int64_t c) : val(c) { }
        int64_t val;
        x0::Arg* assign();
    };

    // base class for X0 instructions
    struct I
    {
        virtual std::list<x0::I*> assign() = 0;
        virtual std::list<std::string> get_vars() = 0;
    };

    struct INoArg : I
    {
        INoArg(no_arg_instr i) : instr(i) { }
        no_arg_instr instr;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    struct ISrc : I
    {
        ISrc(src_instr i, Arg* s) : instr(i), src(s) { }
        src_instr instr;
        Arg* src;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    struct IDst : I
    {
        IDst(dst_instr i, Dst* d) : instr(i), dst(d) { }
        dst_instr instr;
        Dst* dst;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    struct ISrcDst : I
    {
        ISrcDst(src_dst_instr i, Arg* s, Dst* d) : instr(i), src(s), dst(d) { }
        src_dst_instr instr;
        Arg* src;
        Dst* dst;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    struct ISrcSrc : I
    {
        ISrcSrc(src_src_instr i, Arg* s, Arg* s2) : instr(i), src(s), src2(s2) { }
        src_src_instr instr;
        Arg* src;
        Arg* src2;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    // group if statements for var liveliness analysis
    struct IIf : I
    {
        IIf(std::list<I*> iif, std::list<I*> ithen, std::list<I*> ielse) : ifi(iif), theni(ithen), elsei(ielse) { }
        // don't need to abstract conditions here
        std::list<I*> ifi;
        std::list<I*> theni;
        std::list<I*> elsei;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    struct IJmp : I
    {
        IJmp(jmp_instr ca, std::string l) : instr(ca), label(l) { }
        jmp_instr instr;
        std::string label;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    // abstraction of function calls
    struct ICall : I
    {
        ICall(std::string l, std::list<Arg*> args, Var* dst) : label(l), args(args), dst(dst) { }
        std::string label;
        std::list<Arg*> args;
        Var* dst;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    struct ILabel : I
    {
        ILabel(std::string n) : name(n) { }
        std::string name;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };
    struct IRet : I
    {
        IRet(Arg* a) : arg(a) { }
        Arg* arg;
        std::list<x0::I*> assign();
        std::list<std::string> get_vars();
    };

    // program container for X0
    struct P
    {
        P(std::list<I*> i, std::vector<std::string> v, type ty) : instr(i), vars(v), t(ty) { }
        std::list<I*> instr;
        std::vector<std::string> vars;
        type t;
        x0::P assign();
        private:
        void interference();
    };

}


