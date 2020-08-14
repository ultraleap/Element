cmake_minimum_required(VERSION 3.8)

project(libelement_cli 
	VERSION 1.0.0 
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ELEMENT_LIB_DIR "../libelement")
set(DEPENDENCIES_DIR "../dependencies")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(RapidJSON)
add_subdirectory("dependencies" EXCLUDE_FROM_ALL)

set(libelement_cli_sources
    "include/compiler_message.hpp"
    "include/command.hpp"
    "include/evaluate_command.hpp"
    "include/parse_command.hpp"
    "include/typeof_command.hpp"
    "include/message_codes.hpp"

    "src/compiler_message.cpp"
    "src/command.cpp"
    "src/main.cpp"
)

add_subdirectory(${ELEMENT_LIB_DIR} "libelement")

add_executable(${CMAKE_PROJECT_NAME} ${libelement_cli_sources})

IF (WIN32)
	set(debug_dir "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
ELSE()
	set(debug_dir "${CMAKE_BINARY_DIR}")
ENDIF()

set_target_properties(libelement_cli PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY       "${debug_dir}")

add_custom_command (
	TARGET libelement_cli POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/../Common"
	"${debug_dir}")

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${DEPENDENCIES_DIR}/toml11"
        "${DEPENDENCIES_DIR}/rapidjson/include"
        "${ELEMENT_LIB_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_BINARY_DIR}/_deps/fmt-src/include"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libelement CLI11::CLI11 fmt::fmt-header-only)

if (LINUX)
    target_link_libraries(libelement_cli PRIVATE "m" "atomic" "pthread" "stdc++fs")
endif ()

if (CODE_COVERAGE)
    add_test(NAME test_cli COMMAND libelement_cli)
endif()