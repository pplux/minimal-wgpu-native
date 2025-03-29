#include <vector>
#include "demo.h"
#include "sokol_app.h"

#ifndef __EMSCRIPTEN__
#define IMGUI_IMPL_WEBGPU_BACKEND_WGPU
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include <iostream>
#include <ostream>

#include "imgui.h"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_tables.cpp"
#include "imgui_demo.cpp"
#include "backends/imgui_impl_wgpu.cpp"

struct DemoWindow {
    std::unique_ptr<Demo> window;
    uint32_t width = 0;
    uint32_t height = 0;
    bool opened = true;
    std::string name;
    WGPUTexture texture = nullptr;
    WGPUTextureView view = {};
};

struct DemoImgui : public Demo {
    virtual void init(WGPU*);
    virtual void frame(WGPU*, WGPUTextureView);
    virtual void cleanup(WGPU*);
    virtual void resize(WGPU*, uint32_t width, uint32_t height, float dpi);
    virtual void event(WGPU *wpgu, const sapp_event* ev);

    std::vector<DemoWindow> windows;
};

static DemoImgui *imgui = nullptr;

std::unique_ptr<Demo> createDemoImgui() {
    assert(imgui == nullptr);
    auto demo = std::make_unique<DemoImgui>();
    imgui = demo.get();
    return std::move(demo);
}

void addDemoWindow(std::unique_ptr<Demo> &&demo) {
    assert(imgui != nullptr);
    DemoWindow window = {};
    window.window = std::move(demo);
    window.name = "Demo-" + std::to_string(imgui->windows.size());
    imgui->windows.push_back(std::move(window));
}

void DemoImgui::init(WGPU *wgpu) {
    ImGui::CreateContext();
    ImGui_ImplWGPU_InitInfo init_info = {};
    init_info.Device = wgpu->device;
    init_info.NumFramesInFlight = 3;
    init_info.RenderTargetFormat = wgpu->surfaceFormat;
    init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    ImGui_ImplWGPU_Init(&init_info);

    for(auto &&w: windows) {
        w.window->init(wgpu);
    }
}

void DemoImgui::cleanup(WGPU *wgpu) {
    for(auto &&w: windows) {
        w.window->cleanup(wgpu);
    }
    ImGui_ImplWGPU_Shutdown();
}

void DemoImgui::resize(WGPU *, uint32_t width, uint32_t height, float dpi) {
    ImGui_ImplWGPU_InvalidateDeviceObjects();
    ImGui_ImplWGPU_CreateDeviceObjects();
    ImGuiIO &io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width/dpi, (float)height/dpi);
    io.DisplayFramebufferScale = ImVec2(dpi, dpi);
}


void DemoImgui::event(WGPU *, const sapp_event* ev) {
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


void DemoImgui::frame(WGPU *wgpu, WGPUTextureView frame) {
    ImGuiIO &io = ImGui::GetIO();

    bool invalidateObjects = false;
    ImGui_ImplWGPU_NewFrame();
    ImGui::NewFrame();
    static bool show_demo_window = true;

    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    for(auto &&w: windows) {
        if (ImGui::Begin(w.name.c_str())) {
            const ImVec2 size = ImGui::GetContentRegionAvail();
            const auto width = (uint32_t) size.x;
            const auto height = (uint32_t) size.y;
            const bool valid = (width > 0) && (height > 0);
            if (!valid) {
                ImGui::End();
                continue;
            };

            const bool resize = (width != w.width) || (height != w.height);
            if (resize) {
                const float dpi = sapp_dpi_scale();
                w.window->resize(wgpu, width, height, dpi);
                w.width = width;
                w.height = height;

                if (w.texture) {
                    wgpuTextureRelease(w.texture);
                    wgpuTextureViewRelease(w.view);
                    invalidateObjects = true;
                }

                // Create a texture with the new size
                WGPUTextureDescriptor descriptor = {};
                descriptor.size.width = w.width;
                descriptor.size.height = w.height;
                descriptor.size.depthOrArrayLayers = 1;
                descriptor.mipLevelCount = 1;
                descriptor.sampleCount = 1;
                descriptor.dimension = WGPUTextureDimension_2D;
                descriptor.format = wgpu->surfaceFormat;
                descriptor.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding;
                w.texture = wgpuDeviceCreateTexture(wgpu->device, &descriptor);

                // Create a texture view
                WGPUTextureViewDescriptor viewDescriptor = {};
                viewDescriptor.format = wgpu->surfaceFormat;
                viewDescriptor.dimension = WGPUTextureViewDimension_2D;
                viewDescriptor.mipLevelCount = 1;
                viewDescriptor.arrayLayerCount = 1;
                viewDescriptor.baseArrayLayer = 0;
                viewDescriptor.baseMipLevel = 0;
                w.view = wgpuTextureCreateView(w.texture, &viewDescriptor);
            }
            w.window->frame(wgpu, w.view);
            ImGui::Image((ImTextureID)w.view, size);
        }
        ImGui::End();
    }

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

    wgpuCommandBufferRelease(cmd_buffer);
    wgpuCommandEncoderRelease(encoder);

    if (invalidateObjects) {
        ImGui_ImplWGPU_InvalidateDeviceObjects();
    }
}

