#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "helpers.h"
#include <stdio.h>
#include <stdbool.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif


static void test_abss(void)
{
    archive a = create_archive_array("test", 1, 1, 2, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_ABSS, 0x00, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -458.35f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 458.35, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && !signbit(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_absv(void)
{
    archive a = create_archive_array("test", 4, 4, 8, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_ABSV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, -1.3f, -458.35f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.8, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.3, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 458.35, FLOAT_ERROR_MARGIN);
    CU_ASSERT_EQUAL(rvals[3], 0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f, nanf(""), -INFINITY, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]) && !signbit(rvals[2]));
    CU_ASSERT_EQUAL(rvals[3], 0.0);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_sumv(void)
{
    archive a = create_archive_array("test", 4, 1, 5, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_SUMV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, -1.3f, -458.35f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -457.85, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 31.8f, -1.3f, 40.3f, 40.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 110.8, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -0.0f, -0.0f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, -1.3f, nanf(""), -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, INFINITY, -458.35f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && !signbit(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_floors(void)
{
    archive a = create_archive_array("test", 1, 1, 2, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_FLOORS, 0x00, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.3f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -458.35f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -459.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.1f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_floorv(void)
{
    archive a = create_archive_array("test", 4, 4, 8, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_FLOORV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, -1.3f, -458.35f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -459.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.3f, nanf(""), -INFINITY, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -4.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]) && signbit(rvals[2]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_rounds(void)
{
    archive a = create_archive_array("test", 1, 1, 2, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_ROUNDS, 0x00, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.3f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -458.35f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -458.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.1f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_roundv(void)
{
    archive a = create_archive_array("test", 4, 4, 8, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_ROUNDV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, -1.3f, -458.75f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -459.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.3f, nanf(""), -INFINITY, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -3.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]) && signbit(rvals[2]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.5f, -2.5, 1.5, 4.5);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -4.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -2.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 2.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 4.0f, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_ceils(void)
{
    archive a = create_archive_array("test", 1, 1, 2, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_CEILS, 0x00, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.3f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -458.35f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -458.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.1f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_ceilv(void)
{
    archive a = create_archive_array("test", 4, 4, 8, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_CEILV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.8f, -1.3f, -458.75f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -458.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.3f, nanf(""), -INFINITY, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -3.0f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]) && signbit(rvals[2]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_minss(void)
{
    archive a = create_archive_array("test", 2, 1, 3, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_MINSS, 0x00, 0x01, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f, -2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -458.35f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -458.35, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], -0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && signbit(rvals[0]));

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0f, FLOAT_ERROR_MARGIN);

    // min/max are undefined for NaN values

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_minvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_MINVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 3.0f, -3.0f, 3.0f,
        1.0f, 2.0f, 3.0f, 4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 3.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 0.0f, 0.0f, 0.0f,
        -0.0f, 1.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -0.0f, 1.0f, 0.0f, -1.0f,
        0.0f, -1.0f, 0.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2],  0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, 1.5f, 3.2f, -INFINITY,
        2.0f, INFINITY, 3.0f, 1.4f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isinf(rvals[3]) && signbit(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_maxss(void)
{
    archive a = create_archive_array("test", 2, 1, 3, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_MAXSS, 0x00, 0x01, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f, -2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -458.35f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0f, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && !signbit(rvals[0]));

    // min/max are undefined for NaN values

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_maxvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_MAXVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 3.0f, -3.0f, 3.0f,
        1.0f, 2.0f, 3.0f, 4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 4.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 0.0f, 0.0f, 0.0f,
        -0.0f, -1.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 3.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -0.0f, 1.0f, 0.0f, -1.0f,
        0.0f, -1.0f, 0.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 3.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, 1.5f, 3.2f, -INFINITY,
        2.0f, INFINITY, 3.0f, 1.4f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isinf(rvals[1]) && !signbit(rvals[1]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.4, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_minvs(void)
{
    archive a = create_archive_array("test", 5, 4, 9, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_MINVS, 0x00, 0x04, 0x05)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 0.0f, -3.0f, 0.4f,
        1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.4, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 1.0f, -1.0f, -0.0f,
        0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_EQUAL(rvals[3], -0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, -1.5f, 3.2f, -INFINITY,
        -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && signbit(rvals[0]));
    CU_ASSERT_TRUE(isinf(rvals[1]) && signbit(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]) && signbit(rvals[2]));
    CU_ASSERT_TRUE(isinf(rvals[3]) && signbit(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, -1.5f, 3.2f, -INFINITY,
        INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isinf(rvals[3]) && signbit(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_maxvs(void)
{
    archive a = create_archive_array("test", 5, 4, 9, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_MAXVS, 0x00, 0x04, 0x05)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 0.0f, -3.0f, 0.4f,
        1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 1.0f, -1.0f, -0.0f,
        -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_EQUAL(rvals[2], -0.0);
    CU_ASSERT_EQUAL(rvals[3], -0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, -1.5f, 3.2f, -INFINITY,
        -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isinf(rvals[3]) && signbit(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, 1.5f, 3.2f, -INFINITY,
        INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && !signbit(rvals[0]));
    CU_ASSERT_TRUE(isinf(rvals[1]) && !signbit(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]) && !signbit(rvals[2]));
    CU_ASSERT_TRUE(isinf(rvals[3]) && !signbit(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}



MAKE_REGISTER_SUITE_FUNCTION(util,
    CUNIT_CI_TEST(test_abss),
    CUNIT_CI_TEST(test_absv),
    CUNIT_CI_TEST(test_sumv),
    CUNIT_CI_TEST(test_floors),
    CUNIT_CI_TEST(test_floorv),
    CUNIT_CI_TEST(test_rounds),
    CUNIT_CI_TEST(test_roundv),
    CUNIT_CI_TEST(test_ceils),
    CUNIT_CI_TEST(test_ceilv),
    CUNIT_CI_TEST(test_minss),
    CUNIT_CI_TEST(test_minvv),
    CUNIT_CI_TEST(test_maxss),
    CUNIT_CI_TEST(test_maxvv),
    CUNIT_CI_TEST(test_minvs),
    CUNIT_CI_TEST(test_maxvs)
);