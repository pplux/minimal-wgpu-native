cmake_minimum_required(VERSION 3.22)
include(FetchContent)
project(minimal-wgpu-native)
set(CMAKE_CXX_STANDARD 20)

if(APPLE OR WIN32)
    # WGPU Version used & HASH ##############################
    set(WGPU_VERSION "0.19.1.1")
    set(WGPU_BUILD_TYPE "debug") # or release
    #########################################################

    #########################################################
    if(APPLE)
        set(WGPU_PLATFORM "macos-aarch64")
    endif()

    if(WIN32)
        set(WGPU_PLATFORM "windows-x86_64")
    endif()

    # Downloading WGPU Library from wgpu-native repository
    set(WGPU_URL https://github.com/gfx-rs/wgpu-native/releases/download/v${WGPU_VERSION}/wgpu-${WGPU_PLATFORM}-${WGPU_BUILD_TYPE}.zip)

    FetchContent_Declare(
            wgpulib
            URL ${WGPU_URL}
    )

    if (NOT wgpulib_POPULATED)
        FetchContent_Populate(wgpulib)
    endif()
    include_directories(${wgpulib_SOURCE_DIR})
    #########################################################
endif()

add_executable(
        minimal-wgpu
        src/main.cpp
        src/demo.cpp
        src/demo.h)

if(APPLE)
    target_compile_options(minimal-wgpu PRIVATE -x objective-c++)
    target_link_libraries(minimal-wgpu
            "${wgpulib_SOURCE_DIR}/libwgpu_native.a"
            "-framework QuartzCore"
            "-framework Cocoa"
            "-framework MetalKit"
            "-framework Metal")
endif()

if(WIN32)
    target_link_libraries(minimal-wgpu "${wgpulib_SOURCE_DIR}/wgpu_native.lib" Opengl32 ws2_32 D3DCompiler BCrypt Ntdll Userenv)
    # Copy wgpu_native.dll where we produce the binary for the executable:
    add_custom_command(TARGET minimal-wgpu POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${wgpulib_SOURCE_DIR}/wgpu_native.dll
            $<TARGET_FILE_DIR:minimal-wgpu>
    )
endif()

if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(minimal-wgpu PROPERTIES COMPILE_FLAGS "-g -O0")
    set_target_properties(minimal-wgpu PROPERTIES LINK_FLAGS "-g -O0 -s USE_WEBGPU=1 -s WASM=1")
endif()