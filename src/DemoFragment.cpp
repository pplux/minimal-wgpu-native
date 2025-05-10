#include "demo.h"
#include <chrono>
#include <string>

#ifdef MINIMAL_WGPU_IMGUI
#include "imgui.h"
#endif

#ifndef __EMSCRIPTEN__
#define WGPU_C_STR(value) { value, WGPU_STRLEN }
#else
#define WGPU_C_STR(value) value
#endif

const char *initialFragmentCode = R"(
alias vec2f = vec2<f32>;
alias vec4f = vec4<f32>;

struct BufferInfo {
    size: vec2f,
    time: f32,
    frame: u32
};

@group(0) @binding(0)
var<uniform> info: BufferInfo;

@fragment
fn fs_main(@builtin(position) fragCoord: vec4f) -> @location(0) vec4f {
   var p = vec2f(fragCoord.x/info.size.x, fragCoord.y/info.size.y);
   return vec4f(p.x, p.y, fract(info.time), 1.0);
})";

extern const char *testSDF; // Look at the end of the file
extern const char *rickShader;

struct DemoFragment : public Demo {
    void init(WGPU*) override;
    void frame(WGPU*, WGPUTextureView) override;
    void cleanup(WGPU*) override;
    void onError(WGPU *, const char *message) override;
    void resize(WGPU*, uint32_t width, uint32_t height, float dpi) override;

    void replaceShaderCode(const char *shader);

    struct BufferInfo {
        float width;
        float height;
        float time;
        uint32_t frameNumber = 0;
    };

#ifdef MINIMAL_WGPU_IMGUI
    void imgui(WGPU*) override;
#endif

    void rebuild(WGPU*);
    void rebuildDone(WGPU*);

    WGPUShaderModule fragmentModule = {};

    std::string lastError = {};
    WGPURenderPipeline pipeline;
    WGPUShaderModule vertexShaderModule = {};
    WGPUBuffer gpuBufferInfo = {};
    WGPUBindGroupLayout bindGroupLayout = {};
    WGPUBindGroup bindGroup = {};

    char fragmentCode[65536];
    BufferInfo bufferInfo = {};
    std::chrono::high_resolution_clock::time_point startTime;
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

    // Buffer Info creation
    const WGPUBufferDescriptor bufferDescriptor = {
        .nextInChain = nullptr,
        .label = WGPU_C_STR("Uniform Buffer Info"),
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .size = sizeof(BufferInfo),
        .mappedAtCreation = false,
    };
    gpuBufferInfo = wgpuDeviceCreateBuffer(wgpu->device, &bufferDescriptor);


    WGPUBindGroupLayoutEntry layoutEntry = {
        .nextInChain = nullptr,
        .binding = 0,
        .visibility = WGPUShaderStage_Fragment,
        .buffer ={
            .nextInChain = nullptr,
            .type = WGPUBufferBindingType_Uniform,
            .hasDynamicOffset = false,
            .minBindingSize = sizeof(BufferInfo),
        }
    };

    WGPUBindGroupLayoutDescriptor groupLayoutDescriptor = {
        .nextInChain = nullptr,
        .label = WGPU_C_STR("BindGroupLayoutDescriptor"),
        .entryCount = 1,
        .entries = &layoutEntry,
    };

    bindGroupLayout = wgpuDeviceCreateBindGroupLayout(wgpu->device, &groupLayoutDescriptor);

    WGPUBindGroupEntry entry = {
        .nextInChain = nullptr,
        .binding = 0,
        .buffer = gpuBufferInfo,
        .offset = 0,
        .size = sizeof(BufferInfo),
    };

    WGPUBindGroupDescriptor groupDescriptor = {
        .nextInChain = nullptr,
        .label = WGPU_C_STR("Bind Group Description"),
        .layout = bindGroupLayout,
        .entryCount = 1,
        .entries = &entry,
    };
    bindGroup = wgpuDeviceCreateBindGroup(wgpu->device, &groupDescriptor);

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

    #ifdef MINIMAL_WGPU_IMGUI
    replaceShaderCode(initialFragmentCode);
    #else
    // If we are not using ImGUi use a more interesting shader than the initialFragment code
    replaceShaderCode(rickShader);
    #endif
    rebuild(wgpu);
    startTime = std::chrono::high_resolution_clock::now();
}

void DemoFragment::replaceShaderCode(const char* shader)
{
    memcpy(fragmentCode, shader, strlen(shader)+1);
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

void DemoFragment::resize(WGPU *, uint32_t width, uint32_t height, float dpi) {
    bufferInfo.width = width;
    bufferInfo.height = height;
}

void DemoFragment::cleanup(WGPU *) {
    wgpuRenderPipelineRelease(pipeline);
    wgpuShaderModuleRelease(vertexShaderModule);
    wgpuBufferRelease(gpuBufferInfo);
    wgpuBindGroupRelease(bindGroup);
    wgpuBindGroupLayoutRelease(bindGroupLayout);
}

void DemoFragment::frame(WGPU *wgpu, WGPUTextureView frame) {
    if (pipeline) {
        // update the buffer info
        bufferInfo.frameNumber++;
        bufferInfo.time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();
        wgpuQueueWriteBuffer(wgpu->queue, gpuBufferInfo, 0, &bufferInfo, sizeof(bufferInfo));

        // Create the command encoder to do the render pass
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
        wgpuRenderPassEncoderSetBindGroup(renderPassEncoder,0, bindGroup, 0, nullptr);
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
            .label = WGPU_C_STR("pipeline layout"),
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bindGroupLayout
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
        if (ImGui::Button("compile")) { rebuild(wgpu); }
        ImGui::SameLine();
        if (ImGui::Button("initial")) {
            replaceShaderCode(initialFragmentCode);
            rebuild(wgpu);
        }
        ImGui::SameLine();
        if (ImGui::Button("SDF")) {
            replaceShaderCode(testSDF);
            rebuild(wgpu);
        }
        ImGui::SameLine();
        if (ImGui::Button("Rick!")) {
            replaceShaderCode(rickShader);
            rebuild(wgpu);
        }
        const float width = ImGui::GetContentRegionAvail().x;
        ImGui::InputTextMultiline("###FragmentCode", fragmentCode, sizeof(fragmentCode), {width, 256});
        if (!lastError.empty()) {
            ImGui::TextUnformatted(lastError.c_str(), lastError.c_str()+lastError.length());
        }
        const ImVec2 size = ImGui::GetContentRegionAvail();
        imguiShowFrame(wgpu, size);
    }
    ImGui::End();
}
#endif

ADD_DEMO_WINDOW(fragment, createDemoFragment)

const char *testSDF = R"(// SDF functions -> https://danielchasehooper.com/posts/code-animated-rick/
alias vec2f = vec2<f32>;
alias vec3f = vec3<f32>;
alias vec4f = vec4<f32>;

struct BufferInfo {
    size: vec2f,
    time: f32,
    frame: u32
};

@group(0) @binding(0)
var<uniform> info: BufferInfo;

fn round_rect(p: vec2f, size: vec2f, radii: vec4f) -> f32 {
  var r1 = select(radii.xy, radii.zw, p.x < 0.0);
  var r = select(r1.x, r1.y, p.y < 0.0);
  var q = abs(p)-size + r;
  var res = min(max(q.x,q.y), 0.0) + length(max(q,vec2f(0.0))) - r;
  return res;
}

fn bezier(p: vec2f, vo0: vec2f, vo1: vec2f, vo2: vec2f) -> f32 {
  var i = vo0 - vo2;
  var j = vo2 - vo1;
  var k = vo1 - vo0;
  var w = j-k;
  var v0 = vo0 - p;
  var v1 = vo1 - p;
  var v2 = vo2 - p;
  var x = v0.x*v2.y-v0.y*v2.x;
  var y = v1.x*v0.y-v1.y*v0.x;
  var z = v2.x*v1.y-v2.y*v1.x;

  var s = 2.0*(y*j+z*k)-x*i;

  var r =  (y*z-x*x*0.25)/dot(s,s);
  var t = clamp( (0.5*x+y+r*dot(s,w))/(x+y+z),0.0,1.0);

  var d = v0+t*(k+k+t*w);
  var outQ = d + p;
  return length(d);
}

fn mod2(x: f32, y:f32) -> f32 {
  return x - y*floor(x/y);
}

fn star(pos: vec2f , r: f32, points: f32, ratio: f32) -> f32 {
    var an = 3.141593/points;
    var en = 3.141593/(ratio*(points-2.) + 2.);
    var  acs = vec2f(cos(an),sin(an));
    var  ecs = vec2f(cos(en),sin(en));

    var bn = mod2(atan2(pos.y,pos.x),2.0*an) - an;
    var p = length(pos)*vec2f(cos(bn),abs(sin(bn)));
    p -= r*acs;
    p += ecs*clamp( -dot(p,ecs), 0.0, r*acs.y/ecs.y);
    return length(p)*sign(p.x);
}


fn color(pixel : vec2f) -> vec3f {
   if (bezier(pixel, vec2f(-.7, -.35), vec2f(-1.5, -.4), vec2f(-1.2, .35)) < 0.1) { return vec3f(.9, .3, .3); }
   if (round_rect(pixel, vec2f(.3, .4), vec4f(.1)) < 0.0) { return vec3f(.3, .9, .3); }
   if (star(pixel - vec2f(1.,0.), .45, 5., .3) < 0.0) { return vec3f(.2, .4, .9); }
   return vec3f(1);
}

fn supersample(pos: vec2f, zoom: f32) -> vec3f {
   var px = .25/zoom;
   var py = .25/zoom;
   var posz = pos/zoom;
   var c =
      color(posz + vec2f(-px,-py)) +
      color(posz + vec2f(-px, py)) +
      color(posz + vec2f( px,-py)) +
      color(posz + vec2f( px, py));
   return c/4.0;
}

@fragment
fn fs_main(@builtin(position) fragCoord: vec4f) -> @location(0) vec4f {
  var px = (fragCoord.x - info.size.x/2);
  var py = -1.0*(fragCoord.y - info.size.y/2); // inverted Y compared to original GLSL code
  var zoom = 300.0;
  return vec4f(supersample(vec2f(px,py), zoom), 1.0);
}
)";

const char *rickShader = R"(// Animated Rick -> https://danielchasehooper.com/posts/code-animated-rick/

alias vec2f = vec2<f32>;
alias vec3f = vec3<f32>;
alias vec4f = vec4<f32>;
alias vec2i = vec2<i32>;

struct BufferInfo {
    size: vec2f,
    time: f32,
    frame: u32
};

@group(0) @binding(0)
var<uniform> info: BufferInfo;

fn grad(z: vec2i) -> vec2f  {
  var n: i32 = z.x+z.y*11111;
  n = (n<<13)^n;
  n = (n*(n*n*15731+789221)+1376312589)>>16;
  n &= 7;
  var gr = vec2f(f32(n&1),f32(n>>1))*2.0-1.0;
  if (n>=6) { return vec2f(0.0,gr.x);}
  if (n>=4) { return vec2f(gr.x,0.0);}
  return gr;
}

fn noise(p: vec2f) -> f32 {
  var i  = vec2i(floor(p));
  var f = fract(p);
  var u = f*f*(3.0-2.0*f);
  return mix( mix( dot( grad( i+vec2i(0,0) ), f-vec2f(0.0,0.0) ),
                   dot( grad( i+vec2i(1,0) ), f-vec2f(1.0,0.0) ), u.x),
              mix( dot( grad( i+vec2i(0,1) ), f-vec2f(0.0,1.0) ),
                   dot( grad( i+vec2i(1,1) ), f-vec2f(1.0,1.0) ), u.x), u.y);
}

fn warp(p: vec2f, scale : f32, strength: f32) -> vec2f {
  var offsetX = noise(p * scale + vec2f(0.0, 100.0));
  var offsetY = noise(p * scale + vec2f(100.0, 0.0));
  return p + vec2f(offsetX, offsetY) * strength;
}

fn circle(p: vec2f, radius: f32) -> f32 {
   return length(p) - radius;
}

fn round_rect(p: vec2f, size: vec2f, radii: vec4f) -> f32 {
  var r1 = select(radii.xy, radii.zw, p.x < 0.0);
  var r = select(r1.x, r1.y, p.y < 0.0);
  var q = abs(p)-size + r;
  var res = min(max(q.x,q.y), 0.0) + length(max(q,vec2f(0.0))) - r;
  return res;
}

fn bezier(p: vec2f, vo0: vec2f, vo1: vec2f, vo2: vec2f) -> f32 {
  var i = vo0 - vo2;
  var j = vo2 - vo1;
  var k = vo1 - vo0;
  var w = j-k;
  var v0 = vo0 - p;
  var v1 = vo1 - p;
  var v2 = vo2 - p;
  var x = v0.x*v2.y-v0.y*v2.x;
  var y = v1.x*v0.y-v1.y*v0.x;
  var z = v2.x*v1.y-v2.y*v1.x;
  var s = 2.0*(y*j+z*k)-x*i;
  var r =  (y*z-x*x*0.25)/dot(s,s);
  var t = clamp( (0.5*x+y+r*dot(s,w))/(x+y+z),0.0,1.0);
  var d = v0+t*(k+k+t*w);
  var outQ = d + p;
  return length(d);
}

fn mod2(x: f32, y:f32) -> f32 {
  return x - y*floor(x/y);
}

fn parabola(pixel: vec2f, k: f32) -> f32 {
  var pos = pixel;
  pos.x = abs(pos.x);
  var ik = 1.0/k;
  var p = ik*(pos.y - 0.5*ik)/3.0;
  var q = 0.25*ik*ik*pos.x;
  var h = q*q - p*p*p;
  var r = sqrt(abs(h));
  var x = select(
    pow(q+r,1.0/3.0) - pow(abs(q-r),1.0/3.0)*sign(r-q),
    2.0*cos(atan2(r,q)/3.0)*sqrt(p),
    h <= .0);
  return length(pos-vec2f(x,k*x*x)) * sign(pos.x-x);
}

fn star(pos: vec2f , r: f32, points: f32, ratio: f32) -> f32 {
  var an = 3.141593/points;
  var en = 3.141593/(ratio*(points-2.) + 2.);
  var  acs = vec2f(cos(an),sin(an));
  var  ecs = vec2f(cos(en),sin(en));

  var bn = mod2(atan2(pos.y,pos.x),2.0*an) - an;
  var p = length(pos)*vec2f(cos(bn),abs(sin(bn)));
  p -= r*acs;
  p += ecs*clamp( -dot(p,ecs), 0.0, r*acs.y/ecs.y);
  return length(p)*sign(p.x);
}

fn map(value:f32, inMin:f32, inMax:f32, outMin:f32 , outMax:f32) -> f32 {
  var v = clamp(value, inMin, inMax);
  return outMin + (outMax - outMin) * (v - inMin) / (inMax - inMin);
}

fn rotateAt(p:vec2f, angle:f32, origin:vec2f ) -> vec2f {
    var s = sin(angle);
    var c = cos(angle);
    return (p-origin)*mat2x2<f32>( c, -s, s, c ) + origin;
}

fn color(pixelRaw : vec2f) -> vec3f {
    // rotate the whole drawing
    var pixel = rotateAt(pixelRaw, sin(info.time*2.)*.1, vec2f(0,-.6));
    pixel.y += .1;
    {
        // Blink eyes
        if (mod2(info.time, 2.) < .09) {
            // closed eyes
            var d = round_rect(pixel+vec2f(.07,-.16), vec2f(.24,0), vec4(0));
            if (d < .008) { return vec3f(0); }
        }
        else {
            // move pupils randomly
            var pupil_warp = pixel + vec2f(.095,-.18);
            pupil_warp.x -= noise(vec2f(round(info.time)*7.+.5, 0.5))*.1;
            pupil_warp.y -= noise(vec2f(round(info.time)*9.+.5, 0.5))*.1;
            pupil_warp.x = abs(pupil_warp.x) - .16;
            var d = star(pupil_warp, 0.019, 6., .9);
            if (d < 0.007) {
                return vec3f(.1);
            }

            // Eyeballs
            var eye = vec2f(abs(pixel.x+.1)-.17, pixel.y*.93 - .16);
            d = length(eye) - .16;
            if (d < 0.) { return vec3f(step(.013, -d)); }

            // under eye lines
            var should_show = pixel.y < 0.25 &&
                (abs(pixel.x+.29) < .05 ||
                abs(pixel.x-.12) < .085);
            if (abs(d - .04) < .0055 && should_show) { return vec3f(0); }
        }


            // Mouth
            var d = bezier(pixel,
                         vec2f(-.26, -.28),
                         vec2f(-.05,-.42),
                         vec2f(.115, -.25));
            if (d < .11) {
                // Teeth
                var width = .065;
                var teeth = pixel;
                teeth.x = mod2(teeth.x, width)-width*.5;
                teeth.y -= pow(pixel.x+.09, 2.) * 1.5 - .34;
                teeth.y = abs(teeth.y)-.06;
                d = parabola(teeth, 38.);
                if (d < 0. && abs(pixel.x+.06) < .194)
                    { return vec3f(0.902, 0.890, 0.729)*step(d, -.01); }

                // Tongue
                // `map()` is used to change the thickness of
                // the tongue along the x axis
                var tongue = rotateAt(pixel, sin(info.time*2.-1.5)*.15+.1, vec2f(0,-.5));
                var tongue_thickness = map(tongue.x, -.16, .01, .02, .045);
                d = bezier(tongue,
                    vec2f(-.16, -.35),
                    vec2f(.001,-.33),
                    vec2f(.01, -.5)) - tongue_thickness;
                if (d < 0.0)
                    { return vec3f(0.816, 0.302, 0.275)*step(d, -0.01); }

                // mouth fill color
                return vec3f(.42, .147, .152);
            }

            // lip outlines
            if (d < .12 || (abs(d-.16) < .005
                            && (pixel.x*-6.4 > -pixel.y+1.6
                              || pixel.x*1.7 > -pixel.y+.1
                              || pixel.y < -0.49)))
                { return vec3f(0); }

            // lips
            if (d < .16) { return vec3f(.838, .799, 0.76); }



            // Nose
            d = min(
                    bezier(pixel,
                        vec2f(-.15, -.13),
                        vec2f(-.21,-.14),
                        vec2f(-.14, .08)),
                    bezier(pixel,
                        vec2f(-.085, -.01),
                        vec2f(-.12, -.13),
                        vec2f(-.15,-.13)));
            if (d < 0.0055) { return vec3f(0); }


        // Eyebrow
        d = bezier(pixel,
                vec2f(-.34, .38),
                // NEW animate the middle up and down
                vec2f(-.05, 0.5 + cos(info.time)*.1),
                vec2f(.205, .36)) - 0.035;
        if (d < 0.0)
            { return vec3f(.71, .839, .922)*step(d, -.013); }

            d = min(
                // Head
                round_rect(
                pixel,
                vec2f(.36, .6385),
                vec4(.34, .415, .363, .315)),

                // Ear
                round_rect(
                pixel + vec2f(-.32, .15),
                vec2f(.15, 0.12),
                vec4(.13,.1,.13,.13))
            );

            if (d < 0.) { return vec3f(.838, .799, .76)*step(d, -.01); }
    }

    // Hair
    var twist = sin(info.time*2.-length(pixel)*2.1)*.12;
    var hair = rotateAt(pixel, twist, vec2f(0.,.1));
    hair -= vec2f(.08,.15);
    hair.x *= 1.3;
    hair = warp(hair, 4.0, 0.07);
    var d = star(hair, 0.95, 11., .28);
    if (d < 0.) {
        return vec3f(0.682, 0.839, 0.929)*step(d, -0.012);
    }

    return vec3f(1.0);
}


fn supersample(pos: vec2f, zoom: f32) -> vec3f {
   var px = .25/zoom;
   var py = .25/zoom;
   var posz = pos/zoom;
   var c =
      color(posz + vec2f(-px,-py)) +
      color(posz + vec2f(-px, py)) +
      color(posz + vec2f( px,-py)) +
      color(posz + vec2f( px, py));
   return c/4.0;
}

@fragment
fn fs_main(@builtin(position) fragCoord: vec4f) -> @location(0) vec4f {
  var px = (fragCoord.x - info.size.x/2);
  var py = -1.0*(fragCoord.y - info.size.y/2); // inverted Y compared to original GLSL code
  var zoom = 250.0;
  return vec4f(supersample(vec2f(px,py), zoom), 1.0);
}
)";
