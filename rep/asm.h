#pragma once

// Support new instructions by ES(INSTRUCTION)

#define ES(x) x,

#define X0_NO_ARG  \
    ES(RETQ)

#define X0_ONE_DST \
    ES(NEGQ)       \
    ES(POPQ)

#define X0_ONE_SRC \
    ES(PUSHQ)

#define X0_TWO_ARG \
    ES(ADDQ)       \
    ES(SUBQ)       \
    ES(MOVQ)


enum no_arg 
{
    X0_NO_ARG
        NO_ARG_COUNT
};

enum one_src
{
    X0_ONE_SRC
        ONE_SRC_COUNT
};

enum one_dst
{
    X0_ONE_DST
        ONE_DST_COUNT
};

enum two_arg
{
    X0_TWO_ARG
        TWO_ARG_COUNT
};

#undef ES
#define ES(x) #x,

// assumes C++
static inline const char * const i2string(no_arg id)
{
    return (const char *[]) {
        X0_NO_ARG
    }[id];
}
static inline const char * const i2string(one_src id)
{
    return (const char *[]) {
        X0_ONE_SRC
    }[id];
}
static inline const char * const i2string(one_dst id)
{
    return (const char *[]) {
        X0_ONE_DST
    }[id];
}
static inline const char * const i2string(two_arg id)
{
    return (const char *[]) {
        X0_TWO_ARG
    }[id];
}

