include(FetchContent)

FetchContent_Declare(dynasm
    GIT_REPOSITORY https://github.com/Esvandiary/dynasm
    GIT_TAG        a51589334fe9edde22e1cd4020bece5b6459a5fb
    GIT_SHALLOW    true)
FetchContent_GetProperties(dynasm)
if (NOT dynasm_POPULATED)
    FetchContent_Populate(dynasm)
    add_subdirectory("${dynasm_SOURCE_DIR}" "${dynasm_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
