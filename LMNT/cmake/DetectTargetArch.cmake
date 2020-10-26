include ( CheckCSourceCompiles )

check_c_source_compiles("
int main()
{
#if defined(__aarch64__) || defined(_M_ARM64)
  return 0;
#else
  #error Not ARM
#endif
}
"
HAS_ARM64)

check_c_source_compiles("
int main()
{
#if defined(__arm__) || defined(_M_ARM)
  return 0;
#else
  #error Not ARM
#endif
}
"
HAS_ARM)

set(ARMV7A_COMPILE_FLAGS "-marm" "-march=armv7-a" "-mfloat-abi=hard")
string(JOIN " " CMAKE_REQUIRED_FLAGS ${ARMV7A_COMPILE_FLAGS})
check_c_source_compiles("
int main()
{
#if defined(__ARM_ARCH_7A__)
  return 0;
#else
  #error Not ARMv7-A
#endif
}
"
HAS_ARMV7A)
set(CMAKE_REQUIRED_FLAGS)

set(ARMV7M_COMPILE_FLAGS "-mthumb" "-march=armv7-m" "-mfloat-abi=hard")
string(JOIN " " CMAKE_REQUIRED_FLAGS ${ARMV7M_COMPILE_FLAGS})
check_c_source_compiles("
int main()
{
#if defined(__ARM_ARCH_7M__)
  return 0;
#else
  #error Not ARMv7-M
#endif
}
"
HAS_ARMV7M)
set(CMAKE_REQUIRED_FLAGS)

check_c_source_compiles("
int main()
{
#if defined(__i386) || defined(_M_IX86)
  return 0;
#else
  #error Not x86_32
#endif
}
"
HAS_X86)

check_c_source_compiles("
int main()
{
#if defined(__x86_64__) || defined(_M_X64)
  return 0;
#else
  #error Not x86_64
#endif
}
"
HAS_AMD64)

if (HAS_AMD64)
    set(TARGET_ARCH "x86_64")
elseif (HAS_X86)
    set(TARGET_ARCH "x86")
elseif (HAS_ARM64)
    set(TARGET_ARCH "arm64")
elseif (HAS_ARMV7A)
    set(TARGET_ARCH "armv7a")
elseif (HAS_ARMV7M)
    set(TARGET_ARCH "armv7m")
elseif (HAS_ARM)
    set(TARGET_ARCH "arm")
else ()
    set(TARGET_ARCH "unknown")
endif ()
if ("${LMNT_TARGET_ARCH}" STREQUAL "")
    set (LMNT_TARGET_ARCH "${TARGET_ARCH}" CACHE INTERNAL "Target CPU architecture" FORCE )
endif ()
message(STATUS "Target architecture: ${LMNT_TARGET_ARCH}")

if ("${LMNT_TARGET_ARCH}" STREQUAL "armv7a")
    set(TARGET_COMPILE_FLAGS "${ARMV7A_COMPILE_FLAGS}")
elseif ("${LMNT_TARGET_ARCH}" STREQUAL "armv7m")
    set(TARGET_COMPILE_FLAGS "${ARMV7M_COMPILE_FLAGS}")
endif ()

if ("${LMNT_TARGET_COMPILE_FLAGS}" STREQUAL "")
    set (LMNT_TARGET_COMPILE_FLAGS "${TARGET_COMPILE_FLAGS}" CACHE INTERNAL "Target CPU compile flags" FORCE )
endif ()
