#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "helpers.h"
#include <stdio.h>
#include <stdbool.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif


static void test_branch(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 0, 1, 1, 4, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCH,    0x00, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x00)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 0, 0, 0, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCH,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_brancheq(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHEQ,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHEQ,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchne(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHNE,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHNE,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchlt(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHLT,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 3.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHLT,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchle(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHLE,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 3.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHLE,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchgt(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHGT,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 3.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHGT,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchge(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHGE,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 3.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHGE,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchun(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHUN,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x02)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHUN,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHZ,   0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x01),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHZ,    0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchnz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHNZ,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x01),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHNZ,   0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchpos(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHPOS, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x01),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHPOS,  0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchneg(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHNEG, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x01, 0x00, 0x01),
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x05, 0x00, 0x01)
    );
    test_function_data fndata;
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHNEG,  0x00, 0x01, 0x00)
    );

    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);
    {
        lmnt_validation_result vr;
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_ERROR_INVALID_ARCHIVE);
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VERROR_ACCESS_VIOLATION);
    }
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}



MAKE_REGISTER_SUITE_FUNCTION(branch,
    CUNIT_CI_TEST(test_branch),
    CUNIT_CI_TEST(test_brancheq),
    CUNIT_CI_TEST(test_branchne),
    CUNIT_CI_TEST(test_branchlt),
    CUNIT_CI_TEST(test_branchle),
    CUNIT_CI_TEST(test_branchgt),
    CUNIT_CI_TEST(test_branchge),
    CUNIT_CI_TEST(test_branchun),
    CUNIT_CI_TEST(test_branchz),
    CUNIT_CI_TEST(test_branchnz),
    CUNIT_CI_TEST(test_branchpos),
    CUNIT_CI_TEST(test_branchneg)
);