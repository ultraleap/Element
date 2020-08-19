#pragma once

#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "testhelpers.h"

#define TESTSETUP_INCLUDED

#undef  TEST_NAME_PREFIX
#define TEST_NAME_PREFIX "interpreter_"

#undef  TEST_NAME_SUFFIX
#define TEST_NAME_SUFFIX ""

#undef  TEST_LOAD_ARCHIVE
#define TEST_LOAD_ARCHIVE(ctx, name, a, fndata) \
    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);\
    {\
        lmnt_validation_result vr;\
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), LMNT_OK);\
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VALIDATION_OK);\
    }\
    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_find_def((ctx), (name), &((fndata).def)), LMNT_OK);

#undef  TEST_LOAD_ARCHIVE_FAILS_VALIDATION
#define TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, name, a, fndata, code, vcode) \
    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_load_archive((ctx), (a).buf, (a).size), LMNT_OK);\
    {\
        lmnt_validation_result vr;\
        CU_ASSERT_EQUAL_FATAL(lmnt_ictx_prepare_archive((ctx), &vr), (code));\
        CU_ASSERT_EQUAL_FATAL(vr, (vcode));\
    }

#undef  TEST_UNLOAD_ARCHIVE
#define TEST_UNLOAD_ARCHIVE(ctx, a, fndata)

#undef  TEST_EXECUTE
#define TEST_EXECUTE(ctx, fndata, rvals, rvals_count) \
    lmnt_execute(ctx, (fndata).def, (rvals), (lmnt_offset)(rvals_count))

CU_TEST_SETUP()
{
    ctx = create_interpreter();
}

CU_TEST_TEARDOWN()
{
    delete_interpreter(ctx);
    ctx = NULL;
}
