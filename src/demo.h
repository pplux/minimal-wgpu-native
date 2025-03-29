#pragma once

#include <webgpu/webgpu.h>
#include <memory>

#ifdef MINIMAL_WGPU_IMGUI
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#endif

struct WGPUPlatform;

struct WGPU {
    WGPUTextureFormat surfaceFormat;
    WGPUDevice device = nullptr;     // Specific GPU object with some capabilities
    uint32_t requestedDeviceIndex; // index of the requested device, represents the quality/performance tier
    WGPUQueue queue = nullptr;
    WGPUPlatform *platform = nullptr;
};

struct sapp_event; // defined in sokol_app.h

struct Demo {
    virtual ~Demo() {}
    virtual void init(WGPU*) {}

    // 1) If we are rendering with Imgui, this imgui call will be called first.
    //    Note: We are rendering already inside a Begin/End imgui calls.
#ifdef MINIMAL_WGPU_IMGUI
    virtual void imgui() {}
#endif

    // 2) the size of the window will be checked before calling frame
    //    2.1) If we are rendering with imgui, the size will be the remaining size
    //         returned from GetContentRegionAvail.
    virtual void resize(WGPU*, uint32_t width, uint32_t height, float dpi) {}

    // 3) Finally this function will be called to fill the textureView.
    //    3.1) The size of the texture view will be the last resize call
    //    3.2) If we are rendering with imgui, the result of frame will be drawn in the
    //         space left by the previous imgui() callback.
    virtual void frame(WGPU*, WGPUTextureView) {}

    virtual void cleanup(WGPU*) {}

    // Note: if we are rendering with Imgui, this function never gets called, you should use
    //       ImGui::IO instead. Otherwise, if we are rendering a single themo this function will
    //       receive the events directly from sokol.
    virtual void event(WGPU *wpgu, const sapp_event* ev) {}
};

std::unique_ptr<Demo> createDemoTriangle();

#ifdef MINIMAL_WGPU_IMGUI
std::unique_ptr<Demo> createDemoImgui();
void addDemoWindow(std::unique_ptr<Demo> &&demo);
#endif
