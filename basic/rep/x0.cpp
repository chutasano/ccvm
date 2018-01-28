#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "asm.h"
#include "x0.h"

using namespace std;
using namespace x0;

string X0::to_asm()
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

string reg::to_string()
{
    return "%" + this->name;
}

string con::to_string()
{
    stringstream ss;
    ss << hex << "0x" << this->val;
    return ss.str();
}

string mem::to_string()
{
    return std::to_string(this->offset) + "(%" + regname + ")";
}

string NoArg::to_asm()
{
    return i2string(this->instr);
}

string OneArg::to_asm()
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

