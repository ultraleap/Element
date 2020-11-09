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
    # mark as SYSTEM library to silence warnings
    get_target_property(fmt_INCLUDES fmt INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(fmt
        PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${fmt_INCLUDES}"
    )
endif ()

include(FetchContent)
FetchContent_Declare(
    utfcpp
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp
    GIT_TAG v3.1.1
)
FetchContent_GetProperties(utfcpp)
if (NOT utfcpp_POPULATED)
    FetchContent_Populate(utfcpp)
    add_library(utf8cpp INTERFACE)
    target_include_directories(utf8cpp INTERFACE "${utfcpp_SOURCE_DIR}/source")
endif ()

if(BUILD_TESTING)
    FetchContent_Declare(Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2
            GIT_TAG        v2.11.1
            )
    FetchContent_GetProperties(Catch2)
    if(NOT Catch2_POPULATED)
        FetchContent_Populate(Catch2)
        add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR})
    endif()
    FetchContent_MakeAvailable(Catch2)
endif()