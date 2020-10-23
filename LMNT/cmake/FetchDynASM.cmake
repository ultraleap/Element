include(FetchContent)

FetchContent_Declare(dynasm
    GIT_REPOSITORY https://github.com/Esvandiary/dynasm.git
    GIT_TAG        80b89aa759279b5902d7d07835338ff2c28a7caf
#   GIT_SHALLOW    true
)
FetchContent_GetProperties(dynasm)
if (NOT dynasm_POPULATED)
    FetchContent_Populate(dynasm)
    add_subdirectory("${dynasm_SOURCE_DIR}" "${dynasm_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()
