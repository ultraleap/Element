cmake_minimum_required(VERSION 3.14)

CPMAddPackage(
    NAME rapidjson
    GITHUB_REPOSITORY Tencent/rapidjson
    VERSION 1.1.0
    OPTIONS
        "RAPIDJSON_BUILD_DOC OFF"
        "RAPIDJSON_BUILD_EXAMPLES OFF"
        "RAPIDJSON_BUILD_TESTS OFF"
        "RAPIDJSON_ENABLE_INSTRUMENTATION_OPT OFF"
)

if (rapidjson_ADDED)
    add_library(rapidjson INTERFACE)
    target_include_directories(rapidjson INTERFACE "${rapidjson_SOURCE_DIR}/include")
    target_compile_definitions(rapidjson INTERFACE "RAPIDJSON_HAS_STDSTRING")
endif()