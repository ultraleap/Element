cmake_minimum_required(VERSION 3.14)

project(libelement_cli 
	VERSION 1.0.0 
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (ELEMENT_LIB_DIR "../libelement")
set (DEPENDENCIES_DIR "dependencies")

find_package(RapidJSON)
find_package(CLI11 CONFIG)
#find_library(ELEMENT_LIB 
#	NAMES element 
#	HINTS "${ELEMENT_LIB_DIR}")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(libelement_cli_sources

    "include/compiler_message.h"

    "src/compiler_message.cpp"
    "src/main.cpp"
)

add_subdirectory(${ELEMENT_LIB_DIR} "libelement")

add_executable(libelement_cli ${libelement_cli_sources})

target_link_libraries(libelement_cli PRIVATE element)
target_include_directories(libelement_cli PUBLIC "${ELEMENT_LIB_DIR}/include")
#target_include_directories(libelement_cli PUBLIC "${DEPENDENCIES_DIR}/rapidjson/include")
#target_include_directories(libelement_cli PUBLIC "${DEPENDENCIES_DIR}/cli11/include")