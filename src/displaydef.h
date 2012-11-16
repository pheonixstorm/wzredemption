/*
 * DisplayDef.h
 *
 * Definitions of the display structures
 *
 */
#ifndef _displaydef_h
#define _displaydef_h

#include "imd.h"
#include "pieclip.h"
#define DISP_WIDTH		(pie_GetVideoBufferWidth()) 
#define DISP_HEIGHT		(pie_GetVideoBufferHeight())
#define DISP_HARDBITDEPTH	(16)
#define DISP_BITDEPTH	(8)
#define	BOUNDARY_X			(16)
#define BOUNDARY_Y			(16)
//#define BOUNDARY_X		(DISP_WIDTH/20)	   // proportional to resolution - Alex M
//#define	BOUNDARY_Y		(DISP_WIDTH/16)
//#define	BOUNDARY_X		(24)
//#define	BOUNDARY_Y		(24)

typedef struct _screen_disp_data
{
	//UDWORD		dummy;
	iIMDShape	*imd;
//	BOOL		drawnThisFrame;		// for sorting - have we drawn the imd already?
	UDWORD		frameNumber;		// last frame it was drawn
//	UDWORD		animFrame;			// anim Frame
//	SDWORD		bucketDepth;
//	BOOL		onScreen;
//	UDWORD		numTiles;
	UDWORD		screenX,screenY;
	UDWORD		screenR; 
} SCREEN_DISP_DATA;

#endif

