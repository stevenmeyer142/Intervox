// CVertex.cp
// Created by Steve on Tue, Dec 22, 1998 @ 2:42 PM.

#include "NativeOpenGL.h"
#include "CVertex.h"
#include "CTempMemPointer.h"
#include "CMyError.h"

const long kVertexArraySize = 10000;
#if KPtrStaticList
CMySortedList *CVertex::gOldArrays = NULL;
#else
CMySortedList CVertex::gOldArrays;
#endif
CVertex *CVertex::gCurrentArray = NULL;
long CVertex::gCurrentArrayIndex = kVertexArraySize;

CMySortedList *gVertexCache = NULL;

#if kUseFloat
CVertex::CVertex(float x, float y, float z /*, short scalableCoord */) : fX(x), fY(y), fZ(z),
																		 /*fScalableCoord (scalableCoord),*/ fCode(kVertexCode), fIndex(-1)

{
}

#else
CVertex::CVertex(long x, long y, long z /*, short scalableCoord */) : C3DPoint(x, y, z),
																	  fIndex(-1), /* fScalableCoord (scalableCoord), */ fCode(kVertexCode)
{
}
#endif

CVertex::~CVertex()
{
}

void *CVertex::operator new(size_t)
{
	return GetNewInstance();
}

CVertex *CVertex::GetNewInstance()
{
	if (gVertexCache != NULL && gVertexCache->GetSize() > 0)
	{
		return (CVertex *)gVertexCache->Pop();
	}
	else
	{
		if (gCurrentArrayIndex >= kVertexArraySize)
		{
			CTempMemPointer *tempMemPtr = new CTempMemPointer();
			tempMemPtr->Allocate(sizeof(CVertex) * kVertexArraySize);
			gCurrentArray = (CVertex *)tempMemPtr->GetPointer();
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
}

void CVertex::operator delete(void *obj)
{

	if (gVertexCache == NULL)
	{
		gVertexCache = new CMySortedList();
	}

	gVertexCache->Push(obj);
}

void CVertex::Cleanup()
{
	gCurrentArray = NULL;
	gCurrentArrayIndex = kVertexArraySize;

	if (gVertexCache != NULL)
	{
		gVertexCache->RemoveAllItems();
	}

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

void CVertex::CheckSanity(CVertex *vertex)
{
	if (vertex->fCode != kVertexCode)
	{
		CMyError::DebugMessage("Vertex buggy");
	}
}

void CVertex::GetNormalXYZ(float *floatPtr)
{
	fNormal.Normalize();

	floatPtr[0] = fNormal.fX;
	floatPtr[1] = fNormal.fY;
	floatPtr[2] = fNormal.fZ;
}

void CVertex::GetXYZ(float *floatPtr)
{
	floatPtr[0] = fX;
	floatPtr[1] = fY;
	floatPtr[2] = fZ;
}

void CVertex::Scale(float factor)
{
	fX *= factor;
	fY *= factor;
	fZ *= factor;
}
