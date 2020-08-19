#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "testhelpers.h"
#include <stdio.h>
#include <stdbool.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif

static void test_addvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 2.0f, 1.0f, 2.0f,
        2.0f, 3.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 5.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1000.0f, 2000.0f, 1000.0f, 2000.0f,
        -2750.0f, -2750.0f, -2750.0f, -2750.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -1750.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -750.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -1750.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -750.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -1210.0f, -710.0f, -1210.0f, -710.0f,
        2000.0f, 2000.0f, 2000.0f, 2000.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 790.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1290.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 790.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1290.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -30.7f, -40.7f, -50.7f, -60.7f,
        -50.5f, -60.5f, -70.5f, -80.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -81.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -101.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -121.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -141.2, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        -2.0f, -2.0f, -2.0f, -2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, nanf(""), 1.0f, nanf(""),
        0.5f, 0.5f, -0.5f, -0.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.5f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.5f, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_subvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_SUBVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 2.0f, 1.0f, 2.0f,
        3.0f, 3.0f, 3.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        300.0f, 400.0f, 300.0f, 400.0f,
        -2250.0f, -2250.0f, -2150.0f, -2150.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2550.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 2650.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 2450.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 2550.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -1210.0f, -1310.0f, -1210.0f, -1310.0f,
        2000.0f, 2000.0f, 2100.0f, 2100.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -3210.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -3310.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -3310.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -3410.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -3.5f, -4.5f, -3.5f, -4.5f,
        -5.7f, -5.7f, -6.7f, -6.7f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 2.2, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        -2.0f, -3.0f, -2.0f, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 1.0f, 2.0f, 2.0f,
        nanf(""), 3.0f, nanf(""), 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_mulvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_MULVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        4.0f, 8.0f, 4.0f, 8.0f,
        2.0f, 2.0f, 4.0f, 4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 8.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 16.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 16.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 32.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        300.0f, 200.0f, 300.0f, 200.0f,
        -2.5f, -2.5f, -1.5f, -1.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -750.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -500.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -450.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -300.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -1210.0f, -1210.0f, -2420.0f, -2420.0f,
        2000.0f, 500.0f, 2000.0f, 500.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2420000.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -605000.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -4840000.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -1210000.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -3.5f, -3.5f, -7.0f, -7.0f,
        -5.7f, -7.5f, -5.7f, -7.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 19.95, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 26.25, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 39.90, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 52.50, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        5.0f, 5.0f, 0.0f, 0.0f,
        1.0f, 2.0f, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 10.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        5.0f, 10.0f, 5.0f, 10.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        -2.0f, -2.0f, -2.0f, -2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 1.0f, 1.0f, 1.0f,
        nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_divvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_DIVVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        4.0f, 4.0f, 8.0f, 8.0f,
        2.0f, 8.0f, 2.0f, 8.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.5, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 4.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        300.0f, 300.0f, 600.0f, 600.0f,
        -2.5f, -1.25f, -2.5f, -1.25f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -120.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -240.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -240.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -480.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -1210.0f, -1210.0f, -2420.0f, -2420.0f,
        2000.0f, 1000.0f, 2000.0f, 1000.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -0.605, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.210, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -1.210, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -2.420, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -3.5f, -3.5f, -7.0f, -7.0f,
        -5.7f, -4.2f, -5.7f, -4.2f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.6140350877192983, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.8333333333333333, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.2280701754385965, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.6666666666666667, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        5.0f, 5.0f, 10.0f, 10.0f,
        1.0f, 0.5f, 1.0f, 0.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 10.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 10.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 20.0, FLOAT_ERROR_MARGIN);

    // Div-by-zero
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        5.0f, 0.0f, 10.0f, -5.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isinf(rvals[2]));
    CU_ASSERT_TRUE(isinf(rvals[3]));

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), 0.0f, nanf(""), 1.0f,
        -2.0f, -4.0f, -6.0f, -8.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -0.125, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 1.0f, 2.0f, 2.0f,
        nanf(""), 1.0f, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
    nanf(""), nanf(""), nanf(""), nanf(""),
    nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_modvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_MODVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 5.0f, 3.0f, 5.0f,
        2.0f, 2.0f, 2.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        300.0f, 550.0f, 700.0f, 950.0f,
        100.0f, 100.0f, 100.0f, 100.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 50.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 50.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        -5.1f, -7.1f, -5.1f, -7.1f,
        3.0f, 3.0f, 3.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -2.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -1.1, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
    -5.1f, -7.1f, -5.1f, -7.1f,
    -3.0f, -3.0f, -3.0f, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -1.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -2.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -1.1, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 2.0f, 4.0f, 5.0f,
        -3.0f, -3.0f, -3.0f, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        2.5f, 3.5f, 2.5f, 3.5f,
        1.4f, 1.4f, 1.2f, 1.2f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.7, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.1, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.1, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
    0.0f, 3.5f, 0.0f, 4.5f,
    3.5f, 3.5f, 4.5f, 4.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 4.0f, 5.0f, 6.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), 1.0f, 1.0f,
        1.0f, 2.0f, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_powvv(void)
{
    archive a = create_archive_array("test", 8, 4, 12, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_POWVV, 0x00, 0x04, 0x08)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        3.0f, 3.0f, 3.0f, 3.0f,
        1.0f, 2.0f, 3.0f, 4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 9.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 27.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 81.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 3.5f, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, 1.5f, 3.2f, 5.1f,
        2.0f, 1.5f, 3.0f, 1.4f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 2.25, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.8371173070873836, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 32.768, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 9.785843060783371, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        5.0f, 5.0f, 5.0f, 5.0f,
        0.0f, -1.0f, -2.0f, -5.5f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.2, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.04, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.00014310835055998653, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        1.0f, 0.0f, 4.0f, -3.4f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_powvs(void)
{
    archive a = create_archive_array("test", 5, 4, 9, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_POWVS, 0x00, 0x04, 0x05)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 1.0f, 2.0f, 3.0f,
        3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 8.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 27.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 1.0f, 5.0f, 10.0f,
        0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 5.0f, 10.0f, 100.0f,
        -2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.04, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.01, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0001, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, 2.5f, 3.2f, 5.1f,
        1.4f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.7641185337870102, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 3.6067497647680336, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 5.095771783084765, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 9.785843060783371, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        5.0f, 3.3f, 1.0f, 0.1f,
        -1.7f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0648262638677105, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.13137910597720742, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 50.118723362727216, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""),
        4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_sqrtv(void)
{
    archive a = create_archive_array("test", 4, 4, 8, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_SQRTV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        0.0f, 1.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.4142135623730951, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.7320508075688772, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.0f, 5.0f, 10.0f, 100.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 2.23606797749979, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 3.1622776601683795, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 10.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0,
        1.5f, 2.5f, 3.2f, 5.1f)
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.224744871391589, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.5811388300841898, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.7888543819998317, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 2.258317958127243, FLOAT_ERROR_MARGIN);

    // NaN
    TEST_UPDATE_ARGS(ctx, fndata, 0,
        nanf(""), nanf(""), nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_sumv(void)
{
    archive a = create_archive_array("test", 4, 1, 5, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_SUMV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata = { NULL, NULL };
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


MAKE_REGISTER_SUITE_FUNCTION(maths_vector,
    CUNIT_CI_TEST(test_addvv),
    CUNIT_CI_TEST(test_subvv),
    CUNIT_CI_TEST(test_mulvv),
    CUNIT_CI_TEST(test_divvv),
    CUNIT_CI_TEST(test_modvv),
    CUNIT_CI_TEST(test_powvv),
    CUNIT_CI_TEST(test_powvs),
    CUNIT_CI_TEST(test_sqrtv),
    CUNIT_CI_TEST(test_sumv)
);
