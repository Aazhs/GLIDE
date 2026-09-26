cmake_minimum_required(VERSION 3.10)
project(GLIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Platform-specific GLUT configuration
if(APPLE)
    # macOS uses the built-in GLUT framework
    find_package(GLUT REQUIRED)
    set(GLUT_LIBRARIES ${GLUT_LIBRARIES})
    set(GLUT_INCLUDE_DIRS ${GLUT_INCLUDE_DIR})
elseif(WIN32)
    # Windows: look for freeglut
    find_package(GLUT REQUIRED)
    set(GLUT_LIBRARIES ${GLUT_LIBRARIES})
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
