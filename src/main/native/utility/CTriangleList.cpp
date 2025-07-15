// CTriangleList.cp
// Created by Steve on Thu, Dec 3, 1998 @ 1:03 PM.

#include "NativeOpenGL.h"
#ifndef __CTriangleList__
#include "CTriangleList.h"
#endif
#include "CTriangle.h"
#include "CVertex.h"
// #include "CQ3PolyhedronData.h"
// #include "QD3DGroup.h"

CTriangleList::CTriangleList(ListIndex initialSize) : CMySortedList(initialSize)
{
}

CTriangleList::~CTriangleList()
{
}

void CTriangleList::InsertTriangle(CTriangle *triangle)
{
	if (triangle->fPoint1 && triangle->fPoint2 && triangle->fPoint3)
	{

		Push(triangle);
	}
	else
	{
		//		ProgramBreak_AC("Problemos");
	}
}

#if kDebugTriangle
void *CTriangleList::At(ArrayIndex_AC index) // Override
{
	void *result = CMySortedList::At(index);

	CTriangle::CheckSanity((CTriangle *)result);

	return result;
}
#endif
