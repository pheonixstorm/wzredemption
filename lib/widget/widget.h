/*
 * Widget.h
 *
 * Definitions for the Widget library
 */
#ifndef _widget_h
#define _widget_h

#include "Frame.h"
#include "WidgBase.h"

/***********************************************************************************
 *
 * Widget style definitions - these control how the basic widget appears on screen
 */

#define WIDG_HIDDEN			0x8000		// The widget is initially hidden

/************ Form styles ****************/

/* Plain form */
#define WFORM_PLAIN			0

/* Tabbed form */
#define WFORM_TABBED		1

/* Invisible (i.e. see through) form - 
 * can be used in conjunction with WFORM_PLAIN or WFORM_TABBED.
 */
#define WFORM_INVISIBLE		2

/* Clickable form - return form id when the form is clicked */
#define WFORM_CLICKABLE		4

/* Disable movement on a clickable form. */
#define	WFORM_NOCLICKMOVE	8

/* Control whether the primary or secondary buttons work on a clickable form */
#define WFORM_NOPRIMARY		0x10		// Primary works by default - this turns it off
#define WFORM_SECONDARY		0x20

/************ Label styles ***************/

#define WLAB_PLAIN			0			// Plain text only label
#define WLAB_ALIGNLEFT		1			// Align the text at the left of the box
#define WLAB_ALIGNCENTRE	2			// Center the text
#define WLAB_ALIGNRIGHT		4			// Align the text at the right of the box

/************ Button styles **************/

#define WBUT_PLAIN			0			// Plain button (text with a box around it)

/* Disable movement on a button */
#define	WBUT_NOCLICKMOVE	8


/* Control whether the primary or secondary buttons work on a button */
#define WBUT_NOPRIMARY		0x10		// Primary works by default - this turns it off
#define WBUT_SECONDARY		0x20

#define WBUT_TXTCENTRE		64			// text only buttons. centre the text?
/*********** Edit Box styles *************/

#define WEDB_PLAIN		0			// Plain edit box (text with a box around it)
#define WEDB_DISABLED   1			// Disabled. Displayed but never gets focus.

/*********** Bar Graph styles ************/

#define WBAR_PLAIN		0			// Plain bar graph
#define WBAR_TROUGH		1			// Bar graph with a trough showing empty percentage
#define WBAR_DOUBLE		2			// Double bar graph, one on top of other

/*********** Slider styles ***************/

#define WSLD_PLAIN		0			// Plain slider

/***********************************************************************************/

/* Generic widget colour */
typedef struct _w_colourdef
{
	UBYTE	red;
	UBYTE	green;
	UBYTE	blue;
	UBYTE	alpha;
} W_COLOURDEF;


/* Basic initialisation entries common to all widgets */
#define WINIT_BASE \
	UDWORD				formID;			/* ID number of form to put widget on */ \
										/* ID == 0 specifies the default form for the screen */ \
	UWORD				majorID,minorID;	/* Which major and minor tab to put the widget */ \
										/* on for a tabbed form */ \
	UDWORD				id;				/* Unique id number (chosen by user) */ \
	UDWORD				style;			/* widget style */ \
	SWORD				x,y;			/* screen location */ \
	UWORD				width,height;	/* widget size */\
	WIDGET_DISPLAY		pDisplay;		/* Optional display function */\
	WIDGET_CALLBACK		pCallback;		/* Optional callback function */\
	void				*pUserData;		/* Optional user data pointer */\
	UDWORD				UserData		/* User data (if any) */

/* The basic initialisation structure */
typedef struct _w_init
{
	WINIT_BASE;
} W_INIT;

/* Flags for controlling where the tabs appear on a form - 
 * used in the majorPos and minorPos entries of the W_FORMINIT struct
 */
#define	WFORM_TABNONE		0		// No tab
#define WFORM_TABTOP		1
#define WFORM_TABLEFT		2
#define WFORM_TABRIGHT		3
#define WFORM_TABBOTTOM		4

/* Upper limits for major and minor tabs on a tab form.
 * Not the best way to do it I know, but it keeps the memory
 * management MUCH simpler.
 */

#define WFORM_MAXMAJOR		9	   //15		// Maximum number of major tabs on a tab form
#define WFORM_MAXMINOR		5	   //15		// Maximum number of minor tabs off a major

#define TAB_MINOR 0	// Tab types passed into tab display callbacks.
#define TAB_MAJOR 1

typedef void (*TAB_DISPLAY)(struct _widget *psWidget,UDWORD TabType,UDWORD Position,UDWORD Number,BOOL Selected,BOOL Hilight,UDWORD x,UDWORD y,UDWORD Width,UDWORD Height);
typedef void (*FONT_DISPLAY)(UDWORD x,UDWORD y,STRING *String);

/* Form initialisation structure */
typedef struct _w_forminit
{
	/* The basic init entries */
	WINIT_BASE;

	/* Data for a tabbed form */
	BOOL			disableChildren; \
	UWORD			majorPos, minorPos;			// Position of the tabs on the form
	UWORD			majorSize, minorSize;		// Size of the tabs (in pixels)
	SWORD			majorOffset, minorOffset;	// Tab start offset.
	SWORD			tabVertOffset;					// Tab form overlap offset.
	SWORD			tabHorzOffset;					// Tab form overlap offset.
	UWORD			tabMajorThickness;				// The thickness of the tabs
	UWORD			tabMinorThickness;				// The thickness of the tabs
	UWORD			tabMajorGap;						// The space between tabs
	UWORD			tabMinorGap;						// The space between tabs
	UWORD			numMajor;					// Number of major tabs
	UWORD			aNumMinors[WFORM_MAXMAJOR];	// Number of minor tabs for each major
	STRING			*pTip;						// Tool tip for the form itself
	STRING			*apMajorTips[WFORM_MAXMAJOR];	// Tool tips for the major tabs
	STRING			*apMinorTips[WFORM_MAXMAJOR][WFORM_MAXMINOR];
													// Tool tips for the minor tabs
	TAB_DISPLAY		pTabDisplay;		// Optional callback for displaying a tab.
	WIDGET_DISPLAY	pFormDisplay;		// Optional callback to display the form.
} W_FORMINIT;

/* Label initialisation structure */
typedef struct _w_labinit
{
	/* The basic init entries */
	WINIT_BASE;

	STRING		*pText;			// label text
	STRING		*pTip;			// Tool tip for the label.
//	PROP_FONT	*psFont;		// label font
	int			FontID;			// ID of the IVIS font to use for this widget.
} W_LABINIT;

/* Button initialisation structure */
typedef struct _w_butinit
{
	/* The basic init entries */
	WINIT_BASE;

	STRING		*pText;			// button text
	STRING		*pTip;			// Tool tip text
//	PROP_FONT	*psFont;		// button font
	int			FontID;			// ID of the IVIS font to use for this widget.
} W_BUTINIT;

/* Edit box initialisation structure */
typedef struct _w_edbinit
{
	/* The basic init entries */
	WINIT_BASE;

	STRING		*pText;			// initial contents of the edit box
//	PROP_FONT	*psFont;		// edit box font
	int			FontID;			// ID of the IVIS font to use for this widget.
	WIDGET_DISPLAY	pBoxDisplay;		// Optional callback to display the form.
	FONT_DISPLAY pFontDisplay;	// Optional callback to display a string.
} W_EDBINIT;

/* Orientation flags for the bar graph */
#define WBAR_LEFT		0x0001		// Bar graph fills from left to right
#define WBAR_RIGHT		0x0002		// Bar graph fills from right to left
#define WBAR_TOP		0x0003		// Bar graph fills from top to bottom
#define WBAR_BOTTOM		0x0004		// Bar graph fills from bottom to top

/* Bar Graph initialisation structure */
typedef struct _w_barinit
{
	/* The basic init entries */
	WINIT_BASE;

	UWORD		orientation;	// Orientation of the bar on the widget
	UWORD		size;			// Initial percentage of the graph that is filled
	UWORD		minorSize;		// Percentage of second bar graph if there is one
	UWORD		iRange;			// Maximum range
	W_COLOURDEF	sCol;			// Bar colour
	W_COLOURDEF	sMinorCol;		// Minor bar colour
	STRING		*pTip;			// Tool tip text
} W_BARINIT;


/* Orientation of the slider */
#define WSLD_LEFT		0x0001		// Slider is horizontal and starts at left
#define WSLD_RIGHT		0x0002		// Slider is horizontal and starts at the right
#define WSLD_TOP		0x0003		// Slider is vertical and starts at the top
#define WSLD_BOTTOM		0x0004		// Slider is vertical and starts at the bottom

/* Slider initialisation structure */
typedef struct _w_sldinit
{
	/* The basic init entries */
	WINIT_BASE;
	
	UWORD		orientation;	// Orientation of the slider
	UWORD		numStops;		// Number of stops on the slider
	UWORD		barSize;		// Size of the bar
	UWORD		pos;			// Initial position of the slider bar
	STRING		*pTip;			// Tip string
} W_SLDINIT;

/***********************************************************************************/

/* The maximum lenth of strings for the widget system */
#define WIDG_MAXSTR		80

/* The maximum value for bar graph size */
#define WBAR_SCALE		100


/* Structure to specify the heap sizes for the widget library */
typedef struct _w_heapinit
{
	UDWORD		barInit, barExt;		// bar graph heap
	UDWORD		butInit, butExt;		// button heap
	UDWORD		edbInit, edbExt;		// edit box heap
	UDWORD		formInit, formExt;		// form heap
	UDWORD		cFormInit, cFormExt;	// clicable form heap
	UDWORD		tFormInit, tFormExt;	// tab form heap
	UDWORD		labInit, labExt;		// label heap
	UDWORD		sldInit, sldExt;		// slider heap
} W_HEAPINIT;

/* Initialise the widget module */
extern BOOL	widgInitialise(W_HEAPINIT *psInit);

/* Reset the widget module */
extern void widgReset(void);

/* Shut down the widget module */
extern void widgShutDown(void);

/* Create an empty widget screen */
extern BOOL widgCreateScreen(W_SCREEN **ppsScreen);

/* Release a screen and all its associated data */
extern void widgReleaseScreen(W_SCREEN *psScreen);

/* Set the tool tip font for a screen */
extern void widgSetTipFont(W_SCREEN *psScreen, int FontID);

/* Add a form to the widget screen */
extern BOOL widgAddForm(W_SCREEN *psScreen, W_FORMINIT *psInit);

/* Add a label to the widget screen */
extern BOOL widgAddLabel(W_SCREEN *psScreen, W_LABINIT *psInit);

/* Add a button to a form */
extern BOOL widgAddButton(W_SCREEN *psScreen, W_BUTINIT *psInit);

/* Add an edit box to a form */
extern BOOL widgAddEditBox(W_SCREEN *psScreen, W_EDBINIT *psInit);

/* Add a bar graph to a form */
extern BOOL widgAddBarGraph(W_SCREEN *psScreen, W_BARINIT *psInit);

/* Add a slider to a form */
extern BOOL widgAddSlider(W_SCREEN *psScreen, W_SLDINIT *psInit);

/* Delete a widget from the screen */
extern void widgDelete(W_SCREEN *psScreen, UDWORD id);

/* Hide a widget */
extern void widgHide(W_SCREEN *psScreen, UDWORD id);

/* Reveal a widget */
extern void widgReveal(W_SCREEN *psScreen, UDWORD id);

/* Return a pointer to a buffer containing the current string of a widget if any.
 * This will always return a valid string pointer.
 * NOTE: The string must be copied out of the buffer
 */
extern STRING *widgGetString(W_SCREEN *psScreen, UDWORD id);

/* Set the text in a widget */
extern void widgSetString(W_SCREEN *psScreen, UDWORD id, STRING *pText);

/* Set the current tabs for a tab form */
extern void widgSetTabs(W_SCREEN *psScreen, UDWORD id, UWORD major, UWORD minor);

/* Get the current tabs for a tab form */
extern void widgGetTabs(W_SCREEN *psScreen, UDWORD id, UWORD *pMajor, UWORD *pMinor);

/* Get the current position of a widget */
extern void widgGetPos(W_SCREEN *psScreen, UDWORD id, SWORD *pX, SWORD *pY);

/* Get the current position of a slider bar */
extern UDWORD widgGetSliderPos(W_SCREEN *psScreen, UDWORD id);

/* Set the current position of a slider bar */
extern void widgSetSliderPos(W_SCREEN *psScreen, UDWORD id, UWORD pos);

/* Set the current size of a bar graph */
extern void widgSetBarSize(W_SCREEN *psScreen, UDWORD id, UDWORD size);

/* Set the current size of a minor bar on a double graph */
extern void widgSetMinorBarSize(W_SCREEN *psScreen, UDWORD id, UDWORD size);

/* Return the ID of the widget the mouse was over this frame */
extern UDWORD widgGetMouseOver(W_SCREEN *psScreen);

/* Return the user data for a widget */
extern void *widgGetUserData(W_SCREEN *psScreen, UDWORD id);

/* Set the user data for a widget */
extern void widgSetUserData(W_SCREEN *psScreen, UDWORD id,void *UserData);

/* Return the user data for a widget */
UDWORD widgGetUserData2(W_SCREEN *psScreen, UDWORD id);

/* Set the user data for a widget */
void widgSetUserData2(W_SCREEN *psScreen, UDWORD id,UDWORD UserData);

/* Return the user data for the returned widget */
extern void *widgGetLastUserData(W_SCREEN *psScreen);

/* Get widget structure */
extern WIDGET *widgGetFromID(W_SCREEN *psScreen, UDWORD id);

/* Set tip string for a widget */
extern void widgSetTip( W_SCREEN *psScreen, UDWORD id, STRING *pTip );

/* Colour numbers */
enum _w_colour
{
	WCOL_BKGRND,	// Background colours
	WCOL_TEXT,		// Text colour
	WCOL_LIGHT,		// Light colour for 3D effects
	WCOL_DARK,		// Dark colour for 3D effects
	WCOL_HILITE,	// Hilite colour
	WCOL_CURSOR,	// Edit Box cursor colour
	WCOL_TIPBKGRND,	// Background for the tool tip window
	WCOL_DISABLE,	// Text colour on a disabled button

	WCOL_MAX,		// all colour numbers are less than this
};

/* Set a colour on a form */
extern void widgSetColour(W_SCREEN *psScreen, UDWORD id, UDWORD colour,
						  UBYTE red, UBYTE green, UBYTE blue);

// Set the global toop tip text colour.
extern void	widgSetTipColour(W_SCREEN *psScreen, UBYTE red, UBYTE green, UBYTE blue);

/* Possible states for a button */
#define WBUT_DISABLE	0x0001		// Disable (grey out) a button
#define WBUT_LOCK		0x0002		// Fix a button down
#define WBUT_CLICKLOCK	0x0004		// Fix a button down but it is still clickable
#define WBUT_FLASH		0x0008		// Make a button flash.

extern void widgSetButtonFlash(W_SCREEN *psScreen, UDWORD id);
extern void widgClearButtonFlash(W_SCREEN *psScreen, UDWORD id);

/* Get a button or clickable form's state */
extern UDWORD widgGetButtonState(W_SCREEN *psScreen, UDWORD id);

/* Set a button or clickable form's state */
extern void widgSetButtonState(W_SCREEN *psScreen, UDWORD id, UDWORD state);


/* The keys that can be used to press a button */
#define WKEY_NONE			0
#define WKEY_PRIMARY		1
#define WKEY_SECONDARY		2

/* Return which key was used to press the last returned widget */
extern UDWORD widgGetButtonKey(W_SCREEN *psScreen);

/* Initialise the set of widgets that make up a screen.
 * Call this once before calling widgRunScreen and widgDisplayScreen.
 * This should only be called once before calling Run and Display as many times
 * as is required.
 */
extern void widgStartScreen(W_SCREEN *psScreen);

/* Clean up after a screen has been run.
 * Call this after the widgRunScreen / widgDisplayScreen cycle.
 */
extern void widgEndScreen(W_SCREEN *psScreen);

/* Execute a set of widgets for one cycle.
 * Return the id of the widget that was activated, or 0 for none.
 */
extern UDWORD widgRunScreen(W_SCREEN *psScreen);

/* Display the screen's widgets in their current state
 * (Call after calling widgRunScreen, this allows the input
 *  processing to be seperated from the display of the widgets).
 */
extern void widgDisplayScreen(W_SCREEN *psScreen);

// Set the current audio callback function and audio id's.
extern void WidgSetAudio(WIDGET_AUDIOCALLBACK Callback,SWORD HilightID,SWORD ClickedID);

// Get pointer to current audio callback function.
extern WIDGET_AUDIOCALLBACK WidgGetAudioCallback(void);

// Get current audio ID for hilight.
extern SWORD WidgGetHilightAudioID(void);

// Get current audio ID for clicked.
extern SWORD WidgGetClickedAudioID(void);

void sliderEnableDrag(BOOL Enable);

void	setWidgetsStatus( BOOL var );
BOOL	getWidgetsStatus( void );

#endif

