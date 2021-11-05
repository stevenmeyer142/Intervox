//
//  IntervoxHeadlessVulkan.hpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/2/21.
//

#ifndef IntervoxHeadlessVulkan_hpp
#define IntervoxHeadlessVulkan_hpp


#include "base/vulkanexamplebase.h"

#define GEARS 0

#if GEARS
#include "gears/vulkangear.h"
#endif

#include <vector>


#define ENABLE_VALIDATION 0

class IntervoxHeadlessVulkan : public  VulkanExampleBase  {
#if GEARS
    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } vertices;

    struct {
        VkPipeline solid;
    } pipelines;

    std::vector<VulkanGear*> gears;

    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
#endif
    std::vector<uint8_t> fImageData;
    uint32_t    fWidth = 0;
    uint32_t    fHeight = 0;

#if DEBUG_RENDER
#if DEBUG_RENDER_DELETE
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t queueFamilyIndex;

    VkPipelineCache fPipelineCache;
    VkQueue fQueue;
    VkCommandPool fCommandPool;
#endif
    VkCommandBuffer fCommandBuffer;

    VkDescriptorSetLayout fDescriptorSetLayout;
    VkPipelineLayout fPipelineLayout;
    VkPipeline fPipeline;
    std::vector<VkShaderModule> fShaderModules;
    VkBuffer fVertexBuffer, fIndexBuffer;
    VkDeviceMemory fVertexMemory, fIndexMemory;

#if DEBUG_RENDER_DELETE
   struct FrameBufferAttachment {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
    };
#endif
    
    int32_t width, height;
    
//#if DEBUG_RENDER_DELETE
    VkFramebuffer framebuffer;
    FrameBufferAttachment colorAttachment, depthAttachment;
//#endif
    VkRenderPass fRenderPass;

    VkDebugReportCallbackEXT debugReportCallback{};

    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
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

    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer *buffer, VkDeviceMemory *memory, VkDeviceSize size, void *data = nullptr)
    {
        // Create the buffer handle
        VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer));

        // Create the memory backing up the buffer handle
        VkMemoryRequirements memReqs;
        VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
        vkGetBufferMemoryRequirements(device, *buffer, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, memoryPropertyFlags);
        VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, memory));

        if (data != nullptr) {
            void *mapped;
            VK_CHECK_RESULT(vkMapMemory(device, *memory, 0, size, 0, &mapped));
            memcpy(mapped, data, size);
            vkUnmapMemory(device, *memory);
        }

        VK_CHECK_RESULT(vkBindBufferMemory(device, *buffer, *memory, 0));

        return VK_SUCCESS;
    }

    /*
        Submit command buffer to a queue and wait for fence until queue operations have been finished
    */
    void submitWork(VkCommandBuffer cmdBuffer, VkQueue queue)
    {
        VkSubmitInfo submitInfo = vks::initializers::submitInfo();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo();
        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &fence));
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
        VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
        vkDestroyFence(device, fence, nullptr);
    }

#endif
public:
    IntervoxHeadlessVulkan();

    ~IntervoxHeadlessVulkan();

    void buildCommandBuffers();

    void prepareVertices();

    void setupDescriptorPool();
    
    void setupDescriptorSetLayout();

    void setupDescriptorSets();

    void preparePipelines();
 
    void updateUniformBuffers();

    void draw();

    void prepare();

    virtual void render();

    virtual void viewChanged();
    
    void grabImage();

};

#endif /* IntervoxHeadlessVulkan_hpp */
