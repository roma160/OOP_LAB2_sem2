cmake_minimum_required(VERSION 3.16.3)
project(test_imgui)

#=================== EMSCRIPTEN COMPILER SETTINGS ===================
# https://github.com/mosra/toolchains/blob/master/generic/Emscripten-wasm.cmake
if(EMSCRIPTEN)
    if(NOT EMSCRIPTEN_PREFIX)
        if(DEFINED ENV{EMSCRIPTEN})
            file(TO_CMAKE_PATH "$ENV{EMSCRIPTEN}" EMSCRIPTEN_PREFIX)
            # On Homebrew the sysroot is here, however emcc is also available in
            # /usr/local/bin. It's impossible to figure out the sysroot from there, so
            # try this first
        elseif(EXISTS "/usr/local/opt/emscripten/libexec/emcc")
            set(EMSCRIPTEN_PREFIX "/usr/local/opt/emscripten/libexec")
        # Look for emcc in the path as a last resort
        else()
            find_program(_EMSCRIPTEN_EMCC_EXECUTABLE emcc)
            # message(${_EMSCRIPTEN_EMCC_EXECUTABLE})
            if(EXISTS ${_EMSCRIPTEN_EMCC_EXECUTABLE})
                get_filename_component(EMSCRIPTEN_PREFIX ${_EMSCRIPTEN_EMCC_EXECUTABLE} DIRECTORY)
            else()
                message(FATAL_ERROR "Wasn't able to locate the Emscripten path")
            endif()
            mark_as_advanced(_EMSCRIPTEN_EMCC_EXECUTABLE)
        endif()
    endif()

    set(EMSCRIPTEN_TOOLCHAIN_PATH "${EMSCRIPTEN_PREFIX}/system")

    if(CMAKE_HOST_WIN32)
        set(EMCC_SUFFIX ".bat")
    else()
        set(EMCC_SUFFIX "")
    endif()

    # MSVC IntelliSense requires these variables to be put into cache:
    # https://devblogs.microsoft.com/cppblog/configure-intellisense-with-cmake-toolchain-files-in-visual-studio-2019-16-9-preview-2/
    set(CMAKE_C_COMPILER "${EMSCRIPTEN_PREFIX}/emcc${EMCC_SUFFIX}" CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "${EMSCRIPTEN_PREFIX}/em++${EMCC_SUFFIX}" CACHE FILEPATH "CXX compiler" FORCE)

    # The `CACHE PATH "bla"` *has to be* present as otherwise CMake < 3.13.0 would
    # for some reason forget the path to `ar`, calling it as `"" qc bla`, failing
    # with `/bin/sh: : command not found`. This is probably related to CMP0077 in
    # some way but I didn't bother investigating further.
    set(CMAKE_AR "${EMSCRIPTEN_PREFIX}/emar${EMCC_SUFFIX}" CACHE PATH "Path to Emscripten ar")
    set(CMAKE_RANLIB "${EMSCRIPTEN_PREFIX}/emranlib${EMCC_SUFFIX}" CACHE PATH "Path to Emscripten ranlib")

    set(CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}
        "${EMSCRIPTEN_TOOLCHAIN_PATH}"
        "${EMSCRIPTEN_PREFIX}")

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

    # Otherwise FindCorrade fails to find _CORRADE_MODULE_DIR. Why the heck is this
    # not implicit is beyond me.
    set(CMAKE_SYSTEM_PREFIX_PATH ${CMAKE_FIND_ROOT_PATH})

    # Compared to the classic (asm.js) compilation, -s WASM=1 is added to both
    # compiler and linker. The *_INIT variables are available since CMake 3.7, so
    # it won't work in earlier versions. Sorry.
    cmake_minimum_required(VERSION 3.7)
    set(USE_FLAGS "-s USE_SDL=2 -s USE_FREETYPE=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_INIT "-s WASM=1" ${USE_FLAGS})
    set(CMAKE_CXX_FLAGS_RELEASE_INIT "-DNDEBUG -O3" ${USE_FLAGS})
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "-O3 --llvm-lto 1" ${USE_FLAGS})
endif()


set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)
#=================== SDL ===================
set(SDL_DIR ${LIBS_DIR}/SDL)
set(SDL_STATIC TRUE)
set(SDL_SHARED FALSE)
add_subdirectory(${SDL_DIR})


#=================== IMGUI ===================

set(IMGUI_DIR ${LIBS_DIR}/imgui)
add_library(IMGUI STATIC)

target_sources(IMGUI
PUBLIC
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp

PRIVATE
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
)

target_include_directories(IMGUI
    PUBLIC ${IMGUI_DIR}
    PUBLIC ${IMGUI_DIR}/backends
    PUBLIC ${SDL_DIR}/include
)
if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    target_link_libraries(IMGUI
        PUBLIC SDL2-static SDL2main ${OPENGL_LIBRARIES}
    )
endif()


set(SRC_FOLDER "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_FOLDER "${CMAKE_SOURCE_DIR}/include")
file(GLOB_RECURSE "SOURCES" "${SRC_FOLDER}/**.cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
add_executable(main WIN32 "${CMAKE_SOURCE_DIR}/main.cpp" "${SOURCES}")
target_include_directories(main PRIVATE "${INCLUDE_FOLDER}")
target_link_libraries(main PRIVATE IMGUI)

if(EMSCRIPTEN)
    # file(GLOB_RECURSE EMSCRIPTEN_INDEX "${CMAKE_SOURCE_DIR}/emscripten.index/*")
    # add_custom_target(
    #     copy_index
    #     DEPENDS copy_index_command
    #     COMMENT "Copying index files to a build directory: ${CMAKE_BINARY_DIR}"
    # )
    # add_dependencies(main copy_index)

    set(
        JS_RESULT
        "${CMAKE_BINARY_DIR}/main.js"
        "${CMAKE_BINARY_DIR}/main.wasm"
    )
    add_custom_command(
        OUTPUT copy_index_command COMMAND ""
        TARGET main POST_BUILD
        COMMENT "Copying index files to a build directory: ${CMAKE_BINARY_DIR}"
    )
    foreach(F IN LISTS JS_RESULT)
    add_custom_command(
        OUTPUT copy_index_command APPEND
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${F} "${CMAKE_BINARY_DIR}"
    )
    endforeach()

endif()