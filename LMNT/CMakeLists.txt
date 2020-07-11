cmake_minimum_required(VERSION 3.12)

project(lmnt)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DetectTargetArch)

option(LMNT_BUILD_JIT "Build the DynASM-based JIT" ON)
set(LMNT_JIT_ARCHITECTURES "NATIVE" CACHE STRING "List of architectures to build the JIT for")

set(lmnt_sources
    "src/lmnt/archive.c"
    "src/lmnt/validation.c"
    "src/lmnt/extcalls.c"
    "src/lmnt/interpreter.c"
    "src/lmnt/opcodes.c"
    "src/lmnt/ops_branch.c"
    "src/lmnt/ops_fncall.c"
    "src/lmnt/ops_math.c"
    "src/lmnt/ops_misc.c"
    "src/lmnt/ops_trig.c"
    "src/lmnt/ops_util.c"

    "src/lmnt/helpers.h"

    "include/lmnt/common.h"
    "include/lmnt/config.h"
    "include/lmnt/platform.h"
    "include/lmnt/archive.h"
    "include/lmnt/validation.h"
    "include/lmnt/extcalls.h"
    "include/lmnt/interpreter.h"
    "include/lmnt/opcodes.h"
    "include/lmnt/ops_branch.h"
    "include/lmnt/ops_fncall.h"
    "include/lmnt/ops_math.h"
    "include/lmnt/ops_misc.h"
    "include/lmnt/ops_trig.h"
    "include/lmnt/ops_util.h"
)

if (LMNT_TARGET_ARCH STREQUAL "x86_64")
    list(APPEND lmnt_sources
        "include/lmnt/x86_64/platform.h"
        "include/lmnt/x86_64/memcpy_fast.h"
        "src/lmnt/jit/targethelpers-x86.h"
        "src/lmnt/jit/reghelpers-x86.h"
    )
elseif (LMNT_TARGET_ARCH STREQUAL "armv7a" OR LMNT_TARGET_ARCH STREQUAL "armv7m")
    list(APPEND lmnt_sources
        "include/lmnt/arm/platform.h"
        "src/lmnt/jit/targethelpers-arm.h"
        "src/lmnt/jit/reghelpers-arm-vfp.h"
    )
endif ()

list(TRANSFORM LMNT_JIT_ARCHITECTURES REPLACE "NATIVE" "${LMNT_TARGET_ARCH}")
list(REMOVE_DUPLICATES LMNT_JIT_ARCHITECTURES)

if(LMNT_BUILD_JIT)
    include(FetchDynASM)

    string(REPLACE ";" ", " jit_arch_list "${LMNT_JIT_ARCHITECTURES}")
    message(STATUS "Building JIT for: ${jit_arch_list}")

    list(APPEND lmnt_sources
        "src/lmnt/jit.c"

        "src/lmnt/jit/jithelpers.h"
        "src/lmnt/jit/hosthelpers.h"

        "include/lmnt/jit.h"
    )

    set (jit_inputs)
    set (jit_outputs)
    foreach (arch IN LISTS LMNT_JIT_ARCHITECTURES)
        set(lmnt_jit_input "${CMAKE_CURRENT_SOURCE_DIR}/src/lmnt/jit/jit-${arch}.dasc")
        set(lmnt_jit_output "${CMAKE_CURRENT_BINARY_DIR}/jit-${arch}.c")

        set (lmnt_jit_flags "TARGET_${arch}" "HOST_${LMNT_TARGET_ARCH}")
        if (WIN32)
            list (APPEND lmnt_jit_flags "TARGET_WIN" "HOST_WIN")
        endif ()
        
        add_dasm_compile_command("${lmnt_jit_input}" "${lmnt_jit_output}" ${lmnt_jit_flags})
        list(APPEND jit_inputs "${lmnt_jit_input}")
        list(APPEND jit_outputs "${lmnt_jit_output}")
    endforeach ()
endif ()

add_library(lmnt STATIC ${lmnt_sources} ${jit_outputs} ${jit_inputs})
target_include_directories(lmnt
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

if (LMNT_BUILD_JIT)
    target_link_libraries(lmnt PRIVATE dynasm)
    target_compile_options(lmnt PRIVATE "${LMNT_TARGET_COMPILE_FLAGS}")
    foreach (arch IN LISTS LMNT_JIT_ARCHITECTURES)
        string(TOUPPER "${arch}" arch_upper)
        target_compile_definitions(lmnt PRIVATE "LMNT_JIT_HAS_${arch_upper}=1")
    endforeach()
endif ()

if (UNIX)
    target_link_libraries(lmnt PRIVATE "m")
endif ()

option(BUILD_TESTING "Build tests" OFF)
include(CTest)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory("test")
    add_subdirectory("testapp")
endif ()
