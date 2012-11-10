/*
 * Slider.c
 *
 * Slide bar widget definitions.
 */


#include "widget.h"
#include "widgint.h"
#include "slider.h"
#include "vid.h"

/* The widget heaps */
OBJ_HEAP	*psSldHeap;
BOOL DragEnabled = TRUE;

void sliderEnableDrag(BOOL Enable)
{
	DragEnabled = Enable;
}

/* Create a slider widget data structure */
BOOL sliderCreate(W_SLIDER **ppsWidget, W_SLDINIT *psInit)
{
	if (psInit->style & ~(WBAR_PLAIN | WIDG_HIDDEN))
	{
		ASSERT((FALSE, "sliderCreate: Unknown style"));
		return FALSE;
	}

	if (psInit->orientation < WSLD_LEFT || psInit->orientation > WSLD_BOTTOM)
	{
		ASSERT((FALSE, "sliderCreate: Unknown orientation"));
		return FALSE;
	}

	if (((psInit->orientation == WSLD_LEFT || psInit->orientation == WSLD_RIGHT) &&
				psInit->numStops > (psInit->width - psInit->barSize)) ||
		((psInit->orientation == WSLD_TOP || psInit->orientation == WSLD_BOTTOM) &&
				psInit->numStops > (psInit->height - psInit->barSize)))
	{
		ASSERT((FALSE, "sliderCreate: Too many stops for slider length"));
		return FALSE;
	}

	if (psInit->pos > psInit->numStops)
	{
		ASSERT((FALSE, "sliderCreate: slider position greater than stops"));
		return FALSE;
	}

	if (((psInit->orientation == WSLD_LEFT || psInit->orientation == WSLD_RIGHT) &&
				psInit->barSize > psInit->width) ||
		((psInit->orientation == WSLD_TOP || psInit->orientation == WSLD_BOTTOM) &&
				psInit->barSize > psInit->height))
	{
		ASSERT((FALSE, "sliderCreate: slider bar is larger than slider width"));
		return FALSE;
	}

	/* Allocate the required memory */
#if W_USE_MALLOC
	*ppsWidget = (W_SLIDER *)MALLOC(sizeof(W_SLIDER));
	if (*ppsWidget == NULL)
#else
	if (!HEAP_ALLOC(psSldHeap, ppsWidget))
#endif
	{
		ASSERT((FALSE, "sliderCreate: Out of memory"));
		return FALSE;
	}
	/* Allocate the memory for the tip and copy it if necessary */
	if (psInit->pTip)
	{
#if W_USE_STRHEAP
		if (!widgAllocCopyString(&(*ppsWidget)->pTip, psInit->pTip))
		{
			/* Out of memory - just carry on without the tip */
			ASSERT((FALSE, "sliderCreate: Out of memory"));
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
	(*ppsWidget)->type = WIDG_SLIDER;
	(*ppsWidget)->id = psInit->id;
	(*ppsWidget)->formID = psInit->formID;
	(*ppsWidget)->style = psInit->style;
	(*ppsWidget)->x = psInit->x;
	(*ppsWidget)->y = psInit->y;
	(*ppsWidget)->width = psInit->width;
	(*ppsWidget)->height = psInit->height;

	if (psInit->pDisplay)
	{
		(*ppsWidget)->display = psInit->pDisplay;
	}
	else
	{
		(*ppsWidget)->display = sliderDisplay;
	}
	(*ppsWidget)->callback = psInit->pCallback;
	(*ppsWidget)->pUserData = psInit->pUserData;
	(*ppsWidget)->UserData = psInit->UserData;
	(*ppsWidget)->orientation = psInit->orientation;
	(*ppsWidget)->numStops = psInit->numStops;
	(*ppsWidget)->barSize = psInit->barSize;

	sliderInitialise(*ppsWidget);

	(*ppsWidget)->pos = psInit->pos;

	return TRUE;
}


/* Free the memory used by a slider */
void sliderFree(W_SLIDER *psWidget)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_SLIDER)),
		"sliderFree: Invalid widget pointer"));

#if W_USE_STRHEAP
	if (psWidget->pTip)
	{
		widgFreeString(psWidget->pTip);
	}
#endif


#if W_USE_MALLOC
	FREE(psWidget);
#else
	HEAP_FREE(psSldHeap, psWidget);
#endif
}


/* Initialise a slider widget before running it */
void sliderInitialise(W_SLIDER *psWidget)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_SLIDER)),
		"sliderInitialise: Invalid slider pointer"));

	psWidget->state = 0;
	psWidget->pos = 0;
}


/* Get the current position of a slider bar */
UDWORD widgGetSliderPos(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	ASSERT((PTRVALID(psWidget, sizeof(W_SLIDER)),
		"widgGetSliderPos: couldn't find widget from id"));
	if (psWidget)
	{
		return ((W_SLIDER *)psWidget)->pos;
	}

	return 0;
}

/* Set the current position of a slider bar */
void widgSetSliderPos(W_SCREEN *psScreen, UDWORD id, UWORD pos)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	ASSERT((PTRVALID(psWidget, sizeof(W_SLIDER)),
		"widgGetSliderPos: couldn't find widget from id"));
	if (psWidget)
	{
		if (pos > ((W_SLIDER *)psWidget)->numStops)
		{
			((W_SLIDER *)psWidget)->pos = ((W_SLIDER *)psWidget)->numStops;
		}
		else
		{
			((W_SLIDER *)psWidget)->pos = pos;
		}
	}
}

/* Return the current position of the slider bar on the widget */
static void sliderGetBarBox(W_SLIDER *psSlider, SWORD *pX, SWORD *pY,
							UWORD *pWidth, UWORD *pHeight)
{
	switch (psSlider->orientation)
	{
	case WSLD_LEFT:
		*pX = (SWORD)((psSlider->width - psSlider->barSize)
					 * psSlider->pos / psSlider->numStops);
		*pY = 0;
		*pWidth = psSlider->barSize;
		*pHeight = psSlider->height;
		break;
	case WSLD_RIGHT:
		*pX = (SWORD)(psSlider->width - psSlider->barSize
				- (psSlider->width - psSlider->barSize)
				* psSlider->pos / psSlider->numStops);
		*pY = 0;
		*pWidth = psSlider->barSize;
		*pHeight = psSlider->height;
		break;
	case WSLD_TOP:
		*pX = 0;
		*pY = (SWORD)((psSlider->height - psSlider->barSize)
				* psSlider->pos / psSlider->numStops);
		*pWidth = psSlider->width;
		*pHeight = psSlider->barSize;
		break;
	case WSLD_BOTTOM:
		*pX = 0;
		*pY = (SWORD)(psSlider->height - psSlider->barSize
					- (psSlider->height - psSlider->barSize)
					* psSlider->pos / psSlider->numStops);
		*pWidth = psSlider->width;
		*pHeight = psSlider->barSize;
		break;
	}
}


/* Run a slider widget */
void sliderRun(W_SLIDER *psWidget, W_CONTEXT *psContext)
{
	SDWORD  mx,my;
	UDWORD	stopSize;

	if ((psWidget->state & SLD_DRAG) && !mouseDown(MOUSE_LMB))
	{
		psWidget->state &= ~SLD_DRAG;
		widgSetReturn((WIDGET *)psWidget);
	}
	else if (psWidget->state & SLD_DRAG)
	{
		/* Figure out where the drag box should be */
		mx = psContext->mx - psWidget->x;
		my = psContext->my - psWidget->y;
		switch (psWidget->orientation)
		{
		case WSLD_LEFT:
			if (mx <= psWidget->barSize/2)
			{
				psWidget->pos = 0;
			}
			else if (mx >= psWidget->width - psWidget->barSize/2)
			{
				psWidget->pos = psWidget->numStops;
			}
			else
			{
				/* Mouse is in the middle of the slider, calculate which stop */
				stopSize = (psWidget->width - psWidget->barSize) / psWidget->numStops;
				psWidget->pos = (UWORD)((mx + stopSize/2 - psWidget->barSize/2)
											* psWidget->numStops
											/ (psWidget->width - psWidget->barSize));
			}
			break;
		case WSLD_RIGHT:
			if (mx <= psWidget->barSize/2)
			{
				psWidget->pos = psWidget->numStops;
			}
			else if (mx >= psWidget->width - psWidget->barSize/2)
			{
				psWidget->pos = 0;
			}
			else
			{
				/* Mouse is in the middle of the slider, calculate which stop */
				stopSize = (psWidget->width - psWidget->barSize) / psWidget->numStops;
				psWidget->pos = (UWORD)(psWidget->numStops
											- (mx + stopSize/2 - psWidget->barSize/2)
												* psWidget->numStops
												/ (psWidget->width - psWidget->barSize));
			}
			break;
		case WSLD_TOP:
			if (my <= psWidget->barSize/2)
			{
				psWidget->pos = 0;
			}
			else if (my >= psWidget->height - psWidget->barSize/2)
			{
				psWidget->pos = psWidget->numStops;
			}
			else
			{
				/* Mouse is in the middle of the slider, calculate which stop */
				stopSize = (psWidget->height - psWidget->barSize) / psWidget->numStops;
				psWidget->pos = (UWORD)((my + stopSize/2 - psWidget->barSize/2)
											* psWidget->numStops
											/ (psWidget->height - psWidget->barSize));
			}
			break;
		case WSLD_BOTTOM:
			if (my <= psWidget->barSize/2)
			{
				psWidget->pos = psWidget->numStops;
			}
			else if (my >= psWidget->height - psWidget->barSize/2)
			{
				psWidget->pos = 0;
			}
			else
			{
				/* Mouse is in the middle of the slider, calculate which stop */
				stopSize = (psWidget->height - psWidget->barSize) / psWidget->numStops;
				psWidget->pos = (UWORD)(psWidget->numStops
											- (my + stopSize/2 - psWidget->barSize/2)
												* psWidget->numStops
												/ (psWidget->height - psWidget->barSize));
			}
			break;
		}
	}
}


/* Respond to a mouse click */
void sliderClicked(W_SLIDER *psWidget, W_CONTEXT *psContext)
{
#if 0
	SWORD	x,y;
	UWORD	width,height;
	SDWORD	mx,my;

	/* Get the slider position */
	sliderGetBarBox(psWidget, &x,&y, &width,&height);

	/* Did the mouse click on the slider ? */
	mx = psContext->mx - psWidget->x;
	my = psContext->my - psWidget->y;
#endif
	if(DragEnabled) {
		if (psContext->mx >= psWidget->x &&
			psContext->mx <= psWidget->x + psWidget->width &&
			psContext->my >= psWidget->y &&
			psContext->my <= psWidget->y + psWidget->height)
		{
			psWidget->state |= SLD_DRAG;
		}
	}
}


/* Respond to a mouse up */
void sliderReleased(W_SLIDER *psWidget)
{
	(void)psWidget;
}


/* Respond to a mouse moving over a slider */
void sliderHiLite(W_SLIDER *psWidget)
{
	psWidget->state |= SLD_HILITE;
}


/* Respond to the mouse moving off a slider */
void sliderHiLiteLost(W_SLIDER *psWidget)
{
	psWidget->state &= ~SLD_HILITE;
}

/* The slider display function */
void sliderDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
						  UDWORD *pColours)
{
	W_SLIDER	*psSlider;
	SWORD		x0,y0, x1,y1;
	UWORD		width,height;

	psSlider = (W_SLIDER *)psWidget;

	switch (psSlider->orientation)
	{
	case WSLD_LEFT:
	case WSLD_RIGHT:
		/* Draw the line */
		x0 = (SWORD)(psSlider->x + xOffset + psSlider->barSize/(SWORD)2);
		y0 = (SWORD)(psSlider->y + yOffset + psSlider->height/(SWORD)2);
		x1 = (SWORD)(x0 + psSlider->width - psSlider->barSize);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_DARK));
		iV_Line(x0,y0+1, x1,y0+1,*(pColours + WCOL_LIGHT));

		/* Now Draw the bar */
		sliderGetBarBox(psSlider, &x0,&y0, &width,&height);
		x0 = (SWORD)(x0 + psSlider->x + xOffset);
		y0 = (SWORD)(y0 + psSlider->y + yOffset);
		x1 = (SWORD)(x0 + width);
		y1 = (SWORD)(y0 + height);
		pie_BoxFillIndex(x0,y0, x1,y1,WCOL_BKGRND);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_LIGHT));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_LIGHT));
		iV_Line(x1,y0, x1,y1,*(pColours + WCOL_DARK));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_DARK));
		break;
	case WSLD_TOP:
	case WSLD_BOTTOM:
		/* Draw the line */
		x0 = (SWORD)(psSlider->x + xOffset + psSlider->width/(SWORD)2);
		y0 = (SWORD)(psSlider->y + yOffset + psSlider->barSize/(SWORD)2);
		y1 = (SWORD)(y0 + psSlider->height - psSlider->barSize);
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_DARK));
		iV_Line(x0+1,y0, x0+1,y1,*(pColours + WCOL_LIGHT));

		/* Now Draw the bar */
		sliderGetBarBox(psSlider, &x0,&y0, &width,&height);
		x0 = (SWORD)(x0 + psSlider->x + xOffset);
		y0 = (SWORD)(y0 + psSlider->y + yOffset);
		x1 = (SWORD)(x0 + width);
		y1 = (SWORD)(y0 + height);
		pie_BoxFillIndex(x0,y0, x1,y1,WCOL_BKGRND);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_LIGHT));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_LIGHT));
		iV_Line(x1,y0, x1,y1,*(pColours + WCOL_DARK));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_DARK));
		break;
	}

	if (psSlider->state & SLD_HILITE)
	{
		x0 = (SWORD)(psWidget->x + xOffset - 2);
		y0 = (SWORD)(psWidget->y + yOffset - 2);
		x1 = (SWORD)(x0 + psWidget->width + 4);
		y1 = (SWORD)(y0 + psWidget->height + 4);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_HILITE));
		iV_Line(x1,y0, x1,y1,*(pColours + WCOL_HILITE));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_HILITE));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_HILITE));
	}
}

