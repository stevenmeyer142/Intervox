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
			fOffscreenRenderer->copyImageData_RGBA_8888((uint32_t*)toBuffer, width, height);
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
    if (NULL == fOffscreenRenderer)
    {
        CMyError::DebugMessage("CVulkanContext::SetRotation NULL == fOffscreenRenderer");
        return;
    }

    fOffscreenRenderer->rotate(xRot, yRot);
}

void CVulkanContext::Zoom(float factor)
{
    
}

void CVulkanContext::AbsoluteZoom(float zoom)
{
    
}

void CVulkanContext::addMesh(mesh_id_t meshID)
{
    fRenderSettings.fPipelineSettings.
}

void CVulkanContext::removeMesh(mesh_id_t meshID)
{
    
}

bool CVulkanContext::hasMesh(mesh_id_t meshID)
{
    
}


void CVulkanContext::forceCommandBufferRebuild()
{
    
}
