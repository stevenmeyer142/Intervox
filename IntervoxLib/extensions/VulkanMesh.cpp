// VulkanMesh.cp
// Created by Steve on Fri, Jun 23, 2000 @ 12:16 PM.

#include "NativeOpenGL.h"
#include "VulkanMesh.hpp"
#include "utility/CTriangleList.h"
#include "utility/CVertexList.h"
#include "utility/CMyError.h"
#include "utility/CVertex.h"
#include "utility/CTriangle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>



VulkanMesh::VulkanMesh(vks::VulkanDevice *aVulkanDevice)  :
    vulkanDevice(aVulkanDevice)
    
{
}


VulkanMesh::~VulkanMesh()
{
	freeBuffers ();
}


void VulkanMesh::updateUniformBuffer(glm::mat4 perspective, glm::mat4 view)
{
    ubo.projection = perspective;
    ubo.view = view;
    ubo.model = glm::mat4(1.0f);
    ubo.model = glm::translate(ubo.model, pos);
 //   ubo.model = glm::rotate(ubo.model, glm::radians((rotSpeed * timer) + rotOffset), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.normal = glm::inverseTranspose(ubo.view * ubo.model);
    ubo.lightPos = glm::vec3(0.0f, 0.0f, 2.5f);
    ubo.lightPos.x = 0.0f; //sin(glm::radians(timer)) * 8.0f;
    ubo.lightPos.z = 8.0f; // cos(glm::radians(timer)) * 8.0f;
 //   ubo.color = color; // TODO uncomment this
    memcpy(uniformBuffer.mapped, &ubo, sizeof(ubo));
}


void VulkanMesh::Draw(VkCommandBuffer cmdbuffer, VkPipelineLayout pipelineLayout)
{
    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
    vkCmdBindVertexBuffers(cmdbuffer, 0, 1, &vertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(cmdbuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdbuffer, indexCount, 1, 0, 0, 1);
}

void VulkanMesh::setupDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout)
{
    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            pool,
            &descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(vulkanDevice->logicalDevice, &allocInfo, &descriptorSet));

    // Binding 0 : Vertex shader uniform buffer
    VkWriteDescriptorSet writeDescriptorSet =
        vks::initializers::writeDescriptorSet(
            descriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &uniformBuffer.descriptor);

    vkUpdateDescriptorSets(vulkanDevice->logicalDevice, 1, &writeDescriptorSet, 0, NULL);

}

void VulkanMesh::AddTriangles(CTriangleList &triangles, CVertexList &vertices, VkQueue queue)
{
    CGLMIndex verticesListCount = vertices.GetSize();
    CGLMIndex trianglesListCount = triangles.GetSize();
    std::vector<MeshVertex> vBuffer;
    std::vector<uint32_t> iBuffer;

  //   CGLMCoord *normalsPtr =
    float vertex[3];
    float normal[3];
    
    CVertex* vertexClass;
    for (CGLMCoord i = 1; i <= verticesListCount; i++)
    {
        vertexClass = (CVertex*)vertices.At((long)i);
        vertexClass->GetXYZ(vertex);
        vertexClass->GetNormalXYZ(normal);
        vBuffer.push_back(MeshVertex(vertex, normal, color));
    }
                
     CTriangle* triangleClass;
    
    for (CGLMCoord i = 1; i <= trianglesListCount; i++)
    {
        CGLMIndex indicies[3];
        triangleClass = (CTriangle*)triangles.At((long)i);
        triangleClass->GetIndices(indicies);
        
        if (indicies[0] < verticesListCount && indicies[1] < verticesListCount
                && indicies[2] < verticesListCount)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                iBuffer.push_back(static_cast<uint32_t>(indicies[i]));
            }
        }
    }
    addVertexData(vBuffer, iBuffer, queue);
}

void VulkanMesh::addVertexData(std::vector<MeshVertex>& vBuffer,  std::vector<uint32_t>& iBuffer, VkQueue queue)
{
    size_t vertexBufferSize = vBuffer.size() * sizeof(MeshVertex);
    size_t indexBufferSize = iBuffer.size() * sizeof(uint32_t);

    bool useStaging = true;

    if (useStaging)
    {
        vks::Buffer vertexStaging, indexStaging;

        // Create staging buffers
        // Vertex data
        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &vertexStaging,
            vertexBufferSize,
            vBuffer.data());
        // Index data
        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &indexStaging,
            indexBufferSize,
            iBuffer.data());

        // Create device local buffers
        // Vertex buffer
        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &vertexBuffer,
            vertexBufferSize);
        // Index buffer
        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &indexBuffer,
            indexBufferSize);

        // Copy from staging buffers
        VkCommandBuffer copyCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkBufferCopy copyRegion = {};

        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(
            copyCmd,
            vertexStaging.buffer,
            vertexBuffer.buffer,
            1,
            &copyRegion);

        copyRegion.size = indexBufferSize;
        vkCmdCopyBuffer(
            copyCmd,
            indexStaging.buffer,
            indexBuffer.buffer,
            1,
            &copyRegion);

        vulkanDevice->flushCommandBuffer(copyCmd, queue, true);

        vkDestroyBuffer(vulkanDevice->logicalDevice, vertexStaging.buffer, nullptr);
        vkFreeMemory(vulkanDevice->logicalDevice, vertexStaging.memory, nullptr);
        vkDestroyBuffer(vulkanDevice->logicalDevice, indexStaging.buffer, nullptr);
        vkFreeMemory(vulkanDevice->logicalDevice, indexStaging.memory, nullptr);
    }
    else
    {
        // Vertex buffer
        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &vertexBuffer,
            vertexBufferSize,
            vBuffer.data());
        // Index buffer
        vulkanDevice->createBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &indexBuffer,
            indexBufferSize,
            iBuffer.data());
    }

    indexCount = iBuffer.size();

    prepareUniformBuffer();

}

void VulkanMesh::freeBuffers()
{
}

void VulkanMesh::DebugTestDraw()
{
	
}

void VulkanMesh::DebugLookAtTriangles()
{
#if 0
	CGLMIndex* indiciesPtr = (CGLMIndex*)fIndices;
	CGLMCoord *verticesPtr = (CGLMCoord*)fVertices;
	for (int i = 0; i < fTrianglesCount; i++)
	{
		CGLMCoord *vertex1 = verticesPtr + 3 * indiciesPtr[i * 3];
		CGLMCoord *vertex2 = verticesPtr + 3 * indiciesPtr[i * 3 + 1];
		CGLMCoord *vertex3 = verticesPtr + 3 * indiciesPtr[i * 3 + 2];
		
			//	just something to allow me to stop debugger
		if (vertex1 == vertex2 || vertex2 == vertex3)
		{
			CMyError::DebugMessage ("Shit");
		}
	
	}
#endif
}


void VulkanMesh::DebugDrawNormals()
{
#if 0
	if (fIndices != NULL && fVertices != NULL && fNormals != NULL)
	{
		float start[3] = {0, 0, 0};
		float end[3] = { 200, 200, 200 };
		float color[4] = {0, 0, 0, 1};
		::glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT);
		::glEnable(GL_LINE_SMOOTH);
//		::glLineWidth(6);
		::glLineWidth(1);
		::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		
		::glBegin(GL_LINES);
		
		if (false)
		{
			::glVertex3fv(start);
			::glVertex3fv(end);
		}
		else
		{
			float lineEnd[3];
			for (long i = 0; i < fVerticesCount; i ++)
			{
				float *vertex = ((float*)fVertices) + i * 3;
				float *normal = ((float*)fNormals) + i * 3; 
				
				for (short j = 0; j < 3; j++)
				{
					lineEnd[j] = vertex[j] + 2 * normal[j];
				}
					
				glVertex3fv(vertex);
				glVertex3fv(lineEnd);
				}
		}
	
		::glEnd();
		::glPopAttrib();
		CMyError::CheckForGLError (false, true);

/*		CMyError::CheckForGLError (false, true);
		::HLock(fVertices);
	
		::HLock(fNormals);
		::HLock(fIndices);

		::glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT);
		CMyError::CheckForGLError (false, true); 		
		::glEnable(GL_LINE_SMOOTH);
		CMyError::CheckForGLError (false, true); 		
		::glLineWidth(6);
		CMyError::CheckForGLError (false, true); 		
	//	::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, fColor);
		CMyError::CheckForGLError (false, true); 		
		
		::glBegin(GL_TRIANGLES);
		CMyError::CheckForGLError (false, true); 		
		float lineEnd[3];
		for (long i = 0; i < fVerticesCount; i ++)
		{
			float *vertex = ((float*)*fVertices) + i * 3;
			float *normal = ((float*)*fNormals) + i * 3; 
			
			for (short j = 0; i < 3; i++)
			{
				lineEnd[i] = vertex[i] + 20 * normal[i];
			}
				
			glVertex3fv(vertex);
CMyError::CheckForGLError (false, true); 		
			glVertex3fv(lineEnd);
			
CMyError::CheckForGLError (false, true); 		
		}
		::glEnd();
		::glPopAttrib();

		CMyError::CheckForGLError (false, true);

		::HUnlock(fVertices);
		::HUnlock(fNormals);
		::HUnlock(fIndices); */
	}
#endif
}

void VulkanMesh::prepareUniformBuffer()
{
    VK_CHECK_RESULT(vulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &uniformBuffer,
        sizeof(ubo)));
    // Map persistent
    VK_CHECK_RESULT(uniformBuffer.map());
}


