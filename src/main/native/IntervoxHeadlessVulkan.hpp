//
//  IntervoxHeadlessVulkan.hpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/2/21.
//

#ifndef IntervoxHeadlessVulkan_hpp
#define IntervoxHeadlessVulkan_hpp


#include "base/vulkanexamplebase.h"

#include <vector>
#include <memory>
#include <atomic>
#include <map>

#include "extensions/VulkanMeshPipeline.hpp"
#include "extensions/VulkanPipeline.hpp"


#define ENABLE_VALIDATION 1

class CJavaArrSlicesSet;

class IntervoxHeadlessVulkan : public  VulkanExampleBase  {
    std::vector<uint8_t> fImageData;
    std::map<VulkanPipelineTypes, std::shared_ptr<VulkanPipeline>> fPipelines;
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

     void setupDescriptorPool();
    
    void updateUniformBuffers(RenderCommandSettings &renderCommandSettings);

    void draw(VkCommandBuffer drawCommandBuffer);

    void prepare();

    void render(VkCommandBuffer drawCommandBuffer);
    
    virtual void render() {}

 //   virtual void viewChanged();
    
    void grabImage();
     
    // TODO change CJavaArrSlicesSet to CSlicesSet
    int32_t addMeshForRegion(CJavaArrSlicesSet *slicesSet, int regionValue);
    
    void setMeshColor(int32_t meshID, float red, float green, float blue);
    
    uint32_t getWidth() { return width; }
    
    uint32_t getHeight() { return height; }
    
private:
    void updateDescriptorLayouts();

    void ComputeWeightedCenter(CJavaArrSlicesSet *slicesSet, short region, std::shared_ptr<VulkanMesh> mesh);
    
    VkCommandBuffer getCommandBuffer(RenderCommandSettings &renderCommandSettings);
    
    std::shared_ptr<VulkanMeshPipeline> getMeshPipeline();

};

#endif /* IntervoxHeadlessVulkan_hpp */
