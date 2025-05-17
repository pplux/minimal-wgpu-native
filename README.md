# minimal-wgpu-native
Minimal application for wgpu on native platforms (+web)

[video.webm](https://github.com/user-attachments/assets/7a481e7c-3f51-40d0-a8c9-dacaea8bb8b9)

This is an example on how to set up a very minimal native app for Windows and MacOS
using [wgpu-native](https://github.com/gfx-rs/wgpu-native) as backend. For the native
window handling, we use [sokol_app](https://github.com/floooh/sokol).

## Demos
* [basic Triangle](https://pplux.github.io/minimalWGPUNative/minimal-wgpu-triangle.html)
* [fragment SDF](https://pplux.github.io/minimalWGPUNative/minimal-wgpu-fragment.html)
* [Imgui + All demos](https://pplux.github.io/minimalWGPUNative/minimal-wgpu-imgui.html) 

## Compilation

Just use Cmake to configure your project, and the appropriate binary release of
wgpu-native will be automatically downloaded as part of the project setup.
Have a look at the [CMakeLists.txt](CMakeLists.txt) file for details.


## Emscripten

### Locally on linux, mac, WSL:

If you want to use the same code, but targeting the web with [emscripten](https://emscripten.org/), you can compile on linux, mac,
or [WSL](https://learn.microsoft.com/en-us/windows/wsl/about) the project:

```bash
cd minimal-wgpu-native
mkdir build
cd build
source $HOME/<path-to-your-emsdk>/emsdk/emsdk_env.sh
cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
make -j
```

### With Docker

You can use Docker to compile the wasm files with a script ([buildWASM.py](wasm/buildWASM.py))
```bash
cd minimal-wgpu-native/wasm
python3 ./buildWASM.py
```
