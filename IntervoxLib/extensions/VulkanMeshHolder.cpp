// VulkanMeshHolder.cp
// Created by Steve on Fri, Jun 30, 2000 @ 10:42 AM.

#include "NativeOpenGL.h"
#include "VulkanMeshHolder.hpp"
#include "extensions/VulkanMesh.hpp"

#ifndef kNOJNI
#include "utility/CJavaArrSlicesSet.h"
#endif
#include "utility/CCubeMarcher.h"
#include "utility/C3DPoint.h"
#include "utility/CMyError.h"
#include "JNICommon.h"

const bool kUseList = false;
const bool kUseMaterial = true;

const float VulkanMeshHolder::kDefaultSpecularity = .5;
const float VulkanMeshHolder::kDefaultAmbience = .2;
const float VulkanMeshHolder::kDefaultDiffuseness = .6;
const float VulkanMeshHolder::kDefaultShininess = 10;


#define kNotAdded 0

#define kDebugSurface 0
#define kShowVectors 0

//const GLenum VulkanMeshHolder::kDefaultGLFace = GL_FRONT; //GL_FRONT_AND_BACK

VulkanMeshHolder::VulkanMeshHolder() : fGeomID(-1), /*fMaterialFace(kDefaultGLFace), */
#if kUseMeshRenderer
		 fOwnsMeshRenderer(false), fMeshRenderer(NULL), fSpecularity(kDefaultSpecularity), fAmbience(kDefaultAmbience), fDiffuseness(kDefaultDiffuseness), fShininess(kDefaultShininess)
#else
		fHighResMesh(NULL), /*fLowResMesh(NULL), */
		/* fLowResListID(0), fHighResListID(0),  */
		fCenter(0, 0, 0), fWeight(0), fSpecularity(kDefaultSpecularity), fAmbience(kDefaultAmbience), fDiffuseness(kDefaultDiffuseness)
#endif
{
	fColor[0] = 1.0;
	fColor[1] = 1.0;
	fColor[2] = 1.0;
	fColor[3] = 1.0;
}


VulkanMeshHolder::~VulkanMeshHolder()
{
	Deallocate ();
}

#pragma segment Main
void VulkanMeshHolder::Draw(VkCommandBuffer cmdbuffer, VkPipelineLayout pipelineLayout)
{
#if 0
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
#endif
}

jint VulkanMeshHolder::GetResolution()
{
#if kNotAdded
    return fMeshRenderer != NULL ? fMeshRenderer->GetHighResValue() : 0;
#else
    return 0;
#endif
}
	


#ifdef kUseJNI

void VulkanMeshHolder::CreateGeometries(JNIEnv *env, jint width, jint height, jobjectArray objArrays,
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
		
#if kNotAdded
		fLowResMesh = marcher.GetGLMesh(6, info);
#endif
		short fineResolution = 2;
		if (info.numOfVertices > 12000)	
		{
			fineResolution = 4;
		}
		else if (info.numOfVertices > 7000)		
		{
			fineResolution = 3;
		}
		
#if kNotAdded
		fHighResMesh = marcher.GetGLMesh(fineResolution, info);
#endif
		
		if (true)
		{
			ComputeWeightedCenter (slicesSet, regionValue);
		}
#endif
}
#endif


#pragma segment Main
void VulkanMeshHolder::DebugTestDraw()
{
#if kNotAdded
	float color[4] = { 0.0, 0.0, 1.0, 1.0 };
	::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	VulkanMesh::DebugTestDraw();
#endif
}


#pragma segment 
void VulkanMeshHolder::SetGeometryColor(float red, float green, float blue)
{
	fColor[0] = red;
	fColor[1] = green;
	fColor[2] = blue;
}

#if !kUseMeshRenderer
#pragma segment 
void VulkanMeshHolder::DebugShowNormals(bool lowRes)
{
	if (kShowVectors)
	{
#if kNotAdded

		if (lowRes && fLowResMesh)
		{
			fLowResMesh->DebugDrawNormals ();
		}
		else
#endif
            if (fHighResMesh)
		{
			fHighResMesh->DebugDrawNormals ();
		}
	}
}
#endif

void VulkanMeshHolder::DebugDraw(bool lowRes)//Override
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
void VulkanMeshHolder::ComputeWeightedCenter(CSlicesSet &slicesSet, short region)
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
bool VulkanMeshHolder::GetCenter(C3DPoint &center, long &weight)
{
	bool result = false;
#if kNotAdded

	if (fMeshRenderer)
	{
		result = fMeshRenderer->GetCenter(center, weight);
		
	}
#endif
	return result;
}

#pragma segment 
void VulkanMeshHolder::Deallocate()
{
#if kNotAdded

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
#endif
}

#pragma segment Main
void VulkanMeshHolder::GrabGeometries(VulkanMeshHolder *other, jint geomID)
{
#if kNotAdded
	fGeomID = geomID;
	Deallocate ();
	
	fMeshRenderer = other->fMeshRenderer;
	fOwnsMeshRenderer = false;
#endif
}

void VulkanMeshHolder::SetMaterialProperties(float specularity, float ambience, float diffuseness, float shininess)
{
    fSpecularity = specularity;
    fAmbience = ambience;
    fDiffuseness = diffuseness;
	fShininess = shininess;
}

void VulkanMeshHolder::GetMaterialProperties(float& specularity, float& ambience, float& diffuseness, float& shininess)
{
    specularity = fSpecularity;
    ambience = fAmbience;
    diffuseness = fDiffuseness;
	shininess = fShininess;
}
	






