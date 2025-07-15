/*
 *  NativeOpenGL.h
 *  neurosynch
 *
 *  Created by Steven Meyer on Thu Aug 08 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#if !defined(__NATIVEOPENGL_HEADER__)
#define __NATIVEOPENGL_HEADER__ 1
#define kOpenGL 0

typedef float CGLMCoord;

// workaround for a bug 5/16/05
#define __CARBONSOUND__ 1

#define kOSX 1
#define kNoACS 1
#define kUseJNI true
typedef long ListIndex;

typedef ListIndex CGLMIndex;

#define kUSE_VULKAN 1

struct Rect
{
  short top;
  short left;
  short bottom;
  short right;
};
typedef struct Rect Rect;

#endif
