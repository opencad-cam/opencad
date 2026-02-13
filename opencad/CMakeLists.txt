cmake_minimum_required(VERSION 3.20)
project(OpenCAD VERSION 0.1.0 LANGUAGES CXX)

# ============================================================================
# C++ Standard
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt Automoc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# MinGW compatibility - M_PI definition
add_definitions(-D_USE_MATH_DEFINES)

# ============================================================================
# Build Options
# ============================================================================
option(OPENCAD_BUILD_TESTS "Build unit tests" ON)
option(OPENCAD_BUILD_PYTHON "Build Python bindings" ON)
option(OPENCAD_USE_QT6 "Use Qt6 (otherwise Qt5)" ON)

# ============================================================================
# Output Directories
# ============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Dependencies
# ============================================================================

# OpenCASCADE
find_package(OpenCASCADE REQUIRED)
if(OpenCASCADE_FOUND)
    message(STATUS "OpenCASCADE found: ${OpenCASCADE_INCLUDE_DIR}")
    include_directories(${OpenCASCADE_INCLUDE_DIR})
endif()

# Qt6/Qt5
if(OPENCAD_USE_QT6)
    find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets Network REQUIRED)
    set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Network)
else()
    find_package(Qt5 COMPONENTS Core Widgets OpenGL Network REQUIRED)
    set(QT_LIBS Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Network)
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# Eigen3 for constraint solver
find_package(Eigen3 CONFIG REQUIRED)
message(STATUS "Eigen3 found: ${EIGEN3_INCLUDE_DIR}")

# ============================================================================
# Include Directories
# ============================================================================
include_directories(${CMAKE_SOURCE_DIR}/src)

# ============================================================================
# Subdirectories
# ============================================================================
add_subdirectory(src/core)
add_subdirectory(src/io)
add_subdirectory(src/sketch)
add_subdirectory(src/part)
add_subdirectory(src/assembly)
add_subdirectory(src/ai)
add_subdirectory(src/ui)

# Main Application
add_subdirectory(src/app)

# ============================================================================
# Tests
# ============================================================================
if(OPENCAD_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# Installation
# ============================================================================

# Install the executable
install(TARGETS opencad DESTINATION bin)

# Install the scripts directory
install(DIRECTORY scripts DESTINATION bin)

# Install README
install(FILES README.md DESTINATION .)

# Run windeployqt to deploy Qt dependencies
if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${QT_PATH}/bin)
    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            message(STATUS \"Running windeployqt...\")
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                        --no-translations
                        --compiler-runtime
                        --dir \"\${CMAKE_INSTALL_PREFIX}/bin\"
                        \"\${CMAKE_INSTALL_PREFIX}/bin/opencad.exe\"
            )
        ")
    else()
        message(WARNING "windeployqt not found! Installer will not include Qt dependencies.")
    endif()
endif()

# ============================================================================
# CPack Configuration
# ============================================================================
set(CPACK_PACKAGE_NAME "OpenCAD")
set(CPACK_PACKAGE_VENDOR "OpenCAD Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Source CAD and Simulation Tool")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenCAD")
set(CPACK_PACKAGE_EXECUTABLES "opencad" "OpenCAD")
set(CPACK_CREATE_DESKTOP_LINKS "opencad")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

include(CPack)

message(STATUS "")
message(STATUS "=== OpenCAD Configuration ===")
message(STATUS "Version:     ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version:  ${OPENCAD_USE_QT6}")
message(STATUS "Build Tests: ${OPENCAD_BUILD_TESTS}")
message(STATUS "Python:      ${OPENCAD_BUILD_PYTHON}")
message(STATUS "Packing:     CPack enabled")
message(STATUS "=============================")
