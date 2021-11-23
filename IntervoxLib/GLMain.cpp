
//#define __cplusplus 1

#include "NativeOpenGL.h"
#include "com_brazedblue_intervox_view3D_OpenGLJNI.h"
#include "utility/CMyError.h"
#include "JNICommon.h"
#include "utility/CJavaArrSlicesSet.h"
#include "utility/C3DPoint.h"
//#include "CGLTexFont.h"
#include <typeinfo>
#include "extensions/VulkanContext.hpp"
#include "extensions/VulkanMeshHolder.hpp"


const jlong NO_OBJECT = 1;
CMySortedList*	gDebugAllocatedList = NULL;
void DebugHandleShutDown(JNIEnv *env, jobjectArray errRecord);


void DisposeVulkanContext(JNIEnv *env, CVulkanContext *renderer, jobjectArray errRecord);
void Get2DFloatArrayElem(JNIEnv *env, jobjectArray float2dArray, int index1, int length, GLfloat *result);
void Set2DFloatArrayElem(JNIEnv *env, jobjectArray float2dArray, int index1, int length, GLfloat *result);

//void InitializeAGLHack();
/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pCreateGLContext
 * Signature: ([I[Ljava/lang/String;)I
 */
JNIEXPORT jlong JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pCreateGLContext
	(JNIEnv *env, jobject openGLObj, jintArray frameRect, jobjectArray errRecord)
{
	CVulkanContext *dataObj = NULL;
	try
	{
		Rect location;
		
		if (!JavaIntArrayToRect (env, frameRect, location))
		{
			return 0;
		}
		
		dataObj = new CVulkanContext;
		dataObj->initialize (&location);
		
		

//		DebugAddAllocatedObject (dataObj);
	}
	catch (CMyError &err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
	
	return (jlong)dataObj;
} 


/**
	 fill int array with render image
	 @param      qdView        id of 3d context.
	 @param      pixArray 	 array for image in rgba format 
	 @param      width 		 image width 
	 @param      height 		 image height
	 @param	   errRecord     record for error info
	                             
*/
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pGetGLPixelData
	(JNIEnv *env, jobject openGL, jlong qdDisplayObject, jintArray rgbArray,
  		jint width, jint height, jobjectArray errRecord)
{
	try
	{
		CVulkanContext *dataObj = (CVulkanContext*)qdDisplayObject;

		dataObj->FillInJavaRGBArray (env, rgbArray, width, height);

	}
	catch (CMyError &err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pDebugCheckDeallocation
 * Signature: (Lneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pDebugCheckDeallocation
	(JNIEnv *env, jobject, jobjectArray errRecord)
{
	DebugHandleShutDown (env, errRecord);
}
 

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pCreateGLContext
 * Signature: (Ljava/awt/Rectangle;Lneurosynch/view3D/NativeError;)I
 */
/*	JNIEXPORT jint JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pCreateGLContext
	(JNIEnv *env, jobject openGLObj, jintArray frameRect, jobjectArray errRecord)
{
    COpenGLContext *dataObj = NULL;
	try
	{
		Rect location;
		
		if (!JavaIntArrayToRect (env, frameRect, location))
		{
			return 0;
		}
		
		dataObj = new COpenGLContext;
		dataObj->Create (location);

		DebugAddAllocatedObject (dataObj);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
	
	return (jint)dataObj;
} */

/**
	 zoom the context view
	 @param      howMuch       factor (i.e. .5 = zoom in 50 %)
	 @param      qdView 		 the view 
	 @param	   errRecord     record for error info
	                             
*/

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pZoomContext
	(JNIEnv *env, jobject openGL, jfloat amount, jlong glDisplayObject, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glDisplayObject;

		dataObj->Zoom (amount );
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}


/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pDisposeGLContext
 * Signature: (ILneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pDisposeGLContext
	(JNIEnv *env, jobject openGL, jlong glDisplayObject, jobjectArray errRecord)
{
	CVulkanContext *glObject = (CVulkanContext*)glDisplayObject;
	
	::DisposeVulkanContext (env, glObject, errRecord);
}

void DisposeVulkanContext(JNIEnv *env, CVulkanContext *renderer, jobjectArray errRecord)
{
//	DebugRemoveAllocatedObject (context);
	
     delete renderer;
}


/*
long DebugGetFreeVRam()
{
	CGrafPtr port;
	GDHandle device;
	::GetGWorld(&port, &device); // save window's graphics port
	
	return ATIMem_GetFreeVRAM(device);

}*/


/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetRotation 
 * Signature: (I[FLneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetRotation
	(JNIEnv *env, jobject openGL, jlong glDisplayObject, jfloatArray  javaMatrix, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glDisplayObject;
		
		float matrix[kMatrixSize];

		env->GetFloatArrayRegion(javaMatrix, 0, kMatrixSize, matrix);
		CMyError::CheckForJNIException(env);

		dataObj->SetRotation (matrix);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}
/**
	 Creates a 3D mesh from slices data
	 @param      width         slice image width.
	 @param      height 		 slice image height 
	 @param      slices 		 byte values with bit set for each of 7 regions
	 @param      value 		 bit of this region
	 @param	   	 geomID		 geometry identifier
	 @return 	 int generated by native code
	                             
*/
JNIEXPORT jlong JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pCreateGeometryFromRegion
	(JNIEnv *env, jobject openglObj,  jint width, jint height, jobjectArray objArrays, jint regionValue, 
			jlong geomID,  jint resolution, jobjectArray errRecord)
{
	jlong result = NO_OBJECT;
#
	try
	{
		VulkanMeshHolder *meshHolder = new VulkanMeshHolder();
		meshHolder->CreateGeometries (env, width, height, objArrays, regionValue, geomID, resolution);
		
		result = (jlong)meshHolder;
		
	//	DebugAddAllocatedObject(meshHolder);
 	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	
	}

	
	return result; 
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNIJNI
 * Method:    pGetGeometryResolution
 * Signature: (I[Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pGetGeometryResolution
  (JNIEnv *env, jobject, jlong glMesh, jobjectArray errRecord)
{
    jint result = NO_OBJECT;
#if 0
    try
    {
            CGLMeshHolder *mesh = (CGLMeshHolder*)glMesh;
            result = mesh->GetResolution();
    }
    catch (CMyError err)
    {
            FillErrRecord(env, err, errRecord);
    }
    catch (...)
    {
    
    }
#endif
    
    return result;
}



/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pRemoveAllGeometriesFromView
 * Signature: (J[Ljava/lang/String;)V
 */

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pRemoveAllGeometriesFromView
                                    (JNIEnv *, jobject, jlong, jobjectArray)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
		dataObj->RemoveAllRenderables (); 
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pRemoveGeometryFromView
 * Signature: (IILneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pRemoveGeometryFromView
	(JNIEnv *env, jobject openGL, jlong meshObj, jlong glContext, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *context = (COpenGLContext*)glContext;
		context->RemoveRenderable ((CGLMeshHolder*) meshObj); 
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pViewHasGeometry
 * Signature: (IILneurosynch/view3D/NativeError;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pViewHasGeometry
	(JNIEnv *env, jobject openGL, jlong meshObj, jlong glContext, jobjectArray errRecord)
{
	jboolean result = false;
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
 		result = dataObj->HasRenderable ((CGLMeshHolder*)meshObj); 
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
	return result;
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pAddGeometryToView
 * Signature: (IILneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pAddGeometryToView
	(JNIEnv *env, jobject openGL, jlong meshObj, jlong glContext,  jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
		dataObj->AddRenderable ((CGLMeshHolder*)meshObj); 
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetGeometryColor
 * Signature: (IFFFLneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetGeometryColor
	(JNIEnv *env, jobject, jlong glMesh, jfloat red, jfloat green, jfloat blue, jobjectArray errRecord)
{
#if 0
	try
	{
		CGLMeshHolder *mesh = (CGLMeshHolder*)glMesh;
		mesh->SetGeometryColor(red, green, blue);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetGeometryTransparency
 * Signature: (IFLneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetGeometryTransparency
	(JNIEnv *env, jobject, jlong glMesh, jfloat transparency, jobjectArray errRecord)
{
#if 0
	try
	{
		CGLMeshHolder *mesh = (CGLMeshHolder*)glMesh;
		mesh->SetTransparency(transparency);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetLowResView
 * Signature: (IZLneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetLowResView
	(JNIEnv *env, jobject, jlong glContext, jboolean lowRes, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *context = (COpenGLContext*)glContext;
		context->SetLowRes(lowRes);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pDispose3DGeometry
 * Signature: (ILneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pDispose3DGeometry
	(JNIEnv *env, jobject openGL, jlong meshObj, jobjectArray errRecord)
{
#if 0
	CGLMeshHolder *glObject = (CGLMeshHolder*)meshObj;
	
	DebugRemoveAllocatedObject (glObject); 
	
	if (gDebugging)
	{
		CMyError::DebugMessage("mesh being deleted");
	}
	delete glObject;
#endif
	
}


/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetPointerLocation
 * Signature: (I[F[FLneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetPointerLocation
	(JNIEnv *env, jobject openGL, jlong glDisplayObject, jfloatArray startPt, jfloatArray endPt, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glDisplayObject;

		dataObj->SetPointerLocation (env, startPt, endPt);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pShowPointer
 * Signature: (IZLneurosynch/view3D/NativeError;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pShowPointer
	(JNIEnv *env, jobject openGL, jlong glDisplayObject, jboolean show, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glDisplayObject;

		dataObj->ShowPointer (show);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}



void DebugHandleShutDown(JNIEnv *env, jobjectArray errRecord)
{
#if 0
	if (gDebugAllocatedList != NULL)
	{
		long unAllocCount = gDebugAllocatedList->GetSize();
		
		for (long i = 0; i <= unAllocCount; i++)
		{
			CGLObject *obj = (CGLObject*)gDebugAllocatedList->GetElementAt (i);
			
			if (typeid(COpenGLContext) == typeid(*obj))
			{
				DisposeVulkanOffscreenRenderer (env, (COpenGLContext*)obj, errRecord);
			}
			else
			{
				delete obj;
			}
		}
		gDebugAllocatedList->RemoveAllItems ();
		delete gDebugAllocatedList;
		gDebugAllocatedList = NULL;
		
		if (unAllocCount > 0)
		{
			SetErrorCodeAndMessage (env, unAllocCount, "Native objects have not been deallocated", errRecord);
		}
	}
#endif
}


/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pAbsoluteZoomContext
 * Signature: (FILneurosynch/view3D/NativeError;)V
 */

/**
	 zoom the context view
	 @param      absolute      percent 1 - 100% (100% is start)
	 @param      qdView 		 the view 
	 @param	   errRecord     record for error info
	                             
*/

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pAbsoluteZoomContext
	(JNIEnv *env, jobject openGL, jfloat amount, jlong glDisplayObject, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glDisplayObject;

		dataObj->AbsoluteZoom (amount );
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}


/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetZoomFocus
 * Signature: (IIILneurosynch/view3D/NativeError;)V
 */

/**
	 set zoom focus
	 @param      x      		x to zoom to
	 @param      y      		y to zoom to
	 @param      glContext 		the context 
	 @param	   errRecord     record for error info
	                             
*/

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetZoomFocus
	(JNIEnv *env, jobject, jint x, jint y, jlong glContext, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;

		dataObj->SetZoomFocus (x, y);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNIJNI
 * Method:    pSetFiducials
 * Signature: ([[F[Ljava/lang/String;I[Ljava/lang/String;)V
 */

/**
	 set(update) fiducial point locations
	 @param      locations  	2D array of float
	 @param      labels  		array of "String"
	 @param      qdView 		the view 
	 @param	   errRecord     record for error info
	                             
*/

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetFiducials
	(JNIEnv *env, jobject, jobjectArray locations, jobjectArray labels, jlong glContext, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
		CGLFiducials *fiducials = NULL;
		if (locations != NULL)
		{
			fiducials = CGLFiducials::CreateFiducialsFromList (env, locations, labels);
		}
		dataObj->SetFiducials (fiducials);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetShowsFiducials
 * Signature: (ZILneurosynch/view3D/NativeError;)V
 */

/**
	 display the fiducials
	 @param      show  		display fiducials
	 @param      qdView 		the view 
	 @param	   errRecord     record for error info
	                             
*/

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetShowsFiducials
	(JNIEnv *env, jobject, jboolean show,  jlong glContext, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
		dataObj->SetShowFiducials (show);
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetTexFont
 * Signature: ([BLneurosynch/view3D/NativeError;)V
 */

/**
	 create GL texfont data from byte array
	 @param      array  		bytes of texfont data
	 @param	   errRecord     record for error info
	                             
*/
/*
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetTexFont
	(JNIEnv *env, jobject, jbyteArray array, jobjectArray errRecord)
{
	try
	{
		jboolean isCopy;
		
		jbyte *toBuffer = env->GetByteArrayElements(array, &isCopy);
		CMyError::CheckForJNIException(env);
		
		jsize length = env->GetArrayLength( array);
		CMyError::CheckForJNIException(env);
		
                char* bufferCopy = new char[length];
                // error checking here?
                for(long i = 0; i < length; i++)
                {
                    bufferCopy[i] = toBuffer[i];
                }
		env->ReleaseByteArrayElements(array, toBuffer, 0);

                CGLTexFont::SetDefaultTexFont (bufferCopy, length);
                
                delete bufferCopy;
                
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}

} */
   
/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pSetDebugging
 * Signature: (Z)V
 */

/**    
	 enable/disable debugging code
	 @param      debugging  		
	                              
*/   

JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pSetDebugging
  (JNIEnv *, jobject, jboolean debugging)

{
	gDebugging = debugging;
}


/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pCreateGeometryFromVersion
 * Signature: (IILneurosynch/view3D/NativeError;)I
 */

/**
	 create a mesh which shares the geometrical structure
	 @param      mesh 		 other mesh
	 @param	   geomID		 geometry identifier
	 @param	   errRecord     record for error info
	 @return 	   int generated by native code
	                             
*/
/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pGLUTInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pGLUTInit
(JNIEnv *, jobject, jobjectArray)
{
//	::InitializeAGLHack();
#if 0
	int argcp = 0;
	char* argv = NULL;
	::glutInit(&argcp, &argv);	// java seems to have already called this, I think there's no problem calling one mor time
#endif
}

	// 5/22/05 to fix a bad access call, mach-o library wasn't initialized
	//  aglChoosePixelFormat supposedly does the job, and it does!!
#if 0
void InitializeAGLHack()
{
	GLint          attrib[] = { AGL_RGBA, AGL_PIXEL_SIZE, 16, AGL_OFFSCREEN,  AGL_DEPTH_SIZE, 24,  AGL_NONE };
	
	/* Choose an rgb pixel format */
	AGLPixelFormat fmt = aglChoosePixelFormat(NULL, 0, attrib);
	if(fmt != NULL) 
	{
		aglDestroyPixelFormat(fmt);		
	}
}
#endif

JNIEXPORT jlong JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pCreateGeometryFromVersion
	(JNIEnv *env, jobject, jlong mesh, jlong geomID, jobjectArray errRecord)
{
	jlong result = NO_OBJECT;
#if 0
	try
	{
		CGLMeshHolder *meshHolder = new CGLMeshHolder();
		meshHolder->GrabGeometries((CGLMeshHolder*)mesh, geomID);
		
		result = (jlong)meshHolder;
		
		DebugAddAllocatedObject(meshHolder);
 	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	
	}
#endif
	
	return result; 

}



/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pDebugSetOpenGLLighting
 * Signature: (I[[F[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pDebugSetOpenGLLighting
  (JNIEnv *env, jobject, jlong glContext, jobjectArray float2dArray, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
        GLfloat lightDirection[kLightArraySize];
		jfloatArray floatArray = (jfloatArray)env->GetObjectArrayElement(float2dArray, 0);
		CMyError::CheckForJNIException (env); 
		
        JavaArrayToGLFloat(env, floatArray, lightDirection, kLightArraySize);
		dataObj->SetLightDirection(lightDirection);
        
		float specular, ambient, diffuse, shininess;
		Get2DFloatArrayElem(env, float2dArray, 1, 1, &specular);
		Get2DFloatArrayElem(env, float2dArray, 2, 1, &ambient);
		Get2DFloatArrayElem(env, float2dArray, 3, 1, &diffuse);
		
		dataObj->SetLightProperties(specular, ambient, diffuse);
		
		
		 
		Get2DFloatArrayElem(env, float2dArray, 4, 1, &specular);
		Get2DFloatArrayElem(env, float2dArray, 5, 1, &ambient);
		Get2DFloatArrayElem(env, float2dArray, 6, 1, &diffuse);
		Get2DFloatArrayElem(env, float2dArray, 7, 1, &shininess);
	   
	   dataObj->SetMaterialProperties(specular, ambient, diffuse, shininess);
  
	}
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

/*
 * Class:     com_brazedblue_intervox_view3D_OpenGLJNI
 * Method:    pDebugGetOpenGLLighting
 * Signature: (I[[F[Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_brazedblue_intervox_view3D_OpenGLJNI_pDebugGetOpenGLLighting
  (JNIEnv *env, jobject, jlong glContext, jobjectArray float2dArray, jobjectArray errRecord)
{
#if 0
	try
	{
		COpenGLContext *dataObj = (COpenGLContext*)glContext;
		GLfloat *lightLocation = dataObj->GetLightDirection();
                
        Set2DFloatArrayElem(env, float2dArray, 0, 4, lightLocation);
                
		float specular, ambient, diffuse, shininess;
		dataObj->GetLightProperties(specular, ambient, diffuse);

		Set2DFloatArrayElem(env, float2dArray, 1, 1, &specular);
		Set2DFloatArrayElem(env, float2dArray, 2, 1, &ambient);
		Set2DFloatArrayElem(env, float2dArray, 3, 1, &diffuse);
		
		dataObj->GetMaterialProperties(specular, ambient, diffuse, shininess);
		Set2DFloatArrayElem(env, float2dArray, 4, 1, &specular);
		Set2DFloatArrayElem(env, float2dArray, 5, 1, &ambient);
		Set2DFloatArrayElem(env, float2dArray, 6, 1, &diffuse);
		Set2DFloatArrayElem(env, float2dArray, 7, 1, &shininess);
     }
	catch (CMyError err)
	{
		FillErrRecord(env, err, errRecord);
	}
	catch (...)
	{
	}
#endif
}

void Get2DFloatArrayElem(JNIEnv *env, jobjectArray float2dArray, int index1, int length, GLfloat *result)
{
        jfloatArray floatArray = (jfloatArray)env->GetObjectArrayElement(float2dArray, index1);
        CMyError::CheckForJNIException (env); 
        
        JavaArrayToGLFloat(env, floatArray, result, length);
}

void Set2DFloatArrayElem(JNIEnv *env, jobjectArray float2dArray, int index1, int length, GLfloat *result)
{
        jfloatArray floatArray = (jfloatArray)env->GetObjectArrayElement(float2dArray, index1);
        CMyError::CheckForJNIException (env); 
        
        GLFloatToJavaArray(env, result, floatArray, length);
}

