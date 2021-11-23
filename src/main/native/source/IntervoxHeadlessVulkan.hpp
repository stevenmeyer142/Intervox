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
