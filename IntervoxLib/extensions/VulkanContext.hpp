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


class IntervoxHeadlessVulkan;

class CVulkanContext {
public:
	CVulkanContext();

    virtual ~CVulkanContext();
    
	void initialize(const Rect *rect);

	void FillInJavaRGBArray(JNIEnv *env, jintArray array, long width, long height);

    void Rotate(float xRot, float yRot);

    void Zoom(float factor);
    
    void AbsoluteZoom(float zoom);

    void CreateGeometries(JNIEnv *env, jint width, jint height, jobjectArray objArrays,
                        jint regionValue, jint geomID, int resolution);
    
private:
	IntervoxHeadlessVulkan *fOffscreenRenderer = NULL;
};

#endif /* CVULKANCONTEXT_H_ */
