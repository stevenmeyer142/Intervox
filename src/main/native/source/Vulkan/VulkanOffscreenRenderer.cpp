//
//  VulkanOffscreenRenderer.cpp
//  Intervox
//
//  Created by Steven Meyer on 2/9/20.
//  Copyright © 2020 Steven Meyer. All rights reserved.
//

#include "NativeOpenGL.h"
#include "VulkanOffscreenRenderer.hpp"
#if defined(_WIN32)
#pragma comment(linker, "/subsystem:console")
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <android/log.h>
#include "VulkanAndroid.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <array>
#include <iostream>
#include <algorithm>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanTools.h"

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
android_app* androidapp;
#endif

//#define DEBUG (!NDEBUG)

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

VulkanOffscreenRenderer::VulkanOffscreenRenderer() :
fInstance(NULL)
, fPhysicalDevice(NULL)
, fDevice(NULL)
, fQueueFamilyIndex(0)
, fPipelineCache(NULL)
, fQueue(NULL)
, fCommandPool(NULL)
, fDescriptorSetLayout(NULL)
, fPipelineLayout(NULL)
, fPipeline(NULL)
, fVertexBuffer(NULL)
, fIndexBuffer(NULL)
, fVertexMemory(NULL)
, fIndexMemory(NULL)
, fWidth(0)
, fHeight(0)
, fFramebuffer(NULL)
, fRenderPass(NULL)
{
    memset(&fColorAttachment, 0, sizeof(fColorAttachment));
    memset(&fDepthAttachment, 0, sizeof(fDepthAttachment));
}


void VulkanOffscreenRenderer::initialize(int width, int height)
{
    initializeDevice();
    initializeCommandPool();
    std::vector<Vertex> vertices = {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };
    std::vector<uint32_t> indices = { 0, 1, 2 };

    setIndexVertexBuffers(vertices, indices);
    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    initializeFrameBuffer(width, height, colorFormat, &depthFormat);
    createRenderPass(colorFormat, depthFormat);
    prepareGraphicsPipeline();
}



void VulkanOffscreenRenderer::initializeDevice()
{
    LOG("InitializeDevice\n");
    
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
    
    if (layersAvailable) {
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
        const char *validationExt = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
        instanceCreateInfo.enabledLayerCount = layerCount;
        instanceCreateInfo.enabledExtensionCount = 1;
        instanceCreateInfo.ppEnabledExtensionNames = &validationExt;
    }
#endif
    VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &fInstance));
    
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
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(fInstance, "vkCreateDebugReportCallbackEXT"));
        assert(vkCreateDebugReportCallbackEXT);
        VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(fInstance, &debugReportCreateInfo, nullptr, &debugReportCallback));
    }
#endif
    
    /*
     Vulkan device creation
     */
    uint32_t deviceCount = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(fInstance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(fInstance, &deviceCount, physicalDevices.data()));
    fPhysicalDevice = physicalDevices[0];
    
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(fPhysicalDevice, &deviceProperties);
    LOG("GPU: %s\n", deviceProperties.deviceName);
    
    // Request a single graphics queue
    const float defaultQueuePriority(0.0f);
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(fPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(fPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            fQueueFamilyIndex = i;
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
    VK_CHECK_RESULT(vkCreateDevice(fPhysicalDevice, &deviceCreateInfo, nullptr, &fDevice));
    
    // Get a graphics queue
    vkGetDeviceQueue(fDevice, fQueueFamilyIndex, 0, &fQueue);
    
}

void VulkanOffscreenRenderer::setIndexVertexBuffers(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    LOG("setIndexVertexBuffers\n");
    
    cleanupVertexIndexBuffers();
    
    /*
     Prepare vertex and index buffers
     */
    const VkDeviceSize vertexBufferSize = vertices.size() * sizeof(Vertex);
    const VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    
    // Command buffer for copy commands (reused)
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    VkCommandBuffer copyCmd;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(fDevice, &cmdBufAllocateInfo, &copyCmd));
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    
    // Copy input data to VRAM using a staging buffer
    
    // mVertices
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
    
    vkDestroyBuffer(fDevice, stagingBuffer, nullptr);
    vkFreeMemory(fDevice, stagingMemory, nullptr);
    
    // mIndices
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
    
    vkDestroyBuffer(fDevice, stagingBuffer, nullptr);
    vkFreeMemory(fDevice, stagingMemory, nullptr);
}

void VulkanOffscreenRenderer::renderScene()
{
    LOG("renderScene\n");
    
    /*
     Command buffer creation
     */
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo cmdBufAllocateInfo =
    vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    VK_CHECK_RESULT(vkAllocateCommandBuffers(fDevice, &cmdBufAllocateInfo, &commandBuffer));
    
    VkCommandBufferBeginInfo cmdBufInfo =
    vks::initializers::commandBufferBeginInfo();
    
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
    
    VkClearValue clearValues[2];
    clearValues[0].color = { { 1.0f, 1.0f, 1.0f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };
    
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderArea.extent.width = fWidth;
    renderPassBeginInfo.renderArea.extent.height = fHeight;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.renderPass = fRenderPass;
    renderPassBeginInfo.framebuffer = fFramebuffer;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport viewport = {};
    viewport.height = (float)fHeight;
    viewport.width = (float)fWidth;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    // Update dynamic scissor state
    VkRect2D scissor = {};
    scissor.extent.width = fWidth;
    scissor.extent.height = fHeight;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fPipeline);
    
    if (0 != fVertexBuffer && 0 != fIndexBuffer)
    {
    	// Render scene
    	VkDeviceSize offsets[1] = { 0 };
    	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &fVertexBuffer, offsets);
    	vkCmdBindIndexBuffer(commandBuffer, fIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    	std::vector<glm::vec3> pos = {
    			glm::vec3(-1.5f, 0.0f, -4.0f),
				glm::vec3( 0.0f, 0.0f, -2.5f),
				glm::vec3( 1.5f, 0.0f, -4.0f),
    	};

    	for (auto v : pos) {
    		glm::mat4 mvpMatrix = glm::perspective(glm::radians(60.0f), (float)fWidth / (float)fHeight, 0.1f, 256.0f) * glm::translate(glm::mat4(1.0f), v);
    		vkCmdPushConstants(commandBuffer, fPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvpMatrix), &mvpMatrix);
    		vkCmdDrawIndexed(commandBuffer, 3, 1, 0, 0, 0);
    	}
    }
    
    vkCmdEndRenderPass(commandBuffer);
    
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
    
    submitWork(commandBuffer, fQueue);
    
    vkDeviceWaitIdle(fDevice);
}

void VulkanOffscreenRenderer::copyImageData_RGBA_8888(uint32_t* buffer, int width, int height)
{
    if (width != fWidth || height != fHeight)
    {
        LOG("VulkanOffscreenRenderer::copyImageData_RGBA_8888 width != fWidth || height != fHeight");
        return;
    }
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
        VK_CHECK_RESULT(vkCreateImage(fDevice, &imgCreateInfo, nullptr, &dstImage));
        // Create memory to back up the image
        VkMemoryRequirements memRequirements;
        VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
        VkDeviceMemory dstImageMemory;
        vkGetImageMemoryRequirements(fDevice, dstImage, &memRequirements);
        memAllocInfo.allocationSize = memRequirements.size;
        // Memory must be host visible to copy from
        memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAllocInfo, nullptr, &dstImageMemory));
        VK_CHECK_RESULT(vkBindImageMemory(fDevice, dstImage, dstImageMemory, 0));
        
        // Do the actual blit from the offscreen image to our host visible destination image
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        VkCommandBuffer copyCmd;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(fDevice, &cmdBufAllocateInfo, &copyCmd));
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
                       fColorAttachment.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
        
        vkGetImageSubresourceLayout(fDevice, dstImage, &subResource, &subResourceLayout);
        
        // Map image memory so we can start copying from it
        vkMapMemory(fDevice, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
        imagedata += subResourceLayout.offset;
        uint32_t* toPixel = buffer;
        // ppm binary pixel data
        for (int32_t y = 0; y < fHeight; y++) {
            unsigned int *row = (unsigned int*)imagedata;
            for (int32_t x = 0; x < fWidth; x++) {
                *toPixel = *row;
                row++;
                toPixel++;
            }
            imagedata += subResourceLayout.rowPitch;
        }
        
        LOG("Framebuffer image saved copied\n");
        
        // Clean up resources
        vkUnmapMemory(fDevice, dstImageMemory);
        vkFreeMemory(fDevice, dstImageMemory, nullptr);
        vkDestroyImage(fDevice, dstImage, nullptr);
    }
    
}
VulkanOffscreenRenderer::~VulkanOffscreenRenderer()
{
    finalCleanup();
}

void VulkanOffscreenRenderer::finalCleanup()
{
    cleanupVertexIndexBuffers();
    
    if (NULL != fColorAttachment.view)
    {
        vkDestroyImageView(fDevice, fColorAttachment.view, nullptr);
    }
    if (NULL != fColorAttachment.image)
    {
        vkDestroyImage(fDevice, fColorAttachment.image, nullptr);
    }
    if (NULL != fColorAttachment.memory)
    {
        vkFreeMemory(fDevice, fColorAttachment.memory, nullptr);
    }
    if (NULL != fDepthAttachment.view)
    {
        vkDestroyImageView(fDevice, fDepthAttachment.view, nullptr);
    }
    if (NULL != fDepthAttachment.image)
    {
        vkDestroyImage(fDevice, fDepthAttachment.image, nullptr);
    }
    if (NULL != fDepthAttachment.memory)
    {
        vkFreeMemory(fDevice, fDepthAttachment.memory, nullptr);
    }
    if (NULL != fRenderPass)
    {
        vkDestroyRenderPass(fDevice, fRenderPass, nullptr);
    }
    if (NULL != fFramebuffer)
    {
        vkDestroyFramebuffer(fDevice, fFramebuffer, nullptr);
    }
    if (NULL != fPipelineLayout)
    {
        vkDestroyPipelineLayout(fDevice, fPipelineLayout, nullptr);
    }
    if (NULL != fDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(fDevice, fDescriptorSetLayout, nullptr);
    }
    if (NULL != fPipeline)
    {
        vkDestroyPipeline(fDevice, fPipeline, nullptr);
    }
    if (NULL != fPipelineCache)
    {
        vkDestroyPipelineCache(fDevice, fPipelineCache, nullptr);
    }
    if (NULL != fCommandPool)
    {
        vkDestroyCommandPool(fDevice, fCommandPool, nullptr);
    }
    for (auto shadermodule : fShaderModules) {
        vkDestroyShaderModule(fDevice, shadermodule, nullptr);
    }
    if (NULL != fDevice)
    {
        vkDestroyDevice(fDevice, nullptr);
    }
#if DEBUG
    if (debugReportCallback) {
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(fInstance, "vkDestroyDebugReportCallbackEXT"));
        assert(vkDestroyDebugReportCallback);
        vkDestroyDebugReportCallback(fInstance, debugReportCallback, nullptr);
    }
#endif
    if (NULL != fInstance)
    {
        vkDestroyInstance(fInstance, nullptr);
    }
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    vks::android::freeVulkanLibrary();
#endif
}


void VulkanOffscreenRenderer::cleanupVertexIndexBuffers()
{
    if (NULL != fVertexBuffer)
    {
        vkDestroyBuffer(fDevice, fVertexBuffer, nullptr);
        fVertexBuffer = NULL;
    }
    if (NULL != fVertexMemory)
    {
        vkFreeMemory(fDevice, fVertexMemory, nullptr);
        fVertexMemory = NULL;
    }
    
    if (NULL != fIndexBuffer)
    {
        vkDestroyBuffer(fDevice, fIndexBuffer, nullptr);
        fIndexBuffer = NULL;
    }
    if (NULL != fIndexMemory)
    {
        vkFreeMemory(fDevice, fIndexMemory, nullptr);
        fIndexMemory = NULL;
    }
}

void VulkanOffscreenRenderer::initializeCommandPool()
{
    LOG("initializeCommandPool\n");
    
    // Command pool
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = fQueueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(fDevice, &cmdPoolInfo, nullptr, &fCommandPool));
    
    
}


void VulkanOffscreenRenderer::initializeFrameBuffer(int aWidth, int aHeight, VkFormat colorFormat, VkFormat *depthFormat)
{
    /*
     Create framebuffer attachments
     */
    fWidth = aWidth;
    fHeight = aHeight;
    vks::tools::getSupportedDepthFormat(fPhysicalDevice, depthFormat);
    LOG("initializeFrameBuffer width %d, height %d, colorFormat %d, depthFormat %d\n", fWidth, fHeight, colorFormat, *depthFormat);
    
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
    
    VK_CHECK_RESULT(vkCreateImage(fDevice, &image, nullptr, &fColorAttachment.image));
    vkGetImageMemoryRequirements(fDevice, fColorAttachment.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAlloc, nullptr, &fColorAttachment.memory));
    VK_CHECK_RESULT(vkBindImageMemory(fDevice, fColorAttachment.image, fColorAttachment.memory, 0));
    
    VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
    colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageView.format = colorFormat;
    colorImageView.subresourceRange = {};
    colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageView.subresourceRange.baseMipLevel = 0;
    colorImageView.subresourceRange.levelCount = 1;
    colorImageView.subresourceRange.baseArrayLayer = 0;
    colorImageView.subresourceRange.layerCount = 1;
    colorImageView.image = fColorAttachment.image;
    VK_CHECK_RESULT(vkCreateImageView(fDevice, &colorImageView, nullptr, &fColorAttachment.view));
    
    // Depth stencil attachment
    image.format = *depthFormat;
    image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    
    VK_CHECK_RESULT(vkCreateImage(fDevice, &image, nullptr, &fDepthAttachment.image));
    vkGetImageMemoryRequirements(fDevice, fDepthAttachment.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAlloc, nullptr, &fDepthAttachment.memory));
    VK_CHECK_RESULT(vkBindImageMemory(fDevice, fDepthAttachment.image, fDepthAttachment.memory, 0));
    
    VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
    depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthStencilView.format = *depthFormat;
    depthStencilView.flags = 0;
    depthStencilView.subresourceRange = {};
    depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthStencilView.subresourceRange.baseMipLevel = 0;
    depthStencilView.subresourceRange.levelCount = 1;
    depthStencilView.subresourceRange.baseArrayLayer = 0;
    depthStencilView.subresourceRange.layerCount = 1;
    depthStencilView.image = fDepthAttachment.image;
    VK_CHECK_RESULT(vkCreateImageView(fDevice, &depthStencilView, nullptr, &fDepthAttachment.view));
}


void VulkanOffscreenRenderer::createRenderPass(VkFormat colorFormat, VkFormat depthFormat)
{
    LOG("createRenderPass");
    /*
     Create renderpass
     */
    
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
    VK_CHECK_RESULT(vkCreateRenderPass(fDevice, &renderPassInfo, nullptr, &fRenderPass));
    
    VkImageView attachments[2];
    attachments[0] = fColorAttachment.view;
    attachments[1] = fDepthAttachment.view;
    
    VkFramebufferCreateInfo framebufferCreateInfo = vks::initializers::framebufferCreateInfo();
    framebufferCreateInfo.renderPass = fRenderPass;
    framebufferCreateInfo.attachmentCount = 2;
    framebufferCreateInfo.pAttachments = attachments;
    framebufferCreateInfo.width = fWidth;
    framebufferCreateInfo.height = fHeight;
    framebufferCreateInfo.layers = 1;
    VK_CHECK_RESULT(vkCreateFramebuffer(fDevice, &framebufferCreateInfo, nullptr, &fFramebuffer));
}

void VulkanOffscreenRenderer::prepareGraphicsPipeline()
{
    LOG("prepareGraphicsPipeline");
    /*
     Prepare graphics pipeline
     */
    
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
    VkDescriptorSetLayoutCreateInfo descriptorLayout =
    vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(fDevice, &descriptorLayout, nullptr, &fDescriptorSetLayout));
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
    vks::initializers::pipelineLayoutCreateInfo(nullptr, 0);
    
    // MVP via push constant block
    VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
    pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    
    VK_CHECK_RESULT(vkCreatePipelineLayout(fDevice, &pipelineLayoutCreateInfo, nullptr, &fPipelineLayout));
    
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(fDevice, &pipelineCacheCreateInfo, nullptr, &fPipelineCache));
    
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
    
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    shaderStages[0].module = vks::tools::loadShader(androidapp->activity->assetManager, ASSET_PATH "shaders/renderheadless/triangle.vert.spv", device);
    shaderStages[1].module = vks::tools::loadShader(androidapp->activity->assetManager, ASSET_PATH "shaders/renderheadless/triangle.frag.spv", device);
#else
    shaderStages[0].module = vks::tools::loadShader(ASSET_PATH "shaders/triangle.vert.spv", fDevice);
    shaderStages[1].module = vks::tools::loadShader(ASSET_PATH "shaders/triangle.frag.spv", fDevice);
#endif
    fShaderModules = { shaderStages[0].module, shaderStages[1].module };
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(fDevice, fPipelineCache, 1, &pipelineCreateInfo, nullptr, &fPipeline));
}

#if DEBUG
VkDebugReportCallbackEXT debugReportCallback{};
#endif

uint32_t VulkanOffscreenRenderer::getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(fPhysicalDevice, &deviceMemoryProperties);
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        typeBits >>= 1;
    }
    return 0;
}

VkResult VulkanOffscreenRenderer::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer *buffer, VkDeviceMemory *memory, VkDeviceSize size, void *data)
{
    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_CHECK_RESULT(vkCreateBuffer(fDevice, &bufferCreateInfo, nullptr, buffer));
    
    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(fDevice, *buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, memoryPropertyFlags);
    VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAlloc, nullptr, memory));
    
    if (data != nullptr) {
        void *mapped;
        VK_CHECK_RESULT(vkMapMemory(fDevice, *memory, 0, size, 0, &mapped));
        memcpy(mapped, data, size);
        vkUnmapMemory(fDevice, *memory);
    }
    
    VK_CHECK_RESULT(vkBindBufferMemory(fDevice, *buffer, *memory, 0));
    
    return VK_SUCCESS;
}

/*
 Submit command buffer to a queue and wait for fence until queue operations have been finished
 */
void VulkanOffscreenRenderer::submitWork(VkCommandBuffer cmdBuffer, VkQueue queue)
{
    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;
    VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo();
    VkFence fence;
    VK_CHECK_RESULT(vkCreateFence(fDevice, &fenceInfo, nullptr, &fence));
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
    VK_CHECK_RESULT(vkWaitForFences(fDevice, 1, &fence, VK_TRUE, UINT64_MAX));
    vkDestroyFence(fDevice, fence, nullptr);
}

void VulkanOffscreenRenderer::normalRun()
{
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
    
    if (layersAvailable) {
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
        const char *validationExt = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
        instanceCreateInfo.enabledLayerCount = layerCount;
        instanceCreateInfo.enabledExtensionCount = 1;
        instanceCreateInfo.ppEnabledExtensionNames = &validationExt;
    }
#endif
    VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &fInstance));
    
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
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(fInstance, "vkCreateDebugReportCallbackEXT"));
        assert(vkCreateDebugReportCallbackEXT);
        VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(fInstance, &debugReportCreateInfo, nullptr, &debugReportCallback));
    }
#endif
    
    /*
     Vulkan device creation
     */
    uint32_t deviceCount = 0;
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(fInstance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(fInstance, &deviceCount, physicalDevices.data()));
    fPhysicalDevice = physicalDevices[0];
    
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(fPhysicalDevice, &deviceProperties);
    LOG("GPU: %s\n", deviceProperties.deviceName);
    
    // Request a single graphics queue
    const float defaultQueuePriority(0.0f);
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(fPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(fPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());
    for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            fQueueFamilyIndex = i;
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
    VK_CHECK_RESULT(vkCreateDevice(fPhysicalDevice, &deviceCreateInfo, nullptr, &fDevice));
    
    // Get a graphics queue
    vkGetDeviceQueue(fDevice, fQueueFamilyIndex, 0, &fQueue);
    
    LOG("initializeCommandPool\n");
    // Command pool
    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = fQueueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(vkCreateCommandPool(fDevice, &cmdPoolInfo, nullptr, &fCommandPool));
    
    LOG("initializeBuffers\n");
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
        VK_CHECK_RESULT(vkAllocateCommandBuffers(fDevice, &cmdBufAllocateInfo, &copyCmd));
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
            
            vkDestroyBuffer(fDevice, stagingBuffer, nullptr);
            vkFreeMemory(fDevice, stagingMemory, nullptr);
            
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
            
            vkDestroyBuffer(fDevice, stagingBuffer, nullptr);
            vkFreeMemory(fDevice, stagingMemory, nullptr);
        }
    }
    
    /*
     Create framebuffer attachments
     */
    fWidth = 1024;
    fHeight = 1024;
    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormat depthFormat;
    vks::tools::getSupportedDepthFormat(fPhysicalDevice, &depthFormat);
    LOG("initializeFrameBuffer width %d, height %d, colorFormat %d, depthFormat %d", fWidth, fHeight, colorFormat, depthFormat);
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
        
        VK_CHECK_RESULT(vkCreateImage(fDevice, &image, nullptr, &fColorAttachment.image));
        vkGetImageMemoryRequirements(fDevice, fColorAttachment.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAlloc, nullptr, &fColorAttachment.memory));
        VK_CHECK_RESULT(vkBindImageMemory(fDevice, fColorAttachment.image, fColorAttachment.memory, 0));
        
        VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
        colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageView.format = colorFormat;
        colorImageView.subresourceRange = {};
        colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageView.subresourceRange.baseMipLevel = 0;
        colorImageView.subresourceRange.levelCount = 1;
        colorImageView.subresourceRange.baseArrayLayer = 0;
        colorImageView.subresourceRange.layerCount = 1;
        colorImageView.image = fColorAttachment.image;
        VK_CHECK_RESULT(vkCreateImageView(fDevice, &colorImageView, nullptr, &fColorAttachment.view));
        
        // Depth stencil attachment
        image.format = depthFormat;
        image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        
        VK_CHECK_RESULT(vkCreateImage(fDevice, &image, nullptr, &fDepthAttachment.image));
        vkGetImageMemoryRequirements(fDevice, fDepthAttachment.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAlloc, nullptr, &fDepthAttachment.memory));
        VK_CHECK_RESULT(vkBindImageMemory(fDevice, fDepthAttachment.image, fDepthAttachment.memory, 0));
        
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
        depthStencilView.image = fDepthAttachment.image;
        VK_CHECK_RESULT(vkCreateImageView(fDevice, &depthStencilView, nullptr, &fDepthAttachment.view));
    }
    
    
    /*
     Create renderpass
     */
    LOG("createRenderPass");
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
        VK_CHECK_RESULT(vkCreateRenderPass(fDevice, &renderPassInfo, nullptr, &fRenderPass));
        
        VkImageView attachments[2];
        attachments[0] = fColorAttachment.view;
        attachments[1] = fDepthAttachment.view;
        
        VkFramebufferCreateInfo framebufferCreateInfo = vks::initializers::framebufferCreateInfo();
        framebufferCreateInfo.renderPass = fRenderPass;
        framebufferCreateInfo.attachmentCount = 2;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = fWidth;
        framebufferCreateInfo.height = fHeight;
        framebufferCreateInfo.layers = 1;
        VK_CHECK_RESULT(vkCreateFramebuffer(fDevice, &framebufferCreateInfo, nullptr, &fFramebuffer));
    }
    
    /*
     Prepare graphics pipeline
     */
    LOG("prepareGraphicsPipeline");
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
        VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(fDevice, &descriptorLayout, nullptr, &fDescriptorSetLayout));
        
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(nullptr, 0);
        
        // MVP via push constant block
        VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
        
        VK_CHECK_RESULT(vkCreatePipelineLayout(fDevice, &pipelineLayoutCreateInfo, nullptr, &fPipelineLayout));
        
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreatePipelineCache(fDevice, &pipelineCacheCreateInfo, nullptr, &fPipelineCache));
        
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
        
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].pName = "main";
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].pName = "main";
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        shaderStages[0].module = vks::tools::loadShader(androidapp->activity->assetManager, ASSET_PATH "shaders/renderheadless/triangle.vert.spv", device);
        shaderStages[1].module = vks::tools::loadShader(androidapp->activity->assetManager, ASSET_PATH "shaders/renderheadless/triangle.frag.spv", device);
#else
        shaderStages[0].module = vks::tools::loadShader(ASSET_PATH "shaders/triangle.vert.spv", fDevice);
        shaderStages[1].module = vks::tools::loadShader(ASSET_PATH "shaders/triangle.frag.spv", fDevice);
#endif
        fShaderModules = { shaderStages[0].module, shaderStages[1].module };
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(fDevice, fPipelineCache, 1, &pipelineCreateInfo, nullptr, &fPipeline));
    }
    
    /*
     Command buffer creation
     */
    {
        LOG("renderScene");
        VkCommandBuffer commandBuffer;
        VkCommandBufferAllocateInfo cmdBufAllocateInfo =
        vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(fDevice, &cmdBufAllocateInfo, &commandBuffer));
        
        VkCommandBufferBeginInfo cmdBufInfo =
        vks::initializers::commandBufferBeginInfo();
        
        VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));
        
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
        renderPassBeginInfo.framebuffer = fFramebuffer;
        
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        VkViewport viewport = {};
        viewport.height = (float)fHeight;
        viewport.width = (float)fWidth;
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
        // Update dynamic scissor state
        VkRect2D scissor = {};
        scissor.extent.width = fWidth;
        scissor.extent.height = fHeight;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fPipeline);
        
        // Render scene
        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &fVertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, fIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
        
        std::vector<glm::vec3> pos = {
            glm::vec3(-1.5f, 0.0f, -4.0f),
            glm::vec3( 0.0f, 0.0f, -2.5f),
            glm::vec3( 1.5f, 0.0f, -4.0f),
        };
        
        for (auto v : pos) {
            glm::mat4 mvpMatrix = glm::perspective(glm::radians(60.0f), (float)fWidth / (float)fHeight, 0.1f, 256.0f) * glm::translate(glm::mat4(1.0f), v);
            vkCmdPushConstants(commandBuffer, fPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvpMatrix), &mvpMatrix);
            vkCmdDrawIndexed(commandBuffer, 3, 1, 0, 0, 0);
        }
        
        vkCmdEndRenderPass(commandBuffer);
        
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
        
        submitWork(commandBuffer, fQueue);
        
        vkDeviceWaitIdle(fDevice);
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
        VK_CHECK_RESULT(vkCreateImage(fDevice, &imgCreateInfo, nullptr, &dstImage));
        // Create memory to back up the image
        VkMemoryRequirements memRequirements;
        VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
        VkDeviceMemory dstImageMemory;
        vkGetImageMemoryRequirements(fDevice, dstImage, &memRequirements);
        memAllocInfo.allocationSize = memRequirements.size;
        // Memory must be host visible to copy from
        memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(fDevice, &memAllocInfo, nullptr, &dstImageMemory));
        VK_CHECK_RESULT(vkBindImageMemory(fDevice, dstImage, dstImageMemory, 0));
        
        // Do the actual blit from the offscreen image to our host visible destination image
        VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(fCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
        VkCommandBuffer copyCmd;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(fDevice, &cmdBufAllocateInfo, &copyCmd));
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
                       fColorAttachment.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
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
        
        vkGetImageSubresourceLayout(fDevice, dstImage, &subResource, &subResourceLayout);
        
        // Map image memory so we can start copying from it
        vkMapMemory(fDevice, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
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
        file << "P6\n" << fWidth << "\n" << fHeight << "\n" << 255 << "\n";
        
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
        vkUnmapMemory(fDevice, dstImageMemory);
        vkFreeMemory(fDevice, dstImageMemory, nullptr);
        vkDestroyImage(fDevice, dstImage, nullptr);
    }
    
    vkQueueWaitIdle(fQueue);
    
}
