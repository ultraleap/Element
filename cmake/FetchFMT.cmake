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
    #mark as a SYSTEM library to try and silence warnings
    get_target_property(fmt_INCLUDES fmt INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${fmt_INCLUDES}")
endif()