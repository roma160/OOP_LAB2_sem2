cmake_minimum_required(VERSION 3.16.3)
project(test_imgui)

# set(BUILD_EXAMPLES TRUE)
add_subdirectory("imgui-cmake")

add_executable(main WIN32 main.cpp)
target_link_libraries(main IMGUI)

add_executable(test test.cpp)