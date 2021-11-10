#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "lmnt/archive.h"
#include "lmnt/validation.h"
#include "lmnt/interpreter.h"
#include "lmnt/jit.h"

#include "circle.h"
#include "circle_double.h"
#include "circle_ht.h"
#include "simple125.h"

#ifdef _WIN32
#include <Windows.h>

static LARGE_INTEGER getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

static int clock_gettime(int X, struct timeval *tv)
{
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        }
        else {
            offset = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = (LONGLONG)microseconds;
    tv->tv_sec = (long)(t.QuadPart / 1000000);
    tv->tv_usec = t.QuadPart % 1000000;
    return (0);
}

static long long get_current_usec()
{
    struct timeval tv;
    clock_gettime(0, &tv);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}
#else
#include <time.h>

static long long get_current_usec()
{
    struct timespec tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return (tv.tv_sec * 1000000 + tv.tv_nsec / 1000);
}
#endif

lmnt_result double_a_thing(lmnt_ictx* ctx, const lmnt_extcall_info* callinfo, const lmnt_value* args, lmnt_value* rvals)
{
    assert(callinfo->args_count == 1 && callinfo->rvals_count == 1);
    rvals[0] = args[0] * 2.0f;
    return LMNT_OK;
}


lmnt_result hardcoded_circle_ht(float* args, size_t args_count, float* rvals, size_t rvals_count)
{
    float temp22 = fmodf(args[0] / args[3], 1.0f) * (3.14159265f * 2.0f);
    rvals[0] = cosf(temp22) * args[2];
    rvals[1] = sinf(temp22) * args[2];
    rvals[2] = 0.0f;
    rvals[3] = 1.0f;
    args[4] *= rvals[0];
    args[5] *= rvals[1];
    args[6] *= rvals[2];
    args[8] *= rvals[0];
    args[9] *= rvals[1];
    args[10] *= rvals[2];
    args[12] *= rvals[0];
    args[13] *= rvals[1];
    args[14] *= rvals[2];
    rvals[0] = args[4] + args[5] + args[6];
    rvals[1] = args[8] + args[9] + args[10];
    rvals[2] = args[12] + args[13] + args[14];
    rvals[0] += args[7];
    rvals[1] += args[11];
    rvals[2] += args[15];
    return (lmnt_result)rvals_count;
}


static void print_archive_hex(const char* archive, size_t size, size_t width)
{
    for (size_t i = 0; i < size / width; ++i) {
        LMNT_PRINTF("%04zX  |  ", i*width);
        for (size_t j = 0; j < width; ++j)
            LMNT_PRINTF("%02X ", (unsigned char)archive[i*width + j]);
        LMNT_PRINTF("\n");
    }
    if (size - (size/width)*width) {
        LMNT_PRINTF("%04zX  |  ", (size/width)*width);
        for (size_t j = 0; j < (size - (size/width)*width); ++j)
            LMNT_PRINTF("%02X ", (unsigned char)archive[(size/width)*width + j]);
        LMNT_PRINTF("\n");
    }
}


#define THE_TEST filedata_circle_ht
#define THE_TEST_NAME "circle_ht"
#define THE_TEST_DEF "CircleHT"
#define THE_TEST_ARGS { 0.0f, 0.0f, 0.05f, 0.005f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f }
#define THE_TEST_RVALS_SIZE 4
int main(int argc, char** argv)
{
    LMNT_PRINTF("interpreter dispatch method: %s\n", lmnt_get_dispatch_method());
    LMNT_PRINTF("archive size: %zu\n", sizeof(THE_TEST));

    lmnt_ictx ctx;
    char mem[8192];
    lmnt_result ir = lmnt_init(&ctx, mem, sizeof(mem));
    assert(ir == LMNT_OK);

    lmnt_extcall_info extcalls[] = {
        { "double", 1, 1, double_a_thing },
    };
    lmnt_result xr = lmnt_extcalls_set(&ctx, extcalls, 1);
    assert(xr == LMNT_OK);

    lmnt_result lr = lmnt_load_archive(&ctx, THE_TEST, sizeof(THE_TEST));
    assert(lr == LMNT_OK);

    lmnt_validation_result lvr;
    lmnt_result vr = lmnt_prepare_archive(&ctx, &lvr);
    assert(vr == LMNT_OK);
    if (vr != LMNT_OK) {
        printf("VALIDATION FAILED: %u\n", lvr);
        return 1;
    }

    const lmnt_def* def;
    lmnt_result dr = lmnt_find_def(&ctx, THE_TEST_DEF, &def);
    assert(dr == LMNT_OK);
    if (dr != LMNT_OK) {
        printf("FAILED TO FIND DEF: %s\n", THE_TEST_DEF);
        return 2;
    }

    const lmnt_code* defcode;
    const lmnt_instruction* instructions;
    lmnt_archive_get_code(&ctx.archive, def->code, &defcode);
    lmnt_archive_get_code_instructions(&ctx.archive, def->code, &instructions);

    lmnt_value c_args[] = THE_TEST_ARGS;
    lmnt_value c_rvals[THE_TEST_RVALS_SIZE];

    long long tc1 = get_current_usec();
    lmnt_jit_fn_data fndata;
    lmnt_jit_compile_stats stats;
    lmnt_result jr = lmnt_jit_compile_with_stats(&ctx, def, LMNT_JIT_TARGET_CURRENT, &fndata, &stats);
    assert(jr == LMNT_OK);
    long long tc2 = get_current_usec();
    LMNT_PRINTF("time(us) for JIT compile: %lldus\n", tc2 - tc1);
    LMNT_PRINTF("        Allocation Stats        \n");
    LMNT_PRINTF("================================\n");
    LMNT_PRINTF("   Accesses requiring alloc: %zu\n", stats.reg_alloc);
    LMNT_PRINTF("Accesses aligned and reused: %zu\n", stats.reg_aligned);
    LMNT_PRINTF("         Accesses unaligned: %zu\n", stats.reg_unaligned);
    LMNT_PRINTF("   Total register evictions: %zu\n", stats.reg_evicted);
    LMNT_PRINTF(" Evictions requiring writes: %zu\n", stats.reg_evicted_written);
    LMNT_PRINTF("\n");
    int c = 0;
    while (c != 'q' && c != 'Q' && c != EOF)
    {
        if (c != '\n' && c != '\r')
            LMNT_PRINTF("Select test: 1 = LMNT interpreted, 2 = LMNT/JIT, 3 = hard-coded C, q = quit\n");
            LMNT_PRINTF("> ");
        c = getchar();
        if (c == '\n' || c == '\r')
            continue;
        if (c == 'q' || c == 'Q' || c == EOF)
            break;
        LMNT_PRINTF("\n");
        if (c != '1' && c != '2' && c != '3')
        {
            LMNT_PRINTF("That was not one of the options. AGAIN!\n");
            continue;
        }

        lmnt_update_args(&ctx, def, 0, c_args, sizeof(c_args) / sizeof(lmnt_value));

        // lmnt_result er = lmnt_execute(&ctx, def, c_rvals, sizeof(c_rvals)/sizeof(lmnt_value));
        // lmnt_result er = lmnt_jit_execute(&ctx, fn, c_rvals, sizeof(c_rvals) / sizeof(lmnt_value));
        // lmnt_result er = hardcoded_circle_ht(ctx.writable_stack, sizeof(c_args) / sizeof(lmnt_value), c_rvals, sizeof(c_rvals) / sizeof(lmnt_value));
        // assert(er >= THE_TEST_RVALS_SIZE);
        // position = [c_rvals[0], c_rvals[1], c_rvals[2]]
        // intensity = c_rvals[3]
        // printf("%.3f: [%.3f, %.3f, %.3f] @ %.3f\n", c_args[0], c_rvals[0], c_rvals[1], c_rvals[2], c_rvals[3]);
        // getc(stdin);
        lmnt_update_args(&ctx, def, 0, c_args, sizeof(c_args) / sizeof(lmnt_value));

#define THE_TEST_ITERS 100000000

        const long itercount = THE_TEST_ITERS;
        long long t1 = 0, t2 = 0;
        if (c == '1')
        {
            t1 = get_current_usec();
            for (size_t i = 0; i < itercount; ++i)
            {
                lmnt_result er = lmnt_execute(&ctx, def, c_rvals, sizeof(c_rvals) / sizeof(lmnt_value));
                assert(er >= THE_TEST_RVALS_SIZE);
                c_args[0] += 1.0f / 10000.0f;
                lmnt_update_args(&ctx, def, 0, c_args, sizeof(c_args) / sizeof(lmnt_value));
            }
            t2 = get_current_usec();
        }
        else if (c == '2')
        {
            t1 = get_current_usec();
            for (size_t i = 0; i < itercount; ++i)
            {
                lmnt_result er = lmnt_jit_execute(&ctx, &fndata, c_rvals, sizeof(c_rvals) / sizeof(lmnt_value));
                assert(er >= THE_TEST_RVALS_SIZE);
                c_args[0] += 1.0f / 10000.0f;
                lmnt_update_args(&ctx, def, 0, c_args, sizeof(c_args) / sizeof(lmnt_value));
            }
            t2 = get_current_usec();
        }
        else if (c == '3')
        {
            t1 = get_current_usec();
            for (size_t i = 0; i < itercount; ++i)
            {
                lmnt_result er = hardcoded_circle_ht(ctx.writable_stack, sizeof(c_args) / sizeof(lmnt_value), c_rvals, sizeof(c_rvals) / sizeof(lmnt_value));
                assert(er >= THE_TEST_RVALS_SIZE);
                c_args[0] += 1.0f / 10000.0f;
                lmnt_update_args(&ctx, def, 0, c_args, sizeof(c_args) / sizeof(lmnt_value));
            }
            t2 = get_current_usec();
        }

        LMNT_PRINTF("time(us) per %ld iterations: %lldus\n", itercount, t2 - t1);
        LMNT_PRINTF("time(us) per iteration: %.3lfus\n", (double)(t2 - t1) / (double)itercount);
        LMNT_PRINTF("iterations per second: %.0lf", (1000000.0 / ((double)(t2 - t1) / itercount)));
        LMNT_PRINTF("\n");
    }

    return 0;
}
