cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(TOML11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11
    GIT_TAG        v3.6.0
)

FetchContent_GetProperties(TOML11)
if (NOT TOML11_POPULATED)
    FetchContent_Populate(TOML11)
    add_subdirectory(${toml11_SOURCE_DIR} ${toml11_BINARY_DIR})
endif()

FetchContent_MakeAvailable(TOML11)