// CGLTexFont.cp
// Created by Steve on Mon, Aug 21, 2000 @ 1:20 PM.

#include "NativeOpenGL.h"
#ifndef __CGLTexFont__
#include "CGLTexFont.h"
#endif


#include "utility/CMyError.h"

#include <string.h>
//#include <stdlib>
#include <ctype.h>
#include <fstream>
#include <sstream>
//#include<iomanip>

#include <OpenGL/glu.h>

#ifndef GL_VERSION_1_1

#  if defined(GL_EXT_texture_object)
#    define glGenTextures glGenTexturesEXT
#    define glBindTexture glBindTextureEXT
#  else
     /* Without OpenGL 1.1 or the texture object extension, use display lists. */
#    define USE_DISPLAY_LISTS
#  endif

#  if defined(GL_EXT_texture)
#    define GL_INTENSITY4 GL_INTENSITY4_EXT
     int gUseLuminanceAlpha = 0;
#  else
     /* Intensity texture format not in OpenGL 1.0; added by the EXT_texture
        extension and now part of OpenGL 1.1. */
     int gUseLuminanceAlpha = 1;
#  endif

#else
   /* OpenGL 1.1 case. */
   int gUseLuminanceAlpha = 0;
#endif

/* byte swap a 32-bit value */
#define SWAPL(x, n) { \
                 n = ((char *) (x))[0];\
                 ((char *) (x))[0] = ((char *) (x))[3];\
                 ((char *) (x))[3] = n;\
                 n = ((char *) (x))[1];\
                 ((char *) (x))[1] = ((char *) (x))[2];\
                 ((char *) (x))[2] = n; }

/* byte swap a short */
#define SWAPS(x, n) { \
                 n = ((char *) (x))[0];\
                 ((char *) (x))[0] = ((char *) (x))[1];\
                 ((char *) (x))[1] = n; }


CGLTexFont* CGLTexFont::fgDefaultTexFont(NULL);


CGLTexFont::CGLTexFont() : fTexobj(0), fTexWidth(0), fTexHeight(0),
	fMaxAscent(0), fMaxDescent(0), fNumGlyphs(0), fMinGlyph(0), 
	fRange(0), fTexImage(NULL), fTgi(NULL), fTgvi(NULL), fLut(NULL),
	fLastError(NULL), fInitialized(false)
{

}


CGLTexFont::~CGLTexFont()
{
	txfUnloadFont();
}

#pragma segment Main
void CGLTexFont::txfBindFontTexture()
{
	if (fInitialized)
	{
#if !defined(USE_DISPLAY_LISTS)
  		glBindTexture(GL_TEXTURE_2D, fTexobj);
#else
  		glCallList(fTexobj);
#endif
	}
}


#pragma segment Main
void CGLTexFont::txfGetStringMetrics(const char *string, long len, long &width,
  		long &max_ascent, long &max_descent)
{
 	if (fInitialized)
	{
 		long w = 0;
  		for (long i = 0; i < len; i++) 
		{
    		if (string[i] == 27) 
			{
      			switch (string[i + 1]) 
				{
		      		case 'M':
		        		i += 4;
		        		break;
		      		case 'T':
		        		i += 7;
		        		break;
		      		case 'L':
		        		i += 7;
		        		break;
		      		case 'F':
		        		i += 13;
		        		break;
		      	}
    		} 
			else 
			{
      			TexGlyphVertexInfo *tgvi = getTCVI(string[i]);
      			w += (long)tgvi->advance;
    		}
  		}
  		width = w;
  		max_ascent = fMaxAscent;
  		max_descent = fMaxDescent;
	}
}


#pragma segment Main
enum {
  MONO, TOP_BOTTOM, LEFT_RIGHT, FOUR
};

void CGLTexFont::txfRenderFancyString(const char *string, long len)
{
 	if (fInitialized)
	{
  		TexGlyphVertexInfo *tgvi;
  		GLubyte c[4][3];
  		long mode = MONO;

  		for (long i = 0; i < len; i++) 
		{
    		if (string[i] == 27) 
			{
     			switch (string[i + 1]) 
				{
      				case 'M':
	        			mode = MONO;
	        			::glColor3ubv((GLubyte *) & string[i + 2]);
	        			i += 4;
        				break;
      				case 'T':
        				mode = TOP_BOTTOM;
        				::memcpy(c, &string[i + 2], 6);
        				i += 7;
        				break;
			      	case 'L':
			        	mode = LEFT_RIGHT;
			        	::memcpy(c, &string[i + 2], 6);
			        	i += 7;
			        	break;
			      	case 'F':
			        	mode = FOUR;
			        	::memcpy(c, &string[i + 2], 12);
			        	i += 13;
			        	break;
      			}
    		} 
			else 
			{
		      	switch (mode) 
				{
		      		case MONO:
		        		txfRenderGlyph(string[i]);
		        		break;
		      		case TOP_BOTTOM:
		        		tgvi = getTCVI(string[i]);
		        		::glBegin(GL_QUADS);
		        		::glColor3ubv(c[0]);
		        		::glTexCoord2fv(tgvi->t0);
		        		::glVertex2sv(tgvi->v0);
		        		::glTexCoord2fv(tgvi->t1);
		        		::glVertex2sv(tgvi->v1);
		        		::glColor3ubv(c[1]);
		        		::glTexCoord2fv(tgvi->t2);
		        		::glVertex2sv(tgvi->v2);
		        		::glTexCoord2fv(tgvi->t3);
		        		::glVertex2sv(tgvi->v3);
		        		::glEnd();
		        		::glTranslatef(tgvi->advance, 0.0, 0.0);
		        		break;
		      		case LEFT_RIGHT:
		        		tgvi = getTCVI(string[i]);
		        		::glBegin(GL_QUADS);
		        		::glColor3ubv(c[0]);
		        		::glTexCoord2fv(tgvi->t0);
		        		::glVertex2sv(tgvi->v0);
		        		::glColor3ubv(c[1]);
		        		::glTexCoord2fv(tgvi->t1);
		        		::glVertex2sv(tgvi->v1);
		        		::glColor3ubv(c[1]);
		        		::glTexCoord2fv(tgvi->t2);
		        		::glVertex2sv(tgvi->v2);
		        		::glColor3ubv(c[0]);
		        		::glTexCoord2fv(tgvi->t3);
		        		::glVertex2sv(tgvi->v3);
		        		::glEnd();
		        		::glTranslatef(tgvi->advance, 0.0, 0.0);
		        		break;
		      		case FOUR:
		        		tgvi = getTCVI(string[i]);
		        		::glBegin(GL_QUADS);
		        		::glColor3ubv(c[0]);
		        		::glTexCoord2fv(tgvi->t0);
		        		::glVertex2sv(tgvi->v0);
		        		::glColor3ubv(c[1]);
		        		::glTexCoord2fv(tgvi->t1);
		        		::glVertex2sv(tgvi->v1);
		        		::glColor3ubv(c[2]);
		        		::glTexCoord2fv(tgvi->t2);
		        		::glVertex2sv(tgvi->v2);
		        		::glColor3ubv(c[3]);
		        		::glTexCoord2fv(tgvi->t3);
		        		::glVertex2sv(tgvi->v3);
		        		::glEnd();
		        		::glTranslatef(tgvi->advance, 0.0, 0.0);
		        		break;
      			}
    		}
  		}
	}
}


#pragma segment Main
void CGLTexFont::txfUnloadFont()
{
  	if (fTexImage) 
  	{
    	::free(fTexImage);
		fTexImage = NULL;
  	}

  	if (fTgi) 
	{
    	::free(fTgi);
		fTgi = NULL;
  	}
 
  	if (fTgvi) 
	{
    	::free(fTgvi);
		fTgvi = NULL;
  	}

  	if (fLut) 
	{
    	::free(fLut);
		fLut = NULL;
  	}
}


#pragma segment Main
void CGLTexFont::txfRenderString(const char *string, long len)
{
 	if (fInitialized)
	{
	  	int i;
	
	  	for (i = 0; i < len; i++) 
	  	{
	    	txfRenderGlyph(string[i]);
	  	}
	}
}


#pragma segment Main
void CGLTexFont::txfRenderGlyph(long c)
{
 	if (fInitialized)
	{
 		TexGlyphVertexInfo *tgvi = getTCVI(c);
		::glBegin(GL_QUADS);
		::glTexCoord2fv(tgvi->t0);
		::glVertex2sv(tgvi->v0);
		::glTexCoord2fv(tgvi->t1);
		::glVertex2sv(tgvi->v1);
		::glTexCoord2fv(tgvi->t2);
		::glVertex2sv(tgvi->v2);
		::glTexCoord2fv(tgvi->t3);
		::glVertex2sv(tgvi->v3);
		::glEnd();
		::glTranslatef(tgvi->advance, 0.0, 0.0);
	}	
}

void CGLTexFont::LoadFont(std::istream &in)
{
	char fileid[4];
	in.read(fileid, 4);
//	  	size_t  got = ::fread(fileid, 1, 4, file);
//		CMyError::Assert(got == 4 &&  !::strncmp(fileid, "\377txf", 4));

		/*
			  if (got != 4 || strncmp(fileid, "\377txf", 4)) {
			    lastError = "not a texture font file.";
			    goto error;
			  }
		*/
  		/*CONSTANTCONDITION*/
		//	  assert(sizeof(int) == 4);  /* Ensure external file format size. */
  	
	long endianness;

	in.read((char*)&endianness, sizeof(long));
  
	long swap = 0;
	if (endianness == 0x12345678) 
	{
		swap = 0;
	} 
	else if (endianness == 0x78563412) 
	{
	    swap = 1;
	} 
	else 
	{
   		CMyError::Assert(false, "not a texture font file.");
  	}
	
//		char* endOfFileMessage = "premature end of file.";

	long format;

	in.read((char*)&format, sizeof(format));
//	  	CMyError::Assert(got == 1, endOfFileMessage);
	in.read((char*)&fTexWidth, sizeof(fTexWidth));
//	  	CMyError::Assert(got == 1, endOfFileMessage);
	in.read((char*)&fTexHeight, sizeof(fTexHeight));
//	  	CMyError::Assert(got == 1, endOfFileMessage);
	in.read((char*)&fMaxAscent, sizeof(fMaxAscent));
//	  	CMyError::Assert(got == 1, endOfFileMessage);
	in.read((char*)&fMaxDescent, sizeof(fMaxDescent));
//	  	CMyError::Assert(got == 1, endOfFileMessage);
	in.read((char*)&fNumGlyphs, sizeof(fNumGlyphs));
//	  	CMyError::Assert(got == 1, endOfFileMessage);

  	if (swap) 
	{
		char tmp;
	    SWAPL(&format, tmp);
	    SWAPL(&fTexWidth, tmp);
	    SWAPL(&fTexHeight, tmp);
	    SWAPL(&fMaxAscent, tmp);
	    SWAPL(&fMaxDescent, tmp);
	    SWAPL(&fNumGlyphs, tmp);
  	}
	fTgi = (TexGlyphInfo *) malloc(fNumGlyphs * sizeof(TexGlyphInfo));
	CMyError::ThrowErrorIfNULL (fTgi, "out of memory.");

  		/*CONSTANTCONDITION*/
  	CMyError::Assert(sizeof(TexGlyphInfo) == 12);  /* Ensure external file format size. */

	in.read((char*)fTgi, sizeof(TexGlyphInfo) * fNumGlyphs);
	
//	  	got = fread(fTgi, sizeof(TexGlyphInfo), fNumGlyphs, file);
//	  	CMyError::Assert(got == fNumGlyphs, endOfFileMessage);

  	if (swap) 
	{
    	for (long i = 0; i < fNumGlyphs; i++) 
		{
			char tmp;
      		SWAPS(&fTgi[i].c, tmp);
      		SWAPS(&fTgi[i].x, tmp);
      		SWAPS(&fTgi[i].y, tmp);
    	}
  	}
  	fTgvi = (TexGlyphVertexInfo *)
    		malloc(fNumGlyphs * sizeof(TexGlyphVertexInfo));
	CMyError::ThrowErrorIfNULL (fTgvi, "out of memory.");

  	GLfloat w = fTexWidth;
  	GLfloat h = fTexHeight;
  	GLfloat xstep = 0.5 / w;
  	GLfloat ystep = 0.5 / h;
  	for (long i = 0; i < fNumGlyphs; i++) {
    	TexGlyphInfo *tgi;

    	tgi = &fTgi[i];
    	fTgvi[i].t0[0] = tgi->x / w + xstep;
    	fTgvi[i].t0[1] = tgi->y / h + ystep;
    	fTgvi[i].v0[0] = tgi->xoffset;
    	fTgvi[i].v0[1] = tgi->yoffset;
    	fTgvi[i].t1[0] = (tgi->x + tgi->width) / w + xstep;
    	fTgvi[i].t1[1] = tgi->y / h + ystep;
   		fTgvi[i].v1[0] = tgi->xoffset + tgi->width;
    	fTgvi[i].v1[1] = tgi->yoffset;
    	fTgvi[i].t2[0] = (tgi->x + tgi->width) / w + xstep;
    	fTgvi[i].t2[1] = (tgi->y + tgi->height) / h + ystep;
    	fTgvi[i].v2[0] = tgi->xoffset + tgi->width;
    	fTgvi[i].v2[1] = tgi->yoffset + tgi->height;
    	fTgvi[i].t3[0] = tgi->x / w + xstep;
    	fTgvi[i].t3[1] = (tgi->y + tgi->height) / h + ystep;
    	fTgvi[i].v3[0] = tgi->xoffset;
    	fTgvi[i].v3[1] = tgi->yoffset + tgi->height;
    	fTgvi[i].advance = tgi->advance;
  	}

  	long min_glyph = fTgi[0].c;
  	long max_glyph = fTgi[0].c;
  	for (long i = 1; i < fNumGlyphs; i++) 
	{
    	if (fTgi[i].c < min_glyph) 
		{
      		min_glyph = fTgi[i].c;
    	}
    	if (fTgi[i].c > max_glyph) 
		{
      		max_glyph = fTgi[i].c;
    	}
  	}
  	fMinGlyph = min_glyph;
  	fRange = max_glyph - min_glyph + 1;

  	fLut = (TexGlyphVertexInfo **)
    		calloc(fRange, sizeof(TexGlyphVertexInfo *));
	CMyError::ThrowErrorIfNULL (fLut, "out of memory.");


  	for (long i = 0; i < fNumGlyphs; i++) 
	{
    	fLut[fTgi[i].c - fMinGlyph] = &fTgvi[i];
  	}

  	switch (format) 
	{
  		case TXF_FORMAT_BYTE:
    		if (gUseLuminanceAlpha) 
			{
      			unsigned char *orig = (unsigned char *) malloc(fTexWidth * fTexHeight);
				CMyError::ThrowErrorIfNULL (fLut, "out of memory.");
				
				try
				{
					in.read((char*)orig, fTexWidth * fTexHeight);
//	     			got = fread(orig, 1, fTexWidth * fTexHeight, file);
//	  				CMyError::Assert(got == fTexWidth * fTexHeight, endOfFileMessage);
	      			fTexImage = (unsigned char *)
	        				malloc(2 * fTexWidth * fTexHeight);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

			      	for (long i = 0; i < fTexWidth * fTexHeight; i++) 
				  	{
			        	fTexImage[i * 2] = orig[i];
			        	fTexImage[i * 2 + 1] = orig[i];
			      	}
      			}
      			catch (...)
      			{
					free(orig);
      				throw;
      			}
				free(orig);
    		} 
			else 
			{
      			fTexImage = (unsigned char *)
       						 malloc(fTexWidth * fTexHeight);
				CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

				in.read((char*)fTexImage, fTexWidth * fTexHeight);
      //		got = fread(fTexImage, 1, fTexWidth * fTexHeight, file);
  	  //		CMyError::Assert(got == fTexWidth * fTexHeight, endOfFileMessage);
    		}
    		break;
  		case TXF_FORMAT_BITMAP:
    		long width = fTexWidth;
    		long height = fTexHeight;
    		long stride = (width + 7) >> 3;
    		unsigned char *texbitmap = (unsigned char *) malloc(stride * height);
			CMyError::ThrowErrorIfNULL (texbitmap, "out of memory.");

			try
			{
				in.read((char*)texbitmap,  stride * height);
//	    		got = fread(texbitmap, 1, stride * height, file);
//	  			CMyError::Assert(got == stride * height, endOfFileMessage);
	    		if (gUseLuminanceAlpha) 
				{
	      			fTexImage = (unsigned char *) calloc(width * height * 2, 1);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

	      			for (long i = 0; i < height; i++) 
					{
	        			for (long j = 0; j < width; j++) 
						{
	          				if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) 
							{
	            				fTexImage[(i * width + j) * 2] = 255;
	            				fTexImage[(i * width + j) * 2 + 1] = 255;
	          				}
	        			}
	      			}
	    		} 
				else 
				{
	      			fTexImage = (unsigned char *) calloc(width * height, 1);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

	      			for (long i = 0; i < height; i++) 
					{
	        			for (long j = 0; j < width; j++) 
						{
	          				if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) 
							{
	            				fTexImage[i * width + j] = 255;
	          				}
	        			}
	      			}
	    		}
	    	}
	    	catch (...)
	    	{
    			free(texbitmap);
    			throw;
    		}
    		free(texbitmap);
    		break;
  	}
	
	fInitialized = true;
}

#pragma segment Main
void CGLTexFont::LoadFontFromArray(const char* array, long size)
{
	std::string aString(array, size);
	std::istringstream	input(aString);
	
	LoadFont(input);
}


#pragma segment 
void CGLTexFont::SetDefaultTexFont(const char *array, long length)
{
	if (fgDefaultTexFont != NULL)
	{
		delete fgDefaultTexFont;
		fgDefaultTexFont = NULL;
	}
	
	fgDefaultTexFont = new CGLTexFont ();
	
	try
	{
		fgDefaultTexFont->LoadFontFromArray (array, length);
		fgDefaultTexFont->txfEstablishTexture(0, GL_TRUE);
		
	}
	catch(...)
	{
		delete fgDefaultTexFont;
		fgDefaultTexFont = NULL;
		throw;
	}
}



void CGLTexFont::LoadFontFromFile(char *filename)
{

#if	1
	std::ifstream in;
	in.open(filename);
	fInitialized = false;
	try
	{
		LoadFont(in);
	}
	catch (...)
	{
		in.close();
		
		throw;
	}

	in.close();
	
#else	
  	FILE *file = ::fopen(filename, "rb");
	try 
	{
		CMyError::ThrowErrorIfNULL (file, "file open failed.");

		char fileid[4];
	  	size_t  got = ::fread(fileid, 1, 4, file);
		CMyError::Assert(got == 4 &&  !::strncmp(fileid, "\377txf", 4));

			/*
				  if (got != 4 || strncmp(fileid, "\377txf", 4)) {
				    lastError = "not a texture font file.";
				    goto error;
				  }
			*/
	  		/*CONSTANTCONDITION*/
			//	  assert(sizeof(int) == 4);  /* Ensure external file format size. */
	  	
		long endianness;
		got = ::fread(&endianness, sizeof(long), 1, file);
	  
		long swap;
		if (got == 1 && endianness == 0x12345678) 
		{
			swap = 0;
		} 
		else if (got == 1 && endianness == 0x78563412) 
		{
		    swap = 1;
		} 
		else 
		{
	   		CMyError::Assert(false, "not a texture font file.");
	  	}
		
		char* endOfFileMessage = "premature end of file.";

		long format;	  	
		got = fread(&format, sizeof(format), 1, file);
	  	CMyError::Assert(got == 1, endOfFileMessage);
	  	got = fread(&fTexWidth, sizeof(fTexWidth), 1, file);
	  	CMyError::Assert(got == 1, endOfFileMessage);
	  	got = fread(&fTexHeight, sizeof(fTexHeight), 1, file);
	  	CMyError::Assert(got == 1, endOfFileMessage);
	  	got = fread(&fMaxAscent, sizeof(fMaxAscent), 1, file);
	  	CMyError::Assert(got == 1, endOfFileMessage);
	  	got = fread(&fMaxDescent, sizeof(fMaxDescent), 1, file);
	  	CMyError::Assert(got == 1, endOfFileMessage);
	  	got = fread(&fNumGlyphs, sizeof(fNumGlyphs), 1, file);
	  	CMyError::Assert(got == 1, endOfFileMessage);
	
	  	if (swap) 
		{
			char tmp;
		    SWAPL(&format, tmp);
		    SWAPL(&fTexWidth, tmp);
		    SWAPL(&fTexHeight, tmp);
		    SWAPL(&fMaxAscent, tmp);
		    SWAPL(&fMaxDescent, tmp);
		    SWAPL(&fNumGlyphs, tmp);
	  	}
		fTgi = (TexGlyphInfo *) malloc(fNumGlyphs * sizeof(TexGlyphInfo));
		CMyError::ThrowErrorIfNULL (fTgi, "out of memory.");

	  		/*CONSTANTCONDITION*/
	  	CMyError::Assert(sizeof(TexGlyphInfo) == 12);  /* Ensure external file format size. */
	  	got = fread(fTgi, sizeof(TexGlyphInfo), fNumGlyphs, file);
	  	CMyError::Assert(got == fNumGlyphs, endOfFileMessage);
	
	  	if (swap) 
		{
	    	for (long i = 0; i < fNumGlyphs; i++) 
			{
				char tmp;
	      		SWAPS(&fTgi[i].c, tmp);
	      		SWAPS(&fTgi[i].x, tmp);
	      		SWAPS(&fTgi[i].y, tmp);
	    	}
	  	}
	  	fTgvi = (TexGlyphVertexInfo *)
	    		malloc(fNumGlyphs * sizeof(TexGlyphVertexInfo));
		CMyError::ThrowErrorIfNULL (fTgvi, "out of memory.");

	  	GLfloat w = fTexWidth;
	  	GLfloat h = fTexHeight;
	  	GLfloat xstep = 0.5 / w;
	  	GLfloat ystep = 0.5 / h;
	  	for (long i = 0; i < fNumGlyphs; i++) {
	    	TexGlyphInfo *tgi;
	
	    	tgi = &fTgi[i];
	    	fTgvi[i].t0[0] = tgi->x / w + xstep;
	    	fTgvi[i].t0[1] = tgi->y / h + ystep;
	    	fTgvi[i].v0[0] = tgi->xoffset;
	    	fTgvi[i].v0[1] = tgi->yoffset;
	    	fTgvi[i].t1[0] = (tgi->x + tgi->width) / w + xstep;
	    	fTgvi[i].t1[1] = tgi->y / h + ystep;
	   		fTgvi[i].v1[0] = tgi->xoffset + tgi->width;
	    	fTgvi[i].v1[1] = tgi->yoffset;
	    	fTgvi[i].t2[0] = (tgi->x + tgi->width) / w + xstep;
	    	fTgvi[i].t2[1] = (tgi->y + tgi->height) / h + ystep;
	    	fTgvi[i].v2[0] = tgi->xoffset + tgi->width;
	    	fTgvi[i].v2[1] = tgi->yoffset + tgi->height;
	    	fTgvi[i].t3[0] = tgi->x / w + xstep;
	    	fTgvi[i].t3[1] = (tgi->y + tgi->height) / h + ystep;
	    	fTgvi[i].v3[0] = tgi->xoffset;
	    	fTgvi[i].v3[1] = tgi->yoffset + tgi->height;
	    	fTgvi[i].advance = tgi->advance;
	  	}
	
	  	long min_glyph = fTgi[0].c;
	  	long max_glyph = fTgi[0].c;
	  	for (long i = 1; i < fNumGlyphs; i++) 
		{
	    	if (fTgi[i].c < min_glyph) 
			{
	      		min_glyph = fTgi[i].c;
	    	}
	    	if (fTgi[i].c > max_glyph) 
			{
	      		max_glyph = fTgi[i].c;
	    	}
	  	}
	  	fMinGlyph = min_glyph;
	  	fRange = max_glyph - min_glyph + 1;
	
	  	fLut = (TexGlyphVertexInfo **)
	    		calloc(fRange, sizeof(TexGlyphVertexInfo *));
		CMyError::ThrowErrorIfNULL (fLut, "out of memory.");


	  	for (long i = 0; i < fNumGlyphs; i++) 
		{
	    	fLut[fTgi[i].c - fMinGlyph] = &fTgvi[i];
	  	}
	
	  	switch (format) 
		{
	  		case TXF_FORMAT_BYTE:
	    		if (gUseLuminanceAlpha) 
				{
	      			unsigned char *orig = (unsigned char *) malloc(fTexWidth * fTexHeight);
					CMyError::ThrowErrorIfNULL (fLut, "out of memory.");

	     			got = fread(orig, 1, fTexWidth * fTexHeight, file);
	  				CMyError::Assert(got == fTexWidth * fTexHeight, endOfFileMessage);
	      			fTexImage = (unsigned char *)
	        				malloc(2 * fTexWidth * fTexHeight);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

			      	for (long i = 0; i < fTexWidth * fTexHeight; i++) 
				  	{
			        	fTexImage[i * 2] = orig[i];
			        	fTexImage[i * 2 + 1] = orig[i];
			      	}
	      			
					free(orig);
	    		} 
				else 
				{
	      			fTexImage = (unsigned char *)
	       						 malloc(fTexWidth * fTexHeight);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

	      			got = fread(fTexImage, 1, fTexWidth * fTexHeight, file);
	  				CMyError::Assert(got == fTexWidth * fTexHeight, endOfFileMessage);
	    		}
	    		break;
	  		case TXF_FORMAT_BITMAP:
	    		long width = fTexWidth;
	    		long height = fTexHeight;
	    		long stride = (width + 7) >> 3;
	    		unsigned char *texbitmap = (unsigned char *) malloc(stride * height);
				CMyError::ThrowErrorIfNULL (texbitmap, "out of memory.");

	    		got = fread(texbitmap, 1, stride * height, file);
	  			CMyError::Assert(got == stride * height, endOfFileMessage);
	    		if (gUseLuminanceAlpha) 
				{
	      			fTexImage = (unsigned char *) calloc(width * height * 2, 1);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

	      			for (long i = 0; i < height; i++) 
					{
	        			for (long j = 0; j < width; j++) 
						{
	          				if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) 
							{
	            				fTexImage[(i * width + j) * 2] = 255;
	            				fTexImage[(i * width + j) * 2 + 1] = 255;
	          				}
	        			}
	      			}
	    		} 
				else 
				{
	      			fTexImage = (unsigned char *) calloc(width * height, 1);
					CMyError::ThrowErrorIfNULL (fTexImage, "out of memory.");

	      			for (long i = 0; i < height; i++) 
					{
	        			for (long j = 0; j < width; j++) 
						{
	          				if (texbitmap[i * stride + (j >> 3)] & (1 << (j & 7))) 
							{
	            				fTexImage[i * width + j] = 255;
	          				}
	        			}
	      			}
	    		}
	    		free(texbitmap);
	    		break;
	  	}
	}
	catch (CMyError e)
	{
  		fclose(file);
		
		throw;
	}
 	
	fclose(file);
#endif
 /*
error:

  if (txf) {
    if (fTgi)
      free(fTgi);
    if (txf->tgvi)
      free(txf->tgvi);
    if (fLut)
      free(fLut);
    if (fTexImage)
      free(fTexImage);
    free(txf);
  } 
  if (file)
    fclose(file);
  return NULL; */
}


#pragma segment Main
GLuint CGLTexFont::txfEstablishTexture(GLuint texobj, GLboolean setupMipmaps)
{
	if (fInitialized)
	{
  		if (fTexobj == NULL) 
		{
    		if (texobj == 0) 
			{
#if !defined(USE_DISPLAY_LISTS)
      			::glGenTextures(1, &fTexobj);
#else
      			fTexobj = glGenLists(1);
#endif
    		} 
			else 
			{
      			fTexobj = texobj;
    		}
  		}
#if !defined(USE_DISPLAY_LISTS)
  		::glBindTexture(GL_TEXTURE_2D, fTexobj);
#else
  		::glNewList(fTexobj, GL_COMPILE);
#endif

#if 1
  /* XXX Indigo2 IMPACT in IRIX 5.3 and 6.2 does not support the GL_INTENSITY
     internal texture format. Sigh. Win32 non-GLX users should disable this
     code. */
  		if (gUseLuminanceAlpha == 0) 
		{
    		char *renderer = (char *) ::glGetString(GL_RENDERER);
    		char *vendor = (char *) ::glGetString(GL_VENDOR);
    		if (!::strcmp(vendor, "SGI") && !::strncmp(renderer, "IMPACT", 6)) 
			{
      			char *version = (char *) glGetString(GL_VERSION);
      			if (!::strcmp(version, "1.0 Irix 6.2") || !::strcmp(version, "1.0 Irix 5.3")) 
				{
        			long width = fTexWidth;
        			long height = fTexHeight;
 
       				 gUseLuminanceAlpha = 1;
       				 unsigned char *latex = (unsigned char *) ::calloc(width * height * 2, 1);
        			CMyError::ThrowErrorIfNULL (latex, "out of memory");
		
		
        			for (long i = 0; i < height * width; i++) 
					{
          				latex[i * 2] = fTexImage[i];
          				latex[i * 2 + 1] = fTexImage[i];
        			}
        			::free(fTexImage);
        			fTexImage = latex;
      			}
    		}
  		}
#endif

  		if (gUseLuminanceAlpha) 
		{
    		if (setupMipmaps) 
			{
      			::gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE_ALPHA,fTexWidth, fTexHeight,
        						GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, fTexImage);
    		} 
			else 
			{
      			::glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, fTexWidth, fTexHeight, 0,
        					GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, fTexImage);
    		}
  		} 
		else 
		{
#if defined(GL_VERSION_1_1) || defined(GL_EXT_texture)
		/* Use GL_INTENSITY4 as internal texture format since we want to use as
		   little texture memory as possible. */
			if (setupMipmaps) 
			{
			  	::gluBuild2DMipmaps(GL_TEXTURE_2D, GL_INTENSITY4,
			    		fTexWidth, fTexHeight,
			    		GL_LUMINANCE, GL_UNSIGNED_BYTE, fTexImage);
			} 
/*			else 
			{
			  	::glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY4,
			    		fTexWidth, fTexHeight, 0,
			    		GL_LUMINANCE, GL_UNSIGNED_BYTE, fTexImage);
			}
*/
#else
			CMyError::ThrowErrorIfNULL (NULL, "unsupported version of Opengl:");
 
#endif
  		}

#if defined(USE_DISPLAY_LISTS)
  		::glEndList();
  		::glCallList(fTexobj);
#endif
	}
  	
	return fTexobj;
}


#pragma segment Main
const char *CGLTexFont::txfErrorString(void)
{
	return fLastError;
}


TexGlyphVertexInfo *CGLTexFont::getTCVI(int c)
{
	
  	TexGlyphVertexInfo *tgvi = NULL;
	
	if (fInitialized)
	{
	  /* Automatically substitute uppercase letters with lowercase if not
	     uppercase available (and vice versa). */
	  	if ((c >= fMinGlyph) && (c < fMinGlyph + fRange)) 
		{
	    	tgvi = fLut[c - fMinGlyph];
	    	if (tgvi) 
			{
	      		return tgvi;
	   		}
	    	if (islower(c)) 
			{
	     	 	c = toupper(c);
	      		if ((c >= fMinGlyph) && (c < fMinGlyph +fRange)) 
				{
	        		return fLut[c - fMinGlyph];
	      		}
	    	}
	    	if (isupper(c)) 
			{
	      		c = tolower(c);
	      		if ((c >= fMinGlyph) && (c < fMinGlyph +fRange)) 
				{
	        		return fLut[c - fMinGlyph];
	      		}
	    	}
	  	}
	  	else
	  	{
	  		char string[256];
		  	sprintf(string, "texfont: tried to access unavailable font character \"%c\" (%d)\n",
		    	isprint(c) ? c : ' ', c);
			CMyError::ThrowErrorIfNULL (tgvi, string);
		}
	  	/* NOTREACHED */
	}
	
  	return NULL;

}

#pragma segment Main
CGLTexFont *CGLTexFont::GetDefaultTexFont()
{
	if (fgDefaultTexFont == NULL)
	{
		//	this is default initialization, applications should call SetDefaultTexFont (array, length);
		fgDefaultTexFont = new CGLTexFont();
		fgDefaultTexFont->LoadFontFromFile ("default.txf");
		fgDefaultTexFont->txfEstablishTexture(0, GL_TRUE);
	}
	
	return fgDefaultTexFont;
	
}

