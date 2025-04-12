#pragma once

#include <iostream>
#include <webgpu/webgpu.h>
#include <memory>
#include <ostream>
#include <vector>

#ifdef MINIMAL_WGPU_IMGUI
#include <cstdio>
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

    // 1) If we are rendering with imgui the demo we are responsible for
    //    doing the Begin,End,and showFrame code as shown bellow as an example
#ifdef MINIMAL_WGPU_IMGUI
    uint32_t demoImguiIndex = 0; // assigned when we add the window (addDemoWindow)
    virtual void imgui(WGPU *wgpu) {
        char name[64];
        snprintf(name, sizeof(name), "Demo %d", demoImguiIndex);
        if (ImGui::Begin(name)) {
            imguiShowFrame(wgpu, ImGui::GetContentRegionAvail());
            ImGui::End();
        }
    }
    // Call this when you are ready to render the Frame
    //   - it will call resize if needed
    //   - it will call frame() with a textureView
    void imguiShowFrame(WGPU *wgpu, ImVec2 size);
#endif

    // 2) the size of the window will be checked before calling frame
    virtual void resize(WGPU*, uint32_t width, uint32_t height, float dpi) {}

    // 3) Finally this function will be called to fill the textureView.
    virtual void frame(WGPU*, WGPUTextureView) {}

    virtual void cleanup(WGPU*) {}

    virtual void onError(WGPU*, const char* message) {
        std::cerr << "Error:" << message << std::endl;
    }

    // Note: if we are rendering with Imgui, this function never gets called, you should use
    //       ImGui::IO instead. Otherwise, if we are rendering a single themo this function will
    //       receive the events directly from sokol.
    virtual void event(WGPU *wpgu, const sapp_event* ev) {}
};


struct DemoBuilder {
    const char *name;
    std::unique_ptr<Demo> (*func) ();
};
extern std::vector<DemoBuilder> demo_builders;

#define ADD_DEMO_WINDOW(name, func) \
    static struct initializer_##name  {\
        initializer_##name() { demo_builders.push_back({#name, func}); } \
    } initializerInstance_##name;
