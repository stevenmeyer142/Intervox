// CTriangleList.h
// Created by Steve on Thu, Dec 3, 1998 @ 1:03 PM.

#ifndef __CTriangleList__
#define __CTriangleList__

#ifndef __CMySortedList__
#include "CMySortedList.h"
#endif

class CTriangle;
class CQ3PolyhedronData;

#define kDebugTriangle 0

class CTriangleList : public CMySortedList
{
public:
	CTriangleList(ListIndex initialSize);

	void InsertTriangle(CTriangle *triangle);

	virtual ~CTriangleList();

#if kDebugTriangle
	virtual void *At(ArrayIndex_AC index);
#endif
};

#endif
