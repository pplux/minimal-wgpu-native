#pragma once

#if defined(EMSCRIPTEN)
#include <webgpu/webgpu.h>
#else
#include <webgpu.h>
#endif

struct WGPUPlatform;

struct WGPU {
    WGPUTextureFormat surfaceFormat;
    WGPUDevice device;     // Specific GPU object with some capabilities
    uint32_t requestedDeviceIndex; // index of the requested device, represents the quality/performance tier
    WGPUQueue queue;
    WGPUPlatform *platform;
};

namespace demo {
    void init(WGPU*);
    void frame(WGPU*, WGPUTextureView);
    void cleanup(WGPU*);
}

