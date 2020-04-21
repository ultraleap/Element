cmake_minimum_required(VERSION 3.8)

project(libelement_cli 
	VERSION 1.0.0 
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(ELEMENT_LIB_DIR "../libelement")
set(DEPENDENCIES_DIR "../dependencies")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(RapidJSON)
#find_package(CLI11)

set(libelement_cli_sources

    "include/compiler_message.hpp"
    "include/command.hpp"
    "include/evaluate_command.hpp"
    "include/parse_command.hpp"
    "include/typeof_command.hpp"
    "include/message_codes.hpp"
	
    "src/compiler_message.cpp"
    "src/command.cpp"
    "src/main.cpp"
)

add_subdirectory(${ELEMENT_LIB_DIR} "libelement")
add_subdirectory("dependencies/cli11" EXCLUDE_FROM_ALL) 

add_executable(${CMAKE_PROJECT_NAME} ${libelement_cli_sources})

set_target_properties(libelement_cli PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY       "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_custom_command (
	TARGET libelement_cli POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/../Common"
	"${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${ELEMENT_LIB_DIR}/include")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${DEPENDENCIES_DIR}/rapidjson/include")
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC "${DEPENDENCIES_DIR}/toml11")
target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE element CLI11::CLI11)