/*
 * CVulkanContext.cpp
 *
 *  Created on: Feb 11, 2020
 *      Author: stevenmeyer
 */

#include "NativeOpenGL.h"
#include "VulkanContext.hpp"
#include "IntervoxHeadlessVulkan.hpp"
#include "utility/CMyError.h"
#include "utility/CJavaArrSlicesSet.h"

const static bool DEBUG_MODULE = true;

CVulkanContext::CVulkanContext(IntervoxHeadlessVulkan * offscreenRenderer) :
    fOffscreenRenderer(offscreenRenderer)
{
    fRenderSettings.fCamera.flipY = true;
    fRenderSettings.fCamera.setPosition(glm::vec3(0.0f, 0.0f, -256.0f));
    fRenderSettings.fCamera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
//    fRenderSettings.fCamera.setOrthogonal(-(float)offscreenRenderer->getWidth(), (float)offscreenRenderer->getWidth(), (float)offscreenRenderer->getHeight(), -(float)offscreenRenderer->getHeight(), (float)offscreenRenderer->getHeight(), -(float)offscreenRenderer->getHeight());
    fRenderSettings.fCamera.setPerspective(60.0f, (float)offscreenRenderer->getWidth() / (float)offscreenRenderer->getHeight(), 0.001f, 512.0f);

}

CVulkanContext::~CVulkanContext() {
}

void CVulkanContext::initialize(const Rect *rect)
{
	fOffscreenRenderer->initialize(rect->right - rect->left, rect->bottom - rect->top);
}

void CVulkanContext::FillInJavaRGBArray(JNIEnv *env, jintArray array, long width, long height)
{
	if (NULL == fOffscreenRenderer)
	{
		CMyError::DebugMessage("CVulkanContext::FillInJavaRGBArray NULL == fOffscreenRenderer");
		return;
	}
	jboolean isCopy;
	jint *toBuffer = env->GetIntArrayElements(array, &isCopy);
	CMyError::CheckForJNIException(env);

	jsize size = env->GetArrayLength(array);
	CMyError::CheckForJNIException(env);
	if (size == width * height)
	{
		fOffscreenRenderer->renderScene(fRenderSettings);

		if (sizeof(*toBuffer) == sizeof(uint32_t))
		{
			fOffscreenRenderer->copyImageData_RGBA_8888((uint32_t*)toBuffer, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		}
		else
		{
			CMyError::DebugMessage("CVulkanContext::FillInJavaRGBArray sizeof(*toBuffer) != sizeof(uint32_t))");
		}
	}
	else
	{
		CMyError::DebugMessage("CVulkanContext::FillInJavaRGBArray size != width * height)");
	}

	env->ReleaseIntArrayElements(array, toBuffer, 0);
	CMyError::CheckForJNIException(env);
}


void CVulkanContext::Rotate(float xRot, float yRot)
{
    fRenderSettings.fRotation += glm::vec3(xRot, yRot, 0);
}

// TODO: is this function used?
void CVulkanContext::Zoom(float factor)
{
    std::cout << __FUNCTION__ << " factor " << factor << std::endl;
}

void CVulkanContext::AbsoluteZoom(float zoom)
{
    if (zoom != 0)
    {
        fRenderSettings.fCamera.mZoom = zoom;

        std::cout << __FUNCTION__ << " zoom " << zoom << ", fRenderSettings.fCamera.mZoom " << fRenderSettings.fCamera.mZoom << std::endl;
    }
    else{
        std::cerr << __FUNCTION__ << "Error zoom " << zoom << std::endl;
    }

}

void CVulkanContext::addMeshID(mesh_id_t meshID)
{
    fRenderSettings.fMeshPipelineSettings.addMeshID(meshID);
}

void CVulkanContext::removeMeshID(mesh_id_t meshID)
{
    fRenderSettings.fMeshPipelineSettings.removeMeshID(meshID);
}

bool CVulkanContext::hasMeshID(mesh_id_t meshID)
{
    return fRenderSettings.fMeshPipelineSettings.hasMeshID(meshID);
}


void CVulkanContext::forceCommandBufferRebuild()
{
    
}
