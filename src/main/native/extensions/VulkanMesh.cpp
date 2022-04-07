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

#include <iostream>

int32_t VulkanMesh::gNextMeshID = 1;

VulkanMesh::VulkanMesh(vks::VulkanDevice *aVulkanDevice)  :
    fVulkanDevice(aVulkanDevice)
    , fColor(1, 0, 0)
{
    fMeshID = gNextMeshID++;
}


VulkanMesh::~VulkanMesh()
{
	freeBuffers ();
}


void VulkanMesh::updateUniformBuffer(glm::mat4 perspective, glm::mat4 view)
{
    fUbo.projection = perspective;
    fUbo.view = view;
    fUbo.normal = glm::inverseTranspose(fUbo.view * fUbo.model);
    fUbo.lightPos = glm::vec3(0.0f, 2.5f, 2.5f);
    fUbo.color = glm::vec4(fColor, 1.0); //fColor;
    memcpy(fUniformBuffer.mapped, &fUbo, sizeof(fUbo));
}




void VulkanMesh::Draw(VkCommandBuffer cmdbuffer, VkPipelineLayout pipelineLayout)
{
    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &fDescriptorSet, 0, NULL);
    vkCmdBindVertexBuffers(cmdbuffer, 0, 1, &fVertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(cmdbuffer, fIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdbuffer, static_cast<uint32_t>(fIndexCount), 1, 0, 0, 1);
}

void VulkanMesh::setupDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout descriptorSetLayout)
{
    VkDescriptorSetAllocateInfo allocInfo =
        vks::initializers::descriptorSetAllocateInfo(
            pool,
            &descriptorSetLayout,
            1);

    VK_CHECK_RESULT(vkAllocateDescriptorSets(fVulkanDevice->logicalDevice, &allocInfo, &fDescriptorSet));

    // Binding 0 : Vertex shader uniform buffer
    VkWriteDescriptorSet writeDescriptorSet =
        vks::initializers::writeDescriptorSet(
            fDescriptorSet,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            0,
            &fUniformBuffer.descriptor);

    vkUpdateDescriptorSets(fVulkanDevice->logicalDevice, 1, &writeDescriptorSet, 0, NULL);

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
        vBuffer.push_back(MeshVertex(vertex, normal));
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

void VulkanMesh::addVertexData(std::vector<MeshVertex>& vBuffer, std::vector<uint32_t>& iBuffer,
                               VkQueue queue)
{
  
    size_t vertexBufferSize = vBuffer.size() * sizeof(vBuffer[0]);
    size_t indexBufferSize = iBuffer.size() * sizeof(iBuffer[0]);
    fIndexCount = iBuffer.size();

    size_t alignedVertexBufferSize = fVulkanDevice->getAlignedMemory(vertexBufferSize);
    
    // add dummy vertices to have greater than aligned size
    if (vertexBufferSize != alignedVertexBufferSize)
    {
        size_t verticesToAdd = ((alignedVertexBufferSize - vertexBufferSize) / sizeof(vBuffer[0])) + 1;
        vBuffer.insert(vBuffer.end(), verticesToAdd, MeshVertex());
    }
    size_t alignedIndexBufferSize = fVulkanDevice->getAlignedMemory(indexBufferSize);

    if (indexBufferSize != alignedIndexBufferSize)
    {
        size_t verticesToAdd = ((alignedIndexBufferSize - indexBufferSize) / sizeof(iBuffer[0])) + 1;
        iBuffer.insert(iBuffer.end(), verticesToAdd, 0);
    }

    bool useStaging = true;

    if (useStaging)
    {
        vks::Buffer vertexStaging, indexStaging;

        // Create staging buffers
        // Vertex data
            fVulkanDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &vertexStaging,
            alignedVertexBufferSize,
            vBuffer.data());
        // Index data
        fVulkanDevice->createBuffer(
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &indexStaging,
            alignedIndexBufferSize,
            iBuffer.data());

        // Create device local buffers
        // Vertex buffer
         fVulkanDevice->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &fVertexBuffer,
            alignedVertexBufferSize);
        // Index buffer
        fVulkanDevice->createBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &fIndexBuffer,
            alignedIndexBufferSize);

        // Copy from staging buffers
        VkCommandBuffer copyCmd = fVulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkBufferCopy copyRegion = {};

        copyRegion.size = vertexBufferSize;
        vkCmdCopyBuffer(
            copyCmd,
            vertexStaging.buffer,
            fVertexBuffer.buffer,
            1,
            &copyRegion);

        copyRegion.size = alignedIndexBufferSize;
        vkCmdCopyBuffer(
            copyCmd,
            indexStaging.buffer,
            fIndexBuffer.buffer,
            1,
            &copyRegion);

        fVulkanDevice->flushCommandBuffer(copyCmd, queue, true);

        vkDestroyBuffer(fVulkanDevice->logicalDevice, vertexStaging.buffer, nullptr);
        vkFreeMemory(fVulkanDevice->logicalDevice, vertexStaging.memory, nullptr);
        vkDestroyBuffer(fVulkanDevice->logicalDevice, indexStaging.buffer, nullptr);
        vkFreeMemory(fVulkanDevice->logicalDevice, indexStaging.memory, nullptr);
    }
    else
    {
        // Vertex buffer
        fVulkanDevice->createBuffer(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &fVertexBuffer,
            alignedVertexBufferSize,
            vBuffer.data());
        // Index buffer
        fVulkanDevice->createBuffer(
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            &fIndexBuffer,
            indexBufferSize,
            iBuffer.data());
    }


    prepareUniformBuffer();

}

void VulkanMesh::freeBuffers()
{
    
    fUniformBuffer.destroy();
    fVertexBuffer.destroy();
    fIndexBuffer.destroy();

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
    VK_CHECK_RESULT(fVulkanDevice->createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &fUniformBuffer,
        sizeof(fUbo)));
    // Map persistent
    VK_CHECK_RESULT(fUniformBuffer.map());
}


