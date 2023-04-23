cmake_minimum_required(VERSION 3.16.3)
project(test_imgui)

# set(BUILD_EXAMPLES TRUE)
add_subdirectory("libs")

set(SRC_FOLDER "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_FOLDER "${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE "SOURCES" "${SRC_FOLDER}/**.cpp")

add_executable(main WIN32 "${CMAKE_SOURCE_DIR}/main.cpp" "${SOURCES}")
message("Hey there")
message("${INCLUDE_FOLDER} ${CMAKE_SOURCE_DIR}/libs/imgui")
target_include_directories(main PRIVATE "${INCLUDE_FOLDER}" "${CMAKE_SOURCE_DIR}/libs/imgui")
target_link_libraries(main PRIVATE IMGUI)