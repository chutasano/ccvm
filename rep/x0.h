#pragma once

#include <string>
#include <list>

#include "asm.h"
#include "type.h"

namespace x0
{
    struct Arg
    {
        virtual std::string to_string() = 0;
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
        std::string to_string();
    };

    // 8 bit reg
    struct Reg8 : Dst
    {
        Reg8(std::string n) : name(n) { }
        std::string name;
        std::string to_string();
    };

    struct Con : Arg
    {
        Con(int64_t c) : val(c) { }
        int64_t val;
        std::string to_string();
    };

    struct Mem : Dst
    {
        Mem(std::string n, int o) : regname(n), offset(o) { }
        std::string regname;
        int offset;
        std::string to_string();
    };

    // base class for X0 instructions
    struct I
    {
        virtual std::string to_asm() = 0;
    };

    struct Label : I
    {
        Label(std::string n) : name(n) { }
        std::string name;
        std::string to_asm();
    };

    struct NoArg : I
    {
        NoArg(no_arg i) : instr(i) { }
        no_arg instr;
        std::string to_asm();
    };

    struct OneSrc : I
    {
        OneSrc(one_src i, Arg* s) : instr(i), src(s) { }
        one_src instr;
        Arg* src;
        std::string to_asm();
    };

    struct OneDst : I
    {
        OneDst(one_dst i, Dst* d) : instr(i), dst(d) { }
        one_dst instr;
        Dst* dst;
        std::string to_asm();
    };

    struct TwoArg : I
    {
        TwoArg(two_arg i, Arg* s, Dst* d) : instr(i), src(s), dst(d) { }
        two_arg instr;
        Arg* src;
        Dst* dst;
        std::string to_asm();
    };

    struct Call : I
    {
        Call(std::string l) : label(l) { }
        std::string label;
        std::string to_asm();
    };

    struct Ret : I
    {
        Ret(type ty) : t(ty) {  }
        type t;
        std::string to_asm();
    };

    // program container for X0
    struct P
    {
        P(std::list<I*> is) : instr(is) { }
        std::list<I*> instr;
        std::string to_asm();
        void fix();
    };

}


