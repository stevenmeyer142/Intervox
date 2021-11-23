//
//  VulkanPipeline.hpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/23/21.
//

#ifndef VulkanPipeline_hpp
#define VulkanPipeline_hpp

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "extensions/VulkanRenderable.hpp"

class VulkanPipeline
{
    public :
    VulkanPipeline();
    
    virtual ~VulkanPipeline();
    
private:
    std::vector<std::shared_ptr<VulkanRenderable>>  fRenderables;
    
    VkPipeline              fPipeline = VK_NULL_HANDLE;
    VkPipelineLayout        fPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout   fDescriptorSetLayout = VK_NULL_HANDLE;
};

#endif /* VulkanPipeline_hpp */
