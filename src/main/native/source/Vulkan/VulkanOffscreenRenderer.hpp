//
//  VulkanOffscreenRenderer.hpp
//  Intervox
//
//  Created by Steven Meyer on 2/9/20.
//  Copyright © 2020 Steven Meyer. All rights reserved.
//

#ifndef VulkanOffscreenRenderer_hpp
#define VulkanOffscreenRenderer_hpp

#include <vector>
#include <vulkan/vulkan.h>

class VulkanOffscreenRenderer
{
    VkInstance fInstance;
    VkPhysicalDevice fPhysicalDevice;
    VkDevice fDevice;
    uint32_t fQueueFamilyIndex;
    VkPipelineCache fPipelineCache;
    VkQueue fQueue;
    VkCommandPool fCommandPool;
    VkDescriptorSetLayout fDescriptorSetLayout;
    VkPipelineLayout fPipelineLayout;
    VkPipeline fPipeline;
    std::vector<VkShaderModule> fShaderModules;
    VkBuffer fVertexBuffer, fIndexBuffer;
    VkDeviceMemory fVertexMemory, fIndexMemory;
    
    struct FrameBufferAttachment {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
    };
    int32_t fWidth, fHeight;
    VkFramebuffer fFramebuffer;
    FrameBufferAttachment fColorAttachment, fDepthAttachment;
    VkRenderPass fRenderPass;
    
    
public:
    
    struct Vertex {
        float position[3];
        float color[3];
    };
    
    VulkanOffscreenRenderer(); 
     
    void initialize(int width, int height);
     
    void setIndexVertexBuffers(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

    void renderScene();
    
    void copyImageData_RGBA_8888(uint32_t* buffer, int width, int height);
    
    ~VulkanOffscreenRenderer();
    
private :
       
    void initializeDevice();
    
    void finalCleanup();
    
    void cleanupVertexIndexBuffers();
    
    void initializeCommandPool();
     
    void initializeFrameBuffer(int aWidth, int aHeight, VkFormat colorFormat, VkFormat *depthFormat);
    
    
    
    void createRenderPass(VkFormat colorFormat, VkFormat depthFormat);
    
    
    void prepareGraphicsPipeline();
    
    
    
#if DEBUG
    VkDebugReportCallbackEXT debugReportCallback{};
#endif
    
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
    
    VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer *buffer, VkDeviceMemory *memory, VkDeviceSize size, void *data = nullptr);
    
    
    /*
     Submit command buffer to a queue and wait for fence until queue operations have been finished
     */
    void submitWork(VkCommandBuffer cmdBuffer, VkQueue queue);
    
    
public :
    void normalRun();
    
    
};

#endif /* VulkanOffscreenRenderer_hpp */
