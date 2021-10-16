// CMyError.h
// Created by Steve on Wed, Apr 14, 1999 @ 11:15 AM.

#ifndef __CMyError__
#define __CMyError__


#include <stdexcept>

#include <JavaVM/JNI.h>
#include <OpenGL/OpenGL.h>


const short kErrMessageStrLength = 55;
typedef long OSErr;

class CMyError : public std::exception
{
public:
	CMyError(long err, const char* message1, const char* message2 = NULL);
	
	long GetErrorCode() { return fError; }
	
	char* GetMessage() { return fMessage; } 
	
	static void ThrowErrorIfNULL(void* ptr, const char *message = NULL);

	static void ThrowErrorIfOSErr(OSErr err, const char *message = NULL);
	
	static void CheckForJNIException(JNIEnv *env, const char * message = NULL);
	
	static void DebugMessage(const char* message);
	
	static void CheckForGLError(bool toss, bool debugMessage);
	
	static void Assert(bool condition, const char* message = NULL);
	
	virtual ~CMyError() throw();

private :
	char 	fMessage[kErrMessageStrLength + 1];
	long 	fError;
};

#endif
