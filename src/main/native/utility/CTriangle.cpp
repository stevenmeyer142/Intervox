// CTriangle.cp
// Created by Steve on Tue, Dec 22, 1998 @ 1:40 PM.

#include "NativeOpenGL.h"
#include "CTriangle.h"
#include "CVertex.h"

#include "CTempMemPointer.h"
#include "CMyError.h"

const long kTriangleArraySize = 10000;
#if KPtrStaticList
CMySortedList *CTriangle::gOldArrays = NULL;
#else
CMySortedList CTriangle::gOldArrays;
#endif
CTriangle *CTriangle::gCurrentArray = NULL;
long CTriangle::gCurrentArrayIndex = kTriangleArraySize;

long gTrianglesInitialized = 0;
long gTrianglesDeleted = 0;

CTriangle::CTriangle() : fCode(kTriangleCode), fPoint1(NULL), fPoint2(NULL), fPoint3(NULL)
{
}

CTriangle::~CTriangle()
{
}

#pragma segment Main
void CTriangle::AddVertex(float x, float y, float z /*, short scalableCoord */)
{
	if (!fPoint1)
	{
		fPoint1 = new CVertex(x, y, z /*, scalableCoord */);
	}
	else if (!fPoint2)
	{
		fPoint2 = new CVertex(x, y, z /*, scalableCoord */);
	}
	else if (!fPoint3)
	{
		fPoint3 = new CVertex(x, y, z /*, scalableCoord */);

		ComputeNormal();
	}
	else
	{
		//		ProgramBreak_AC("To many vertices.");
	}
}

CTriangle *CTriangle::GetNewInstance()
{
	if (gCurrentArrayIndex >= kTriangleArraySize)
	{
		CTempMemPointer *tempMemPtr = new CTempMemPointer();
		tempMemPtr->Allocate(sizeof(CTriangle) * kTriangleArraySize);
		gCurrentArray = (CTriangle *)tempMemPtr->GetPointer();
		gCurrentArrayIndex = 0;

#if KPtrStaticList
		if (gOldArrays == NULL)
		{
			gOldArrays = new CMySortedList();
		}

		gOldArrays->Push(tempMemPtr);

#else
		gOldArrays.Push(tempMemPtr);
#endif
	}

	return gCurrentArray + gCurrentArrayIndex++;
}

void CTriangle::operator delete(void *)
{
	gTrianglesDeleted++;
}

void *CTriangle::operator new(size_t)
{
	gTrianglesInitialized++;
	return GetNewInstance();
}

void CTriangle::Cleanup()
{
	gCurrentArray = NULL;
	gCurrentArrayIndex = kTriangleArraySize;

	gTrianglesDeleted = 0;
	gTrianglesInitialized = 0;

#if KPtrStaticList
	if (gOldArrays != NULL)
	{
		long size = gOldArrays->GetSize();

		for (long i = 1; i <= size; i++)
		{
			CTempMemPointer *tempMemPtr = (CTempMemPointer *)gOldArrays->At(i);
			delete tempMemPtr;
		}

		gOldArrays->RemoveAllItems();
	}
#else
	long size = gOldArrays.GetSize();
	if (size > 0)
	{
		for (long i = 1; i <= size; i++)
		{
			CTempMemPointer *tempMemPtr = (CTempMemPointer *)gOldArrays.At(i);
			delete tempMemPtr;
		}

		gOldArrays.RemoveAllItems();
	}
#endif
}

void CTriangle::CheckSanity(CTriangle *triangle)
{
	if (triangle->fCode != kTriangleCode)
	{
		CMyError::DebugMessage("Vertex buggy");
	}
}

void CTriangle::ComputeNormal()
{
	if (fPoint1 && fPoint2 && fPoint3)
	{
		C3DFloatPoint pt1, pt2, pt3;
		fPoint1->GetPoint(pt1);
		fPoint2->GetPoint(pt2);
		fPoint3->GetPoint(pt3);
		pt2 -= pt1;
		pt3 -= pt1;
		fNormal = pt2.CrossProduct(pt3);

		fNormal.Normalize();
	}
}

void CTriangle::GetIndices(CGLMIndex *indicesPtr)
{
	indicesPtr[0] = fPoint1->GetIndex();
	indicesPtr[1] = fPoint2->GetIndex();
	indicesPtr[2] = fPoint3->GetIndex();
}

void CTriangle::Scale(float factor)
{
	if (fPoint1)
	{
		fPoint1->Scale(factor);
	}
	if (fPoint2)
	{
		fPoint2->Scale(factor);
	}
	if (fPoint3)
	{
		fPoint3->Scale(factor);
	}
}
