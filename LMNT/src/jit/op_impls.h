#ifndef LMNT_JIT_OP_IMPLS_H
#define LMNT_JIT_OP_IMPLS_H

#include <math.h>

static float remss(float x, float y)
{
    return x - floorf(x / y) * y;
}

static float sinrf(float x)
{
    return sinf(x * ((float)(M_PI * 2.)));
}

static float cosrf(float x)
{
    return cosf(x * ((float)(M_PI * 2.)));
}

static float tanrf(float x)
{
    return tanf(x * ((float)(M_PI * 2.)));
}

static float asinrf(float f)
{
    return asinf(f) * ((float)(1. / (M_PI * 2.)));
}

static float acosrf(float f)
{
    return acosf(f) * ((float)(1. / (M_PI * 2.)));
}

static float atanrf(float f)
{
    return atanf(f) * ((float)(1. / (M_PI * 2.)));
}

static float atan2rf(float fy, float fx)
{
    return atan2f(fy, fx) * ((float)(1. / (M_PI * 2.)));
}

#endif
