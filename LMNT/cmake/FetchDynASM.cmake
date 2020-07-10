include(FetchContent)

FetchContent_Declare(dynasm
    GIT_REPOSITORY https://github.com/Esvandiary/dynasm
    GIT_TAG        2fd96bb2aa096e07d91cd67d96460be68aa313cf
    GIT_SHALLOW    true)
FetchContent_GetProperties(dynasm)
if (NOT dynasm_POPULATED)
    FetchContent_Populate(dynasm)
    add_subdirectory("${dynasm_SOURCE_DIR}" "${dynasm_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
