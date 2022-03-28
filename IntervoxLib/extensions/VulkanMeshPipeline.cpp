//
//  VulkanPipeline.cpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/23/21.
//

#include "VulkanMeshPipeline.hpp"
#include <array>

#define VERTEX_BUFFER_BIND_ID 0


VulkanMeshPipeline::VulkanMeshPipeline(VkDevice device) :
    fDevice(device){
   
}

VulkanMeshPipeline::~VulkanMeshPipeline()
{
    for (auto& shaderModule : fShaderModules)
    {
        vkDestroyShaderModule(fDevice, shaderModule, nullptr);
    }
    vkDestroyPipeline(fDevice, fPipeline, nullptr);
    vkDestroyPipelineLayout(fDevice, fPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(fDevice, fDescriptorSetLayout, nullptr);
}


void VulkanMeshPipeline::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fPipeline);
    // TODO move descripterset from mesh to here
    for (auto mesh : fMeshes)
    {
        mesh->Draw(commandBuffer, fPipelineLayout);
    }
}

void VulkanMeshPipeline::addMesh(std::shared_ptr<VulkanMesh> mesh)
{
    fMeshes.push_back(mesh);
}

uint32_t VulkanMeshPipeline::getUniformBufferCount()
{
    return static_cast<uint32_t>(fMeshes.size());
}



void VulkanMeshPipeline::updateUniformBuffer(glm::mat4 perspective, glm::mat4 view)
{
    for (auto mesh : fMeshes)
    {
        mesh->updateUniformBuffer(perspective, view);
    }
}

void VulkanMeshPipeline::createPipeline(const std::string& shadersPath, VkRenderPass renderPass,
                                        VkPipelineCache pipelineCache)
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        vks::initializers::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            0,
            VK_FALSE);

    // TODO change these to defaults
    VkPipelineRasterizationStateCreateInfo rasterizationState =
        vks::initializers::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_NONE, //VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
       //     VK_CULL_MODE_NONE,
        //    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                                0);

    VkPipelineColorBlendAttachmentState blendAttachmentState =
        vks::initializers::pipelineColorBlendAttachmentState(
            0xf,
            VK_FALSE);

    VkPipelineColorBlendStateCreateInfo colorBlendState =
        vks::initializers::pipelineColorBlendStateCreateInfo(
            1,
            &blendAttachmentState);

    VkPipelineDepthStencilStateCreateInfo depthStencilState =
        vks::initializers::pipelineDepthStencilStateCreateInfo(
            VK_TRUE,
            VK_TRUE,
            VK_COMPARE_OP_LESS_OR_EQUAL);

    VkPipelineViewportStateCreateInfo viewportState =
        vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

    VkPipelineMultisampleStateCreateInfo multisampleState =
        vks::initializers::pipelineMultisampleStateCreateInfo(
            VK_SAMPLE_COUNT_1_BIT,
            0);

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState =
        vks::initializers::pipelineDynamicStateCreateInfo(
            dynamicStateEnables.data(),
            static_cast<uint32_t>(dynamicStateEnables.size()),
            0);

    // Solid rendering pipeline
    // Load shaders
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

    shaderStages[0] = loadShader(shadersPath + "gears/gears.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(shadersPath + "gears/gears.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
        vks::initializers::pipelineCreateInfo(
            fPipelineLayout,
            renderPass,
            0);

    pipelineCreateInfo.pVertexInputState = &fVertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(fDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &fPipeline));

}


void VulkanMeshPipeline::setupDescriptorSetLayout()
{

    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
    {
        // Binding 0 : Vertex shader uniform buffer
        vks::initializers::descriptorSetLayoutBinding(
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            VK_SHADER_STAGE_VERTEX_BIT,
            0)
    };

    VkDescriptorSetLayoutCreateInfo descriptorLayout =
        vks::initializers::descriptorSetLayoutCreateInfo(
            setLayoutBindings.data(),
            static_cast<uint32_t>(setLayoutBindings.size()));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(fDevice, &descriptorLayout, nullptr, &fDescriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &fDescriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(fDevice, &pPipelineLayoutCreateInfo, nullptr, &fPipelineLayout));

}

void VulkanMeshPipeline::setupVertexDescriptions()
{
    // Binding and attribute descriptions are shared across all gears
    fVertices.bindingDescriptions.resize(1);
    fVertices.bindingDescriptions[0] =
        vks::initializers::vertexInputBindingDescription(
            VERTEX_BUFFER_BIND_ID,
            sizeof(MeshVertex),
            VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // Describes memory layout and shader positions
    fVertices.attributeDescriptions.resize(2);
    // Location 0 : Position
    fVertices.attributeDescriptions[0] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0);
    // Location 1 : Normal
    fVertices.attributeDescriptions[1] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            sizeof(float) * 3);
    // Location 2 : Color
//    fVertices.attributeDescriptions[2] =
//        vks::initializers::vertexInputAttributeDescription(
//            VERTEX_BUFFER_BIND_ID,
//            2,
//            VK_FORMAT_R32G32B32_SFLOAT,
//            sizeof(float) * 6);

    fVertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    fVertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(fVertices.bindingDescriptions.size());
    fVertices.inputState.pVertexBindingDescriptions = fVertices.bindingDescriptions.data();
    fVertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(fVertices.attributeDescriptions.size());
    fVertices.inputState.pVertexAttributeDescriptions = fVertices.attributeDescriptions.data();

}

// this should only need to be called once
void VulkanMeshPipeline::setupLayoutsAndPipeline(const std::string& shadersPath, VkRenderPass renderPass, VkPipelineCache pipelineCache)
{
    setupDescriptorSetLayout();
    setupVertexDescriptions();
    createPipeline(shadersPath, renderPass, pipelineCache);
}

// this must be called every time the meshes change
void VulkanMeshPipeline::setupDescripterSets(VkDescriptorPool pool)
{
    for (auto& mesh :fMeshes)
    {
        mesh->setupDescriptorSet(pool, fDescriptorSetLayout);
    }
}

void VulkanMeshPipeline::setMeshColor(int32_t meshID, const glm::vec3& color)
{
    std::cout << "setting mess color [" << color[0] << "," << color[1] << ","
    << color[2] << "]" << std::endl;
    for (auto& mesh :fMeshes)
    {
        if (mesh->getMeshID() == meshID)
        {
            std::cout << "color set" << std::endl;
            mesh->setColor(color);
            break;
        }
    }
}





VkPipelineShaderStageCreateInfo VulkanMeshPipeline::loadShader(std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), fDevice);
#endif
    shaderStage.pName = "main";
    assert(shaderStage.module != VK_NULL_HANDLE);
    fShaderModules.push_back(shaderStage.module);
    return shaderStage;

}



