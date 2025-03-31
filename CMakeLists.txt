cmake_minimum_required(VERSION 3.14)
project(SubChat LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find OpenGL
find_package(OpenGL REQUIRED)

find_package(Freetype REQUIRED)

# Find GLEW
find_package(GLEW REQUIRED)

# GLFW configuration
if (NOT GLFW_DIR)
    set(GLFW_DIR "${CMAKE_SOURCE_DIR}/submodules/glfw") # Use a path relative to the source directory
endif ()


option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)

# Build GLFW in its own folder inside the binary directory
add_subdirectory(${GLFW_DIR} ${CMAKE_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)
include_directories(${GLFW_DIR}/deps)

# Dear ImGui configuration
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/submodules/imgui")
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

set(TINYXML_DIR "${CMAKE_SOURCE_DIR}/submodules/tinyxml2")
include_directories(${TINYXML_DIR})

set(UTFCPP_DIR "${CMAKE_SOURCE_DIR}/submodules/utfcpp/source")
include_directories(${UTFCPP_DIR})



include_directories(${FREETYPE_INCLUDE_DIRS})

# Gather source files
file(GLOB sources "${CMAKE_SOURCE_DIR}/*.cpp")

# Create the executable
add_executable(${PROJECT_NAME}
        ${sources}
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/misc/freetype/imgui_freetype.cpp
        ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
        ${TINYXML_DIR}/tinyxml2.cpp


)

# Link with GLFW, OpenGL and GLEW
target_link_libraries(${PROJECT_NAME} glfw OpenGL::GL GLEW::GLEW Freetype::Freetype)
