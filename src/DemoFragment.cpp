#include "demo.h"
#include <string>

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
    void resize(WGPU*, uint32_t width, uint32_t height, float dpi) override;

#ifdef MINIMAL_WGPU_IMGUI
    void imgui(WGPU*) override;
#endif

    WGPURenderPipeline pipeline;
    WGPUShaderModule vertexShaderModule = {};

    std::string fragmentCode;
};

std::unique_ptr<Demo> createDemoFragment() {
    return std::make_unique<DemoFragment>();
}

void DemoFragment::init(WGPU *wgpu) {
    static const char *code = R"(
        @vertex
        fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
            // Oversized triangle to fill the screen
            let positions = array<vec2<f32>,3>(
                vec2<f32>(-1.0, -1.0),
                vec2<f32>(3.0, -1.0),
                vec2<f32>(-1.0, 3.0));
            return vec4<f32>(positions[in_vertex_index], 0.0, 1.0);
        }

    )";

    fragmentCode = R"(
        @fragment
        fn fs_main() -> @location(0) vec4<f32> {
            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
        )";

    auto createShaderModule = [wgpu](const char *code) -> WGPUShaderModule {
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
        const WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(wgpu->device, &shaderModuleDescriptor);
        return shaderModule;
    };

    vertexShaderModule = createShaderModule(code);
    WGPUShaderModule fragmentModule = createShaderModule(fragmentCode.c_str());

    const WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {
            .label = WGPU_C_STR("pipeline layout")
    };
    const WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(wgpu->device, &pipelineLayoutDescriptor);
    const WGPUColorTargetState colorTargetStates = {.format = wgpu->surfaceFormat, .writeMask = WGPUColorWriteMask_All};
    const WGPUFragmentState fragment = {
            .module = fragmentModule,
            .entryPoint = WGPU_C_STR("fs_main"),
            .targetCount = 1,
            .targets = &colorTargetStates,
    };

    const WGPURenderPipelineDescriptor pipelineDescriptor = {
            .label = WGPU_C_STR("Render Fragment"),
            .layout = pipelineLayout,
            .vertex = { .module = vertexShaderModule, .entryPoint = WGPU_C_STR("vs_main")},
            .primitive = { .topology = WGPUPrimitiveTopology_TriangleList},
            .multisample = { .count = 1, .mask = 0xFFFFFFFF},
            .fragment = &fragment,

    };

    pipeline = wgpuDeviceCreateRenderPipeline(wgpu->device, &pipelineDescriptor);
    wgpuShaderModuleRelease(fragmentModule);
    wgpuPipelineLayoutRelease(pipelineLayout);
}

void DemoFragment::cleanup(WGPU *) {
    wgpuRenderPipelineRelease(pipeline);
    wgpuShaderModuleRelease(vertexShaderModule);
}

void DemoFragment::resize(WGPU *wgpu, uint32_t width, uint32_t height, float) {
}

void DemoFragment::frame(WGPU *wgpu, WGPUTextureView frame) {
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


#ifdef MINIMAL_WGPU_IMGUI
void DemoFragment::imgui(WGPU *wgpu) {
    char name[64];
    snprintf(name, sizeof(name), "Demo %d", demoImguiIndex);
    if (ImGui::Begin(name)) {
        imguiShowFrame(wgpu, {ImGui::GetContentRegionAvail().x , 256});
        ImGui::End();
    }
}
#endif

ADD_DEMO_WINDOW(fragment, createDemoFragment)
