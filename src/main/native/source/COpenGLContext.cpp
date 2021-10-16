// COpenGLContext.cp
// Created by Steve on Mon, Jun 12, 2000 @ 12:43 PM.

#include "NativeOpenGL.h"

#ifndef __COpenGLContext__
#include "COpenGLContext.h"
#endif

#include "utility/CMyError.h"
#include <OpenGL/glu.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include "CGLMeshHolder.h" 
#include "CGLPointer.h"
#include "CGLFiducials.h"
#include "utility/C3DFloatPoint.h"
//#include "CGLTexFont.h"
#include "JNICommon.h"
#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#endif

const short COpenGLContext::kPixelSize = 32;
//AGLContext COpenGLContext::fgSharedContext = NULL;

#define COMPILE_REMOVE 0

COpenGLContext::COpenGLContext() : 
#if !kUseFrameBuffer
fAGLContext(NULL), 
#endif
#if kMyUsePBuffer
		fAGLPBuffer(NULL),
#endif
#if kUseFrameBuffer
	fFramebuffer(0),
	fRenderbuffer(0),
#endif
#if kUseOffscreenPtr
			fScreenPtr(NULL),
#endif
fUseGWorld(false), fPointer(NULL),
		fFiducials(NULL), fFiducialsShown(false), fLowRes(false),
                fSpecularity(1), fAmbience(1), fDiffuseness(1)
{
	fClearColor[0] = 1.0f;
	fClearColor[1] = 1.0f;
	fClearColor[2] = 1.0f;
	fClearColor[3] = 1.0f;
        fLightDirection[0] = -.2;
        fLightDirection[1] = -.2;
        fLightDirection[2] = -1.0;
        fLightDirection[3] = 0.0;
	fPerspective.SetValues (0, 256, 0, 256, 500, -500);
	fPerspective.SetOrthogonal(true);
	
	SetTranslation(0, 0, 0);
	
	SetIdentityMatrix4(fMatrix);
}


COpenGLContext::~COpenGLContext()
{
	CleanUp ();
}

#pragma segment Main
#ifndef kNOJNI
void COpenGLContext::FillInJavaRGBArray(JNIEnv *env, jintArray array, long width, long height)
{
	if (fAGLContext != NULL)
	{
#if false
		Rect bounds;
		fGWorld.GetPixMapBounds(bounds);
		long *fromBuffer = (long*)fGWorld.GetBaseAddress ();
		short rowLong = fGWorld.GetRowBytes() / 4;	
		
		if (fromBuffer != NULL && bounds.right - bounds.left == width && bounds.bottom - bounds.top == height)
		{
			jboolean isCopy;
			jint *toBuffer = env->GetIntArrayElements(array, &isCopy);
			CMyError::CheckForJNIException(env);

			long toIndex = 0;
			long fromRowStart = 0;
			for (long y = 0; y < height; y++)
			{
				for (long x = 0; x < width; x++)
				{
						// make alpha 255
					toBuffer[toIndex++] = fromBuffer[fromRowStart + x] | 0xFF000000;
				}
				
				fromRowStart += rowLong;
			}
			env->ReleaseIntArrayElements(array, toBuffer, 0);
			CMyError::CheckForJNIException(env);

//			env->SetIntArrayRegion(array, 0,   width * height, buffer);
//			CMyError::CheckForJNIException(env);
		}
		else 
		{
			CMyError::DebugMessage ("Bad Arguments, QD3DPixmapDrawContext::FillInJavaRGBArray");
		}

#else
			jboolean isCopy;
			jint *toBuffer = env->GetIntArrayElements(array, &isCopy);
			CMyError::CheckForJNIException(env);
			
			jsize size = env->GetArrayLength(array);
			CMyError::CheckForJNIException(env);
			if (size * sizeof(jint) == width * height * sizeof(4 * GL_UNSIGNED_BYTE))
			{
				if (PrepareOffScreen())
				{
					BecomeCurrentContext();
					::glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,toBuffer); 
					CMyError::CheckForGLError (false, true);
					ReleaseOffScreen();
					
					for (long i = 0; i < size; i++)
					{
						toBuffer[i] = OSSwapHostToBigInt32(toBuffer[i]);
						toBuffer[i] = toBuffer[i] >> 8;
						toBuffer[i] |= 0xFF000000;
						
					}
				}
			}
			else
			{
				CMyError::DebugMessage ("Bad array length, QD3DPixmapDrawContext::FillInJavaRGBArray");
			}
			
			env->ReleaseIntArrayElements(array, toBuffer, 0);
			CMyError::CheckForJNIException(env);
	
#endif			
	}
}
#endif

#pragma segment Main
void COpenGLContext::TestDraw()
{
     CGLMeshHolder::DebugTestDraw();
			// glutSolidTeapot(100.0);
}
#if kUseFrameBuffer
bool COpenGLContext::setUpFrameBuffer(GLsizei width, GLsizei height)
{
	GLenum status;

	//Set up a FBO with one renderbuffer attachment
	glGenFramebuffersEXT(1, &fFramebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fFramebuffer);
	glGenRenderbuffersEXT(1, &fRenderbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fRenderbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								 GL_RENDERBUFFER_EXT, fRenderbuffer);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		result = false;
	}
	
	return result;
}
#endif


#pragma segment Main
CGLContextObj COpenGLContext::setupContextOffscreen(void *baseaddr, GLsizei width, GLsizei height, GLsizei rowbytes, GLsizei pixelsize)
{
#if !COMPILE_REMOVE
	//	error handling this shit
	int last_attribute = 6;
	CGLPixelFormatAttribute attribs[] =
	{
		kCGLPFAOffScreen,
	    kCGLPFADepthSize, (CGLPixelFormatAttribute)16,
		(CGLPixelFormatAttribute)0
	};

	CGLPixelFormatObj pixelFormatObj;
	GLint numPixelFormats;
	long value;

	CGLChoosePixelFormat (attribs, &pixelFormatObj, &numPixelFormats);

//	if( pixelFormatObj == NULL ) {
//	    attribs[last_attribute] = 0;
//	    CGLChoosePixelFormat (attribs, &pixelFormatObj, &numPixelFormats);
//	}

	if( pixelFormatObj == NULL ) {
	    // Your code to notify the user and take action.
		return NULL;
	}

	/* Create an OpenGL context */
	CGLContextObj ctx = NULL;
	CGLError err = CGLCreateContext(pixelFormatObj, NULL, &ctx);
	CGLDestroyPixelFormat(pixelFormatObj);
	if(err != NULL) {

		return NULL;
	}
/*	
	if (fgSharedContext == NULL)
	{
		fgSharedContext = ctx;
	} */

	/* Attach the off screen area to the context */
	err = CGLSetOffScreen(ctx, width, height, rowbytes, baseaddr);
	if(!err) {
		return NULL;
	}
	
	/* Make the context the current context */
	err =CGLSetCurrentContext(ctx);
	if(!err) return NULL;


	CMyError::CheckForGLError (false, true);
	
	return ctx;
#else
	return NULL;
#endif
}

#pragma segment Main
void COpenGLContext::CleanUp()
{
#if kUsePixelBuffer
	if (fAGLContext != NULL)
	{
		CGLContextObj ctx = fAGLContext;
		fAGLContext = NULL;
#if kPre105
#if !COMPILE_REMOVE
//		CGLSetDrawable(ctx, NULL);
#endif
#else
		aglSetWindowRef(ctx, NULL);
#endif
		CGLSetCurrentContext(NULL);
		CGLDestroyContext(ctx);
	}
#endif
#if kMyUsePBuffer
	if (fAGLPBuffer)
	{
		CGLDestroyPBuffer(fAGLPBuffer);
		fAGLPBuffer = NULL;
	}
#endif
	
#if kUseOffscreenPtr
	if (fScreenPtr)
	{
		free(fScreenPtr);
		fScreenPtr = NULL;
	}
#endif	
	if (fPointer != NULL)
	{
		delete fPointer;
		fPointer = NULL;
	}
	
	if (fFiducials != NULL)
	{
		delete fFiducials;
		fFiducials = NULL;
	}
}

#pragma segment Main
void COpenGLContext::SetRotation(float* matrix)
{
	for (short i = 0; i < kMatrixSize; i++)
	{
		fMatrix[i] = matrix[i];
	}
}


#pragma segment Main
void COpenGLContext::SetIdentityMatrix4(float* matrix)
{
	for (short col = 0; col < 4; col++)
	{
		for (short row = 0; row < 4; row++)
		{
			float value = (row == col) ? 1.0f : 0.0f;
			
			matrix[col * 4 + row] = value;
		}
		
	}
}

#pragma segment 
void COpenGLContext::AddRenderable(CGLRenderable* renderable)
{
	if (!GetRenderableWithID (renderable->GetGeomID()))
	{
		C3DPoint center;
		long weight;
		if (fRenderables.GetSize () == 0 && renderable->GetCenter (center, weight))
		{
			fPerspective.SetAbsoluteZoomFocus (center.fX, center.fY);
		}
		fRenderables.InsertLast (renderable);
	}
}

#pragma segment 
CGLRenderable* COpenGLContext::GetRenderableWithID(long geomID)
{
	 CGLRenderable* result = NULL;
	 
	long size = fRenderables.GetSize ();
	for (long i = 1; i <= size; i++)
	{
		CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(i);
		if (renderable->GetGeomID() == geomID)
		{
			result = renderable;
			break;
		}
	}
	
	return result;
}

bool COpenGLContext::PrepareOffScreen()
{
	bool result = false;
#if kUseOffscreenPtr
	if (fUseGWorld && fAGLContext != NULL && fScreenPtr)
	{
		result = aglSetOffScreen(fAGLContext, fWidth, fHeight, fRowBytes, fScreenPtr);
		CMyError::CheckForGLError (true, true);
	}	
#endif
#if kUseGWorld
	if (fUseGWorld && fAGLContext != NULL && fGWorld.LockPixels())
	{
		result = aglSetOffScreen(fAGLContext, fGWorld.GetWidth(), fGWorld.GetHeight(), fGWorld.GetRowBytes (), fGWorld.GetBaseAddress ());
		CMyError::CheckForGLError (true, true);
	}	
#endif
	return true;
//	return result;
}

void COpenGLContext::ReleaseOffScreen()
{
#if kUseOffscreenPtr
#if kPre105
	aglSetDrawable(ctx, NULL);
#else
	aglSetWindowRef(ctx, NULL);
#endif
#endif
#if kUseGWorld
	if (fUseGWorld)
	{
		if (fAGLContext != NULL)
		{
			::aglSetDrawable(fAGLContext, NULL);
		}
		
		fGWorld.UnlockPixels();
	}
#endif
}

#pragma segment Main
void COpenGLContext::PrepareToDraw()
{
#if !kUseFrameBuffer
	if(!PrepareOffScreen())
	{ 
		return;
	}	

	BecomeCurrentContext();
#endif
	
#if kUseFrameBuffer

#endif
	CMyError::CheckForGLError (false, true);
  	::glClearColor (fClearColor[0], fClearColor[1], fClearColor[2], fClearColor[3]);
   	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	UpdateProjection ();
 	
 	::glMatrixMode(GL_MODELVIEW);
	::glLoadIdentity();
	SetLights ();
	ComputeTranslation();
	::glTranslatef(-fTranslation[0], -fTranslation[1], -fTranslation[2]);

  	::glMultMatrixf(fMatrix);
	::glTranslatef(fTranslation[0], fTranslation[1], fTranslation[2]);
	
}


#pragma segment Main
void COpenGLContext::RemoveRenderable(CGLRenderable* renderable)
{
	fRenderables.RemoveElement(renderable);
}


#pragma segment Main
void COpenGLContext::RemoveAllRenderables()
{
	fRenderables.RemoveAllItems();
}


#pragma segment Main
bool COpenGLContext::HasRenderable(CGLRenderable* renderable)
{
	return (GetRenderableWithID (renderable->GetGeomID()) != NULL);
}

#pragma segment 
void COpenGLContext::UpdateProjection()
{
	BecomeCurrentContext();
	fPerspective.Update ();

	CMyError::CheckForGLError (false, true);
}


#pragma segment 
void COpenGLContext::Zoom(float factor)
{
	fPerspective.Zoom (factor);
	
	UpdateProjection ();
}


#pragma segment 
void COpenGLContext::BecomeCurrentContext()
{
#if !COMPILE_REMOVE
	if (fAGLContext != NULL && fAGLContext != ::CGLGetCurrentContext())
	{
		CGLSetCurrentContext(NULL);
		CGLSetCurrentContext(fAGLContext);
	}
#endif
}


#pragma segment 
void COpenGLContext::FinishDraw()
{
	ReleaseOffScreen();
}

#pragma segment 
#ifdef kUseJNI
void COpenGLContext::SetPointWithArray(JNIEnv *env, jfloatArray array, GLfloat* point)
{
    ::JavaArrayToGLFloat(env, array, point, 3);
}
#endif

#pragma segment 
#ifdef kUseJNI
void COpenGLContext::SetPointerLocation(JNIEnv *env, jfloatArray startPt, jfloatArray endPt)
{
	if (fPointer == NULL)
	{
		CreatePointerObject ();
	}
	
	float start[3], end[3];
	SetPointWithArray (env, startPt, start);
	SetPointWithArray (env, endPt, end);
	
	fPointer->SetLocation (start, end);
}
#endif

#pragma segment 
void COpenGLContext::ShowPointer(bool show)
{
	if (fPointer == NULL)
	{
		CreatePointerObject ();
	}
	
	fPointer->SetShown (show);
	
}


#pragma segment 
void COpenGLContext::CreatePointerObject()
{
	if (fPointer == NULL)
	{
		fPointer = new CGLPointer;
	}
}




#pragma segment Main
void COpenGLContext::Render()
{
	if (fAGLContext != NULL)
	{
		PrepareToDraw();
		if (fPointer != NULL && fPointer->IsShown ())
		{
			fPointer->Render(fLowRes);
		}
		
		if (fRenderables.GetSize () > 0)
		{
			long size = fRenderables.GetSize ();
			
			if (false)
			{
				for (long i = 1; i <= size; i++)
				{
					CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(i);
					renderable->DebugDraw(fLowRes);
				}
			}
				// render opaque first
			for (long i = 1; i <= size; i++)
			{
				CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(i);
				if (renderable->IsOpaque())
				{
					renderable->Render(fLowRes);
				}
			}
			
			
				// draw translucent objects
			::glEnable(GL_BLEND);
			::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			::glDepthMask(false);
			for (long i = 1; i <= size; i++)
			{
				CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(i);
				if (!renderable->IsOpaque())
				{
					renderable->Render(fLowRes);
				}
			}
			::glDepthMask(true);
			::glDisable(GL_BLEND);
		}
		else
		{
			::glPushAttrib(GL_LIGHTING_BIT);
			float color[4] = {1.0, 1.0, 1.0, 1.0};
			

			::glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
			::glutSolidTeapot(160.0);
			::glPopAttrib();
		}
		
		FinishDraw ();
	}
}






#pragma segment Main



#pragma segment Main
void COpenGLContext::Create(Rect &rect)
{
#if kMyUsePBuffer
	if (!this->SetUpContextWithPBuffer(rect.right - rect.left, rect.bottom - rect.top))
	{
		fprintf(stderr, "Could not SetUpContextWithPBuffer.");
		this->CleanUp();
		return;
	}
#endif
#if kUseOffscreenPtr
	if (fScreenPtr)
	{
		free(fScreenPtr);
		fScreenPtr = NULL;
	}
	fWidth = rect.right - rect.left;
	fHeight = rect.bottom - rect.top;
	fRowBytes = fWidth * kPixelSize;
	fScreenPtr = malloc(fHeight * fRowBytes);
	if (fScreenPtr)
	{
		fAGLContext = setupContextOffscreen (fScreenPtr, fWidth, 
											 fHeight, fRowBytes, kPixelSize);
	}
#endif
#if kUseGWorld
	fUseGWorld = true; 
	
	if (fUseGWorld)
	{
		fGWorld.AllocateGWorld (rect, kPixelSize, COffScreenGWorld::kInVRAM);
		// need better error handling here
		
		if (fGWorld.LockPixels())
		{
			void *baseaddr = fGWorld.GetBaseAddress ();
			short rowbytes = fGWorld.GetRowBytes ();
		
			/* Setup the OpenGL context */
			fAGLContext = setupContextOffscreen (baseaddr, rect.right- rect.left, 
						rect.bottom- rect.top, rowbytes, fGWorld.GetPixelDepth());
			fGWorld.UnlockPixels();
		}
	}
	else
	{
//		fAGLContext = SetupContext ((AGLDrawable)win);
	}
				
	if (fAGLContext == NULL)
	{
		CMyError::CheckForGLError (false, true);
	}
	
	
	if (!fUseGWorld)
	{
	    glDrawBuffer(GL_BACK);
	
	} 
	
	CMyError::CheckForGLError (false, true);
#endif	
   	::glEnable(GL_LIGHTING);
   	::glEnable(GL_DEPTH_TEST);

	if (CGLMeshHolder::kDefaultGLFace != GL_FRONT_AND_BACK)
	{
		::glEnable(GL_CULL_FACE);
		::glCullFace(GL_BACK);
	}
	
//  	glShadeModel (GL_SMOOTH);
	CMyError::CheckForGLError (false, true);
	
}

/*
#pragma segment Main
void COpenGLContext::SetViewBox(float left, float right, float top, float bottom,
				float front, float back)
{
	fPerspective.SetValues (left, right, bottom, top, front, back);
}
*/

#pragma segment Main
#ifdef kNOJNI	
void COpenGLContext::Rotate(float xAngle, float yAngle)
{		// this was for debugging in GLBrainView app
	GLdouble  xAxis[3] = {0, -1, 0};
	GLdouble  yAxis[3] = {1, 0, 0};
	GLdouble  zeroPoint[3] = {0, 0, 0};
  	GLint viewport[4];
   	GLdouble mvmatrix[16], projmatrix[16];

	BecomeCurrentContext ();
	
	::glPushAttrib(GL_TRANSFORM_BIT);		
  	::glMatrixMode(GL_MODELVIEW);
	::glPushMatrix();
	::glLoadMatrixf(fMatrix);
  
    ::glGetIntegerv (GL_VIEWPORT, viewport);
    ::glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);
    ::glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);

    ::gluUnProject (xAxis[0], xAxis[1], xAxis[2], 
       mvmatrix, projmatrix, viewport, &xAxis[0], &xAxis[1], &xAxis[2]); 
    ::gluUnProject (yAxis[0], yAxis[1], yAxis[2], 
       mvmatrix, projmatrix, viewport, &yAxis[0], &yAxis[1], &yAxis[2]); 
    ::gluUnProject (zeroPoint[0], zeroPoint[1], zeroPoint[2], 
       mvmatrix, projmatrix, viewport, &zeroPoint[0], &zeroPoint[1], &zeroPoint[2]); 
				 
	::glRotated(xAngle, xAxis[0] - zeroPoint[0], xAxis[1] - zeroPoint[1], xAxis[2] - zeroPoint[2]);
	::glRotated(yAngle, yAxis[0] - zeroPoint[0], yAxis[1] - zeroPoint[1], yAxis[2] - zeroPoint[2]);
	::glGetFloatv(GL_MODELVIEW_MATRIX, fMatrix);
	
	::glPopMatrix();
	::glPopAttrib();
}

#endif


#pragma segment Main
void COpenGLContext::SetLights()
{
	GLfloat light[kLightArraySize];
	light[kLightArraySize - 1] = 1.0; // alpha
	
	/* Set the pixel size attribute */
	if (CGLMeshHolder::kDefaultGLFace == GL_FRONT_AND_BACK)
	{
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	}
	
	glLightfv(GL_LIGHT0, GL_POSITION, fLightDirection);
        
        for (int i = 0; i < kLightArraySize - 1; i++)
        {
            light[i] = fSpecularity;
        }
	glLightfv(GL_LIGHT0, GL_SPECULAR, light);
        for (int i = 0; i < kLightArraySize - 1; i++)
        {
            light[i] = fDiffuseness;
        }
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
        for (int i = 0; i < kLightArraySize - 1; i++)
        {
            light[i] = fAmbience;
        }
	glLightfv(GL_LIGHT0, GL_AMBIENT, light);
	glEnable(GL_LIGHT0);
}

#pragma segment Main
void COpenGLContext::SetZoomFocus(long h, long v)
{
//	h += fTranslation[0];
//	v += fTranslation[1];
	fPerspective.SetZoomFocus (h, v);
}





#pragma segment 
void COpenGLContext::AbsoluteZoom(float zoom)
{
	fPerspective.AbsoluteZoom (zoom * 2);
//	fPerspective.AbsoluteZoom (zoom);
}


#pragma segment 
void COpenGLContext::DebugCleanup()
{
	CleanUp();
}

#pragma segment 
void COpenGLContext::SetFiducials(CGLFiducials *fiducials)
{
	if (fFiducials != NULL)
	{
		RemoveRenderable (fFiducials);
		
		delete fFiducials;
		fFiducials = NULL;
	}

	
	fFiducials = fiducials;
	
	if (fFiducialsShown && fFiducials != NULL)
	{
		AddRenderable(fiducials);
	}

}


#pragma segment 
void COpenGLContext::SetShowFiducials(bool show)
{
	if (fFiducials != NULL)
	{
		if (show)
		{
			AddRenderable(fFiducials);
		}
		else
		{
			RemoveRenderable (fFiducials);
		}
	}
	
	fFiducialsShown = show;
}

#pragma segment Main
void COpenGLContext::ComputeTranslation()
{
	
	DblRect box;
	fPerspective.GetViewBox (box);
	C3DPoint center((long)(box.right - box.left),(long)( box.bottom - box.top), 0);
	 
	long weight = 0;
	long size = fRenderables.GetSize ();

		// render opaque first
	for (long i = 1; i <= size; i++)
	{
		CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(i);
		long newWeight;
		C3DPoint newCenter;
		if (renderable->GetCenter (newCenter, newWeight) && newWeight > weight)
		{
			center = newCenter;
			weight = newWeight;
		}
	}
	
/*	DblPoint focus;
	fPerspective.GetFocus (focus);
	
	focus.h += -fTranslation[0] - center.fX;
	focus.v += -fTranslation[1] - center.fY;
	fPerspective.SetZoomFocus (focus.h, focus.v); */
	
	SetTranslation(-center.fX, -center.fY, -center.fZ);
}










void COpenGLContext::SetTranslation(float x, float y, float z)
{
	fTranslation[0] = x;
	fTranslation[1] = y;
	fTranslation[2] = z;
}
#if kMyUsePBuffer
bool COpenGLContext::SetUpContextWithPBuffer(int width, int height)
{
#if !COMPILE_REMOVE

#define kPbufferMaxLevels 0
	if (!aglCreatePBuffer (width, height, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, kPbufferMaxLevels, &fAGLPBuffer))
	{
		fprintf(stderr, "Could not create PBuffer\n");
		return false;
	}
	
	//	error handling this shit
	AGLPixelFormat fmt;
	GLboolean      ok;
	GLint          attrib[] = { AGL_RGBA, AGL_PIXEL_SIZE, 0,  AGL_DEPTH_SIZE, 24,  AGL_NONE };
	
	attrib[2] = kPixelSize;
	
	/* Choose an rgb pixel format */
	fmt = aglChoosePixelFormat(NULL, 0, attrib);
	if(fmt == NULL)
	{
		fprintf(stderr, "Could not choose PixelFormat\n");
		return false;
		
	}
	
	/* Create an OpenGL context */
	fAGLContext = aglCreateContext(fmt, NULL);
	if(fAGLContext == NULL)
	{
		fprintf(stderr, "Could not create context\n");
		aglDestroyPixelFormat(fmt);
		return false;
		
	}
	
	
	/* Make the context the current context */
	ok = aglSetCurrentContext(fAGLContext);
	if(!ok) return NULL;
	
	if (!aglSetPBuffer (fAGLContext, fAGLPBuffer, 0, 0, 0))	
	{
		fprintf(stderr, "Could not setPBuffer\n");
		aglDestroyPixelFormat(fmt);
		return false;
	}
	
	/* The pixel format is no longer needed */
	aglDestroyPixelFormat(fmt);
	
	CMyError::CheckForGLError (false, true);
	
	return true;
#else
	return false;
#endif
}
#endif

#pragma segment 
#if 0
AGLContext COpenGLContext::SetupContext(AGLDrawable win)
{
	GLint          attrib[] = {AGL_RGBA, AGL_DOUBLEBUFFER, 	AGL_DEPTH_SIZE, 24, AGL_NONE };
	AGLPixelFormat fmt;
	AGLContext     ctx;
	GLboolean      ok;

	/* Choose an rgb pixel format */
	fmt = aglChoosePixelFormat(NULL, 0, attrib);
	if(fmt == NULL) return NULL;

	/* Create an OpenGL context */
	ctx = aglCreateContext(fmt, NULL);
	if(ctx == NULL) return NULL;

	/* Attach the window to the context */
	ok = aglSetDrawable(ctx, win);
	if(!ok) return NULL;
	
	/* Make the context the current context */
	ok = aglSetCurrentContext(ctx);
	if(!ok) return NULL;

	/* Pixel format is no longer needed */
	aglDestroyPixelFormat(fmt);

	return ctx;
	
}
#endif
void COpenGLContext::SetMaterialProperties(float specularity, float ambience, float diffuseness, float shininess)
{
    long size = fRenderables.GetSize ();
    for (long i = 1; i <= size; i++)
    {
        CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(i);
        renderable->SetMaterialProperties(specularity, ambience, diffuseness, shininess);
    }
}

void COpenGLContext::GetMaterialProperties(float& specularity, float& ambience, float& diffuseness, float& shininess)
{
    long size = fRenderables.GetSize ();
    if (size > 0)
    {
        CGLRenderable* renderable = (CGLRenderable*)fRenderables.At(1);
        renderable->GetMaterialProperties(specularity, ambience, diffuseness, shininess);
    }
    else
    {
        specularity = CGLRenderable::kDefaultSpecularity;
        ambience = CGLRenderable::kDefaultAmbience;
        diffuseness = CGLRenderable::kDefaultDiffuseness;
		shininess = CGLRenderable::kDefaultShininess;
    }
    
}
        
void COpenGLContext::SetLightProperties(float specularity, float ambience, float diffuseness)
{
    fSpecularity = specularity;
    fAmbience = ambience;
    fDiffuseness = diffuseness;
}

void COpenGLContext::GetLightProperties(float& specularity, float& ambience, float& diffuseness)
{
    specularity = fSpecularity;
    ambience = fAmbience;
    diffuseness = fDiffuseness;
}

void COpenGLContext::SetLightDirection(float* lightDirection)
{
	for (int i = 0; i < kLightArraySize; i++)
	{
		fLightDirection[i] = lightDirection[i];
	}
}
											 

		




