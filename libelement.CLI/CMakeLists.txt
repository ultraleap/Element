cmake_minimum_required(VERSION 3.8)

project(libelement_cli 
	VERSION 1.0.0 
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (ELEMENT_LIB_DIR "../libelement")
set (DEPENDENCIES_DIR "dependencies")
set (CLI11_DIR "${DEPENDENCIES_DIR}/cli11")

find_package(RapidJSON)
#find_package(CLI11 CONFIG REQUIRED) #TEMP Can't make this work as my CMake-foo is subpar, so will do the nastier add_subdirectory for now

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(libelement_cli_sources

    "include/compiler_message.hpp"
    "include/base_command.hpp"
    "include/evaluate_command.hpp"
    "include/parse_command.hpp"
    "include/typeof_command.hpp"
	
    "src/compiler_message.cpp"
    "src/main.cpp"
)

add_subdirectory(${ELEMENT_LIB_DIR} "libelement")
add_subdirectory("${DEPENDENCIES_DIR}/cli11") #TEMP 

add_executable(libelement_cli ${libelement_cli_sources})

target_include_directories(libelement_cli PUBLIC "${ELEMENT_LIB_DIR}/include")
target_include_directories(libelement_cli PUBLIC "${DEPENDENCIES_DIR}/rapidjson/include")
target_include_directories(libelement_cli
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(libelement_cli PRIVATE element)
target_link_libraries(libelement_cli PRIVATE CLI11::CLI11) #TEMP 