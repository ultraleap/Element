cmake_minimum_required(VERSION 3.12)

set(lmnt_sources
    "archive.c"
    "validation.c"
    "extcalls.c"
    "interpreter.c"
    "opcodes.c"
    "ops_bounds.c"
    "ops_branch.c"
    "ops_fncall.c"
    "ops_math.c"
    "ops_misc.c"
    "ops_trig.c"

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

if (LMNT_BUILD_JIT)
    add_subdirectory("jit")
    target_sources(lmnt PRIVATE $<TARGET_OBJECTS:lmnt_jit>)
endif ()