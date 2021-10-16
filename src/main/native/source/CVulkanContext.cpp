/*
 * CVulkanContext.cpp
 *
 *  Created on: Feb 11, 2020
 *      Author: stevenmeyer
 */

#include "NativeOpenGL.h"
#include "CVulkanContext.h"
#include "Vulkan/VulkanOffscreenRenderer.hpp"
#include "utility/CMyError.h"

CVulkanContext::CVulkanContext() {


}

CVulkanContext::~CVulkanContext() {
	if (NULL != fOffscreenRenderer)
	{
		delete fOffscreenRenderer;
	}
}

void CVulkanContext::initialize(const Rect *rect)
{
	if (NULL == fOffscreenRenderer)
	{
		fOffscreenRenderer = new VulkanOffscreenRenderer();
	}
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
		fOffscreenRenderer->renderScene();

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


