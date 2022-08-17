#ifndef LMNT_JIT_OP_IMPLS_H
#define LMNT_JIT_OP_IMPLS_H

#include <math.h>

static float remss(float x, float y)
{
    return x - floorf(x / y) * y;
}

#endif