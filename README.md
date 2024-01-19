# minimal-wgpu-native
Minimal application for wgpu on native platforms (+web)

<img width="279" alt="image" src="https://github.com/pplux/minimal-wgpu-native/assets/46521/fe603e35-4623-4131-b562-6e60bd706c2f">

This is an example on how to set up a very minimal native app for Windows and MacOS
using [wgpu-native](https://github.com/gfx-rs/wgpu-native) as backend. For the native
window handling we use [sokol_app](https://github.com/floooh/sokol)[1].

### Compilation

Just use Cmake to configure your project, the appropriate binary release of
wgpu-native will be automatically downloaded as part of the project setup.
Have a look at the [CMakeLists.txt](CMakeLists.txt) file for details.


### Emscripten

If you want to use the same code, but targetting the web
with [emscripten](https://emscripten.org/), you can compile on linux, mac,
or [WSL](https://learn.microsoft.com/en-us/windows/wsl/about) the project:

```bash
cd minimal-wgpu-native
mkdir build
cd build
source $HOME/<path-to-your-emsdk>/emsdk/emsdk_env.sh
cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
make -j
```
 
### Notes

[1] We Currently use a [modified version of sokol_app](https://github.com/floooh/sokol/pull/938) 
to be able to create the WebGPU Context on windows without having to override the GL/DX Context
created by sokol otherwise.
