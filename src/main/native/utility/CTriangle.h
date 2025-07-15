// CTriangle.h
// Created by Steve on Tue, Dec 22, 1998 @ 1:40 PM.

#ifndef __CTriangle__
#define __CTriangle__

#ifndef __CMySortedList__
#include "CMySortedList.h"
#endif

#ifndef __C3DFloatPoint__
#include "C3DFloatPoint.h"
#endif

#ifndef KPtrStaticList
#define KPtrStaticList 1
#endif

#include <stdlib.h> // For size_t

class CVertex;

const long kTriangleCode = 0x22AA11BB;

class CTriangle
{
#if KPtrStaticList
	static CMySortedList *gOldArrays;

#else
	static CMySortedList gOldArrays;
#endif
	static CTriangle *gCurrentArray;
	static long gCurrentArrayIndex;
	C3DFloatPoint fNormal;

	static CTriangle *GetNewInstance();

	//	debugging
	long fCode;

public:
	CVertex *fPoint1;
	CVertex *fPoint2;
	CVertex *fPoint3;

	CTriangle();

	virtual ~CTriangle();

	void AddVertex(float x, float y, float z /*, short scalableCoord */);

	void *operator new(size_t size); //	override

	void operator delete(void *ptr);

	static void Cleanup();

	void GetNormal(C3DFloatPoint &normal) { normal = fNormal; }

	static void CheckSanity(CTriangle *triangle);
	void ComputeNormal();

	void Scale(float factor);

	void GetIndices(CGLMIndex *indicesPtr);

	friend class CVertexList;
};

#endif
