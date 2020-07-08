cmake_minimum_required(VERSION 3.11)

# For VS
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(libelement)

set(CMAKE_CXX_STANDARD 17)

set(LOG_VERBOSITY "log_flags::none" CACHE STRING "logging flags")

configure_file (
    "src/configuration.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/configuration.hpp"
)

set(libelement_sources
    "src/token.cpp"
    "src/interpreter.cpp"
    "src/construct.cpp"
    "src/construct.hpp"
    "src/common.cpp"

    "src/common_internal.hpp"
    "src/interpreter_internal.hpp"
    "src/token_internal.hpp"

    #AST
    "src/ast/ast.cpp"
    "src/ast/ast_internal.hpp"
    "src/ast/ast_indexes.hpp"
    "src/ast/fwd.hpp"
    "src/ast/scope.cpp"
    "src/ast/scope.hpp"
    
    #Object model
    "src/obj_model/fwd.hpp"
    "src/obj_model/errors.hpp"

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

    "src/obj_model/functions.cpp"
    "src/obj_model/functions.hpp"

    "src/obj_model/types.cpp"
    "src/obj_model/types.hpp"

    "src/obj_model/intermediaries.hpp"
    "src/obj_model/intermediaries.cpp"

    "src/obj_model/scope.hpp"
    "src/obj_model/scope.cpp"

    "src/obj_model/metainfo.cpp"

    #Expression tree
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
target_link_libraries(libelement PRIVATE utf8cpp fmt::fmt-header-only)

if (UNIX)
    target_link_libraries(libelement PRIVATE "m")
endif ()

if (BUILD_TESTING)
    add_executable(libelement_test_app
        "test/main.c")
    target_link_libraries(libelement_test_app PRIVATE libelement)

    set_target_properties(libelement_test_app PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY       "${debug_dir}")
endif ()

target_compile_definitions(libelement
    PRIVATE
        ${LEGACY_COMPILER}
)