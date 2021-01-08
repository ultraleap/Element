cmake_minimum_required(VERSION 3.14)

if (NOT ELEMENT_USE_EXTERNAL_FMT)
    include(FetchFMT)
endif ()
include(FetchUTFCPP)
include(FetchGHCFilesystem)

if (BUILD_TESTING)
    include(FetchCatch2)
endif()