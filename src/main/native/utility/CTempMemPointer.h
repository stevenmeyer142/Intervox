// CTempMemPointer.h
// Created by Steve on Wed, Mar 24, 1999 @ 12:24 PM.

#ifndef __CTempMemPointer__
#define __CTempMemPointer__

// #include <Carbon/Carbon.h>

#include <stddef.h>

class CTempMemPointer
{
	void *fPtr;

public:
	CTempMemPointer();

	void Allocate(size_t itsSize);

	void *GetPointer();

	void Release();

	virtual ~CTempMemPointer();
};

#endif
