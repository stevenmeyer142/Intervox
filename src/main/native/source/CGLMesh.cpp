// CGLMesh.cp
// Created by Steve on Fri, Jun 23, 2000 @ 12:16 PM.

#include "NativeOpenGL.h"
#ifndef __CGLMesh__
#include "CGLMesh.h"
#endif

#define GL_EXT_compiled_vertex_array 1
const bool kAutoNormal = false;
const bool kCompile = false;


#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include "utility/CTriangleList.h"
#include "utility/CVertexList.h"
#include "utility/CMyError.h"
#include "utility/CVertex.h"
#include "utility/CTriangle.h"


CGLMesh::CGLMesh() : fIndices(NULL), fNormals(NULL), fVertices(NULL), fTrianglesCount(0), fVerticesCount(0), 
		fUseNormals(true) 
{
}


CGLMesh::~CGLMesh()
{
	ReleaseHandles ();
}





#pragma segment Main
void CGLMesh::Render()
{
	if (fIndices != NULL && fVertices != NULL && fNormals)
	{
		CMyError::CheckForGLError (false, true);
//		::HLock(fVertices);
	
//		::HLock(fNormals);
//		::HLock(fIndices);

		
	  	::glPushAttrib(GL_ENABLE_BIT | GL_EVAL_BIT);

		if (kAutoNormal)
		{
		  	::glEnable(GL_AUTO_NORMAL);
		  	::glEnable(GL_NORMALIZE);
		}
		if (fUseNormals)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		CMyError::CheckForGLError (false, true);
		
		glVertexPointer(3, GL_FLOAT, 0, fVertices);
		if (fUseNormals)
		{
			glNormalPointer(GL_FLOAT, 0, fNormals);
		}
#if kCompile
	   		::glLockArraysEXT (0, fVerticesCount);
#endif

		glDrawElements(GL_TRIANGLES, fTrianglesCount * 3, GL_UNSIGNED_INT, fIndices);

		::glFinish();	//	do this before unlocking handles
	CMyError::CheckForGLError (false, true); 		

#if kCompile

	    	::glUnlockArraysEXT ();

#endif
	CMyError::CheckForGLError (false, true); 		
		
		
 		CMyError::CheckForGLError (false, true);

		if (fUseNormals)
		{
			glDisableClientState(GL_NORMAL_ARRAY);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
	
  		::glPopAttrib();
 

		CMyError::CheckForGLError (false, true); 		
//		::HUnlock(fVertices);
//		::HUnlock(fNormals);
//		::HUnlock(fIndices);

	} 
}


#pragma segment Main
void CGLMesh::AddTriangles(CTriangleList &triangles, CVertexList &vertices)
{
	try 
	{
		fTrianglesCount = 0;
		
		if (fIndices != NULL || fNormals != NULL || fVertices != NULL)
		{
			CMyError::DebugMessage ("CGLMesh::AddTriangles- Handles already created.");
			ReleaseHandles ();
		}
		
		CGLMIndex verticesListCount = vertices.GetSize(); 
		CGLMIndex trianglesListCount = triangles.GetSize(); 
		fVerticesCount = verticesListCount;
		fTrianglesCount = 0;
		
		fNormals = ::malloc(verticesListCount * 3 * sizeof(CGLMCoord));
		CMyError::ThrowErrorIfNULL(fNormals, "Couldn't allocate temp handle");
		fVertices = ::malloc(verticesListCount * 3 * sizeof(CGLMCoord));
		CMyError::ThrowErrorIfNULL(fVertices, "Couldn't allocate temp handle");
		fIndices = ::malloc(trianglesListCount * 3 * sizeof(CGLMIndex));
		CMyError::ThrowErrorIfNULL(fIndices, "Couldn't allocate temp handle");
		
		// add Failure Handling here
		
		if (fNormals != NULL && fVertices != NULL && fIndices != NULL)
		{
//			::HLock(fNormals);				
//			::HLock(fVertices);	
			
			CGLMCoord *normalsPtr = (CGLMCoord*)fNormals;			
			CGLMCoord *verticesPtr = (CGLMCoord*)fVertices;
			
			CVertex* vertexClass; 
			for (CGLMCoord i = 1; i <= verticesListCount; i++)
			{
				vertexClass = (CVertex*)vertices.At((long)i);
				vertexClass->GetXYZ(verticesPtr);
				vertexClass->GetNormalXYZ(normalsPtr);
				if (false)
				{
					for (short i = 0; i < 3; i++)
					{
						normalsPtr[i] = -normalsPtr[i];
					}
				}
				normalsPtr += 3;
	 			verticesPtr += 3;
			}		
						
//			::HUnlock(fNormals);				
//			::HUnlock(fVertices);	
	
	
//			::HLock(fIndices);	
			
			CGLMIndex* indiciesPtr = (CGLMIndex*)fIndices;
			CTriangle* triangleClass;
			
			for (CGLMCoord i = 1; i <= trianglesListCount; i++)
			{
				triangleClass = (CTriangle*)triangles.At((long)i);
				triangleClass->GetIndices(indiciesPtr);
				
				if (indiciesPtr[0] < verticesListCount && indiciesPtr[1] < verticesListCount
						&& indiciesPtr[2] < verticesListCount)
				{
					indiciesPtr +=3;
					fTrianglesCount++;
				}
			}		
						
//			::HUnlock(fIndices);				
		}
	}
	catch (CMyError err)
	{
#if 0
		if (fIndices != NULL)
		{
			::HUnlock(fIndices);						
		}
		if (fVertices != NULL)
		{
			::HUnlock(fVertices);						
		}
		if (fNormals != NULL)
		{
			::HUnlock(fNormals);						
		}
#endif		
		throw;
	}
	
}

#pragma segment Main
void CGLMesh::ReleaseHandles()
{
	if (fIndices != NULL)
	{
		::free(fIndices);
		fIndices = NULL;
	}

	if (fNormals != NULL)
	{
		::free(fNormals);
		fNormals = NULL;
	}

	if (fVertices != NULL)
	{
		::free(fVertices);
		fVertices = NULL;
	}
}

#pragma segment Main
void CGLMesh::DebugTestDraw()
{
	
}

#pragma segment Main
void CGLMesh::DebugLookAtTriangles()
{
	CGLMIndex* indiciesPtr = (CGLMIndex*)fIndices;
	CGLMCoord *verticesPtr = (CGLMCoord*)fVertices;
	for (int i = 0; i < fTrianglesCount; i++)
	{
		CGLMCoord *vertex1 = verticesPtr + 3 * indiciesPtr[i * 3];
		CGLMCoord *vertex2 = verticesPtr + 3 * indiciesPtr[i * 3 + 1];
		CGLMCoord *vertex3 = verticesPtr + 3 * indiciesPtr[i * 3 + 2];
		
			//	just something to allow me to stop debugger
		if (vertex1 == vertex2 || vertex2 == vertex3)
		{
			CMyError::DebugMessage ("Shit");
		}
	
	}
}

#pragma segment Main
void CGLMesh::DebugDrawNormals()
{
	if (fIndices != NULL && fVertices != NULL && fNormals != NULL)
	{
		float start[3] = {0, 0, 0};
		float end[3] = { 200, 200, 200 };
		float color[4] = {0, 0, 0, 1};
		::glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT);
		::glEnable(GL_LINE_SMOOTH);
//		::glLineWidth(6);
		::glLineWidth(1);
		::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		
		::glBegin(GL_LINES);
		
		if (false)
		{
			::glVertex3fv(start);
			::glVertex3fv(end);
		}
		else
		{
			float lineEnd[3];
			for (long i = 0; i < fVerticesCount; i ++)
			{
				float *vertex = ((float*)fVertices) + i * 3;
				float *normal = ((float*)fNormals) + i * 3; 
				
				for (short j = 0; j < 3; j++)
				{
					lineEnd[j] = vertex[j] + 2 * normal[j];
				}
					
				glVertex3fv(vertex);
				glVertex3fv(lineEnd);
				}
		}
	
		::glEnd();
		::glPopAttrib();
		CMyError::CheckForGLError (false, true);

/*		CMyError::CheckForGLError (false, true);
		::HLock(fVertices);
	
		::HLock(fNormals);
		::HLock(fIndices);

		::glPushAttrib(GL_LINE_BIT | GL_LIGHTING_BIT);
		CMyError::CheckForGLError (false, true); 		
		::glEnable(GL_LINE_SMOOTH);
		CMyError::CheckForGLError (false, true); 		
		::glLineWidth(6);
		CMyError::CheckForGLError (false, true); 		
	//	::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, fColor);
		CMyError::CheckForGLError (false, true); 		
		
		::glBegin(GL_TRIANGLES);
		CMyError::CheckForGLError (false, true); 		
		float lineEnd[3];
		for (long i = 0; i < fVerticesCount; i ++)
		{
			float *vertex = ((float*)*fVertices) + i * 3;
			float *normal = ((float*)*fNormals) + i * 3; 
			
			for (short j = 0; i < 3; i++)
			{
				lineEnd[i] = vertex[i] + 20 * normal[i];
			}
				
			glVertex3fv(vertex);
CMyError::CheckForGLError (false, true); 		
			glVertex3fv(lineEnd);
			
CMyError::CheckForGLError (false, true); 		
		}
		::glEnd();
		::glPopAttrib();

		CMyError::CheckForGLError (false, true);

		::HUnlock(fVertices);
		::HUnlock(fNormals);
		::HUnlock(fIndices); */
	}	
}


