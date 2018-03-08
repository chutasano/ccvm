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
    TVEC // TVEC should be the last one, TVEC+n will internally be used to separate
         // different vector types
};

// global should be made in r0 and populated during the type checking phase
// map from type (TVEC + n) to list of types
extern std::map<int, std::vector<int> > vec_type;

enum tbool
{
    TB_FALSE = 0,
    TB_TRUE = 1
};

