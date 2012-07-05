/*
 * Bar.h
 *
 * Definitions for Bar Graph functions.
 */
#ifndef _bar_h
#define _bar_h

/* The widget heap */
extern OBJ_HEAP	*psBarHeap;

typedef struct _w_bargraph
{
	/* The common widget data */
	WIDGET_BASE;

	UWORD		barPos;				// Orientation of the bar on the widget
	UWORD		majorSize;			// Percentage of the main bar that is filled
	UWORD		minorSize;			// Percentage of the minor bar if there is one
	UWORD		iRange;				// Maximum range
	UWORD		iValue;				// Current value
	UBYTE		majorCol;			// Colour for the major bar
	UBYTE		minorCol;			// Colour for the minor bar
	STRING		*pTip;				// The tool tip for the graph
} W_BARGRAPH;

/* Create a barGraph widget data structure */
extern BOOL barGraphCreate(W_BARGRAPH **ppsWidget, W_BARINIT *psInit);

/* Free the memory used by a barGraph */
extern void barGraphFree(W_BARGRAPH *psWidget);

/* Initialise a barGraph widget before running it */
extern void barGraphInitialise(W_BARGRAPH *psWidget);

#if 0
/* Run a barGraph widget */
extern void barGraphRun(W_BARGRAPH *psWidget);

/* Respond to a mouse click */
extern void barGraphClicked(W_BARGRAPH *psWidget);

/* Respond to a mouse up */
extern void barGraphReleased(W_BARGRAPH *psWidget);
#endif

/* Respond to a mouse moving over a barGraph */
extern void barGraphHiLite(W_BARGRAPH *psWidget, W_CONTEXT *psContext);

/* Respond to the mouse moving off a barGraph */
extern void barGraphHiLiteLost(W_BARGRAPH *psWidget);

/* The bar graph display function */
extern void barGraphDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
							UDWORD *pColours);

/* The double bar graph display function */
extern void barGraphDisplayDouble(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
								  UDWORD *pColours);

/* The trough bar graph display function */
extern void barGraphDisplayTrough(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset,
							UDWORD *pColours);

#endif

