// CSlicesSet.cp
// Created by Steve on Mon, Oct 26, 1998 @ 10:25 AM.

#include "NativeOpenGL.h"
#ifndef __CSlicesSet__
#include "CSlicesSet.h"
#endif

#include "C3DPoint.h"

const short CSlicesSet::kResolution = 4;

CSlicesSet::CSlicesSet() : fWidth(0), fHeight(0), fDepth(0)

{
}

CSlicesSet::~CSlicesSet()
{
}

#pragma segment Main
long CSlicesSet::GetPixelValue(long /*x */, long /*y */, long /*z */)
{
	return 0;
}
