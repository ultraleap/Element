cmake_minimum_required(VERSION 3.11)

# For VS
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(element)

set(CMAKE_CXX_STANDARD 17)

set(element_sources
    "src/token.cpp"
    "src/interpreter.cpp"
    "src/construct.cpp"
    "src/construct.hpp"
    "src/common.cpp"

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

add_subdirectory("dependencies" EXCLUDE_FROM_ALL)

add_library(element STATIC ${element_sources})

# For VS
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${element_sources})

# Don't rely on compiler extensions
set_property(TARGET element PROPERTY CMAKE_CXX_EXTENSIONS OFF)

# Copy prelude alongside built library
add_custom_command (
	TARGET element POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_SOURCE_DIR}/../Common"
	"${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

target_include_directories(element
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/memorypool"
        # TODO: this, much nicer
        "${CMAKE_CURRENT_SOURCE_DIR}/../LMNT/include"
        "${CMAKE_BINARY_DIR}/_deps/fmt-src/include"
)
target_link_libraries(element PRIVATE utf8cpp fmt::fmt-header-only)

if (UNIX)
    target_link_libraries(element PRIVATE "m")
endif ()

if (BUILD_TESTING)
    add_executable(element_test_app
        "test/main.c")
    target_link_libraries(element_test_app PRIVATE element)

    set_target_properties(element_test_app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY       "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
endif ()