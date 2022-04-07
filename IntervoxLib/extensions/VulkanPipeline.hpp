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
#include <set>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

typedef int32_t mesh_id_t;

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


struct MeshPipelineSettings
{
    std::set<mesh_id_t> fMeshIds;
};

// TODO: make this a class
struct RenderCommandSettings
{
 //   VkCommandBuffer fCommandBuffer = VK_NULL_HANDLE;  // destroy command buffers needs fields from VulkanExampleBase, use map
    int32_t         fPipelinesVersion = -1;
    std::string     fContextID;
    MeshPipelineSettings fMeshPipelineSettings;
 };

#endif /* VulkanPipeline_h */
