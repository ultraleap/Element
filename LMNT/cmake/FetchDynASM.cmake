include(FetchContent)

FetchContent_Declare(dynasm
    GIT_REPOSITORY https://github.com/Esvandiary/dynasm
    GIT_TAG        b6166a1e899349c6ae0a265cb5d6db8289e69fc8
    GIT_SHALLOW    true)
FetchContent_GetProperties(dynasm)
if (NOT dynasm_POPULATED)
    FetchContent_Populate(dynasm)
    add_subdirectory("${dynasm_SOURCE_DIR}" "${dynasm_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()