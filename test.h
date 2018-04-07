#pragma once

#include "rep/type.h"

struct vec_t
{
    vec_t(type t, int64_t val) : t(t), val(val) { }
    type t;
    // val -> raw value for num/bool/void
    // size for vector
    int64_t val;
};

void test_all(bool);

