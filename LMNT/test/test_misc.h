#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "helpers.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif

static void test_assignss(void)
{
    archive a = create_archive_array("test", 1, 1, 2, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNSS, 0x00, 0x00, 0x01)
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
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assignvv(void)
{
    archive a = create_archive_array("test", 4, 4, 8, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNVV, 0x00, 0x00, 0x04)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f, -1.0f, -2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 2.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 0.0f, -0.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_EQUAL(rvals[1], 0.0);
    CU_ASSERT_EQUAL(rvals[2], -0.0);
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assignsv(void)
{
    archive a = create_archive_array("test", 1, 4, 5, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNSV, 0x00, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 1.0);
    CU_ASSERT_EQUAL(rvals[1], 1.0);
    CU_ASSERT_EQUAL(rvals[2], 1.0);
    CU_ASSERT_EQUAL(rvals[3], 1.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);
    CU_ASSERT_EQUAL(rvals[1], 0.0);
    CU_ASSERT_EQUAL(rvals[2], 0.0);
    CU_ASSERT_EQUAL(rvals[3], 0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], -0.0);
    CU_ASSERT_EQUAL(rvals[1], -0.0);
    CU_ASSERT_EQUAL(rvals[2], -0.0);
    CU_ASSERT_EQUAL(rvals[3], -0.0);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));
    CU_ASSERT_TRUE(isnan(rvals[1]));
    CU_ASSERT_TRUE(isnan(rvals[2]));
    CU_ASSERT_TRUE(isnan(rvals[3]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigniis(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata;

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x002A, 0x0000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 42.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0xFFD6, 0xFFFF, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -42.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x0000, 0x0000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x58CB, 0xFFA9, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -5678901.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assignibs(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata;

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x3D71, 0x422A, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 42.56, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x0000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x8000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], -0.0);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 1, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x4E6A, 0xCAAD, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -5678901.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigniiv(void)
{
    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata;

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIV, 0x002A, 0x0000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 42.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 42.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 42.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 42.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIV, 0xFFD6, 0xFFFF, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -42.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -42.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -42.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -42.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIV, 0x0000, 0x0000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 0.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 0.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIV, 0x58CB, 0xFFA9, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -5678901.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -5678901.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -5678901.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -5678901.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assignibv(void)
{
    lmnt_value rvals[4];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata;

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBV, 0x0000, 0x3F80, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 1.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBV, 0x3D71, 0x422A, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 42.56, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], 42.56, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], 42.56, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], 42.56, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBV, 0x0000, 0x0000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], 0.0);
    CU_ASSERT_EQUAL(rvals[1], 0.0);
    CU_ASSERT_EQUAL(rvals[2], 0.0);
    CU_ASSERT_EQUAL(rvals[3], 0.0);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBV, 0x0000, 0x8000, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_EQUAL(rvals[0], -0.0);
    CU_ASSERT_EQUAL(rvals[1], -0.0);
    CU_ASSERT_EQUAL(rvals[2], -0.0);
    CU_ASSERT_EQUAL(rvals[3], -0.0);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    a = create_archive_array("test", 0, 4, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBV, 0x4E6A, 0xCAAD, 0x00)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -5678901.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[1], -5678901.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[2], -5678901.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(rvals[3], -5678901.0, FLOAT_ERROR_MARGIN);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_indexris(void)
{
    archive a = create_archive_array("test", 1, 1, 2, 1, 0, 4,
        LMNT_OP_BYTES(LMNT_OP_INDEXRIS, 0x04, 0x00, 0x05),
        350.0f, 700.0f, -100.0f, 50.0f
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 350.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -100.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 8.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_indexrir(void)
{
    archive a = create_archive_array("test", 2, 1, 4, 1, 0, 4,
        LMNT_OP_BYTES(LMNT_OP_INDEXRIR, 0x04, 0x00, 0x05),
        350.0f, 700.0f, -100.0f, 50.0f
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 350.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 2.0f, 6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -100.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f, 6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 10.0f, 6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 16.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, 6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), LMNT_ERROR_ACCESS_VIOLATION);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}



MAKE_REGISTER_SUITE_FUNCTION(misc,
    CUNIT_CI_TEST(test_assignss),
    CUNIT_CI_TEST(test_assignvv),
    CUNIT_CI_TEST(test_assignsv),
    CUNIT_CI_TEST(test_assigniis),
    CUNIT_CI_TEST(test_assignibs),
    CUNIT_CI_TEST(test_assigniiv),
    CUNIT_CI_TEST(test_assignibv),
    CUNIT_CI_TEST(test_indexris),
    CUNIT_CI_TEST(test_indexrir)
);
