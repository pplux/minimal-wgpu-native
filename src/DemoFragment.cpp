#include "demo.h"
#include <string>
#include <__chrono/calendar.h>

#ifdef MINIMAL_WGPU_IMGUI
#include "imgui.h"
#endif

#ifndef __EMSCRIPTEN__
#define WGPU_C_STR(value) { value, WGPU_STRLEN }
#else
#define WGPU_C_STR(value) value
#endif

struct DemoFragment : public Demo {
    void init(WGPU*) override;
    void frame(WGPU*, WGPUTextureView) override;
    void cleanup(WGPU*) override;
    void onError(WGPU *, const char *message) override;

#ifdef MINIMAL_WGPU_IMGUI
    void imgui(WGPU*) override;
#endif

    void rebuild(WGPU*);
    void rebuildDone(WGPU*);

    WGPUShaderModule fragmentModule = {};

    std::string lastError = {};
    WGPURenderPipeline pipeline;
    WGPUShaderModule vertexShaderModule = {};

    char fragmentCode[65536];
};

std::unique_ptr<Demo> createDemoFragment() {
    return std::make_unique<DemoFragment>();
}


WGPUShaderModule createShaderModule(WGPU *wgpu, const char *code) {
#ifdef __EMSCRIPTEN__
    const WGPUShaderModuleWGSLDescriptor shaderCode = {
        .chain = { .sType =  WGPUSType_ShaderModuleWGSLDescriptor },
        .code = WGPU_C_STR(code) };
#else
    const WGPUShaderSourceWGSL shaderCode = {
        .chain = { .sType = WGPUSType_ShaderSourceWGSL },
        .code = WGPU_C_STR(code) };
#endif

    const WGPUShaderModuleDescriptor shaderModuleDescriptor = {
            .nextInChain = &shaderCode.chain,
            .label = WGPU_C_STR("Basic Fragment"),
    };

    // Note: wgpuShaderModuleGetCompilationInfo is not implemented in native, but the device will trigger an
    //       error. And we can use that to invalidate the shader.

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(wgpu->device, &shaderModuleDescriptor);
    return shaderModule;
};

void DemoFragment::init(WGPU *wgpu) {
    vertexShaderModule = createShaderModule(wgpu,
    R"(
        @vertex
        fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
            // Oversized triangle to fill the screen
            let positions = array<vec2<f32>,3>(
                vec2<f32>(-1.0, -1.0),
                vec2<f32>(3.0, -1.0),
                vec2<f32>(-1.0, 3.0));
            return vec4<f32>(positions[in_vertex_index], 0.0, 1.0);
        }

    )");

    const char *initialFragmentCode = "@fragment\nfn fs_main() -> @location(0) vec4<f32>\n{\n  return vec4<f32>(1.0, 0.0, 0.0, 1.0);\n}\n";

    memcpy(fragmentCode, initialFragmentCode, strlen(initialFragmentCode)+1);
    rebuild(wgpu);
}

void DemoFragment::rebuild(WGPU *wgpu) {
    lastError = "";
    fragmentModule = createShaderModule(wgpu, fragmentCode);
    // On WASM we trigger the check of the shader asynchronously
    // on Native, wgpu lib will generate an error, that we will capture on "lastError"
    //
    // On both cases, we wait until next frame to try to use the new fragmentModule, until we know
    // there if there's an error or not.
#ifdef __EMSCRIPTEN__
    WGPUCompilationInfoCallback info = {};

    info = [](WGPUCompilationInfoRequestStatus status, struct WGPUCompilationInfo const *info, void* userdata1) {
        auto demo = reinterpret_cast<DemoFragment*>(userdata1);

        if (status != WGPUCompilationInfoRequestStatus_Success) {
            std::cerr << "Failed to get shader compilation info.\n";
            return;
        }

        for (uint32_t i = 0; i < info->messageCount; ++i) {
            const WGPUCompilationMessage& msg = info->messages[i];

            const bool isError = (msg.type == WGPUCompilationMessageType_Error);
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s: %llu:%llu -> %s\n", (isError?"ERROR":"Warning"), msg.lineNum, msg.linePos, msg.message);
            if (isError) {
                demo->lastError += buffer;
            }
            std::cerr << buffer << std::endl;
        }

    };
    wgpuShaderModuleGetCompilationInfo(fragmentModule, info, this);
#endif
}


void DemoFragment::onError(WGPU *, const char *message) {
    lastError = message;
}

void DemoFragment::cleanup(WGPU *) {
    wgpuRenderPipelineRelease(pipeline);
    wgpuShaderModuleRelease(vertexShaderModule);
}

void DemoFragment::frame(WGPU *wgpu, WGPUTextureView frame) {
    if (pipeline) {
        WGPUCommandEncoderDescriptor commandEncoderDescriptor = {.label = WGPU_C_STR("Frame")};
        WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &commandEncoderDescriptor);

        WGPURenderPassColorAttachment renderPassColorAttachment{
            .view = frame,
            .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
        };
        WGPURenderPassDescriptor renderPass = {
            .label = WGPU_C_STR("Main Pass"),
            .colorAttachmentCount = 1,
            .colorAttachments = &renderPassColorAttachment,
        };

        WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPass);
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, pipeline);
        wgpuRenderPassEncoderDraw(renderPassEncoder, 3, 1, 0, 0);
        wgpuRenderPassEncoderEnd(renderPassEncoder);
        wgpuRenderPassEncoderRelease(renderPassEncoder);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, nullptr);
        wgpuQueueSubmit(wgpu->queue, 1, &commandBuffer);

        // post-display free
        wgpuCommandBufferRelease(commandBuffer);
        wgpuCommandEncoderRelease(commandEncoder);
    }

    if (fragmentModule) {
        if (!lastError.empty()) {
            wgpuShaderModuleRelease(fragmentModule);
            fragmentModule = {};
            return;
        }
        // Compilation ok!

        if (pipeline) {
            wgpuRenderPipelineRelease(pipeline);
        }

        const WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {
            .label = WGPU_C_STR("pipeline layout")
        };
        const WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(
            wgpu->device, &pipelineLayoutDescriptor);
        const WGPUColorTargetState colorTargetStates = {
            .format = wgpu->surfaceFormat, .writeMask = WGPUColorWriteMask_All
        };
        const WGPUFragmentState fragment = {
            .module = fragmentModule,
            .entryPoint = WGPU_C_STR("fs_main"),
            .targetCount = 1,
            .targets = &colorTargetStates,
        };

        const WGPURenderPipelineDescriptor pipelineDescriptor = {
            .label = WGPU_C_STR("Render Fragment"),
            .layout = pipelineLayout,
            .vertex = {.module = vertexShaderModule, .entryPoint = WGPU_C_STR("vs_main")},
            .primitive = {.topology = WGPUPrimitiveTopology_TriangleList},
            .multisample = {.count = 1, .mask = 0xFFFFFFFF},
            .fragment = &fragment,

        };

        pipeline = wgpuDeviceCreateRenderPipeline(wgpu->device, &pipelineDescriptor);
        wgpuShaderModuleRelease(fragmentModule);
        wgpuPipelineLayoutRelease(pipelineLayout);
        fragmentModule = {};
    }
}


#ifdef MINIMAL_WGPU_IMGUI
void DemoFragment::imgui(WGPU *wgpu) {
    char name[64];
    snprintf(name, sizeof(name), "Demo %d", demoImguiIndex);
    if (ImGui::Begin(name)) {
        const float width = ImGui::GetContentRegionAvail().x;
        imguiShowFrame(wgpu, {width, 256});
        ImGui::InputTextMultiline("###FragmentCode", fragmentCode, sizeof(fragmentCode), {width, 256});
        if (ImGui::Button("reload")) { rebuild(wgpu); }
        if (!lastError.empty()) {
            ImGui::TextUnformatted(lastError.c_str(), lastError.c_str()+lastError.length());
        }
        ImGui::End();
    }
}
#endif

ADD_DEMO_WINDOW(fragment, createDemoFragment)
