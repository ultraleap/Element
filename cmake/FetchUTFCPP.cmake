cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(utfcpp
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp
    GIT_TAG v3.1.1
)

FetchContent_GetProperties(utfcpp)
if (NOT utfcpp_POPULATED)
    FetchContent_Populate(utfcpp)
    add_library(utfcpp INTERFACE)
    target_include_directories(utfcpp INTERFACE "${utfcpp_SOURCE_DIR}/source")
endif()