cmake_minimum_required(VERSION 3.20)
project(tire_auralization LANGUAGES C CXX)

add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(ext/glfw)

# openal soft
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "" FORCE)
add_subdirectory(ext/openal-soft)

# opengl
find_package(OpenGL REQUIRED)

# glad
add_library(glad ext/glad/src/gl.c)
target_include_directories(glad PUBLIC ext/glad/include)

# imgui
file(GLOB IMGUI_WILDCARD_SOURCES "ext/imgui/*.cpp")
add_library(imgui 
	${IMGUI_WILDCARD_SOURCES}
	ext/imgui/backends/imgui_impl_glfw.cpp
	ext/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
	ext/imgui
	ext/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw glad)

# main executable
file(GLOB WILDCARD_SOURCES "src/*.cpp")
add_executable(tire_auralization ${WILDCARD_SOURCES})
target_include_directories(tire_auralization PRIVATE
	ext/glad/include
	ext/imgui
	ext/imgui/backends
)
target_link_libraries(tire_auralization PRIVATE
	${CMAKE_DL_LIBS}
	glfw
	glad
	imgui
	OpenAL::OpenAL
	OpenGL::GL
)
