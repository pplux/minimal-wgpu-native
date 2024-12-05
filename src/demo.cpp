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

    auto updateModifiers = [io](uint32_t mods)
    {
        io->AddKeyEvent(ImGuiMod_Ctrl, (mods & SAPP_MODIFIER_CTRL) != 0);
        io->AddKeyEvent(ImGuiMod_Shift, (mods & SAPP_MODIFIER_SHIFT) != 0);
        io->AddKeyEvent(ImGuiMod_Alt, (mods & SAPP_MODIFIER_ALT) != 0);
        io->AddKeyEvent(ImGuiMod_Super, (mods & SAPP_MODIFIER_SUPER) != 0);
    };

    auto mapKeyCode = [](sapp_keycode key) -> ImGuiKey {
    switch (key) {
        case SAPP_KEYCODE_SPACE:        return ImGuiKey_Space;
        case SAPP_KEYCODE_APOSTROPHE:   return ImGuiKey_Apostrophe;
        case SAPP_KEYCODE_COMMA:        return ImGuiKey_Comma;
        case SAPP_KEYCODE_MINUS:        return ImGuiKey_Minus;
        case SAPP_KEYCODE_PERIOD:       return ImGuiKey_Apostrophe;
        case SAPP_KEYCODE_SLASH:        return ImGuiKey_Slash;
        case SAPP_KEYCODE_0:            return ImGuiKey_0;
        case SAPP_KEYCODE_1:            return ImGuiKey_1;
        case SAPP_KEYCODE_2:            return ImGuiKey_2;
        case SAPP_KEYCODE_3:            return ImGuiKey_3;
        case SAPP_KEYCODE_4:            return ImGuiKey_4;
        case SAPP_KEYCODE_5:            return ImGuiKey_5;
        case SAPP_KEYCODE_6:            return ImGuiKey_6;
        case SAPP_KEYCODE_7:            return ImGuiKey_7;
        case SAPP_KEYCODE_8:            return ImGuiKey_8;
        case SAPP_KEYCODE_9:            return ImGuiKey_9;
        case SAPP_KEYCODE_SEMICOLON:    return ImGuiKey_Semicolon;
        case SAPP_KEYCODE_EQUAL:        return ImGuiKey_Equal;
        case SAPP_KEYCODE_A:            return ImGuiKey_A;
        case SAPP_KEYCODE_B:            return ImGuiKey_B;
        case SAPP_KEYCODE_C:            return ImGuiKey_C;
        case SAPP_KEYCODE_D:            return ImGuiKey_D;
        case SAPP_KEYCODE_E:            return ImGuiKey_E;
        case SAPP_KEYCODE_F:            return ImGuiKey_F;
        case SAPP_KEYCODE_G:            return ImGuiKey_G;
        case SAPP_KEYCODE_H:            return ImGuiKey_H;
        case SAPP_KEYCODE_I:            return ImGuiKey_I;
        case SAPP_KEYCODE_J:            return ImGuiKey_J;
        case SAPP_KEYCODE_K:            return ImGuiKey_K;
        case SAPP_KEYCODE_L:            return ImGuiKey_L;
        case SAPP_KEYCODE_M:            return ImGuiKey_M;
        case SAPP_KEYCODE_N:            return ImGuiKey_N;
        case SAPP_KEYCODE_O:            return ImGuiKey_O;
        case SAPP_KEYCODE_P:            return ImGuiKey_P;
        case SAPP_KEYCODE_Q:            return ImGuiKey_Q;
        case SAPP_KEYCODE_R:            return ImGuiKey_R;
        case SAPP_KEYCODE_S:            return ImGuiKey_S;
        case SAPP_KEYCODE_T:            return ImGuiKey_T;
        case SAPP_KEYCODE_U:            return ImGuiKey_U;
        case SAPP_KEYCODE_V:            return ImGuiKey_V;
        case SAPP_KEYCODE_W:            return ImGuiKey_W;
        case SAPP_KEYCODE_X:            return ImGuiKey_X;
        case SAPP_KEYCODE_Y:            return ImGuiKey_Y;
        case SAPP_KEYCODE_Z:            return ImGuiKey_Z;
        case SAPP_KEYCODE_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case SAPP_KEYCODE_BACKSLASH:    return ImGuiKey_Backslash;
        case SAPP_KEYCODE_RIGHT_BRACKET:return ImGuiKey_RightBracket;
        case SAPP_KEYCODE_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case SAPP_KEYCODE_ESCAPE:       return ImGuiKey_Escape;
        case SAPP_KEYCODE_ENTER:        return ImGuiKey_Enter;
        case SAPP_KEYCODE_TAB:          return ImGuiKey_Tab;
        case SAPP_KEYCODE_BACKSPACE:    return ImGuiKey_Backspace;
        case SAPP_KEYCODE_INSERT:       return ImGuiKey_Insert;
        case SAPP_KEYCODE_DELETE:       return ImGuiKey_Delete;
        case SAPP_KEYCODE_RIGHT:        return ImGuiKey_RightArrow;
        case SAPP_KEYCODE_LEFT:         return ImGuiKey_LeftArrow;
        case SAPP_KEYCODE_DOWN:         return ImGuiKey_DownArrow;
        case SAPP_KEYCODE_UP:           return ImGuiKey_UpArrow;
        case SAPP_KEYCODE_PAGE_UP:      return ImGuiKey_PageUp;
        case SAPP_KEYCODE_PAGE_DOWN:    return ImGuiKey_PageDown;
        case SAPP_KEYCODE_HOME:         return ImGuiKey_Home;
        case SAPP_KEYCODE_END:          return ImGuiKey_End;
        case SAPP_KEYCODE_CAPS_LOCK:    return ImGuiKey_CapsLock;
        case SAPP_KEYCODE_SCROLL_LOCK:  return ImGuiKey_ScrollLock;
        case SAPP_KEYCODE_NUM_LOCK:     return ImGuiKey_NumLock;
        case SAPP_KEYCODE_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case SAPP_KEYCODE_PAUSE:        return ImGuiKey_Pause;
        case SAPP_KEYCODE_F1:           return ImGuiKey_F1;
        case SAPP_KEYCODE_F2:           return ImGuiKey_F2;
        case SAPP_KEYCODE_F3:           return ImGuiKey_F3;
        case SAPP_KEYCODE_F4:           return ImGuiKey_F4;
        case SAPP_KEYCODE_F5:           return ImGuiKey_F5;
        case SAPP_KEYCODE_F6:           return ImGuiKey_F6;
        case SAPP_KEYCODE_F7:           return ImGuiKey_F7;
        case SAPP_KEYCODE_F8:           return ImGuiKey_F8;
        case SAPP_KEYCODE_F9:           return ImGuiKey_F9;
        case SAPP_KEYCODE_F10:          return ImGuiKey_F10;
        case SAPP_KEYCODE_F11:          return ImGuiKey_F11;
        case SAPP_KEYCODE_F12:          return ImGuiKey_F12;
        case SAPP_KEYCODE_KP_0:         return ImGuiKey_Keypad0;
        case SAPP_KEYCODE_KP_1:         return ImGuiKey_Keypad1;
        case SAPP_KEYCODE_KP_2:         return ImGuiKey_Keypad2;
        case SAPP_KEYCODE_KP_3:         return ImGuiKey_Keypad3;
        case SAPP_KEYCODE_KP_4:         return ImGuiKey_Keypad4;
        case SAPP_KEYCODE_KP_5:         return ImGuiKey_Keypad5;
        case SAPP_KEYCODE_KP_6:         return ImGuiKey_Keypad6;
        case SAPP_KEYCODE_KP_7:         return ImGuiKey_Keypad7;
        case SAPP_KEYCODE_KP_8:         return ImGuiKey_Keypad8;
        case SAPP_KEYCODE_KP_9:         return ImGuiKey_Keypad9;
        case SAPP_KEYCODE_KP_DECIMAL:   return ImGuiKey_KeypadDecimal;
        case SAPP_KEYCODE_KP_DIVIDE:    return ImGuiKey_KeypadDivide;
        case SAPP_KEYCODE_KP_MULTIPLY:  return ImGuiKey_KeypadMultiply;
        case SAPP_KEYCODE_KP_SUBTRACT:  return ImGuiKey_KeypadSubtract;
        case SAPP_KEYCODE_KP_ADD:       return ImGuiKey_KeypadAdd;
        case SAPP_KEYCODE_KP_ENTER:     return ImGuiKey_KeypadEnter;
        case SAPP_KEYCODE_KP_EQUAL:     return ImGuiKey_KeypadEqual;
        case SAPP_KEYCODE_LEFT_SHIFT:   return ImGuiKey_LeftShift;
        case SAPP_KEYCODE_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case SAPP_KEYCODE_LEFT_ALT:     return ImGuiKey_LeftAlt;
        case SAPP_KEYCODE_LEFT_SUPER:   return ImGuiKey_LeftSuper;
        case SAPP_KEYCODE_RIGHT_SHIFT:  return ImGuiKey_RightShift;
        case SAPP_KEYCODE_RIGHT_CONTROL:return ImGuiKey_RightCtrl;
        case SAPP_KEYCODE_RIGHT_ALT:    return ImGuiKey_RightAlt;
        case SAPP_KEYCODE_RIGHT_SUPER:  return ImGuiKey_RightSuper;
        case SAPP_KEYCODE_MENU:         return ImGuiKey_Menu;
        default:                        return ImGuiKey_None;
    }
};

#if defined(__APPLE__)
    const ImGuiKey CopyPasteModifier = ImGuiMod_Super;
    const uint32_t CopyPasteModifierSokol = SAPP_MODIFIER_SUPER;
#else
    const ImGuiKey CopyPasteModifier = ImGuiMod_Ctrl;
    const uint32_t CopyPasteModifierSokol = SAPP_MODIFIER_CTRL;
#endif

    switch (ev->type) {
        case SAPP_EVENTTYPE_FOCUSED:
            io->AddFocusEvent(true);
            break;
        case SAPP_EVENTTYPE_UNFOCUSED:
            io->AddFocusEvent(false);
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            updateModifiers(ev->modifiers);
            io->AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io->AddMousePosEvent(ev->mouse_x/dpi_scale, ev->mouse_y/dpi_scale);
            io->AddMouseButtonEvent((int)ev->mouse_button, true);
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            updateModifiers(ev->modifiers);
            io->AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io->AddMousePosEvent(ev->mouse_x/dpi_scale, ev->mouse_y/dpi_scale);
            io->AddMouseButtonEvent((int)ev->mouse_button, false);
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            io->AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io->AddMousePosEvent(ev->mouse_x/dpi_scale, ev->mouse_y/dpi_scale);
            break;
        case SAPP_EVENTTYPE_MOUSE_ENTER:
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
            // FIXME: since the sokol_app.h emscripten backend doesn't support
            // mouse capture, mouse buttons must be released when the mouse leaves the
            // browser window, so that they don't "stick" when released outside the window.
            // A cleaner solution would be a new sokol_app.h function to query
            // "platform behaviour flags".
            io->AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            #if defined(__EMSCRIPTEN__)
            for (int i = 0; i < SAPP_MAX_MOUSEBUTTONS; i++) {
                io->AddMouseButtonEvent(i, false);
            }
            #endif
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            io->AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io->AddMouseWheelEvent(ev->scroll_x, ev->scroll_y);
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
            updateModifiers(ev->modifiers);
            // on web platform, don't forward Ctrl-X, Ctrl-V to the browser
            if ((ev->modifiers & CopyPasteModifierSokol) && ((ev->key_code == SAPP_KEYCODE_X) || (ev->key_code == SAPP_KEYCODE_C) )) {
                sapp_consume_event();
            }
            io->AddKeyEvent(mapKeyCode(ev->key_code), true);
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            updateModifiers(ev->modifiers);
            // intercept Ctrl-V, this is handled via EVENTTYPE_CLIPBOARD_PASTED
            if ((ev->modifiers & CopyPasteModifier) && (ev->key_code == SAPP_KEYCODE_V)) {
                break;
            }
            // on web platform, don't forward Ctrl-X, Ctrl-V to the browser
            if ((ev->modifiers & CopyPasteModifierSokol) && ((ev->key_code == SAPP_KEYCODE_X) || (ev->key_code == SAPP_KEYCODE_C) )) {
                sapp_consume_event();
            }
            io->AddKeyEvent(mapKeyCode(ev->key_code), false);
            break;
        case SAPP_EVENTTYPE_CHAR:
            updateModifiers(ev->modifiers);
            if ((ev->char_code >= 32) &&
                (ev->char_code != 127) &&
                (0 == (ev->modifiers & (SAPP_MODIFIER_ALT|SAPP_MODIFIER_CTRL|SAPP_MODIFIER_SUPER))))
            {
                io->AddInputCharacter(ev->char_code);
            }
            break;
        case SAPP_EVENTTYPE_CLIPBOARD_PASTED:
            // simulate a Ctrl-V key down/up
            io->AddKeyEvent(CopyPasteModifier, true);
            io->AddKeyEvent(ImGuiKey_V, true);
            io->AddKeyEvent(ImGuiKey_V, false);
            io->AddKeyEvent(CopyPasteModifier, false);
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