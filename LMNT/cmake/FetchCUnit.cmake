include(FetchContent)

FetchContent_Declare(cunit
    GIT_REPOSITORY https://gitlab.com/Esvandiary/cunit  # TODO: https://gitlab.com/cunity/cunit
    GIT_TAG        allow-disable-tests-and-examples     # TODO: updated commit hash or 3.2.7 when released
    GIT_SHALLOW    true)
FetchContent_GetProperties(cunit)
if (NOT cunit_POPULATED)
    FetchContent_Populate(cunit)
    set(CUNIT_DISABLE_TESTS TRUE)
    set(CUNIT_DISABLE_EXAMPLES TRUE)

    # Modify compile flags to ensure we build CUnit with asserts enabled
    foreach (n IN ITEMS "" "_RELEASE" "_RELWITHDEBINFO" "_MINSIZEREL" "_DEBUG")
        string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_C_FLAGS${n} "${CMAKE_C_FLAGS${n}}")
        string(REGEX REPLACE "[-/]DNDEBUG" "" CMAKE_CXX_FLAGS${n} "${CMAKE_CXX_FLAGS${n}}")
    endforeach ()

    add_subdirectory("${cunit_SOURCE_DIR}" "${cunit_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
