#pragma once

#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "lmnt/jit.h"
#include "testhelpers.h"

#define TESTSETUP_INCLUDED

#undef  TEST_NAME_PREFIX
#define TEST_NAME_PREFIX "jit_native_"

#undef  TEST_NAME_SUFFIX
#define TEST_NAME_SUFFIX ""

#undef  TEST_LOAD_ARCHIVE
#define TEST_LOAD_ARCHIVE(ctx, name, a, fndata) \
    CU_ASSERT_EQUAL_FATAL(lmnt_load_archive((ctx), (a).buf, (a).size), LMNT_OK);\
    {\
        lmnt_validation_result vr;\
        CU_ASSERT_EQUAL_FATAL(lmnt_prepare_archive((ctx), &vr), LMNT_OK);\
        CU_ASSERT_EQUAL_FATAL(vr, LMNT_VALIDATION_OK);\
        CU_ASSERT_EQUAL_FATAL(lmnt_find_def((ctx), (name), &((fndata).def)), LMNT_OK);\
        lmnt_jit_fn_data* jitfn = (lmnt_jit_fn_data*)calloc(1, sizeof(lmnt_jit_fn_data));\
        CU_ASSERT_EQUAL_FATAL(lmnt_jit_compile((ctx), (fndata).def, LMNT_JIT_TARGET_NATIVE, jitfn), LMNT_OK);\
        (fndata).data = jitfn;\
    }

#undef  TEST_LOAD_ARCHIVE_FAILS_VALIDATION
#define TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, name, a, fndata, code, vcode) \
    CU_ASSERT_EQUAL_FATAL(lmnt_load_archive((ctx), (a).buf, (a).size), LMNT_OK);\
    {\
        lmnt_validation_result vr;\
        CU_ASSERT_EQUAL_FATAL(lmnt_prepare_archive((ctx), &vr), (code));\
        CU_ASSERT_EQUAL_FATAL(vr, (vcode));\
    }

#undef  TEST_UNLOAD_ARCHIVE
#define TEST_UNLOAD_ARCHIVE(ctx, a, fndata) \
    if ((fndata).data) {\
        lmnt_jit_delete_function((lmnt_jit_fn_data*)((fndata).data));\
        free((fndata).data);\
        (fndata).data = NULL;\
    }

#undef  TEST_EXECUTE
#define TEST_EXECUTE(ctx, fndata, rvals, rvals_count) \
    lmnt_jit_execute(ctx, (lmnt_jit_fn_data*)((fndata).data), (rvals), (lmnt_offset)(rvals_count))

CU_TEST_SETUP()
{
    ctx = create_interpreter();
}

CU_TEST_TEARDOWN()
{
    delete_interpreter(ctx);
    ctx = NULL;
}
