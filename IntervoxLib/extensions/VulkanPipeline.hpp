//
//  VulkanPipeline.hpp
//  IntervoxLib
//
//  Created by Steven Meyer on 2/24/22.
//
#pragma once
#ifndef VulkanPipeline_h
#define VulkanPipeline_h

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


enum VulkanPipelineTypes
{
    MESH_PIPELINE
};

class VulkanPipeline {
public:
    virtual void updateUniformBuffer(glm::mat4 perspective, glm::mat4 view) = 0;
    virtual uint32_t getUniformBufferCount() = 0;
    virtual void Draw(VkCommandBuffer drawCommandBuffer) = 0;
    virtual void setupLayoutsAndPipeline(const std::string& shadersPath, VkRenderPass renderPass, VkPipelineCache pipelineCache) = 0;
    virtual void setupDescripterSets(VkDescriptorPool pool) = 0;
    virtual ~VulkanPipeline() {}
};

struct PipelineSettings
{
    
    virtual ~PipelineSettings() {}
};

struct MeshPipelineSettings : public PipelineSettings
{
    std::vector<int32_t> fMeshIds;
};

struct RenderCommandSettings
{
    std::map<VulkanPipelineTypes, std::shared_ptr<PipelineSettings>> fPipelineSettings;
 //   VkCommandBuffer fCommandBuffer = VK_NULL_HANDLE;  // destroy command buffers needs fields from VulkanExampleBase, use map
    int32_t         fPipelinesVersion = -1;
    std::string     fContextID;
 };

#endif /* VulkanPipeline_h */
