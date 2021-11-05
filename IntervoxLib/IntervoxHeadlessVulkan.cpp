//
//  IntervoxHeadlessVulkan.cpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/2/21.
//

#include "IntervoxHeadlessVulkan.hpp"

#if DEBUG_RENDER
#define BUFFER_ELEMENTS 32

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#define LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "vulkanExample", __VA_ARGS__))
#else
#define LOG(...) printf(__VA_ARGS__)
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessageCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData)
{
    LOG("[VALIDATION]: %s - %s\n", pLayerPrefix, pMessage);
    return VK_FALSE;
}

#endif

#define VERTEX_BUFFER_BIND_ID 0

IntervoxHeadlessVulkan::IntervoxHeadlessVulkan() : VulkanExampleBase(ENABLE_VALIDATION)
{

#if GEARS
    title = "Vulkan gears";
    camera.type = Camera::CameraType::lookat;
    camera.setPosition(glm::vec3(0.0f, 2.5f, -16.0f));
    camera.setRotation(glm::vec3(-23.75f, 41.25f, 21.0f));
    camera.setPerspective(60.0f, (float)width / (float)height, 0.001f, 256.0f);
    timerSpeed *= 0.25f;
#endif
    
#if DEBUG_RENDER
    fWidth = 1024;
    fHeight = 1024;
    width = fWidth;
    height = fHeight;
#endif
}

#if DEBUG_RENDER
IntervoxHeadlessVulkan::~IntervoxHeadlessVulkan()
{
    vkDestroyBuffer(device, fVertexBuffer, nullptr);
    vkFreeMemory(device, fVertexMemory, nullptr);
    vkDestroyBuffer(device, fIndexBuffer, nullptr);
    vkFreeMemory(device, fIndexMemory, nullptr);
#if DEBUG_RENDER_DELETE
    vkDestroyImageView(device, colorAttachment.view, nullptr);
    vkDestroyImage(device, colorAttachment.image, nullptr);
    vkFreeMemory(device, colorAttachment.memory, nullptr);
    vkDestroyImageView(device, depthAttachment.view, nullptr);
    vkDestroyImage(device, depthAttachment.image, nullptr);
    vkFreeMemory(device, depthAttachment.memory, nullptr);
#endif
    vkDestroyRenderPass(device, fRenderPass, nullptr);
#if DEBUG_RENDER_DELETE
    vkDestroyFramebuffer(device, framebuffer, nullptr);
#endif
    vkDestroyPipelineLayout(device, fPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, fDescriptorSetLayout, nullptr);
    vkDestroyPipeline(device, fPipeline, nullptr);
    vkDestroyPipelineCache(device, fPipelineCache, nullptr);
    vkDestroyCommandPool(device, fCommandPool, nullptr);
    for (auto shadermodule : fShaderModules) {
        vkDestroyShaderModule(device, shadermodule, nullptr);
    }
    
#if DEBUG_RENDER_DELETE
    vkDestroyDevice(device, nullptr);
#if DEBUG
    if (debugReportCallback) {
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        assert(vkDestroyDebugReportCallback);
        vkDestroyDebugReportCallback(instance, debugReportCallback, nullptr);
    }
#endif
    vkDestroyInstance(instance, nullptr);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    vks::android::freeVulkanLibrary();
#endif
#endif
}
#else
IntervoxHeadlessVulkan::~IntervoxHeadlessVulkan()
{
#if GEARS
    // Clean up used Vulkan resources
    // Note : Inherited destructor cleans up resources stored in base class
    vkDestroyPipeline(device, pipelines.solid, nullptr);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    for (auto& gear : gears)
    {
        delete(gear);
    }
#endif
}
#endif

void IntervoxHeadlessVulkan::buildCommandBuffers()
{
#if GEARS
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VkClearColorValue clearColor = { { 1.0f, 0.025f, 0.025f, 1.0f } };
    VkClearValue clearValues[2];
 //   clearValues[0].color = defaultClearColor;
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

        for (auto& gear : gears)
        {
            gear->draw(drawCmdBuffers[i], pipelineLayout);
        }

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
#endif
}

void IntervoxHeadlessVulkan::prepareVertices()
{
#if GEARS
    // Gear definitions
    std::vector<float> innerRadiuses = { 1.0f, 0.5f, 1.3f };
    std::vector<float> outerRadiuses = { 4.0f, 2.0f, 2.0f };
    std::vector<float> widths = { 1.0f, 2.0f, 0.5f };
    std::vector<int32_t> toothCount = { 20, 10, 10 };
    std::vector<float> toothDepth = { 0.7f, 0.7f, 0.7f };
    std::vector<glm::vec3> colors = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.2f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    std::vector<glm::vec3> positions = {
        glm::vec3(-3.0, 0.0, 0.0),
        glm::vec3(3.1, 0.0, 0.0),
        glm::vec3(-3.1, -6.2, 0.0)
    };
    std::vector<float> rotationSpeeds = { 1.0f, -2.0f, -2.0f };
    std::vector<float> rotationOffsets = { 0.0f, -9.0f, -30.0f };

    gears.resize(positions.size());
    for (int32_t i = 0; i < gears.size(); ++i)
    {
        GearInfo gearInfo = {};
        gearInfo.innerRadius = innerRadiuses[i];
        gearInfo.outerRadius = outerRadiuses[i];
        gearInfo.width = widths[i];
        gearInfo.numTeeth = toothCount[i];
        gearInfo.toothDepth = toothDepth[i];
        gearInfo.color = colors[i];
        gearInfo.pos = positions[i];
        gearInfo.rotSpeed = rotationSpeeds[i];
        gearInfo.rotOffset = rotationOffsets[i];

        gears[i] = new VulkanGear(vulkanDevice);
        gears[i]->generate(&gearInfo, queue);
    }

    // Binding and attribute descriptions are shared across all gears
    vertices.bindingDescriptions.resize(1);
    vertices.bindingDescriptions[0] =
        vks::initializers::vertexInputBindingDescription(
            VERTEX_BUFFER_BIND_ID,
            sizeof(Vertex),
            VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // Describes memory layout and shader positions
    vertices.attributeDescriptions.resize(3);
    // Location 0 : Position
    vertices.attributeDescriptions[0] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0);
    // Location 1 : Normal
    vertices.attributeDescriptions[1] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            sizeof(float) * 3);
    // Location 2 : Color
    vertices.attributeDescriptions[2] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            2,
            VK_FORMAT_R32G32B32_SFLOAT,
            sizeof(float) * 6);

    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
#endif
}

void IntervoxHeadlessVulkan::setupDescriptorPool()
{
#if GEARS
    // One UBO for each gear
    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3),
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        vks::initializers::descriptorPoolCreateInfo(
            static_cast<uint32_t>(poolSizes.size()),
            poolSizes.data(),
            // Three descriptor sets (for each gear)
            3);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
#endif
}

void IntervoxHeadlessVulkan::setupDescriptorSetLayout()
{
#if GEARS
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
    {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
#endif
}

void IntervoxHeadlessVulkan::setupDescriptorSets()
{
#if GEARS
    for (auto& gear : gears)
    {
        gear->setupDescriptorSet(descriptorPool, descriptorSetLayout);
    }
#endif
}

void IntervoxHeadlessVulkan::preparePipelines()
{
#if GEARS
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        vks::initializers::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            0,
            VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
        vks::initializers::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
            0);

    VkPipelineColorBlendAttachmentState blendAttachmentState =
        vks::initializers::pipelineColorBlendAttachmentState(
            0xf,
            VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlendState =
        vks::initializers::pipelineColorBlendStateCreateInfo(
            1,
            &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        vks::initializers::pipelineDepthStencilStateCreateInfo(
            VK_TRUE,
            VK_TRUE,
            VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
        vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
        vks::initializers::pipelineMultisampleStateCreateInfo(
            VK_SAMPLE_COUNT_1_BIT,
            0);

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
        vks::initializers::pipelineDynamicStateCreateInfo(
            dynamicStateEnables.data(),
            static_cast<uint32_t>(dynamicStateEnables.size()),
            0);

    // Solid rendering pipeline
    // Load shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    shaderStages[0] = loadShader(getShadersPath() + "gears/gears.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getShadersPath() + "gears/gears.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
        vks::initializers::pipelineCreateInfo(
            pipelineLayout,
            renderPass,
            0);

    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, fPipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid));
#endif
}

void IntervoxHeadlessVulkan::updateUniformBuffers()
{
#if GEARS
    for (auto& gear : gears)
    {
        gear->updateUniformBuffer(camera.matrices.perspective, camera.matrices.view, timer * 360.0f);
    }
#endif
}

void IntervoxHeadlessVulkan::draw()
{
#if GEARS
#ifndef INTERVOX_LIB
    VulkanExampleBase::prepareFrame();
#endif

    // Command buffer to be submitted to the queue
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

#ifdef INTERVOX_LIB
    VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo();
    VkFence fence;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &fence));
#endif
    // Submit to queue
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

#ifndef INTERVOX_LIB
   VulkanExampleBase::submitFrame();
#endif
    
#ifdef INTERVOX_LIB
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
    vkDestroyFence(device, fence, nullptr);
#endif

#endif
}

void IntervoxHeadlessVulkan::prepare()
{
    VulkanExampleBase::prepare();
    prepareVertices();
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSets();
    updateUniformBuffers();
    buildCommandBuffers();
    prepared = true;
}

#if DEBUG_RENDER
void IntervoxHeadlessVulkan::render()
{
#if DEBUG_RENDER_DELETE
        LOG("Running headless rendering example\n");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        LOG("loading vulkan lib");
        vks::android::loadVulkanLibrary();
#endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan headless example";
        appInfo.pEngineName = "VulkanExample";
        appInfo.apiVersion = VK_API_VERSION_1_0;

        /*
            Vulkan instance creation (without surface extensions)
        */
        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        uint32_t layerCount = 0;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        const char* validationLayers[] = { "VK_LAYER_GOOGLE_threading",    "VK_LAYER_LUNARG_parameter_validation",    "VK_LAYER_LUNARG_object_tracker","VK_LAYER_LUNARG_core_validation",    "VK_LAYER_LUNARG_swapchain", "VK_LAYER_GOOGLE_unique_objects" };
        layerCount = 6;
#else
        const char* validationLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
        layerCount = 1;
#endif
#if DEBUG
        // Check if layers are available
        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<VkLayerProperties> instanceLayers(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data());

        bool layersAvailable = true;
        for (auto layerName : validationLayers) {
            bool layerAvailable = false;
            for (auto instanceLayer : instanceLayers) {
                if (strcmp(instanceLayer.layerName, layerName) == 0) {
                    layerAvailable = true;
                    break;
                }
            }
            if (!layerAvailable) {
                layersAvailable = false;
                break;
            }
        }

        const char *validationExt = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
        if (layersAvailable) {
            instanceCreateInfo.ppEnabledLayerNames = validationLayers;
            instanceCreateInfo.enabledLayerCount = layerCount;
            instanceCreateInfo.enabledExtensionCount = 1;
            instanceCreateInfo.ppEnabledExtensionNames = &validationExt;
        }
#endif
        VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        vks::android::loadVulkanFunctions(instance);
#endif
#if DEBUG
        if (layersAvailable) {
            VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = {};
            debugReportCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugReportCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugMessageCallback;

            // We have to explicitly load this function.
            PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
            assert(vkCreateDebugReportCallbackEXT);
            VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(instance, &debugReportCreateInfo, nullptr, &debugReportCallback));
        }
#endif

        /*
            Vulkan device creation
        */
        uint32_t deviceCount = 0;
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));
        physicalDevice = physicalDevices[0];

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        LOG("GPU: %s\n", deviceProperties.deviceName);

        // Request a single graphics queue
        const float defaultQueuePriority(0.0f);
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
        for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndex = i;
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = i;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
                break;
            }
        }
        // Create logical device
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));
#endif
        // Get a graphics queue
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &fQueue);

        // Command pool
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK_RESULT(vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &fCommandPool));

        /*
            Prepare vertex and index buffers
        */
        struct Vertex {
            float position[3];
            float color[3];
        };
        {
            std::vector<Vertex> vertices = {
                { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
                { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
                { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
            };
            std::vector<uint32_t> indices = { 0, 1, 2 };

            const VkDeviceSize vertexBufferSize = vertices.size() * sizeof(Vertex);
            const VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);

            VkBuffer stagingBuffer;
            VkDeviceMemory stagingMemory;

            // Command buffer for copy commands (reused)
            VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
            VkCommandBuffer copyCmd;
            VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &copyCmd));
            VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

            // Copy input data to VRAM using a staging buffer
            {
                // Vertices
                createBuffer(
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &stagingBuffer,
                    &stagingMemory,
                    vertexBufferSize,
                    vertices.data());

                createBuffer(
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    &fVertexBuffer,
                    &fVertexMemory,
                    vertexBufferSize);

                VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
                VkBufferCopy copyRegion = {};
                copyRegion.size = vertexBufferSize;
                vkCmdCopyBuffer(copyCmd, stagingBuffer, fVertexBuffer, 1, &copyRegion);
                VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

                submitWork(copyCmd, fQueue);

                vkDestroyBuffer(device, stagingBuffer, nullptr);
                vkFreeMemory(device, stagingMemory, nullptr);

                // Indices
                createBuffer(
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    &stagingBuffer,
                    &stagingMemory,
                    indexBufferSize,
                    indices.data());

                createBuffer(
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    &fIndexBuffer,
                    &fIndexMemory,
                    indexBufferSize);

                VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
                copyRegion.size = indexBufferSize;
                vkCmdCopyBuffer(copyCmd, stagingBuffer, fIndexBuffer, 1, &copyRegion);
                VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

                submitWork(copyCmd, fQueue);

                vkDestroyBuffer(device, stagingBuffer, nullptr);
                vkFreeMemory(device, stagingMemory, nullptr);
            }
        }

        /*
            Create framebuffer attachments
        */
//#if DEBUG_RENDER_DELETE
    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
        VkFormat depthFormat;
        vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);
        {

            // Color attachment
            VkImageCreateInfo image = vks::initializers::imageCreateInfo();
            image.imageType = VK_IMAGE_TYPE_2D;
            image.format = colorFormat;
            image.extent.width = fWidth;
            image.extent.height = fHeight;
            image.extent.depth = 1;
            image.mipLevels = 1;
            image.arrayLayers = 1;
            image.samples = VK_SAMPLE_COUNT_1_BIT;
            image.tiling = VK_IMAGE_TILING_OPTIMAL;
            image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

            VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
            VkMemoryRequirements memReqs;

            VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &colorAttachment.image));
            vkGetImageMemoryRequirements(device, colorAttachment.image, &memReqs);
            memAlloc.allocationSize = memReqs.size;
            memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &colorAttachment.memory));
            VK_CHECK_RESULT(vkBindImageMemory(device, colorAttachment.image, colorAttachment.memory, 0));

            VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
            colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            colorImageView.format = colorFormat;
            colorImageView.subresourceRange = {};
            colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            colorImageView.subresourceRange.baseMipLevel = 0;
            colorImageView.subresourceRange.levelCount = 1;
            colorImageView.subresourceRange.baseArrayLayer = 0;
            colorImageView.subresourceRange.layerCount = 1;
            colorImageView.image = colorAttachment.image;
            VK_CHECK_RESULT(vkCreateImageView(device, &colorImageView, nullptr, &colorAttachment.view));

            // Depth stencil attachment
            image.format = depthFormat;
            image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            VK_CHECK_RESULT(vkCreateImage(device, &image, nullptr, &depthAttachment.image));
            vkGetImageMemoryRequirements(device, depthAttachment.image, &memReqs);
            memAlloc.allocationSize = memReqs.size;
            memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &depthAttachment.memory));
            VK_CHECK_RESULT(vkBindImageMemory(device, depthAttachment.image, depthAttachment.memory, 0));

            VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
            depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            depthStencilView.format = depthFormat;
            depthStencilView.flags = 0;
            depthStencilView.subresourceRange = {};
            depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            depthStencilView.subresourceRange.baseMipLevel = 0;
            depthStencilView.subresourceRange.levelCount = 1;
            depthStencilView.subresourceRange.baseArrayLayer = 0;
            depthStencilView.subresourceRange.layerCount = 1;
            depthStencilView.image = depthAttachment.image;
            VK_CHECK_RESULT(vkCreateImageView(device, &depthStencilView, nullptr, &depthAttachment.view));
        }

        /*
            Create renderpass
        */
        {
            std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
            // Color attachment
            attchmentDescriptions[0].format = colorFormat;
            attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
            attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            // Depth attachment
            attchmentDescriptions[1].format = depthFormat;
            attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
            VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments = &colorReference;
            subpassDescription.pDepthStencilAttachment = &depthReference;

            // Use subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies;

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            // Create the actual renderpass
            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
            renderPassInfo.pAttachments = attchmentDescriptions.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();
            VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &fRenderPass));

            VkImageView attachments[2];
            attachments[0] = colorAttachment.view;
            attachments[1] = depthAttachment.view;

            VkFramebufferCreateInfo framebufferCreateInfo = vks::initializers::framebufferCreateInfo();
            framebufferCreateInfo.renderPass = fRenderPass;
            framebufferCreateInfo.attachmentCount = 2;
            framebufferCreateInfo.pAttachments = attachments;
            framebufferCreateInfo.width = fWidth;
            framebufferCreateInfo.height = fHeight;
            framebufferCreateInfo.layers = 1;
            VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer));
        }
//#endif
//#if  DEBUG_RENDER_ADD
//    setupImageViews();
//    setupFrameBuffer();
//#endif
        /*
            Prepare graphics pipeline
        */
        {
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
            VkDescriptorSetLayoutCreateInfo descriptorLayout =
                vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
            VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &fDescriptorSetLayout));

            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
                vks::initializers::pipelineLayoutCreateInfo(nullptr, 0);

            // MVP via push constant block
            VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
            pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
            pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

            VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &fPipelineLayout));

            VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
            pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
            VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &fPipelineCache));

            // Create pipeline
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
                vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

            VkPipelineRasterizationStateCreateInfo rasterizationState =
                vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

            VkPipelineColorBlendAttachmentState blendAttachmentState =
                vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

            VkPipelineColorBlendStateCreateInfo colorBlendState =
                vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

            VkPipelineDepthStencilStateCreateInfo depthStencilState =
                vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

            VkPipelineViewportStateCreateInfo viewportState =
                vks::initializers::pipelineViewportStateCreateInfo(1, 1);

            VkPipelineMultisampleStateCreateInfo multisampleState =
                vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

            std::vector<VkDynamicState> dynamicStateEnables = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            VkPipelineDynamicStateCreateInfo dynamicState =
                vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

            VkGraphicsPipelineCreateInfo pipelineCreateInfo =
                vks::initializers::pipelineCreateInfo(fPipelineLayout, fRenderPass);

            std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

            pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
            pipelineCreateInfo.pRasterizationState = &rasterizationState;
            pipelineCreateInfo.pColorBlendState = &colorBlendState;
            pipelineCreateInfo.pMultisampleState = &multisampleState;
            pipelineCreateInfo.pViewportState = &viewportState;
            pipelineCreateInfo.pDepthStencilState = &depthStencilState;
            pipelineCreateInfo.pDynamicState = &dynamicState;
            pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
            pipelineCreateInfo.pStages = shaderStages.data();

            // Vertex bindings an attributes
            // Binding description
            std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
                vks::initializers::vertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX),
            };

            // Attribute descriptions
            std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
                vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),                    // Position
                vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),    // Color
            };

            VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
            vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
            vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
            vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

            pipelineCreateInfo.pVertexInputState = &vertexInputState;

            // TODO: There is no command line arguments parsing (nor Android settings) for this
            // example, so we have no way of picking between GLSL or HLSL shaders.
            // Hard-code to glsl for now.
            const std::string shadersPath = getAssetPath() + "shaders/glsl/renderheadless/";

            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].pName = "main";
            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].pName = "main";
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
            shaderStages[0].module = vks::tools::loadShader(androidapp->activity->assetManager, (shadersPath + "triangle.vert.spv").c_str(), device);
            shaderStages[1].module = vks::tools::loadShader(androidapp->activity->assetManager, (shadersPath + "triangle.frag.spv").c_str(), device);
#else
            shaderStages[0].module = vks::tools::loadShader((shadersPath + "triangle.vert.spv").c_str(), device);
            shaderStages[1].module = vks::tools::loadShader((shadersPath + "triangle.frag.spv").c_str(), device);
#endif
            shaderModules = { shaderStages[0].module, shaderStages[1].module };
            VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, fPipelineCache, 1, &pipelineCreateInfo, nullptr, &fPipeline));
        }

        /*
            Command buffer creation
        */
        {
            VkCommandBufferAllocateInfo cmdBufAllocateInfo =
                vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
            VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &fCommandBuffer));

            VkCommandBufferBeginInfo cmdBufInfo =
                vks::initializers::commandBufferBeginInfo();

            VK_CHECK_RESULT(vkBeginCommandBuffer(fCommandBuffer, &cmdBufInfo));

            VkClearValue clearValues[2];
            clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
            clearValues[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderArea.extent.width = fWidth;
            renderPassBeginInfo.renderArea.extent.height = fHeight;
            renderPassBeginInfo.clearValueCount = 2;
            renderPassBeginInfo.pClearValues = clearValues;
            renderPassBeginInfo.renderPass = fRenderPass;
//#if DEBUG_RENDER_DELETE
            renderPassBeginInfo.framebuffer = framebuffer;
//#endif
//#if DEBUG_RENDER_ADD
//            renderPassBeginInfo.framebuffer = frameBuffers[0];
//#endif
            
            vkCmdBeginRenderPass(fCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport = {};
            viewport.height = (float)fHeight;
            viewport.width = (float)fWidth;
            viewport.minDepth = (float)0.0f;
            viewport.maxDepth = (float)1.0f;
            vkCmdSetViewport(fCommandBuffer, 0, 1, &viewport);

            // Update dynamic scissor state
            VkRect2D scissor = {};
            scissor.extent.width = fWidth;
            scissor.extent.height = fHeight;
            vkCmdSetScissor(fCommandBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(fCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fPipeline);

            // Render scene
            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(fCommandBuffer, 0, 1, &fVertexBuffer, offsets);
            vkCmdBindIndexBuffer(fCommandBuffer, fIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            std::vector<glm::vec3> pos = {
                glm::vec3(-1.5f, 0.0f, -4.0f),
                glm::vec3( 0.0f, 0.0f, -2.5f),
                glm::vec3( 1.5f, 0.0f, -4.0f),
            };

            for (auto v : pos) {
                glm::mat4 mvpMatrix = glm::perspective(glm::radians(60.0f), (float)fWidth / (float)fHeight, 0.1f, 256.0f) * glm::translate(glm::mat4(1.0f), v);
                vkCmdPushConstants(fCommandBuffer, fPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvpMatrix), &mvpMatrix);
                vkCmdDrawIndexed(fCommandBuffer, 3, 1, 0, 0, 0);
            }

            vkCmdEndRenderPass(fCommandBuffer);

            VK_CHECK_RESULT(vkEndCommandBuffer(fCommandBuffer));

            submitWork(fCommandBuffer, fQueue);

            vkDeviceWaitIdle(device);
        }

        /*
            Copy framebuffer image to host visible image
        */
        const char* imagedata;
        {
            // Create the linear tiled destination image to copy to and to read the memory from
            VkImageCreateInfo imgCreateInfo(vks::initializers::imageCreateInfo());
            imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
            imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
            imgCreateInfo.extent.width = fWidth;
            imgCreateInfo.extent.height = fHeight;
            imgCreateInfo.extent.depth = 1;
            imgCreateInfo.arrayLayers = 1;
            imgCreateInfo.mipLevels = 1;
            imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
            imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            // Create the image
            VkImage dstImage;
            VK_CHECK_RESULT(vkCreateImage(device, &imgCreateInfo, nullptr, &dstImage));
            // Create memory to back up the image
            VkMemoryRequirements memRequirements;
            VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
            VkDeviceMemory dstImageMemory;
            vkGetImageMemoryRequirements(device, dstImage, &memRequirements);
            memAllocInfo.allocationSize = memRequirements.size;
            // Memory must be host visible to copy from
            memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory));
            VK_CHECK_RESULT(vkBindImageMemory(device, dstImage, dstImageMemory, 0));

            // Do the actual blit from the offscreen image to our host visible destination image
            VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
            VkCommandBuffer copyCmd;
            VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &copyCmd));
            VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
            VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));

            // Transition destination image to transfer destination layout
            vks::tools::insertImageMemoryBarrier(
                copyCmd,
                dstImage,
                0,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

            // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

            VkImageCopy imageCopyRegion{};
            imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.srcSubresource.layerCount = 1;
            imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopyRegion.dstSubresource.layerCount = 1;
            imageCopyRegion.extent.width = fWidth;
            imageCopyRegion.extent.height = fHeight;
            imageCopyRegion.extent.depth = 1;

            vkCmdCopyImage(
                copyCmd,
//#if DEBUG_RENDER_DELETE
                colorAttachment.image,
//#else
//                colorAttachments[0].image,
//#endif
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageCopyRegion);

            // Transition destination image to general layout, which is the required layout for mapping the image memory later on
            vks::tools::insertImageMemoryBarrier(
                copyCmd,
                dstImage,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

            VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

            submitWork(copyCmd, fQueue);

            // Get layout of the image (including row pitch)
            VkImageSubresource subResource{};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            VkSubresourceLayout subResourceLayout;

            vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

            // Map image memory so we can start copying from it
            vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
            imagedata += subResourceLayout.offset;

        /*
            Save host visible framebuffer image to disk (ppm format)
        */

#if defined (VK_USE_PLATFORM_ANDROID_KHR)
            const char* filename = strcat(getenv("EXTERNAL_STORAGE"), "/headless.ppm");
#else
            const char* filename = "headless.ppm";
#endif
            std::ofstream file(filename, std::ios::out | std::ios::binary);

            // ppm header
            file << "P6\n" << fWidth << "\n" << height << "\n" << 255 << "\n";

            // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
            // Check if source is BGR and needs swizzle
            std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
            const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

            // ppm binary pixel data
            for (int32_t y = 0; y < fHeight; y++) {
                unsigned int *row = (unsigned int*)imagedata;
                for (int32_t x = 0; x < fWidth; x++) {
                    if (colorSwizzle) {
                        file.write((char*)row + 2, 1);
                        file.write((char*)row + 1, 1);
                        file.write((char*)row, 1);
                    }
                    else {
                        file.write((char*)row, 3);
                    }
                    row++;
                }
                imagedata += subResourceLayout.rowPitch;
            }
            file.close();

            LOG("Framebuffer image saved to %s\n", filename);

            // Clean up resources
            vkUnmapMemory(device, dstImageMemory);

            vkFreeMemory(device, dstImageMemory, nullptr);
            vkDestroyImage(device, dstImage, nullptr);
        }

        vkQueueWaitIdle(fQueue);
}

#else
void IntervoxHeadlessVulkan::render()
{
    if (!prepared)
        return;
    vkDeviceWaitIdle(device);
    draw();
    vkDeviceWaitIdle(device);
    if (!paused)
    {
        updateUniformBuffers();
    }
}
#endif

void IntervoxHeadlessVulkan::viewChanged()
{
    updateUniformBuffers();
}

void IntervoxHeadlessVulkan::grabImage()
{
    bool supportsBlit = true;

    // Check blit support for source and destination
    VkFormatProperties formatProps;

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    vkGetPhysicalDeviceFormatProperties(physicalDevice, getColorFormat(), &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Source for the copy is the last rendered swapchain image
    VkImage srcImage = getImageAtIndex(currentBuffer);

    // Create the linear tiled destination image to copy to and to read the memory from
    VkImageCreateInfo imageCreateCI(vks::initializers::imageCreateInfo());
    imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
    // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
    imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateCI.extent.width = fWidth;
    imageCreateCI.extent.height = fHeight;
    imageCreateCI.extent.depth = 1;
    imageCreateCI.arrayLayers = 1;
    imageCreateCI.mipLevels = 1;
    imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // Create the image
    VkImage dstImage;
    VK_CHECK_RESULT(vkCreateImage(device, &imageCreateCI, nullptr, &dstImage));
    // Create memory to back up the image
    VkMemoryRequirements memRequirements;
    VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
    VkDeviceMemory dstImageMemory;
    vkGetImageMemoryRequirements(device, dstImage, &memRequirements);
    memAllocInfo.allocationSize = memRequirements.size;
    // Memory must be host visible to copy from
    memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, dstImage, dstImageMemory, 0));

    // Do the actual blit from the swapchain image to our host visible destination image
    VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Transition destination image to transfer destination layout
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        dstImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // Transition swapchain image from present to transfer source layout
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        srcImage,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
    if (supportsBlit)
    {
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = fWidth;
        blitSize.y = fHeight;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        // Issue the blit command
        vkCmdBlitImage(
            copyCmd,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            VK_FILTER_NEAREST);
    }
    else
    {
        // Otherwise use image copy (requires us to manually flip components)
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = fWidth;
        imageCopyRegion.extent.height = fHeight;
        imageCopyRegion.extent.depth = 1;

        // Issue the copy command
        vkCmdCopyImage(
            copyCmd,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        dstImage,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // Transition back the swap chain image after the blit is done
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        srcImage,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    vulkanDevice->flushCommandBuffer(copyCmd, queue);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* data;
    vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
    data += subResourceLayout.offset;

    std::ofstream file("saved_intevox.ppm", std::ios::out | std::ios::binary);

    // ppm header
    file << "P6\n" << fWidth << "\n" << fHeight << "\n" << 255 << "\n";

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    bool colorSwizzle = false;
    // Check if source is BGR
    // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
    if (!supportsBlit)
    {
        std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
        colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), getColorFormat()) != formatsBGR.end());
    }

    // ppm binary pixel data
    for (uint32_t y = 0; y < fHeight; y++)
    {
        unsigned int *row = (unsigned int*)data;
        for (uint32_t x = 0; x < fWidth; x++)
        {
            if (colorSwizzle)
            {
                file.write((char*)row+2, 1);
                file.write((char*)row+1, 1);
                file.write((char*)row, 1);
            }
            else
            {
                file.write((char*)row, 3);
            }
            row++;
        }
        data += subResourceLayout.rowPitch;
    }
    file.close();

    std::cout << "Screenshot saved to disk" << std::endl;

    // Clean up resources
    vkUnmapMemory(device, dstImageMemory);
    vkFreeMemory(device, dstImageMemory, nullptr);
    vkDestroyImage(device, dstImage, nullptr);

}



