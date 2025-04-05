#include "demo.h"

#ifndef __EMSCRIPTEN__
#define WGPU_C_STR(value) { value, WGPU_STRLEN }
#else
#define WGPU_C_STR(value) value
#endif

struct DemoTriangle : public Demo {
    virtual void init(WGPU*);
    virtual void frame(WGPU*, WGPUTextureView);
    virtual void cleanup(WGPU*);
    virtual void resize(WGPU*, uint32_t width, uint32_t height);

    WGPURenderPipeline pipeline;

    // Used
    WGPUCommandEncoder commandEncoder;
    WGPUCommandBuffer commandBuffer;

    float bgColor[3] = {};
};

std::unique_ptr<Demo> createDemoTriangle() {
    return std::make_unique<DemoTriangle>();
}

void DemoTriangle::init(WGPU *wgpu) {
    static const char *code = R"(
        @vertex
        fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
            let x = f32(i32(in_vertex_index) - 1);
            let y = f32(i32(in_vertex_index & 1u) * 2 - 1);
            return vec4<f32>(x, y, 0.0, 1.0);
        }

        @fragment
        fn fs_main() -> @location(0) vec4<f32> {
            return vec4<f32>(1.0, 0.0, 0.0, 1.0);
        }
    )";

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
            .label = WGPU_C_STR("Basic Triangle"),
    };
    const WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(wgpu->device, &shaderModuleDescriptor);
    const WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {
            .label = WGPU_C_STR("pipeline layout")
    };
    const WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(wgpu->device, &pipelineLayoutDescriptor);
    const WGPUColorTargetState colorTargetStates = {.format = wgpu->surfaceFormat, .writeMask = WGPUColorWriteMask_All};
    const WGPUFragmentState fragment = {
            .module = shaderModule,
            .entryPoint = WGPU_C_STR("fs_main"),
            .targetCount = 1,
            .targets = &colorTargetStates,
    };
    const WGPURenderPipelineDescriptor pipelineDescriptor = {
            .label = WGPU_C_STR("Render Triangle"),
            .layout = pipelineLayout,
            .vertex = { .module = shaderModule, .entryPoint = WGPU_C_STR("vs_main")},
            .primitive = { .topology = WGPUPrimitiveTopology_TriangleList},
            .multisample = { .count = 1, .mask = 0xFFFFFFFF},
            .fragment = &fragment,

    };

    pipeline = wgpuDeviceCreateRenderPipeline(wgpu->device, &pipelineDescriptor);
    wgpuShaderModuleRelease(shaderModule);
    wgpuPipelineLayoutRelease(pipelineLayout);

    static float b = 0.0f;
    b += 0.33;
    bgColor[0] = b;
    bgColor[1] = b;
    bgColor[2] = b;
}

void DemoTriangle::cleanup(WGPU *) {
    wgpuRenderPipelineRelease(pipeline);
}

void DemoTriangle::resize(WGPU *wgpu, uint32_t width, uint32_t height) {
}

void DemoTriangle::frame(WGPU *wgpu, WGPUTextureView frame) {
    WGPUCommandEncoderDescriptor commandEncoderDescriptor = {.label = WGPU_C_STR("Frame")};
    commandEncoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &commandEncoderDescriptor);

    WGPURenderPassColorAttachment renderPassColorAttachment{
            .view = frame,
            .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
            .clearValue = {.r = bgColor[0], .g = bgColor[1], .b = bgColor[2], .a = 1.0},

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

    commandBuffer = wgpuCommandEncoderFinish(commandEncoder, nullptr);
    wgpuQueueSubmit(wgpu->queue, 1, &commandBuffer);

    // post-display free
    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(commandEncoder);
}

ADD_DEMO_WINDOW(triangle, createDemoTriangle)
