cmake_minimum_required(VERSION 3.14)

project(libelement_cli 
	VERSION 1.0.0 
	DESCRIPTION "CLI for libelement" 
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(RapidJSON)
find_package(CLI11 CONFIG)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(libelement_cli_sources

    #"inc/stuff.h"
    #"src/stuff.cpp"

    "src/main.cpp"
)

add_executable(libelement_cli ${libelement_cli_sources})
target_include_directories(libelement_cli PUBLIC "dependencies/rapidjson/include")
target_include_directories(libelement_cli PUBLIC "dependencies/cli11/include")