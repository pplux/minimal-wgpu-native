#include <thread>
#include <memory>

#include "demo.h"
#include <wgpu/wgpu.h>

#define SOKOL_NO_ENTRY 1
#define SOKOL_APP_IMPL 1
#define SOKOL_NOAPI 1

#if defined(__APPLE__)
#include <Foundation/Foundation.h>
#include <QuartzCore/CAMetalLayer.h>
#define SOKOL_METAL 1
#endif

#if defined(__EMSCRIPTEN__)
#define SOKOL_WGPU 1
struct WGPUPlatform {
    // Not needed with SOKOL, as it already handles WebGPU internally
};
#else
struct WGPUPlatform {
    // In order of creation:
    WGPUInstance instance; // WGPU library entry point
    struct {
        WGPUSurface object;   // Represents the connection with the system's window
        WGPUSurfaceConfiguration config;
    } surface;
    WGPUAdapter adapter;   // Identifier of a particular WGPU implementation on the system
};
#endif

#include "sokol_app.h"


void requestDevice(WGPU *wgpu);
void onDevice(WGPU *wgpu);

std::unique_ptr<Demo> demo;

#ifndef __EMSCRIPTEN__
void init(WGPU *wgpu) {

    // Specifics to wgpu-native
    wgpuSetLogCallback([](WGPULogLevel level, const char *msg, void *){
        fprintf(stderr, "WGPU [%d] %s\n", level, msg);
        }, nullptr);
    wgpuSetLogLevel(WGPULogLevel::WGPULogLevel_Error);

    { // Surface
        WGPUSurfaceDescriptor surfaceDescriptor = {};

#ifdef _MSC_VER
        HINSTANCE hinstance =  GetModuleHandle(NULL);
        WGPUSurfaceDescriptorFromWindowsHWND nativeSurfaceDescriptor = {};
        nativeSurfaceDescriptor.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
        nativeSurfaceDescriptor.hinstance = hinstance;
        nativeSurfaceDescriptor.hwnd = (void*) sapp_win32_get_hwnd();
#endif

#ifdef __APPLE__
        id metal_layer = NULL;
        NSWindow *ns_window = reinterpret_cast<NSWindow*>(sapp_macos_get_window());
        [ns_window.contentView setWantsLayer:YES];
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer:metal_layer];
        WGPUSurfaceDescriptorFromMetalLayer nativeSurfaceDescriptor = {};
        nativeSurfaceDescriptor.layer = metal_layer;
        nativeSurfaceDescriptor.chain.sType = WGPUSType_SurfaceDescriptorFromMetalLayer;
#endif

        surfaceDescriptor.nextInChain = &nativeSurfaceDescriptor.chain;
        wgpu->platform->surface.object = wgpuInstanceCreateSurface(wgpu->platform->instance, &surfaceDescriptor);
    }

    { // Adapter (async)
        WGPURequestAdapterOptions adapterOptions = {};
        adapterOptions.compatibleSurface = wgpu->platform->surface.object;
        wgpuInstanceRequestAdapter(wgpu->platform->instance, &adapterOptions, [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const * message, void * userdata){
            WGPU *wgpu = static_cast<WGPU*>(userdata);
            wgpu->platform->adapter = adapter;
            requestDevice(wgpu);
        }, wgpu);
    }
}

void onDevice(WGPU *wgpu) {
    wgpu->queue = wgpuDeviceGetQueue(wgpu->device);

    WGPUSurfaceCapabilities surfaceCapabilities = {};
    wgpuSurfaceGetCapabilities(wgpu->platform->surface.object, wgpu->platform->adapter, &surfaceCapabilities);

    auto &config = wgpu->platform->surface.config;
    config.device = wgpu->device;
    config.usage = WGPUTextureUsage_RenderAttachment;
    config.format = surfaceCapabilities.formats[0];
    config.viewFormatCount = 1;
    config.viewFormats = &config.format;
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = surfaceCapabilities.alphaModes[0];
    config.width = sapp_width();
    config.height = sapp_height();

    wgpuSurfaceConfigure(wgpu->platform->surface.object, &wgpu->platform->surface.config);
    wgpuSurfaceCapabilitiesFreeMembers(surfaceCapabilities);

    wgpu->surfaceFormat = config.viewFormats[0];

    demo->init(wgpu);
    demo->resize(wgpu, config.width, config.height);
}

void requestDevice(WGPU *wgpu) {
    WGPUDeviceDescriptor deviceDescriptor = {};
    switch(wgpu->requestedDeviceIndex) {
        case 0: // Performance tier
        deviceDescriptor.label = "Performance";
        deviceDescriptor.defaultQueue.label = "default";
        break;
        // case 1 -> mid tier ...
        // case 2 -> low tier ...
        default:
            fprintf(stderr, "Could not find a capable device after %d tries\n", wgpu->requestedDeviceIndex+1);
            exit(-1);
    }
    deviceDescriptor.uncapturedErrorCallbackInfo.callback =
            [](WGPUErrorType type, const char *msg, void *userdata) {
                WGPU *wgpu = static_cast<WGPU *>(userdata);
                fprintf(stderr, "WGPU Device (%p) Error (type = 0x%x) %s\n", wgpu->device, type, msg ? msg : "");
            };
    // Request Device (Async)
    wgpuAdapterRequestDevice(wgpu->platform->adapter, &deviceDescriptor, [](WGPURequestDeviceStatus status, WGPUDevice device, const char *message, void *userdata){
        WGPU *wgpu = static_cast<WGPU*>(userdata);
        if (status == WGPURequestDeviceStatus_Success) {
            wgpu->device = device;
            onDevice(wgpu);
            return;
        }
        wgpu->requestedDeviceIndex++;
        requestDevice(wgpu);
    }, wgpu);

}

void cleanup(WGPU *wgpu) {
    demo->cleanup(wgpu);
    wgpuDeviceRelease(wgpu->device);
    wgpuAdapterRelease(wgpu->platform->adapter);
    wgpuSurfaceRelease(wgpu->platform->surface.object);
}

void frame(WGPU *wgpu) {
    if (!wgpu->queue) return; /* Wait for the queue to be created */

    auto reconfigureSurface = [wgpu]() -> bool {
        const uint32_t width = sapp_width();
        const uint32_t height = sapp_height();
        if (width == 0 || height == 0) return false;

        if ((wgpu->platform->surface.config.width != width) || (wgpu->platform->surface.config.height != height)) {
            wgpu->platform->surface.config.width = sapp_width();
            wgpu->platform->surface.config.height = sapp_height();
            wgpuSurfaceConfigure(wgpu->platform->surface.object, &wgpu->platform->surface.config);
            demo->resize(wgpu, width, height);
            return true;
        }
        return false;
    };

    if (reconfigureSurface()) return;

    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(wgpu->platform->surface.object, &surfaceTexture);

    switch (surfaceTexture.status) {
        case WGPUSurfaceGetCurrentTextureStatus_Success:
            break;
        case WGPUSurfaceGetCurrentTextureStatus_Timeout:
        case WGPUSurfaceGetCurrentTextureStatus_Outdated:
        case WGPUSurfaceGetCurrentTextureStatus_Lost: {
            // Skip this frame, and re-configure surface.
            if (surfaceTexture.texture != NULL) {
                wgpuTextureRelease(surfaceTexture.texture);
            }
            reconfigureSurface();
            return;
        }
        case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
        case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
        case WGPUSurfaceGetCurrentTextureStatus_Force32:
            fprintf(stderr, "GetCurrentTexture Failed (0x%x)\n", surfaceTexture.status);
            exit(-1);
    }

    WGPUTextureView frame =
            wgpuTextureCreateView(surfaceTexture.texture, NULL);

    demo->frame(wgpu, frame);
    wgpuTextureViewRelease(frame);
    wgpuTextureRelease(surfaceTexture.texture);
}

void present(WGPU *wgpu) {
    wgpuSurfacePresent(wgpu->platform->surface.object);
}

#else

// SOKOL Already initializes WGPU:
void init(WGPU *wgpu) {
    wgpu->device = (WGPUDevice) sapp_wgpu_get_device();
    wgpu->queue = wgpuDeviceGetQueue(wgpu->device);
    wgpu->surfaceFormat =  _sapp.wgpu.render_format;
    demo->init(wgpu);
}

void cleanup(WGPU *wgpu) {
    demo->cleanup(wgpu);
}

void frame(WGPU *wgpu) {
   demo->frame(wgpu, (WGPUTextureView)(const_cast<void*>(sapp_wgpu_get_render_view())));
}

#endif


// This needs to be static for EMSCRIPTEN (main doesn't work like a native app)
namespace {
    WGPUPlatform platform = {};
    WGPU wgpu = {.platform = &platform};
}

int main(int argc, char* argv[]) {

    demo = std::make_unique<ImguiDemo>();

    sapp_desc sokolConfig = {
            .user_data = &wgpu,
            .init_userdata_cb = [](void *ptr){ init(static_cast<WGPU*>(ptr)); },
            .frame_userdata_cb = [](void *ptr){ frame(static_cast<WGPU*>(ptr)); },
            .cleanup_userdata_cb = [](void *ptr){ cleanup(static_cast<WGPU*>(ptr)); },
            .event_userdata_cb = [](const sapp_event *e, void *ptr){ demo->event(static_cast<WGPU*>(ptr), e); },
            .width = 1024,
            .height = 768,
            .window_title = "Minimal WGPU Native"
    };

#ifndef __EMSCRIPTEN__
    WGPUInstanceDescriptor instanceDescriptor = {};
    platform.instance = wgpuCreateInstance(&instanceDescriptor);
    sapp_run(&sokolConfig);
    wgpuInstanceRelease(platform.instance);
#else
    sapp_run(&sokolConfig);
#endif
    return 0;
}
