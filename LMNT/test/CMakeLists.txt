include(FetchCUnit)

add_executable(test_interpreter "test_interpreter.c")
target_link_libraries(test_interpreter PRIVATE lmnt cunit)
add_test(NAME test_interpreter COMMAND $<TARGET_FILE:test_interpreter>)

if (LMNT_BUILD_JIT)
    add_executable(test_jit_native "test_jit_native.c")
    target_link_libraries(test_jit_native PRIVATE lmnt cunit)
    add_test(NAME test_jit_native COMMAND $<TARGET_FILE:test_jit_native>)
endif ()
