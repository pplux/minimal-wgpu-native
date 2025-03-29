cmake_minimum_required(VERSION 3.22)
if (NOT DEFINED EMSCRIPTEN)
    cmake_policy(SET CMP0135 NEW) # FetchContent timestamp set to uncompress time
    include(FetchContent)
endif()
project(minimal-wgpu-native)
set(CMAKE_CXX_STANDARD 20)
include_directories("imgui")

if(APPLE OR WIN32)
    # WGPU Version used & HASH ##############################
    set(WGPU_VERSION "24.0.3.1")
    set(WGPU_BUILD_TYPE "debug") # or release
    #########################################################

    #########################################################
    if(APPLE)
        set(WGPU_PLATFORM "macos-aarch64")
    endif()

    if(WIN32)
        set(WGPU_PLATFORM "windows-x86_64-msvc")
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
    include_directories(${wgpulib_SOURCE_DIR}/include)
    include_directories(${wgpulib_SOURCE_DIR}/include/webgpu)
    include_directories(${wgpulib_SOURCE_DIR}/include/wgpu)
    #########################################################
endif()

include_directories(imgui)

add_executable(
        minimal-wgpu-imgui
        src/main.cpp
        src/DemoImgui.cpp
        src/DemoTriangle.cpp
        src/demo.h)

add_executable(
        minimal-wgpu
        src/main.cpp
        src/DemoTriangle.cpp
        src/demo.h)

target_compile_definitions(minimal-wgpu-imgui PRIVATE MINIMAL_WGPU_IMGUI)

macro(setup_app app)
    if(APPLE)
        target_compile_options(${app} PRIVATE -x objective-c++)
        target_link_libraries(${app}
                "${wgpulib_SOURCE_DIR}/lib/libwgpu_native.a"
                "-framework QuartzCore"
                "-framework Cocoa"
                "-framework MetalKit"
                "-framework Metal")
    endif()

    if(WIN32)
        target_link_libraries(${app} "${wgpulib_SOURCE_DIR}/lib/wgpu_native.lib" Opengl32 ws2_32 D3DCompiler BCrypt Ntdll Userenv)
        # Copy wgpu_native.dll where we produce the binary for the executable:
        add_custom_command(TARGET ${app} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${wgpulib_SOURCE_DIR}/lib/wgpu_native.dll
                $<TARGET_FILE_DIR:${app}>
        )
    endif()

    if (EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
        set_target_properties(${app} PROPERTIES COMPILE_FLAGS "-g -O0")
	set_target_properties(${app} PROPERTIES LINK_FLAGS "-g -O0 -s USE_WEBGPU=1 -s WASM=1 --shell-file=${CMAKE_SOURCE_DIR}/wasm/shell.html")
    endif()
endmacro()

setup_app(minimal-wgpu)
setup_app(minimal-wgpu-imgui)
