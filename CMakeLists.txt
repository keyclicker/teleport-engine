cmake_minimum_required(VERSION 3.15)
project(teleport_engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

#find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SFML COMPONENTS system window graphics network audio)

include_directories(${SFML_INCLUDE_DIR})

add_executable(teleport_engine Game/main.cpp Engine/Game.cpp Engine/Utils/Math.cpp Engine/Renderer.cpp Engine/Utils/Clip.cpp)

#target_link_libraries(teleport_engine glfw)
target_link_libraries(teleport_engine OpenGL::GL)
target_link_libraries(teleport_engine ${OPENGL_LIBRARIES} sfml-graphics sfml-window sfml-system)