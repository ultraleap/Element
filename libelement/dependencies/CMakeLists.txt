cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        6.2.0
    GIT_SHALLOW    TRUE
    CMAKE_ARGS
        -DFMT_DOC=OFF
        -DFMT_INSTALL=OFF
        -DFMT_TEST=OFF
)
FetchContent_GetProperties(fmt)
if (NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
    add_subdirectory("${fmt_SOURCE_DIR}" "${fmt_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif ()

include(FetchContent)
FetchContent_Declare(
    utfcpp
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp
    GIT_TAG v3.1
)
FetchContent_GetProperties(utfcpp)
if (NOT utfcpp_POPULATED)
    FetchContent_Populate(utfcpp)
    add_library(utf8cpp INTERFACE)
    target_include_directories(utf8cpp INTERFACE "${utfcpp_SOURCE_DIR}/source")
endif ()