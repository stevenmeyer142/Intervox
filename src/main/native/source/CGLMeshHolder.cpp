// CGLMeshHolder.cp
// Created by Steve on Fri, Jun 30, 2000 @ 10:42 AM.

#include "NativeOpenGL.h"
#ifndef __CGLMeshHolder__
#include "CGLMeshHolder.h"
#endif

#include "CGLMesh.h"

#ifndef kNOJNI
#include "utility/CJavaArrSlicesSet.h"
#endif

#include "utility/CCubeMarcher.h"
#include "utility/C3DPoint.h"
#include "utility/CMyError.h"
#include "JNICommon.h"

#include "CGLMeshRenderer.h"

const bool kUseList = false;
const bool kUseMaterial = true;


#define kDebugSurface 0
#define kShowVectors 0

const GLenum CGLMeshHolder::kDefaultGLFace = GL_FRONT; //GL_FRONT_AND_BACK

CGLMeshHolder::CGLMeshHolder() : fGeomID(-1), fMaterialFace(kDefaultGLFace),
#if kUseMeshRenderer
		 fOwnsMeshRenderer(false), fMeshRenderer(NULL), fSpecularity(kDefaultSpecularity), fAmbience(kDefaultAmbience), fDiffuseness(kDefaultDiffuseness), fShininess(kDefaultShininess)
#else
		fHighResMesh(NULL), fLowResMesh(NULL),  
		fLowResListID(0), fHighResListID(0), 
		fCenter(0, 0, 0), fWeight(0), fSpecularity(kDefaultSpecularity), fAmbience(kDefaultAmbience), fDiffuseness(kDefaultDiffuseness)
#endif
{
	fColor[0] = 1.0;
	fColor[1] = 1.0;
	fColor[2] = 1.0;
	fColor[3] = 1.0;
}


CGLMeshHolder::~CGLMeshHolder()
{
	Deallocate ();
}

#pragma segment Main
void CGLMeshHolder::Render(bool lowRes)
{	
	if (kUseMaterial)	
	{
		GLfloat	adjustedColor[4];
		adjustedColor[3] = fColor[3];
		::glPushAttrib(GL_LIGHTING_BIT);
		for (int i = 0; i < 3; i++)
		{
			adjustedColor[i] = fColor[i] * fSpecularity;
		}
		::glMaterialfv(fMaterialFace, GL_SPECULAR, adjustedColor);
		adjustedColor[0] = fShininess;
		glMaterialfv(fMaterialFace, GL_SHININESS, adjustedColor);

		for (int i = 0; i < 3; i++)
		{
			adjustedColor[i] = fColor[i] * fAmbience;
		}
		::glMaterialfv(fMaterialFace, GL_AMBIENT, adjustedColor);
		for (int i = 0; i < 3; i++)
		{
			adjustedColor[i] = fColor[i] * fDiffuseness;
		}
		::glMaterialfv(fMaterialFace, GL_DIFFUSE, adjustedColor);
		
		::glFrontFace(GL_CW);
	}
	else// draw wireframe
	{
		::glPolygonMode(GL_FRONT, GL_LINE);
	}
	if (kDebugSurface && false)
	{
		float red[4] = {1, 0, 0, 1};
		float blue[4] = {0, 1, 0, 1};
		::glMaterialfv(GL_BACK, GL_EMISSION , red);	// debugging
		::glMaterialfv(GL_FRONT, GL_EMISSION , blue);	// debugging
	//	::glMaterialfv(fMaterialFace, GL_AMBIENT_AND_DIFFUSE, red);
	}
	 
#if kUseMeshRenderer
	
	if (fMeshRenderer)
	{
		fMeshRenderer->Render(lowRes);
	}	
	
#else	
	if (lowRes && fLowResMesh)
	{
		if (kUseList)
		{
			if (!::glIsList(fLowResListID))
			{
				fLowResListID = ::glGenLists(1);
				::glNewList(fLowResListID, GL_COMPILE);
				fLowResMesh->Render();
				::glEndList();
			}
			::glCallList(fLowResListID);
		}
		else
		{
			fLowResMesh->Render();
		}
	}
	else if (fHighResMesh)
	{
		if (kUseList)
		{
			if (!::glIsList(fHighResListID))
			{
				fHighResListID = ::glGenLists(1);
				::glNewList(fHighResListID, GL_COMPILE);
				fHighResMesh->Render();
				::glEndList();
			}
			::glCallList(fHighResListID);
		}
		else
		{
			fHighResMesh->Render();
		}
	}
#endif
	if (kUseMaterial)
	{
		::glPopAttrib();
	}
}

jint CGLMeshHolder::GetResolution() 
{ 
    return fMeshRenderer != NULL ? fMeshRenderer->GetHighResValue() : 0; 
}
	


#ifdef kUseJNI

void CGLMeshHolder::CreateGeometries(JNIEnv *env, jint width, jint height, jobjectArray objArrays, 
            jint regionValue, jint geomID, int resolution)
{
		fGeomID = geomID;

#if kUseMeshRenderer
		Deallocate ();
		
		fMeshRenderer = new CGLMeshRenderer();
		fMeshRenderer->CreateGeometries (env, width, height, objArrays, regionValue, resolution);
		fOwnsMeshRenderer = true;
#else	
		fWeight = 0;
		fCenter = C3DPoint(0, 0, 0);
		
		CJavaArrSlicesSet slicesSet(env, objArrays, width, height);
		
		C3DPoint startPoint(0, 0, 0);

		
		CCubeMarcher marcher(&slicesSet, 2, regionValue, startPoint);
		
		GeometryInfo info;
		
		fLowResMesh = marcher.GetGLMesh(6, info);
		
		short fineResolution = 2;
		if (info.numOfVertices > 12000)	
		{
			fineResolution = 4;
		}
		else if (info.numOfVertices > 7000)		
		{
			fineResolution = 3;
		}
		
		fHighResMesh = marcher.GetGLMesh(fineResolution, info); 
		
		if (true)
		{
			ComputeWeightedCenter (slicesSet, regionValue);
		}
#endif
}
#endif


#pragma segment Main
void CGLMeshHolder::DebugTestDraw()
{
	float color[4] = { 0.0, 0.0, 1.0, 1.0 };
	::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	CGLMesh::DebugTestDraw();
}


#pragma segment 
void CGLMeshHolder::SetGeometryColor(float red, float green, float blue)
{
	fColor[0] = red;
	fColor[1] = green;
	fColor[2] = blue;
}

#if !kUseMeshRenderer
#pragma segment 
void CGLMeshHolder::DebugShowNormals(bool lowRes)
{
	if (kShowVectors)
	{
		if (lowRes && fLowResMesh)
		{
			fLowResMesh->DebugDrawNormals ();
		}
		else if (fHighResMesh)
		{
			fHighResMesh->DebugDrawNormals ();
		}
	}
}
#endif

#pragma segment Main
void CGLMeshHolder::DebugDraw(bool lowRes)//Override
{
#if kUseMeshRenderer	
	if (fMeshRenderer)
	{
		fMeshRenderer->DebugDraw (lowRes);
	}
	
#else
	DebugShowNormals (lowRes);
#endif
}


#if !kUseMeshRenderer
#pragma segment Main
void CGLMeshHolder::ComputeWeightedCenter(CSlicesSet &slicesSet, short region)
{
	C3DPoint center = C3DPoint(0, 0, 0);
	long weight = 0;
	
	long depth = slicesSet.GetDepth ();
	long width = slicesSet.GetWidth ();
	long height = slicesSet.GetHeight ();
	
	for (long z = 0; z < depth; z++)
	{
		for (long y = 0; y < height; y++)
		{
			for (long x = 0; x < width; x++)
			{
				if (slicesSet. GetPixelValue (x, y, z) & region)
				{
					center += C3DPoint(x, y, z);
					weight++;
				}
			}
		}
	}
	
	if (weight != 0)
	{
		center.fX /= weight;
		center.fY /= weight;
		center.fZ /= weight;
	}
	
	fCenter = center;
	fWeight = weight;
}
#endif

#pragma segment 
bool CGLMeshHolder::GetCenter(C3DPoint &center, long &weight)
{
	bool result = false;
	if (fMeshRenderer)
	{
		result = fMeshRenderer->GetCenter(center, weight);
		
	}
	
	return result;
}

#pragma segment 
void CGLMeshHolder::Deallocate()
{
#if !kUseMeshRenderer
	if (::glIsList(fHighResListID))
	{
		::glDeleteLists(fHighResListID, 1);
	}
	
	if (::glIsList(fLowResListID))
	{
		::glDeleteLists(fLowResListID, 1);
	}
	
	if (fHighResMesh)
	{
		delete fHighResMesh;
		fHighResMesh = NULL;
	}

	if (fLowResMesh)
	{
		delete fLowResMesh;
		fLowResMesh = NULL;
	}
#else
	if (fOwnsMeshRenderer && fMeshRenderer)
	{
		if (gDebugging)
		{
			CMyError::DebugMessage("mesh renderer being deleted");
		}

		delete fMeshRenderer;
		fMeshRenderer = NULL;
	}
#endif
	
}

#pragma segment Main
void CGLMeshHolder::GrabGeometries(CGLMeshHolder *other, jint geomID)
{
	fGeomID = geomID;
	Deallocate ();
	
	fMeshRenderer = other->fMeshRenderer;
	fOwnsMeshRenderer = false;
}

void CGLMeshHolder::SetMaterialProperties(float specularity, float ambience, float diffuseness, float shininess)
{
    fSpecularity = specularity;
    fAmbience = ambience;
    fDiffuseness = diffuseness;
	fShininess = shininess;
}

void CGLMeshHolder::GetMaterialProperties(float& specularity, float& ambience, float& diffuseness, float& shininess)
{
    specularity = fSpecularity;
    ambience = fAmbience;
    diffuseness = fDiffuseness;
	shininess = fShininess;
}
	






