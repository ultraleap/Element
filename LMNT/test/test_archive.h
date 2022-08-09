#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "testhelpers.h"
#include <stdio.h>
#include <stdbool.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif


static void test_archive_backbranches(void)
{
    // def correctly claims to have backbranches
    archive a = create_archive_array_with_flags("test", LMNT_DEFFLAG_HAS_BACKBRANCHES, 1, 1, 2, 4, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x02, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4180, 0x03), // 16
        1.0, 5.0
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 16.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // def has backbranches but claims not to
    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_NONE, 1, 1, 2, 4, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x02, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4180, 0x03), // 16
        1.0, 5.0
    );
    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_DEF_FLAGS);
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // def has forwards branch and claims not to have backbranches
    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_NONE, 1, 1, 2, 4, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x02, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIBS, 0x0000, 0x4180, 0x03), // 16
        1.0, 5.0
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 16.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}


static void test_archive_default_args(void)
{
    archive a = create_archive_array_with_flags("test", LMNT_DEFFLAG_HAS_DEFAULT_ARGS, 2, 1, 3, 1, 2, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x00, 0x01, 0x02),
        1.0, 5.0
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    const lmnt_value* defargs = NULL;
    lmnt_loffset defargscount = 0;
    CU_ASSERT_EQUAL(lmnt_get_default_args(ctx, fndata.def, &defargs, &defargscount), LMNT_OK);
    CU_ASSERT_PTR_NOT_NULL(defargs);
    CU_ASSERT_EQUAL(defargscount, 2);
    CU_ASSERT_DOUBLE_EQUAL(defargs[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(defargs[1], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // def has additional elements in data section used as default args
    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_HAS_DEFAULT_ARGS, 2, 1, 3, 1, 3, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x00, 0x01, 0x02),
        1.0, 5.0, 3.0
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    defargs = NULL;
    defargscount = 0;
    CU_ASSERT_EQUAL(lmnt_get_default_args(ctx, fndata.def, &defargs, &defargscount), LMNT_OK);
    CU_ASSERT_PTR_NOT_NULL(defargs);
    CU_ASSERT_EQUAL(defargscount, 2);
    CU_ASSERT_DOUBLE_EQUAL(defargs[0], 1.0, FLOAT_ERROR_MARGIN);
    CU_ASSERT_DOUBLE_EQUAL(defargs[1], 5.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // def has partial set of default args
    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_HAS_DEFAULT_ARGS, 2, 1, 3, 1, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x00, 0x01, 0x02),
        1.0
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    defargs = NULL;
    defargscount = 0;
    CU_ASSERT_EQUAL(lmnt_get_default_args(ctx, fndata.def, &defargs, &defargscount), LMNT_OK);
    CU_ASSERT_PTR_NOT_NULL(defargs);
    CU_ASSERT_EQUAL(defargscount, 1);
    CU_ASSERT_DOUBLE_EQUAL(defargs[0], 1.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // def has no default args
    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_NONE, 2, 1, 3, 1, 2, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x00, 0x01, 0x02),
        1.0, 5.0
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    defargs = NULL;
    defargscount = 0;
    CU_ASSERT_EQUAL(lmnt_get_default_args(ctx, fndata.def, &defargs, &defargscount), LMNT_ERROR_NOT_FOUND);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    // def claims to have default args, but no matching data section present
    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_HAS_DEFAULT_ARGS, 2, 1, 3, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x00, 0x01, 0x02)
    );
    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_DEF_DEFAULT_ARGS);
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}


MAKE_REGISTER_SUITE_FUNCTION(archive,
    CUNIT_CI_TEST(test_archive_backbranches),
    CUNIT_CI_TEST(test_archive_default_args)
);