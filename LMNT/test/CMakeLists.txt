include(FetchCUnit)

add_executable(test_lmnt
    "test_maths_scalar.c"
)
target_link_libraries(test_lmnt PRIVATE lmnt cunit)

add_test(NAME unit_tests
	COMMAND $<TARGET_FILE:test_lmnt>
)
