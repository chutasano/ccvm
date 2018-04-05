#pragma once

#include <map>
#include <vector>

enum type
{
    TNUM = 0,
    TBOOL,
    TVOID,
    TERROR,
    TUNKNOWN,
    TVEC, // TVEC+n will internally be used to separate different vector types
    TFUN = 0x3FFFFFFF // TFUN+n to separate different func types
};

// global should be made in r0 and populated during the type checking phase
// map from type (TVEC + n) to list of types
extern std::map<int, std::vector<int> > vec_type;
extern std::map<int, std::vector<int> > fun_type;

enum tbool
{
    TB_FALSE = 0,
    TB_TRUE = 1
};

enum tvoid
{
    TV_VOID = 0
};

static inline std::string type2name(int n)
{
    switch(n)
    {
        case TNUM:
            return "_LANG_NUM_T";
        case TBOOL:
            return "_LANG_BOOL_T";
        case TVOID:
            return "_LANG_VOID_T";
        case TVEC:
            return "_LANG_VEC_T";
        case TFUN:
            return "_LANG_FUN_T";
        default:
            if (n == TERROR || n == TUNKNOWN)
            {
                return "WTF_WRONG_TYPE";
            }
            else if (n > TVEC && n < TFUN)
            {
                return "_LANG_VEC_T_" + std::to_string(n - TVEC);
            }
            else
            {
                return "_LANG_FUN_T_" + std::to_string(n - TFUN);
            }
    }
}
