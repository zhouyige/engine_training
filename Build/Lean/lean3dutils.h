#pragma once

#include "Lean3D.h"

#ifndef DLL
#	if defined( WIN32 ) || defined( _WINDOWS )
#		define DLL extern "C" __declspec( dllimport )
#	else
#  if defined( __GNUC__ ) && __GNUC__ >= 4
#   define DLL extern "C" __attribute__ ((visibility("default")))
#  else
#		define DLL extern "C"
#  endif
#	endif
#endif

const int leanutMaxStatMode = 2;


DLL void leanutFreeMem( char **ptr );

DLL bool leanutDumpMessages();


DLL bool leanutInitOpenGL( int hDC );

DLL void leanutReleaseOpenGL();

DLL void leanutSwapBuffers();


DLL const char *leanutGetResourcePath( int type );

DLL void leanutSetResourcePath( int type, const char *path );

DLL bool leanutLoadResourcesFromDisk( const char *contentDir );

DLL ResHandle leanutCreateGeometryRes( const char *name, int numVertices, int numTriangleIndices, 
								   float *posData, unsigned int *indexData, short *normalData,
								   short *tangentData, short *bitangentData, 
								   float *texData1, float *texData2 );

DLL bool leanutCreateTGAImage( const unsigned char *pixels, int width, int height, int bpp,
                              char **outData, int *outSize );

DLL bool leanutScreenshot( const char *filename );


DLL void leanutPickRay( NodeHandle cameraNode, float nwx, float nwy, float *ox, float *oy, float *oz,
                       float *dx, float *dy, float *dz );

DLL NodeHandle leanutPickNode( NodeHandle cameraNode, float nwx, float nwy );

DLL void leanutShowText( const char *text, float x, float y, float size,
                        float colR, float colG, float colB, ResHandle fontMaterialRes );

DLL void leanutShowFrameStats( ResHandle fontMaterialRes, ResHandle panelMaterialRes, int mode );
