#include "demo.h"

#define IMGUI_IMPL_WEBGPU_BACKEND_WGPU
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imconfig.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_tables.cpp"
#include "imgui_demo.cpp"
#include "backends/imgui_impl_wgpu.cpp"

namespace demo {

    static struct State {
        WGPURenderPipeline pipeline;
    } state;

    void init(WGPU *wgpu) {
        ImGui::CreateContext();
        ImGui_ImplWGPU_InitInfo init_info;
        init_info.Device = wgpu->device;
        init_info.NumFramesInFlight = 3;
        init_info.RenderTargetFormat = wgpu->surfaceFormat;
        init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
        ImGui_ImplWGPU_Init(&init_info);

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

    void resize(WGPU *wgpu, uint32_t width, uint32_t height) {
        ImGui_ImplWGPU_InvalidateDeviceObjects();
        ImGui_ImplWGPU_CreateDeviceObjects();
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)width, (float)height);
    }


    void frame(WGPU *wgpu, WGPUTextureView frame) {

        ImGui_ImplWGPU_NewFrame();
        ImGui::NewFrame();
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Render();

        WGPURenderPassColorAttachment color_attachments = {};
        color_attachments.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        color_attachments.loadOp = WGPULoadOp_Clear;
        color_attachments.storeOp = WGPUStoreOp_Store;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        color_attachments.clearValue = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        color_attachments.view = frame;

        WGPURenderPassDescriptor render_pass_desc = {};
        render_pass_desc.colorAttachmentCount = 1;
        render_pass_desc.colorAttachments = &color_attachments;
        render_pass_desc.depthStencilAttachment = nullptr;

        WGPUCommandEncoderDescriptor enc_desc = {};
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &enc_desc);

        WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
        wgpuRenderPassEncoderEnd(pass);
        wgpuRenderPassEncoderRelease(pass);

        WGPUCommandBufferDescriptor cmd_buffer_desc = {};
        WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);

        wgpuQueueSubmit(wgpu->queue, 1, &cmd_buffer);

        present(wgpu);

        wgpuCommandBufferRelease(cmd_buffer);
        wgpuCommandEncoderRelease(encoder);

#if 0
        WGPUCommandEncoderDescriptor commandEncoderDescriptor = {.label = "Frame"};
        WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(wgpu->device, &commandEncoderDescriptor);

        WGPURenderPassColorAttachment renderPassColorAttachment{
                .view = frame,
                .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
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
        wgpuRenderPassEncoderRelease(renderPassEncoder);

        WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(commandEncoder, nullptr);
        wgpuQueueSubmit(wgpu->queue, 1, &commandBuffer);

        // ready to display
        present(wgpu);

        // post-display free
        wgpuCommandBufferRelease(commandBuffer);
        wgpuCommandEncoderRelease(commandEncoder);
#endif
    }

} // demo namespace