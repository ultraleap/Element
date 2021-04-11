cmake_minimum_required(VERSION 3.14)

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 6.2.0
)

if (fmt_ADDED)
    get_target_property(fmt_INCLUDES fmt INTERFACE_INCLUDE_DIRECTORIES)
    set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${fmt_INCLUDES}")
endif()