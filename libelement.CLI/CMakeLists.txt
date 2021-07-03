cmake_minimum_required(VERSION 3.8)

project(element_cli
	VERSION 1.0.0 
	LANGUAGES CXX)

#todo: per-target, but make sure it works on mac os
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)
set(ELEMENT_LIB_DIR "../libelement")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(ELEMENT_FORCE_FALLBACK_FS "Force element to use the fallback filesystem instead of <filesystem>" OFF)

add_subdirectory("dependencies" EXCLUDE_FROM_ALL)
find_package(RapidJSON)

set(element_cli_sources
    "include/compiler_message.hpp"
    "include/command.hpp"
    "include/evaluate_command.hpp"
    "include/parse_command.hpp"
    "include/typeof_command.hpp"
    "include/message_codes.hpp"
    "include/filesystem.hpp"

    "src/compiler_message.cpp"
    "src/message_codes.cpp"
    "src/command.cpp"
    "src/main.cpp"
)

set(ELEMENT_CODE_COVERAGE ${CODE_COVERAGE})
set(ELEMENT_BUILD_TESTING ${BUILD_TESTING})
add_subdirectory(${ELEMENT_LIB_DIR} "libelement")

add_executable(${CMAKE_PROJECT_NAME} ${element_cli_sources})

set_target_properties(element_cli PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY       "$<TARGET_FILE_DIR:element_cli>")

add_custom_command(
    TARGET element_cli POST_BUILD 
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../ContentFiles/ElementPackages"
        "$<TARGET_FILE_DIR:element_cli>/ElementPackages"
    COMMAND
        ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/../data/Element.ElementMessage-messages.toml"
        "$<TARGET_FILE_DIR:element_cli>/Messages-ELE.toml"
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${element_SOURCE_DIR}/src"
)

if (ELEMENT_FORCE_FALLBACK_FS)
    add_definitions(-DELEMENT_FORCE_FALLBACK_FS)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        element
        lmnt
        CLI11::CLI11
        fmt::fmt-header-only
        toml11::toml11
        rapidjson
        ghc_filesystem
)

if (UNIX AND NOT APPLE)
    target_link_libraries(element_cli PRIVATE "m" "atomic" "pthread" "stdc++fs")
endif()

if (CODE_COVERAGE)
    add_test(NAME test_cli COMMAND element_cli)
endif()

find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    find_package(Python)
    add_custom_target(element_cli_clang_check
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../libelement/scripts/run-clang-format.py -r
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Checking element CLI sources against clang format styling.")

    add_custom_target(element_cli_clang_format
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../libelement/scripts/run-clang-format.py -i -r
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Formatting element CLI sources to match clang format styling.")
endif()