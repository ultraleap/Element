cmake_minimum_required(VERSION 3.11.0)

include(FetchContent)

FetchContent_Declare(
    ghc_filesystem
    GIT_REPOSITORY https://github.com/gulrak/filesystem
    GIT_TAG v1.3.2
    GIT_SHALLOW TRUE
    CMAKE_ARGS
        -DGHC_FILESYSTEM_BUILD_TESTING=OFF
        -DGHC_FILESYSTEM_BUILD_EXAMPLES=OFF
        -DGHC_FILESYSTEM_WITH_INSTALL=OFF
)
FetchContent_GetProperties(ghc_filesystem)
if(NOT ghc_filesystem_POPULATED)
    FetchContent_Populate(ghc_filesystem)
    add_subdirectory("${ghc_filesystem_SOURCE_DIR}" "${ghc_filesystem_BINARY_DIR}" EXCLUDE_FROM_ALL)
endif()
