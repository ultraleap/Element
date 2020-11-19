set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Look for an executable called sphinx-build
find_program(SPHINX_EXECUTABLE
        NAMES sphinx-build sphinx-build.exe
        PATHS
        "${BUILD_DIR}/venv/bin"
        "${BUILD_DIR}/venv/Scripts"
        DOC "Path to sphinx-build executable")

include(FindPackageHandleStandardArgs)

# Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(Sphinx
        "Failed to find sphinx-build executable"
        SPHINX_EXECUTABLE)