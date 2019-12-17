cmake_minimum_required(VERSION 3.8)

project(element)

set(CMAKE_CXX_STANDARD 11)

set(element_sources
    "src/token.cpp"
    "src/interpreter.cpp"
    "src/construct.cpp"
    "src/construct.hpp"

    "src/common_internal.hpp"
    "src/interpreter_internal.hpp"
    "src/token_internal.hpp"

    "src/ast/ast.cpp"
    "src/ast/ast_internal.hpp"
    "src/ast/ast_indexes.hpp"
    "src/ast/functions.cpp"
    "src/ast/functions.hpp"
    "src/ast/fwd.hpp"
    "src/ast/scope.cpp"
    "src/ast/scope.hpp"
    "src/ast/types.cpp"
    "src/ast/types.hpp"

    "src/etree/compiler.cpp"
    "src/etree/compiler.hpp"
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
)

add_library(element STATIC ${element_sources})
target_include_directories(element
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/memorypool"
        # TODO: this, much nicer
        "${CMAKE_CURRENT_SOURCE_DIR}/../LMNT/include"
)

if (UNIX)
    target_link_libraries(element PRIVATE "m")
endif ()


if (BUILD_TESTING)
    add_executable(element_test_app
        "test/main.c")
    target_link_libraries(element_test_app PRIVATE element)
endif ()