#pragma once

#include <string>
#include <vector>

namespace x0
{
    struct arg
    {
        virtual std::string to_string() = 0;
    };

    struct reg : arg
    {
        reg(std::string n) : name(n) { }
        // the register name without the %
        // example: rax
        std::string name;
        std::string to_string();
    };

    struct con : arg
    {
        con(int c) : val(c) { }
        int val;
        std::string to_string();
    };

    struct mem : arg
    {
        mem(std::string n, int o) : regname(n), offset(o) { }
        std::string regname;
        int offset;
        std::string to_string();
    };

    // base class for X0 instructions
    struct I
    {
        virtual std::string to_asm() = 0;
    };

    struct NoArg : I
    {
        NoArg(no_arg i) : instr(i) { }
        no_arg instr;
        std::string to_asm();
    };

    struct OneArg : I
    {
        OneArg(one_arg i, reg* d) : instr(i), dst(d) { }
        one_arg instr;
        reg* dst;
        std::string to_asm();
    };

    struct TwoArg : I
    {
        TwoArg(two_arg i, arg* s, reg* d) : instr(i), src(s), dst(d) { }
        two_arg instr;
        arg* src;
        reg* dst;
        std::string to_asm();
    };

    struct Call : I
    {
        Call(std::string l) : label(l) { }
        std::string label;
        std::string to_asm();
    };
}

// program container for X0
struct X0
{
    std::vector<x0::I*> instr;
    std::string to_asm();
};

