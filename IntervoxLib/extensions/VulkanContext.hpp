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

	void initialize(const Rect *rect);

	void FillInJavaRGBArray(JNIEnv *env, jintArray array, long width, long height);

	virtual ~CVulkanContext();

private:
	IntervoxHeadlessVulkan *fOffscreenRenderer = NULL;
};

#endif /* CVULKANCONTEXT_H_ */
