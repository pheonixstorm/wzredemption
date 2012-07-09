/*
 * Tip.c
 *
 * The tool tip display system.
 *
 */

#include "Frame.h"
#include "FrameInt.h"
#include "Widget.h"
#include "WidgInt.h"
#include "Tip.h"
#include "Vid.h"


/* Time delay before showing the tool tip */
#define TIP_PAUSE	200

/* How long to display the tool tip */
#define TIP_TIME	4000

/* Size of border around tip text */
#define TIP_HGAP	6
#define TIP_VGAP	3


/* The tool tip state */
static enum _tip_state
{
	TIP_NONE,			// No tip, and no button hilited
	TIP_WAIT,			// A button is hilited, but not yet ready to show the tip
	TIP_ACTIVE,			// A tip is being displayed
} tipState;


static SDWORD		startTime;			// When the tip was created
static SDWORD		mx,my;				// Last mouse coords
static SDWORD		wx,wy,ww,wh;		// Position and size of button to place tip by
static SDWORD		tx,ty,tw,th;		// Position and size of the tip box
static SDWORD		fx,fy;				// Position of the text
static STRING		*pTip;				// Tip text
static UDWORD		*pColours;			// The colours for the tool tip
static WIDGET		*psWidget;			// The button the tip is for
//static PROP_FONT	*psFont;			// The font to display the tip with
static int FontID = 0;	// ID for the Ivis Font.
static int TipColour;

/* Initialise the tool tip module */
void tipInitialise(void)
{
	tipState = TIP_NONE;
}

// Set the global toop tip text colour.
void widgSetTipColour(W_SCREEN *psScreen, UBYTE red, UBYTE green, UBYTE blue)
{
#ifdef WIN32 
	TipColour = -1;					// use bitmap colourings.
#else
//	TipColour = screenGetCacheColour(red,green,blue);
	TipColour = (UBYTE)pal_GetNearestColour(red,green,blue);
#endif
}

/*
 * Setup a tool tip.
 * The tip module will then wait until the correct points to
 * display and then remove the tool tip.
 * i.e. The tip will not be displayed immediately.
 * Calling this while another tip is being displayed will restart
 * the tip system.
 * psSource is the widget that started the tip.
 * x,y,width,height - specify the position of the button to place the
 * tip by.
 */
//void tipStart(WIDGET *psSource, STRING *pNewTip, PROP_FONT *psNewFont,
void tipStart(WIDGET *psSource, STRING *pNewTip, int NewFontID,
					 UDWORD *pNewColours, SDWORD x, SDWORD y, UDWORD width, UDWORD height)
{
	ASSERT((PTRVALID(psSource, sizeof(WIDGET)),
		"tipStart: Invalid widget pointer"));
//	ASSERT((PTRVALID(pNewTip, WIDG_MAXSTR),
//		"tipStart: Invalid tip pointer"));
//	ASSERT((PTRVALID(psNewFont, sizeof(PROP_FONT)),
//		"tipStart: Invalid font pointer"));
	ASSERT((PTRVALID(pNewColours, sizeof(UDWORD) * WCOL_MAX),
		"tipStart: Invalid colours pointer"));

	tipState = TIP_WAIT;
	startTime = GetTickCount();
	mx = mouseX();
	my = mouseY();
	wx = x; wy = y;
	ww = width; wh = height;
	pTip = pNewTip;
	psWidget = psSource;
	FontID = NewFontID;
	pColours = pNewColours;
}


/* Stop a tool tip (e.g. if the hilite is lost on a button).
 * psSource should be the same as the widget that started the tip.
 */
void tipStop(WIDGET *psSource)
{
	ASSERT((PTRVALID(psSource, sizeof(WIDGET)),
		"tipStop: Invalid widget pointer"));

	if (tipState != TIP_NONE && psSource == psWidget)
	{
		tipState = TIP_NONE;
	}
}

#ifdef WIN32
#define RIGHTBORDER		(0)
#define BOTTOMBORDER	(0)
#else
#define RIGHTBORDER		(24)
#define BOTTOMBORDER	(16)
#endif

/* Update and possibly display the tip */
void tipDisplay(void)
{
	SDWORD		newMX,newMY;
	SDWORD		currTime;
	SDWORD		fw, topGap;
//	UDWORD		time;

	switch (tipState)
	{
	case TIP_WAIT:
		/* See if the tip has to be shown */
		newMX = mouseX();
		newMY = mouseY();
		currTime = GetTickCount();
		if (newMX == mx &&
			newMY == my &&
			(currTime - startTime > TIP_PAUSE))
		{
			/* Activate the tip */
			tipState = TIP_ACTIVE;

			/* Calculate the size of the tip box */
			topGap = TIP_VGAP;
			iV_SetFont(FontID);

			fw = iV_GetTextWidth(pTip);
			tw = fw + TIP_HGAP*2;
			th = topGap*2 + iV_GetTextLineSize()+iV_GetTextBelowBase();

			/* Position the tip box */
			tx = wx + (ww >> 1);
			ty = wy + wh + TIP_VGAP;

			/* Check the box is on screen */
			if (tx < 0)
			{
				tx = 0;
			}
			if (tx + tw >= (SDWORD)screenWidth-RIGHTBORDER)
			{
				tx = screenWidth-RIGHTBORDER - tw - 1;
			}
			if (ty < 0)
			{
				ty = 0;
			}
			if (ty + th >= (SDWORD)screenHeight-BOTTOMBORDER)
			{
				/* Position the tip above the button */
				ty = wy - th - TIP_VGAP;
			}


			/* Position the text */
			fx = tx + TIP_HGAP;
			fy = ty + (th - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();

			/* Note the time */
			startTime = GetTickCount();
		}
		else if (newMX != mx ||
				 newMY != my ||
				 mousePressed(MOUSE_LMB))
		{
			mx = newMX;
			my = newMY;
			startTime = currTime;
		}
		break;
	case TIP_ACTIVE:
		/* See if the tip still needs to be displayed */
//		time = GetTickCount();
//		if (mousePressed(MOUSE_LMB) ||
//			((time - startTime) > TIP_TIME))
//		{
//			tipState = TIP_NONE;
//			return;
//		}

		/* Draw the tool tip */
		pie_BoxFillIndex(tx,ty, tx+tw, ty+th,(UBYTE)*(pColours + WCOL_TIPBKGRND));
		iV_Box(tx,ty, tx+tw-1, ty+th-1,*(pColours + WCOL_LIGHT));
		iV_Line(tx+1, ty+th-2, tx+1,    ty+1,  *(pColours + WCOL_DARK));
		iV_Line(tx+2, ty+1,    tx+tw-2, ty+1,  *(pColours + WCOL_DARK));
		iV_Line(tx,	  ty+th,   tx+tw,   ty+th, *(pColours + WCOL_DARK));
		iV_Line(tx+tw,ty+th-1, tx+tw,   ty,    *(pColours + WCOL_DARK));


		iV_SetFont(FontID);
//		iV_SetTextColour((UWORD)*(pColours + WCOL_TEXT));
		iV_SetTextColour((UWORD)TipColour);
		iV_DrawText(pTip,fx,fy);
		break;
	}
}

