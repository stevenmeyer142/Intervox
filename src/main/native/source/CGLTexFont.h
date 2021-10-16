// CGLTexFont.h
// Created by Steve on Mon, Aug 21, 2000 @ 1:20 PM.

// based on texfont.c\h  Copyright (c) Mark J. Kilgard, 1997. 


#ifndef __CGLTexFont__
#define __CGLTexFont__

#ifndef __CGLObject__
#include "CGLObject.h"
#endif

#include <OpenGL/gl.h>
#include <JavaVM/jni.h>
#include <iostream>

#include <stdio.h>


#define TXF_FORMAT_BYTE		0
#define TXF_FORMAT_BITMAP	1

typedef struct {
  unsigned short c;       /* Potentially support 16-bit glyphs. */
  unsigned char width;
  unsigned char height;
  signed char xoffset;
  signed char yoffset;
  signed char advance;
  char dummy;           /* Space holder for alignment reasons. */
  short x;
  short y;
} TexGlyphInfo;

typedef struct {
  GLfloat t0[2];
  GLshort v0[2];
  GLfloat t1[2];
  GLshort v1[2];
  GLfloat t2[2];
  GLshort v2[2];
  GLfloat t3[2];
  GLshort v3[2];
  GLfloat advance;
} TexGlyphVertexInfo;


class CGLTexFont : public CGLObject
{
  GLuint 				fTexobj;
  int 					fTexWidth;
  int 					fTexHeight;
  int 					fMaxAscent;
  int 					fMaxDescent;
  int 					fNumGlyphs;
  int 					fMinGlyph;
  int 					fRange;
  unsigned char*		fTexImage;
  TexGlyphInfo*			fTgi;
  TexGlyphVertexInfo*	fTgvi;
  TexGlyphVertexInfo**	fLut;
  char*					fLastError;
  bool					fInitialized;
  
  static CGLTexFont 	*fgDefaultTexFont;

public:
	CGLTexFont();
	
	virtual ~CGLTexFont();

	const char *txfErrorString(void);

	void LoadFont(std::istream &in);
	
	void LoadFontFromArray(const char* array, long size);

	void LoadFontFromFile(char *filename);
	
	//void ReadGlyphInfo(File file);
	
	void txfUnloadFont();
	
	static CGLTexFont *GetDefaultTexFont();
	
	static void SetDefaultTexFont(const char *array, long length);

	GLuint txfEstablishTexture(GLuint texobj, GLboolean setupMipmaps);

	void txfBindFontTexture();

	void txfGetStringMetrics(const char *string, long len, long &width,
  		long &max_ascent, long &max_descent);

	void txfRenderGlyph(long c);

	void txfRenderString(const char *string, long len);

	void txfRenderFancyString(const char *string, long len);
	
private :
	TexGlyphVertexInfo *getTCVI(int c);
};

#endif
