cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0079 NEW)

include(FetchDynASM)

string(REPLACE ";" ", " jit_arch_list "${LMNT_JIT_ARCHITECTURES}")
message(STATUS "Building LMNT JIT for: ${jit_arch_list}")

set(jit_sources
    "jit.c"

    "jithelpers.h"
    "hosthelpers.h"
)

if (LMNT_TARGET_ARCH STREQUAL "x86_64")
    list(APPEND jit_sources
        "targethelpers-x86.h"
        "reghelpers-x86.h"
    )
elseif (LMNT_TARGET_ARCH STREQUAL "armv7a" OR LMNT_TARGET_ARCH STREQUAL "armv7m")
    list(APPEND jit_sources
        "targethelpers-arm.h"
        "reghelpers-arm-vfp.h"
    )
endif ()

set (jit_inputs)
set (jit_outputs)
foreach (arch IN LISTS LMNT_JIT_ARCHITECTURES)
    string(TOLOWER "${arch}" arch_lower)
    set(jit_input "${CMAKE_CURRENT_SOURCE_DIR}/jit-${arch_lower}.dasc")
    set(jit_output "${CMAKE_CURRENT_BINARY_DIR}/jit-${arch_lower}.c")

    if (NOT EXISTS "${jit_input}")
        message(FATAL_ERROR "There is no LMNT JIT implementation for specified architecture ${arch}!")
    endif ()

    set (jit_flags "TARGET_${arch}" "HOST_${LMNT_TARGET_ARCH}")
    if (WIN32)
        list (APPEND jit_flags "TARGET_WIN" "HOST_WIN")
    endif ()
    
    add_dasm_compile_command("${jit_input}" "${jit_output}" ${jit_flags})
    list(APPEND jit_inputs "${jit_input}")
    list(APPEND jit_outputs "${jit_output}")
endforeach ()

add_library(lmnt_jit OBJECT ${jit_sources} ${jit_outputs} ${jit_inputs})
target_include_directories(lmnt_jit PRIVATE $<TARGET_PROPERTY:lmnt,INCLUDE_DIRECTORIES>)

target_link_libraries(lmnt_jit PRIVATE dynasm)
foreach (arch IN LISTS LMNT_JIT_ARCHITECTURES)
    string(TOUPPER "${arch}" arch_upper)
    target_compile_definitions(lmnt_jit PRIVATE "LMNT_JIT_HAS_${arch_upper}=1")
endforeach()