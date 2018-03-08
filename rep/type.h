#pragma once

#include <list>
#include <unordered_map>

enum type
{
    TNUM = 0,
    TBOOL,
    TVOID,
    TERROR,
    TVEC // TVEC should be the last one, TVEC+n will internally be used to separate
         // different vector types
};

// global should be made in r0 and populated during the type checking phase
// map from type (TVEC + n) to list of types
extern std::unordered_map<type, std::list<type>, std::hash<int> > vec_type;

enum tbool
{
    TB_FALSE = 0,
    TB_TRUE = 1
};

