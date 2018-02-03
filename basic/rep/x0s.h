#pragma once

#include <string>
#include <vector>

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

    struct Var : Dst
    {
        Var(std::string n) : var(n) { }
        std::string var;
        x0::Arg* assign();
    };

    struct Con : Arg
    {
        Con(int c) : val(c) { }
        int val;
        x0::Arg* assign();
    };

    // base class for X0 instructions
    struct I
    {
        virtual x0::I* assign() = 0;
    };

    struct NoArg : I
    {
        NoArg(no_arg i) : instr(i) { }
        no_arg instr;
        x0::I* assign();
    };

    struct OneSrc : I
    {
        OneSrc(one_src i, Arg* s) : instr(i), src(s) { }
        one_src instr;
        Arg* src;
        x0::I* assign();
    };

    struct OneDst : I
    {
        OneDst(one_dst i, Dst* d) : instr(i), dst(d) { }
        one_dst instr;
        Dst* dst;
        x0::I* assign();
    };

    struct TwoArg : I
    {
        TwoArg(two_arg i, Arg* s, Dst* d) : instr(i), src(s), dst(d) { }
        two_arg instr;
        Arg* src;
        Dst* dst;
        x0::I* assign();
    };

    struct Call : I
    {
        Call(std::string l) : label(l) { }
        std::string label;
        x0::I* assign();
    };

    struct Ret : I
    {
        Ret(Arg* a) : arg(a) { }
        Arg* arg;
        x0::I* assign();
    };

    // program container for X0
    struct P
    {
        P(std::vector<I*> i, std::vector<std::string> v) : instr(i), vars(v) { }
        std::vector<I*> instr;
        std::vector<std::string> vars;
        x0::P assign();
    };

}


