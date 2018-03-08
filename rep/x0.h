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

    struct ILabel : I
    {
        ILabel(std::string n) : name(n) { }
        std::string name;
        std::string to_asm();
    };

    struct INoArg : I
    {
        INoArg(no_arg_instr i) : instr(i) { }
        no_arg_instr instr;
        std::string to_asm();
    };

    struct ISrc : I
    {
        ISrc(src_instr i, Arg* s) : instr(i), src(s) { }
        src_instr instr;
        Arg* src;
        std::string to_asm();
    };

    struct IDst : I
    {
        IDst(dst_instr i, Dst* d) : instr(i), dst(d) { }
        dst_instr instr;
        Dst* dst;
        std::string to_asm();
    };

    struct ISrcDst : I
    {
        ISrcDst(src_dst_instr i, Arg* s, Dst* d) : instr(i), src(s), dst(d) { }
        src_dst_instr instr;
        Arg* src;
        Dst* dst;
        std::string to_asm();
    };

    struct ISrcSrc : I
    {
        ISrcSrc(src_src_instr i, Arg* s, Arg* s2) : instr(i), src(s), src2(s2) { }
        src_src_instr instr;
        Arg* src;
        Arg* src2;
        std::string to_asm();
    };

    struct IJmp : I
    {
        IJmp(jmp_instr ca, std::string l) : instr(ca), label(l) { }
        jmp_instr instr;
        std::string label;
        std::string to_asm();
    };

    struct ICall : I
    {
        ICall(std::string l) : label(l) { }
        std::string label;
        std::string to_asm();
    };

    struct IRet : I
    {
        IRet(type ty) : t(ty) {  }
        type t;
        std::string to_asm();
    };

    struct Tag
    {
        Tag(std::string name, std::list<type> vals) : name(name), vals(vals) { }
        Tag(std::string name, type val) : name(name), vals({val}) { }
        std::string name;
        std::list<type> vals;
        std::string to_asm();
    };

    // program container for X0
    struct P
    {
        P(std::list<I*> is, std::list<Tag> g) : instr(is), globals(g) { }
        std::list<I*> instr;
        std::list<Tag> globals;
        std::string to_asm();
        void fix();
    };

}


