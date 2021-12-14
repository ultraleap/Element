cmake_minimum_required(VERSION 3.12)

project(lmnt)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DetectTargetArch)

option(LMNT_BUILD_JIT "Build the DynASM-based JIT" ON)
set(LMNT_JIT_ARCHITECTURES "NATIVE" CACHE STRING "List of architectures to build the JIT for")
# check if a JIT for the current native architecture exists
# if so, set that arch as a JIT target if specified; if not, just remove native as a JIT target
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/jit/jit-${LMNT_TARGET_ARCH}.dasc")
    list(TRANSFORM LMNT_JIT_ARCHITECTURES REPLACE "NATIVE" "${LMNT_TARGET_ARCH}")
else ()
    list(REMOVE_ITEM LMNT_JIT_ARCHITECTURES "NATIVE")
endif ()
list(REMOVE_DUPLICATES LMNT_JIT_ARCHITECTURES)

# Enable ASAN if requested
option(LMNT_BUILD_WITH_ASAN "Build with the GCC/Clang address sanitizer enabled" OFF)

# Configure linking to a math library
set (DEFAULT_MATH_LIBS)
if (UNIX)
    set (DEFAULT_MATH_LIBS "m")
endif ()
set(LMNT_MATH_LIBRARIES "${DEFAULT_MATH_LIBS}" CACHE STRING "Math libraries to link against (e.g. m on Unix-likes)")

# Add main library
add_subdirectory("src")
add_subdirectory("include")

# Add tests if required
option(BUILD_TESTING "Build tests" OFF)
include(CTest)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory("test")
    add_subdirectory("testapp")
endif ()

