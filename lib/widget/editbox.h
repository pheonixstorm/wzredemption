/*
 * EditBox.h
 *
 * Definitions for the edit box functions.
 */
#ifndef _editbox_h
#define _editbox_h

/* The widget heap */
extern OBJ_HEAP	*psEdbHeap;

/* Edit Box states */
#define WEDBS_FIXED		0x0001		// No editing is going on
#define WEDBS_INSERT	0x0002		// Insertion editing
#define WEDBS_OVER		0x0003		// Overwrite editing
#define WEDBS_MASK		0x000f		// 
#define WEDBS_HILITE	0x0010		//
#define WEDBS_DISABLE   0x0020		// disable button from selection

typedef struct _w_editbox
{
	/* The common widget data */
	WIDGET_BASE;

	UDWORD		state;						// The current edit box state
	STRING		aText[WIDG_MAXSTR];			// The text in the edit box
//	PROP_FONT	*psFont;					// The font for the edit box
	int FontID;
	UWORD		insPos;						// The insertion point in the buffer
	UWORD		printStart;					// Where in the string appears at the far left of the box
	UWORD		printChars;					// The number of characters appearing in the box
	UWORD		printWidth;					// The pixel width of the characters in the box
	WIDGET_DISPLAY	pBoxDisplay;			// Optional callback to display the edit box background.
	FONT_DISPLAY pFontDisplay;				// Optional callback to display a string.
	SWORD HilightAudioID;					// Audio ID for form clicked sound
	SWORD ClickedAudioID;					// Audio ID for form hilighted sound
	WIDGET_AUDIOCALLBACK AudioCallback;		// Pointer to audio callback function
} W_EDITBOX;

/* Create an edit box widget data structure */
extern BOOL editBoxCreate(W_EDITBOX **ppsWidget, W_EDBINIT *psInit);

/* Free the memory used by an edit box */
extern void editBoxFree(W_EDITBOX *psWidget);

/* Initialise an edit box widget */
extern void editBoxInitialise(W_EDITBOX *psWidget);

/* Set the current string for the edit box */
extern void editBoxSetString(W_EDITBOX *psWidget, STRING *pText);

/* Respond to loss of focus */
extern void editBoxFocusLost(W_EDITBOX *psWidget);

/* Run an edit box widget */
extern void editBoxRun(W_EDITBOX *psWidget, W_CONTEXT *psContext);

/* Respond to a mouse click */
extern void editBoxClicked(W_EDITBOX *psWidget, W_CONTEXT *psContext);

/* Respond to a mouse button up */
extern void editBoxReleased(W_EDITBOX *psWidget);

/* Respond to a mouse moving over an edit box */
extern void editBoxHiLite(W_EDITBOX *psWidget);

/* Respond to the mouse moving off an edit box */
extern void editBoxHiLiteLost(W_EDITBOX *psWidget);

/* The edit box display function */
extern void editBoxDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);

/* set state of edit box */
extern void editBoxSetState(W_EDITBOX *psEditBox, UDWORD state);

#endif
