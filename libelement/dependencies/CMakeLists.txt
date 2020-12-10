cmake_minimum_required(VERSION 3.14)

include(FetchFMT)
include(FetchUTFCPP)
include(FetchGHCFilesystem)

if (BUILD_TESTING)
    include(FetchCatch2)
endif()