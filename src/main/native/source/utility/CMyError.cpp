// CMyError.cp
// Created by Steve on Wed, Apr 14, 1999 @ 11:15 AM.

#include "NativeOpenGL.h"
#ifndef __CMyError__
#include "CMyError.h"
#endif

#include <String.h>
#include <OpenGL/glu.h>
#include <OpenGL/gl.h>
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
#if 0
	GLenum errorCode = ::glGetError();
	const GLubyte *errString = NULL;
	if (errorCode != GL_NO_ERROR)
	{
		errString = ::gluErrorString(errorCode);
	} 
	
	if (errString)
	{
		if (debugMessage)
		{
			DebugMessage ((const char*)errString);
		}
		
		if (toss)
		{
			throw CMyError(errorCode, "gl Error:", (const char*)errString);
		}
	}
#endif
}

#pragma segment 
void CMyError::Assert(bool condition, const char* message )
{
	if (!condition)
	{
		throw CMyError(0, "Failed assertion. ", message);
	}
	
}



#pragma segment Main
#if !kOpenGL
void CMyError::ThrowErrorIf3DErr(TQ3Status theStatus, const char *message)
{
	if (theStatus != kQ3Success)
	{
		TQ3Error theError = Q3Error_Get(NULL);
		if (theError == kQ3ErrorMacintoshError)
			ThrowErrorIfOSErr (Q3MacintoshError_Get(NULL), message);
		else if (theError != kQ3ErrorNone)
			ThrowErrorIfOSErr (theError, message);	
	}
	
}
#endif

#pragma segment Main
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


#pragma segment Main
#if !kOpenGL
void CMyError::CheckFor3DErrorAndThrow()
{
	TQ3Error theError = Q3Error_Get(NULL);
	if (theError == kQ3ErrorMacintoshError)
		ThrowErrorIfOSErr (Q3MacintoshError_Get(NULL), NULL);
	else if (theError != kQ3ErrorNone)
		ThrowErrorIfOSErr (theError, NULL);	
	
}
#endif

#pragma segment Main
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




#pragma segment Main
void CMyError::ThrowErrorIfOSErr(OSErr err, const char *message)
{
	if (err != 0)
	{
		throw CMyError(err, message);
	}
	
}


#pragma segment Main
void CMyError::ThrowErrorIfNULL(void* ptr, const char *message)
{
	if (ptr == NULL)
	{
		throw CMyError(0, "NULL pointer", message);
	}
	
}

