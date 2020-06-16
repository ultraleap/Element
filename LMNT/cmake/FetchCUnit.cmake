include(FetchContent)

FetchContent_Declare(cunit
    GIT_REPOSITORY https://gitlab.com/cunity/cunit
    GIT_TAG        99e89b2ae16594b089a749fc7b9f8383349e3c91 # TODO: 3.2.7
    GIT_SHALLOW    true)
FetchContent_GetProperties(cunit)
if (NOT cunit_POPULATED)
    FetchContent_Populate(cunit)
    set(CUNIT_DISABLE_TESTS ON)
    set(CUNIT_DISABLE_EXAMPLES ON)
    add_subdirectory("${cunit_SOURCE_DIR}" "${cunit_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
