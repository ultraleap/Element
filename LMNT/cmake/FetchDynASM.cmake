include(FetchContent)

FetchContent_Declare(dynasm
    GIT_REPOSITORY https://github.com/Esvandiary/dynasm
    GIT_TAG        141de6b19afc137953d15ed2c720934ad4899ea9
    GIT_SHALLOW    true)
FetchContent_GetProperties(dynasm)
if (NOT dynasm_POPULATED)
    FetchContent_Populate(dynasm)
    add_subdirectory("${dynasm_SOURCE_DIR}" "${dynasm_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
