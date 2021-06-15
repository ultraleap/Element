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
    archive a = create_archive_array_with_flags("test", LMNT_DEFFLAG_HAS_BACKBRANCHES, 1, 1, 2, 4, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x02, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x10, 0x00, 0x03),
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


    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_NONE, 1, 1, 2, 4, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x02, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x00, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x10, 0x00, 0x03),
        1.0, 5.0
    );
    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_DEF_FLAGS);
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    a = create_archive_array_with_flags("test", LMNT_DEFFLAG_NONE, 1, 1, 2, 4, 0, 2,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x02, 0x00, 0x02),
        LMNT_OP_BYTES(LMNT_OP_CMP, 0x02, 0x01, 0x00),
        LMNT_OP_BYTES(LMNT_OP_BRANCHCLT, 0x00, 0x03, 0x00),
        LMNT_OP_BYTES(LMNT_OP_ASSIGNIIS, 0x10, 0x00, 0x03),
        1.0, 5.0
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 16.0, FLOAT_ERROR_MARGIN);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}


MAKE_REGISTER_SUITE_FUNCTION(archive,
    CUNIT_CI_TEST(test_archive_backbranches)
);