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
    ss  << ".globl main" << endl
        << "main:" << endl;
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

string NoArg::to_asm()
{
    return i2string(this->instr);
}

string OneSrc::to_asm()
{
    return i2string(this->instr) + string("\t") + this->src->to_string();
}

string OneDst::to_asm()
{
    return i2string(this->instr) + string("\t") + this->dst->to_string();
}

string TwoArg::to_asm()
{
    return i2string(this->instr) + string("\t") + this->src->to_string() + ", " + this->dst->to_string();
}

string Call::to_asm()
{
    return "CALLQ\t" + this->label;
}

string Ret::to_asm()
{
    // prints out the final value because simply returning
    // won't give us 64 bits (Linux gives 8 bits)
    // this should be processed by the callee of the program
    // in automated tests
    stringstream ss;
    ss << "MOVQ\t%rax, %rdi\n";
    switch (this->t)
    {
        case NUM:
            ss << "    CALLQ\t_lang_print_num\n";
            break;
        case BOOL:
            ss << "    CALLQ\t_lang_print_bool\n";
            break;
        default:
            cerr << "WTF\n"; //sanity check
            exit(1);
            break;
    }
    ss << "    RETQ";
    return ss.str();
}

void P::fix()
{
    // can't ++it in the for loop because erase will advance iterator
    for (auto it = begin(this->instr); it != end(this->instr);)
    {
        // I think it's okay to do a little hack here; only TwoArg needs fixing
        // so I'd need a bunch of stubs if I were to do it OO
        if (typeid(**it) == typeid(TwoArg))
        {
            auto i = static_cast<TwoArg*>(*it);
            if (i->instr == MOVQ &&
                    typeid(*(i->src)) == typeid(Reg) &&
                    typeid(*(i->dst)) == typeid(Reg) &&
                    (static_cast<Reg*>(i->src))->name == (static_cast<Reg*>(i->dst))->name)
            {
                it = this->instr.erase(it);
            }
            else
            {
                // we need to fix two args both being memory references
                if (typeid(*(i->src)) == typeid(Mem) &&
                        typeid(*(i->dst)) == typeid(Mem))
                {
                    TwoArg* movq = new TwoArg(MOVQ, i->src, new Reg("rax"));
                    i->src = new Reg("rax");
                    it = this->instr.insert(it, movq);
                }
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}
