#pragma once

#include <webgpu/webgpu.h>
#include <memory>

struct WGPUPlatform;

struct WGPU {
    WGPUTextureFormat surfaceFormat;
    WGPUDevice device;     // Specific GPU object with some capabilities
    uint32_t requestedDeviceIndex; // index of the requested device, represents the quality/performance tier
    WGPUQueue queue;
    WGPUPlatform *platform;
};

struct sapp_event; // defined in sokol_app.h

struct Demo {
    virtual ~Demo() {}
    virtual void init(WGPU*) {}
    virtual void frame(WGPU*, WGPUTextureView) {}
    virtual void cleanup(WGPU*) {}
    virtual void resize(WGPU*, uint32_t width, uint32_t height, float dpi) {}
    virtual void event(WGPU *wpgu, const sapp_event* ev) {}
};

std::unique_ptr<Demo> createDemoTriangle();

#ifdef MINIMAL_WGPU_IMGUI
std::unique_ptr<Demo> createDemoImgui();
void addDemoWindow(std::unique_ptr<Demo> &&demo);
#endif
