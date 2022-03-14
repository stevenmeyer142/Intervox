//
//  IntervoxHeadlessVulkan.hpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/2/21.
//

#ifndef IntervoxHeadlessVulkan_hpp
#define IntervoxHeadlessVulkan_hpp


#include "base/vulkanexamplebase.h"

#define GEARS 1

#if GEARS
#include "gears/vulkangear.h"
#endif

#include <vector>
#include <memory>
#include <atomic>
#include <map>

#if USE_MESH_PIPELINE
#include "extensions/VulkanMeshPipeline.hpp"
#include "extensions/VulkanPipeline.hpp"
#endif

#define ENABLE_VALIDATION 1

class CJavaArrSlicesSet;

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

    std::vector<std::shared_ptr<VulkanGear>> fGears;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

#endif
    std::vector<uint8_t> fImageData;

#if USE_MESH_PIPELINE
    std::map<VulkanPipelineTypes, std::shared_ptr<VulkanPipeline>> fPipelines;
#endif

    bool            fInitialized = false;
    int32_t         fPipelinesVersion = 0;
    std::map<std::string, VkCommandBuffer> fContextCommandBuffers;

public:
    IntervoxHeadlessVulkan();

    ~IntervoxHeadlessVulkan();
    
    void initialize(uint32_t width, uint32_t height);
    
    void renderScene(RenderCommandSettings &renderCommandSettings);
    
    void copyImageData_RGBA_8888(uint32_t* toBuffer, uint32_t aWidth, uint32_t aHeight);

    void buildCommandBuffers(RenderCommandSettings &renderCommandSettings, VkCommandBuffer drawCommandBuffer);

    void prepareVertices(bool offset);

    void setupDescriptorPool();
    
    void setupDescriptorSetLayout();

    void setupDescriptorSets();

    void preparePipelines();
 
    void updateUniformBuffers();

    void draw(VkCommandBuffer drawCommandBuffer);

    void prepare();

    void render(VkCommandBuffer drawCommandBuffer);
    
    virtual void render() {}

    virtual void viewChanged();
    
    void grabImage();
    
    void rotate(float xRot, float yRot);
    
    // TODO change CJavaArrSlicesSet to CSlicesSet
    void* addMeshForRegion(CJavaArrSlicesSet *slicesSet, int regionValue);
    
private:
    void updateDescriptorLayouts();

    void ComputeWeightedCenter(CJavaArrSlicesSet *slicesSet, short region, std::shared_ptr<VulkanMesh> mesh);
    
    VkCommandBuffer getCommandBuffer(RenderCommandSettings &renderCommandSettings);
    
    std::shared_ptr<VulkanMeshPipeline> getMeshPipeline();

};

#endif /* IntervoxHeadlessVulkan_hpp */
