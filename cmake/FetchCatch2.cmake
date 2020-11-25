cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG        v2.11.1
)

FetchContent_GetProperties(Catch2)
if (NOT Catch2_POPULATED)
    FetchContent_Populate(Catch2)
    add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
endif()

FetchContent_MakeAvailable(Catch2)