#include "demo.h"

namespace demo {

    static struct State {
        WGPURenderPipeline pipeline;
    } state;

    void init(WGPU *wgpu) {
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

        const WGPUShaderModuleWGSLDescriptor shaderCode = {.chain = {.sType = WGPUSType_ShaderModuleWGSLDescriptor}, .code = code };
        const WGPUShaderModuleDescriptor shaderModuleDescriptor = {
                .nextInChain = &shaderCode.chain,
                .label = "Basic Triangle",
        };
        const WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(wgpu->device, &shaderModuleDescriptor);
        const WGPUPipelineLayoutDescriptor pipelineLayoutDescriptor = {
                .label = "pipeline layout"
        };
        const WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(wgpu->device, &pipelineLayoutDescriptor);
        const WGPUColorTargetState colorTargetStates = {.format = wgpu->surfaceFormat, .writeMask = WGPUColorWriteMask_All};
        const WGPUFragmentState fragment = {
                .module = shaderModule,
                .entryPoint = "fs_main",
                .targetCount = 1,
                .targets = &colorTargetStates,
        };
        const WGPURenderPipelineDescriptor pipelineDescriptor = {
                .label = "Render Triangle",
                .layout = pipelineLayout,
                .vertex = { .module = shaderModule, .entryPoint = "vs_main"},
                .primitive = { .topology = WGPUPrimitiveTopology_TriangleList},
                .multisample = { .count = 1, .mask = 0xFFFFFFFF},
                .fragment = &fragment,

        };

        state.pipeline = wgpuDeviceCreateRenderPipeline(wgpu->device, &pipelineDescriptor);

        wgpuShaderModuleRelease(shaderModule);
        wgpuPipelineLayoutRelease(pipelineLayout);
    }

    void cleanup(WGPU *) {
        wgpuRenderPipelineRelease(state.pipeline);
    }

    void frame(WGPU *wgpu, WGPUTextureView frame) {
        WGPUCommandEncoderDescriptor commandEncoderDescriptor = {.label = "Frame"};
        WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &commandEncoderDescriptor);

        WGPURenderPassColorAttachment renderPassColorAttachment{
                .view = frame,
                .loadOp = WGPULoadOp_Clear,
                .storeOp = WGPUStoreOp_Store,
                .clearValue = {.r = 0.2, .g = 0.2, .b = 0.2, .a = 1.0},

        };
        WGPURenderPassDescriptor renderPass = {
                .label = "Main Pass",
                .colorAttachmentCount = 1,
                .colorAttachments = &renderPassColorAttachment,
        };

        WGPURenderPassEncoder renderPassEncoder = wgpuCommandEncoderBeginRenderPass(commandEncoder, &renderPass);
        wgpuRenderPassEncoderSetPipeline(renderPassEncoder, state.pipeline);
        wgpuRenderPassEncoderDraw(renderPassEncoder, 3, 1, 0, 0);
        wgpuRenderPassEncoderEnd(renderPassEncoder);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, nullptr);
        wgpuQueueSubmit(wgpu->queue, 1, &commandBuffer);

        wgpuCommandBufferRelease(commandBuffer);
        wgpuRenderPassEncoderRelease(renderPassEncoder);
        wgpuCommandEncoderRelease(commandEncoder);
    }

} // demo namespace