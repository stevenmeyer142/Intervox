/*
 * CVulkanContext.h
 *
 *  Created on: Feb 11, 2020
 *      Author: stevenmeyer
 */

#ifndef CVULKANCONTEXT_H_
#define CVULKANCONTEXT_H_

#include <stddef.h>
#include <jni.h>
#include "extensions/VulkanPipeline.hpp"


class IntervoxHeadlessVulkan;


class CVulkanContext {
public:
	CVulkanContext(IntervoxHeadlessVulkan * offscreenRenderer);

    virtual ~CVulkanContext();
    
	void initialize(const Rect *rect);

	void FillInJavaRGBArray(JNIEnv *env, jintArray array, long width, long height);

    void Rotate(float xRot, float yRot);

    void Zoom(float factor);
    
    void AbsoluteZoom(float zoom);
    
    void addMesh(int32_t meshID);
    
    void removeMesh(int32_t meshID);

private:
	IntervoxHeadlessVulkan *fOffscreenRenderer = NULL;
    RenderCommandSettings fRenderSettings;
};

#endif /* CVULKANCONTEXT_H_ */
