//
//  VulkanPipeline.hpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/23/21.
//

#ifndef VulkanMeshPipeline_hpp
#define VulkanMeshPipeline_hpp

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "extensions/VulkanMesh.hpp"
#include "base/vulkanexamplebase.h"

class VulkanMeshPipeline
{
    public :
    VulkanMeshPipeline(VkDevice device);
    
    virtual ~VulkanMeshPipeline();
    
    virtual void Draw(VkCommandBuffer commandBuffer);
    
    void addMesh(std::shared_ptr<VulkanMesh> mesh);
    
    void updateUniformBuffer(glm::mat4 perspective, glm::mat4 view);
    
    void setupDescriptorsAndPipeline(const std::string& shadersPath, VkRenderPass renderPass,
                                     VkPipelineCache pipelineCache, VkDescriptorPool pool);
    


private:
    
    void setupDescriptorSetLayout();
    
    void createPipeline(const std::string& shadersPath, VkRenderPass renderPass,
                        VkPipelineCache pipelineCache);
    
    void setupVertexDescriptions();

    VkPipelineShaderStageCreateInfo loadShader(std::string fileName, VkShaderStageFlagBits stage);
    
    std::vector<std::shared_ptr<VulkanMesh>>  fMeshes;
    
    // TODO Add object cleanup
    VkPipeline              fPipeline = VK_NULL_HANDLE;
    VkPipelineLayout        fPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout   fDescriptorSetLayout = VK_NULL_HANDLE;
    VkDevice                fDevice = VK_NULL_HANDLE;

    struct {
        VkPipelineVertexInputStateCreateInfo inputState;
        std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    } fVertices;
    // List of shader modules created (stored for cleanup)
    std::vector<VkShaderModule> fShaderModules;

};

#endif /* VulkanMeshPipeline_hpp */
