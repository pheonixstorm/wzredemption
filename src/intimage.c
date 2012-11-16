/*
 * Image.c
 *
 * Image definitions and related functions.
 *
 */

#include <stdio.h>
#include <math.h>

#include "frame.h"
#include "widget.h"

#include "objects.h"
#include "loop.h"
#include "edit2d.h"
#include "map.h"
/* Includes direct access to render library */
#include "ivisdef.h"
#include "piestate.h"

#include "piemode.h"

#include "vid.h"
#include "bitimage.h"

#include "display3d.h"
#include "edit3d.h"
#include "disp2d.h"
#include "structure.h"
#include "research.h"
#include "function.h"
#include "gtime.h"
#include "hci.h"
#include "stats.h"
#include "game.h"
#include "power.h"
#include "audio.h"
#include "audio_id.h"
#include "widgint.h"
#include "bar.h"
#include "form.h"
#include "label.h"
#include "button.h"
#include "editbox.h"
#include "slider.h"
#include "fractions.h"
#include "order.h"
#include "winmain.h"

#include "intimage.h"


#define TRANSRECT

static BOOL	EnableLocks = TRUE;
static SDWORD LockRefs = 0;

IMAGEFILE *IntImages;	// All the 2d graphics for the user interface.

// Form frame definitions.
IMAGEFRAME FrameNormal = {
	0,0, 0,0,
	IMAGE_FRAME_C0,
	IMAGE_FRAME_C1,
	IMAGE_FRAME_C3,
	IMAGE_FRAME_C2,
	IMAGE_FRAME_HT, FR_SOLID,
	IMAGE_FRAME_VR, FR_SOLID,
	IMAGE_FRAME_HB, FR_SOLID,
	IMAGE_FRAME_VL, FR_SOLID,
	{{FR_FRAME,	0,1, 0,-1 ,190},
	{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0}},
};

IMAGEFRAME FrameRadar = {
	0,0, 0,0,
	IMAGE_FRAME_C0,
	IMAGE_FRAME_C1,
	IMAGE_FRAME_C3,
	IMAGE_FRAME_C2,
	IMAGE_FRAME_HT, FR_SOLID,
	IMAGE_FRAME_VR, FR_SOLID,
	IMAGE_FRAME_HB, FR_SOLID,
	IMAGE_FRAME_VL, FR_SOLID,
	{{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0},
	{FR_IGNORE, 0,0, 0,0 ,0}},
};
//IMAGEFRAME FrameObject = {
//	0,0, 0,0,
//	-1,
//	-1,
//	-1,
//	-1,
//	-1, FR_SOLID,
//	-1, FR_SOLID,
//	-1, FR_SOLID,
//	-1, FR_SOLID,
//	{{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0}},
//};
//
//IMAGEFRAME FrameStats = {
//	0,0, 0,0,
//	-1,
//	-1,
//	-1,
//	-1,
//	IMAGE_FRAME_HT, FR_SOLID,
//	IMAGE_FRAME_VR, FR_SOLID,
//	IMAGE_FRAME_HB, FR_SOLID,
//	IMAGE_FRAME_VL, FR_SOLID,
//	{{FR_FRAME, 8,3, -6,-5 ,190},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0}},
//};
//
//IMAGEFRAME FrameDesignView = {
//	0,0, 0,0,
//	IMAGE_FRAME_VC0,
//	IMAGE_FRAME_VC1,
//	IMAGE_FRAME_VC2,
//	IMAGE_FRAME_VC3,
//	IMAGE_FRAME_HT2, FR_SOLID,
//	IMAGE_FRAME_VR2, FR_SOLID,
//	IMAGE_FRAME_HB2, FR_SOLID,
//	IMAGE_FRAME_VL2, FR_SOLID,
//	{{FR_FRAME, 0,0, 0,0, 1},
//	{FR_FRAME, 0,0, 0,0 ,1},
//	{FR_FRAME, 0,0, 0,0 ,1},
//	{FR_FRAME, 0,0, 0,0 ,1},
//	{FR_FRAME, 0,0, 0,0 ,1}},
//};
//
//IMAGEFRAME FrameDesignHilight = {
//	0,0, 0,0,
//	IMAGE_FRAME_HC0,
//	IMAGE_FRAME_HC1,
//	IMAGE_FRAME_HC2,
//	IMAGE_FRAME_HC3,
//	IMAGE_FRAME_HTH, FR_SOLID,
//	IMAGE_FRAME_VRH, FR_SOLID,
//	IMAGE_FRAME_HBH, FR_SOLID,
//	IMAGE_FRAME_VLH, FR_SOLID,
//	{{FR_FRAME, 0,0, 0,0, 1},
//	{FR_FRAME, 0,0, 0,0 ,1},
//	{FR_FRAME, 0,0, 0,0 ,1},
//	{FR_FRAME, 0,0, 0,0 ,1},
//	{FR_FRAME, 0,0, 0,0 ,1}},
//};
//
//IMAGEFRAME FrameText = {
//	0,0, 0,0,
//	-1,
//	-1,
//	IMAGE_FRAME_C3,
//	IMAGE_FRAME_C2,
//	-1, FR_SOLID,
//	IMAGE_FRAME_VR, FR_SOLID,
//	IMAGE_FRAME_HB, FR_SOLID,
//	IMAGE_FRAME_VL, FR_SOLID,
//	{{FR_FRAME,	0,1, 0,-1 ,224},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0},
//	{FR_IGNORE, 0,0, 0,0 ,0}},
//};

// Tab definitions, defines graphics to use for major and minor tabs.
TABDEF	StandardTab = {
	IMAGE_TAB1,			// Major tab normal.
	IMAGE_TAB1DOWN,		// Major tab clicked.
	IMAGE_TABHILIGHT,	// Major tab hilighted by mouse.
	IMAGE_TABSELECTED,	// Major tab currently selected.

	IMAGE_TAB1,			// Minor tab tab Normal.
	IMAGE_TAB1DOWN,		// Minor tab clicked.
	IMAGE_TABHILIGHT,	// Minor tab hilighted by mouse.
	IMAGE_TABSELECTED,	// Minor tab currently selected.
};
TABDEF SystemTab = {
	IMAGE_DES_WEAPONS,
	IMAGE_DES_WEAPONSDOWN,
	IMAGE_DES_EXTRAHI,
	IMAGE_DES_WEAPONSDOWN,

	/*IMAGE_TAB1,
	IMAGE_TAB1DOWN,
	IMAGE_TABHILIGHT,
	IMAGE_TABSELECTED,*/
	IMAGE_SIDETAB,
	IMAGE_SIDETABDOWN,
	IMAGE_SIDETABHI,
	IMAGE_SIDETABSEL,
};

TABDEF	SmallTab = {
	IMAGE_TAB1_SM,			// Major tab normal.
	IMAGE_TAB1DOWN_SM,		// Major tab clicked.
	IMAGE_TABHILIGHT_SM,	// Major tab hilighted by mouse.
	IMAGE_TAB1SELECTED_SM,	// Major tab currently selected.

	IMAGE_TAB1_SM,			// Minor tab tab Normal.
	IMAGE_TAB1DOWN_SM,		// Minor tab clicked.
	IMAGE_TABHILIGHT_SM,	// Minor tab hilighted by mouse.
	IMAGE_TAB1SELECTED_SM,	// Minor tab currently selected.
};

// Read bitmaps used by the interface.
//
BOOL imageInitBitmaps(void)
{
  	IntImages = (IMAGEFILE*)resGetData("IMG","intfac.img");
//	IntImages = iV_LoadImageFile("intpc.img");

	return TRUE;
}

void imageDeleteBitmaps(void)
{
//	iV_FreeImageFile(IntImages);
}


void DrawEnableLocks(BOOL Enable)
{
	EnableLocks = Enable;
}


void DrawBegin(void)
{
	if(EnableLocks) {
		if(LockRefs == 0) {
			pie_LocalRenderBegin();
		}

		LockRefs++;
	}
}


void DrawEnd(void)
{
	if(EnableLocks) {
		LockRefs--;

		ASSERT((LockRefs >= 0,"Inbalanced DrawEnd()"));

		if(LockRefs == 0) {
			pie_LocalRenderEnd();
		}
	}
}

void RenderWindowFrame(IMAGEFRAME *Frame,UDWORD x,UDWORD y,UDWORD Width,UDWORD Height)
{
	RenderWindow(Frame,x,y,Width,Height,FALSE);
}

void RenderOpaqueWindow(IMAGEFRAME *Frame,UDWORD x,UDWORD y,UDWORD Width,UDWORD Height)
{
	RenderWindow(Frame,x,y,Width,Height,TRUE);
}



#define INCEND	(0)

// Render a window frame.
//
void RenderWindow(IMAGEFRAME *Frame,UDWORD x,UDWORD y,UDWORD Width,UDWORD Height,BOOL Opaque)
{
	SWORD WTopRight = 0;
	SWORD WTopLeft = 0;
	SWORD WBottomRight = 0;
	SWORD WBottomLeft = 0;
	SWORD HTopRight = 0;
	SWORD HTopLeft = 0;
	SWORD HBottomRight = 0;
	SWORD HBottomLeft = 0;
	UWORD RectI;
	FRAMERECT *Rect;
	BOOL Masked = FALSE;

	x += Frame->OffsetX0;
	y += Frame->OffsetY0;
	Width -= Frame->OffsetX1+Frame->OffsetX0;
	Height -= Frame->OffsetY1+Frame->OffsetY0;

	for(RectI=0; RectI<5; RectI++) {
		Rect = &Frame->FRect[RectI];

//		if(Opaque==FALSE) {
//			screenSetFillCacheColour(Rect->ColourIndex);
//		}

		switch(Rect->Type) {
			case FR_FRAME:
				if(Opaque==FALSE)
				{
					if(Masked == FALSE) {
						Width &= 0xfffc;	// Software transboxfill needs to be a multiple of 4 pixels.
						Masked = TRUE;
					}

					if (pie_GetRenderEngine() == ENGINE_GLIDE)
					{
						iV_UniTransBoxFill( x+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset,
										(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE, FILLTRANS);
					}
					else
					{
						iV_TransBoxFill( x+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset);
					}
				}
				else
				{
					pie_BoxFillIndex( x+Rect->TLXOffset,
								y+Rect->TLYOffset,
								x+Width-INCEND+Rect->BRXOffset,
								y+Height-INCEND+Rect->BRYOffset,Rect->ColourIndex);
				}
				break;

			case FR_LEFT:
				if(Opaque==FALSE) {
					if(Masked == FALSE) {
						Width &= 0xfffc;	// Software transboxfill needs to be a multiple of 4 pixels.
						Masked = TRUE;
					}

					if (pie_GetRenderEngine() == ENGINE_GLIDE)
					{
						iV_UniTransBoxFill(x+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset,
										(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE, FILLTRANS);
					} else 
					{
						iV_TransBoxFill( x+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset);
					}
				} else {
					iV_BoxFill( x+Rect->TLXOffset,
								y+Rect->TLYOffset,
								x+Rect->BRXOffset,
								y+Height-INCEND+Rect->BRYOffset,Rect->ColourIndex);
				}
				break;

			case FR_RIGHT:
				if(Opaque==FALSE) {
					if(Masked == FALSE) {
						Width &= 0xfffc;	// Software transboxfill needs to be a multiple of 4 pixels.
						Masked = TRUE;
					}
					if (pie_GetRenderEngine() == ENGINE_GLIDE)
					{
						iV_UniTransBoxFill( x+Width-INCEND+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset,
										(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE, FILLTRANS);
					} else 
					{
						iV_TransBoxFill( x+Width-INCEND+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset);
					}
				} else {
					iV_BoxFill( x+Width-INCEND+Rect->TLXOffset,
								y+Rect->TLYOffset,
								x+Width-INCEND+Rect->BRXOffset,
								y+Height-INCEND+Rect->BRYOffset,Rect->ColourIndex);
				}
				break;
			
			case FR_TOP:
				if(Opaque==FALSE) {
					if(Masked == FALSE) {
						Width &= 0xfffc;	// Software transboxfill needs to be a multiple of 4 pixels.
						Masked = TRUE;
					}
					if (pie_GetRenderEngine() == ENGINE_GLIDE)
					{
						iV_UniTransBoxFill( x+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Rect->BRYOffset,
										(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE, FILLTRANS);
					}else
					  {
						iV_TransBoxFill( x+Rect->TLXOffset,
										y+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Rect->BRYOffset);
					}
				} else {
					iV_BoxFill( x+Rect->TLXOffset,
								y+Rect->TLYOffset,
								x+Width-INCEND+Rect->BRXOffset,
								y+Rect->BRYOffset,Rect->ColourIndex);
				}
				break;

			case FR_BOTTOM:
				if(Opaque==FALSE) 
				{
					if(Masked == FALSE) 
					{
						Width &= 0xfffc;	// Software transboxfill needs to be a multiple of 4 pixels.
						Masked = TRUE;
					}
					if (pie_GetRenderEngine() == ENGINE_GLIDE)
					{
						iV_UniTransBoxFill( x+Rect->TLXOffset,
										y+Height-INCEND+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset,
										(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE, FILLTRANS);
					} 
					else
					{
						iV_TransBoxFill( x+Rect->TLXOffset,
										y+Height-INCEND+Rect->TLYOffset,
										x+Width-INCEND+Rect->BRXOffset,
										y+Height-INCEND+Rect->BRYOffset);
					}

				} else {
					iV_BoxFill( x+Rect->TLXOffset,
						   		y+Height-INCEND+Rect->TLYOffset,
						   		x+Width-INCEND+Rect->BRXOffset,
						   		y+Height-INCEND+Rect->BRYOffset,Rect->ColourIndex);
				}
				break;
		}
	}


	DrawBegin();

	if(Frame->TopLeft >= 0) {
		WTopLeft = (SWORD)iV_GetImageWidth(IntImages,Frame->TopLeft);
		HTopLeft = (SWORD)iV_GetImageHeight(IntImages,Frame->TopLeft);
		iV_DrawTransImage(IntImages,Frame->TopLeft,x,y);
	}

	if(Frame->TopRight >= 0) {
		WTopRight = (SWORD)iV_GetImageWidth(IntImages,Frame->TopRight);
		HTopRight = (SWORD)iV_GetImageHeight(IntImages,Frame->TopRight);
		iV_DrawTransImage(IntImages,Frame->TopRight,x+Width-WTopRight, y);
	}
	
	if(Frame->BottomRight >= 0) {
		WBottomRight = (SWORD)iV_GetImageWidth(IntImages,Frame->BottomRight);
		HBottomRight = (SWORD)iV_GetImageHeight(IntImages,Frame->BottomRight);
		iV_DrawTransImage(IntImages,Frame->BottomRight,x+Width-WBottomRight,y+Height-HBottomRight);
	}

	if(Frame->BottomLeft >= 0) {
		WBottomLeft = (SWORD)iV_GetImageWidth(IntImages,Frame->BottomLeft);
		HBottomLeft = (SWORD)iV_GetImageHeight(IntImages,Frame->BottomLeft);
		iV_DrawTransImage(IntImages,Frame->BottomLeft,x,y+Height-HBottomLeft);
	}

	if(Frame->TopEdge >= 0) {
		if(Frame->TopType == FR_SOLID) {
			iV_DrawImageRect(IntImages,Frame->TopEdge,
								x+iV_GetImageWidth(IntImages,Frame->TopLeft),
								y,
								0,0,
								Width-WTopLeft-WTopRight,
								iV_GetImageHeight(IntImages,Frame->TopEdge));
		} else {
			iV_DrawTransImageRect(IntImages,Frame->TopEdge,
								x+iV_GetImageWidth(IntImages,Frame->TopLeft),
								y,
								0,0,
								Width-WTopLeft-WTopRight,
								iV_GetImageHeight(IntImages,Frame->TopEdge));
		}
	}

	if(Frame->BottomEdge >= 0) {
		if(Frame->BottomType == FR_SOLID) {
			iV_DrawImageRect(IntImages,Frame->BottomEdge,
								x+WBottomLeft,
								y+Height-iV_GetImageHeight(IntImages,Frame->BottomEdge),
								0,0,
								Width-WBottomLeft-WBottomRight,
								iV_GetImageHeight(IntImages,Frame->BottomEdge));
		} else {
			iV_DrawTransImageRect(IntImages,Frame->BottomEdge,
								x+WBottomLeft,
								y+Height-iV_GetImageHeight(IntImages,Frame->BottomEdge),
								0,0,
								Width-WBottomLeft-WBottomRight,
								iV_GetImageHeight(IntImages,Frame->BottomEdge));
		}
	}

	if(Frame->LeftEdge >= 0) {
		if(Frame->LeftType == FR_SOLID) {
			iV_DrawImageRect(IntImages,Frame->LeftEdge,
								x,
								y+HTopLeft,
								0,0,
								iV_GetImageWidth(IntImages,Frame->LeftEdge),
								Height-HTopLeft-HBottomLeft);
		} else {
			iV_DrawTransImageRect(IntImages,Frame->LeftEdge,
								x,
								y+HTopLeft,
								0,0,
								iV_GetImageWidth(IntImages,Frame->LeftEdge),
								Height-HTopLeft-HBottomLeft);
		}
	}

	if(Frame->RightEdge >= 0) {
		if(Frame->RightType == FR_SOLID) {
			iV_DrawImageRect(IntImages,Frame->RightEdge,
								x+Width-iV_GetImageWidth(IntImages,Frame->RightEdge),
								y+HTopRight,
								0,0,
								iV_GetImageWidth(IntImages,Frame->RightEdge),
								Height-HTopRight-HBottomRight);
		} else {
			iV_DrawTransImageRect(IntImages,Frame->RightEdge,
								x+Width-iV_GetImageWidth(IntImages,Frame->RightEdge),
								y+HTopRight,
								0,0,
								iV_GetImageWidth(IntImages,Frame->RightEdge),
								Height-HTopRight-HBottomRight);
		}
	}

	DrawEnd();
}
