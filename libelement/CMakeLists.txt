cmake_minimum_required(VERSION 3.11)

# For VS
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(libelement)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LOG_VERBOSITY "log_flags::none" CACHE STRING "logging flags")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Code Coverage Configuration
add_library(coverage_config INTERFACE)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(GEN_HTML "Enable code coverage html output" OFF)

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

    "src/object_model/intermediaries/declaration_wrapper.cpp"
    "src/object_model/intermediaries/declaration_wrapper.hpp"
    "src/object_model/intermediaries/function_instance.cpp"
    "src/object_model/intermediaries/function_instance.hpp"
    "src/object_model/intermediaries/struct_instance.cpp"
    "src/object_model/intermediaries/struct_instance.hpp"
    "src/object_model/intermediaries/list_wrapper.cpp"
    "src/object_model/intermediaries/list_wrapper.hpp"

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
    "src/object_model/object.cpp"
    "src/object_model/object.hpp"
    "src/object_model/object_model_builder.cpp"
    "src/object_model/object_model_builder.hpp"
    "src/object_model/port.cpp"
    "src/object_model/port.hpp"
    "src/object_model/scope.cpp"
    "src/object_model/scope.hpp"
    "src/object_model/type_annotation.cpp"
    "src/object_model/type_annotation.hpp"

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
	"${CMAKE_CURRENT_SOURCE_DIR}/../ContentFiles"
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

if (UNIX AND NOT APPLE)
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

    add_executable(libelement_test_app ${TEST_SOURCES})
    target_link_libraries(libelement_test_app PRIVATE Catch2 libelement fmt::fmt-header-only )
    target_link_libraries(libelement_test_app PUBLIC coverage_config)
    target_include_directories(libelement_test_app PRIVATE "${libelement_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/_deps/fmt-src/include")
    add_test(NAME libelement_test
            COMMAND libelement_test_app -r junit -o ${CMAKE_BINARY_DIR}/libemelement_test_app_details.xml)

    set_target_properties(libelement_test_app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY       "${debug_dir}")

    # Copy prelude alongside built library
    add_custom_command (
            TARGET libelement_test_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/../ContentFiles"
            "${debug_dir}")

    if(CODE_COVERAGE)
        # Add an OR exit 0 in case tests fail but still want coverage of tests ran
        add_custom_target(libelement_test_coverage
                COMMAND ./libelement_test_app || (exit 0)
                DEPENDS libelement_test_app)

        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

        # Ensure that lcov has been installed
        find_package(Lcov)

        if(Lcov_FOUND)
            add_custom_command(
                    TARGET libelement_test_coverage POST_BUILD
                    COMMAND ${LCOV_BIN} --capture --directory . --output-file coverage.info
                    COMMAND ${LCOV_BIN} --remove coverage.info '/Library/*' '${CMAKE_BINARY_DIR}/*' '${CMAKE_SOURCE_DIR}/dependencies/*' '${CMAKE_SOURCE_DIR}/test/*' --output-file coverage.info
                    COMMAND ${LCOV_BIN} --list coverage.info)

            if(GEN_HTML)
                add_custom_command(TARGET libelement_test_coverage POST_BUILD
                        COMMAND mkdir -p coverage-html
                        COMMAND ${GENHTML_BIN} -o coverage-html coverage.info)
            endif()
        else()
            message(WARNING "Lcov not found, coverage report not generated for libelement test app.")
        endif()
    endif()
endif ()

target_compile_definitions(libelement
    PRIVATE
        ${LEGACY_COMPILER}
)