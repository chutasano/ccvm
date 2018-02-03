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
    return "MOVQ\t" + this->arg->to_string() + ", %rax\n" + 
           "    RETQ";
}

void P::fix()
{
    for (auto it = begin(this->instr); it != end(this->instr); ++it)
    {
        // I think it's okay to do a little hack here; only two arg needs fixing
        // so I'd need a bunch of stubs if I were to do it OO
        if (typeid(**it) == typeid(TwoArg))
        {
            auto i = static_cast<TwoArg*>(*it);
            // we need to fix two args both being memory references
            if (typeid(*(i->src)) == typeid(Mem) &&
                typeid(*(i->dst)) == typeid(Mem))
            {
                this->instr.insert(it, new TwoArg(MOVQ, i->src, new Reg("rax")));
                i->src = new Reg("rax");
                cout << "FIXING!!!!!!!!\n";
            }
        }

    }
}
