cmake_minimum_required(VERSION 3.11)

set(IS_ROOT_PROJECT false)
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if (NOT HAS_PARENT)
    set(IS_ROOT_PROJECT true)
endif()

# For VS
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(element VERSION 1.0.0)

#todo: do it per-target, but needs to be checked on mac/xcode
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ELEMENT_LOG_VERBOSITY "log_flags::none" CACHE STRING "logging flags")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Code Coverage Configuration
add_library(element_coverage_config INTERFACE)
add_library(element::coverage_config ALIAS element_coverage_config)
option(ELEMENT_CODE_COVERAGE "Enable coverage reporting" OFF)
option(ELEMENT_GEN_HTML "Enable code coverage html output" OFF)

if (ELEMENT_CODE_COVERAGE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "ELEMENT_CODE_COVERAGE ON")
        # Add required flags (GCC & LLVM/Clang)
        target_compile_options(element_coverage_config INTERFACE
            -O0        # no optimization
            -g         # generate debug info
            --coverage # sets all required flags
        )

        if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
            target_link_options(element_coverage_config INTERFACE --coverage)
        else()
            target_link_libraries(element_coverage_config INTERFACE --coverage)
        endif()
    else()
        message(FATAL_ERROR "ELEMENT_CODE_COVERAGE is ON but the compiler is not GNU|Clang")
    endif()
endif()

configure_file (
    "src/configuration.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/configuration.hpp"
)

set(element_sources
    "src/source_information.hpp"

    "src/log_errors.hpp"
    "src/log_errors.cpp"

    "src/token.cpp"
    "src/token_internal.cpp"
    "src/interpreter.cpp"
    "src/interpreter_internal.cpp"
    "src/common_internal.cpp"
    "src/object.cpp"

    "src/common_internal.hpp"
    "src/interpreter_internal.hpp"
    "src/token_internal.hpp"

    #AST
    "src/ast/ast.cpp"
    "src/ast/ast_internal.hpp"
    "src/ast/ast_internal.cpp"
    "src/ast/parser.cpp"
    "src/ast/parser_internal.cpp"
    "src/ast/parser_internal.hpp"
    "src/ast/ast_indexes.hpp"
    "src/ast/fwd.hpp"
    
    #Object model
    "src/object_model/constraints/any_constraint.hpp"
    "src/object_model/constraints/bool_type.cpp"
    "src/object_model/constraints/bool_type.hpp"
    "src/object_model/constraints/constraint.hpp"
    "src/object_model/constraints/constraint.cpp"
    "src/object_model/constraints/num_type.cpp"
    "src/object_model/constraints/num_type.hpp"
    "src/object_model/constraints/type.hpp"
    "src/object_model/constraints/user_function_constraint.cpp"
    "src/object_model/constraints/user_function_constraint.hpp"
    "src/object_model/constraints/user_type.hpp"

    "src/object_model/declarations/constraint_declaration.cpp"
    "src/object_model/declarations/constraint_declaration.hpp"
    "src/object_model/declarations/declaration.cpp"
    "src/object_model/declarations/declaration.hpp"
    "src/object_model/declarations/function_declaration.cpp"
    "src/object_model/declarations/function_declaration.hpp"
    "src/object_model/declarations/namespace_declaration.cpp"
    "src/object_model/declarations/namespace_declaration.hpp"
    "src/object_model/declarations/struct_declaration.cpp"
    "src/object_model/declarations/struct_declaration.hpp"

    "src/object_model/expressions/call_expression.cpp"
    "src/object_model/expressions/call_expression.hpp"
    "src/object_model/expressions/expression.hpp"
    "src/object_model/expressions/expression_chain.cpp"
    "src/object_model/expressions/expression_chain.hpp"
    "src/object_model/expressions/identifier_expression.cpp"
    "src/object_model/expressions/identifier_expression.hpp"
    "src/object_model/expressions/indexing_expression.cpp"
    "src/object_model/expressions/indexing_expression.hpp"
    "src/object_model/expressions/literal_expression.cpp"
    "src/object_model/expressions/literal_expression.hpp"
    "src/object_model/expressions/anonymous_block_expression.cpp"
    "src/object_model/expressions/anonymous_block_expression.hpp"
    "src/object_model/expressions/lambda_expression.cpp"
    "src/object_model/expressions/lambda_expression.hpp"

    "src/object_model/intermediaries/declaration_wrapper.cpp"
    "src/object_model/intermediaries/declaration_wrapper.hpp"
    "src/object_model/intermediaries/function_instance.cpp"
    "src/object_model/intermediaries/function_instance.hpp"
    "src/object_model/intermediaries/struct_instance.cpp"
    "src/object_model/intermediaries/struct_instance.hpp"
    "src/object_model/intermediaries/list_wrapper.cpp"
    "src/object_model/intermediaries/list_wrapper.hpp"
    "src/object_model/intermediaries/anonymous_block_instance.cpp"
    "src/object_model/intermediaries/anonymous_block_instance.hpp"

    "src/object_model/intrinsics/intrinsic.cpp"
    "src/object_model/intrinsics/intrinsic.hpp"
    "src/object_model/intrinsics/intrinsic_binary.cpp"
    "src/object_model/intrinsics/intrinsic_binary.hpp"
    "src/object_model/intrinsics/intrinsic_constructor_bool.cpp"
    "src/object_model/intrinsics/intrinsic_constructor_bool.hpp"
    "src/object_model/intrinsics/intrinsic_constructor_num.cpp"
    "src/object_model/intrinsics/intrinsic_constructor_num.hpp"
    "src/object_model/intrinsics/intrinsic_constructor_list.cpp"
    "src/object_model/intrinsics/intrinsic_constructor_list.hpp"
    "src/object_model/intrinsics/intrinsic_function.cpp"
    "src/object_model/intrinsics/intrinsic_function.hpp"
    "src/object_model/intrinsics/intrinsic_if.cpp"
    "src/object_model/intrinsics/intrinsic_if.hpp"
    "src/object_model/intrinsics/intrinsic_for.cpp"
    "src/object_model/intrinsics/intrinsic_for.hpp"
    "src/object_model/intrinsics/intrinsic_nullary.cpp"
    "src/object_model/intrinsics/intrinsic_nullary.hpp"
    "src/object_model/intrinsics/intrinsic_unary.cpp"
    "src/object_model/intrinsics/intrinsic_unary.hpp"
    "src/object_model/intrinsics/intrinsic_list.cpp"
    "src/object_model/intrinsics/intrinsic_list.hpp"
    "src/object_model/intrinsics/intrinsic_list_fold.cpp"
    "src/object_model/intrinsics/intrinsic_list_fold.hpp"
    "src/object_model/intrinsics/intrinsic_compiler_list_indexer.cpp"
    "src/object_model/intrinsics/intrinsic_compiler_list_indexer.hpp"

    "src/object_model/call_stack.cpp"
    "src/object_model/call_stack.hpp"
    "src/object_model/capture_stack.cpp"
    "src/object_model/capture_stack.hpp"
    "src/object_model/compilation_context.cpp"
    "src/object_model/compilation_context.hpp"
    "src/object_model/error.cpp"
    "src/object_model/error.hpp"
    "src/object_model/error_map.cpp"
    "src/object_model/error_map.hpp"
    "src/object_model/fwd.hpp"
    "src/object_model/identifier.cpp"
    "src/object_model/identifier.hpp"
    "src/object_model/metainfo.cpp"
    "src/object_model/metainfo.hpp"
    "src/object_model/object_internal.cpp"
    "src/object_model/object_internal.hpp"
    "src/object_model/object_model_builder.cpp"
    "src/object_model/object_model_builder.hpp"
    "src/object_model/port.cpp"
    "src/object_model/port.hpp"
    "src/object_model/scope.cpp"
    "src/object_model/scope.hpp"
    "src/object_model/type_annotation.cpp"
    "src/object_model/type_annotation.hpp"

    #Instruction tree
    "src/instruction_tree/evaluator.cpp"
    "src/instruction_tree/evaluator.hpp"
    "src/instruction_tree/instructions.cpp"
    "src/instruction_tree/instructions.hpp"
    "src/instruction_tree/fwd.hpp"

    #Util
    "src/stringutil.hpp"
    "src/typeutil.hpp"
    
    #C API
    "include/element/element.h"
    "include/element/common.h"
    "include/element/ast.h"
    "include/element/parser.h"
    "include/element/token.h"
    "include/element/interpreter.h"
    "include/element/object.h"

    #MISC
    "src/configuration.hpp.in"
    "src/filesystem.hpp"
)

add_subdirectory("dependencies" EXCLUDE_FROM_ALL)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(element STATIC ${element_sources})
add_library(element::element ALIAS element)

# For VS
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${element_sources})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES "${CMAKE_CURRENT_BINARY_DIR}/configuration.hpp")

# Don't rely on compiler extensions
set_property(TARGET element PROPERTY CMAKE_CXX_EXTENSIONS OFF)

#todo: probably not good enough
IF (WIN32)
    set(debug_dir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
ELSE()
    set(debug_dir "${CMAKE_CURRENT_BINARY_DIR}")
ENDIF()

# Copy prelude alongside built library
add_custom_command (
	TARGET element POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_CURRENT_SOURCE_DIR}/../ContentFiles"
	"${debug_dir}")

target_include_directories(element
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/memorypool"
        # TODO: this, much nicer
        "${CMAKE_CURRENT_SOURCE_DIR}/../LMNT/include"
        "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(element PRIVATE utfcpp fmt::fmt-header-only ghc_filesystem )
target_link_libraries(element PUBLIC element_coverage_config)

if (UNIX AND NOT APPLE)
    target_link_libraries(element PRIVATE "m" "atomic" "pthread" "stdc++fs")
endif ()

## A C project to ensure our C API is correct
add_executable(element_app_c "${CMAKE_CURRENT_SOURCE_DIR}/test/main.c")
target_link_libraries(element_app_c PRIVATE element)
target_include_directories(element_app_c PRIVATE "${element_SOURCE_DIR}/include")
set_target_properties(element_app_c PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY       "${debug_dir}")
add_custom_command (
        TARGET element_app_c POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../ContentFiles"
        "${debug_dir}")

if (ELEMENT_BUILD_TESTING)
    include(CTest)
    set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/catch2)
    add_library(Catch INTERFACE)
    target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

    file(GLOB_RECURSE TEST_SOURCES
            "${PROJECT_SOURCE_DIR}/test/*.test.cpp"
            "${PROJECT_SOURCE_DIR}/test/*.test.hpp"
            )

    add_executable(element_tests ${TEST_SOURCES})
    target_link_libraries(element_tests PRIVATE Catch2 element fmt::fmt-header-only )
    target_link_libraries(element_tests PUBLIC element_coverage_config)
    target_include_directories(element_tests PRIVATE "${element_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
    add_test(NAME libelement_test
            COMMAND element_tests -r junit -o ${CMAKE_BINARY_DIR}/libemelement_test_app_details.xml)

    set_target_properties(element_tests PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY       "${debug_dir}")

    # Copy prelude alongside built library
    add_custom_command (
            TARGET element_tests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/../ContentFiles"
            "${debug_dir}")

    if (ELEMENT_CODE_COVERAGE)
        # Add an OR exit 0 in case tests fail but still want coverage of tests ran
        add_custom_target(element_test_coverage
                COMMAND ./element_tests || (exit 0)
                DEPENDS element_tests)

        # Ensure that lcov has been installed
        find_package(Lcov)

        if(Lcov_FOUND)
            add_custom_command(
                    TARGET element_test_coverage POST_BUILD
                    COMMAND ${LCOV_BIN} --capture --directory . --output-file coverage.info
                    COMMAND ${LCOV_BIN} --remove coverage.info '/Library/*' '${CMAKE_BINARY_DIR}/*' '${CMAKE_SOURCE_DIR}/dependencies/*' '${CMAKE_SOURCE_DIR}/test/*' --output-file coverage.info
                    COMMAND ${LCOV_BIN} --list coverage.info)

            if(GEN_HTML)
                add_custom_command(TARGET element_test_coverage POST_BUILD
                        COMMAND mkdir -p coverage-html
                        COMMAND ${GENHTML_BIN} -o coverage-html coverage.info)
            endif()
        else()
            message(WARNING "Lcov not found, coverage report not generated for libelement test app.")
        endif()
    endif()
endif()

find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    find_package(Python)
    add_custom_target(element_clang_check
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-clang-format.py -r
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/test
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Checking element sources against clang format styling.")

    add_custom_target(element_clang_format
            COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run-clang-format.py -i -r
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                ${CMAKE_CURRENT_SOURCE_DIR}/test
                ${CMAKE_CURRENT_SOURCE_DIR}/include
            COMMENT "Formatting element sources to match clang format styling.")
endif()