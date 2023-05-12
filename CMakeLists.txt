cmake_minimum_required(VERSION 3.16.3)
project(test_imgui)

# set(BUILD_EXAMPLES TRUE)
add_subdirectory("libs")

set(SRC_FOLDER "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_FOLDER "${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE "SOURCES" "${SRC_FOLDER}/**.cpp")

add_executable(main "${CMAKE_SOURCE_DIR}/main.cpp" "${SOURCES}")
target_include_directories(main PRIVATE "${INCLUDE_FOLDER}")
target_link_libraries(main PRIVATE IMGUI)

add_custom_target(copy_graph_data
    # TARGET main POST_BUILD
    COMMENT "Copying sparse graph to a build directory: ${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/sparse_graph_data.txt" "${CMAKE_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/graph_data.txt" "${CMAKE_BINARY_DIR}"
)
add_dependencies(main copy_graph_data)