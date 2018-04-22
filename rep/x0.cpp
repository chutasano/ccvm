#include <iomanip>
#include <iostream>
#include <typeinfo>
#include <sstream>
#include <string>
#include <vector>

#include "asm.h"
#include "x0.h"

using namespace std;
using namespace x0;

string P::to_asm()
{
    stringstream ss;
    ss << ".section .data" << endl;
    for (const Tag &t : globals)
    {
        ss << "    " << ".globl " << t.name << endl
           << "    " << t.name << ":" << endl
           << t.to_asm() << endl;
    }
    ss  << ".section .text" << endl;
    for (auto i : instr)
    {
        ss << "    " << i->to_asm() << endl;
    }
    return ss.str();
}

string Reg::to_string()
{
    return "%" + this->name;
}

string Reg8::to_string()
{
    return "%" + this->name;
}

string Con::to_string()
{
    stringstream ss;
    ss << hex << "$0x" << this->val;
    return ss.str();
}

string Mem::to_string()
{
    if (this->offset == 0)
    {
        return "(%" + regname + ")";
    }
    else
    {
        return std::to_string(this->offset) + "(%" + regname + ")";
    }
}

string Global::to_string()
{
    return name;
}

string ILabel::to_asm()
{
    string ret = "";
    if (is_global)
    {
        ret = "\n.globl " + name + "\n";
    }
    ret += name + ":";
    return ret;
}

string INoArg::to_asm()
{
    return i2string(this->instr);
}

string ISrc::to_asm()
{
    return i2string(this->instr) + string("\t") + this->src->to_string();
}

string IDst::to_asm()
{
    return i2string(this->instr) + string("\t") + this->dst->to_string();
}

string ISrcDst::to_asm()
{
    return i2string(this->instr) + string("\t") + this->src->to_string() + ", " + this->dst->to_string();
}

string ISrcSrc::to_asm()
{
    return i2string(this->instr) + string("\t") + this->src->to_string() + ", " + this->src2->to_string();
}

string IJmp::to_asm()
{
    return i2string(this->instr) + string("\t") + this->label;
}

string ICall::to_asm()
{
    string ret =  "CALLQ\t";
    if (typeid(*f) == typeid(Reg))
    {
        ret += "*";
    }
    ret += f->to_string();
    return ret;
}

string IRet::to_asm()
{
    // prints out the final value because simply returning
    // won't give us 64 bits (Linux gives 8 bits)
    // this should be processed by the callee of the program
    // in automated tests
    stringstream ss;
    if (print)
    {
        ss << "MOVQ\t%rax, %rdi\n";
        type ty = static_cast<type>(t);
        switch (ty)
        {
            case TNUM:
                ss << "    CALLQ\t_lang_print_num\n";
                break;
            case TBOOL:
                ss << "    CALLQ\t_lang_print_bool\n";
                break;
            case TVOID:
                ss << "    CALLQ\t_lang_print_void\n";
                break;
            default:
                if (t > TVEC && t < TFUN)
                {
                    ss << "    CALLQ\t_lang_print_vec\n";
                    break;
                }
                else
                {
                    cerr <<"WTF??? got unknown type: " << t << "\n";
                    exit(2);
                }
        }
        ss << "    ";
    }
    ss << "RETQ";
    return ss.str();
}

void P::fix()
{
    // can't ++it in the for loop because erase will advance iterator
    for (auto it = begin(this->instr); it != end(this->instr);)
    {
        if (typeid(**it) == typeid(ISrcDst))
        {
            auto i = static_cast<ISrcDst*>(*it);
            if (i->instr == MOVQ &&
                    typeid(*(i->src)) == typeid(Reg) &&
                    typeid(*(i->dst)) == typeid(Reg) &&
                    (static_cast<Reg*>(i->src))->name == (static_cast<Reg*>(i->dst))->name)
            {
                it = this->instr.erase(it);
            }
            else
            {
                if (i->instr == MOVZBQ &&
                        typeid(*(i->dst)) == typeid(Mem))
                {
                    Dst* local_dst = i->dst;
                    i->dst = new Reg("rax");
                    it = this->instr.insert(++it, new ISrcDst(MOVQ, i->dst, local_dst));
                }
                if (i->instr == LEAQ &&
                        typeid(*(i->src)) == typeid(Global) &&
                        typeid(*(i->dst)) == typeid(Mem))
                {
                    ISrcDst* leaq = new ISrcDst(LEAQ, i->src, new Reg("rax"));
                    i->src = new Reg("rax");
                    i->instr = MOVQ;
                    it = this->instr.insert(it, leaq);
                }
                // we need to fix two args both being memory references
                else if (typeid(*(i->src)) == typeid(Mem) &&
                        typeid(*(i->dst)) == typeid(Mem))
                {
                    ISrcDst* movq = new ISrcDst(MOVQ, i->src, new Reg("rax"));
                    i->src = new Reg("rax");
                    it = this->instr.insert(it, movq);
                }
                ++it;
            }
        }
        else if (typeid(**it) == typeid(ICall))
        {
            auto c = static_cast<ICall*>(*it);
            if (typeid(*(c->f)) == typeid(Mem))
            {
                ISrcDst* leaq = new ISrcDst(LEAQ, c->f, new Reg("rax"));
                c->f = new Reg("rax");
                it = this->instr.insert(it, leaq);
            }
            ++it;
        }
        else
        {
            ++it;
        }
    }
}

string Tag::to_asm() const
{
    stringstream ss;
    for (auto v : vals)
    {
        ss << "        .quad " << v << endl;
    }
    return ss.str();
}

