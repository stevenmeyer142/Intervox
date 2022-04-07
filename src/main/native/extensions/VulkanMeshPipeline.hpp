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
#include "extensions/VulkanPipeline.hpp"

class VulkanMeshPipeline : public VulkanPipeline
{
public :
    VulkanMeshPipeline(VkDevice device);
    
    virtual ~VulkanMeshPipeline();
    
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    
    void addMesh(std::shared_ptr<VulkanMesh> mesh);
    
    virtual void updateUniformBuffer(glm::mat4 perspective, glm::mat4 view) override;
    
    virtual void setupLayoutsAndPipeline(const std::string& shadersPath, VkRenderPass renderPass, VkPipelineCache pipelineCache) override;
    
    virtual void setupDescripterSets(VkDescriptorPool pool) override;
    
    virtual uint32_t getUniformBufferCount() override;
    
    void setMeshColor(int32_t meshID, const glm::vec3& color);
    
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
