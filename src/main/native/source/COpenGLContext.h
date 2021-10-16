// COpenGLContext.h
// Created by Steve on Mon, Jun 12, 2000 @ 12:43 PM.

#ifndef __COpenGLContext__
#define __COpenGLContext__

#ifndef __CGLObject__
#include "CGLObject.h"
#endif

#include <OpenGL/OpenGL.h>


#include <JavaVM/jni.h>

#include <OpenGL/gl.h>

#ifndef __CMySortedList__
#include "utility/CMySortedList.h"
#endif

#define kUseGWorld 0	// legacy

#if kUseGWorld
#ifndef __COffScreenGWorld__
#include "COffScreenGWorld.h"
#endif

#endif

#ifndef __CGLPerspective__
#include "CGLPerspective.h"
#endif

#define kUseOffscreenPtr 0	// for debugging
#define kPre105 1			//  pre 10.5
#define kMyUsePBuffer 0
#define kUseFrameBuffer 0	// never fully implemented
#define kUsePixelBuffer  1

const short	kMatrixSize = 16;
const short kLightArraySize = 4;


class CGLRenderable;
class CGLPointer;
class CGLFiducials;
//class CGLTexFont;

class COpenGLContext : public CGLObject
{
public:
	COpenGLContext();
	
	void Create(Rect &rect);
	
#ifndef kNOJNI
	void FillInJavaRGBArray(JNIEnv *env, jintArray array, long width, long height);
#endif
	
	virtual ~COpenGLContext();
	
	void SetRotation(float* matrix);
#ifdef kNOJNI	
	void Rotate(float xAngle, float yAngle);
#endif	
	void AddRenderable(CGLRenderable* renderable);
	
	bool HasRenderable(CGLRenderable* mesh);
	
	void SetLowRes(bool lowRes) { fLowRes = lowRes; }
	
	void Render();
	
	void RemoveRenderable(CGLRenderable* renderable);
	
	void RemoveAllRenderables();
	
	void Zoom(float factor);
	
	void AbsoluteZoom(float zoom);

	void DebugCleanup();
	
	void SetFiducials(CGLFiducials *fiducials);
	
	void SetShowFiducials(bool show);
	
	void SetZoomFocus(long h, long v);
        
	GLfloat* GetLightDirection() { return fLightDirection; }
	
	void SetLightDirection(float* lightDirection);
	
	void SetMaterialProperties(float specularity, float ambience, float diffuseness, float shininess);

	void GetMaterialProperties(float& specularity, float& ambience, float& diffuseness, float& shininess);
	
	void SetLightProperties(float specularity, float ambience, float diffuseness);

	void GetLightProperties(float& specularity, float& ambience, float& diffuseness);
		
//	void SetViewBox(float left, float right, float top, float bottom,
//				float front, float back);
	
#ifdef kUseJNI
	void SetPointerLocation(JNIEnv *env, jfloatArray startPt, jfloatArray endPt);
#endif
	
	void ShowPointer(bool show);
	
//	GWorldPtr GetGWorld() { return fGWorld.GetGWorld(); }

	void FinishDraw();
		
private :
	void ComputeTranslation();

	void SetTranslation(float x, float y, float z);
	
	void SetLights();
	
	void CreatePointerObject();
	
	void UpdateProjection();
	
	void BecomeCurrentContext();
	
#ifdef kUseJNI
	static void SetPointWithArray(JNIEnv *env, jfloatArray array, GLfloat* point);
#endif	
	void TestDraw();
	
	static void SetIdentityMatrix4(float* matrix);
	
#if kUseFrameBuffer
	bool setUpFrameBuffer(GLsizei width, GLsizei height);
#endif

#if kMyUsePBuffer
	bool SetUpContextWithPBuffer(int width, int height);
#endif

#if kUsePixelBuffer
	CGLContextObj setupContextOffscreen(void *baseaddr, GLsizei width, GLsizei height, GLsizei rowbytes, GLsizei pixelsize);
#endif
	
//	AGLContext SetupContext(AGLDrawable win);
	
	void PrepareToDraw();
	
	void CleanUp();
	
	CGLRenderable* GetRenderableWithID(long geomID);
	bool PrepareOffScreen();
	void ReleaseOffScreen();
	
	static const short 		kPixelSize;
#if kUsePixelBuffer
	CGLContextObj 			fAGLContext;
#endif
#if kMyUsePBuffer
	CGLPBufferObj			fAGLPBuffer;
#endif
#if kUseFrameBuffer
	GLuint			fFramebuffer;
	GLuint			fRenderbuffer;
#endif
#if kUseOffscreenPtr
	void*				fScreenPtr;
	int					fWidth;
	int					fHeight;
	int					fRowBytes;
#endif
#if kUseGWorld
	COffScreenGWorld		fGWorld;
#endif
    bool 				fUseGWorld;	// this flag is transitional for PBuffers, which aren't implemented
	float				fMatrix[kMatrixSize];
	CMySortedList			fRenderables;
	CGLPerspective 			fPerspective;

	float				fClearColor[4];
	
	float				fTranslation[3];
	
		// neurosynch specific
	CGLPointer			*fPointer;
	CGLFiducials			*fFiducials;
	bool				fFiducialsShown;
    bool				fLowRes;
	
	GLfloat 			fLightDirection[kLightArraySize];
 
	float		fSpecularity;
	float		fAmbience;
	float		fDiffuseness;
};

#endif
