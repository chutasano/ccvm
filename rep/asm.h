#pragma once

// Support new instructions by ES(INSTRUCTION)

#define ES(x) x,

#define X0_NO_ARG  \
    ES(RETQ)

#define X0_DST \
    ES(NEGQ)       \
    ES(POPQ)       \
    ES(SETE)       \
    ES(SETL)       \
    ES(SETG)       \
    ES(SETLE)      \
    ES(SETGE)       

#define X0_SRC \
    ES(PUSHQ)

#define X0_SRC_DST \
    ES(ADDQ)       \
    ES(SUBQ)       \
    ES(MOVQ)       \
    ES(XORQ)       \
    ES(CMPQ)       \
    ES(MOVZBQ)

#define X0_SRC_SRC 

enum no_arg_instr
{
    X0_NO_ARG
        NO_ARG_COUNT
};

enum src_instr
{
    X0_SRC
        SRC_COUNT
};

enum dst_instr
{
    X0_DST
        DST_COUNT
};

enum src_dst_instr
{
    X0_SRC_DST
        SRC_DST_COUNT
};

enum src_src_instr
{
    X0_SRC_SRC
        SRC_SRC_COUNT
};

#undef ES
#define ES(x) #x,

// assumes C++
static inline const char * const i2string(no_arg_instr id)
{
    return (const char *[]) {
        X0_NO_ARG
    }[id];
}
static inline const char * const i2string(src_instr id)
{
    return (const char *[]) {
        X0_SRC
    }[id];
}
static inline const char * const i2string(dst_instr id)
{
    return (const char *[]) {
        X0_DST
    }[id];
}
static inline const char * const i2string(src_dst_instr id)
{
    return (const char *[]) {
        X0_SRC_DST
    }[id];
}
static inline const char * const i2string(src_src_instr id)
{
    return (const char *[]) {
        X0_SRC_SRC
    }[id];
}

