//
//  IntervoxHeadlessVulkan.cpp
//  IntervoxLib
//
//  Created by Steven Meyer on 11/2/21.
//

#include "NativeOpenGL.h"
#include "IntervoxHeadlessVulkan.hpp"
#include "utility/CJavaArrSlicesSet.h"
#include "utility/C3DPoint.h"
#include "utility/CCubeMarcher.h"
#include "utility/CMyError.h"

static bool DEV_DEBUG = true;

#define VERTEX_BUFFER_BIND_ID 0

IntervoxHeadlessVulkan::IntervoxHeadlessVulkan() : VulkanExampleBase(ENABLE_VALIDATION)

{
    width = 256;
    height = 256;
    
  //   timerSpeed *= 0.25f;
    fImageData.resize(height * width * sizeof(uint32_t));

    initialize(256, 256);
}


IntervoxHeadlessVulkan::~IntervoxHeadlessVulkan()
{
    fPipelines.clear();
}


void IntervoxHeadlessVulkan::renderScene(RenderCommandSettings &renderCommandSettings)
{
    VkCommandBuffer drawCommandBuffer = getCommandBuffer(renderCommandSettings);
    if (renderCommandSettings.fPipelinesVersion != fPipelinesVersion)
    {
        buildCommandBuffers(renderCommandSettings, drawCommandBuffer);
    }
    updateUniformBuffers(renderCommandSettings);
    render(drawCommandBuffer);
    grabImage();
}

void IntervoxHeadlessVulkan::initialize(uint32_t aWidth, uint32_t aHeight)
{
    if (!fInitialized)
    {
        fInitialized = true;
        width = aWidth;
        height = aHeight;
        fImageData.resize(height * width * sizeof(uint32_t));
        
        initVulkan();
        prepare();
    }
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


void IntervoxHeadlessVulkan::buildCommandBuffers(RenderCommandSettings &renderCommandSettings, VkCommandBuffer drawCommandBuffer)
{
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

    // TODO  for now just one framebuffer, and multiple command buffers
     renderPassBeginInfo.framebuffer = frameBuffers[0];

    VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffer, &cmdBufInfo));

    vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
    vkCmdSetViewport(drawCommandBuffer, 0, 1, &viewport);

    VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);
    vkCmdSetScissor(drawCommandBuffer, 0, 1, &scissor);

    // TODO: draw pipelines in proper order
    for (auto& pipelinesPair : fPipelines)
    {
        pipelinesPair.second->Draw(drawCommandBuffer, renderCommandSettings);
    }

    vkCmdEndRenderPass(drawCommandBuffer);

    VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffer));
}


void IntervoxHeadlessVulkan::setupDescriptorPool()
{
    
    if (VK_NULL_HANDLE != descriptorPool)
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
    
    uint32_t uniformBufferCount = 0;

    for (auto& pipelinePair : fPipelines)
    {
        uniformBufferCount += pipelinePair.second->getUniformBufferCount();
    }
    
    uniformBufferCount = std::max(uniformBufferCount, 1u);

    std::vector<VkDescriptorPoolSize> poolSizes =
    {
        vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBufferCount),
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo =
        vks::initializers::descriptorPoolCreateInfo(
            static_cast<uint32_t>(poolSizes.size()),
            poolSizes.data(),
            uniformBufferCount);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));

}

void IntervoxHeadlessVulkan::updateUniformBuffers(RenderCommandSettings &renderCommandSettings)
{
    for (auto pipelinePair : fPipelines)
    {
        pipelinePair.second->updateUniformBuffer(renderCommandSettings.fCamera.matrices.perspective,  renderCommandSettings.fCamera.matrices.view);
    }
}

void IntervoxHeadlessVulkan::draw(VkCommandBuffer drawCommandBuffer)
{    VkSubmitInfo submitInfo = vks::initializers::submitInfo();
   // Command buffer to be submitted to the queue
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCommandBuffer;

    // Submit to queue
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
}

void IntervoxHeadlessVulkan::prepare()
{
    VulkanExampleBase::prepare();
    auto meshPipeLine = std::make_shared<VulkanMeshPipeline>(device);
    fPipelines[MESH_PIPELINE] = meshPipeLine;

  //  prepareVertices(false);
  //  prepareVertices(true);
    
    auto shadersPath = getShadersPath();
    for (auto& pipelinesPair : fPipelines)
    {
        pipelinesPair.second->setupLayoutsAndPipeline(shadersPath, renderPass, pipelineCache);
    }

    updateDescriptorLayouts();
  
    prepared = true;
}

void IntervoxHeadlessVulkan::updateDescriptorLayouts()
{
    setupDescriptorPool();
    for (auto& pipelinesPair : fPipelines)
    {
        pipelinesPair.second->setupDescripterSets(descriptorPool);
    }
}

void IntervoxHeadlessVulkan::render(VkCommandBuffer drawCommandBuffer)
{
    if (!prepared)
        return;
    vkDeviceWaitIdle(device);
    draw(drawCommandBuffer);
    vkDeviceWaitIdle(device);
 }


//void IntervoxHeadlessVulkan::viewChanged()
//{
//    updateUniformBuffers();
//}

void IntervoxHeadlessVulkan::grabImage()
{
    bool supportsBlit = true;

    // Check blit support for source and destination
    VkFormatProperties formatProps;

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    vkGetPhysicalDeviceFormatProperties(physicalDevice, getColorFormat(), &formatProps);
    if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
 //       std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
        supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &formatProps);
    if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
//        std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
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

 
#if 0 // debugging code
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



int32_t IntervoxHeadlessVulkan::addMeshForRegion(CJavaArrSlicesSet *slicesSet, int regionValue)
{
    
    C3DPoint startPoint(0, 0, 0);

    
    CCubeMarcher marcher(slicesSet, 2, regionValue, startPoint);
    
    GeometryInfo info;
    auto vulkanMesh = std::make_shared<VulkanMesh>(vulkanDevice);
    
    bool success = marcher.Get3DMesh(2, info, vulkanMesh, queue);
        
    if (success)
    {
      //  ComputeWeightedCenter(slicesSet, regionValue, vulkanMesh);
        getMeshPipeline()->addMesh(vulkanMesh);
        updateDescriptorLayouts();
        return vulkanMesh->getMeshID();  // this is only used as an id.
    }
    else
    {
        CMyError::DebugMessage("IntervoxHeadlessVulkan::addMeshForRegion failed to create mesh");
        return -1;
    }
}

void IntervoxHeadlessVulkan::setMeshColor(int32_t meshID, float red, float green, float blue)
{
    auto meshPipeline = getMeshPipeline();
    meshPipeline->setMeshColor(meshID, glm::vec3(red, green, blue));
}



void IntervoxHeadlessVulkan::ComputeWeightedCenter(CJavaArrSlicesSet *slicesSet, short region,
                                                   std::shared_ptr<VulkanMesh> mesh)
{
    // TODO change C3DPoint to glm::vec3
    C3DPoint center = C3DPoint(0, 0, 0);
    long weight = 0;
    
    long depth = slicesSet->GetDepth ();
    long width = slicesSet->GetWidth ();
    long height = slicesSet->GetHeight ();
    
    for (long z = 0; z < depth; z++)
    {
        for (long y = 0; y < height; y++)
        {
            for (long x = 0; x < width; x++)
            {
                if (slicesSet->GetPixelValue (x, y, z) & region)
                {
                    center += C3DPoint(x, y, z);
                    weight++;
                }
            }
        }
    }
    
    if (weight != 0)
    {
        center.fX /= weight;
        center.fY /= weight;
        center.fZ /= weight;
    }
    
    glm::vec3 vecCenter(center.fX, center.fY, center.fZ);
    mesh->setWeightedCenter(vecCenter);
    mesh->setWeight(weight);
    
    glm::mat4 model;
    model = glm::translate(model, -vecCenter);
    
    // TODO why is this -60?
    model = glm::scale(model, glm::vec3(-60));
    mesh->setModelMatrix(model);
}


VkCommandBuffer IntervoxHeadlessVulkan::getCommandBuffer(RenderCommandSettings &renderCommandSettings)
{
    auto iter = fContextCommandBuffers.find(renderCommandSettings.fContextID);
    
    VkCommandBuffer result = VK_NULL_HANDLE;
    if (iter == fContextCommandBuffers.end())
    {
        VkCommandBufferAllocateInfo cmdBufAllocateInfo =
            vks::initializers::commandBufferAllocateInfo(
                cmdPool,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, &result));
        fContextCommandBuffers[renderCommandSettings.fContextID] = result;
    }
    else
    {
        result = iter->second;
    }
    
    return result;
}

std::shared_ptr<VulkanMeshPipeline> IntervoxHeadlessVulkan::getMeshPipeline()
{
    return std::static_pointer_cast<VulkanMeshPipeline>(fPipelines[MESH_PIPELINE]);
}

