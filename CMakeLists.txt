cmake_minimum_required(VERSION 3.10)
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
project(GLIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to an optimised build: the algorithm benchmarks shown in the
# app are only meaningful with optimisation turned on.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Platform-specific GLUT configuration
if(APPLE)
    # macOS uses the built-in GLUT framework
    find_package(GLUT REQUIRED)
    set(GLUT_LIBRARIES ${GLUT_LIBRARIES})
    set(GLUT_INCLUDE_DIRS ${GLUT_INCLUDE_DIR})
elseif(WIN32)
    # Prefer the vcpkg config targets so a static build gets FREEGLUT_STATIC
    # and the required Windows system libraries from FreeGLUT itself.
    find_package(FreeGLUT CONFIG QUIET)
    if(TARGET FreeGLUT::freeglut_static)
        set(GLUT_LIBRARIES FreeGLUT::freeglut_static)
    elseif(TARGET FreeGLUT::freeglut)
        set(GLUT_LIBRARIES FreeGLUT::freeglut)
    else()
        find_package(GLUT REQUIRED)
    endif()
elseif(UNIX)
    # Linux: use freeglut
    find_package(GLUT REQUIRED)
    set(GLUT_LIBRARIES ${GLUT_LIBRARIES})
endif()

# Source files
# Note: Algorithm and Graph classes are header-only for simplicity.
# Only main.cpp needs compilation since the Application class is also
# defined entirely in its header (keeping the build simple for students).
set(SOURCES
    src/main.cpp
)

# Create executable
add_executable(glide ${SOURCES})

# Include directories
target_include_directories(glide PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    src/
)

# Link libraries
target_link_libraries(glide
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
)

# macOS-specific settings
if(APPLE)
    # Suppress deprecation warnings for GLUT on macOS
    target_compile_definitions(glide PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(glide PRIVATE /W3)
else()
    target_compile_options(glide PRIVATE -Wall -Wextra -Wno-deprecated-declarations)
endif()
