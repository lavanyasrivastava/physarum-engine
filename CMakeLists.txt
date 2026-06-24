cmake_minimum_required(VERSION 3.10)
project(PhysarumEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
include_directories(extern/glad/include)

# Find GLFW (installed via Homebrew)
find_package(glfw3 REQUIRED)

# Build the executable — include GLAD's .c source too
add_executable(PhysarumEngine main.cpp extern/glad/src/glad.c)

# Link GLFW (and OpenGL framework on macOS)
target_link_libraries(PhysarumEngine glfw)

if(APPLE)
    target_link_libraries(PhysarumEngine "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(PhysarumEngine OpenGL::GL)
endif()