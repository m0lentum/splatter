cmake_minimum_required(VERSION 3.13)
set(EXEC_NAME "test")
project(${EXEC_NAME})

find_package(glfw3 3.2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${EXEC_NAME} ${SOURCES})

target_include_directories(${EXEC_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(${EXEC_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(${EXEC_NAME} glfw)
target_link_libraries(${EXEC_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${EXEC_NAME} ${OPENGL_gl_LIBRARY})

target_link_libraries(${EXEC_NAME} X11)
target_link_libraries(${EXEC_NAME} Xrandr)
target_link_libraries(${EXEC_NAME} Xi)
target_link_libraries(${EXEC_NAME} Xxf86vm)
target_link_libraries(${EXEC_NAME} GL)
