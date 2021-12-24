//
//  IntervoxHeadlessVulkan.cpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/2/21.
//

#include "IntervoxHeadlessVulkan.hpp"

static bool DEV_DEBUG = true;

#define VERTEX_BUFFER_BIND_ID 0

IntervoxHeadlessVulkan::IntervoxHeadlessVulkan() : VulkanExampleBase(ENABLE_VALIDATION)
{
    width = 256;
    height = 256;
#if GEARS
    title = "Vulkan gears";
    camera.type = Camera::CameraType::lookat;
    camera.setPosition(glm::vec3(0.0f, 2.5f, -16.0f));
    camera.setRotation(glm::vec3(-23.75f, 41.25f, 21.0f));
    camera.setPerspective(60.0f, (float)width / (float)height, 0.001f, 256.0f);
    timerSpeed *= 0.25f;
    fImageData.resize(height * width * sizeof(uint32_t));

#endif
}


IntervoxHeadlessVulkan::~IntervoxHeadlessVulkan()
{
#if GEARS
    // Clean up used Vulkan resources
    // Note : Inherited destructor cleans up resources stored in base class
    
#if !USE_MESH_PIPELINE
    vkDestroyPipeline(device, pipelines.solid, nullptr);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
#endif

    for (auto& gear : gears)
    {
        delete(gear);
    }
#endif
#if USE_MESH_PIPELINE
    fMeshPipelines.clear();
#endif
}


void IntervoxHeadlessVulkan::renderScene()
{
    render();
    grabImage();
}

void IntervoxHeadlessVulkan::initialize(uint32_t aWidth, uint32_t aHeight)
{
    width = aWidth;
    height = aHeight;
    fImageData.resize(height * width * sizeof(uint32_t));
    
    initVulkan();
    prepare();
}

void IntervoxHeadlessVulkan::copyImageData_RGBA_8888(uint32_t* toBuffer, uint32_t aWidth, uint32_t aHeight)
{
    if ((aWidth == width) && (aHeight == height) && (fImageData.size() == height * width * sizeof(*toBuffer)))
    {
        memcpy(toBuffer, fImageData.data(), height * width * sizeof(*toBuffer));
    }
    else
    {
        std::cerr << "Error IntervoxHeadlessVulkan::copyImageData_RGBA_8888 bad height and width" << std::endl;
    }
}    


void IntervoxHeadlessVulkan::buildCommandBuffers()
{
#if GEARS
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VkClearColorValue clearColor = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    VkClearValue clearValues[2];
 //   clearValues[0].color = defaultClearColor;
    clearValues[0].color = clearColor;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
    {
        renderPassBeginInfo.framebuffer = frameBuffers[i];

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
        vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
        vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

#if !USE_MESH_PIPELINE
        vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.solid);

        for (auto& gear : gears)
        {
            gear->draw(drawCmdBuffers[i], pipelineLayout);
        }
#else
        for (auto& pipeline : fMeshPipelines)
        {
            pipeline->Draw(drawCmdBuffers[i]);
        }
#endif

        vkCmdEndRenderPass(drawCmdBuffers[i]);

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
    }
#endif
}

void IntervoxHeadlessVulkan::prepareVertices()
{

#if GEARS
    // Gear definitions
    std::vector<float> innerRadiuses = { 1.0f, 0.5f, 1.3f };
    std::vector<float> outerRadiuses = { 4.0f, 2.0f, 2.0f };
    std::vector<float> widths = { 1.0f, 2.0f, 0.5f };
    std::vector<int32_t> toothCount = { 20, 10, 10 };
    std::vector<float> toothDepth = { 0.7f, 0.7f, 0.7f };
    std::vector<glm::vec3> colors = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.2f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };
    std::vector<glm::vec3> positions = {
        glm::vec3(-3.0, 0.0, 0.0),
        glm::vec3(3.1, 0.0, 0.0),
        glm::vec3(-3.1, -6.2, 0.0)
    };
    std::vector<float> rotationSpeeds = { 1.0f, -2.0f, -2.0f };
    std::vector<float> rotationOffsets = { 0.0f, -9.0f, -30.0f };

    gears.resize(positions.size());
    for (int32_t i = 0; i < gears.size(); ++i)
    {
        GearInfo gearInfo = {};
        gearInfo.innerRadius = innerRadiuses[i];
        gearInfo.outerRadius = outerRadiuses[i];
        gearInfo.width = widths[i];
        gearInfo.numTeeth = toothCount[i];
        gearInfo.toothDepth = toothDepth[i];
        gearInfo.color = colors[i];
        gearInfo.pos = positions[i];
        gearInfo.rotSpeed = rotationSpeeds[i];
        gearInfo.rotOffset = rotationOffsets[i];

        gears[i] = new VulkanGear(vulkanDevice);
#if USE_MESH_PIPELINE
        auto mesh = gears[i]->generate(&gearInfo, queue);
        fMeshPipelines[0]->addMesh(mesh);
#else
        gears[i]->generate(&gearInfo, queue);
#endif
    }

#if !USE_MESH_PIPELINE
    // Binding and attribute descriptions are shared across all gears
    vertices.bindingDescriptions.resize(1);
    vertices.bindingDescriptions[0] =
        vks::initializers::vertexInputBindingDescription(
            VERTEX_BUFFER_BIND_ID,
            sizeof(Vertex),
            VK_VERTEX_INPUT_RATE_VERTEX);

    // Attribute descriptions
    // Describes memory layout and shader positions
    vertices.attributeDescriptions.resize(3);
    // Location 0 : Position
    vertices.attributeDescriptions[0] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            0);
    // Location 1 : Normal
    vertices.attributeDescriptions[1] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            1,
            VK_FORMAT_R32G32B32_SFLOAT,
            sizeof(float) * 3);
    // Location 2 : Color
    vertices.attributeDescriptions[2] =
        vks::initializers::vertexInputAttributeDescription(
            VERTEX_BUFFER_BIND_ID,
            2,
            VK_FORMAT_R32G32B32_SFLOAT,
            sizeof(float) * 6);

    vertices.inputState = vks::initializers::pipelineVertexInputStateCreateInfo();
    vertices.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertices.bindingDescriptions.size());
    vertices.inputState.pVertexBindingDescriptions = vertices.bindingDescriptions.data();
    vertices.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertices.attributeDescriptions.size());
    vertices.inputState.pVertexAttributeDescriptions = vertices.attributeDescriptions.data();
#endif
#endif
}

void IntervoxHeadlessVulkan::setupDescriptorPool()
{
#if GEARS
    // TODO  class method for VulkanMeshPipeline to return pool sizes.
    // move descriptorsetlayout from mesh to pipeline
    
    // One UBO for each gear
    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3),
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        vks::initializers::descriptorPoolCreateInfo(
            static_cast<uint32_t>(poolSizes.size()),
            poolSizes.data(),
            // Three descriptor sets (for each gear)
            3);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
#endif
}

void IntervoxHeadlessVulkan::setupDescriptorSetLayout()
{
#if GEARS
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

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            &descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
#endif
}

void IntervoxHeadlessVulkan::setupDescriptorSets()
{
#if GEARS
    for (auto& gear : gears)
    {
        gear->setupDescriptorSet(descriptorPool, descriptorSetLayout);
    }
#endif
}

void IntervoxHeadlessVulkan::preparePipelines()
{
#if GEARS
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
        vks::initializers::pipelineInputAssemblyStateCreateInfo(
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            0,
            VK_FALSE);

    VkPipelineRasterizationStateCreateInfo rasterizationState =
        vks::initializers::pipelineRasterizationStateCreateInfo(
            VK_POLYGON_MODE_FILL,
            VK_CULL_MODE_BACK_BIT,
            VK_FRONT_FACE_CLOCKWISE,
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

    shaderStages[0] = loadShader(getShadersPath() + "gears/gears.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = loadShader(getShadersPath() + "gears/gears.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo =
        vks::initializers::pipelineCreateInfo(
            pipelineLayout,
            renderPass,
            0);

    pipelineCreateInfo.pVertexInputState = &vertices.inputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCreateInfo.pStages = shaderStages.data();

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipelines.solid));
#endif
}

void IntervoxHeadlessVulkan::updateUniformBuffers()
{
#if GEARS
#if !USE_MESH_PIPELINE
    for (auto& gear : gears)
    {
        gear->updateUniformBuffer(camera.matrices.perspective, camera.matrices.view, timer * 360.0f);
    }
#endif
#endif
#if USE_MESH_PIPELINE
    for (auto pipeline : fMeshPipelines)
    {
        pipeline->updateUniformBuffer(camera.matrices.perspective,  camera.matrices.view);
    }
#endif
}

void IntervoxHeadlessVulkan::draw()
{
#if GEARS
#ifndef INTERVOX_LIB
    VulkanExampleBase::prepareFrame();
#endif

    // Command buffer to be submitted to the queue
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

#ifndef INTERVOX_LIB
    VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo();
    VkFence fence;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceInfo, nullptr, &fence));
#endif
    // Submit to queue
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

#ifndef INTERVOX_LIB
   VulkanExampleBase::submitFrame();
#endif
    
#ifndef INTERVOX_LIB
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
    vkDestroyFence(device, fence, nullptr);
#endif

#endif
}

void IntervoxHeadlessVulkan::prepare()
{
    VulkanExampleBase::prepare();
#if USE_MESH_PIPELINE
    auto meshPipeLine = std::make_shared<VulkanMeshPipeline>(device);
    fMeshPipelines.push_back(meshPipeLine);
#endif

    prepareVertices();
#if !USE_MESH_PIPELINE
    setupDescriptorSetLayout();
    preparePipelines();
    setupDescriptorPool();
    setupDescriptorSets();
#else
    setupDescriptorPool();
    auto shadersPath = getShadersPath();
    for (auto& meshPipeLine : fMeshPipelines)
    {
        meshPipeLine->setupDescriptorsAndPipeline(shadersPath, renderPass, pipelineCache, descriptorPool);
    }
#endif
    updateUniformBuffers();
    buildCommandBuffers();
    prepared = true;
}

void IntervoxHeadlessVulkan::render()
{
    if (!prepared)
        return;
    vkDeviceWaitIdle(device);
    draw();
    vkDeviceWaitIdle(device);
    if (!paused)
    {
        updateUniformBuffers();
    }
}


void IntervoxHeadlessVulkan::viewChanged()
{
    updateUniformBuffers();
}

void IntervoxHeadlessVulkan::grabImage()
{
    bool supportsBlit = true;

    // Check blit support for source and destination
    VkFormatProperties formatProps;

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    vkGetPhysicalDeviceFormatProperties(physicalDevice, getColorFormat(), &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
        std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Source for the copy is the last rendered swapchain image
    VkImage srcImage = getImageAtIndex(currentBuffer);

    // Create the linear tiled destination image to copy to and to read the memory from
    VkImageCreateInfo imageCreateCI(vks::initializers::imageCreateInfo());
    imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
    // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
    imageCreateCI.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateCI.extent.width = width;
    imageCreateCI.extent.height = height;
    imageCreateCI.extent.depth = 1;
    imageCreateCI.arrayLayers = 1;
    imageCreateCI.mipLevels = 1;
    imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // Create the image
    VkImage dstImage;
    VK_CHECK_RESULT(vkCreateImage(device, &imageCreateCI, nullptr, &dstImage));
    // Create memory to back up the image
    VkMemoryRequirements memRequirements;
    VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
    VkDeviceMemory dstImageMemory;
    vkGetImageMemoryRequirements(device, dstImage, &memRequirements);
    memAllocInfo.allocationSize = memRequirements.size;
    // Memory must be host visible to copy from
    memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory));
    VK_CHECK_RESULT(vkBindImageMemory(device, dstImage, dstImageMemory, 0));

    // Do the actual blit from the swapchain image to our host visible destination image
    VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // Transition destination image to transfer destination layout
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        dstImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // Transition swapchain image from present to transfer source layout
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        srcImage,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
    if (supportsBlit)
    {
        // Define the region to blit (we will blit the whole swapchain image)
        VkOffset3D blitSize;
        blitSize.x = width;
        blitSize.y = height;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;

        // Issue the blit command
        vkCmdBlitImage(
            copyCmd,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            VK_FILTER_NEAREST);
    }
    else
    {
        // Otherwise use image copy (requires us to manually flip components)
        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.layerCount = 1;
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.layerCount = 1;
        imageCopyRegion.extent.width = width;
        imageCopyRegion.extent.height = height;
        imageCopyRegion.extent.depth = 1;

        // Issue the copy command
        vkCmdCopyImage(
            copyCmd,
            srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        dstImage,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // Transition back the swap chain image after the blit is done
    vks::tools::insertImageMemoryBarrier(
        copyCmd,
        srcImage,
        VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    vulkanDevice->flushCommandBuffer(copyCmd, queue);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
    VkSubresourceLayout subResourceLayout;
    vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    const char* data;
    vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
    data += subResourceLayout.offset;

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    bool colorSwizzle = false;
    // Check if source is BGR
    // Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
    if (!supportsBlit)
    {
        std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
        colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), getColorFormat()) != formatsBGR.end());
    }

 
#if 1  // debugging code
    std::ofstream file("saved_intevox.ppm", std::ios::out | std::ios::binary);

    // ppm header
    file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

    // ppm binary pixel data
    const char* debug_data = data;
    for (uint32_t y = 0; y < height; y++)
    {
        unsigned int *row = (unsigned int*)debug_data;
        for (uint32_t x = 0; x < width; x++)
        {
            if (colorSwizzle)
            {
                file.write((char*)row+2, 1);
                file.write((char*)row+1, 1);
                file.write((char*)row, 1);
            }
            else
            {
                file.write((char*)row, 3);
            }
            row++;
        }
        debug_data += subResourceLayout.rowPitch;
    }
    file.close();

    std::cout << "Screenshot saved to disk" << std::endl;
#endif

    for (uint32_t y = 0; y < height; y++)
    {
        const uint8_t *row = reinterpret_cast<const uint8_t*>( data);
        uint8_t* toRow = fImageData.data() + y * width * 4;
        for (uint32_t x = 0; x < width; x++)
        {
            // reverse order for correct rendering in java
            if (!colorSwizzle)
            {
                toRow[0] = row[2];
                toRow[1] = row[1];
                toRow[2] = row[0];
                toRow[3] = 0xFF;
            }
            else
            {
                toRow[0] = row[0];
                toRow[1] = row[0];
                toRow[2] = row[0];
                toRow[3] = 0xFF;

             }
            row += 4;
            toRow += 4;
        }
        data += subResourceLayout.rowPitch;
    }

    // Clean up resources
    vkUnmapMemory(device, dstImageMemory);
    vkFreeMemory(device, dstImageMemory, nullptr);
    vkDestroyImage(device, dstImage, nullptr);

}



