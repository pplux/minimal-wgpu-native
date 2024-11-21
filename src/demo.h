#pragma once

#include <webgpu/webgpu.h>

struct WGPUPlatform;

struct WGPU {
    WGPUTextureFormat surfaceFormat;
    WGPUDevice device;     // Specific GPU object with some capabilities
    uint32_t requestedDeviceIndex; // index of the requested device, represents the quality/performance tier
    WGPUQueue queue;
    WGPUPlatform *platform;
};
void present(WGPU*);

struct sapp_event; // defined in sokol_app.h

namespace demo {
    void init(WGPU*);
    void frame(WGPU*, WGPUTextureView);
    void cleanup(WGPU*);
    void resize(WGPU*, uint32_t width, uint32_t height);
    void event(WGPU *wpgu, const sapp_event* ev);
}

