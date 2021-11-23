// CMyError.cp
// Created by Steve on Wed, Apr 14, 1999 @ 11:15 AM.

#include "NativeOpenGL.h"
#ifndef __CMyError__
#include "CMyError.h"
#endif

#include <cstring>
#if kOpenGL

#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
#endif
#include "JNICommon.h"
#define kUseCGL 1

CMyError::CMyError(long err, const char* message1, const char* message2) 
{
	//	need a non-zero error
	fError = err != 0 ? err : -1;
	fMessage[0] = '\0';
	if (message1 != NULL)
	{
		strncat(fMessage, message1, kErrMessageStrLength);
	}
	
	if (message2 != NULL)
	{
		strncat(fMessage, message2, kErrMessageStrLength - strlen(fMessage));
	}
}


CMyError::~CMyError() throw()
{
}

#pragma segment Main


void CMyError::CheckForGLError(bool toss, bool debugMessage)
{
}

#pragma segment 
void CMyError::Assert(bool condition, const char* message )
{
	if (!condition)
	{
		throw CMyError(0, "Failed assertion. ", message);
	}
	
}




void CMyError::DebugMessage(const char* message)
{
	if (gDebugging)
	{
//		unsigned char localString[256];
//		short length = strlen(message);
//
//		localString[0] = length < 256 ? length : 255;
//
//		memcpy(localString + 1, message, localString[0]);
		::printf(message);
	}
}



void CMyError::CheckForJNIException(JNIEnv *env, const char * message)
{
#if kNoACS
	jthrowable exception = env->ExceptionOccurred();
	if (exception != NULL)
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
		throw CMyError(1, "Java exception occurred");
	}
#endif
}




void CMyError::ThrowErrorIfOSErr(OSErr err, const char *message)
{
	if (err != 0)
	{
		throw CMyError(err, message);
	}
	
}


void CMyError::ThrowErrorIfNULL(void* ptr, const char *message)
{
	if (ptr == NULL)
	{
		throw CMyError(0, "NULL pointer", message);
	}
	
}

