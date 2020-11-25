cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson
    GIT_TAG        v1.1.0
    GIT_SHALLOW    TRUE
)

FetchContent_GetProperties(rapidjson)
if (NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
    add_library(rapidjson INTERFACE)
    target_include_directories(rapidjson INTERFACE "${rapidjson_SOURCE_DIR}/include")
endif()