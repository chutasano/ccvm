#pragma once

// Support new instructions by ES(INSTRUCTION)

#define X0_NO_ARG  \
    ES(RETQ)

#define X0_ONE_ARG \
    ES(NEGQ)       \
    ES(PUSHQ)      \
    ES(POPQ)

#define X0_TWO_ARG \
    ES(ADDQ)       \
    ES(SUBQ)       \
    ES(MOVQ)

#define ES(x) x,

namespace x0
{
    enum no_arg 
    {
        X0_NO_ARG
        NO_ARG_COUNT
    };

    // does not include callq or any other label arg calls
    enum one_arg
    {
        X0_ONE_ARG
        ONE_ARG_COUNT
    };

    enum two_arg
    {
        X0_TWO_ARG
        TWO_ARG_COUNT
    };

#undef ES
#define ES(x) #x,

    // assumes C++
    const char * const i2string(no_arg id)
    {
        return (const char *[]) {
            X0_NO_ARG
        }[id];
    }
    const char * const i2string(one_arg id)
    {
        return (const char *[]) {
            X0_ONE_ARG
        }[id];
    }
    const char * const i2string(two_arg id)
    {
        return (const char *[]) {
            X0_TWO_ARG
        }[id];
    }
}

