cmake_minimum_required(VERSION 3.12)

target_include_directories(lmnt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(lmnt PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/common.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/archive.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/validation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/extcalls.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/interpreter.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/opcodes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/ops_bounds.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/ops_branch.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/ops_fncall.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/ops_math.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/ops_misc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/ops_trig.h"
)

if (LMNT_TARGET_ARCH STREQUAL "x86_64")
    target_sources(lmnt PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/x86_64/platform.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/x86_64/memcpy_fast.h"
    )
elseif (LMNT_TARGET_ARCH STREQUAL "armv7a" OR LMNT_TARGET_ARCH STREQUAL "armv7m")
    target_sources(lmnt PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/arm/platform.h"
    )
endif ()

if (LMNT_BUILD_JIT)
    target_sources(lmnt PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/lmnt/jit.h"
    )
endif ()