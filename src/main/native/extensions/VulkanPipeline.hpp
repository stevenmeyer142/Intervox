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
#include "base/camera.hpp"

typedef int32_t mesh_id_t;

enum VulkanPipelineTypes
{
    MESH_PIPELINE
};

struct MeshPipelineSettings
{
    std::set<mesh_id_t> fMeshIds;
    
    void addMeshID(mesh_id_t meshID) { fMeshIds.insert(meshID);}
    void removeMeshID(mesh_id_t meshID) { fMeshIds.erase(meshID);}
    bool hasMeshID(mesh_id_t meshID) { return fMeshIds.find(meshID) != fMeshIds.end(); }
};

// TODO: make this a class
struct RenderCommandSettings
{
 //   VkCommandBuffer fCommandBuffer = VK_NULL_HANDLE;  // destroy command buffers needs fields from VulkanExampleBase, use map
    Camera fCamera;
    int32_t         fPipelinesVersion = -1;
    std::string     fContextID;
    glm::vec3            fRotation;
    MeshPipelineSettings fMeshPipelineSettings;
 };


class VulkanPipeline {
public:
    virtual void updateUniformBuffer(RenderCommandSettings &renderCommandSettings) = 0;
    virtual uint32_t getUniformBufferCount() = 0;
    virtual void Draw(VkCommandBuffer drawCommandBuffer, RenderCommandSettings &renderCommandSettings) = 0;
    virtual void setupLayoutsAndPipeline(const std::string& shadersPath, VkRenderPass renderPass, VkPipelineCache pipelineCache) = 0;
    virtual void setupDescripterSets(VkDescriptorPool pool) = 0;
    virtual ~VulkanPipeline() {}
};



#endif /* VulkanPipeline_h */
