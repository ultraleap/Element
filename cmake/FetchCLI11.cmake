cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11
    GIT_TAG        v1.9.1
)

FetchContent_GetProperties(CLI11)
if (NOT CLI11_POPULATED)
    FetchContent_Populate(CLI11)
    add_subdirectory(${cli11_SOURCE_DIR} ${cli11_BINARY_DIR})
endif()

FetchContent_MakeAvailable(CLI11)