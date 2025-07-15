#ifndef __JNICommon__
#define __JNICommon__

#include <jni.h>

#if kOpenGL
#include <OpenGL/gl.h>
#else
typedef double GLfloat;
#endif

class CMyError;

bool JavaIntArrayToRect(JNIEnv *env, jintArray jRect, Rect &rect);
void JavaArrayToGLFloat(JNIEnv *env, jfloatArray, GLfloat *glfloat, int size);
void GLFloatToJavaArray(JNIEnv *env, GLfloat *glfloat, jfloatArray, int size);
void FillErrRecord(JNIEnv *env, CMyError &err, jobjectArray errRecord);
void SetErrorCodeAndMessage(JNIEnv *env, long code, char *message, jobjectArray errRecord, bool alert = true);

void DebugMessage(JNIEnv *env, char *message, jobject errRecord);

#endif
