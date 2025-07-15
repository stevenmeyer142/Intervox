// CVertex.h
// Created by Steve on Tue, Dec 22, 1998 @ 2:42 PM.

#ifndef __CVertex__
#define __CVertex__

#include "CMySortedList.h"
#include "C3DFloatPoint.h"
#include "C3DPoint.h"

#define KPtrStaticList 1

#include <stdlib.h> // For size_t

const short kScaleX = 1;
const short kScaleY = 2;
const short kScaleZ = 4;

const long kVertexCode = 0xEE11EE11;
#define kUseFloat true

// TODO change allocators
#if kUseFloat
class CVertex
{
public:
	float fX;
	float fY;
	float fZ;

private:
#else
class CVertex : public C3DPoint
{
#endif
#if KPtrStaticList
	static CMySortedList *gOldArrays;

#else
	static CMySortedList gOldArrays;
#endif
	static CVertex *gCurrentArray;
	static long gCurrentArrayIndex;

	static CVertex *GetNewInstance();

	//	short	fScalableCoord;

	long fCode;
	C3DFloatPoint fNormal;

public:
	ListIndex fIndex;

#if kUseFloat
	CVertex(float x, float y, float z /*, short scalableCoord */);
#else
	CVertex(long x, long y, long z /*, short scalableCoord */);
#endif
	virtual ~CVertex();

	void SetIndex(ListIndex index) { fIndex = index; }

	void *operator new(size_t); //	override

	void operator delete(void *ptr);

	static void Cleanup();

	long GetIndex() { return fIndex; }

	void GetPoint(C3DFloatPoint &pt)
	{
		pt.fX = fX;
		pt.fY = fY;
		pt.fZ = fZ;
	}

	void AddNormal(const C3DFloatPoint &normal) { fNormal += normal; }

	void GetNormal(C3DFloatPoint &normal)
	{
		fNormal.Normalize();
		normal = fNormal;
	}

	static void CheckSanity(CVertex *vertex);

	void GetXYZ(float *floatPtr);

	void GetNormalXYZ(float *floatPtr);

	void Scale(float factor);
};

#endif
