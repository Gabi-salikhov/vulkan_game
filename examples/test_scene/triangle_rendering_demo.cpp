#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

// Include Vortex Engine components
#include "engine/core/vulkan_context.h"
#include "engine/core/window.h"
#include "engine/renderer/buffer_allocator.h"
#include "engine/renderer/command_buffer.h"
#include "engine/renderer/pipeline_system.h"
#include "engine/renderer/shader_system.h"
#include "engine/renderer/synchronization.h"

// Vertex data for a triangle
struct Vertex {
    float pos[3];
    float color[3];
};

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Bottom vertex - Red
    {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Top-left vertex - Green
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}    // Top-right vertex - Blue
};

const std::vector<uint32_t> indices = {0, 1, 2};

class TriangleRenderer {
   public:
    TriangleRenderer(SDL_Window* window);
    ~TriangleRenderer();

    bool initialize();
    void render();
    void cleanup();

   private:
    bool createRenderPass();
    bool createFramebuffers();
    bool createPipeline();
    bool createVertexBuffer();
    bool createIndexBuffer();
    bool createCommandBuffers();
    bool createCommandPool();

    void recordCommandBuffer(uint32_t imageIndex);
    void updateUniformBuffer(uint32_t currentImage);

    // Vulkan objects
    SDL_Window* m_window;
    std::unique_ptr<VortexEngine::VulkanContext> m_vulkanContext;
    std::unique_ptr<VortexEngine::Window> m_windowSystem;
    std::unique_ptr<VortexEngine::BufferAllocator> m_bufferAllocator;
    std::unique_ptr<VortexEngine::ShaderSystem> m_shaderSystem;
    std::unique_ptr<VortexEngine::PipelineSystem> m_pipelineSystem;
    std::unique_ptr<VortexEngine::CommandBufferManager> m_commandBufferManager;
    std::unique_ptr<VortexEngine::SyncObjects> m_syncObjects;

    // Rendering objects
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_framebuffers;
    VkPipeline m_pipeline;
    VkPipelineLayout m_pipelineLayout;

    // Buffer objects
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;
    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    // Command pool
    VkCommandPool m_commandPool;

    // Swapchain
    uint32_t m_swapchainImageCount;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;

    // Timing
    std::chrono::high_resolution_clock::time_point m_startTime;
    float m_time = 0.0f;
};

TriangleRenderer::TriangleRenderer(SDL_Window* window)
    : m_window(window),
      m_vulkanContext(nullptr),
      m_windowSystem(nullptr),
      m_bufferAllocator(nullptr),
      m_shaderSystem(nullptr),
      m_pipelineSystem(nullptr),
      m_commandBufferManager(nullptr),
      m_syncObjects(nullptr),
      m_renderPass(VK_NULL_HANDLE),
      m_pipeline(VK_NULL_HANDLE),
      m_pipelineLayout(VK_NULL_HANDLE),
      m_vertexBuffer(VK_NULL_HANDLE),
      m_vertexBufferMemory(VK_NULL_HANDLE),
      m_indexBuffer(VK_NULL_HANDLE),
      m_indexBufferMemory(VK_NULL_HANDLE),
      m_commandPool(VK_NULL_HANDLE) {
    m_startTime = std::chrono::high_resolution_clock::now();
}

TriangleRenderer::~TriangleRenderer() { cleanup(); }

bool TriangleRenderer::initialize() {
    try {
        std::cout << "Initializing Triangle Renderer..." << std::endl;

        // Initialize window system
        m_windowSystem = std::make_unique<VortexEngine::Window>();
        if (!m_windowSystem->initialize("Triangle Renderer", 800, 600)) {
            std::cerr << "Failed to initialize window system" << std::endl;
            return false;
        }

        // Initialize Vulkan context
        m_vulkanContext = std::make_unique<VortexEngine::VulkanContext>();

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Triangle Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vortex Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        if (!m_vulkanContext->initialize(createInfo)) {
            std::cerr << "Failed to initialize Vulkan context" << std::endl;
            return false;
        }
        
        // Create a surface for the window using our Window class
        VkSurfaceKHR surface;
        if (!m_windowSystem->createVulkanSurface(m_vulkanContext->getInstance(), &surface)) {
            std::cerr << "Failed to create window surface" << std::endl;
            return false;
        }
        
        // Store the surface in the Vulkan context
        m_vulkanContext->setSurface(surface);

        // Create swapchain
        if (!m_vulkanContext->createSwapChain(surface)) {
            std::cerr << "Failed to create swap chain" << std::endl;
            return false;
        }

        // Get swapchain info
        m_swapchainImageCount = m_vulkanContext->getSwapChainImageCount();
        m_swapchainImageFormat = m_vulkanContext->getSwapChainImageFormat();
        m_swapchainExtent = m_vulkanContext->getSwapChainExtent();

        std::cout << "Swapchain created with " << m_swapchainImageCount << " images" << std::endl;

        // Initialize subsystems (without pipeline system yet)
        m_bufferAllocator = std::make_unique<VortexEngine::BufferAllocator>();
        m_bufferAllocator->initialize(m_vulkanContext->getDevice(), m_vulkanContext->getPhysicalDevice(), nullptr);
        
        m_shaderSystem = std::make_unique<VortexEngine::ShaderSystem>();
        m_shaderSystem->initialize(m_vulkanContext->getDevice());
        
        // Create command pool
        if (!createCommandPool()) {
            return false;
        }
        
        m_commandBufferManager = std::make_unique<VortexEngine::CommandBufferManager>(
            m_vulkanContext->getDevice(), m_commandPool);
        
        m_syncObjects = std::make_unique<VortexEngine::SyncObjects>(m_vulkanContext->getDevice(),
                                                                    m_swapchainImageCount);

        // Load shaders
        if (!m_shaderSystem->loadShader("triangle", "shaders/common/common.vert.spv", "shaders/common/common.frag.spv")) {
            std::cerr << "Failed to load shaders" << std::endl;
            return false;
        }
        
        // Verify shaders were loaded successfully
        VkShaderModule vertexShader = m_shaderSystem->getVertexShader("triangle");
        VkShaderModule fragmentShader = m_shaderSystem->getFragmentShader("triangle");
        
        if (vertexShader == VK_NULL_HANDLE || fragmentShader == VK_NULL_HANDLE) {
            std::cerr << "Shader modules are invalid after loading!" << std::endl;
            std::cerr << "Vertex shader: " << vertexShader << std::endl;
            std::cerr << "Fragment shader: " << fragmentShader << std::endl;
            return false;
        }
        
        std::cout << "Shaders loaded successfully" << std::endl;

        // Create render pass
        if (!createRenderPass()) {
            return false;
        }
        
        // Skip pipeline system for now and create pipeline directly
        // m_pipelineSystem = std::make_unique<VortexEngine::PipelineSystem>();
        // m_pipelineSystem->initialize(m_vulkanContext->getDevice(), m_renderPass);

        // Create pipeline using direct Vulkan API
        if (!createPipeline()) {
            return false;
        }

        // Create framebuffers
        if (!createFramebuffers()) {
            return false;
        }

        // Create vertex and index buffers
        if (!createVertexBuffer()) {
            return false;
        }

        if (!createIndexBuffer()) {
            return false;
        }

        // Create command buffers
        if (!createCommandBuffers()) {
            return false;
        }

        // Create sync objects
        if (!m_syncObjects->create()) {
            return false;
        }

        std::cout << "Triangle Renderer initialized successfully" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during initialization: " << e.what() << std::endl;
        return false;
    }
}

void TriangleRenderer::render() {
    if (!m_syncObjects) {
        return;
    }

    // Begin frame synchronization
    m_syncObjects->beginFrame();

    // Acquire next image
    uint32_t imageIndex =
        m_vulkanContext->acquireNextImage(m_syncObjects->getImageAvailableSemaphore());
    if (imageIndex == static_cast<uint32_t>(-1)) {
        // Swapchain needs to be recreated
        return;
    }

    // Update uniform buffer (time)
    updateUniformBuffer(imageIndex);

    // Record command buffer
    recordCommandBuffer(imageIndex);

    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_syncObjects->getImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;

    // Get command buffer for this frame
    auto cmdBuffer = m_commandBufferManager->allocateCommandBuffer();
    if (cmdBuffer) {
        VkCommandBuffer cmdBuffers[] = {cmdBuffer->getHandle()};
        submitInfo.pCommandBuffers = cmdBuffers;

        VkSemaphore signalSemaphores[] = {m_syncObjects->getRenderFinishedSemaphore()};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkFence fence = m_syncObjects->getInFlightFence();

        VkResult result = vkQueueSubmit(m_vulkanContext->getGraphicsQueue(), 1, &submitInfo, fence);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to submit command buffer: " << result << std::endl;
        }

        m_commandBufferManager->freeCommandBuffer(cmdBuffer);
    }

    // Present frame
    if (!m_vulkanContext->presentFrame(imageIndex, m_syncObjects->getRenderFinishedSemaphore())) {
        // Swapchain needs to be recreated
    }

    // End frame synchronization
    m_syncObjects->endFrame();
    m_syncObjects->nextFrame();
}

void TriangleRenderer::cleanup() {
    std::cout << "Cleaning up Triangle Renderer..." << std::endl;

    if (m_commandBufferManager) {
        m_commandBufferManager->waitForAllCommandBuffers();
    }

    // Destroy buffers
    if (m_vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_vulkanContext->getDevice(), m_vertexBuffer, nullptr);
        m_vertexBuffer = VK_NULL_HANDLE;
    }

    if (m_vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_vulkanContext->getDevice(), m_vertexBufferMemory, nullptr);
        m_vertexBufferMemory = VK_NULL_HANDLE;
    }

    if (m_indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_vulkanContext->getDevice(), m_indexBuffer, nullptr);
        m_indexBuffer = VK_NULL_HANDLE;
    }

    if (m_indexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_vulkanContext->getDevice(), m_indexBufferMemory, nullptr);
        m_indexBufferMemory = VK_NULL_HANDLE;
    }

    // Destroy framebuffers
    for (auto framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(m_vulkanContext->getDevice(), framebuffer, nullptr);
    }
    m_framebuffers.clear();

    // Destroy pipeline
    if (m_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_vulkanContext->getDevice(), m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    // Destroy pipeline layout
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_vulkanContext->getDevice(), m_pipelineLayout, nullptr);
        m_pipelineLayout = VK_NULL_HANDLE;
    }

    // Destroy render pass
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_vulkanContext->getDevice(), m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    // Destroy command pool
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_vulkanContext->getDevice(), m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }

    // Cleanup subsystems
    m_syncObjects.reset();
    m_commandBufferManager.reset();
    m_pipelineSystem.reset();
    m_shaderSystem.reset();
    m_bufferAllocator.reset();
    m_vulkanContext.reset();

    std::cout << "Triangle Renderer cleaned up" << std::endl;
}

bool TriangleRenderer::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    VkResult result =
        vkCreateRenderPass(m_vulkanContext->getDevice(), &renderPassInfo, nullptr, &m_renderPass);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create render pass: " << result << std::endl;
        return false;
    }

    std::cout << "Render pass created successfully" << std::endl;
    return true;
}

bool TriangleRenderer::createFramebuffers() {
    m_framebuffers.resize(m_swapchainImageCount);

    for (size_t i = 0; i < m_swapchainImageCount; i++) {
        VkImageView attachments[] = {m_vulkanContext->getSwapChainImageViews()[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtent.width;
        framebufferInfo.height = m_swapchainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(m_vulkanContext->getDevice(), &framebufferInfo,
                                              nullptr, &m_framebuffers[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to create framebuffer: " << result << std::endl;
            return false;
        }
    }

    std::cout << "Created " << m_framebuffers.size() << " framebuffers" << std::endl;
    return true;
}

bool TriangleRenderer::createPipeline() {
    std::cout << "Starting pipeline creation..." << std::endl;
    
    // Create pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 0;
    layoutInfo.pushConstantRangeCount = 0;

    std::cout << "Creating pipeline layout..." << std::endl;
    VkResult result = vkCreatePipelineLayout(m_vulkanContext->getDevice(), &layoutInfo, nullptr,
                                             &m_pipelineLayout);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout: " << result << std::endl;
        return false;
    }
    std::cout << "Pipeline layout created successfully" << std::endl;

    // Get shader modules
    std::cout << "Getting shader modules..." << std::endl;
    VkShaderModule vertexShader = m_shaderSystem->getVertexShader("triangle");
    VkShaderModule fragmentShader = m_shaderSystem->getFragmentShader("triangle");
    
    std::cout << "Vertex shader: " << vertexShader << std::endl;
    std::cout << "Fragment shader: " << fragmentShader << std::endl;
    
    if (vertexShader == VK_NULL_HANDLE || fragmentShader == VK_NULL_HANDLE) {
        std::cerr << "Error: Invalid shader modules!" << std::endl;
        return false;
    }

    // Create shader stages
    std::cout << "Creating shader stages..." << std::endl;
    VkPipelineShaderStageCreateInfo vertexShaderStage{};
    vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStage.module = vertexShader;
    vertexShaderStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentShaderStage{};
    fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage.module = fragmentShader;
    fragmentShaderStage.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStage, fragmentShaderStage};

    // Vertex input state - empty for now
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pViewports = nullptr; // Will be set dynamically
    viewportState.pScissors = nullptr; // Will be set dynamically

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    // Multisampling state
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth stencil state - disabled
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    // Color blending state
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Dynamic state
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Graphics pipeline create info
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;

    // Create pipeline
    std::cout << "Creating pipeline directly with Vulkan API..." << std::endl;
    std::cout << "Device: " << m_vulkanContext->getDevice() << std::endl;
    std::cout << "Render pass: " << m_renderPass << std::endl;
    std::cout << "Pipeline layout: " << m_pipelineLayout << std::endl;
    
    // Try to create the pipeline with error checking
    std::cout << "Calling vkCreateGraphicsPipelines..." << std::endl;
    
    // First, let's try to validate the pipeline info
    std::cout << "Validating pipeline create info..." << std::endl;
    std::cout << "sType: " << pipelineInfo.sType << std::endl;
    std::cout << "stageCount: " << pipelineInfo.stageCount << std::endl;
    std::cout << "layout: " << pipelineInfo.layout << std::endl;
    std::cout << "renderPass: " << pipelineInfo.renderPass << std::endl;
    
    // Try to create the pipeline
    result = vkCreateGraphicsPipelines(m_vulkanContext->getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    std::cout << "vkCreateGraphicsPipelines returned" << std::endl;
    
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline: " << result << std::endl;
        return false;
    }
    
    std::cout << "Pipeline created successfully with handle: " << m_pipeline << std::endl;
    return true;
}

bool TriangleRenderer::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Create staging buffer
    auto stagingAlloc = m_bufferAllocator->allocateBuffer(
        VortexEngine::BufferAllocator::BufferType::Staging,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    
    if (stagingAlloc.buffer == VK_NULL_HANDLE) {
        return false;
    }
    
    stagingBuffer = stagingAlloc.buffer;
    stagingBufferMemory = stagingAlloc.memory;

    // Map and copy vertex data
    void* data;
    vkMapMemory(m_vulkanContext->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_vulkanContext->getDevice(), stagingBufferMemory);

    // Create vertex buffer
    // Create vertex buffer
    auto vertexAlloc = m_bufferAllocator->allocateBuffer(
        VortexEngine::BufferAllocator::BufferType::Vertex,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    if (vertexAlloc.buffer == VK_NULL_HANDLE) {
        vkDestroyBuffer(m_vulkanContext->getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_vulkanContext->getDevice(), stagingBufferMemory, nullptr);
        return false;
    }
    
    m_vertexBuffer = vertexAlloc.buffer;
    m_vertexBufferMemory = vertexAlloc.memory;

    // Copy staging buffer to vertex buffer using command buffer
    auto cmdBuffer = m_commandBufferManager->allocateCommandBuffer();
    if (cmdBuffer) {
        cmdBuffer->beginRecording();
        
        // Set up buffer copy region
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = bufferSize;
        
        // Record the copy command
        vkCmdCopyBuffer(cmdBuffer->getHandle(), stagingBuffer, m_vertexBuffer, 1, &copyRegion);
        
        cmdBuffer->endRecording();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        VkCommandBuffer cmdBuffers[] = {cmdBuffer->getHandle()};
        submitInfo.pCommandBuffers = cmdBuffers;

        vkQueueSubmit(m_vulkanContext->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_vulkanContext->getGraphicsQueue());

        m_commandBufferManager->freeCommandBuffer(cmdBuffer);
    }

    // Cleanup staging buffer
    m_bufferAllocator->deallocateBuffer(stagingAlloc);

    std::cout << "Vertex buffer created successfully" << std::endl;
    return true;
}

bool TriangleRenderer::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    // Create staging buffer for indices
    auto stagingAlloc2 = m_bufferAllocator->allocateBuffer(
        VortexEngine::BufferAllocator::BufferType::Staging,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    
    if (stagingAlloc2.buffer == VK_NULL_HANDLE) {
        return false;
    }
    
    stagingBuffer = stagingAlloc2.buffer;
    stagingBufferMemory = stagingAlloc2.memory;

    // Map and copy index data
    void* data;
    vkMapMemory(m_vulkanContext->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_vulkanContext->getDevice(), stagingBufferMemory);

    // Create index buffer
    // Create index buffer
    auto indexAlloc = m_bufferAllocator->allocateBuffer(
        VortexEngine::BufferAllocator::BufferType::Index,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    if (indexAlloc.buffer == VK_NULL_HANDLE) {
        vkDestroyBuffer(m_vulkanContext->getDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_vulkanContext->getDevice(), stagingBufferMemory, nullptr);
        return false;
    }
    
    m_indexBuffer = indexAlloc.buffer;
    m_indexBufferMemory = indexAlloc.memory;

    // Copy staging buffer to index buffer using command buffer
    auto cmdBuffer = m_commandBufferManager->allocateCommandBuffer();
    if (cmdBuffer) {
        cmdBuffer->beginRecording();
        
        // Set up buffer copy region
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = bufferSize;
        
        // Record the copy command
        vkCmdCopyBuffer(cmdBuffer->getHandle(), stagingBuffer, m_indexBuffer, 1, &copyRegion);
        
        cmdBuffer->endRecording();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        VkCommandBuffer cmdBuffers[] = {cmdBuffer->getHandle()};
        submitInfo.pCommandBuffers = cmdBuffers;

        vkQueueSubmit(m_vulkanContext->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_vulkanContext->getGraphicsQueue());

        m_commandBufferManager->freeCommandBuffer(cmdBuffer);
    }

    // Cleanup staging buffer
    m_bufferAllocator->deallocateBuffer(stagingAlloc2);

    std::cout << "Index buffer created successfully" << std::endl;
    return true;
}

bool TriangleRenderer::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_vulkanContext->getGraphicsQueueFamilyIndex();

    VkResult result =
        vkCreateCommandPool(m_vulkanContext->getDevice(), &poolInfo, nullptr, &m_commandPool);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create command pool: " << result << std::endl;
        return false;
    }

    std::cout << "Command pool created successfully" << std::endl;
    return true;
}

bool TriangleRenderer::createCommandBuffers() {
    // Command buffers are now managed by CommandBufferManager
    // We'll create them on demand during rendering
    return true;
}

void TriangleRenderer::recordCommandBuffer(uint32_t imageIndex) {
    auto cmdBuffer = m_commandBufferManager->allocateCommandBuffer();
    if (!cmdBuffer) {
        std::cerr << "Failed to allocate command buffer" << std::endl;
        return;
    }

    // Begin recording
    cmdBuffer->beginRecording();

    // Begin render pass
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    cmdBuffer->beginRenderPass(m_renderPass, m_framebuffers[imageIndex], m_swapchainExtent,
                               &clearColor, 1);

    // Bind pipeline
    cmdBuffer->bindPipeline(m_pipeline);

    // Bind vertex buffer
    cmdBuffer->bindVertexBuffers(m_vertexBuffer);

    // Bind index buffer
    cmdBuffer->bindIndexBuffer(m_indexBuffer);

    // Draw
    cmdBuffer->drawIndexed(static_cast<uint32_t>(indices.size()));

    // End render pass
    cmdBuffer->endRenderPass();

    // End recording
    cmdBuffer->endRecording();

    // Command buffer will be submitted and freed in render()
}

void TriangleRenderer::updateUniformBuffer(uint32_t currentImage) {
    // Update time uniform
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time =
        std::chrono::duration<float, std::chrono::seconds::period>(currentTime - m_startTime)
            .count();
    m_time = time;
}

int main() {
    // Create window system
    VortexEngine::Window windowSystem;
    if (!windowSystem.initialize("Vortex Engine - Triangle Demo", 800, 600)) {
        std::cerr << "Failed to initialize window system" << std::endl;
        return -1;
    }

    // Create renderer
    TriangleRenderer renderer(windowSystem.getSDLWindow());
    if (!renderer.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }

    // Main loop
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        renderer.render();
    }

    // Cleanup
    renderer.cleanup();
    windowSystem.shutdown();

    return 0;
}