include(FetchCUnit)

set(test_headers
    "test_bounds.h"
    "test_branch.h"
    "test_fncall.h"
    "test_maths_scalar.h"
    "test_maths_vector.h"
    "test_misc.h"
    "test_trig.h"
    "testhelpers.h"
)

add_executable(test_interpreter "test_interpreter.c" "testsetup_interpreter.h" ${test_headers})
target_link_libraries(test_interpreter PRIVATE lmnt cunit)
add_test(NAME test_interpreter COMMAND $<TARGET_FILE:test_interpreter>)

if (LMNT_BUILD_JIT)
    add_executable(test_jit_native "test_jit_native.c" "testsetup_jit_native.h" ${test_headers})
    target_link_libraries(test_jit_native PRIVATE lmnt cunit)
    add_test(NAME test_jit_native COMMAND $<TARGET_FILE:test_jit_native>)
endif ()
