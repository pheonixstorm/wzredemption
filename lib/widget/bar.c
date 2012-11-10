/*
 * bar.c
 *
 * Functions for the bar graph widget
 */


#include "widget.h"
#include "widgint.h"
#include "tip.h"
#include "form.h"
#include "bar.h"
#include "vid.h"
#include "piepalette.h"

/* The widget heap */
OBJ_HEAP	*psBarHeap;

/* Create a barGraph widget data structure */
BOOL barGraphCreate(W_BARGRAPH **ppsWidget, W_BARINIT *psInit)
{
	if (psInit->style & ~(WBAR_PLAIN | WBAR_TROUGH | WBAR_DOUBLE | WIDG_HIDDEN))
	{
		ASSERT((FALSE, "Unknown bar graph style"));
		return FALSE;
	}

	if (psInit->orientation < WBAR_LEFT || psInit->orientation > WBAR_BOTTOM)
	{
		ASSERT((FALSE, "barGraphCreate: Unknown orientation"));
		return FALSE;
	}

	if (psInit->size > WBAR_SCALE)
	{
		ASSERT((FALSE, "barGraphCreate: Bar size out of range"));
		return FALSE;
	}
	if ((psInit->style & WBAR_DOUBLE) && (psInit->minorSize > WBAR_SCALE))
	{
		ASSERT((FALSE, "barGraphCreate: Minor bar size out of range"));
		return FALSE;
	}

	/* Allocate the required memory */
#if W_USE_MALLOC
	*ppsWidget = (W_BARGRAPH *)MALLOC(sizeof(W_BARGRAPH));
	if (*ppsWidget == NULL)
#else
	if (!HEAP_ALLOC(psBarHeap, ppsWidget))
#endif
	{
		ASSERT((FALSE, "barGraphCreate: Out of memory"));
		return FALSE;
	}
	/* Allocate the memory for the tip and copy it if necessary */
	if (psInit->pTip)
	{
#if W_USE_STRHEAP
		if (!widgAllocCopyString(&(*ppsWidget)->pTip, psInit->pTip))
		{
			/* Out of memory - just carry on without the tip */
			ASSERT((FALSE, "barGraphCreate: Out of memory"));
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
	(*ppsWidget)->type = WIDG_BARGRAPH;
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
	(*ppsWidget)->barPos = psInit->orientation;
	(*ppsWidget)->majorSize = psInit->size;
	(*ppsWidget)->minorSize = psInit->minorSize;
	(*ppsWidget)->iRange = psInit->iRange;

	/* Set the display function */
	if (psInit->pDisplay)
	{
		(*ppsWidget)->display = psInit->pDisplay;
	}
	else if (psInit->style & WBAR_TROUGH)
	{
		(*ppsWidget)->display = barGraphDisplayTrough;
	}
	else if (psInit->style & WBAR_DOUBLE)
	{
		(*ppsWidget)->display = barGraphDisplayDouble;
	}
	else
	{
		(*ppsWidget)->display = barGraphDisplay;
	}
	/* Set the major colour */
//	(*ppsWidget)->majorCol = screenGetCacheColour(psInit->sCol.red,
//											psInit->sCol.green, psInit->sCol.blue);
	(*ppsWidget)->majorCol = (UBYTE)pal_GetNearestColour(psInit->sCol.red,
															psInit->sCol.green, psInit->sCol.blue);

	/* Set the minor colour if necessary */
	if (psInit->style & WBAR_DOUBLE)
	{
//		(*ppsWidget)->minorCol = screenGetCacheColour(psInit->sMinorCol.red,
//												psInit->sMinorCol.green, psInit->sMinorCol.blue);
		(*ppsWidget)->majorCol = (UBYTE)pal_GetNearestColour(psInit->sMinorCol.red,
												psInit->sMinorCol.green, psInit->sMinorCol.blue);
	}

	barGraphInitialise(*ppsWidget);

	return TRUE;
}


/* Free the memory used by a barGraph */
void barGraphFree(W_BARGRAPH *psWidget)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_BARGRAPH)),
		"barGraphFree: Invalid widget pointer"));

#if W_USE_STRHEAP
	if (psWidget->pTip)
	{
		widgFreeString(psWidget->pTip);
	}
#endif

#if W_USE_MALLOC
	FREE(psWidget);
#else
	HEAP_FREE(psBarHeap, psWidget);
#endif
}

/* Initialise a barGraph widget before running it */
void barGraphInitialise(W_BARGRAPH *psWidget)
{
	(void)psWidget;
}


/* Set the current size of a bar graph */
void widgSetBarSize(W_SCREEN *psScreen, UDWORD id, UDWORD iValue)
{
	W_BARGRAPH		*psBGraph;
	UDWORD			size;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgSetBarSize: Invalid screen pointer"));

	psBGraph = (W_BARGRAPH *)widgGetFromID(psScreen, id);
	if (psBGraph == NULL || psBGraph->type != WIDG_BARGRAPH)
	{
		ASSERT((FALSE, "widgSetBarSize: Couldn't find widget from id"));
		return;
	}

	if ( iValue < psBGraph->iRange )
	{
		psBGraph->iValue = (UWORD) iValue;
	}
	else
	{
		psBGraph->iValue = psBGraph->iRange;
	}

	size = WBAR_SCALE * psBGraph->iValue / psBGraph->iRange;

	psBGraph->majorSize = (UWORD)size;
}


/* Set the current size of a minor bar on a double graph */
void widgSetMinorBarSize(W_SCREEN *psScreen, UDWORD id, UDWORD iValue )
{
	W_BARGRAPH		*psBGraph;
	UDWORD			size;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgSetBarSize: Invalid screen pointer"));

	psBGraph = (W_BARGRAPH *)widgGetFromID(psScreen, id);
	if (psBGraph == NULL || psBGraph->type != WIDG_BARGRAPH)
	{
		ASSERT((FALSE, "widgSetBarSize: Couldn't find widget from id"));
		return;
	}

	size = WBAR_SCALE * iValue / psBGraph->iRange;
	if (size > WBAR_SCALE)
	{
		size = WBAR_SCALE;
	}

	psBGraph->minorSize = (UWORD)size;
}


#if 0
/* Run a barGraph widget */
void barGraphRun(W_BARGRAPH *psWidget)
{
}


/* Respond to a mouse click */
void barGraphClicked(W_BARGRAPH *psWidget)
{
}


/* Respond to a mouse up */
void barGraphReleased(W_BARGRAPH *psWidget)
{
}
#endif


/* Respond to a mouse moving over a barGraph */
void barGraphHiLite(W_BARGRAPH *psWidget, W_CONTEXT *psContext)
{
	if (psWidget->pTip)
	{
		tipStart((WIDGET *)psWidget, psWidget->pTip, psContext->psScreen->TipFontID,
				 psContext->psForm->aColours,
				 psWidget->x + psContext->xOffset, psWidget->y + psContext->yOffset,
				 psWidget->width, psWidget->height);
	}
}


/* Respond to the mouse moving off a barGraph */
void barGraphHiLiteLost(W_BARGRAPH *psWidget)
{
	tipStop((WIDGET *)psWidget);
}


/* The simple bar graph display function */
void barGraphDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
							UDWORD *pColours)
{
	SDWORD		x0,y0, x1,y1;
	W_BARGRAPH	*psBGraph;

	psBGraph = (W_BARGRAPH *)psWidget;

	/* figure out which way the bar graph fills */
	switch (psBGraph->barPos)
	{
	case WBAR_LEFT:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		break;
	case WBAR_RIGHT:
		y0 = yOffset + psWidget->y;
		x1 = xOffset + psWidget->x + psWidget->width;
		x0 = x1 - psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		break;
	case WBAR_TOP:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width;
		y1 = y0 + psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		break;
	case WBAR_BOTTOM:
		x0 = xOffset + psWidget->x;
		x1 = x0 + psWidget->width;
		y1 = yOffset + psWidget->y + psWidget->height;
		y0 = y1 - psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		break;
	}

	/* Now draw the graph */
	pie_BoxFillIndex(x0,y0, x1,y1,psBGraph->majorCol);
	iV_Line(x0,y1, x0,y0,*(pColours + WCOL_LIGHT));
	iV_Line(x0,y0, x1,y0,*(pColours + WCOL_LIGHT));
	iV_Line(x1,y0, x1,y1,*(pColours + WCOL_DARK));
	iV_Line(x0,y1, x1,y1,*(pColours + WCOL_DARK));
}


/* The double bar graph display function */
void barGraphDisplayDouble(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
								  UDWORD *pColours)
{
	SDWORD		x0,y0, x1,y1, x2,y2, x3,y3;
	W_BARGRAPH	*psBGraph;

	psBGraph = (W_BARGRAPH *)psWidget;

	/* figure out which way the bar graph fills */
	switch (psBGraph->barPos)
	{
	case WBAR_LEFT:
		/* Calculate the major bar */
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + 2*psWidget->height/3;

		/* Calculate the minor bar */
		x2 = x0;
		y2 = y0 + psWidget->height/3;
		x3 = x2 + psWidget->width * psBGraph->minorSize / WBAR_SCALE;
		y3 = y0 + psWidget->height;
		break;
	case WBAR_RIGHT:
		/* Calculate the major bar */
		y0 = yOffset + psWidget->y;
		x1 = xOffset + psWidget->x + psWidget->width;
		x0 = x1 - psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + 2*psWidget->height/3;

		/* Calculate the minor bar */
		x3 = x1;
		y2 = y0 + psWidget->height/3;
		x2 = x3 - psWidget->width * psBGraph->minorSize / WBAR_SCALE;
		y3 = y0 + psWidget->height;
		break;
	case WBAR_TOP:
		/* Calculate the major bar */
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + 2*psWidget->width/3;
		y1 = y0 + psWidget->height * psBGraph->majorSize / WBAR_SCALE;

		/* Calculate the minor bar */
		x2 = x0 + psWidget->width/3;
		y2 = y0;
		x3 = x0 + psWidget->width;
		y3 = y2 + psWidget->height * psBGraph->minorSize / WBAR_SCALE;
		break;
	case WBAR_BOTTOM:
		/* Calculate the major bar */
		x0 = xOffset + psWidget->x;
		x1 = x0 + 2*psWidget->width/3;
		y1 = yOffset + psWidget->y + psWidget->height;
		y0 = y1 - psWidget->height * psBGraph->majorSize / WBAR_SCALE;

		/* Calculate the minor bar */
		x2 = x0 + psWidget->width/3;
		x3 = x0 + psWidget->width;
		y3 = y1;
		y2 = y3 - psWidget->height * psBGraph->minorSize / WBAR_SCALE;
		break;
	}

	/* Draw the minor bar graph */
	if (psBGraph->minorSize > 0)
	{
		pie_BoxFillIndex(x2,y2, x3,y3,psBGraph->minorCol);
		iV_Line(x2,y3, x2,y2,*(pColours + WCOL_LIGHT));
		iV_Line(x2,y2, x3,y2,*(pColours + WCOL_LIGHT));
		iV_Line(x3,y2, x3,y3,*(pColours + WCOL_DARK));
		iV_Line(x2,y3, x3,y3,*(pColours + WCOL_DARK));
	}

	/* Draw the major bar graph */
	pie_BoxFillIndex(x0,y0, x1,y1,psBGraph->majorCol);
	iV_Line(x0,y1, x0,y0,*(pColours + WCOL_LIGHT));
	iV_Line(x0,y0, x1,y0,*(pColours + WCOL_LIGHT));
	iV_Line(x1,y0, x1,y1,*(pColours + WCOL_DARK));
	iV_Line(x0,y1, x1,y1,*(pColours + WCOL_DARK));
}


/* The trough bar graph display function */
void barGraphDisplayTrough(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
							UDWORD *pColours)
{
	SDWORD		x0,y0, x1,y1;		// Position of the bar
	SDWORD		tx0,ty0, tx1,ty1;	// Position of the trough
	W_BARGRAPH	*psBGraph;
	BOOL		showBar=TRUE, showTrough=TRUE;

	psBGraph = (W_BARGRAPH *)psWidget;

	/* figure out which way the bar graph fills */
	switch (psBGraph->barPos)
	{
	case WBAR_LEFT:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		if (x0 == x1)
		{
			showBar = FALSE;
		}
		tx0 = x1+1;
		ty0 = y0;
		tx1 = x0 + psWidget->width;
		ty1 = y1;
		if (tx0 >= tx1)
		{
			showTrough = FALSE;
		}
		break;
	case WBAR_RIGHT:
		y0 = yOffset + psWidget->y;
		x1 = xOffset + psWidget->x + psWidget->width;
		x0 = x1 - psWidget->width * psBGraph->majorSize / WBAR_SCALE;
		y1 = y0 + psWidget->height;
		if (x0 == x1)
		{
			showBar = FALSE;
		}
		tx0 = xOffset + psWidget->x;
		ty0 = y0;
		tx1 = x0-1;
		ty1 = y1;
		if (tx0 >= tx1)
		{
			showTrough = FALSE;
		}
		break;
	case WBAR_TOP:
		x0 = xOffset + psWidget->x;
		y0 = yOffset + psWidget->y;
		x1 = x0 + psWidget->width;
		y1 = y0 + psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		if (y0 == y1)
		{
			showBar = FALSE;
		}
		tx0 = x0;
		ty0 = y1+1;
		tx1 = x1;
		ty1 = y0 + psWidget->height;
		if (ty0 >= ty1)
		{
			showTrough = FALSE;
		}
		break;
	case WBAR_BOTTOM:
		x0 = xOffset + psWidget->x;
		x1 = x0 + psWidget->width;
		y1 = yOffset + psWidget->y + psWidget->height;
		y0 = y1 - psWidget->height * psBGraph->majorSize / WBAR_SCALE;
		if (y0 == y1)
		{
			showBar = FALSE;
		}
		tx0 = x0;
		ty0 = yOffset + psWidget->y;
		tx1 = x1;
		ty1 = y0-1;
		if (ty0 >= ty1)
		{
			showTrough = FALSE;
		}
		break;
	}

	/* Now draw the graph */
	if (showBar)
	{
		pie_BoxFillIndex(x0,y0, x1,y1,psBGraph->majorCol);
	}
	if (showTrough)
	{
		pie_BoxFillIndex(tx0,ty0, tx1,ty1,WCOL_BKGRND);
		iV_Line(tx0,ty1, tx0,ty0,*(pColours + WCOL_DARK));
		iV_Line(tx0,ty0, tx1,ty0,*(pColours + WCOL_DARK));
		iV_Line(tx1,ty0, tx1,ty1,*(pColours + WCOL_LIGHT));
		iV_Line(tx0,ty1, tx1,ty1,*(pColours + WCOL_LIGHT));
	}
}

