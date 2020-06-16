#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "helpers.h"
#include <stdio.h>

lmnt_ictx* ctx;

// CU_SUITE_SETUP()
// {
//     return CUE_SUCCESS;
// }

// CU_SUITE_TEARDOWN()
// {
//     return CUE_SUCCESS;
// }

CU_TEST_SETUP()
{
    ctx = create_interpreter();
}

CU_TEST_TEARDOWN()
{
    delete_interpreter(ctx);
    ctx = NULL;
}

static void test_add_positive(void)
{
    archive a = create_archive_array("test", 2, 1, 3, 1, 0,
        LMNT_OP_BYTES(LMNT_OP_ADDSS, 0x00, 0x01, 0x02)
    );
    const lmnt_def* def;
    TEST_LOAD_ARCHIVE(ctx, "test", def, a);
    delete_archive_array(a);

    lmnt_value args[] = {1.0f, 2.0f};
    const size_t args_count = sizeof(args)/sizeof(lmnt_value);
    CU_ASSERT_EQUAL(lmnt_update_args(ctx, def, 0, args, args_count), LMNT_OK);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);
    CU_ASSERT_EQUAL(lmnt_execute(ctx, def, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 3.0, 0.000001);
}

CUNIT_CI_RUN("maths",
    CUNIT_CI_TEST(test_add_positive)
);
