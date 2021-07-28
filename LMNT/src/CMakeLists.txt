cmake_minimum_required(VERSION 3.12)

set(lmnt_sources
    "archive.c"
    "validation.c"
    "extcalls.c"
    "interpreter.c"
    "opcodes.c"

    "ops_bounds_impl.h"
    "ops_branch_impl.h"
    "ops_fncall_impl.h"
    "ops_math_impl.h"
    "ops_misc_impl.h"
    "ops_trig_impl.h"

    "helpers.h"
)

add_library(lmnt STATIC ${lmnt_sources})
target_include_directories(lmnt
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

if (UNIX)
    target_link_libraries(lmnt PRIVATE "m")
endif ()

if (LMNT_BUILD_WITH_ASAN)
    target_compile_options(lmnt PUBLIC "-fsanitize=address")
    target_link_options(lmnt PUBLIC "-fsanitize=address" "-static-libasan")
endif ()

if (LMNT_BUILD_JIT)
    add_subdirectory("jit")
    target_sources(lmnt PRIVATE $<TARGET_OBJECTS:lmnt_jit>)
endif ()