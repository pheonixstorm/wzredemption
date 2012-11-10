/*
 * Button.c
 *
 * Functions for the button widget
 */

#include "frame.h"
#include "frameint.h"
#include "widget.h"
#include "widgint.h"
#include "button.h"
#include "form.h"
#include "tip.h"
#include "vid.h"

/* The widget heap */
OBJ_HEAP	*psButHeap;

/* Initialise the button module */
BOOL buttonStartUp(void)
{
	return TRUE;
}


/* Create a button widget data structure */
BOOL buttonCreate(W_BUTTON **ppsWidget, W_BUTINIT *psInit)
{
	if (psInit->style & ~(WBUT_PLAIN | WIDG_HIDDEN | WFORM_NOCLICKMOVE |
						  WBUT_NOPRIMARY | WBUT_SECONDARY | WBUT_TXTCENTRE ))
	{
		ASSERT((FALSE, "Unknown button style"));
		return FALSE;
	}

//#ifdef DEBUG
//	if (psInit->pText)
//	{
//		ASSERT((PTRVALID(psInit->psFont, sizeof(PROP_FONT)),
//			"buttonCreate: Invalid font pointer"));
//	}
//#endif

	/* Allocate the required memory */
#if W_USE_MALLOC
	*ppsWidget = (W_BUTTON *)MALLOC(sizeof(W_BUTTON));
	if (*ppsWidget == NULL)
#else
	if (!HEAP_ALLOC(psButHeap, ppsWidget))
#endif
	{
		ASSERT((FALSE, "buttonCreate: Out of memory"));
		return FALSE;
	}
	/* Allocate memory for the text and copy it if necessary */
	if (psInit->pText)
	{
#if W_USE_STRHEAP
		if (!widgAllocCopyString(&(*ppsWidget)->pText, psInit->pText))
		{
			ASSERT((FALSE, "buttonCreate: Out of memory"));
#if W_USE_MALLOC
			FREE(*ppsWidget);
#else
			HEAP_FREE(psButHeap, *ppsWidget);
#endif
			return FALSE;
		}
#else
		(*ppsWidget)->pText = psInit->pText;
#endif
	}
	else
	{
		(*ppsWidget)->pText = NULL;
	}
	/* Allocate the memory for the tip and copy it if necessary */
	if (psInit->pTip)
	{
#if W_USE_STRHEAP
		if (!widgAllocCopyString(&(*ppsWidget)->pTip, psInit->pTip))
		{
			/* Out of memory - just carry on without the tip */
			ASSERT((FALSE, "buttonCreate: Out of memory"));
			(*ppsWidget)->pTip = NULL;
		}
#else
		(*ppsWidget)->pTip = psInit->pTip;
#endif
	}
	else
	{
		(*ppsWidget)->pTip = NULL;
	}

	/* Initialise the structure */
	(*ppsWidget)->type = WIDG_BUTTON;
	(*ppsWidget)->id = psInit->id;
	(*ppsWidget)->formID = psInit->formID;
	(*ppsWidget)->style = psInit->style;
	(*ppsWidget)->x = psInit->x;
	(*ppsWidget)->y = psInit->y;
	(*ppsWidget)->width = psInit->width;
	(*ppsWidget)->height = psInit->height;
	(*ppsWidget)->callback = psInit->pCallback;
	(*ppsWidget)->pUserData = psInit->pUserData;
	(*ppsWidget)->UserData = psInit->UserData;
	(*ppsWidget)->AudioCallback = WidgGetAudioCallback();
	(*ppsWidget)->HilightAudioID = WidgGetHilightAudioID();
	(*ppsWidget)->ClickedAudioID = WidgGetClickedAudioID();


	if (psInit->pDisplay)
	{
		(*ppsWidget)->display = psInit->pDisplay;
	}
	else
	{
		(*ppsWidget)->display = buttonDisplay;
	}
//	(*ppsWidget)->psFont = psInit->psFont;
	(*ppsWidget)->FontID = psInit->FontID;

	buttonInitialise(*ppsWidget);

	return TRUE;
}


/* Free the memory used by a button */
void buttonFree(W_BUTTON *psWidget)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_BUTTON)),
		"buttonFree: invalid button pointer"));

#if W_USE_STRHEAP
	if (psWidget->pText)
	{
		widgFreeString(psWidget->pText);
	}
	if (psWidget->pTip)
	{
		widgFreeString(psWidget->pTip);
	}
#endif

#if W_USE_MALLOC
	FREE(psWidget);
#else
	HEAP_FREE(psButHeap, psWidget);
#endif
}


/* Initialise a button widget before it is run */
void buttonInitialise(W_BUTTON *psWidget)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_BUTTON)),
		"buttonDisplay: Invalid widget pointer"));

	psWidget->state = WBUTS_NORMAL;
}


/* Get a button's state */
UDWORD buttonGetState(W_BUTTON *psButton)
{
	UDWORD State = 0;

	if (psButton->state & WBUTS_GREY)
	{
		State |= WBUT_DISABLE;
	}

	if (psButton->state & WBUTS_LOCKED)
	{
		State |= WBUT_LOCK;
	}

	if (psButton->state & WBUTS_CLICKLOCK)
	{
		State |= WBUT_CLICKLOCK;
	}

	if (psButton->state & WBUTS_FLASH)
	{
		State |= WBUT_FLASH;
	}

	return State;
}


void buttonSetFlash(W_BUTTON *psButton)
{
	psButton->state |= WBUTS_FLASH;
}


void buttonClearFlash(W_BUTTON *psButton)
{
	psButton->state &= ~WBUTS_FLASH;
	psButton->state &= ~WBUTS_FLASHON;
}


/* Set a button's state */
void buttonSetState(W_BUTTON *psButton, UDWORD state)
{
	ASSERT((!((state & WBUT_LOCK) && (state & WBUT_CLICKLOCK)),
		"widgSetButtonState: Cannot have WBUT_LOCK and WBUT_CLICKLOCK"));

	if (state & WBUT_DISABLE)
	{
		psButton->state |= WBUTS_GREY;
	}
	else
	{
		psButton->state &= ~WBUTS_GREY;
	}
	if (state & WBUT_LOCK)
	{
		psButton->state |= WBUTS_LOCKED;
	}
	else
	{
		psButton->state &= ~WBUTS_LOCKED;
	}
	if (state & WBUT_CLICKLOCK)
	{
		psButton->state |= WBUTS_CLICKLOCK;
	}
	else
	{
		psButton->state &= ~WBUTS_CLICKLOCK;
	}
}


extern UDWORD gameTime2;

/* Run a button widget */
void buttonRun(W_BUTTON *psButton)
{
//	(void)psButton;
	if(psButton->state & WBUTS_FLASH) {
		if (((gameTime2/250) % 2) == 0) {
			psButton->state &= ~WBUTS_FLASHON;
		} else {
			psButton->state |= WBUTS_FLASHON;
		}
	}
}


/* Respond to a mouse click */
void buttonClicked(W_BUTTON *psWidget, UDWORD key)
{
	/* Can't click a button if it is disabled or locked down */
	if (!(psWidget->state & (WBUTS_GREY | WBUTS_LOCKED)))
	{
		// Check this is the correct key
		if ((!(psWidget->style & WBUT_NOPRIMARY) && key == WKEY_PRIMARY) ||
			((psWidget->style & WBUT_SECONDARY) && key == WKEY_SECONDARY))
		{
			if(psWidget->AudioCallback) {
				psWidget->AudioCallback(psWidget->ClickedAudioID);
			}
			psWidget->state &= ~WBUTS_FLASH;	// Stop it flashing
			psWidget->state &= ~WBUTS_FLASHON;
			psWidget->state |= WBUTS_DOWN;
		}
	}

	/* Kill the tip if there is one */
	if (psWidget->pTip)
	{
		tipStop((WIDGET *)psWidget);
	}
}

/* Respond to a mouse button up */
void buttonReleased(W_BUTTON *psWidget, UDWORD key)
{
	if (psWidget->state & WBUTS_DOWN)
	{
		// Check this is the correct key
		if ((!(psWidget->style & WBUT_NOPRIMARY) && key == WKEY_PRIMARY) ||
			((psWidget->style & WBUT_SECONDARY) && key == WKEY_SECONDARY))
		{
			widgSetReturn((WIDGET *)psWidget);
			psWidget->state &= ~WBUTS_DOWN;
		}
	}
}


/* Respond to a mouse moving over a button */
void buttonHiLite(W_BUTTON *psWidget, W_CONTEXT *psContext)
{
	psWidget->state |= WBUTS_HILITE;

	if(psWidget->AudioCallback) {
		psWidget->AudioCallback(psWidget->HilightAudioID);
	}

	/* If there is a tip string start the tool tip */
	if (psWidget->pTip)
	{
		tipStart((WIDGET *)psWidget, psWidget->pTip, psContext->psScreen->TipFontID,
				 psContext->psForm->aColours,
				 psWidget->x + psContext->xOffset, psWidget->y + psContext->yOffset,
				 psWidget->width,psWidget->height);
	}
}


/* Respond to the mouse moving off a button */
void buttonHiLiteLost(W_BUTTON *psWidget)
{
	psWidget->state &= ~(WBUTS_DOWN | WBUTS_HILITE);
	if (psWidget->pTip)
	{
		tipStop((WIDGET *)psWidget);
	}
}


/* Display a button */
void buttonDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_BUTTON	*psButton;
	SDWORD		x0,y0,x1,y1, fx,fy,fw;
//	PROP_FONT	*psCurrFont;
	int			CurrFontID;

	ASSERT((PTRVALID(psWidget, sizeof(W_BUTTON)),
		"buttonDisplay: Invalid widget pointer"));

	psButton = (W_BUTTON *)psWidget;
//	psCurrFont = psButton->psFont;
	CurrFontID = psButton->FontID;

	x0=psButton->x + xOffset;
	y0=psButton->y + yOffset;
	x1=x0 + psButton->width;
	y1=y0 + psButton->height;

	if (psButton->state & (WBUTS_DOWN | WBUTS_LOCKED | WBUTS_CLICKLOCK))
	{
		/* Display the button down */
		pie_BoxFillIndex(x0,y0,x1,y1,WCOL_BKGRND);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_DARK));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_DARK));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_LIGHT));
		iV_Line(x1,y1, x1,y0,*(pColours + WCOL_LIGHT));

		if (psButton->pText)
		{
			iV_SetFont(psButton->FontID);
			iV_SetTextColour((UWORD)*(pColours + WCOL_TEXT));
			fw = iV_GetTextWidth(psButton->pText);
			if(psButton->style & WBUT_NOCLICKMOVE) {
				fx = x0 + (psButton->width - fw) / 2 + 1;
				fy = y0 + 1 + (psButton->height - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();
			} else {
				fx = x0 + (psButton->width - fw) / 2;
				fy = y0 + (psButton->height - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();
			}
			iV_DrawText(psButton->pText,fx,fy);
		}

		if (psButton->state & WBUTS_HILITE)
		{
			/* Display the button hilite */
			iV_Line(x0+3,y0+3, x1-2,y0+3,*(pColours + WCOL_HILITE));
			iV_Line(x0+3,y0+3, x0+3,y1-2,*(pColours + WCOL_HILITE));
			iV_Line(x0+3,y1-2, x1-2,y1-2,*(pColours + WCOL_HILITE));
			iV_Line(x1-2,y1-2, x1-2,y0+3,*(pColours + WCOL_HILITE));
		}
	}
	else if (psButton->state & WBUTS_GREY)
	{
		/* Display the disabled button */
		pie_BoxFillIndex(x0,y0,x1,y1,WCOL_BKGRND);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_LIGHT));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_LIGHT));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_DARK));
		iV_Line(x1,y1, x1,y0,*(pColours + WCOL_DARK));

		if (psButton->pText)
		{
			iV_SetFont(psButton->FontID);
			fw = iV_GetTextWidth(psButton->pText);
			fx = x0 + (psButton->width - fw) / 2;
			fy = y0 + (psButton->height - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();
			iV_SetTextColour((UWORD)*(pColours + WCOL_LIGHT));
			iV_DrawText(psButton->pText,fx+1,fy+1);
			iV_SetTextColour((UWORD)*(pColours + WCOL_DISABLE));
			iV_DrawText(psButton->pText,fx,fy);
		}

		if (psButton->state & WBUTS_HILITE)
		{
			/* Display the button hilite */
			iV_Line(x0+2,y0+2, x1-3,y0+2,*(pColours + WCOL_HILITE));
			iV_Line(x0+2,y0+2, x0+2,y1-3,*(pColours + WCOL_HILITE));
			iV_Line(x0+2,y1-3, x1-3,y1-3,*(pColours + WCOL_HILITE));
			iV_Line(x1-3,y1-3, x1-3,y0+2,*(pColours + WCOL_HILITE));
		}
	}
	else 
	{
		/* Display the button up */
		pie_BoxFillIndex(x0,y0,x1,y1,WCOL_BKGRND);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_LIGHT));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_LIGHT));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_DARK));
		iV_Line(x1,y1, x1,y0,*(pColours + WCOL_DARK));

		//if (0)
		if (psButton->pText)
		{
			iV_SetFont(psButton->FontID);
			iV_SetTextColour((UWORD)*(pColours + WCOL_TEXT));
			fw = iV_GetTextWidth(psButton->pText);
			fx = x0 + (psButton->width - fw) / 2;
			fy = y0 + (psButton->height - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();
			iV_DrawText(psButton->pText,fx,fy);
		}

		if (psButton->state & WBUTS_HILITE)
		{
			/* Display the button hilite */
			iV_Line(x0+2,y0+2, x1-3,y0+2,*(pColours + WCOL_HILITE));
			iV_Line(x0+2,y0+2, x0+2,y1-3,*(pColours + WCOL_HILITE));
			iV_Line(x0+2,y1-3, x1-3,y1-3,*(pColours + WCOL_HILITE));
			iV_Line(x1-3,y1-3, x1-3,y0+2,*(pColours + WCOL_HILITE));
		}
	}
}


