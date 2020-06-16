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
    add_subdirectory("${cunit_SOURCE_DIR}" "${cunit_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
