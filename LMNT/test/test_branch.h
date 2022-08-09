#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "testhelpers.h"
#include <stdio.h>
#include <stdbool.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif


static void test_branch(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    test_function_data fndata = { NULL, NULL };

    archive a = create_archive_array("test", 0, 1, 1, 4, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCH,    0x00, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x00), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x00) // 5
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // test "one past the end" branch
    a = create_archive_array("test", 0, 1, 1, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4040, 0x00), // 3
        LMNT_OP_BYTES(LMNT_OP_BRANCH,    0x00, 0x05, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x00), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x00)  // 5
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    a = create_archive_array("test", 0, 1, 1, 16, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x437F, 0x00), // 255
        LMNT_OP_BYTES(LMNT_OP_BRANCH,    0x00, 0x0C, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x00), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4288, 0x00), // 68
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4250, 0x00), // 52
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4210, 0x00), // 36
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x41A0, 0x00), // 20
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4080, 0x00), // 4
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x41B0, 0x00), // 22
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4218, 0x00)  // 38
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 4.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 0, 0, 0, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCH,    0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchceq(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCEQ, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCEQ,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcne(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCNE, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCNE,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchclt(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcle(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLE, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLE,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcgt(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCGT, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCGT,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcge(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCGE, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCGE,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcun(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 2, 1, 3, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCUN, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x02), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x02) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHCUN,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHZ,   0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHZ,    0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchnz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHNZ,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHNZ,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchpos(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHPOS, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHPOS,  0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchneg(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHNEG, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHNEG,  0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchun(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMP,       0x00, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHUN,  0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);

    // TODO: separate into "invalid" test suite
    a = create_archive_array("test", 1, 0, 1, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_BRANCHUN,   0x00, 0x02, 0x00)
    );

    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_ACCESS_VIOLATION);
    delete_archive_array(a);
    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}



static void test_branchceq_cmpz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMPZ,      0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCEQ, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.2f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcne_cmpz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMPZ,      0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCNE, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.1f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchclt_cmpz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMPZ,      0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.01f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcle_cmpz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMPZ,      0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLE, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.3f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -0.3f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcgt_cmpz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMPZ,      0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCGT, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_branchcge_cmpz(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    archive a = create_archive_array("test", 1, 1, 2, 5, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_CMPZ,      0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCGE, 0x00, 0x04, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x3F80, 0x01), // 1
        LMNT_OP_BYTES(LMNT_OP_RETURN,    0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x40A0, 0x01) // 5
    );
    test_function_data fndata = { NULL, NULL };
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
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assignceq(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCEQ, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -5.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigncne(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCNE, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -5.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, -0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}


static void test_assignclt(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCLT, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigncle(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCLE, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigncgt(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCGT, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigncge(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCGE, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_assigncun(void)
{
    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    archive a;
    test_function_data fndata = { NULL, NULL };

    a = create_archive_array("test", 2, 1, 3, 2, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNCUN, 0x0001, 0x0000, 0x04),
        -2.0f, 18.0f
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -6.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -5.0f, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, -4.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, INFINITY, INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -2.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), 1.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""), nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 18.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}


MAKE_REGISTER_SUITE_FUNCTION(branch,
    CUNIT_CI_TEST(test_branch),
    CUNIT_CI_TEST(test_branchz),
    CUNIT_CI_TEST(test_branchnz),
    CUNIT_CI_TEST(test_branchpos),
    CUNIT_CI_TEST(test_branchneg),
    CUNIT_CI_TEST(test_branchun),
    CUNIT_CI_TEST(test_branchceq),
    CUNIT_CI_TEST(test_branchcne),
    CUNIT_CI_TEST(test_branchclt),
    CUNIT_CI_TEST(test_branchcle),
    CUNIT_CI_TEST(test_branchcgt),
    CUNIT_CI_TEST(test_branchcge),
    CUNIT_CI_TEST(test_branchcun),
    CUNIT_CI_TEST(test_branchceq_cmpz),
    CUNIT_CI_TEST(test_branchcne_cmpz),
    CUNIT_CI_TEST(test_branchclt_cmpz),
    CUNIT_CI_TEST(test_branchcle_cmpz),
    CUNIT_CI_TEST(test_branchcgt_cmpz),
    CUNIT_CI_TEST(test_branchcge_cmpz),
    CUNIT_CI_TEST(test_branchcge_cmpz),
    CUNIT_CI_TEST(test_assignceq),
    CUNIT_CI_TEST(test_assigncne),
    CUNIT_CI_TEST(test_assignclt),
    CUNIT_CI_TEST(test_assigncle),
    CUNIT_CI_TEST(test_assigncgt),
    CUNIT_CI_TEST(test_assigncge),
    CUNIT_CI_TEST(test_assigncun)
);
