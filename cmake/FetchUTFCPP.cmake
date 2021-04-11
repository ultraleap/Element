cmake_minimum_required(VERSION 3.14)

CPMAddPackage(
    NAME utfcpp
    GITHUB_REPOSITORY nemtrif/utfcpp
    VERSION 3.1.1
)

if (utfcpp_ADDED)
    add_library(utfcpp INTERFACE)
    target_include_directories(utfcpp INTERFACE "${utfcpp_SOURCE_DIR}/source")
endif()