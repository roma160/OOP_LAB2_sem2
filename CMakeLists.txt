cmake_minimum_required(VERSION 3.16.3)
project(test_imgui)

# set(BUILD_EXAMPLES TRUE)
add_subdirectory("imgui-cmake")

set(SRC_FOLDER "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE "SOURCES" "${SRC_FOLDER}/**.cpp")
add_executable(main WIN32 "${CMAKE_SOURCE_DIR}/main.cpp" "${SOURCES}")
target_include_directories(main PRIVATE "${SRC_FOLDER}")
target_link_libraries(main IMGUI)

add_executable(test "${CMAKE_SOURCE_DIR}/test.cpp" "${SOURCES}")
target_include_directories(test PRIVATE "${SRC_FOLDER}")
