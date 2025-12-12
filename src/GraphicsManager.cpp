#include "GraphicsManager.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#define GLFW_INCLUDE_NONE
#include <webgpu/webgpu.h>
#include <glfw3webgpu.h>
#include "GLFW/glfw3.h"
#include "spdlog/spdlog.h"

// --- Helpers ---
template< typename T > constexpr const T* to_ptr( const T& val ) { return &val; }
template< typename T, std::size_t N > constexpr const T* to_ptr( const T (&&arr)[N] ) { return arr; }

// Global targets
static WGPUAdapter* g_AdapterTarget = nullptr;
static WGPUDevice* g_DeviceTarget = nullptr;

// State tracking
static uint32_t g_CurrentWidth = 0;
static uint32_t g_CurrentHeight = 0;

static WGPUTextureFormat wgpuSurfaceGetPreferredFormat( WGPUSurface surface, WGPUAdapter adapter ) {
    WGPUSurfaceCapabilities capabilities{};
    wgpuSurfaceGetCapabilities( surface, adapter, &capabilities );
    WGPUTextureFormat result = WGPUTextureFormat_BGRA8Unorm;
    if (capabilities.formatCount > 0) result = capabilities.formats[0];
    wgpuSurfaceCapabilitiesFreeMembers( capabilities );
    return result;
}

namespace gspengine {

    GraphicsManager::GraphicsManager() {}
    GraphicsManager::~GraphicsManager() { Shutdown(); }

    void GraphicsManager::Startup() {
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
        
        if (!glfwInit()) {
            spdlog::error("Failed to initialize GLFW");
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        // FORCE SHOW IMMEDIATELY
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE); 
        
        m_window = glfwCreateWindow(800, 600, "GSP Engine - FINAL", nullptr, nullptr);

        if (!m_window) {
            spdlog::error("Failed to create a window.");
            glfwTerminate();
            return;
        }

        // 1. Instance
        WGPUInstanceDescriptor desc = {};
        desc.nextInChain = nullptr;
        m_instance = wgpuCreateInstance(&desc);
        if (!m_instance) { spdlog::error("Failed to create Instance"); return; }
        spdlog::info("WebGPU Instance created");

        // 2. Surface
        m_surface = glfwCreateWindowWGPUSurface(m_instance, m_window);

        // 3. Adapter
        WGPURequestAdapterOptions adapterOpts = {};
        adapterOpts.compatibleSurface = m_surface;
        adapterOpts.backendType = WGPUBackendType_Vulkan; 
        
        g_AdapterTarget = &m_adapter;

        wgpuInstanceRequestAdapter(m_instance, &adapterOpts, 
            WGPURequestAdapterCallbackInfo {
                .nextInChain = nullptr,
                .mode = WGPUCallbackMode_AllowSpontaneous,
                .callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* u1, void* u2) {
                    if (status == WGPURequestAdapterStatus_Success) {
                        if (g_AdapterTarget) *g_AdapterTarget = adapter;
                    }
                }
            }
        );

        while (!m_adapter) wgpuInstanceProcessEvents(m_instance);
        g_AdapterTarget = nullptr;
        spdlog::info("Adapter Found");

        // 4. Device
        WGPUDeviceDescriptor deviceDesc = {};
        deviceDesc.label = WGPUStringView{ "My Device", 9 };
        deviceDesc.requiredFeatureCount = 0;
        deviceDesc.requiredLimits = nullptr;
        deviceDesc.defaultQueue.label = WGPUStringView{ "The Queue", 9 };
        
        deviceDesc.uncapturedErrorCallbackInfo = WGPUUncapturedErrorCallbackInfo {
            .callback = [](const WGPUDevice* device, WGPUErrorType type, WGPUStringView message, void* u1, void* u2) {
                spdlog::error("WebGPU Error: {}", std::string(message.data, message.length));
            }
        };

        g_DeviceTarget = &m_device;

        wgpuAdapterRequestDevice(m_adapter, &deviceDesc, 
            WGPURequestDeviceCallbackInfo {
                .nextInChain = nullptr,
                .mode = WGPUCallbackMode_AllowSpontaneous,
                .callback = [](WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* u1, void* u2) {
                    if (status == WGPURequestDeviceStatus_Success) {
                        if (g_DeviceTarget) *g_DeviceTarget = device;
                    }
                }
            }
        );

        while (!m_device) wgpuInstanceProcessEvents(m_instance);
        g_DeviceTarget = nullptr;
        spdlog::info("Device Created");

        // 5. Queue
        m_queue = wgpuDeviceGetQueue(m_device);

        // 6. Surface Configuration
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height); // Get ACTUAL size
        g_CurrentWidth = (uint32_t)width;
        g_CurrentHeight = (uint32_t)height;

        WGPUSurfaceConfiguration config = {};
        config.device = m_device;
        config.format = WGPUTextureFormat_BGRA8Unorm; 
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.width = g_CurrentWidth;
        config.height = g_CurrentHeight;
        config.presentMode = WGPUPresentMode_Immediate; // FORCE IMMEDIATE
        config.alphaMode = WGPUCompositeAlphaMode_Opaque; 

        wgpuSurfaceConfigure(m_surface, &config);

        InitializePipeline();
        CreateBuffers(); 
        
        spdlog::info("GraphicsManager started successfully");
    }

    // Shader Loader
    WGPUShaderModule GraphicsManager::LoadShaderModule(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::error("Could not open shader file: {}", path);
            return nullptr;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string shaderCode = buffer.str();

        WGPUShaderSourceWGSL wgslDesc{};
        wgslDesc.chain.next = nullptr;
        wgslDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
        wgslDesc.code = WGPUStringView{ shaderCode.c_str(), shaderCode.length() };

        WGPUShaderModuleDescriptor descriptor{};
        descriptor.nextInChain = &wgslDesc.chain;
        descriptor.label = WGPUStringView{ path.c_str(), path.length() };

        return wgpuDeviceCreateShaderModule(m_device, &descriptor);
    }

    // Initialize Pipeline
    void GraphicsManager::InitializePipeline() {
        WGPUShaderModule shaderModule = LoadShaderModule("assets/shaders/shader.wgsl");
        if (!shaderModule) return;

        // Vertex
        WGPUVertexAttribute vertAttributes[2];
        vertAttributes[0].shaderLocation = 0; 
        vertAttributes[0].format = WGPUVertexFormat_Float32x2;
        vertAttributes[0].offset = 0;
        vertAttributes[1].shaderLocation = 1; 
        vertAttributes[1].format = WGPUVertexFormat_Float32x2;
        vertAttributes[1].offset = 2 * sizeof(float);

        WGPUVertexBufferLayout vertexBufferLayout{};
        vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
        vertexBufferLayout.arrayStride = 4 * sizeof(float);
        vertexBufferLayout.attributeCount = 2;
        vertexBufferLayout.attributes = vertAttributes;

        // Instance
        WGPUVertexAttribute instAttributes[2];
        instAttributes[0].shaderLocation = 2; 
        instAttributes[0].format = WGPUVertexFormat_Float32x3;
        instAttributes[0].offset = offsetof(InstanceData, translation);
        instAttributes[1].shaderLocation = 3; 
        instAttributes[1].format = WGPUVertexFormat_Float32x2;
        instAttributes[1].offset = offsetof(InstanceData, scale);

        WGPUVertexBufferLayout instanceBufferLayout{};
        instanceBufferLayout.stepMode = WGPUVertexStepMode_Instance;
        instanceBufferLayout.arrayStride = sizeof(InstanceData);
        instanceBufferLayout.attributeCount = 2;
        instanceBufferLayout.attributes = instAttributes;

        WGPUVertexBufferLayout layouts[] = { vertexBufferLayout, instanceBufferLayout };

        // Pipeline
        WGPURenderPipelineDescriptor pipelineDesc{};
        pipelineDesc.label = WGPUStringView{ "Main Pipeline", 13 };
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = WGPUStringView{ "vertex_shader_main", 18 };
        pipelineDesc.vertex.bufferCount = 2;
        pipelineDesc.vertex.buffers = layouts;

        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
        pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
        pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
        pipelineDesc.primitive.cullMode = WGPUCullMode_None;

        WGPUBlendState blendState{};
        blendState.color.operation = WGPUBlendOperation_Add;
        blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
        blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
        blendState.alpha.operation = WGPUBlendOperation_Add;
        blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
        blendState.alpha.dstFactor = WGPUBlendFactor_One;

        WGPUColorTargetState colorTarget{};
        colorTarget.format = WGPUTextureFormat_BGRA8Unorm; 
        colorTarget.blend = &blendState;
        colorTarget.writeMask = WGPUColorWriteMask_All;

        WGPUFragmentState fragmentState{};
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = WGPUStringView{ "fragment_shader_main", 20 };
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;

        pipelineDesc.fragment = &fragmentState;
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = ~0u;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        m_pipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);
        if (m_pipeline) spdlog::info("Render Pipeline initialized successfully");
        else spdlog::error("Failed to create Render Pipeline");

        wgpuShaderModuleRelease(shaderModule);
    }

    // Create Buffers
    void GraphicsManager::CreateBuffers() {
        const float vertices[] = {
            -1.0f, -1.0f, 0.0f, 1.0f, 
             1.0f, -1.0f, 1.0f, 1.0f, 
            -1.0f,  1.0f, 0.0f, 0.0f, 
             1.0f,  1.0f, 1.0f, 0.0f, 
        };

        WGPUBufferDescriptor vertDesc = {};
        vertDesc.label = WGPUStringView{ "Vertex Buffer", 13 };
        vertDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        vertDesc.size = sizeof(vertices);
        
        m_vertexBuffer = wgpuDeviceCreateBuffer(m_device, &vertDesc);
        wgpuQueueWriteBuffer(m_queue, m_vertexBuffer, 0, vertices, sizeof(vertices));

        WGPUBufferDescriptor uniformDesc = {};
        uniformDesc.label = WGPUStringView{ "Uniform Buffer", 14 };
        uniformDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
        uniformDesc.size = sizeof(Uniforms);
        m_uniformBuffer = wgpuDeviceCreateBuffer(m_device, &uniformDesc);

        WGPUBufferDescriptor instDesc = {};
        instDesc.label = WGPUStringView{ "Instance Buffer", 15 };
        instDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        instDesc.size = sizeof(InstanceData) * 100;
        m_instanceBuffer = wgpuDeviceCreateBuffer(m_device, &instDesc);

        spdlog::info("GPU Buffers created successfully");
    }

    // Draw Loop
    void GraphicsManager::Draw() {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        
        if (width == 0 || height == 0) return;

        // CRITICAL FIX: Only reconfigure if the PIXEL SIZE actually changed.
        // We do NOT trust the "Outdated" status blindly anymore.
        if (static_cast<uint32_t>(width) != g_CurrentWidth || static_cast<uint32_t>(height) != g_CurrentHeight) {
            g_CurrentWidth = width;
            g_CurrentHeight = height;

            WGPUSurfaceConfiguration config = {};
            config.device = m_device;
            config.format = WGPUTextureFormat_BGRA8Unorm; 
            config.usage = WGPUTextureUsage_RenderAttachment;
            config.width = static_cast<uint32_t>(width);
            config.height = static_cast<uint32_t>(height);
            config.presentMode = WGPUPresentMode_Immediate; 
            config.alphaMode = WGPUCompositeAlphaMode_Opaque; 
            
            wgpuSurfaceConfigure(m_surface, &config);
            spdlog::info("Resized Surface to {}x{}", width, height);
        }

        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(m_surface, &surfaceTexture);

        // If it's still bad after we ensured size match, skip this frame.
        if (surfaceTexture.status != 0) {
            return;
        }

        WGPUTextureViewDescriptor viewDesc = {};
        viewDesc.label = WGPUStringView{ "Surface View", 12 };
        viewDesc.format = WGPUTextureFormat_BGRA8Unorm; 
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = WGPUTextureAspect_All;

        WGPUTextureView view = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);

        WGPUCommandEncoderDescriptor encoderDesc = {};
        encoderDesc.label = WGPUStringView{ "My Encoder", 10 };
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device, &encoderDesc);

        WGPURenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = view;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        // GREEN SCREEN
        colorAttachment.clearValue = WGPUColor{ 0.0, 1.0, 0.0, 1.0 }; 

        #ifndef WEBGPU_BACKEND_WGPU
        colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        #endif

        WGPURenderPassDescriptor renderPassDesc = {};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;

        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        
        if (m_pipeline) {
            wgpuRenderPassEncoderSetPipeline(renderPass, m_pipeline);
        }
        
        wgpuRenderPassEncoderEnd(renderPass);
        wgpuRenderPassEncoderRelease(renderPass);

        WGPUCommandBufferDescriptor cmdDesc = {};
        WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(encoder, &cmdDesc);
        wgpuQueueSubmit(m_queue, 1, &command_buffer);

        wgpuSurfacePresent(m_surface);

        wgpuCommandBufferRelease(command_buffer);
        wgpuCommandEncoderRelease(encoder);
        wgpuTextureViewRelease(view);
        wgpuTextureRelease(surfaceTexture.texture);
    }

    void GraphicsManager::Shutdown() {
        if (m_instanceBuffer) wgpuBufferRelease(m_instanceBuffer);
        if (m_uniformBuffer) wgpuBufferRelease(m_uniformBuffer);
        if (m_vertexBuffer) wgpuBufferRelease(m_vertexBuffer);
        if (m_pipeline) wgpuRenderPipelineRelease(m_pipeline);
        if (m_queue) wgpuQueueRelease(m_queue);
        if (m_device) wgpuDeviceRelease(m_device);
        if (m_adapter) wgpuAdapterRelease(m_adapter);
        if (m_surface) wgpuSurfaceRelease(m_surface);
        if (m_instance) wgpuInstanceRelease(m_instance);

        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
        spdlog::info("GraphicsManager shut down");
    }

    bool GraphicsManager::ShouldClose() const {
        return m_window && glfwWindowShouldClose(m_window);
    }

    void GraphicsManager::SetWindowShouldClose(bool value) {
        if (m_window) glfwSetWindowShouldClose(m_window, value);
    }

    GLFWwindow* GraphicsManager::GetWindow() const {
        return m_window;
    }
}
