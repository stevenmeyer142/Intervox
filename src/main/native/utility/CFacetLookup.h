// CFacetLookup.h
// Created by Steve on Mon, Dec 21, 1998 @ 1:53 PM.

#ifndef __CFacetLookup__
#define __CFacetLookup__

const short kMaxTriangles = 5;

class CTriangle;
class CCubeMarcher;

class CFacetLookup
{
public:
	CFacetLookup();

	static void GetTriangles(short index, CTriangle **data,
							 float x, float y, float z, float resolution, CCubeMarcher *marcher, bool gradate);

	virtual ~CFacetLookup();

#ifndef kNoACS
	static void WriteCubeStuff(CStream_AC &stream);
#endif
};

#endif
