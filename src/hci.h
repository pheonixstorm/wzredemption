/*
 * HCI.h
 *
 * Function definitions for the in game interface code.
 */
#ifndef _hci_h
#define _hci_h

#include "widget.h"
#include "Message.h"
#ifdef WIN32
#include "cdspan.h"
#include "PieClip.h"
#endif
//#include "IntImage.h"

// store the objects that are being used for the object bar
#define			MAX_OBJECTS		15//10 we need at least 15 for the 3 different types of factory


#define	BASE_COORDS_X	(640)
#define	BASE_COORDS_Y	(480)

#ifdef WIN32
#define	BASE_COORDS_X	(640)
#define	BASE_COORDS_Y	(480)
#define E_W (DISP_WIDTH - BASE_COORDS_X)
#define E_H (DISP_HEIGHT - BASE_COORDS_Y)
#define	D_W	((DISP_WIDTH - BASE_COORDS_X)/2)
#define	D_H ((DISP_HEIGHT - BASE_COORDS_Y)/2)
#else
#define	D_W (0)
#define	D_H	(0)
#define E_W (0)
#define E_H (0)
#endif

#define IDRET_FORM				1		// The reticule form
#define IDRET_OPTIONS			2		// option button
#define IDRET_BUILD				3		// build button
#define IDRET_MANUFACTURE		4		// manufacture button
#define IDRET_RESEARCH			5		// research button
#define IDRET_INTEL_MAP			6		// intelligence map button
#define IDRET_DESIGN			7		// design droids button
#define IDRET_CANCEL			8		// central cancel button
#define IDRET_COMMAND			9		// command droid button
#define IDRET_TRANSPORTER		10		// transporter button
#define IDRET_ORDER				11		// droid order button
#define IDPOW_POWERBAR_T		102		// power bar - trough
#define	IDTRANTIMER_BUTTON		11012	//transporter button on timer display



/* Object screen IDs */	 

#define IDOBJ_FORM			3000		// The object back form for build/manufacture/research
#define IDOBJ_CLOSE			3001		// The form for the close button
#define IDOBJ_OBJSTART		3002		// The first ID for droids/factories/research
#define IDOBJ_OBJEND		3021		// The last ID for droids/factories/research
#define IDOBJ_STATSTART		3100		// The first ID for stats
#define IDOBJ_STATEND		3199		// The last ID for stats
#define IDOBJ_PROGBARSTART  3200		// The first ID for stats progress bars.
#define IDOBJ_PROGBAREND	3299		// The last ID for stats progress bars.
#define IDOBJ_POWERBARSTART 3300		// The first ID for power bars.
#define IDOBJ_POWERBAREND	3399		// The first ID for power bars.
#define IDOBJ_COUNTSTART	3400		// The first ID for progress number labels.
#define IDOBJ_COUNTEND		3499		// The last ID for progress number labels.
#define IDOBJ_TABFORM		3500		// The object tab form for build/manufacture/research
#define IDOBJ_FACTORYSTART	3600		// The first ID for factory number labels
#define IDOBJ_FACTORYEND	3699		// The last ID for factory number labels
#define IDOBJ_CMDEXPSTART	3700		// The first ID for factory number labels
#define IDOBJ_CMDEXPEND		3749		// The last ID for factory number labels
#define IDOBJ_CMDFACSTART	3750		// The first ID for factory number labels
#define IDOBJ_CMDFACEND		3799		// The last ID for factory number labels
#define IDOBJ_CMDVTOLFACSTART	3800	// The first ID for VTOL factory number labels
#define IDOBJ_CMDVTOLFACEND		3849	// The last ID for VTOL factory number labels


#define IDSTAT_FORM				4000		// The stats form for structure/droid/research type
#define IDSTAT_TITLEFORM		4001		// The form for the close box
#define IDSTAT_LABEL			4002		// Unused
#define IDSTAT_CLOSE			4003		// The stats close box
#define IDSTAT_TABFORM			4004		// The tab form with the stats buttons
#define IDSTAT_START			4100		// The first stats ID
#define IDSTAT_END				4179		// The last stats ID
//#define IDSTAT_BARSTART		4200
#define IDSTAT_BAREND			4299
#define IDSTAT_TIMEBARSTART		4300
#define IDSTAT_TIMEBAREND		4399
#define IDSTAT_SLIDER			4400
#define IDSTAT_SLIDERCOUNT		4401
#define IDSTAT_INFINITE_BUTTON	4402
#define IDSTAT_LOOP_BUTTON		4403
#define IDSTAT_LOOP_LABEL		4404
#define IDSTAT_DP_BUTTON		4405
#define IDSTAT_RESICONSTART		4500
#define IDSTAT_RESICONEND		4599
#define IDSTAT_PRODSTART		4600
#define IDSTAT_PRODEND			4699
#define IDSTAT_MANULIMITS		4700

#define IDSTAT_ALLYSTART		4800
#define IDSTAT_ALLYEND			4900

// Reticule position.
#define RET_X				23
#define RET_Y				(324+E_H) 
#define RET_FORMWIDTH		132
#define RET_FORMHEIGHT		132

/* Option positions */
#define OPT_GAP			5

// Object screen position.
#define BASE_GAP		6
#define OBJ_BACKX		(RET_X + RET_FORMWIDTH + BASE_GAP + D_W)	// X coord of object screen back form.
#define OBJ_BACKY		RET_Y	// Y coord of object screen back form.
#define OBJ_BACKWIDTH	320	//316		// Width of object screen back form.
#define OBJ_BACKHEIGHT	115		// Height of object screen back form. 

/* Build screen positions */
#define OBJ_TABX		2	// X coord of object screen tab form.
#define OBJ_TABY		6	// Y coord of object screen tab form.
#define OBJ_WIDTH		316	//312//310	// Width of object screen tab form.  
#define OBJ_HEIGHT		112	// Height of object screen tab form. 
#define OBJ_GAP			2	// Gap between buttons.
#define OBJ_STARTX		2	// Offset of first obj button from left of tab form.
#define OBJ_STARTY		42	//44	// Offset of first obj button from top of tab form.
#define OBJ_STATSTARTY	0

//slider bar positions
#ifdef PSX
#define STAT_SLDX			4	// Slider x.
#define STAT_SLDY			8	// Slider y.
#else
#define STAT_SLDX			8	// Slider x.
#define STAT_SLDY			4	// Slider y.
#endif
#define STAT_SLDWIDTH		70	// Slider width.
#define STAT_SLDHEIGHT		12	//4	// Slider height.

// Power bar position.
#define POW_X			OBJ_BACKX
#define POW_Y			(OBJ_BACKY + OBJ_BACKHEIGHT + 6)
#define POW_BARWIDTH	308

#define POW_GAPX		5
#define POW_GAPY		2
#define POW_CLICKBARMAJORRED	0xcc
#define POW_CLICKBARMAJORGREEN	0
#define POW_CLICKBARMAJORBLUE	0

//tab details
#define OBJ_TABWIDTH	26	
#define OBJ_TABHEIGHT	11
#define	OBJ_TABOFFSET	2

/* close button data */
#define CLOSE_WIDTH		15
#define CLOSE_HEIGHT	15
#define CLOSE_SIZE		15

// Stat screen position.
#define STAT_X				23
#define STAT_Y				(45 + E_H)
#define STAT_WIDTH			RET_FORMWIDTH	// Width of the tab form.
#define STAT_HEIGHT			273				// Height of the tab form.
#define STAT_TABWIDTH		15
#define STAT_TABHEIGHT		40
#define STAT_TABFORMX		0	// Offset of the tab form within the main form.
#define STAT_TABFORMY		18	// Offset of the tab form within the main form.


// 2 16 bit values packed into a DWORD.
#define PACKDWORD(a,b)	( ( (a)<<16 ) | (b) )
#define UNPACKDWORD_HI(a) ( (a)>>16 )					
#define UNPACKDWORD_LOW(a) ( (a) & 0xffff)

// 3 10 bit values packed into a DWORD.
#define PACKDWORD_TRI(a,b,c) ( (((a) & 0x3ff) << 20) | (((b) & 0x3ff) << 10) | ((c) & 0x3ff) )
#define UNPACKDWORD_TRI_A(a) ( ((a)>>20) & 0x3ff )
#define UNPACKDWORD_TRI_B(a) ( ((a)>>10) & 0x3ff )
#define UNPACKDWORD_TRI_C(a) ( (a) & 0x3ff)

// 4 8 bit values packed into a DWORD.
#define PACKDWORD_QUAD(a,b,c,d) ( (((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((c) & 0xff) << 8) | ((d) & 0xff)  )
#define UNPACKDWORD_QUAD_A(a) ( ((a)>>24) & 0xff )
#define UNPACKDWORD_QUAD_B(a) ( ((a)>>16) & 0xff )
#define UNPACKDWORD_QUAD_C(a) ( ((a)>>8) & 0xff )
#define UNPACKDWORD_QUAD_D(a) ( (a) & 0xff)


//#define BUILDPOINTS_STRUCTDIV 1
//#define BUILDPOINTS_DROIDDIV 5
//#define POWERPOINTS_STRUCTDIV 1
#define POWERPOINTS_DROIDDIV	5 //3 

#define OBJ_BUTWIDTH		60		// Button width.
#define OBJ_BUTHEIGHT		46		// Button height.
#ifdef WIN32
#define OBJ_TEXTX			2
#else
#define OBJ_TEXTX			4
#endif
#define OBJ_T1TEXTY			2
#define OBJ_T2TEXTY			14
#define OBJ_T3TEXTY			26
#define OBJ_B1TEXTY			8

#define STAT_SLD_OX	(0)			// Stat window slider offset.
#define STAT_SLD_OY	(0)
#define STAT_SLDSTOPS		10	// Slider number of stops.
#ifdef WIN32
#define STAT_PROGBARX			3
#define STAT_PROGBARY			36
#define STAT_PROGBARWIDTH		(OBJ_BUTWIDTH-8)
#define STAT_PROGBARHEIGHT		4
#define STAT_TIMEBARX			3
#define STAT_TIMEBARY			(OBJ_BUTHEIGHT-STAT_PROGBARHEIGHT-3)
#define STAT_POWERBARX			3
#define STAT_POWERBARY			(OBJ_BUTHEIGHT-STAT_PROGBARHEIGHT-6)
#else
#define STAT_PROGBARX			4
#define STAT_PROGBARY			34
#define STAT_PROGBARWIDTH		(OBJ_BUTWIDTH-12)
#define STAT_PROGBARHEIGHT		4
#define STAT_TIMEBARX			4
#define STAT_TIMEBARY			(OBJ_BUTHEIGHT-STAT_PROGBARHEIGHT-8)
#define STAT_POWERBARX			4
#define STAT_POWERBARY			(OBJ_BUTHEIGHT-STAT_PROGBARHEIGHT-6)
#endif
#define STAT_PROGBARMAJORRED	255//0xcc
#define STAT_PROGBARMAJORGREEN	235//0
#define STAT_PROGBARMAJORBLUE	19//0
#define STAT_PROGBARMINORRED	0x55
#define STAT_PROGBARMINORGREEN	0
#define STAT_PROGBARMINORBLUE	0
#define STAT_PROGBARTROUGHRED	0
#define STAT_PROGBARTROUGHGREEN	32
#define STAT_PROGBARTROUGHBLUE	64
#define STAT_TEXTRED	255
#define STAT_TEXTGREEN	255
#define STAT_TEXTBLUE	0

/* maximum array sizes */
#define	MAXSTRUCTURES	80
#define	MAXRESEARCH		80 //40 can have 80 topic displayed at one now AB 13/09/99
#define	MAXTEMPLATES	40 //20
#define	MAXFEATURES		40
#define	MAXCOMPONENT	40
#define	MAXEXTRASYS		40

typedef enum {
	INT_NORMAL,		// Standard mode (just the reticule)
#ifdef WIN32
	INT_OPTION,		// Option screen
	INT_EDIT,		// Edit mode
#endif
	INT_EDITSTAT,	// Stat screen up for placing objects
	INT_OBJECT,		// Object screen
	INT_STAT,		// Object screen with stat screen
	INT_CMDORDER,	// Object screen with command droids and orders screen
	INT_DESIGN,		// Design screen
	INT_INTELMAP,	// Intelligence Map
	INT_ORDER,
	INT_INGAMEOP,	// in game options.
	//INT_TUTORIAL,	// Tutorial mode - message display
	INT_TRANSPORTER, //Loading/unloading a Transporter
	INT_MISSIONRES,	// Results of a mission display.
	INT_MULTIMENU,	// multiplayer only, player stats etc...
	INT_CDCHANGE,		// CD Change message box 

    INT_MAXMODE,   //leave as last so we can start the objMode at this value
} INTMODE;

//NOT ANYMORE! 10/08/98 AB
//#ifdef WIN32
//#define INCLUDE_PRODSLIDER	// Include quantity slider in manufacture window.
//#endif

//#ifdef WIN32
#define INCLUDE_FACTORYLISTS
//#endif

extern INTMODE intMode;

/* The widget screen */
extern W_SCREEN		*psWScreen;

/* the widget font */
extern int WFont;
extern int SmallWFont;

/* Which is the currently selected player */
extern UDWORD			selectedPlayer;

// The last widget ID from widgRunScreen
extern UDWORD			intLastWidget;

/* The button ID of the objects stat when the stat screen is displayed */
extern UDWORD			objStatID;

/* The flag to specify if the Intelligence screen is up */
//extern BOOL				intelMapUp;

/* The current template for the design screen to start with*/
extern DROID_TEMPLATE	*psCurrTemplate;
extern DROID_TEMPLATE	**apsTemplateList;

//two colours used for drawing the footprint outline for objects in 2D
extern UDWORD	outlineOK;
extern UDWORD	outlineNotOK;

//two colours used for drawing the footprint outline for objects in 3D
#define			outlineOK3D		255
#define			outlineNotOK3D	 14		//arbitary value!

//value gets set to colour used for drawing
extern UDWORD	outlineColour;
extern UDWORD	outlineColour3D;

//Buffer to hold the 3D view for the Intelligence Screen
extern iSurface *pIntelMapSurface;
/*Message View Buffer width and height - MAXIMUM Sizes! - only need to be 
as big as Pie View in Research Msg now*/
#define	MSG_BUFFER_WIDTH		INTMAP_PIEWIDTH//DISP_WIDTH//640
#define	MSG_BUFFER_HEIGHT		INTMAP_PIEHEIGHT//DISP_HEIGHT//480

/* pointer to hold the imd to use for a new template in the design screen */
extern iIMDShape	*pNewDesignIMD;

#ifdef WIN32
extern UBYTE	*DisplayBuffer;
extern SDWORD	displayBufferSize;
#endif

extern BOOL ClosingMessageView;
extern BOOL ClosingIntelMap;
extern BOOL	ClosingTrans;
extern BOOL	ClosingTransCont;
extern BOOL	ClosingTransDroids;

/* Initialise the in game interface */
extern BOOL intInitialise(void);

/* Shut down the in game interface */
extern void intShutDown(void);

/* Return codes for the widget interface */
typedef enum _int_retval
{
	INT_NONE,		// no key clicks have been intercepted
	INT_INTERCEPT,	// key clicks have been intercepted
	//INT_FULLSCREENPAUSE,	// The widget interface is full screen and
							// the rest of the game should pause
	//INT_INTELPAUSE,			// The Intelligence Map is up and all update 
							// routines should pause - hopefully!
	INT_INTELNOSCROLL,		//The 3DView of the intelligence screen is up
							// and we don't want scroll (or update!)
	INT_QUIT,		// The game should quit
} INT_RETVAL;

/* Run the widgets for the in game interface */
extern INT_RETVAL intRunWidgets(void);

/* Display the widgets for the in game interface */
extern void intDisplayWidgets(void);

/* Add the reticule widgets to the widget screen */
extern BOOL intAddReticule(void);
extern void intRemoveReticule(void);

/* Set the map view point to the world coordinates x,y */
extern void intSetMapPos(UDWORD x, UDWORD y);

/* Set the map view point to the world coordinates x,y */
extern void intSetMapPos(UDWORD x, UDWORD y);

/* Tell the interface when an object is created
 * - it may have to be added to a screen
 */
extern void intNewObj(BASE_OBJECT *psObj);

/* Tell the interface a construction droid has finished building */
extern void intBuildFinished(DROID *psDroid);
/* Tell the interface a construction droid has started building*/
extern void intBuildStarted(DROID *psDroid);
/* Tell the interface a research facility has completed a topic */
extern void intResearchFinished(STRUCTURE *psBuilding);
/* Tell the interface a factory has completed building ALL droids */
extern void intManufactureFinished(STRUCTURE *psBuilding);

/* Sync the interface to an object */
extern void intObjectSelected(BASE_OBJECT *psObj);

// add the construction interface if a constructor droid is selected
extern void intConstructorSelected(DROID *psDroid);
extern BOOL intBuildSelectMode(void);
extern BOOL intDemolishSelectMode(void);
extern BOOL intBuildMode(void);

// add the construction interface if a constructor droid is selected
void intCommanderSelected(DROID *psDroid);

extern UWORD numForms(UDWORD total, UDWORD perForm);

//sets up the Intelligence Screen as far as the interface is concerned
//extern void addIntelScreen(BOOL playImmediate);
extern void addIntelScreen(void);

// update shadow...
extern void intSetShadowPower(UDWORD quantity);

/* Reset the widget screen to just the reticule */
extern void intResetScreen(BOOL NoAnim);

/* Refresh icons on the interface, without disturbing the layout. i.e. smartreset*/
extern VOID intRefreshScreen(VOID);

/* Add the options widgets to the widget screen */
extern BOOL intAddOptions(void);

/* Remove the stats widgets from the widget screen */
extern void intRemoveStats(void);

/* Remove the stats widgets from the widget screen */
extern void intRemoveStatsNoAnim(void);

/*sets which list of structures to use for the interface*/
extern STRUCTURE* interfaceStructList(void);

//sets up the Transporter Screen as far as the interface is concerned
extern void addTransporterInterface(DROID *psSelected, BOOL onMission);

#ifdef WIN32
/* CD change box */
extern void addCDChangeInterface( CD_INDEX CDrequired,
		CDSPAN_CALLBACK fpOKCallback, CDSPAN_CALLBACK fpCancelCallback );
#endif

/*causes a reticule button to start flashing*/
extern void flashReticuleButton(UDWORD buttonID);

// stop a reticule button flashing
extern void stopReticuleButtonFlash(UDWORD buttonID);

//toggles the Power Bar display on and off
extern void togglePowerBar(void);

//displays the Power Bar
extern void intShowPowerBar(void);

//hides the power bar from the display
//extern void intHidePowerBar(void);

//hides the power bar from the display - regardless of what player requested!
extern void forceHidePowerBar(void);

/* Add the Proximity message buttons */
extern BOOL intAddProximityButton(PROXIMITY_DISPLAY *psProxDisp, UDWORD inc);

/*Remove a Proximity Button - when the message is deleted*/
extern void intRemoveProximityButton(PROXIMITY_DISPLAY *psProxDisp);

/* Allows us to fool the widgets with a keypress */
void	setKeyButtonMapping( UDWORD	val );

#ifdef PSX
void SetMouseFormPosition(W_FORMINIT *sFormInit);

void intDestroyStructure(STRUCTURE *psStruct);
void intDestroyDroid(DROID *psDroid);

void intInitObjectCycle(void);
#endif

STRUCTURE *intFindAStructure(void);
STRUCTURE* intGotoNextStructureType(UDWORD structType,BOOL JumpTo,BOOL CancelDrive);
DROID *intGotoNextDroidType(DROID *CurrDroid,UDWORD droidType,BOOL AllowGroup);

/*Checks to see if there are any research topics to do and flashes the button*/
extern void intCheckResearchButton(void);

// see if a reticule button is enabled
extern BOOL intCheckReticuleButEnabled(UDWORD id);

//access function for selected object in the interface
extern BASE_OBJECT * getCurrentSelected(void);

//initialise all the previous obj - particularly useful for when go Off world!
extern void intResetPreviousObj(void);

extern void HandleClosingWindows(void);

extern BOOL intIsRefreshing(void);

void intReopenBuild(BOOL reopen);
BOOL intGetReopenBuild(void);

extern void intDemolishCancel(void);

#endif

