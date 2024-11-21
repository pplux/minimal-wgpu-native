#include "demo.h"
#include "sokol_app.h"

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


void event(WGPU *wpgu, const sapp_event* ev) {
    const float dpi_scale = sapp_dpi_scale();
    ImGuiIO* io = &ImGui::GetIO();
    switch (ev->type) {
        case SAPP_EVENTTYPE_FOCUSED:
            //simgui_add_focus_event(true);
            break;
        case SAPP_EVENTTYPE_UNFOCUSED:
            //simgui_add_focus_event(false);
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            //simgui_add_mouse_pos_event(ev->mouse_x / dpi_scale, ev->mouse_y / dpi_scale);
            //simgui_add_mouse_button_event((int)ev->mouse_button, true);
            //_simgui_update_modifiers(io, ev->modifiers);
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            //simgui_add_mouse_pos_event(ev->mouse_x / dpi_scale, ev->mouse_y / dpi_scale);
            //simgui_add_mouse_button_event((int)ev->mouse_button, false);
            //_simgui_update_modifiers(io, ev->modifiers);
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            //simgui_add_mouse_pos_event(ev->mouse_x / dpi_scale, ev->mouse_y / dpi_scale);
            break;
        case SAPP_EVENTTYPE_MOUSE_ENTER:
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
            // FIXME: since the sokol_app.h emscripten backend doesn't support
            // mouse capture, mouse buttons must be released when the mouse leaves the
            // browser window, so that they don't "stick" when released outside the window.
            // A cleaner solution would be a new sokol_app.h function to query
            // "platform behaviour flags".
            #if defined(__EMSCRIPTEN__)
            for (int i = 0; i < SAPP_MAX_MOUSEBUTTONS; i++) {
            //    simgui_add_mouse_button_event(i, false);
            }
            #endif
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            //simgui_add_mouse_wheel_event(ev->scroll_x, ev->scroll_y);
            break;
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
            //simgui_add_touch_pos_event(ev->touches[0].pos_x / dpi_scale, ev->touches[0].pos_y / dpi_scale);
            //simgui_add_touch_button_event(0, true);
            break;
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
            //simgui_add_touch_pos_event(ev->touches[0].pos_x / dpi_scale, ev->touches[0].pos_y / dpi_scale);
            break;
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
            //simgui_add_touch_pos_event(ev->touches[0].pos_x / dpi_scale, ev->touches[0].pos_y / dpi_scale);
            //simgui_add_touch_button_event(0, false);
            break;
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
            //simgui_add_touch_button_event(0, false);
            break;
        case SAPP_EVENTTYPE_KEY_DOWN:
            /*
            _simgui_update_modifiers(io, ev->modifiers);
            // intercept Ctrl-V, this is handled via EVENTTYPE_CLIPBOARD_PASTED
            if (!_simgui.desc.disable_paste_override) {
                if (_simgui_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_V)) {
                    break;
                }
            }
            // on web platform, don't forward Ctrl-X, Ctrl-V to the browser
            if (_simgui_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_X)) {
                sapp_consume_event();
            }
            if (_simgui_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_C)) {
                sapp_consume_event();
            }
            // it's ok to add ImGuiKey_None key events
            _simgui_add_sapp_key_event(io, ev->key_code, true);
            */
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            /*
            _simgui_update_modifiers(io, ev->modifiers);
            // intercept Ctrl-V, this is handled via EVENTTYPE_CLIPBOARD_PASTED
            if (_simgui_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_V)) {
                break;
            }
            // on web platform, don't forward Ctrl-X, Ctrl-V to the browser
            if (_simgui_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_X)) {
                sapp_consume_event();
            }
            if (_simgui_is_ctrl(ev->modifiers) && (ev->key_code == SAPP_KEYCODE_C)) {
                sapp_consume_event();
            }
            // it's ok to add ImGuiKey_None key events
            _simgui_add_sapp_key_event(io, ev->key_code, false);
            */
            break;
        case SAPP_EVENTTYPE_CHAR:
            /* on some platforms, special keys may be reported as
               characters, which may confuse some ImGui widgets,
               drop those, also don't forward characters if some
               modifiers have been pressed
            */
            /*
            _simgui_update_modifiers(io, ev->modifiers);
            if ((ev->char_code >= 32) &&
                (ev->char_code != 127) &&
                (0 == (ev->modifiers & (SAPP_MODIFIER_ALT|SAPP_MODIFIER_CTRL|SAPP_MODIFIER_SUPER))))
            {
                simgui_add_input_character(ev->char_code);
            }
            */
            break;
        case SAPP_EVENTTYPE_CLIPBOARD_PASTED:
            /*
            // simulate a Ctrl-V key down/up
            if (!_simgui.desc.disable_paste_override) {
                _simgui_add_imgui_key_event(io, _simgui_copypaste_modifier(), true);
                _simgui_add_imgui_key_event(io, ImGuiKey_V, true);
                _simgui_add_imgui_key_event(io, ImGuiKey_V, false);
                _simgui_add_imgui_key_event(io, _simgui_copypaste_modifier(), false);
            }
            */
            break;
        default:
            break;
    }
}


    void frame(WGPU *wgpu, WGPUTextureView frame) {

        ImGuiIO* io = &ImGui::GetIO();

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