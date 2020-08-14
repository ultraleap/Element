cmake_minimum_required(VERSION 3.11)

# For VS
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(libelement)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LOG_VERBOSITY "log_flags::none" CACHE STRING "logging flags")

# Code Coverage Configuration
add_library(coverage_config INTERFACE)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if (CODE_COVERAGE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "CODE_COVERAGE ON")
        # Add required flags (GCC & LLVM/Clang)
        target_compile_options(coverage_config INTERFACE
            -O0        # no optimization
            -g         # generate debug info
            --coverage # sets all required flags
        )

        if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
            target_link_options(coverage_config INTERFACE --coverage)
        else()
            target_link_libraries(coverage_config INTERFACE --coverage)
        endif()
    else()
        message(FATAL_ERROR "CODE_COVERAGE is ON but the compiler is not GNU|Clang")
    endif()
endif()

configure_file (
    "src/configuration.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/configuration.hpp"
)

set(libelement_sources
    "notes.txt"

    "src/source_information.hpp"

    "src/log_errors.hpp"
    "src/log_errors.cpp"

    "src/token.cpp"
    "src/interpreter.cpp"
    "src/common.cpp"

    "src/common_internal.hpp"
    "src/interpreter_internal.hpp"
    "src/token_internal.hpp"

    #AST
    "src/ast/ast.cpp"
    "src/ast/ast_internal.hpp"
    "src/ast/ast_indexes.hpp"
    "src/ast/fwd.hpp"
    
    #Object model
    "src/obj_model/fwd.hpp"
    "src/obj_model/errors.hpp"
    "src/obj_model/errors.cpp"

    "src/obj_model/object_model.hpp"
    "src/obj_model/object_model.cpp"
    
    "src/obj_model/object.hpp"
    "src/obj_model/object.cpp"

    "src/obj_model/object_model.hpp"
    "src/obj_model/object_model.cpp"

    "src/obj_model/port.hpp"
    "src/obj_model/port.cpp"
    "src/obj_model/type_annotation.hpp"
    
    "src/obj_model/declarations.hpp"
    "src/obj_model/declarations.cpp"
    
    "src/obj_model/expressions.hpp"
    "src/obj_model/expressions.cpp"

    "src/obj_model/intrinsics.cpp"
    "src/obj_model/intrinsics.hpp"

    "src/obj_model/types.cpp"
    "src/obj_model/types.hpp"

    "src/obj_model/intermediaries.hpp"
    "src/obj_model/intermediaries.cpp"

    "src/obj_model/scope.hpp"
    "src/obj_model/scope.cpp"

    "src/obj_model/metainfo.cpp"

    #Expression tree
    "src/etree/evaluator.cpp"
    "src/etree/evaluator.hpp"
    "src/etree/expressions.cpp"
    "src/etree/expressions.hpp"
    "src/etree/fwd.hpp"

    "src/lmnt/compiler.cpp"
    "src/lmnt/compiler.hpp"

    "src/stringutil.hpp"
    "src/typeutil.hpp"
    
    "include/element/common.h"
    "include/element/ast.h"
    "include/element/token.h"
    "include/element/interpreter.h"
    "include/element/lmnt.h"
    
	"src/configuration.hpp.in"
	"${CMAKE_CURRENT_BINARY_DIR}/configuration.hpp"
)

add_subdirectory("dependencies" EXCLUDE_FROM_ALL)

add_library(libelement STATIC ${libelement_sources})

# For VS
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${libelement_sources})

# Don't rely on compiler extensions
set_property(TARGET libelement PROPERTY CMAKE_CXX_EXTENSIONS OFF)

IF (WIN32)
    set(debug_dir "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
ELSE()
    set(debug_dir "${CMAKE_BINARY_DIR}")
ENDIF()

# Copy prelude alongside built library
add_custom_command (
	TARGET libelement POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_SOURCE_DIR}/../Common"
	"${debug_dir}")

target_include_directories(libelement
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/memorypool"
        # TODO: this, much nicer
        "${CMAKE_CURRENT_SOURCE_DIR}/../LMNT/include"
        "${CMAKE_BINARY_DIR}/_deps/fmt-src/include"
        "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(libelement PRIVATE utf8cpp fmt::fmt-header-only )
target_link_libraries(libelement PUBLIC coverage_config)

if (UNIX)
    target_link_libraries(libelement PRIVATE "m" "atomic" "pthread" "stdc++fs")
endif ()

if (BUILD_TESTING)
    include(CTest)
    set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/catch2)
    add_library(Catch INTERFACE)
    target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

    file(GLOB_RECURSE TEST_SOURCES
            "${PROJECT_SOURCE_DIR}/test/*.test.cpp"
            )

    #set(TEST_SOURCES
    #        test/main.test.cpp
    #       test/eval.test.cpp)

    add_executable(libelement_test_app ${TEST_SOURCES})
    target_link_libraries(libelement_test_app PRIVATE Catch2 libelement)
    target_link_libraries(libelement_test_app PUBLIC coverage_config)

    add_test(NAME run_libelement_test_app
            COMMAND libelement_test_app -r junit -o ${CMAKE_BINARY_DIR}/libemelement_test_app_details.xml)

    set_target_properties(libelement_test_app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY       "${debug_dir}")
endif ()

target_compile_definitions(libelement
    PRIVATE
        ${LEGACY_COMPILER}
)