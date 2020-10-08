cmake_minimum_required(VERSION 3.14)

project("ElementDocumentation")

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})


## --------------------- Gather Project Files for Doxygen ---------------------

set(SPHINX_RST
        "index.rst"
        "Overview.rst"
        "UserGuide.rst"
        "element_dotnet/index.rst"
        "libelement/index.rst"
        "PyElement/index.rst")

file(GLOB_RECURSE ELEMENT_DOTNET_SOURCES
        ${CMAKE_SOURCE_DIR}/../Element.NET/*.cs)

foreach (file IN LISTS ELEMENT_DOTNET_SOURCES)
    string(APPEND ELEMENT_DOTNET_FILES " \"${file}\"")
endforeach()

file(GLOB_RECURSE LIBELEMENT_SOURCES
        ${CMAKE_SOURCE_DIR}/../libelement/src/*.cpp
        ${CMAKE_SOURCE_DIR}/../libelement/src/*.hpp)

foreach (file IN LISTS LIBELEMENT_SOURCES)
    string(APPEND LIBELEMENT_FILES " \"${file}\"")
endforeach()

file(GLOB_RECURSE PYELEMENT_SOURCES
        ${CMAKE_SOURCE_DIR}/../PyElement/*.py)

foreach (file IN LISTS PYELEMENT_SOURCES)
    string(APPEND PYELEMENT_FILES " \"${file}\"")
endforeach()


## --------------------- Doxygen Target ---------------------

find_package(Doxygen REQUIRED)

# Create the relevant doxyfiles for each language
set(DOXYGEN_INPUT_FILES ${ELEMENT_DOTNET_FILES})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/DotnetDoxyfile")
set(DOXYGEN_INPUT_FILES ${LIBELEMENT_FILES})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/LibDoxyfile")
set(DOXYGEN_INPUT_FILES ${PYELEMENT_FILES})
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/PyDoxyfile")

set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc)
set(ELEMENT_DOTNET_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/dotnet)
set(LIBELEMENT_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/libelement)
set(PYELEMENT_OUTPUT_DIR ${DOXYGEN_OUTPUT_DIR}/pyelement)

# Build all directories required and copy images to correct location
add_custom_command(
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${ELEMENT_DOTNET_OUTPUT_DIR}
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${LIBELEMENT_OUTPUT_DIR}
        COMMAND "${CMAKE_COMMAND}" -E make_directory ${PYELEMENT_OUTPUT_DIR}
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXYGEN_OUTPUT_DIR}/img"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/img" "${DOXYGEN_OUTPUT_DIR}/img"
        COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/doc.stamp"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/doc.stamp"
)

# Build dotnet xml
add_custom_command(
        COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/DotnetDoxyfile"
        COMMAND "${CMAKE_COMMAND}" -E touch "${ELEMENT_DOTNET_OUTPUT_DIR}/doc.stamp"
        DEPENDS ${ELEMENT_DOTNET_SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/doc.stamp"
        WORKING_DIRECTORY "${ELEMENT_DOTNET_OUTPUT_DIR}"
        COMMENT "Generating element.NET documentation"
        OUTPUT "${ELEMENT_DOTNET_OUTPUT_DIR}/doc.stamp"
)

# Build libelement xml
add_custom_command(
        COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/LibDoxyfile"
        COMMAND "${CMAKE_COMMAND}" -E touch "${LIBELEMENT_OUTPUT_DIR}/doc.stamp"
        DEPENDS ${LIBELEMENT_SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/doc.stamp"
        WORKING_DIRECTORY "${LIBELEMENT_OUTPUT_DIR}"
        COMMENT "Generating element.NET documentation"
        OUTPUT "${LIBELEMENT_OUTPUT_DIR}/doc.stamp"
)

# Build libelement xml
add_custom_command(
        COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/PyDoxyfile"
        COMMAND "${CMAKE_COMMAND}" -E touch "${PYELEMENT_OUTPUT_DIR}/doc.stamp"
        DEPENDS ${PYELEMENT_SOURCES} "${CMAKE_CURRENT_BINARY_DIR}/doc.stamp"
        WORKING_DIRECTORY "${PYELEMENT_OUTPUT_DIR}"
        COMMENT "Generating element.NET documentation"
        OUTPUT "${PYELEMENT_OUTPUT_DIR}/doc.stamp"
)

add_custom_target(Doxygen_Generation
        DEPENDS "${ELEMENT_DOTNET_OUTPUT_DIR}/doc.stamp"
                "${LIBELEMENT_OUTPUT_DIR}/doc.stamp"
                "${PYELEMENT_OUTPUT_DIR}/doc.stamp")


## --------------------- Sphinx Installation ---------------------

find_package(Sphinx)
find_package(Python)

if (NOT ${Sphinx_FOUND})
    message(STATUS "Installing Python Packages for Sphinx Documentation")
    if (WIN32)
        execute_process(COMMAND py -3 -m pip install --user virtualenv)
        execute_process(COMMAND py -3 -m venv venv)
        execute_process(COMMAND ./venv/Scripts/pip3 install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt -q --upgrade)
    else ()
        execute_process(COMMAND python3 -m pip install --user virtualenv)
        execute_process(COMMAND python3 -m venv venv)
        execute_process(COMMAND ./venv/bin/pip3 install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt -q --upgrade)
    endif()

    message(STATUS "Installing Python Packages for Sphinx Documentation - Completed")
    find_package(Sphinx REQUIRED)
endif()

## --------------------- Documentation (Sphinx) Generation ---------------------

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD ${DOXYGEN_OUTPUT_DIR}/html)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

message("Sphinx_build: " ${SPHINX_BUILD})

add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
        COMMAND ${SPHINX_EXECUTABLE} -b html
        -D breathe_projects.ElementDotNET=${ELEMENT_DOTNET_OUTPUT_DIR}/xml
        -D breathe_projects.Libelement=${LIBELEMENT_OUTPUT_DIR}/xml
        -D breathe_projects.PyElement=${PYELEMENT_OUTPUT_DIR}/xml
        ${SPHINX_SOURCE} ${SPHINX_BUILD}
        DEPENDS
        ${SPHINX_RST}
        ${CMAKE_CURRENT_BINARY_DIR}/doc.stamp
        Doxygen_Generation
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Sphinx")

add_custom_target(Documentation DEPENDS ${SPHINX_INDEX_FILE})