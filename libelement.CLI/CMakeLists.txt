cmake_minimum_required(VERSION 3.8)

project(libelement_cli 
	VERSION 1.0.0 
	LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (ELEMENT_LIB_DIR "../libelement")
set (DEPENDENCIES_DIR "../dependencies")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(RapidJSON)
#find_package(CLI11)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Required to copy *.toml file for use during debug
configure_file("../Common/Messages.toml" "config/Messages.toml" COPYONLY)

set(libelement_cli_sources

    "include/compiler_message.hpp"
    "include/command.hpp"
    "include/evaluate_command.hpp"
    "include/parse_command.hpp"
    "include/typeof_command.hpp"
    "include/message_codes.hpp"
    "include/compilation_input.hpp"
	
    "src/compiler_message.cpp"
    "src/command.cpp"
    "src/main.cpp"

	"config/Messages.toml"
)

add_subdirectory(${ELEMENT_LIB_DIR} "libelement")
add_subdirectory("dependencies/cli11") 

add_executable(${CMAKE_PROJECT_NAME} ${libelement_cli_sources})

#Required to copy *.toml file to output at runtime, not sure if this is the best approach, but works for now
add_custom_command (
	TARGET ${CMAKE_PROJECT_NAME} POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"config/Messages.toml"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/config/Messages.toml")

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${ELEMENT_LIB_DIR}/include")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${DEPENDENCIES_DIR}/rapidjson/include")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${DEPENDENCIES_DIR}/toml11")
target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE element)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE CLI11::CLI11)