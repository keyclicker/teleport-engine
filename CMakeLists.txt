cmake_minimum_required(VERSION 3.15)
project(teleport_engine)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(teleport_engine Game/main.cpp)

target_link_libraries(teleport_engine glfw)
target_link_libraries(teleport_engine OpenGL::GL)