include(FetchContent)

FetchContent_Declare(cunit
    GIT_REPOSITORY https://gitlab.com/cunity/cunit
    GIT_TAG        3.2.6
    GIT_SHALLOW    true)
FetchContent_GetProperties(cunit)
if (NOT cunit_POPULATED)
    FetchContent_Populate(cunit)
    set(CUNIT_DISABLE_TESTS ON)
    set(CUNIT_DISABLE_EXAMPLES ON)
    add_subdirectory("${cunit_SOURCE_DIR}" "${cunit_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
