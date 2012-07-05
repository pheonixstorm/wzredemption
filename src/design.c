/*
 * Design.c
 *
 * Functions for design screen.
 *
 */

#include <stdio.h>
#include <math.h>

#include "Frame.h"
#include "Widget.h"

#include "Objects.h"
#include "Loop.h"
#include "Edit2D.h"
#include "Map.h"

/* Includes direct access to render library */
#include "ivisdef.h"
#include "vid.h"
#include "pieMatrix.h"//matrix code
#include "pieState.h"

#ifdef PSX
#include "Primatives.h"
#include "DrawIMD_psx.h"
#include "InitPSX.h"	// for DISPLAYMODE_PAL & DISPLAYMODE_NTSC.
#endif

#include "Display3d.h"
#include "Edit3D.h"
#include "Disp2D.h"
#include "Structure.h"
#include "Research.h"
#include "Function.h"
#include "GTime.h"
#include "HCI.h"
#include "Stats.h"
#include "game.h"
#include "power.h"
#include "audio.h"
#include "audio_id.h"
#include "WidgInt.h"
#include "bar.h"
#include "form.h"
#include "label.h"
#include "button.h"
#include "editbox.h"
#include "slider.h"
#include "fractions.h"
#include "Order.h"
#include "Projectile.h"

#include "IntImage.h"
#include "IntDisplay.h"
#include "Design.h"
#include "Text.h"
#include "component.h"
#include "Script.h"
#include "scriptTabs.h"
#include "WinMain.h"
#include "Objects.h"
#include "display.h"
#include "Console.h"
#include "CmdDroid.h"
#include "ScriptExtern.h"
#ifdef PSX
#include "StringEntry.h"
#include "csnap.h"
#include "dcache.h"
#include "VPad.h"
#include "CtrlPSX.h"
#include "mission.h"
extern CURSORSNAP InterfaceSnap;
#endif

#ifdef WIN32

#include "MultiPlay.h"
#include "multistat.h"

#define FLASH_BUTTONS		// Enable flashing body part buttons.

#endif

#define MAX_TABS	4

#define TAB_USEMAJOR 0
#define TAB_USEMINOR 1

//how many buttons can be put on the system component form
#define DES_BUTSPERFORM  8

#define MAX_DESIGN_COMPONENTS 40	// Max number of stats the design screen can cope with.
#define MAX_SYSTEM_COMPONENTS 32	// can only fit 8 buttons on a system component form


/***************************************************************************************/
/*                  Max values for the design bar graphs                               */

#define DBAR_WEAPMAXROF				240 //700//3500	// maximum shots per minute???

//these are calculated now when the game gets loaded up
//#define DBAR_MAXWEIGHT				3500			// maximum weight for a component
//#define DBAR_SENSORMAXRANGE			(30*TILE_UNITS)	// maximum sensor range
//#define DBAR_SENSORMAXPOWER			1000			// maximum sensor power
//#define DBAR_ECMMAXPOWER			1000			// maximum ecm power
//#define DBAR_REPAIRMAXPOINTS		1000			// maximum repair points
//#define DBAR_WEAPMAXRANGE			(30*TILE_UNITS)	// maximum weapon range
//#define DBAR_WEAPMAXDAMAGE			30				// maximum weapon damage
//#define DBAR_BODYMAXARMOUR			25				// maximum armour on the body
//#define DBAR_BODYMAXPOINTS			1000			// maximum body points of the body
//#define DBAR_BODYMAXPOWER			20000			// maximum power output of the body
//#define DBAR_PROPMAXSPEED			1000//2000			// maximum propulsion speed factor
//#define DBAR_CONSTMAXPOINTS			20				// maximum build points for a constructor
//#define DBAR_MAXPOWER				300				// maximum power required to build a template

#define DBAR_TEMPLATEMAXPOINTS      8400            //maximum body points for a template
#define DBAR_TEMPLATEMAXPOWER       1000            //maximum power points for a template

/* The maximum number of characters on the component buttons */
#define DES_COMPBUTMAXCHAR			5
/* The maximum number of characters in a design name */
#define DES_NAMEMAXCHAR				MAX_STR_SIZE


/* Which type of system is displayed on the design screen */
typedef enum _des_sysmode
{
	IDES_SENSOR,		// The sensor clickable is displayed
	IDES_ECM,			// The ECM clickable is displayed
	IDES_CONSTRUCT,		// The Constructor clickable is displayed
	IDES_REPAIR,		// The Repair clickable is displayed
	IDES_WEAPON,		// The Weapon clickable is displayed
	IDES_COMMAND,		// The command droid clickable is displayed
	IDES_NOSYSTEM,		// No system clickable has been displayed
} DES_SYSMODE;
DES_SYSMODE desSysMode;

/* The major component tabs on the design screen */
#define IDES_MAINTAB	0
#define IDES_EXTRATAB	1
#define IDES_EXTRATAB2	2

/* Which component type is being selected on the design screen */
typedef enum _des_compmode
{
	IDES_BRAIN,			// The brain for the droid
	IDES_SYSTEM,		// The main system for the droid (sensor, ECM, constructor)
	IDES_TURRET,		// The weapon for the droid
	IDES_BODY,			// The droid body
	IDES_PROPULSION,	// The propulsion system
	IDES_NOCOMPONENT,	// No system has been selected
} DES_COMPMODE;
DES_COMPMODE desCompMode;

/* Which type of propulsion is being selected */
typedef enum _des_propmode
{
	IDES_GROUND,		// Ground propulsion (wheeled, tracked, etc).
	IDES_AIR,			// Air propulsion
	IDES_NOPROPULSION,	// No propulsion has be selected
} DES_PROPMODE;
DES_PROPMODE desPropMode;


#ifdef WIN32
#define STRING_BUFFER_SIZE (32 * MAX_NAME_SIZE)
char StringBuffer[STRING_BUFFER_SIZE];
#endif

/* Design screen positions */
#define DESIGN_Y				(59 + D_H)	//the top left y value for all forms on the design screen

#define DES_TABTHICKNESS	0
#define DES_MAJORSIZE		40
#define DES_MINORSIZE		11
#define DES_TABBUTGAP		2
#define DES_TABBUTWIDTH		60
#define DES_TABBUTHEIGHT	46
#define DES_TITLEY			10
#define DES_TITLEHEIGHT		20
#define DES_NAMELABELX		10
#define DES_NAMELABELWIDTH	100
#define	DES_TAB_LEFTOFFSET 	OBJ_TABOFFSET
#define	DES_TAB_RIGHTOFFSET	OBJ_TABOFFSET
#define	DES_TAB_SYSOFFSET	0
#define DES_TAB_SYSWIDTH	12
#define DES_TAB_SYSHEIGHT	19
#define DES_TAB_WIDTH		OBJ_TABWIDTH
#define DES_TAB_HEIGHT		OBJ_TABHEIGHT
#define DES_TAB_SYSHORZOFFSET	OBJ_TABOFFSET
#define DES_TAB_SYSGAP		4

#define DES_LEFTFORMX		RET_X
#define DES_LEFTFORMY		DESIGN_Y
#define DES_LEFTFORMWIDTH	RET_FORMWIDTH
#define DES_LEFTFORMHEIGHT	258
#define	DES_LEFTFORMBUTX	2
#define	DES_LEFTFORMBUTY	2

#define DES_CENTERFORMWIDTH		315
#define DES_CENTERFORMHEIGHT	262
#define DES_CENTERFORMX			POW_X
#define DES_CENTERFORMY			DESIGN_Y

#define DES_3DVIEWX				8
#define DES_3DVIEWY				25
#define DES_3DVIEWWIDTH			236
#define DES_3DVIEWHEIGHT		192

#define	DES_STATSFORMX			POW_X
#define	DES_STATSFORMY			(DES_CENTERFORMY + DES_CENTERFORMHEIGHT + 3)
#define	DES_STATSFORMWIDTH		DES_CENTERFORMWIDTH
#define	DES_STATSFORMHEIGHT		100

#define	DES_PARTFORMX			DES_3DVIEWX + DES_3DVIEWWIDTH + 2
#define	DES_PARTFORMY			DES_3DVIEWY
#define	DES_PARTFORMHEIGHT		DES_3DVIEWHEIGHT

#define DES_POWERFORMX			DES_3DVIEWX
#define DES_POWERFORMY			(DES_3DVIEWY + DES_3DVIEWHEIGHT + 2)
#define DES_POWERFORMWIDTH		(DES_CENTERFORMWIDTH - 2*DES_POWERFORMX)
#define DES_POWERFORMHEIGHT		40

#define DES_RIGHTFORMX		RADTLX
#define DES_RIGHTFORMY		DESIGN_Y
#define DES_RIGHTFORMWIDTH	(RET_FORMWIDTH + 20)
#define DES_RIGHTFORMHEIGHT DES_LEFTFORMHEIGHT
#define	DES_RIGHTFORMBUTX	2
#define	DES_RIGHTFORMBUTY	2

#define DES_BARFORMX		6
#define DES_BARFORMY		6
#define	DES_BARFORMWIDTH	300
#define	DES_BARFORMHEIGHT	85

#define DES_NAMEBOXX		DES_3DVIEWX
#define DES_NAMEBOXY		6
#define	DES_NAMEBOXWIDTH	DES_CENTERFORMWIDTH - 2*DES_NAMEBOXX
#define	DES_NAMEBOXHEIGHT	14

/* The central boxes on the design screen */
#define DES_COMPBUTWIDTH	150
#define DES_COMPBUTHEIGHT	85
#define DES_BRAINX			165
#define DES_BRAINY			50
#define DES_BRAINVIEWX		(DES_BRAINX + 10)
#define DES_BRAINVIEWY		(DES_BRAINY + DES_COMPBUTHEIGHT + 10)
#define DES_BRAINVIEWWIDTH	80

#define DES_MAINBUTWIDTH	36
#define DES_MAINBUTHEIGHT	24

#define DES_ICONX			5			
#define DES_ICONY			22//28	
#define DES_ICONWIDTH		76	
#define DES_ICONHEIGHT		53

#define DES_POWERX				1
#define DES_POWERY				6
#define DES_POWERSEPARATIONX	4
#define DES_POWERSEPARATIONY	2

#define	DES_PARTSEPARATIONX		6
#define	DES_PARTSEPARATIONY		6

/* Positions of stuff on the clickable boxes (Design screen) */
#define DES_CLICKLABELHEIGHT	14
#define DES_CLICKBARX			154
#define DES_CLICKBARY			7
#define DES_CLICKBARWIDTH		140
#define DES_CLICKBARHEIGHT		11
#define DES_CLICKGAP			9
#define DES_CLICKBARNAMEX		126
#define DES_CLICKBARNAMEWIDTH	20
#define DES_CLICKBARNAMEHEIGHT	19

#define DES_CLICKBARMAJORRED	255		//0xcc
#define DES_CLICKBARMAJORGREEN	235		//0
#define DES_CLICKBARMAJORBLUE	19		//0
#define DES_CLICKBARMINORRED	0x55
#define DES_CLICKBARMINORGREEN	0
#define DES_CLICKBARMINORBLUE	0

#define DES_WEAPONBUTTON_X		26
#define DES_SYSTEMBUTTON_X		68
#define DES_SYSTEMBUTTON_Y		10

// Stat bar y positions.
#ifdef WIN32
#define	DES_STATBAR_Y1	(DES_CLICKBARY)
#define	DES_STATBAR_Y2	(DES_CLICKBARY+DES_CLICKBARHEIGHT + DES_CLICKGAP)
#define	DES_STATBAR_Y3	(DES_CLICKBARY+(DES_CLICKBARHEIGHT + DES_CLICKGAP)*2)
#define	DES_STATBAR_Y4	(DES_CLICKBARY+(DES_CLICKBARHEIGHT + DES_CLICKGAP)*3)
#else
 #ifdef DISPLAYMODE_PAL
	#define	DES_STATBAR_Y1	(DES_CLICKBARY)
	#define	DES_STATBAR_Y2	(DES_CLICKBARY+DES_CLICKBARHEIGHT + DES_CLICKGAP-1)
	#define	DES_STATBAR_Y3	(DES_CLICKBARY+(DES_CLICKBARHEIGHT + DES_CLICKGAP)*2)-2
	#define	DES_STATBAR_Y4	(DES_CLICKBARY+(DES_CLICKBARHEIGHT + DES_CLICKGAP)*3)-4
 #else
	#define	DES_STATBAR_Y1	(DES_CLICKBARY)
	#define	DES_STATBAR_Y2	(DES_CLICKBARY+DES_CLICKBARHEIGHT + DES_CLICKGAP)
	#define	DES_STATBAR_Y3	(DES_CLICKBARY+(DES_CLICKBARHEIGHT + DES_CLICKGAP)*2)
	#define	DES_STATBAR_Y4	(DES_CLICKBARY+(DES_CLICKBARHEIGHT + DES_CLICKGAP)*3)
 #endif
#endif

/* the widget screen */
extern W_SCREEN		*psWScreen;

/* the widget font */
//extern PROP_FONT	*psWFont;
extern int WFont;

extern	UDWORD				objID;					// unique ID creation thing..

/* default droid design template */
DROID_TEMPLATE	sDefaultDesignTemplate;

extern void intDisplayPlainForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void desSetupDesignTemplates( void );

/* Set the current mode of the design screen, and display the appropriate component lists */
static void intSetDesignMode(DES_COMPMODE newCompMode);
/* Set all the design bar graphs from a design template */
static void intSetDesignStats(DROID_TEMPLATE *psTemplate);
/* Set up the system clickable form of the design screen given a set of stats */
static BOOL intSetSystemForm(COMP_BASE_STATS *psStats);
/* Set up the propulsion clickable form of the design screen given a set of stats */
static BOOL intSetPropulsionForm(PROPULSION_STATS *psStats);
/* Add the component tab form to the design screen */
static BOOL intAddComponentForm(UDWORD numButtons);
/* Add the template tab form to the design screen */
static BOOL intAddTemplateForm(DROID_TEMPLATE *psSelected);
/* Add the Major system tab form to the design screen */
static BOOL intAddMultiComponentForm(UBYTE *aSensor, UDWORD numSensor,
							 UBYTE *aECM, UDWORD numECM,
							 UBYTE	*aConstruct, UDWORD numConstruct,
							 UBYTE *aWeapon, UDWORD numWeapon);
// count the number of available components
static UDWORD intNumAvailable(UBYTE *aAvailable, UDWORD numEntries,
							  COMP_BASE_STATS *asStats, UDWORD size);
/* Add the system buttons (weapons, command droid, etc) to the design screen */
static BOOL intAddSystemButtons(SDWORD mode);
/* Add the component buttons to the main tab of the system or component form */
static BOOL intAddComponentButtons(COMP_BASE_STATS *psStats, UDWORD size,
								   UBYTE *aAvailable,	UDWORD numEntries,
								   UDWORD compID,UDWORD WhichTab);
/* Add the droid template buttons to a form */
BOOL intAddTemplateButtons(UDWORD formID, UDWORD formWidth, UDWORD formHeight,
								  UDWORD butWidth, UDWORD butHeight, UDWORD gap,
								  DROID_TEMPLATE *psSelected);
/* Add the component buttons to the main tab of the component form */
static BOOL intAddExtraSystemButtons(UDWORD sensorIndex, UDWORD ecmIndex,
									 UDWORD constIndex, UDWORD repairIndex, UDWORD brainIndex);
/* Set the bar graphs for the system clickable */
static void intSetSystemStats(COMP_BASE_STATS *psStats);
/* Set the shadow bar graphs for the system clickable */
static void intSetSystemShadowStats(COMP_BASE_STATS *psStats);
/* Set the bar graphs for the sensor stats */
static void intSetSensorStats(SENSOR_STATS *psStats);
/* Set the shadow bar graphs for the sensor stats */
static void intSetSensorShadowStats(SENSOR_STATS *psStats);
/* Set the bar graphs for the ECM stats */
static void intSetECMStats(ECM_STATS *psStats);
/* Set the shadow bar graphs for the ECM stats */
static void intSetECMShadowStats(ECM_STATS *psStats);
/* Set the bar graphs for the Repair stats */
static void intSetRepairStats(REPAIR_STATS *psStats);
/* Set the shadow bar graphs for the Repair stats */
static void intSetRepairShadowStats(REPAIR_STATS *psStats);
/* Set the bar graphs for the Constructor stats */
static void intSetConstructStats(CONSTRUCT_STATS *psStats);
/* Set the shadow bar graphs for the Constructor stats */
static void intSetConstructShadowStats(CONSTRUCT_STATS *psStats);
/* Set the bar graphs for the Weapon stats */
static void intSetWeaponStats(WEAPON_STATS *psStats);
/* Set the shadow bar graphs for the weapon stats */
static void intSetWeaponShadowStats(WEAPON_STATS *psStats);
/* Set the bar graphs for the Body stats */
static void intSetBodyStats(BODY_STATS *psStats);
/* Set the shadow bar graphs for the Body stats */
static void intSetBodyShadowStats(BODY_STATS *psStats);
/* Set the bar graphs for the Propulsion stats */
static void intSetPropulsionStats(PROPULSION_STATS *psStats);
/* Set the shadow bar graphs for the Propulsion stats */
static void intSetPropulsionShadowStats(PROPULSION_STATS *psStats);
/* Check whether a droid template is valid */
static BOOL intValidTemplate(DROID_TEMPLATE *psTempl);
/* General display window for the design form */
void intDisplayDesignForm(struct _widget *psWidget, UDWORD xOffset, 
								 UDWORD yOffset, UDWORD *pColours);
/* Sets the Design Power Bar for a given Template */
static void intSetDesignPower(DROID_TEMPLATE *psTemplate);
/* Sets the Power shadow Bar for the current Template with new stat*/
static void intSetTemplatePowerShadowStats(COMP_BASE_STATS *psStats);
/* Sets the Body Points Bar for a given Template */
static void intSetBodyPoints(DROID_TEMPLATE *psTemplate);
/* Sets the Body Points shadow Bar for the current Template with new stat*/
static void intSetTemplateBodyShadowStats(COMP_BASE_STATS *psStats);
/* Return the location of a COMP_BASE_STATS */
//static LOC intGetLocation(COMP_BASE_STATS *psStats);
/*Function to set the shadow bars for all the stats when the mouse is over 
the Template buttons*/
static void runTemplateShadowStats(UDWORD id);

static BOOL intCheckValidWeaponForProp(void);

static BOOL checkTemplateIsVtol(DROID_TEMPLATE *psTemplate);

static UWORD weaponROF(WEAPON_STATS *psStat);

/* save the current Template if valid. Return TRUE if stored */
BOOL saveTemplate(void);

void desCreateDefaultTemplate( void );

//extern iIMDShape *CurrentStatsTemplate;
//extern iIMDShape *CurrentStatsShape;
//extern SWORD CurrentStatsIndex;

UDWORD ViewRotation = 0;
iIMDShape *ViewShape = NULL;

/* The current name of the design */
static STRING			aCurrName[WIDG_MAXSTR];

/* Store a list of component stats pointers for the design screen */
extern UDWORD			maxComponent;
extern UDWORD			numComponent;
extern COMP_BASE_STATS	**apsComponentList;
extern UDWORD			maxExtraSys;
extern UDWORD			numExtraSys;
extern COMP_BASE_STATS	**apsExtraSysList;

/* The button id of the component that is in the design */
static UDWORD			desCompID;

static UDWORD			droidTemplID;

/* The current design being edited on the design screen */
DROID_TEMPLATE			sCurrDesign;

/* Flag to indictate whether a 'spare' template button is required */
static BOOL				newTemplate = FALSE;

void intDisplayTemplateForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void intDisplayStatusForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void intDisplayComponentForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void intDisplayStatForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void intDisplayViewForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void intDisplayTemplateButton(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);
void intDisplayComponentButton(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);

extern void RenderCompositeDroid(UDWORD Index,iVector *Rotation,iVector *Position,iVector *TurretRotation,
								 DROID *psDroid,BOOL RotXYZ);

extern BOOL bRender3DOnly;


/* Add the design widgets to the widget screen */
BOOL _intAddDesign( BOOL bShowCentreScreen )
{
	W_FORMINIT		sFormInit;
	W_LABINIT		sLabInit;
	W_EDBINIT		sEdInit;
	W_BUTINIT		sButInit;
	W_BARINIT		sBarInit;

#ifdef PSX
	HideMissionTimer();
#endif

	desSetupDesignTemplates();

	//set which states are to be paused while design screen is up
	setDesignPauseState();

#ifdef WIN32
	if((GetGameMode() == GS_NORMAL) && !bMultiPlayer)
	{	// Only do this in main game.
		BOOL radOnScreen = radarOnScreen;
			
		bRender3DOnly = TRUE;
		radarOnScreen = FALSE;

	// Just display the 3d, no interface
		displayWorld();
	// Upload the current display back buffer into system memory.
		pie_UploadDisplayBuffer(DisplayBuffer);
	// Make it darker.
		iV_ScaleBitmapRGB(DisplayBuffer,iV_GetDisplayWidth(),iV_GetDisplayHeight(),2,2,2);

		radarOnScreen = radOnScreen;
		bRender3DOnly = FALSE;
	}
#endif

	//initialise flags
	newTemplate = FALSE;

	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	memset(&sLabInit, 0, sizeof(W_LABINIT));
	memset(&sEdInit, 0, sizeof(W_EDBINIT));
	memset(&sButInit, 0, sizeof(W_BUTINIT));
	memset(&sBarInit, 0, sizeof(W_BARINIT));

	/* Add the main design form */
#ifdef PSX
	WidgSetOTIndex(OT2D_FARFARBACK);
#endif
	sFormInit.formID = 0;
	sFormInit.id = IDDES_FORM;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = (SWORD)DES_CENTERFORMX;	//0;
	sFormInit.y = (SWORD)DES_CENTERFORMY;	//0;
	sFormInit.width = DES_CENTERFORMWIDTH;	//DISP_WIDTH-1;
	sFormInit.height = DES_CENTERFORMHEIGHT;	//DES_BASEHEIGHT;
	sFormInit.pDisplay = intDisplayPlainForm;		//intDisplayStatusForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

//	widgSetColour(psWScreen, 0, WCOL_TEXT,
//				255,255,255);
//	widgSetColour(psWScreen, 0, WCOL_CURSOR,
//				255,255,0);

	/* add the edit name box */
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
#ifdef WIN32
	sEdInit.formID = IDDES_FORM;
	sEdInit.id = IDDES_NAMEBOX;
	sEdInit.style = WEDB_PLAIN;
	sEdInit.x = DES_NAMEBOXX;
	sEdInit.y = DES_NAMEBOXY;
	sEdInit.width = DES_NAMEBOXWIDTH;
	sEdInit.height = DES_NAMEBOXHEIGHT;
	sEdInit.pText = strresGetString(psStringRes, STR_DES_NEWVEH);
	sEdInit.FontID = WFont;
	sEdInit.pBoxDisplay = intDisplayEditBox;
	if (!widgAddEditBox(psWScreen, &sEdInit))
	{
		return FALSE;
	}
#endif

	CurrentStatsTemplate = NULL;
//	CurrentStatsShape = NULL;
//	CurrentStatsIndex = -1;

	/* Initialise the current design */
	if (psCurrTemplate != NULL)
	{
		memcpy(&sCurrDesign, psCurrTemplate, sizeof(DROID_TEMPLATE));
#ifndef HASH_NAMES
		strncpy(aCurrName, getStatName( psCurrTemplate ), WIDG_MAXSTR - 1);
		strcpy( sCurrDesign.aName, aCurrName );
#endif
	}
	else
	{
		memcpy(&sCurrDesign, &sDefaultDesignTemplate, sizeof(DROID_TEMPLATE));
		strcpy(aCurrName, strresGetString(psStringRes, STR_DES_NEWVEH));
#ifndef HASH_NAMES
		strcpy( sCurrDesign.aName, aCurrName );
#endif
	}

	/* Add the design templates form */
	if (!intAddTemplateForm(psCurrTemplate))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	/* Add the 3D View form */
	sFormInit.formID = IDDES_FORM;
	sFormInit.id = IDDES_3DVIEW;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = DES_3DVIEWX;
	sFormInit.y = DES_3DVIEWY;
	sFormInit.width = DES_3DVIEWWIDTH;
	sFormInit.height = DES_3DVIEWHEIGHT;
	sFormInit.pDisplay = intDisplayViewForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	/* Add the part button form */
	sFormInit.formID = IDDES_FORM;
	sFormInit.id = IDDES_PARTFORM;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = DES_PARTFORMX;
	sFormInit.y = DES_PARTFORMY;
	sFormInit.width = (UWORD)(iV_GetImageWidth(IntImages, IMAGE_DES_TURRET) +
						2*DES_PARTSEPARATIONX);
	sFormInit.height = DES_PARTFORMHEIGHT;
	sFormInit.pDisplay = intDisplayDesignForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FORE);
#endif

	// add the body part button
	sButInit.formID = IDDES_PARTFORM;
	sButInit.id = IDDES_BODYBUTTON;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = DES_PARTSEPARATIONX;
	sButInit.y = DES_PARTSEPARATIONY;
	sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_BODY);
	sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_BODY);
	sButInit.pTip = strresGetString(psStringRes, STR_DES_BODY);
	sButInit.FontID = WFont;
#ifdef FLASH_BUTTONS
	sButInit.pDisplay = intDisplayButtonFlash;
#else
	sButInit.pDisplay = intDisplayButtonHilight;
#endif
#ifdef WIN32
	sButInit.pUserData = (void*)PACKDWORD_TRI(1, IMAGE_DES_BODYH, IMAGE_DES_BODY);
#else
	sButInit.pUserData = (void*)PACKDWORD_TRI(1, IMAGE_DES_WIDEHILIGHT, IMAGE_DES_BODY);
#endif
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

	// add the propulsion part button
	sButInit.formID = IDDES_PARTFORM;
	sButInit.id = IDDES_PROPBUTTON;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = DES_PARTSEPARATIONX;
	sButInit.y = (UWORD)(iV_GetImageHeight(IntImages, IMAGE_DES_PROPULSION) + 
					2 * DES_PARTSEPARATIONY);
	sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_PROPULSION);
	sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_PROPULSION);
	sButInit.pTip = strresGetString(psStringRes, STR_DES_PROPULSION);
	sButInit.FontID = WFont;
#ifdef FLASH_BUTTONS
	sButInit.pDisplay = intDisplayButtonFlash;
#else
	sButInit.pDisplay = intDisplayButtonHilight;
#endif
#ifdef WIN32
	sButInit.pUserData = (void*)PACKDWORD_TRI(1, IMAGE_DES_PROPULSIONH, IMAGE_DES_PROPULSION);
#else
	sButInit.pUserData = (void*)PACKDWORD_TRI(1, IMAGE_DES_WIDEHILIGHT, IMAGE_DES_PROPULSION);
#endif
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

	// add the turret part button
	sButInit.formID = IDDES_PARTFORM;
	sButInit.id = IDDES_SYSTEMBUTTON;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = DES_PARTSEPARATIONX;
	sButInit.y = (UWORD)(iV_GetImageHeight(IntImages, IMAGE_DES_PROPULSION) +
				 iV_GetImageHeight(IntImages, IMAGE_DES_BODY)   +
				 3*DES_PARTSEPARATIONY);
	sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_TURRET);
	sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_TURRET);
	sButInit.pTip = strresGetString(psStringRes, STR_DES_TURRET);
	sButInit.FontID = WFont;
#ifdef FLASH_BUTTONS
	sButInit.pDisplay = intDisplayButtonFlash;
#else
	sButInit.pDisplay = intDisplayButtonHilight;
#endif
#ifdef WIN32
	sButInit.pUserData = (void*)PACKDWORD_TRI(1, IMAGE_DES_TURRETH, IMAGE_DES_TURRET);
#else
	sButInit.pUserData = (void*)PACKDWORD_TRI(1, IMAGE_DES_WIDEHILIGHT, IMAGE_DES_TURRET);
#endif
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

	/* add the delete button */
	sButInit.formID = IDDES_PARTFORM;
	sButInit.id = IDDES_BIN;
	sButInit.style = WBUT_PLAIN;
	sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_BIN);
	sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_BIN);
	sButInit.x = DES_PARTSEPARATIONX;
	sButInit.y = (UWORD)(DES_PARTFORMHEIGHT - sButInit.height - DES_PARTSEPARATIONY);
	sButInit.pTip = strresGetString(psStringRes, STR_DES_DEL);
	sButInit.FontID = WFont;
	sButInit.pDisplay = intDisplayButtonHilight;
#ifdef WIN32
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_DES_BINH, IMAGE_DES_BIN);
#else
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_DES_WIDEHILIGHT, IMAGE_DES_BIN);
#endif
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FARFARBACK);
#endif
	/* add central stats form */
	sFormInit.formID = 0;
	sFormInit.id = IDDES_STATSFORM;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = (SWORD)DES_STATSFORMX;
	sFormInit.y = (SWORD)DES_STATSFORMY;
	sFormInit.width = DES_STATSFORMWIDTH;
	sFormInit.height = DES_STATSFORMHEIGHT;
	sFormInit.pDisplay = intDisplayPlainForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	/* Add the body form */
#ifdef PSX
	WidgSetOTIndex(OT2D_FARBACK);
#endif
	sFormInit.formID = IDDES_STATSFORM;
	sFormInit.id = IDDES_BODYFORM;
	sFormInit.style = WFORM_CLICKABLE | WFORM_NOCLICKMOVE;
	sFormInit.width = DES_BARFORMWIDTH;
	sFormInit.height = DES_BARFORMHEIGHT;
	sFormInit.x = DES_BARFORMX;
	sFormInit.y = DES_BARFORMY;
	sFormInit.pDisplay = intDisplayStatForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	/* Add the graphs for the Body */
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	sBarInit.formID = IDDES_BODYFORM;
	sBarInit.id = IDDES_BODYARMOUR_K;
	sBarInit.style = WBAR_PLAIN;
	sBarInit.orientation = WBAR_LEFT;
	sBarInit.x = DES_CLICKBARX;
	sBarInit.y = DES_STATBAR_Y1;	//DES_CLICKBARY;
	sBarInit.width = DES_CLICKBARWIDTH;
	sBarInit.height = DES_CLICKBARHEIGHT;
	sBarInit.size = 50;
	sBarInit.sCol.red = DES_CLICKBARMAJORRED;
	sBarInit.sCol.green = DES_CLICKBARMAJORGREEN;
	sBarInit.sCol.blue = DES_CLICKBARMAJORBLUE;
	sBarInit.sMinorCol.red = DES_CLICKBARMINORRED;
	sBarInit.sMinorCol.green = DES_CLICKBARMINORGREEN;
	sBarInit.sMinorCol.blue = DES_CLICKBARMINORBLUE;
	sBarInit.pDisplay = intDisplayStatsBar;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_ARMOUR_KIN);
	sBarInit.iRange = (UWORD)getMaxBodyArmour();//DBAR_BODYMAXARMOUR;
	if (!widgAddBarGraph(psWScreen, &sBarInit))
	{
		return TRUE;
	}

	sBarInit.id = IDDES_BODYARMOUR_H;
	sBarInit.y  = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_ARMOUR_HEAT);
	sBarInit.iRange = (UWORD)getMaxBodyArmour();//DBAR_BODYMAXARMOUR;
	if (!widgAddBarGraph(psWScreen, &sBarInit))
	{
		return TRUE;
	}

	//body points added AB 3/9/97
	//sBarInit.id = IDDES_BODYPOINTS;
	//sBarInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
	//if (!widgAddBarGraph(psWScreen, &sBarInit))
	//{
	//	return TRUE;
	//}
	sBarInit.id = IDDES_BODYPOWER;
	sBarInit.y = DES_STATBAR_Y3;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_POWER);
	sBarInit.iRange = (UWORD)getMaxBodyPower();//DBAR_BODYMAXPOWER;
	if (!widgAddBarGraph(psWScreen, &sBarInit))
	{
		return TRUE;
	}
	sBarInit.id = IDDES_BODYWEIGHT;
	sBarInit.y = DES_STATBAR_Y4;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
	sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
	if (!widgAddBarGraph(psWScreen, &sBarInit))
	{
		return TRUE;
	}
	
	/* Add the labels for the Body */
	sLabInit.formID = IDDES_BODYFORM;
	sLabInit.id = IDDES_BODYARMOURKLAB;
	sLabInit.style = WLAB_PLAIN;
	sLabInit.x = DES_CLICKBARNAMEX;
	sLabInit.y = DES_CLICKBARY - DES_CLICKBARHEIGHT/3;
	sLabInit.width = DES_CLICKBARNAMEWIDTH;
	sLabInit.height = DES_CLICKBARHEIGHT;
//	sLabInit.pText = "Armour against Kinetic weapons";
	sLabInit.pTip = strresGetString(psStringRes, STR_DES_ARMOUR_KIN);
	sLabInit.FontID = WFont;
	sLabInit.pDisplay = intDisplayImage;
    //just to confuse things even more - the graphics were named incorrectly!
	sLabInit.pUserData = (void*)IMAGE_DES_ARMOUR_EXPLOSIVE;//IMAGE_DES_ARMOUR_KINETIC;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return TRUE;
	}
	sLabInit.id = IDDES_BODYARMOURHLAB;
	sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//	sLabInit.pText = "Armour against Heat weapons";
	sLabInit.pTip = strresGetString(psStringRes, STR_DES_ARMOUR_HEAT);
	sLabInit.pDisplay = intDisplayImage;
	sLabInit.pUserData = (void*)IMAGE_DES_ARMOUR_KINETIC;//IMAGE_DES_ARMOUR_EXPLOSIVE;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return TRUE;
	}
	//body points added AB 3/9/97
	//sLabInit.id = IDDES_BODYPOINTSLAB;
	//sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
	//sLabInit.pText = "Body Points";
	//sLabInit.pTip = sLabInit.pText;
	//sLabInit.pDisplay = intDisplayImage;
	//sLabInit.pUserData = (void*)IMAGE_DES_BODYPOINTS;
	//if (!widgAddLabel(psWScreen, &sLabInit))
	//{
	//	return TRUE;
	//}
	sLabInit.id = IDDES_BODYPOWERLAB;
	sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//	sLabInit.pText = "Power";
	sLabInit.pTip = strresGetString(psStringRes, STR_DES_POWER);
	sLabInit.pDisplay = intDisplayImage;
	sLabInit.pUserData = (void*)IMAGE_DES_POWER;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return TRUE;
	}
	sLabInit.id = IDDES_BODYWEIGHTLAB;
	sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//	sLabInit.pText = "Weight";
	sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
	sLabInit.pDisplay = intDisplayImage;
	sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return TRUE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FORE);
#endif

	/* add power/points bar subform */
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = IDDES_FORM;
	sFormInit.id = IDDES_POWERFORM;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = DES_POWERFORMX;
	sFormInit.y = DES_POWERFORMY;
	sFormInit.width = DES_POWERFORMWIDTH;
	sFormInit.height = DES_POWERFORMHEIGHT;
	sFormInit.pDisplay = intDisplayDesignForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FARFARFORE);
#endif
	/* Set the text colour for the form */
	widgSetColour(psWScreen, IDDES_POWERFORM, WCOL_TEXT, 0, 164, 0);

	/* Add the design template power bar and label*/
	sLabInit.formID	= IDDES_POWERFORM;
	sLabInit.id = IDDES_TEMPPOWERLAB;
	sLabInit.x = DES_POWERX;
	sLabInit.y = DES_POWERY;
	sLabInit.pTip = strresGetString(psStringRes, STR_DES_TEMPPOWER);
	sLabInit.pDisplay = intDisplayImage;
	sLabInit.pUserData = (void*)IMAGE_DES_POWER;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return TRUE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FARFORE);
#endif
	memset(&sBarInit, 0, sizeof(W_BARINIT));
	sBarInit.formID = IDDES_POWERFORM;
	sBarInit.id = IDDES_POWERBAR;
	sBarInit.style = WBAR_PLAIN;
	sBarInit.orientation = WBAR_LEFT;
	sBarInit.x = (SWORD)(DES_POWERX + DES_POWERSEPARATIONX +
					iV_GetImageWidth(IntImages,IMAGE_DES_BODYPOINTS));
	sBarInit.y = DES_POWERY;
	sBarInit.width = (SWORD)(DES_POWERFORMWIDTH - 15 -
					iV_GetImageWidth(IntImages,IMAGE_DES_BODYPOINTS));
	sBarInit.height = iV_GetImageHeight(IntImages,IMAGE_DES_POWERBACK);
	sBarInit.pDisplay = intDisplayDesignPowerBar;//intDisplayStatsBar;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_TEMPPOWER);
	sBarInit.iRange = DBAR_TEMPLATEMAXPOWER;//WBAR_SCALE;
	if (!widgAddBarGraph(psWScreen, &sBarInit))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FARFARFORE);
#endif
	/* Add the design template body points bar and label*/
	sLabInit.formID	= IDDES_POWERFORM;
	sLabInit.id = IDDES_TEMPBODYLAB;
	sLabInit.x = DES_POWERX;
	sLabInit.y = (SWORD)(DES_POWERY + DES_POWERSEPARATIONY +
						iV_GetImageHeight(IntImages,IMAGE_DES_BODYPOINTS));
	sLabInit.pTip = strresGetString(psStringRes, STR_DES_TEMPBODY);
	sLabInit.pDisplay = intDisplayImage;
	sLabInit.pUserData = (void*)IMAGE_DES_BODYPOINTS;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return TRUE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FARFORE);
#endif
	memset(&sBarInit, 0, sizeof(W_BARINIT));
	sBarInit.formID = IDDES_POWERFORM;
	sBarInit.id = IDDES_BODYPOINTS;
	sBarInit.style = WBAR_PLAIN;
	sBarInit.orientation = WBAR_LEFT;
	sBarInit.x = (SWORD)(DES_POWERX + DES_POWERSEPARATIONX +
					iV_GetImageWidth(IntImages,IMAGE_DES_BODYPOINTS));
	sBarInit.y = (SWORD)(DES_POWERY + DES_POWERSEPARATIONY + 4 +
							iV_GetImageHeight(IntImages,IMAGE_DES_BODYPOINTS));
	sBarInit.width = (SWORD)(DES_POWERFORMWIDTH - 15 -
					iV_GetImageWidth(IntImages,IMAGE_DES_BODYPOINTS));
	sBarInit.height = iV_GetImageHeight(IntImages,IMAGE_DES_POWERBACK);
	sBarInit.pDisplay = intDisplayDesignPowerBar;//intDisplayStatsBar;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_TEMPBODY);
	sBarInit.iRange = DBAR_TEMPLATEMAXPOINTS;//(UWORD)getMaxBodyPoints();//DBAR_BODYMAXPOINTS;
	if (!widgAddBarGraph(psWScreen, &sBarInit))
	{
		return FALSE;
	}

	/* Add the variable bits of the design screen and set the bar graphs */
	desCompMode = IDES_NOCOMPONENT;
	desSysMode = IDES_NOSYSTEM;
	desPropMode = IDES_NOPROPULSION;
	intSetDesignStats(&sCurrDesign);
	intSetBodyPoints(&sCurrDesign);
	intSetDesignPower(&sCurrDesign);
	intSetDesignMode(IDES_BODY);

	/* hide design and component forms until required */
	if ( bShowCentreScreen == FALSE )
	{
		widgHide( psWScreen, IDDES_FORM );
	}
	widgHide( psWScreen, IDDES_STATSFORM );
	widgHide( psWScreen, IDDES_RIGHTBASE );

#ifdef PSX
	if(GetControllerType(0) == CON_MOUSE) {
		intRemoveMouseInterface();
	}
#endif

	return TRUE;
}

/* set up droid templates before going into design screen */
void desSetupDesignTemplates( void )
{
	DROID_TEMPLATE	*psTempl;
	UDWORD			i;

	/* init template list */
	memset( apsTemplateList, 0, sizeof(DROID_TEMPLATE*) * MAXTEMPLATES );
	apsTemplateList[0] = &sDefaultDesignTemplate;
	i = 1;
	psTempl = apsDroidTemplates[selectedPlayer];
	while ((psTempl != NULL) && (i < MAXTEMPLATES))
	{
		/* add template to list if not a transporter,
		 * cyborg, person or command droid
		 */
		if ( psTempl->droidType != DROID_TRANSPORTER        &&
			 psTempl->droidType != DROID_CYBORG             &&
			 psTempl->droidType != DROID_CYBORG_SUPER       &&
             psTempl->droidType != DROID_CYBORG_CONSTRUCT   &&
             psTempl->droidType != DROID_CYBORG_REPAIR      &&
//			 psTempl->droidType != DROID_COMMAND     &&
			 psTempl->droidType != DROID_PERSON         )
		{
			apsTemplateList[i] = psTempl;
			i++;
		}

		/* next template */
		psTempl = psTempl->psNext;
	}
}

/* Add the design template form */
static BOOL _intAddTemplateForm(DROID_TEMPLATE *psSelected)
{
	W_FORMINIT	sFormInit;
	UDWORD		numButtons, butPerForm;
	UDWORD		i;


	/* Count the number of minor tabs needed for the template form */
	numButtons = 0;
	for( i=0; i<MAXTEMPLATES; i++ )
	{
		if ( apsTemplateList[i] != NULL )
		{
			numButtons++;
		}
	}

	/* Calculate how many buttons will go on a single form */
	butPerForm = ((DES_LEFTFORMWIDTH - DES_TABTHICKNESS - DES_TABBUTGAP) / 
						(DES_TABBUTWIDTH + DES_TABBUTGAP)) *
				 ((DES_LEFTFORMHEIGHT - DES_TABTHICKNESS - DES_TABBUTGAP) / 
						(DES_TABBUTHEIGHT + DES_TABBUTGAP));

	/* add a form to place the tabbed form on */
#ifdef PSX
	WidgSetOTIndex(OT2D_FARBACK);
#endif
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = 0;
	sFormInit.id = IDDES_TEMPLBASE;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = (SWORD)RET_X;
	sFormInit.y = (SWORD)DESIGN_Y;
	sFormInit.width = RET_FORMWIDTH;
	sFormInit.height = DES_LEFTFORMHEIGHT + 4;
	sFormInit.pDisplay = intDisplayPlainForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	/* Add the design templates form */
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = IDDES_TEMPLBASE;	//IDDES_FORM;
	sFormInit.id = IDDES_TEMPLFORM;
	sFormInit.style = WFORM_TABBED;
	sFormInit.x = 2;//DES_LEFTFORMX;	//DES_TEMPLX;
	sFormInit.y = 2;//DES_LEFTFORMY;		//DES_TEMPLY;
	sFormInit.width = DES_LEFTFORMWIDTH;	//DES_TEMPLWIDTH;
	sFormInit.height = DES_LEFTFORMHEIGHT;	//DES_TEMPLHEIGHT;
	sFormInit.numMajor = numForms(numButtons, butPerForm);
	sFormInit.majorPos = WFORM_TABTOP;
	sFormInit.minorPos = WFORM_TABNONE;
	sFormInit.majorSize = DES_TAB_WIDTH;
	sFormInit.majorOffset = DES_TAB_LEFTOFFSET;
	sFormInit.tabVertOffset = (DES_TAB_HEIGHT/2);			//(DES_TAB_HEIGHT/2)+2;
	sFormInit.tabMajorThickness = DES_TAB_HEIGHT;
	sFormInit.pFormDisplay = intDisplayObjectForm;		//intDisplayTemplateForm;
	sFormInit.pUserData = (void*)&StandardTab;
	sFormInit.pTabDisplay = intDisplayTab;
	for (i=0; i< sFormInit.numMajor; i++)
	{
		sFormInit.aNumMinors[i] = 1;
	}
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

//#ifdef PSX
//	SetCurrentSnapFormID(&InterfaceSnap,sFormInit.id);
//#endif
	/* Put the buttons on it */
	if (!intAddTemplateButtons(IDDES_TEMPLFORM, DES_LEFTFORMWIDTH - DES_TABTHICKNESS,
							   DES_LEFTFORMHEIGHT - DES_TABTHICKNESS,
							   DES_TABBUTWIDTH, DES_TABBUTHEIGHT, DES_TABBUTGAP,
							   psSelected ))
	{
		return FALSE;
	}

	return TRUE;
}


	
/* Add the droid template buttons to a form */
BOOL intAddTemplateButtons(UDWORD formID, UDWORD formWidth, UDWORD formHeight,
								  UDWORD butWidth, UDWORD butHeight, UDWORD gap,
								  DROID_TEMPLATE *psSelected)
{
	W_FORMINIT		sButInit;
	W_BARINIT		sBarInit;
	DROID_TEMPLATE	*psTempl = NULL;
	STRING			aButText[DES_COMPBUTMAXCHAR + 1];
	SDWORD			BufferID;
	UDWORD			i;
#ifdef WIN32
	char TempString[256];
	int BufferPos = 0;
#endif

	ClearStatBuffers();

	memset(aButText, 0, DES_COMPBUTMAXCHAR + 1);
	memset(&sButInit, 0, sizeof(W_BUTINIT));

	/* Set up the button struct */
	sButInit.formID = formID;
	sButInit.id = IDDES_TEMPLSTART;
	sButInit.style = WFORM_CLICKABLE;
	sButInit.x = DES_LEFTFORMBUTX;
	sButInit.y = DES_LEFTFORMBUTY;
	sButInit.width = OBJ_BUTWIDTH;			//DES_TABBUTWIDTH;
	sButInit.height = OBJ_BUTHEIGHT;		//DES_TABBUTHEIGHT;

	/* Add each button */
	memset(&sBarInit, 0, sizeof(W_BARINIT));
	sBarInit.id = IDDES_BARSTART;
	sBarInit.style = WBAR_PLAIN;
	sBarInit.orientation = WBAR_LEFT;
	sBarInit.x = STAT_TIMEBARX;
	sBarInit.y = STAT_TIMEBARY;
	sBarInit.width = STAT_PROGBARWIDTH;
	sBarInit.height = STAT_PROGBARHEIGHT;
	sBarInit.size = 50;
	sBarInit.sCol.red = STAT_PROGBARMAJORRED;
	sBarInit.sCol.green = STAT_PROGBARMAJORGREEN;
	sBarInit.sCol.blue = STAT_PROGBARMAJORBLUE;
	sBarInit.sMinorCol.red = STAT_PROGBARMINORRED;
	sBarInit.sMinorCol.green = STAT_PROGBARMINORGREEN;
	sBarInit.sMinorCol.blue = STAT_PROGBARMINORBLUE;
	sBarInit.pTip = strresGetString(psStringRes, STR_DES_POWERUSE);

	droidTemplID = 0;
	for( i=0; i<MAXTEMPLATES; i++ )
	{
		if ( apsTemplateList[i] != NULL )
		{
			psTempl = apsTemplateList[i];

			/* Set the tip and add the button */


			// On the playstation the tips are additionaly setup when they are displayed ... because we only have one text name buffer
#ifdef HASH_NAMES
			strncpy(aButText, "TempTip", DES_COMPBUTMAXCHAR);
#else
			strncpy(aButText, getTemplateName( psTempl ), DES_COMPBUTMAXCHAR);
#endif
			sButInit.pTip = getTemplateName(psTempl);

			BufferID = GetStatBuffer();
			ASSERT((BufferID >= 0,"Unable to aquire stat buffer."));
			RENDERBUTTON_INUSE(&StatBuffers[BufferID]);
			StatBuffers[BufferID].Data = (void*)psTempl;
			sButInit.pUserData = (void*)&StatBuffers[BufferID];
			sButInit.pDisplay = intDisplayTemplateButton;

#ifdef PSX
			WidgSetOTIndex(OT2D_BACK);
#endif
			if (!widgAddForm(psWScreen, &sButInit))
			{
				return FALSE;
			}

			sBarInit.iRange = POWERPOINTS_DROIDDIV;
			sBarInit.size = (UWORD)(psTempl->powerPoints  / POWERPOINTS_DROIDDIV);
			if(sBarInit.size > WBAR_SCALE) sBarInit.size = WBAR_SCALE;

#ifdef WIN32
			sprintf(TempString,"%s - %d",strresGetString(psStringRes, STR_DES_POWERUSE),psTempl->powerPoints);
			ASSERT((BufferPos+strlen(TempString)+1 < STRING_BUFFER_SIZE,"String Buffer Overrun"));
			strcpy(&StringBuffer[BufferPos],TempString);
			sBarInit.pTip = &StringBuffer[BufferPos];
			BufferPos += strlen(TempString)+1;
#endif

			sBarInit.formID = sButInit.id;
#ifdef PSX
			WidgSetOTIndex(OT2D_FORE);
#endif
			if (!widgAddBarGraph(psWScreen, &sBarInit))
			{
				return FALSE;
			}

			/* if the current template matches psSelected lock the button */
			if (psTempl == psSelected)
			{
				droidTemplID = sButInit.id;
				widgSetButtonState(psWScreen, droidTemplID, WBUT_LOCK);
				widgSetTabs(psWScreen, IDDES_TEMPLFORM, sButInit.majorID, 0);
			}

#ifdef PSX
			// Snap to the new droid template.
			if(i == 0) {
				SetCurrentSnapID(&InterfaceSnap,sButInit.id);
			}
#endif

			/* Update the init struct for the next button */
			sBarInit.id += 1;
			sButInit.id += 1;
			sButInit.x = (SWORD)(sButInit.x + butWidth + gap);
			if (sButInit.x + butWidth + gap > formWidth)
			{
				sButInit.x = DES_LEFTFORMBUTX;
				sButInit.y = (SWORD)(sButInit.y + butHeight + gap);
			}
			if (sButInit.y + butHeight + gap > formHeight)
			{
				sButInit.y = DES_LEFTFORMBUTY;
				sButInit.majorID += 1;
			}
			//check don't go over max templates that can fit on the form
			if (sButInit.id >= IDDES_TEMPLEND)
			{
				break;
			}
		}
	}

	return TRUE;
}


/*static UDWORD MaxComponents(UDWORD NumStats)
{
	if(NumStats > MAX_DESIGN_COMPONENTS) {
		return MAX_DESIGN_COMPONENTS;
	}

	return NumStats;
}*/



/* Set the current mode of the design screen, and display the appropriate
 * component lists
 */
static void intSetDesignMode(DES_COMPMODE newCompMode)
{
	UDWORD	weaponIndex;
//	UBYTE	aCmdAvailable[MAX_CMDDROIDS];
//	SDWORD	i;
	//UDWORD	NumComponents;
	//UDWORD	NumSensors;
	//UDWORD	NumECMs;
	//UDWORD	NumConstructs;
	//UDWORD	NumWeapons;

	if (newCompMode != desCompMode)
	{
		/* Have to change the component display - remove the old one */
		switch (desCompMode)
		{
		case IDES_NOCOMPONENT:
			/* Nothing displayed so nothing to remove */
			break;
		case IDES_BRAIN:
/*
			widgDelete(psWScreen, IDDES_COMPFORM);
			widgDelete(psWScreen, IDDES_RIGHTBASE);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, 0);
*/
		case IDES_SYSTEM:
			widgDelete(psWScreen, IDDES_COMPFORM);
			widgDelete(psWScreen, IDDES_RIGHTBASE);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, 0);
			break;
		case IDES_TURRET:
			widgDelete(psWScreen, IDDES_COMPFORM);
			widgDelete(psWScreen, IDDES_RIGHTBASE);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, 0);
			break;
		case IDES_BODY:
			widgDelete(psWScreen, IDDES_COMPFORM);
			widgDelete(psWScreen, IDDES_RIGHTBASE);
			widgSetButtonState(psWScreen, IDDES_BODYFORM, 0);
			break;
		case IDES_PROPULSION:
			widgDelete(psWScreen, IDDES_COMPFORM);
			widgDelete(psWScreen, IDDES_RIGHTBASE);
			widgSetButtonState(psWScreen, IDDES_PROPFORM, 0);
			break;
		}

		/* Set up the display for the new mode */
		desCompMode = newCompMode;
		switch (desCompMode)
		{
		case IDES_NOCOMPONENT:
			/* Nothing to display */
			break;
		case IDES_BRAIN:
			// initialise the available array
/*			aCmdAvailable[0] = 0;
			for(i=1; i<MAX_CMDDROIDS; i++)
			{
				if (i<=numCommandDroids[selectedPlayer])
				{
					aCmdAvailable[i] = AVAILABLE;
				}
				else
				{
					aCmdAvailable[i] = 0;
				}
			}
			intAddComponentForm(numCommandDroids[selectedPlayer]);
			intAddComponentButtons((COMP_BASE_STATS *)asCommandDroids[selectedPlayer],
								   sizeof(COMMAND_DROID),
								   aCmdAvailable,MAX_CMDDROIDS,
								   sCurrDesign.asParts[COMP_BRAIN],TAB_USEMAJOR);
			intAddSystemButtons(IDES_BRAIN);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, WBUT_LOCK);*/
		case IDES_SYSTEM:
			intAddComponentForm(
				intNumAvailable(apCompLists[selectedPlayer][COMP_SENSOR], numSensorStats,
								(COMP_BASE_STATS *)asSensorStats, sizeof(SENSOR_STATS)) +
				intNumAvailable(apCompLists[selectedPlayer][COMP_ECM], numECMStats,
								(COMP_BASE_STATS *)asECMStats, sizeof(ECM_STATS)) +
				intNumAvailable(apCompLists[selectedPlayer][COMP_BRAIN], numBrainStats,
								(COMP_BASE_STATS *)asBrainStats, sizeof(BRAIN_STATS)) +
				intNumAvailable(apCompLists[selectedPlayer][COMP_CONSTRUCT], numConstructStats,
								(COMP_BASE_STATS *)asConstructStats, sizeof(CONSTRUCT_STATS)) +
				intNumAvailable(apCompLists[selectedPlayer][COMP_REPAIRUNIT], numRepairStats,
								(COMP_BASE_STATS *)asRepairStats, sizeof(REPAIR_STATS)));
			intAddExtraSystemButtons(sCurrDesign.asParts[COMP_SENSOR],
									 sCurrDesign.asParts[COMP_ECM],
									 sCurrDesign.asParts[COMP_CONSTRUCT],
									 sCurrDesign.asParts[COMP_REPAIRUNIT],
									 sCurrDesign.asParts[COMP_BRAIN]);
			intAddSystemButtons(IDES_SYSTEM);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, WBUT_LOCK);
			break;
		case IDES_TURRET:
			intAddComponentForm(
				intNumAvailable(apCompLists[selectedPlayer][COMP_WEAPON], numWeaponStats,
								(COMP_BASE_STATS *)asWeaponStats, sizeof(WEAPON_STATS)));
			weaponIndex = (sCurrDesign.numWeaps > 0) ? sCurrDesign.asWeaps[0] : 0;
			intAddComponentButtons((COMP_BASE_STATS *)asWeaponStats,
								   sizeof(WEAPON_STATS),
								   apCompLists[selectedPlayer][COMP_WEAPON],
								   //NumWeapons, weaponIndex,TAB_USEMINOR);
								   numWeaponStats, weaponIndex,TAB_USEMAJOR);
			intAddSystemButtons(IDES_TURRET);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, WBUT_LOCK);
			break;
			//NumSensors = MaxComponents(numSensorStats);
			//NumECMs = MaxComponents(numECMStats);
			//NumConstructs = MaxComponents(numConstructStats);
			//NumWeapons = MaxComponents(numWeaponStats);
/*			intAddMultiComponentForm(apCompLists[selectedPlayer][COMP_SENSOR],
									 numSensorStats,//NumSensors,
									 apCompLists[selectedPlayer][COMP_ECM],
									 numECMStats,//NumECMs,
									 apCompLists[selectedPlayer][COMP_CONSTRUCT],
									 numConstructStats,//NumConstructs,
									 apCompLists[selectedPlayer][COMP_WEAPON],
									 numWeaponStats);//NumWeapons);
			if (sCurrDesign.numWeaps > 0)
			{
				weaponIndex = sCurrDesign.asWeaps[0];
			}
			else
			{
				weaponIndex = 0;
			}
			intAddComponentButtons((COMP_BASE_STATS *)asWeaponStats,
								   sizeof(WEAPON_STATS),
								   apCompLists[selectedPlayer][COMP_WEAPON],
								   //NumWeapons, weaponIndex,TAB_USEMINOR);
								   numWeaponStats, weaponIndex,TAB_USEMINOR);
			intAddExtraSystemButtons(sCurrDesign.asParts[COMP_SENSOR],
									 sCurrDesign.asParts[COMP_ECM],
									 sCurrDesign.asParts[COMP_CONSTRUCT],
									 sCurrDesign.asParts[COMP_REPAIRUNIT]);
			widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, WBUT_LOCK);
			break;*/
		case IDES_BODY:
			/*NumComponents = numBodyStats;
			if(NumComponents > MAX_DESIGN_COMPONENTS) {
				NumComponents = MAX_DESIGN_COMPONENTS;
			}*/
			intAddComponentForm(
				intNumAvailable(apCompLists[selectedPlayer][COMP_BODY], numBodyStats,
								(COMP_BASE_STATS *)asBodyStats, sizeof(BODY_STATS)));
			intAddComponentButtons((COMP_BASE_STATS *)asBodyStats,
								   sizeof(BODY_STATS),
								   apCompLists[selectedPlayer][COMP_BODY],
								   //NumComponents, sCurrDesign.asParts[COMP_BODY],TAB_USEMAJOR);
								   numBodyStats, sCurrDesign.asParts[COMP_BODY],TAB_USEMAJOR);
			widgSetButtonState(psWScreen, IDDES_BODYFORM, WBUT_LOCK);
			break;
		case IDES_PROPULSION:
			/*NumComponents = numPropulsionStats;
			if(NumComponents > MAX_DESIGN_COMPONENTS) {
				NumComponents = MAX_DESIGN_COMPONENTS;
			}*/
			intAddComponentForm(
				intNumAvailable(apCompLists[selectedPlayer][COMP_PROPULSION], numPropulsionStats,
								(COMP_BASE_STATS *)asPropulsionStats, sizeof(PROPULSION_STATS)));
			intAddComponentButtons((COMP_BASE_STATS *)asPropulsionStats,
								   sizeof(PROPULSION_STATS),
								   apCompLists[selectedPlayer][COMP_PROPULSION],
								   //NumComponents, sCurrDesign.asParts[COMP_PROPULSION],TAB_USEMAJOR);
								   numPropulsionStats, sCurrDesign.asParts[COMP_PROPULSION],
								   TAB_USEMAJOR);
			widgSetButtonState(psWScreen, IDDES_PROPFORM, WBUT_LOCK);
			break;
		}
	}
}

static COMP_BASE_STATS *
intChooseSystemStats( DROID_TEMPLATE *psTemplate )
{
	COMP_BASE_STATS		*psStats = NULL;


	// Check for a command droid
/*	if (psTemplate->asParts[COMP_BRAIN] != 0)
	{
		return (COMP_BASE_STATS *)(asCommandDroids[selectedPlayer] +
								psTemplate->asParts[COMP_BRAIN]);
	}*/

	// Choose correct system stats
	switch (droidTemplateType(psTemplate))
	{
	case DROID_COMMAND:
//		psStats = (COMP_BASE_STATS *)(asCommandDroids[selectedPlayer] +
//								psTemplate->asParts[COMP_BRAIN]);
		psStats = (COMP_BASE_STATS *)(asBrainStats +
								psTemplate->asParts[COMP_BRAIN]);
		break;
	case DROID_SENSOR:
		psStats = (COMP_BASE_STATS *)(asSensorStats +
								psTemplate->asParts[COMP_SENSOR]);
		break;
	case DROID_ECM:
		psStats = (COMP_BASE_STATS *)(asECMStats +
								psTemplate->asParts[COMP_ECM]);
		break;
	case DROID_CONSTRUCT:
	case DROID_CYBORG_CONSTRUCT:
		psStats = (COMP_BASE_STATS *)(asConstructStats +
								psTemplate->asParts[COMP_CONSTRUCT]);
		break;
	case DROID_REPAIR:
	case DROID_CYBORG_REPAIR:
		psStats = (COMP_BASE_STATS *)(asRepairStats +
								psTemplate->asParts[COMP_REPAIRUNIT]);
		break;
	case DROID_WEAPON:
	case DROID_PERSON:
	case DROID_CYBORG:
	case DROID_CYBORG_SUPER:
	case DROID_DEFAULT:
		psStats = (COMP_BASE_STATS *)(asWeaponStats +
								psTemplate->asWeaps[0]);
		break;
	default:
		DBERROR( ("intSetDesignStats: unrecognised droid type") );
		return NULL;
	}

	return psStats;
}

/* set SHOWTEMPLATENAME to 0 to show template components in edit box */
#define SHOWTEMPLATENAME	0


/*
	Go through the template for player 0 matching up all the 
	components from SourceTemplate

		
	NULL is returned if no match is found

	= Matches Body,Proulsion & weapon

	- This is used for generating cyborg names 
*/
DROID_TEMPLATE *MatchTemplate(DROID_TEMPLATE *SourceTemplate,UDWORD player)
{
	DROID_TEMPLATE *pDroidDesign;		  

	for(pDroidDesign = apsDroidTemplates[player]; pDroidDesign != NULL; 
		pDroidDesign = pDroidDesign->psNext)
	{
		
		if (pDroidDesign->asParts[COMP_BODY]==SourceTemplate->asParts[COMP_BODY])
		{

			if (pDroidDesign->asParts[COMP_PROPULSION]==SourceTemplate->asParts[COMP_PROPULSION])
			{

				if (pDroidDesign->numWeaps)
				{
					if (pDroidDesign->asWeaps[0]==SourceTemplate->asWeaps[0])
					{
						return (pDroidDesign);
					}
				}
			}
		}
	}
	return NULL;

}



STRING *GetDefaultTemplateName(DROID_TEMPLATE *psTemplate)
{
	COMP_BASE_STATS		*psStats;
	STRING				*pStr;
	/*
		First we check for the special cases of the Transporter & Cyborgs
	*/
	if(psTemplate->droidType == DROID_TRANSPORTER) 
	{
		return strresGetString(NULL,HashString("Transporter"));
	}
#ifdef PSX


	if( psTemplate->droidType == DROID_CYBORG OR
		psTemplate->droidType == DROID_CYBORG_SUPER OR
        psTemplate->droidType == DROID_CYBORG_CONSTRUCT OR
        psTemplate->droidType == DROID_CYBORG_REPAIR) 
	{
		DROID_TEMPLATE *pStatTemplate;

		// If the namehash is null, then we've probably just made up this template
		if (psTemplate->NameHash!=NULL)
		{
			pStatTemplate=psTemplate;			
		}
		else
		{
			// Match this template with one from the stats
			pStatTemplate=MatchTemplate(psTemplate,0);		
		}

		if (pStatTemplate==NULL)
		{
			return ("Cyborg");
		}		
		else
		{
			return strresGetString(NULL,pStatTemplate->NameHash);
		}


	}
#endif
	/*
		Now get the normal default droid name based on its components

	*/
	aCurrName[0]=0;		// Reset string to null
	psStats = intChooseSystemStats( psTemplate );

	if ( psTemplate->asWeaps[0]					!= 0 ||
		 psTemplate->asParts[COMP_CONSTRUCT]	!= 0 ||
		 psTemplate->asParts[COMP_SENSOR]		!= 0 ||
		 psTemplate->asParts[COMP_ECM]			!= 0 ||
		 psTemplate->asParts[COMP_REPAIRUNIT]   != 0 ||
		 psTemplate->asParts[COMP_BRAIN]		!= 0    )
	{
		pStr = getStatName( psStats );
		strcpy( aCurrName, pStr );
		strcat( aCurrName, " " );
//		DBPRINTF(("%s",aCurrName));
//	DBPRINTF(("a) templ=%p stat=%p name=%s\n",psTemplate,psStats,pStr);
	}

	psStats = (COMP_BASE_STATS *) (asBodyStats + psTemplate->asParts[COMP_BODY]);
	if ( psTemplate->asParts[COMP_BODY] != 0 )
	{
		pStr = getStatName( psStats );

		if ( strlen( aCurrName ) + strlen( pStr ) > WIDG_MAXSTR )
		{
			DBPRINTF( ("GetDefaultTemplateName: name string too long %s+%s\n",aCurrName,pStr) );
			return NULL;
		}

		strcat( aCurrName, pStr );
		strcat( aCurrName, " " );
//		DBPRINTF(("b) templ=%p stat=%p name=%s\n",psTemplate,psStats,pStr);
	}

	psStats = (COMP_BASE_STATS *) (asPropulsionStats + psTemplate->asParts[COMP_PROPULSION]);
	if ( psTemplate->asParts[COMP_PROPULSION] != 0 )
	{
		pStr = getStatName( psStats );

		if ( strlen( aCurrName ) + strlen( pStr ) > WIDG_MAXSTR )
		{
			DBPRINTF( ("GetDefaultTemplateName: name string too long %s+%s\n",aCurrName,pStr) );
			return NULL;
		}

		strcat( aCurrName, pStr );
//		DBPRINTF(("c) templ=%p stat=%p name=%s\n",psTemplate,psStats,pStr);
	}

//	DBPRINTF(("TEMPLATE NAME [%s]\n",aCurrName);

	return aCurrName;
}




static void intSetEditBoxTextFromTemplate( DROID_TEMPLATE *psTemplate )
{
#if SHOWTEMPLATENAME
#ifdef WIN32
	widgSetString(psWScreen, IDDES_NAMEBOX, getStatName(psTemplate));
#endif
#else

	strcpy( aCurrName, "" );

	/* show component names if default template else show stat name */
	if ( psTemplate->droidType != DROID_DEFAULT )
	{
		strcpy( aCurrName, getTemplateName(psTemplate) );
	}
	else
	{
		GetDefaultTemplateName(psTemplate);
	}
	
#ifdef WIN32
	widgSetString(psWScreen, IDDES_NAMEBOX, aCurrName);
#endif
#endif
}

/* Set all the design bar graphs from a design template */
static void intSetDesignStats( DROID_TEMPLATE *psTemplate )
{
	COMP_BASE_STATS		*psStats = intChooseSystemStats( psTemplate );

	/* Set system stats */
	intSetSystemForm( psStats );

	/* Set the body stats */
	intSetBodyStats(asBodyStats + psTemplate->asParts[COMP_BODY]);

	/* Set the propulsion stats */
	intSetPropulsionForm(asPropulsionStats + psTemplate->asParts[COMP_PROPULSION]);

	/* Set the name in the edit box */
	intSetEditBoxTextFromTemplate( psTemplate );
}

/* Set up the system clickable form of the design screen given a set of stats */
static BOOL _intSetSystemForm(COMP_BASE_STATS *psStats)
{
	SENSOR_STATS	*psSensor;
	ECM_STATS		*psECM;
	CONSTRUCT_STATS	*psConst;
	WEAPON_STATS	*psWeapon;
	REPAIR_STATS	*psRepair;
	W_FORMINIT		sFormInit;
	W_BARINIT		sBarInit;
	W_LABINIT		sLabInit;
//	W_LABINIT		sTitleInit;
	DES_SYSMODE		newSysMode=0;

	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	memset(&sLabInit, 0, sizeof(W_LABINIT));
//	memset(&sTitleInit, 0, sizeof(W_LABINIT));
	memset(&sBarInit, 0, sizeof(W_BARINIT));

	/* Figure out what the new mode should be */
	switch (statType(psStats->ref))
	{
	case COMP_WEAPON:
		newSysMode = IDES_WEAPON;
		break;
	case COMP_SENSOR:
		newSysMode = IDES_SENSOR;
		break;
	case COMP_ECM:
		newSysMode = IDES_ECM;
		break;
	case COMP_CONSTRUCT:
		newSysMode = IDES_CONSTRUCT;
		break;
	case COMP_BRAIN:
		newSysMode = IDES_COMMAND;
		break;
	case COMP_REPAIRUNIT:
		newSysMode = IDES_REPAIR;
		break;
	}

	/* If the correct form is already displayed just set the stats */
	if (newSysMode == desSysMode)
	{
		intSetSystemStats(psStats);
	
		return TRUE;
	}

	/* Remove the old form if necessary */
	if (desSysMode != IDES_NOSYSTEM)
	{
		widgDelete(psWScreen, IDDES_SYSTEMFORM);
	}

	/* Set the new mode */
	desSysMode = newSysMode;

	/* Add the system form */
#ifdef PSX
	WidgSetOTIndex(OT2D_FARBACK);
#endif
	sFormInit.formID = IDDES_STATSFORM;
	sFormInit.id = IDDES_SYSTEMFORM;
	sFormInit.style = (WFORM_CLICKABLE | WFORM_NOCLICKMOVE);
	sFormInit.x = DES_BARFORMX;
	sFormInit.y = DES_BARFORMY;
	sFormInit.width = DES_BARFORMWIDTH;	//COMPBUTWIDTH;
	sFormInit.height = DES_BARFORMHEIGHT;	//COMPBUTHEIGHT;
	sFormInit.pTip = getStatName(psStats);	/* set form tip to stats string */
	sFormInit.pUserData = psStats;			/* store component stats */
	sFormInit.pDisplay = intDisplayStatForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	/* Initialise the bargraph struct */
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	sBarInit.formID = IDDES_SYSTEMFORM;
	sBarInit.style = WBAR_PLAIN;//WBAR_DOUBLE;
	sBarInit.orientation = WBAR_LEFT;
	sBarInit.x = DES_CLICKBARX;
	sBarInit.y = DES_STATBAR_Y1;	//DES_CLICKBARY;
	sBarInit.width = DES_CLICKBARWIDTH;
	sBarInit.height = DES_CLICKBARHEIGHT;
	sBarInit.sCol.red = DES_CLICKBARMAJORRED;
	sBarInit.sCol.green = DES_CLICKBARMAJORGREEN;
	sBarInit.sCol.blue = DES_CLICKBARMAJORBLUE;
	sBarInit.sMinorCol.red = DES_CLICKBARMINORRED;
	sBarInit.sMinorCol.green = DES_CLICKBARMINORGREEN;
	sBarInit.sMinorCol.blue = DES_CLICKBARMINORBLUE;
	sBarInit.pDisplay = intDisplayStatsBar;

	/* Initialise the label struct */
	sLabInit.formID = IDDES_SYSTEMFORM;
	sLabInit.style = WLAB_PLAIN;
	sLabInit.x = DES_CLICKBARNAMEX;
	sLabInit.y = DES_CLICKBARY - DES_CLICKBARHEIGHT/3;
	sLabInit.width = DES_CLICKBARNAMEWIDTH;
	sLabInit.height = DES_CLICKBARHEIGHT;
	sLabInit.FontID = WFont;

//	/* Initialise the title label struct */
//	sTitleInit.formID = IDDES_SYSTEMFORM;
//	sTitleInit.style = WLAB_ALIGNCENTRE;
//	sTitleInit.x = DES_CLICKGAP;
//	sTitleInit.y = DES_CLICKGAP;
//	sTitleInit.width = DES_COMPBUTWIDTH - DES_CLICKGAP*2;
//	sTitleInit.height = DES_CLICKLABELHEIGHT;
//	sTitleInit.psFont = psWFont;

	/* See what type of system stats we've got */
	if (psStats->ref >= REF_SENSOR_START &&
		psStats->ref < REF_SENSOR_START + REF_RANGE)
	{
		ASSERT((PTRVALID(psStats, sizeof(SENSOR_STATS)),
			"intAddSystemForm: Invalid sensor stats pointer"));
		psSensor = (SENSOR_STATS *)psStats;

		/* Add the bar graphs*/ 
		sBarInit.id = IDDES_SENSORRANGE;
		sBarInit.iRange = (UWORD)getMaxSensorRange();//DBAR_SENSORMAXRANGE;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_SENSORPOWER;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
//printf("%d\n",DES_STATBAR_Y2);	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = (UWORD)getMaxSensorPower();//DBAR_SENSORMAXPOWER;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_SENSORWEIGHT;
		sBarInit.y = DES_STATBAR_Y3;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_SENSORRANGELAB;
//		sLabInit.pText = "Sensor Range";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_SENSOR_RANGE);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_RANGE;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_SENSORPOWERLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//		sLabInit.pText = " Sensor Power";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_SENSOR_POWER);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_POWER;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_SENSORWEIGHTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//		sLabInit.pText = "Weight";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}

//		/* Add the title label */
//		sTitleInit.id = IDDES_SYSTEMLAB;
//		sTitleInit.pText = "Sensor";
//		if (!widgAddLabel(psWScreen, &sTitleInit))
//		{
//			return FALSE;
//		}
	}
	else if (psStats->ref >= REF_ECM_START &&
			 psStats->ref < REF_ECM_START + REF_RANGE)
	{
		ASSERT((PTRVALID(psStats, sizeof(ECM_STATS)),
			"intAddSystemForm: Invalid ecm stats pointer"));
		psECM = (ECM_STATS *)psStats;

		/* Add the bar graphs */
		sBarInit.id = IDDES_ECMPOWER;
		sBarInit.iRange = (UWORD)getMaxECMPower();//DBAR_ECMMAXPOWER;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_ECMWEIGHT;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_ECMPOWERLAB;
//		sLabInit.pText = "ECM Power";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_ECM_POWER);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_POWER;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_ECMWEIGHTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//		sLabInit.pText = "Weight";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}

//		/* Add the title label */
//		sTitleInit.id = IDDES_SYSTEMLAB;
//		sTitleInit.pText = "ECM";
//		if (!widgAddLabel(psWScreen, &sTitleInit))
//		{
//			return FALSE;
//		}
	}
	else if (psStats->ref >= REF_CONSTRUCT_START &&
			 psStats->ref < REF_CONSTRUCT_START + REF_RANGE)
	{
		ASSERT((PTRVALID(psStats, sizeof(CONSTRUCT_STATS)),
			"intAddSystemForm: Invalid constructor stats pointer"));
		psConst = (CONSTRUCT_STATS *)psStats;

		/* Add the bar graphs */
		sBarInit.id = IDDES_CONSTPOINTS;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_BUILD_POINTS);
		sBarInit.iRange = (UWORD)getMaxConstPoints();//DBAR_CONSTMAXPOINTS;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_CONSTWEIGHT;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_CONSTPOINTSLAB;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_BUILD_POINTS);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_BUILDRATE;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_CONSTWEIGHTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}

//		/* Add the title label */
//		sTitleInit.id = IDDES_SYSTEMLAB;
//		sTitleInit.pText = "Constructor";
//		if (!widgAddLabel(psWScreen, &sTitleInit))
//		{
//			return FALSE;
//		}
	}
	else if (psStats->ref >= REF_REPAIR_START &&
			 psStats->ref < REF_REPAIR_START + REF_RANGE)
	{
		ASSERT((PTRVALID(psStats, sizeof(REPAIR_STATS)),
			"intAddSystemForm: Invalid repair stats pointer"));
		psRepair = (REPAIR_STATS *)psStats;

		/* Add the bar graphs */
		sBarInit.id = IDDES_REPAIRPOINTS;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_BUILD_POINTS);
		sBarInit.iRange = (UWORD)getMaxRepairPoints();//DBAR_REPAIRMAXPOINTS;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_REPAIRWEIGHT;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_REPAIRPTLAB;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_BUILD_POINTS);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_BUILDRATE;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_REPAIRWGTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
	}
	else if (psStats->ref >= REF_WEAPON_START &&
			 psStats->ref < REF_WEAPON_START + REF_RANGE)
	{
		ASSERT((PTRVALID(psStats, sizeof(WEAPON_STATS)),
			"intAddSystemForm: Invalid ecm stats pointer"));
		psWeapon = (WEAPON_STATS *)psStats;

		/* Add the bar graphs */
		sBarInit.id = IDDES_WEAPRANGE;
		sBarInit.iRange = (UWORD)getMaxWeaponRange();//DBAR_WEAPMAXRANGE;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_RANGE);
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_WEAPDAMAGE;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = (UWORD)getMaxWeaponDamage();//DBAR_WEAPMAXDAMAGE;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_DAMAGE);
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_WEAPROF;
		sBarInit.y = DES_STATBAR_Y3;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = DBAR_WEAPMAXROF;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_ROF);
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_WEAPWEIGHT;
		sBarInit.y = DES_STATBAR_Y4;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_WEAPRANGELAB;
//		sLabInit.pText = "Range";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_RANGE);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_RANGE;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_WEAPDAMAGELAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//		sLabInit.pText = "Dam";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_DAMAGE);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_DAMAGE;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_WEAPROFLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//		sLabInit.pText = "ROF";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_ROF);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_FIRERATE;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_WEAPWEIGHTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
//		sLabInit.pText = "Weight";
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}

//		/* Add the title label */
//		sTitleInit.id = IDDES_SYSTEMLAB;
//		sTitleInit.pText = "Weapon";
//		if (!widgAddLabel(psWScreen, &sTitleInit))
//		{
//			return FALSE;
//		}
	}

	// Add the correct component form
	switch (desSysMode)
	{
//	case IDES_COMMAND:
//		intSetDesignMode(IDES_BRAIN);
		break;
	case IDES_SENSOR:
	case IDES_CONSTRUCT:
	case IDES_ECM:
	case IDES_REPAIR:
	case IDES_COMMAND:
		intSetDesignMode(IDES_SYSTEM);
		break;
	case IDES_WEAPON:
		intSetDesignMode(IDES_TURRET);
		break;
	}

	/* Set the stats */
	intSetSystemStats(psStats);

	/* Lock the form down if necessary */
	if (desCompMode == IDES_SYSTEM)
	{
		widgSetButtonState(psWScreen, IDDES_SYSTEMFORM, WBUT_LOCK);
	}

	return TRUE;
}


/* Set up the propulsion clickable form of the design screen given a set of stats */
static BOOL intSetPropulsionForm(PROPULSION_STATS *psStats)
{
	W_FORMINIT		sFormInit;
	W_BARINIT		sBarInit;
	W_LABINIT		sLabInit;
//	W_LABINIT		sTitleInit;
	DES_PROPMODE	newPropMode=0;

	ASSERT((PTRVALID(psStats, sizeof(PROPULSION_STATS)),
		"intAddPropulsionForm: Invalid propulsion stats pointer"));

	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	memset(&sLabInit, 0, sizeof(W_LABINIT));
//	memset(&sTitleInit, 0, sizeof(W_LABINIT));
	memset(&sBarInit, 0, sizeof(W_BARINIT));

	/* figure out what the new mode should be */
	switch (asPropulsionTypes[psStats->propulsionType].travel)
	{
	case GROUND:
		newPropMode = IDES_GROUND;
		break;
	case AIR:
		newPropMode = IDES_AIR;
		break;
	}

	/* If the mode hasn't changed, just set the stats */
	if (desPropMode == newPropMode)
	{
		intSetPropulsionStats(psStats);
		return TRUE;
	}

	/* Remove the old form if necessary */
	if (desPropMode != IDES_NOPROPULSION)
	{
		widgDelete(psWScreen, IDDES_PROPFORM);
	}

	/* Set the new mode */
	desPropMode = newPropMode;

	/* Add the propulsion form */
#ifdef PSX
	WidgSetOTIndex(OT2D_FARBACK);
#endif
	sFormInit.formID = IDDES_STATSFORM;
	sFormInit.id = IDDES_PROPFORM;
	sFormInit.style = WFORM_CLICKABLE | WFORM_NOCLICKMOVE;
	sFormInit.x = DES_BARFORMX;
	sFormInit.y = DES_BARFORMY;
	sFormInit.width = DES_BARFORMWIDTH;	//DES_COMPBUTWIDTH;
	sFormInit.height = DES_BARFORMHEIGHT;	//DES_COMPBUTHEIGHT;
	sFormInit.pTip = getStatName(psStats);	/* set form tip to stats string */
	sFormInit.pDisplay = intDisplayStatForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	/* Initialise the bargraph struct */
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	sBarInit.formID = IDDES_PROPFORM;
	sBarInit.style = WBAR_PLAIN;//WBAR_DOUBLE;
	sBarInit.orientation = WBAR_LEFT;
	sBarInit.x = DES_CLICKBARX;
	sBarInit.y = DES_STATBAR_Y1;	//DES_CLICKBARY;
	sBarInit.width = DES_CLICKBARWIDTH;
	sBarInit.height = DES_CLICKBARHEIGHT;
	sBarInit.sCol.red = DES_CLICKBARMAJORRED;
	sBarInit.sCol.green = DES_CLICKBARMAJORGREEN;
	sBarInit.sCol.blue = DES_CLICKBARMAJORBLUE;
	sBarInit.sMinorCol.red = DES_CLICKBARMINORRED;
	sBarInit.sMinorCol.green = DES_CLICKBARMINORGREEN;
	sBarInit.sMinorCol.blue = DES_CLICKBARMINORBLUE;
	sBarInit.pDisplay = intDisplayStatsBar;

	/* Initialise the label struct */
	sLabInit.formID = IDDES_PROPFORM;
	sLabInit.style = WLAB_PLAIN;
	sLabInit.x = DES_CLICKBARNAMEX;
	sLabInit.y = DES_CLICKBARY - DES_CLICKBARHEIGHT/3;
	sLabInit.width = DES_CLICKBARNAMEWIDTH;
	sLabInit.height = DES_CLICKBARNAMEHEIGHT;	//DES_CLICKBARHEIGHT;
	sLabInit.FontID = WFont;

//	/* Initialise the title label struct */
//	sTitleInit.formID = IDDES_PROPFORM;
//	sTitleInit.style = WLAB_ALIGNCENTRE;
//	sTitleInit.x = DES_CLICKGAP;
//	sTitleInit.y = DES_CLICKGAP;
//	sTitleInit.width = DES_COMPBUTWIDTH - DES_CLICKGAP*2;
//	sTitleInit.height = DES_CLICKLABELHEIGHT;
//	sTitleInit.psFont = psWFont;

	/* See what type of propulsion we've got */
	switch (desPropMode)
	{
	case IDES_AIR:
		/* Add the bar graphs */
		sBarInit.id = IDDES_PROPAIR;
		sBarInit.iRange = (UWORD)getMaxPropulsionSpeed();//DBAR_PROPMAXSPEED;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_AIR);
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_PROPWEIGHT;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_PROPAIRLAB;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_AIR);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_HOVER;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_PROPWEIGHTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}

//		/* Add the title label */
//		sTitleInit.id = IDDES_PROPLAB;
//		sTitleInit.pText = "Air Prop";
//		if (!widgAddLabel(psWScreen, &sTitleInit))
//		{
//			return FALSE;
//		}
		break;
	case IDES_GROUND:
		/* Add the bar graphs */
		sBarInit.id = IDDES_PROPROAD;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_ROAD);
		sBarInit.iRange = (UWORD)getMaxPropulsionSpeed();//DBAR_PROPMAXSPEED;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_PROPCOUNTRY;
		sBarInit.y = DES_STATBAR_Y2;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_OFFROAD);
		sBarInit.iRange = (UWORD)getMaxPropulsionSpeed();//DBAR_PROPMAXSPEED;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_PROPWATER;
		sBarInit.y = DES_STATBAR_Y3;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_WATER);
		sBarInit.iRange = (UWORD)getMaxPropulsionSpeed();//DBAR_PROPMAXSPEED;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}
		sBarInit.id = IDDES_PROPWEIGHT;
		sBarInit.y = DES_STATBAR_Y4;	//+= DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sBarInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sBarInit.iRange = (UWORD)getMaxComponentWeight();//DBAR_MAXWEIGHT;
		if (!widgAddBarGraph(psWScreen, &sBarInit))
		{
			return FALSE;
		}

		/* Add the labels */
		sLabInit.id = IDDES_PROPROADLAB;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_ROAD);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_ROAD;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_PROPCOUNTRYLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_OFFROAD);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_CROSSCOUNTRY;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_PROPWATERLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WATER);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_HOVER;	//WATER;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}
		sLabInit.id = IDDES_PROPWEIGHTLAB;
		sLabInit.y += DES_CLICKBARHEIGHT + DES_CLICKGAP;
		sLabInit.pTip = strresGetString(psStringRes, STR_DES_WEIGHT);
		sLabInit.pDisplay = intDisplayImage;
		sLabInit.pUserData = (void*)IMAGE_DES_WEIGHT;
		if (!widgAddLabel(psWScreen, &sLabInit))
		{
			return FALSE;
		}

//		/* Add the title label */
//		sTitleInit.id = IDDES_PROPLAB;
//		sTitleInit.pText = "Grnd Prop";
//		if (!widgAddLabel(psWScreen, &sTitleInit))
//		{
//			return FALSE;
//		}
		break;
	}

	/* Set the stats */
	intSetPropulsionStats(psStats);

	/* Lock the form down if necessary */
	if (desCompMode == IDES_PROPULSION)
	{
		widgSetButtonState(psWScreen, IDDES_PROPFORM, WBUT_LOCK);
	}

	return TRUE;
}


// count the number of available components
static UDWORD intNumAvailable(UBYTE *aAvailable, UDWORD numEntries,
							  COMP_BASE_STATS *asStats, UDWORD size)
{
	UDWORD				numButtons, i;
	COMP_BASE_STATS		*psCurrStats;

	numButtons = 0;
	psCurrStats = asStats;
	for(i=0; i < numEntries; i++)
	{
		/*all components have a flag which indicates whether they can be seen 
		in design screen now so don't need to check for default location*/
		//if ((aAvailable[i] & AVAILABLE) &&
		//	intGetLocation(psCurrStats) != LOC_DEFAULT)

		if (psCurrStats->design AND
			(aAvailable[i] & AVAILABLE))
		{
			numButtons++;
		}

		psCurrStats = (COMP_BASE_STATS *)( (UBYTE *)psCurrStats + size );
	}

	return numButtons;
}


/* Add the component tab form to the design screen */
static BOOL intAddComponentForm(UDWORD numButtons)
{
	W_FORMINIT		sFormInit;
	UDWORD			i, butPerForm, numFrm;

	memset(&sFormInit, 0, sizeof(W_FORMINIT));

	/* Count the number of buttons that will be on the tabbed form */
/*	numButtons = 0;
	for(i=0; i < numEntries; i++)
	{
		if (aAvailable[i] & AVAILABLE)
		{
			numButtons++;
		}
	}*/

	/* Calculate how many buttons will go on a single form */
/*	butPerForm = ((DES_RIGHTFORMWIDTH - DES_TABTHICKNESS - DES_TABBUTGAP) / 
						(DES_TABBUTWIDTH + DES_TABBUTGAP)) *
				 ((DES_RIGHTFORMHEIGHT - DES_TABTHICKNESS - DES_TABBUTGAP) / 
						(DES_TABBUTHEIGHT + DES_TABBUTGAP));*/
	butPerForm = DES_BUTSPERFORM;

	/* add a form to place the tabbed form on */
#ifdef PSX
	WidgSetOTIndex(OT2D_FARBACK);
#endif
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = 0;
	sFormInit.id = IDDES_RIGHTBASE;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = (SWORD)(RADTLX-2);
	sFormInit.y = (SWORD)DESIGN_Y;
	sFormInit.width = RET_FORMWIDTH;
	sFormInit.height = DES_RIGHTFORMHEIGHT + 4;
	sFormInit.pDisplay = intDisplayPlainForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	//now a single form
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = IDDES_RIGHTBASE;
	sFormInit.id = IDDES_COMPFORM;
	sFormInit.style = WFORM_TABBED;
	sFormInit.x = 2;
	sFormInit.y = 40;
	sFormInit.width = DES_RIGHTFORMWIDTH;
	sFormInit.height = DES_RIGHTFORMHEIGHT;
	numFrm = numForms(numButtons, butPerForm);
	sFormInit.numMajor = (UWORD)(numFrm >= WFORM_MAXMAJOR ? WFORM_MAXMAJOR-1 : numFrm);
	sFormInit.majorPos = WFORM_TABTOP;
	sFormInit.minorPos = WFORM_TABNONE;
	sFormInit.majorSize = DES_TAB_WIDTH;
	sFormInit.majorOffset = DES_TAB_LEFTOFFSET;
	sFormInit.tabVertOffset = (DES_TAB_HEIGHT/2);
	sFormInit.tabMajorThickness = DES_TAB_HEIGHT;
	sFormInit.pFormDisplay = intDisplayObjectForm;
	sFormInit.pUserData = (void*)&StandardTab;
	sFormInit.pTabDisplay = intDisplayTab;
	for (i=0; i< sFormInit.numMajor; i++)
	{
		sFormInit.aNumMinors[i] = 1;
	}

	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	return TRUE;
}

/* Add the Major system tab form to the design screen */
/*static BOOL intAddMultiComponentForm(UBYTE *aSensor, UDWORD numSensor,
									 UBYTE *aECM, UDWORD numECM,
									 UBYTE	*aConstruct, UDWORD numConstruct,
									 UBYTE *aWeapon, UDWORD numWeapon)
{
	W_FORMINIT		sFormInit;
	UDWORD			i, numButtons, butPerForm;

	memset(&sFormInit, 0, sizeof(W_FORMINIT));

	// Calculate how many buttons will go on a single form
	butPerForm = ((DES_RIGHTFORMWIDTH - DES_MINORSIZE - DES_TABBUTGAP) / 
						(DES_TABBUTWIDTH + DES_TABBUTGAP)) *
				 ((DES_RIGHTFORMHEIGHT - DES_MAJORSIZE - DES_TABBUTGAP) / 
						(DES_TABBUTHEIGHT + DES_TABBUTGAP)); 

	// add a form to place the tabbed form on
#ifdef PSX
	WidgSetOTIndex(OT2D_FARBACK);
#endif
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = 0;
	sFormInit.id = IDDES_RIGHTBASE;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = RADTLX-2;
	sFormInit.y = DESIGN_Y;
	sFormInit.width = DES_RIGHTFORMWIDTH;//RET_FORMWIDTH;
	sFormInit.height = DES_RIGHTFORMHEIGHT + 4;
	sFormInit.pDisplay = intDisplayPlainForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	// Initialise the form structure
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = IDDES_RIGHTBASE;			//0;
	sFormInit.id = IDDES_COMPFORM;
	sFormInit.style = WFORM_TABBED;
	sFormInit.x = 2;
	sFormInit.y = 10;
	sFormInit.width = DES_RIGHTFORMWIDTH;
	sFormInit.height = DES_RIGHTFORMHEIGHT;
	sFormInit.majorPos = WFORM_TABTOP;
	sFormInit.minorPos = WFORM_TABRIGHT;//WFORM_TABNONE;
	sFormInit.majorSize = 35;
	sFormInit.minorSize = 26;//0;
	sFormInit.minorOffset = 10;//0;
	sFormInit.majorOffset = 4;
	sFormInit.tabVertOffset = 0;//20;
	sFormInit.tabHorzOffset = -5;//0;
	sFormInit.tabMajorThickness = 44;
	sFormInit.tabMinorThickness = 11;//0;
	sFormInit.tabMajorGap = DES_TAB_SYSGAP;
	sFormInit.tabMinorGap = 0;//DES_TAB_SYSGAP - 2;
	sFormInit.numMajor = 3;
	sFormInit.apMajorTips[IDES_MAINTAB] = strresGetString(psStringRes, STR_DES_WEAPONS);
	sFormInit.apMajorTips[IDES_EXTRATAB] = strresGetString(psStringRes, STR_DES_OTHER);
	sFormInit.apMajorTips[IDES_EXTRATAB2] = strresGetString(psStringRes, STR_DES_COMMAND);
	sFormInit.pFormDisplay = intDisplayObjectForm;
	sFormInit.pUserData = (void*)&SystemTab;
	sFormInit.pTabDisplay = intDisplaySystemTab;

	// Calculate the number of minor tabs for each major
	numButtons = 0;
	for(i=0; i < numWeapon; i++)
	{
		if (aWeapon[i] & AVAILABLE)
		{
			numButtons++;
			if (numButtons == MAX_SYSTEM_COMPONENTS)
			{
				break;
			}
		}
	}
	sFormInit.aNumMinors[0] = numForms(numButtons, butPerForm);
// Hack so we can use design screen after pressing 'A'.
//	if(sFormInit.aNumMinors[0] >= WFORM_MAXMINOR) {
//		sFormInit.aNumMinors[0] = WFORM_MAXMINOR-1;
//	}

	numButtons = 0;
    for(i=0; i < numSensor; i++)
	{
		if ((aSensor[i] & AVAILABLE) AND 
			intGetLocation((COMP_BASE_STATS *)&asSensorStats[i]) != LOC_DEFAULT)
		{
			numButtons++;
			if (numButtons == MAX_SYSTEM_COMPONENTS)
			{
				break;
			}
		}
	}
	if (numButtons < MAX_SYSTEM_COMPONENTS)
	{
		for(i=0; i < numECM; i++)
		{
			if ((aECM[i] & AVAILABLE) AND 
				intGetLocation((COMP_BASE_STATS *)&asECMStats[i]) != LOC_DEFAULT)
			{
				numButtons++;
			}
		}
		for(i=0; i < numConstruct; i++)
		{
			if (aConstruct[i] & AVAILABLE)
			{
				numButtons++;
			}
		}
	}

	sFormInit.aNumMinors[1] = numForms(numButtons, butPerForm);
	sFormInit.aNumMinors[2] = 1;
// Hack so we can use design screen after pressing 'A'.
//	if(sFormInit.aNumMinors[1] >= WFORM_MAXMINOR) {
//		sFormInit.aNumMinors[1] = WFORM_MAXMINOR-1;
//	}


	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	return TRUE;
}*/


/* Add the system buttons (weapons, command droid, etc) to the design screen */
static BOOL intAddSystemButtons(SDWORD mode)
{
	W_BUTINIT	sButInit;

	memset(&sButInit, 0, sizeof(W_BUTINIT));

	// add the weapon button
	sButInit.formID = IDDES_RIGHTBASE;
	sButInit.id = IDDES_WEAPONS;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = DES_WEAPONBUTTON_X;
	sButInit.y = DES_SYSTEMBUTTON_Y;
	sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_WEAPONS);
	sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_WEAPONS);
	sButInit.pTip = strresGetString(psStringRes, STR_DES_WEAPONS);
	sButInit.FontID = WFont;
	sButInit.pDisplay = intDisplayButtonHilight;
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_DES_EXTRAHI , IMAGE_DES_WEAPONS);
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

    //if currently got a VTOL proplusion attached then don't add the system buttons
    if (!checkTemplateIsVtol(&sCurrDesign))
    {
	    // add the system button
	    sButInit.formID = IDDES_RIGHTBASE;
	    sButInit.id = IDDES_SYSTEMS;
	    sButInit.style = WBUT_PLAIN;
	    sButInit.x = DES_SYSTEMBUTTON_X;
	    sButInit.y = DES_SYSTEMBUTTON_Y;
	    sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_SYSTEMS);
	    sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_SYSTEMS);
	    sButInit.pTip = strresGetString(psStringRes, STR_DES_OTHER);
	    sButInit.FontID = WFont;
	    sButInit.pDisplay = intDisplayButtonHilight;
	    sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_DES_EXTRAHI , IMAGE_DES_SYSTEMS);
	    if (!widgAddButton(psWScreen, &sButInit))
	    {
		    return FALSE;
	    }
    }

#if 0	// command turrets now in systems
	// add the command droid button
	sButInit.formID = IDDES_RIGHTBASE;
	sButInit.id = IDDES_COMMAND;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = 86;
	sButInit.y = 10;
	sButInit.width = iV_GetImageWidth(IntImages, IMAGE_DES_COMMAND);
	sButInit.height = iV_GetImageHeight(IntImages, IMAGE_DES_COMMAND);
	sButInit.pTip = strresGetString(psStringRes, STR_DES_COMMAND);
	sButInit.FontID = WFont;
	sButInit.pDisplay = intDisplayButtonHilight;
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_DES_EXTRAHI , IMAGE_DES_COMMAND);
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}
#endif

	// lock down the correct button
	switch (mode)
	{
	case IDES_TURRET:
		widgSetButtonState(psWScreen, IDDES_WEAPONS, WBUT_LOCK);
		break;
	case IDES_BRAIN:
/*
		widgSetButtonState(psWScreen, IDDES_COMMAND, WBUT_LOCK);
		break;
*/
	case IDES_SYSTEM:
		widgSetButtonState(psWScreen, IDDES_SYSTEMS, WBUT_LOCK);
		break;
	default:
		ASSERT((FALSE, "intAddSystemButtons: unexpected mode"));
		break;
	}

	return TRUE;
}


/* Add the component buttons to the main tab of the component form */
static BOOL intAddComponentButtons(COMP_BASE_STATS *psStats, UDWORD size,
								   UBYTE *aAvailable,	UDWORD numEntries,
								   UDWORD compID,UDWORD WhichTab)
{
	W_FORMINIT			sButInit;
    W_TABFORM           *psTabForm;
	UDWORD				i, maxComponents;
	COMP_BASE_STATS		*psCurrStats;
	STRING				aButText[DES_COMPBUTMAXCHAR + 1];
	SDWORD				BufferID;
	PROPULSION_STATS	*psPropStats;
	BOOL				bVTol, bWeapon, bVtolWeapon;
    UWORD               numTabs;

	ClearObjectBuffers();

#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif

	memset(aButText, 0, DES_COMPBUTMAXCHAR + 1);
	memset(&sButInit, 0, sizeof(W_BUTINIT));

	/* Set up the button struct */
	sButInit.formID = IDDES_COMPFORM;
	sButInit.majorID = IDES_MAINTAB;
	sButInit.minorID = 0;
	sButInit.id = IDDES_COMPSTART;
	sButInit.style = WFORM_CLICKABLE;
	sButInit.x = DES_RIGHTFORMBUTX;
	sButInit.y = DES_RIGHTFORMBUTY;
	sButInit.width = DES_TABBUTWIDTH;
	sButInit.height = DES_TABBUTHEIGHT;

	//need to set max number of buttons possible
	if (psStats->ref >=REF_WEAPON_START && psStats->ref < REF_WEAPON_START + 
		REF_RANGE)
	{
		maxComponents = MAX_SYSTEM_COMPONENTS;
	}
	else
	{
		maxComponents = MAX_DESIGN_COMPONENTS;
	}

	/*if adding weapons - need to check if the propulsion is a VTOL*/
	bVTol = FALSE;

	if ( (psStats->ref >= REF_WEAPON_START) &&
		 (psStats->ref < REF_WEAPON_START + REF_RANGE) )
	{
		bWeapon = TRUE;
	}
	else
	{
		bWeapon = FALSE;
	}

	if ( bWeapon )
	{
		//check if the current Template propulsion has been set
		if (sCurrDesign.asParts[COMP_PROPULSION])
		{
			psPropStats = asPropulsionStats + sCurrDesign.
				asParts[COMP_PROPULSION];
			ASSERT((PTRVALID(psPropStats, sizeof(PROPULSION_STATS)),
				"intAddComponentButtons: invalid propulsion stats pointer"));
			if (asPropulsionTypes[psPropStats->propulsionType].travel == AIR)
			{
				bVTol = TRUE;
			}
		}
	}

	/* Add each button */
	desCompID = 0;
	numComponent = 0;
	psCurrStats = psStats;
	for (i=0; i<numEntries; i++)
	{
		/* If we are out of space in the list - stop */
		if (numComponent >= maxComponents)
		{
			//ASSERT((FALSE,
			//	"intAddComponentButtons: Too many components for the list"));
			break;
		}

		/* Skip unavailable entries and non-design ones*/
		if (!(aAvailable[i] & AVAILABLE) OR !psCurrStats->design)
		{
			/* Update the stats pointer for the next button */
			psCurrStats = (COMP_BASE_STATS *)(((UBYTE *)psCurrStats) + size);

			continue;
		}

		/*skip indirect weapons if VTOL propulsion or numVTOLattackRuns for the weapon is zero*/
		if ( bWeapon )
		{
			if ( ((WEAPON_STATS *)psCurrStats)->vtolAttackRuns )
			{
				bVtolWeapon = TRUE;
			}
			else
			{
				bVtolWeapon = FALSE;
			}

			if ( (bVTol && !bVtolWeapon) || (!bVTol && bVtolWeapon) )
			{
				/* Update the stats pointer for the next button */
				psCurrStats = (COMP_BASE_STATS *)(((UBYTE *)psCurrStats) + size);
				continue;
			}
		}

		/* Set the tip and add the button */
		strncpy(aButText, getStatName(psCurrStats), DES_COMPBUTMAXCHAR);
		sButInit.pTip = getStatName(psCurrStats);

		BufferID = GetObjectBuffer();
		ASSERT((BufferID >= 0,"Unable to acquire Topic buffer."));
		RENDERBUTTON_INUSE(&ObjectBuffers[BufferID]);
		ObjectBuffers[BufferID].Data = psCurrStats;
		sButInit.pUserData = (void*)&ObjectBuffers[BufferID];
		sButInit.pDisplay = intDisplayComponentButton;

		if (!widgAddForm(psWScreen, &sButInit))
		{
			return FALSE;
		}

		/* Store the stat pointer in the list */
		apsComponentList[numComponent++] = psCurrStats;

		/* If this matches the component ID lock the button */
		if (i == compID)
		{
			desCompID = sButInit.id;
			widgSetButtonState(psWScreen, sButInit.id, WBUT_LOCK);
			widgSetTabs(psWScreen, IDDES_COMPFORM, sButInit.majorID, sButInit.minorID);
		}

		// if this is a command droid that is in use or dead - make it unavailable
		if (statType(psCurrStats->ref) == COMP_BRAIN)
		{
			if ( ( ((COMMAND_DROID *)psCurrStats)->psDroid != NULL ) ||
				 ((COMMAND_DROID *)psCurrStats)->died )
			{
				widgSetButtonState(psWScreen, sButInit.id, WBUT_DISABLE);
			}
		}

		if(WhichTab == TAB_USEMAJOR) {
			/* Update the init struct for the next button */
			sButInit.id += 1;
			sButInit.x += DES_TABBUTWIDTH + DES_TABBUTGAP;
			if (sButInit.x + DES_TABBUTWIDTH+DES_TABBUTGAP > DES_RIGHTFORMWIDTH - DES_TABTHICKNESS)
			{
				sButInit.x = DES_RIGHTFORMBUTX;
				sButInit.y += DES_TABBUTHEIGHT + DES_TABBUTGAP;
			}
			if (sButInit.y + DES_TABBUTHEIGHT+DES_TABBUTGAP > DES_RIGHTFORMHEIGHT - DES_MAJORSIZE)
			{
				sButInit.y = DES_RIGHTFORMBUTY;
				sButInit.majorID += 1;
				if (sButInit.majorID >= WFORM_MAXMAJOR)
				{
					DBPRINTF(("Too many buttons for component form"));
					return FALSE;
				}
			}
		} else {
			/* Update the init struct for the next button */
			sButInit.id += 1;
			sButInit.x += DES_TABBUTWIDTH + DES_TABBUTGAP;
			if (sButInit.x + DES_TABBUTWIDTH+DES_TABBUTGAP > DES_RIGHTFORMWIDTH - DES_MINORSIZE)
			{
				sButInit.x = DES_RIGHTFORMBUTX;
				sButInit.y += DES_TABBUTHEIGHT + DES_TABBUTGAP;
			}
			if (sButInit.y + DES_TABBUTHEIGHT+DES_TABBUTGAP > DES_RIGHTFORMHEIGHT - DES_MAJORSIZE)
			{
				sButInit.y = DES_RIGHTFORMBUTY;
				sButInit.minorID += 1;
				if (sButInit.minorID >= WFORM_MAXMINOR)
				{
					DBPRINTF(("Too many buttons for component form"));
					return FALSE;
				}
			}
		}

		/* Update the stats pointer for the next button */
		psCurrStats = (COMP_BASE_STATS *)(((UBYTE *)psCurrStats) + size);
	}

    //hack to sort out the tabs on the weapon form
    //need to check how many buttons have been added to see if need all the tabs that are there
    psTabForm = (W_TABFORM *) widgGetFromID(psWScreen,IDDES_COMPFORM);
    if (psTabForm)
    {
        numTabs = psTabForm->numMajor;
        if (numComponent < (UDWORD)(numTabs * DES_BUTSPERFORM))
        {
            psTabForm->numMajor = numForms(numComponent, DES_BUTSPERFORM);
        }
    }

	return TRUE;
}


/* Return the location of a COMP_BASE_STATS */
/*static LOC intGetLocation(COMP_BASE_STATS *psStats)
{
	switch (statType(psStats->ref))
	{
	case COMP_SENSOR:
		return ((SENSOR_STATS *)psStats)->location;
		break;
	case COMP_ECM:
		return ((ECM_STATS *)psStats)->location;
		break;
	case COMP_REPAIRUNIT:
		return ((REPAIR_STATS *)psStats)->location;
		break;
	}

	// Nothing else has a location, so return Turret so it can be
	// selected on the design screen.
	//
	return LOC_TURRET;
}*/

/* Add the component buttons to the main tab of the component form */
static BOOL intAddExtraSystemButtons(UDWORD sensorIndex, UDWORD ecmIndex,
									 UDWORD constIndex, UDWORD repairIndex,
									 UDWORD brainIndex)
{
	W_FORMINIT		sButInit;
	UDWORD			i, buttonType, size=0;
	UDWORD			compIndex=0, numStats=0;
	COMP_BASE_STATS	*psCurrStats=0;
	UBYTE			*aAvailable=0;
	STRING			aButText[DES_COMPBUTMAXCHAR + 1];
	SDWORD			BufferID;

#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	memset(aButText, 0, DES_COMPBUTMAXCHAR + 1);
	memset(&sButInit, 0, sizeof(W_BUTINIT));

	// Set up the button struct
	sButInit.formID = IDDES_COMPFORM;
	sButInit.majorID = 0;
	sButInit.minorID = 0;
	sButInit.id = IDDES_EXTRASYSSTART;
	sButInit.style = WFORM_CLICKABLE;
	sButInit.x = DES_RIGHTFORMBUTX;
	sButInit.y = DES_RIGHTFORMBUTY;
	sButInit.width = DES_TABBUTWIDTH;
	sButInit.height = DES_TABBUTHEIGHT;

	// Add the buttons :
	// buttonType == 0  -  Sensor Buttons
	// buttonType == 1  -  ECM Buttons
	// buttonType == 2  -  Constructor Buttons
	// buttonType == 3  -  Repair Buttons
	// buttonType == 4  -  Brain Buttons
	numExtraSys = 0;
	for(buttonType = 0; buttonType < 5; buttonType++)
	{
		switch (buttonType)
		{
		case 0:
			// Sensor Buttons
			psCurrStats = (COMP_BASE_STATS *)asSensorStats;
			size = sizeof(SENSOR_STATS);
			aAvailable = apCompLists[selectedPlayer][COMP_SENSOR];
			//numStats = MaxComponents(numSensorStats);
			numStats = numSensorStats;
			compIndex = sensorIndex;
			break;
		case 1:
			// ECM Buttons
			psCurrStats = (COMP_BASE_STATS *)asECMStats;
			size = sizeof(ECM_STATS);
			aAvailable = apCompLists[selectedPlayer][COMP_ECM];
			//numStats = MaxComponents(numECMStats);
			numStats = numECMStats;
			compIndex = ecmIndex;
			break;
		case 2:
			// Constructor Buttons
			psCurrStats = (COMP_BASE_STATS *)asConstructStats;
			size = sizeof(CONSTRUCT_STATS);
			aAvailable = apCompLists[selectedPlayer][COMP_CONSTRUCT];
			//numStats = MaxComponents(numConstructStats);
			numStats = numConstructStats;
			compIndex = constIndex;
			break;
		case 3:
			// Repair Buttons
			psCurrStats = (COMP_BASE_STATS *)asRepairStats;
			size = sizeof(REPAIR_STATS);
			aAvailable = apCompLists[selectedPlayer][COMP_REPAIRUNIT];
			//numStats = MaxComponents(numECMStats);
			numStats = numRepairStats;
			compIndex = repairIndex;
			break;
		case 4:
			// Repair Buttons
			psCurrStats = (COMP_BASE_STATS *)asBrainStats;
			size = sizeof(BRAIN_STATS);
			aAvailable = apCompLists[selectedPlayer][COMP_BRAIN];
			//numStats = MaxComponents(numECMStats);
			numStats = numBrainStats;
			compIndex = brainIndex;
			break;
		}
		for (i=0; i<numStats; i++)
		{
			// If we are out of space in the list - stop
			if (numExtraSys >= MAXEXTRASYS)
			{
				ASSERT((FALSE,
					"intAddExtraSystemButtons: Too many components for the list"));
				return FALSE;
			}

			// Skip unavailable entries or non-design ones
			if (!(aAvailable[i] & AVAILABLE) ||
				//intGetLocation(psCurrStats) == LOC_DEFAULT)
				!psCurrStats->design)
			{
				// Update the stats pointer for the next button
				psCurrStats = (COMP_BASE_STATS *)(((UBYTE *)psCurrStats) + size);

				continue;
			}

			// Set the tip and add the button
			strncpy(aButText, getStatName(psCurrStats), DES_COMPBUTMAXCHAR);
			sButInit.pTip = getStatName(psCurrStats);

			BufferID = sButInit.id-IDDES_EXTRASYSSTART;
			ASSERT((BufferID < NUM_OBJECTBUFFERS,"BufferID > NUM_OBJECTBUFFERS"));
			/*switch(buttonType) {
				case 0:
					RENDERBUTTON_INUSE(&System0Buffers[BufferID]);
					System0Buffers[BufferID].Data = psCurrStats;
					sButInit.pUserData = (void*)&System0Buffers[BufferID];
					break;
				case 1:
					RENDERBUTTON_INUSE(&System1Buffers[BufferID]);
					System1Buffers[BufferID].Data = psCurrStats;
					sButInit.pUserData = (void*)&System1Buffers[BufferID];
					break;
				case 2:
					RENDERBUTTON_INUSE(&System2Buffers[BufferID]);
					System2Buffers[BufferID].Data = psCurrStats;
					sButInit.pUserData = (void*)&System2Buffers[BufferID];
					break;
			}*/
			//just use one set of buffers for mixed system form
			RENDERBUTTON_INUSE(&System0Buffers[BufferID]);
			if (statType(psCurrStats->ref) == COMP_BRAIN)
			{
				System0Buffers[BufferID].Data = ((BRAIN_STATS *)psCurrStats)->psWeaponStat;
			}
			else
			{
				System0Buffers[BufferID].Data = psCurrStats;
			}
			sButInit.pUserData = (void*)&System0Buffers[BufferID];

			sButInit.pDisplay = intDisplayComponentButton;

			if (!widgAddForm(psWScreen, &sButInit))
			{
				return FALSE;
			}

			// Store the stat pointer in the list 
			apsExtraSysList[numExtraSys++] = psCurrStats;

			// If this matches the sensorIndex note the form and button 
			if (i == compIndex)
			{
				desCompID = sButInit.id;
				widgSetButtonState(psWScreen, sButInit.id, WBUT_LOCK);
				widgSetTabs(psWScreen, IDDES_COMPFORM,
							sButInit.majorID, sButInit.minorID);
			}

			// Update the init struct for the next button
			sButInit.id += 1;
			sButInit.x += DES_TABBUTWIDTH + DES_TABBUTGAP;
			if (sButInit.x + DES_TABBUTWIDTH+DES_TABBUTGAP > DES_RIGHTFORMWIDTH - DES_MINORSIZE)
			{
				sButInit.x = DES_RIGHTFORMBUTX;
				sButInit.y += DES_TABBUTHEIGHT + DES_TABBUTGAP;
			}
			if (sButInit.y + DES_TABBUTHEIGHT+DES_TABBUTGAP > DES_RIGHTFORMHEIGHT - DES_MAJORSIZE)
			{
				sButInit.y = DES_RIGHTFORMBUTY;
				sButInit.majorID += 1;
			}

			// Update the stats pointer for the next button 
			psCurrStats = (COMP_BASE_STATS *)(((UBYTE *)psCurrStats) + size);
		}
	}

	return TRUE;
}




/* Set the bar graphs for the system clickable */
static void intSetSystemStats(COMP_BASE_STATS *psStats)
{
	W_FORM	*psForm;

	ASSERT(( PTRVALID(psStats, sizeof(COMP_BASE_STATS)),
//			 (((UBYTE *)psStats >= (UBYTE *)asCommandDroids) &&
//			  ((UBYTE *)psStats < (UBYTE *)asCommandDroids + sizeof(asCommandDroids))),
		"intSetSystemStats: Invalid stats pointer"));

	/* set form tip to stats string */
	widgSetTip( psWScreen, IDDES_SYSTEMFORM, getStatName(psStats) );

	/* set form stats for later display in intDisplayStatForm */
	psForm = (W_FORM *) widgGetFromID( psWScreen, IDDES_SYSTEMFORM );
	if ( psForm != NULL )
	{
		psForm->pUserData = psStats;
	}

	/* Set the correct system stats */
	switch (statType(psStats->ref))
	{
	case COMP_SENSOR:
		intSetSensorStats((SENSOR_STATS *)psStats);
		break;
	case COMP_ECM:
		intSetECMStats((ECM_STATS *)psStats);
		break;
	case COMP_WEAPON:
		intSetWeaponStats((WEAPON_STATS *)psStats);
		break;
	case COMP_CONSTRUCT:
		intSetConstructStats((CONSTRUCT_STATS *)psStats);
		break;
	case COMP_REPAIRUNIT:
		intSetRepairStats((REPAIR_STATS *)psStats);
		break;
	}
}

/* Set the shadow bar graphs for the system clickable */
static void intSetSystemShadowStats(COMP_BASE_STATS *psStats)
{
	/* Set the correct system stats - psStats can be set to NULL if
	 * desSysMode does not match the type of the stats.
	 */
	if (psStats)
	{
		switch (statType(psStats->ref))
		{
		case COMP_SENSOR:
			if (desSysMode == IDES_SENSOR)
			{
				intSetSensorShadowStats((SENSOR_STATS *)psStats);
			}
			else
			{
				psStats = NULL;
			}
			break;
		case COMP_ECM:
			if (desSysMode == IDES_ECM)
			{
				intSetECMShadowStats((ECM_STATS *)psStats);
			}
			else
			{
				psStats = NULL;
			}
			break;
		case COMP_WEAPON:
			if (desSysMode == IDES_WEAPON)
			{
				intSetWeaponShadowStats((WEAPON_STATS *)psStats);
			}
			else
			{
				psStats = NULL;
			}
			break;
		case COMP_CONSTRUCT:
			if (desSysMode == IDES_CONSTRUCT)
			{
				intSetConstructShadowStats((CONSTRUCT_STATS *)psStats);
			}
			else
			{
				psStats = NULL;
			}
			break;
		case COMP_BRAIN:
			psStats = NULL;
			break;
		case COMP_REPAIRUNIT:
			if (desSysMode == IDES_REPAIR)
			{
				intSetRepairShadowStats((REPAIR_STATS *)psStats);
			}
			else
			{
				psStats = NULL;
			}
			break;
		}
	}

	if (psStats == NULL)
	{
		switch (desSysMode)
		{
		case IDES_SENSOR:
			intSetSensorShadowStats(NULL);
			break;
		case IDES_ECM:
			intSetECMShadowStats(NULL);
			break;
		case IDES_WEAPON:
			intSetWeaponShadowStats(NULL);
			break;
		case IDES_CONSTRUCT:
			intSetConstructShadowStats(NULL);
			break;
		case IDES_REPAIR:
			intSetRepairShadowStats(NULL);
			break;
		}
	}
}

/* Set the bar graphs for the sensor stats */
static void intSetSensorStats(SENSOR_STATS *psStats)
{
	ASSERT((PTRVALID(psStats, sizeof(SENSOR_STATS)),
		"intSetSensorStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_SENSOR_START) &&
			(psStats->ref < REF_SENSOR_START + REF_RANGE),
		"intSetSensorStats: stats ref is out of range"));

	/* range */
	//widgSetBarSize(psWScreen, IDDES_SENSORRANGE, psStats->range);
	widgSetBarSize(psWScreen, IDDES_SENSORRANGE, 
		sensorRange(psStats, (UBYTE)selectedPlayer));
	/* power */
	//widgSetBarSize(psWScreen, IDDES_SENSORPOWER, psStats->power);
	widgSetBarSize(psWScreen, IDDES_SENSORPOWER, 
		sensorPower(psStats, (UBYTE)selectedPlayer));
	/* weight */
	widgSetBarSize(psWScreen, IDDES_SENSORWEIGHT, psStats->weight);
}

/* Set the shadow bar graphs for the sensor stats */
static void intSetSensorShadowStats(SENSOR_STATS *psStats)
{
	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(SENSOR_STATS)),
		"intSetSensorShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
			((psStats->ref >= REF_SENSOR_START) &&
			 (psStats->ref < REF_SENSOR_START + REF_RANGE)),
		"intSetSensorShadowStats: stats ref is out of range"));

	if (psStats)
	{
		/* range */
		//widgSetMinorBarSize(psWScreen, IDDES_SENSORRANGE, psStats->range);
		widgSetMinorBarSize(psWScreen, IDDES_SENSORRANGE, 
			sensorRange(psStats, (UBYTE)selectedPlayer));
		/* power */
		//widgSetMinorBarSize(psWScreen, IDDES_SENSORPOWER, psStats->power);
		widgSetMinorBarSize(psWScreen, IDDES_SENSORPOWER, 
			sensorPower(psStats, (UBYTE)selectedPlayer));
		/* weight */
		widgSetMinorBarSize(psWScreen, IDDES_SENSORWEIGHT, psStats->weight);
	}
	else
	{
		/* Remove the shadow bars */
		widgSetMinorBarSize(psWScreen, IDDES_SENSORRANGE, 0);
		widgSetMinorBarSize(psWScreen, IDDES_SENSORPOWER, 0);
		widgSetMinorBarSize(psWScreen, IDDES_SENSORWEIGHT, 0);		
	}
}


/* Set the bar graphs for the ECM stats */
static void intSetECMStats(ECM_STATS *psStats)
{
	ASSERT((PTRVALID(psStats, sizeof(ECM_STATS)),
		"intSetECMStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_ECM_START) &&
			(psStats->ref < REF_ECM_START + REF_RANGE),
		"intSetECMStats: stats ref is out of range"));

	/* power */
	//widgSetBarSize(psWScreen, IDDES_ECMPOWER, psStats->power);
	widgSetBarSize(psWScreen, IDDES_ECMPOWER, 
		ecmPower(psStats, (UBYTE)selectedPlayer));
	/* weight */
	widgSetBarSize(psWScreen, IDDES_ECMWEIGHT, psStats->weight);
}

/* Set the shadow bar graphs for the ECM stats */
static void intSetECMShadowStats(ECM_STATS *psStats)
{
	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(ECM_STATS)),
		"intSetECMShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
			((psStats->ref >= REF_ECM_START) &&
			 (psStats->ref < REF_ECM_START + REF_RANGE)),
		"intSetECMShadowStats: stats ref is out of range"));

	if (psStats)
	{
		/* power */
		//widgSetMinorBarSize(psWScreen, IDDES_ECMPOWER, psStats->power);
		widgSetMinorBarSize(psWScreen, IDDES_ECMPOWER, 
			ecmPower(psStats, (UBYTE)selectedPlayer));
		/* weight */
		widgSetMinorBarSize(psWScreen, IDDES_ECMWEIGHT, psStats->weight);
	}
	else
	{
		/* Remove the shadow bars */
		widgSetMinorBarSize(psWScreen, IDDES_ECMPOWER, 0);
		widgSetMinorBarSize(psWScreen, IDDES_ECMWEIGHT, 0);
	}
}


/* Set the bar graphs for the Constructor stats */
static void intSetConstructStats(CONSTRUCT_STATS *psStats)
{
	ASSERT((PTRVALID(psStats, sizeof(CONSTRUCT_STATS)),
		"intSetConstructStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_CONSTRUCT_START) &&
			(psStats->ref < REF_CONSTRUCT_START + REF_RANGE),
		"intSetConstructStats: stats ref is out of range"));

	/* power */
	//widgSetBarSize(psWScreen, IDDES_CONSTPOINTS, psStats->constructPoints);
	widgSetBarSize(psWScreen, IDDES_CONSTPOINTS, 
		constructorPoints(psStats, (UBYTE)selectedPlayer));
	/* weight */
	widgSetBarSize(psWScreen, IDDES_CONSTWEIGHT, psStats->weight);
}


/* Set the shadow bar graphs for the Constructor stats */
static void intSetConstructShadowStats(CONSTRUCT_STATS *psStats)
{
	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(CONSTRUCT_STATS)),
		"intSetConstructShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
		    ((psStats->ref >= REF_CONSTRUCT_START) &&
			 (psStats->ref < REF_CONSTRUCT_START + REF_RANGE)),
		"intSetConstructShadowStats: stats ref is out of range"));

	if (psStats)
	{
		/* power */
		//widgSetMinorBarSize(psWScreen, IDDES_CONSTPOINTS, psStats->constructPoints);
		widgSetMinorBarSize(psWScreen, IDDES_CONSTPOINTS, 
			constructorPoints(psStats, (UBYTE)selectedPlayer));
		/* weight */
		widgSetMinorBarSize(psWScreen, IDDES_CONSTWEIGHT, psStats->weight);
	}
	else
	{
		/* reset the shadow bars */
		widgSetMinorBarSize(psWScreen, IDDES_CONSTPOINTS, 0);
		widgSetMinorBarSize(psWScreen, IDDES_CONSTWEIGHT, 0);
	}
}

/* Set the bar graphs for the Repair stats */
static void intSetRepairStats(REPAIR_STATS *psStats)
{
	ASSERT((PTRVALID(psStats, sizeof(REPAIR_STATS)),
		"intSetRepairStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_REPAIR_START) &&
			(psStats->ref < REF_REPAIR_START + REF_RANGE),
		"intSetRepairStats: stats ref is out of range"));

	/* power */
	//widgSetBarSize(psWScreen, IDDES_REPAIRPOINTS, psStats->repairPoints);
	widgSetBarSize(psWScreen, IDDES_REPAIRPOINTS, 
		repairPoints(psStats, (UBYTE)selectedPlayer));
	/* weight */
	widgSetBarSize(psWScreen, IDDES_REPAIRWEIGHT, psStats->weight);
}


/* Set the shadow bar graphs for the Repair stats */
static void intSetRepairShadowStats(REPAIR_STATS *psStats)
{
	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(REPAIR_STATS)),
		"intSetRepairShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
		    ((psStats->ref >= REF_REPAIR_START) &&
			 (psStats->ref < REF_REPAIR_START + REF_RANGE)),
		"intSetRepairShadowStats: stats ref is out of range"));

	if (psStats)
	{
		/* power */
		//widgSetMinorBarSize(psWScreen, IDDES_REPAIRPOINTS, psStats->repairPoints);
		widgSetMinorBarSize(psWScreen, IDDES_REPAIRPOINTS, 
			repairPoints(psStats, (UBYTE)selectedPlayer));
		/* weight */
		widgSetMinorBarSize(psWScreen, IDDES_REPAIRWEIGHT, psStats->weight);
	}
	else
	{
		/* reset the shadow bars */
		widgSetMinorBarSize(psWScreen, IDDES_REPAIRPOINTS, 0);
		widgSetMinorBarSize(psWScreen, IDDES_REPAIRWEIGHT, 0);
	}
}


/* Set the bar graphs for the Weapon stats */
static void intSetWeaponStats(WEAPON_STATS *psStats)
{
	UDWORD	size;

	ASSERT((PTRVALID(psStats, sizeof(WEAPON_STATS)),
		"intSetWeaponStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_WEAPON_START) &&
			(psStats->ref < REF_WEAPON_START + REF_RANGE),
		"intSetWeaponStats: stats ref is out of range"));

	/* range */
	widgSetBarSize(psWScreen, IDDES_WEAPRANGE, proj_GetLongRange(psStats,0));
	/* rate of fire */
	/*size = (DBAR_WEAPMAXROF - (psStats->firePause + (psStats->
		firePause *	asWeaponUpgrade[selectedPlayer][psStats->weaponSubClass].
		firePause)/100));*/
	//size = DBAR_WEAPMAXROF - weaponFirePause(psStats, (UBYTE)selectedPlayer);
	//size = weaponFirePause(psStats, (UBYTE)selectedPlayer);
    size = weaponROF(psStats);
	/*if (size != 0)
	{
		size = ONEMIN / size;
	}*/
    //This Hack not needed anymore!!!
	/* Hack to set the ROF to zero for the NULL weapon */
	/*if (psStats == asWeaponStats)
	{
		size = 0;
	}*/
	widgSetBarSize(psWScreen, IDDES_WEAPROF, size);
	/* damage */
	//widgSetBarSize(psWScreen, IDDES_WEAPDAMAGE, psStats->damage);
	widgSetBarSize(psWScreen, IDDES_WEAPDAMAGE, (UWORD)weaponDamage(psStats, 
		(UBYTE)selectedPlayer));
	/* weight */
	widgSetBarSize(psWScreen, IDDES_WEAPWEIGHT, psStats->weight);
}

/* Set the shadow bar graphs for the Weapon stats */
static void intSetWeaponShadowStats(WEAPON_STATS *psStats)
{
	UDWORD	size;

	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(WEAPON_STATS)),
		"intSetWeaponShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
			((psStats->ref >= REF_WEAPON_START) &&
			 (psStats->ref < REF_WEAPON_START + REF_RANGE)),
		"intSetWeaponShadowStats: stats ref is out of range"));

	if (psStats)
	{
		/* range */
		widgSetMinorBarSize(psWScreen, IDDES_WEAPRANGE, proj_GetLongRange(psStats,0));
		/* rate of fire */
		/*size = (DBAR_WEAPMAXROF - (psStats->firePause + (psStats->
			firePause *	asWeaponUpgrade[selectedPlayer][psStats->weaponSubClass].
			firePause)/100));*/
		//size = DBAR_WEAPMAXROF - weaponFirePause(psStats, (UBYTE)selectedPlayer);
		//widgSetMinorBarSize(psWScreen, IDDES_WEAPROF, size);
		/*size = weaponFirePause(psStats, (UBYTE)selectedPlayer);
		if (size != 0)
		{
			size = ONEMIN / size;
		}*/
        size = weaponROF(psStats);
        //This Hack not needed anymore!!!
    	/* Hack to set the ROF to zero for the NULL weapon */
	    /*if (psStats == asWeaponStats)
	    {
		    size = 0;
	    }*/
		widgSetMinorBarSize(psWScreen, IDDES_WEAPROF, size);
		/* damage */
		//widgSetMinorBarSize(psWScreen, IDDES_WEAPDAMAGE, psStats->damage);
		widgSetMinorBarSize(psWScreen, IDDES_WEAPDAMAGE, (UWORD)weaponDamage(
			psStats, (UBYTE)selectedPlayer));
		/* weight */
		widgSetMinorBarSize(psWScreen, IDDES_WEAPWEIGHT, psStats->weight);
	}
	else
	{
		/* Reset the shadow bars */
		widgSetMinorBarSize(psWScreen, IDDES_WEAPRANGE, 0);
		widgSetMinorBarSize(psWScreen, IDDES_WEAPROF, 0);
		widgSetMinorBarSize(psWScreen, IDDES_WEAPDAMAGE, 0);
		widgSetMinorBarSize(psWScreen, IDDES_WEAPWEIGHT, 0);
	}
}

/* Set the bar graphs for the Body stats */
static void intSetBodyStats(BODY_STATS *psStats)
{
	W_FORM	*psForm;

	ASSERT((PTRVALID(psStats, sizeof(BODY_STATS)),
		"intSetBodyStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_BODY_START) &&
			(psStats->ref < REF_BODY_START + REF_RANGE),
		"intSetBodyStats: stats ref is out of range"));

	/* set form tip to stats string */
	widgSetTip( psWScreen, IDDES_BODYFORM, getStatName(psStats) );

	/* armour */
	//	size = WBAR_SCALE * psStats->armourValue/DBAR_BODYMAXARMOUR;
	//do kinetic armour
	//widgSetBarSize(psWScreen, IDDES_BODYARMOUR_K, psStats->armourValue[WC_KINETIC]);
	widgSetBarSize(psWScreen, IDDES_BODYARMOUR_K, bodyArmour(psStats,
		(UBYTE)selectedPlayer, DROID_BODY_UPGRADE, WC_KINETIC));
	//do heat armour
	//widgSetBarSize(psWScreen, IDDES_BODYARMOUR_H, psStats->armourValue[WC_HEAT] );
	widgSetBarSize(psWScreen, IDDES_BODYARMOUR_H, bodyArmour(psStats,
		(UBYTE)selectedPlayer, DROID_BODY_UPGRADE, WC_HEAT));
	/* body points */
	/*size = WBAR_SCALE * psStats->body/DBAR_BODYMAXPOINTS;
	if (size > WBAR_SCALE)
	{
		size = WBAR_SCALE;
	}
	widgSetBarSize(psWScreen, IDDES_BODYPOINTS, size);*/
	/* power */
	//widgSetBarSize(psWScreen, IDDES_BODYPOWER, psStats->powerOutput);
	widgSetBarSize(psWScreen, IDDES_BODYPOWER, bodyPower(psStats, 
		(UBYTE)selectedPlayer,DROID_BODY_UPGRADE));

	/* weight */
	widgSetBarSize(psWScreen, IDDES_BODYWEIGHT, psStats->weight);

	/* set form stats for later display in intDisplayStatForm */
	psForm = (W_FORM *) widgGetFromID( psWScreen, IDDES_BODYFORM );
	if ( psForm != NULL )
	{
		psForm->pUserData = psStats;
	}
}

/* Set the shadow bar graphs for the Body stats */
static void intSetBodyShadowStats(BODY_STATS *psStats)
{
	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(BODY_STATS)),
		"intSetBodyShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
			((psStats->ref >= REF_BODY_START) &&
			 (psStats->ref < REF_BODY_START + REF_RANGE)),
		"intSetBodyShadowStats: stats ref is out of range"));

	if (psStats)
	{
		/* armour - kinetic*/
		//size = WBAR_SCALE * psStats->armourValue/DBAR_BODYMAXARMOUR;
		//widgSetMinorBarSize(psWScreen, IDDES_BODYARMOUR_K,psStats->armourValue[WC_KINETIC]);
		widgSetMinorBarSize(psWScreen, IDDES_BODYARMOUR_K, bodyArmour(psStats,
			(UBYTE)selectedPlayer, DROID_BODY_UPGRADE, WC_KINETIC));
		//armour - heat 
		//widgSetMinorBarSize(psWScreen, IDDES_BODYARMOUR_H,psStats->armourValue[WC_HEAT]);
		widgSetMinorBarSize(psWScreen, IDDES_BODYARMOUR_H,bodyArmour(psStats,
			(UBYTE)selectedPlayer, DROID_BODY_UPGRADE, WC_HEAT));
		/* body points */
//			size = WBAR_SCALE * psStats->bodyPoints/DBAR_BODYMAXPOINTS;
//			if (size > WBAR_SCALE)
//			{
//				size = WBAR_SCALE;
//			}
//			widgSetMinorBarSize(psWScreen, IDDES_BODYPOINTS, size);
		/* power */
		//widgSetMinorBarSize(psWScreen, IDDES_BODYPOWER, psStats->powerOutput);
		widgSetMinorBarSize(psWScreen, IDDES_BODYPOWER, bodyPower(psStats, 
			(UBYTE)selectedPlayer, DROID_BODY_UPGRADE));

		/* weight */
		widgSetMinorBarSize(psWScreen, IDDES_BODYWEIGHT, psStats->weight);
	}
	else
	{
		/* Reset the shadow bars */
		widgSetMinorBarSize(psWScreen, IDDES_BODYARMOUR_K, 0);
		widgSetMinorBarSize(psWScreen, IDDES_BODYARMOUR_H, 0);
//		widgSetMinorBarSize(psWScreen, IDDES_BODYPOINTS, 0);
		widgSetMinorBarSize(psWScreen, IDDES_BODYPOWER, 0);
		widgSetMinorBarSize(psWScreen, IDDES_BODYWEIGHT, 0);
	}
}

/* Sets the Design Power Bar for a given Template */
static void intSetDesignPower(DROID_TEMPLATE *psTemplate)
{
	/* use the same scale as PowerBar in main window so values are relative */
	widgSetBarSize(psWScreen, IDDES_POWERBAR, calcTemplatePower(psTemplate));
}

/* Set the shadow bar graphs for the template power points - psStats is new hilited stats*/
static void intSetTemplatePowerShadowStats(COMP_BASE_STATS *psStats)
{
	UDWORD				type;
	//SDWORD				Avail, Used, Total;
	DROID_TEMPLATE		compTempl;

	if (&sCurrDesign != NULL AND psStats != NULL)
	{
		//create the comparison Template
		memcpy(&compTempl, &sCurrDesign, sizeof(DROID_TEMPLATE));
		type = statType(psStats->ref);
		/*if type = BODY or PROPULSION can do a straight comparison but if the new stat is
		a 'system' stat then need to find out which 'system' is currently in place so the
		comparison is meaningful*/
		if (desCompMode == IDES_SYSTEM)
		{
			//work out current system component
			if (sCurrDesign.asParts[COMP_ECM])
			{
				type = COMP_ECM;
			}
			else if (sCurrDesign.asParts[COMP_SENSOR])
			{
				type = COMP_SENSOR;
			}
			else if (sCurrDesign.asParts[COMP_CONSTRUCT])
			{
				type = COMP_CONSTRUCT;
			}
			else if (sCurrDesign.asParts[COMP_REPAIRUNIT])
			{
				type = COMP_REPAIRUNIT;
			}
			else if (sCurrDesign.asWeaps[0])
			{
				type = COMP_WEAPON;
			}
			else
			{
				type = COMP_UNKNOWN;
			}
		}

		switch (type)
		{
		case COMP_BODY:
			compTempl.asParts[COMP_BODY] = (BODY_STATS *)psStats - asBodyStats;
			break;
		case COMP_PROPULSION:
			compTempl.asParts[COMP_PROPULSION] = (PROPULSION_STATS *)psStats -
				asPropulsionStats;
			break;
		case COMP_ECM:
			compTempl.asParts[COMP_ECM] = (ECM_STATS *)psStats - asECMStats;
			break;
		case COMP_SENSOR:
			compTempl.asParts[COMP_SENSOR] = (SENSOR_STATS *)psStats - 
				asSensorStats;
			break;
		case COMP_CONSTRUCT:
			compTempl.asParts[COMP_CONSTRUCT] = (CONSTRUCT_STATS *)psStats - 
				asConstructStats;
			break;
		case COMP_REPAIRUNIT:
			compTempl.asParts[COMP_REPAIRUNIT] = (REPAIR_STATS *)psStats - 
				asRepairStats;
			break;
		case COMP_WEAPON:
			compTempl.asWeaps[0] = (WEAPON_STATS *)psStats - asWeaponStats;
			break;
		//default:
			//don't want to draw for unknown comp
		}

		widgSetMinorBarSize( psWScreen, IDDES_POWERBAR,
								calcTemplatePower(&compTempl));
	}
	else
	{
		/* Reset the shadow bar */
		widgSetMinorBarSize(psWScreen, IDDES_POWERBAR, 0);
	}
}

/* Sets the Body Points Bar for a given Template */
static void intSetBodyPoints(DROID_TEMPLATE *psTemplate)
{
	// If total greater than Body Bar size then scale values.
	widgSetBarSize( psWScreen, IDDES_BODYPOINTS, calcTemplateBody(psTemplate, 
		(UBYTE)selectedPlayer) );
}

/* Set the shadow bar graphs for the template Body points - psStats is new hilited stats*/
static void intSetTemplateBodyShadowStats(COMP_BASE_STATS *psStats)
{
	UDWORD				type;
	DROID_TEMPLATE		compTempl;

//#ifdef PSX
//	// Something in this function may be causeing a crash....
//#warning HMMMMMMM.... THIS FUNCTION APPEARS TO CAUSE A CRASH.... MUST FIX....
//	return;
//#endif

	if (&sCurrDesign != NULL AND psStats != NULL)
	{
		//create the comparison Template
		memcpy(&compTempl, &sCurrDesign, sizeof(DROID_TEMPLATE));
		type = statType(psStats->ref);
		/*if type = BODY or PROPULSION can do a straight comparison but if the new stat is
		a 'system' stat then need to find out which 'system' is currently in place so the
		comparison is meaningful*/
		if (desCompMode == IDES_SYSTEM)
		{
			//work out current system component
			if (sCurrDesign.asParts[COMP_ECM])
			{
				type = COMP_ECM;
			}
			else if (sCurrDesign.asParts[COMP_SENSOR])
			{
				type = COMP_SENSOR;
			}
			else if (sCurrDesign.asParts[COMP_CONSTRUCT])
			{
				type = COMP_CONSTRUCT;
			}
			else if (sCurrDesign.asParts[COMP_REPAIRUNIT])
			{
				type = COMP_REPAIRUNIT;
			}
			else if (sCurrDesign.asWeaps[0])
			{
				type = COMP_WEAPON;
			}
			else
			{
				type = COMP_UNKNOWN;
			}
		}

		switch (type)
		{
		case COMP_BODY:
			compTempl.asParts[COMP_BODY] = (BODY_STATS *)psStats - asBodyStats;
			break;
		case COMP_PROPULSION:
			compTempl.asParts[COMP_PROPULSION] = (PROPULSION_STATS *)psStats -
				asPropulsionStats;
			break;
		case COMP_ECM:
			compTempl.asParts[COMP_ECM] = (ECM_STATS *)psStats - asECMStats;
			break;
		case COMP_SENSOR:
			compTempl.asParts[COMP_SENSOR] = (SENSOR_STATS *)psStats - 
				asSensorStats;
			break;
		case COMP_CONSTRUCT:
			compTempl.asParts[COMP_CONSTRUCT] = (CONSTRUCT_STATS *)psStats - 
				asConstructStats;
			break;
		case COMP_REPAIRUNIT:
			compTempl.asParts[COMP_REPAIRUNIT] = (REPAIR_STATS *)psStats - 
				asRepairStats;
			break;
		case COMP_WEAPON:
//			compTempl.asWeaps[COMP_WEAPON] = (WEAPON_STATS *)psStats - asWeaponStats;
			compTempl.asWeaps[0] = (WEAPON_STATS *)psStats - asWeaponStats;
			break;
		//default:
			//don't want to draw for unknown comp
		}

		widgSetMinorBarSize( psWScreen, IDDES_BODYPOINTS,
								calcTemplateBody(&compTempl, (UBYTE)selectedPlayer));
	}
	else
	{
		/* Reset the shadow bar */
		widgSetMinorBarSize(psWScreen, IDDES_BODYPOINTS, 0);
	}
}


/* Calculate the speed of a droid over a type of terrain */
static UDWORD intCalcSpeed(TYPE_OF_TERRAIN type, PROPULSION_STATS *psProp)
{
	UDWORD		weight;

	/* Calculate the weight */
	weight = calcDroidWeight(&sCurrDesign);
	if (weight == 0)
	{
		return 0;
	}
    //we want the design screen to show zero speed over water for all prop types except Hover and Vtol
    if (type == TER_WATER)
    {
        if (!(psProp->propulsionType == HOVER OR psProp->propulsionType == LIFT))
        {
            return 0;
        }
    }


	return calcDroidSpeed(calcDroidBaseSpeed(&sCurrDesign, weight, 
		(UBYTE)selectedPlayer), type, psProp - asPropulsionStats);
}


/* Set the bar graphs for the Propulsion stats */
static void intSetPropulsionStats(PROPULSION_STATS *psStats)
{
	W_FORM	    *psForm;
    UDWORD      weight;

	ASSERT((PTRVALID(psStats, sizeof(PROPULSION_STATS)),
		"intSetPropulsionStats: Invalid stats pointer"));
	ASSERT(((psStats->ref >= REF_PROPULSION_START) &&
			(psStats->ref < REF_PROPULSION_START + REF_RANGE),
		"intSetPropulsionStats: stats ref is out of range"));

	/* set form tip to stats string */
	widgSetTip( psWScreen, IDDES_PROPFORM, getStatName(psStats) );

	/* set form stats for later display in intDisplayStatForm */
	psForm = (W_FORM *) widgGetFromID( psWScreen, IDDES_PROPFORM );
	if ( psForm != NULL )
	{
		psForm->pUserData = psStats;
	}

	switch (desPropMode)
	{
	case IDES_GROUND:
		/* Road speed */
		widgSetBarSize(psWScreen, IDDES_PROPROAD, intCalcSpeed(TER_ROAD, psStats));
		/* Cross country speed - grass */
		widgSetBarSize(psWScreen, IDDES_PROPCOUNTRY, intCalcSpeed(TER_SANDYBRUSH, psStats));
		/* Water speed */
		widgSetBarSize(psWScreen, IDDES_PROPWATER, intCalcSpeed(TER_WATER, psStats));
		break;
	case IDES_AIR:
		/* Air speed - terrain type doesn't matter, use road */
		widgSetBarSize(psWScreen, IDDES_PROPAIR, intCalcSpeed(TER_ROAD, psStats));
		break;
	}

	/* weight */
	//widgSetBarSize(psWScreen, IDDES_PROPWEIGHT, psStats->weight);

    /* propulsion weight is a percentage of the body weight */
    if (sCurrDesign.asParts[COMP_BODY] != 0)
    {
       	weight = psStats->weight * (asBodyStats + sCurrDesign.asParts[COMP_BODY])->
            weight / 100;
    }
    else
    {
        //if haven't got a body - can't calculate a value
        weight = 0;
    }
    widgSetBarSize(psWScreen, IDDES_PROPWEIGHT, weight);
}


/* Set the shadow bar graphs for the Propulsion stats */
static void intSetPropulsionShadowStats(PROPULSION_STATS *psStats)
{
    UDWORD      weight;


	ASSERT((psStats == NULL || PTRVALID(psStats, sizeof(PROPULSION_STATS)),
		"intSetPropulsionShadowStats: Invalid stats pointer"));
	ASSERT((psStats == NULL ||
			((psStats->ref >= REF_PROPULSION_START) &&
			 (psStats->ref < REF_PROPULSION_START + REF_RANGE)),
		"intSetPropulsionShadowStats: stats ref is out of range"));

	/* Only set the shadow stats if they are the right type */
	if (psStats &&
		((asPropulsionTypes[psStats->propulsionType].travel == GROUND &&
		  desPropMode != IDES_GROUND) ||
		 (asPropulsionTypes[psStats->propulsionType].travel == AIR &&
		  desPropMode != IDES_AIR)))
	{
		return;
	}

	switch (desPropMode)
	{
	case IDES_GROUND:
		if (psStats)
		{
			/* Road speed */
			widgSetMinorBarSize( psWScreen, IDDES_PROPROAD,
									intCalcSpeed(TER_ROAD, psStats) );
			/* Cross country speed - grass */
			widgSetMinorBarSize( psWScreen, IDDES_PROPCOUNTRY,
									intCalcSpeed(TER_SANDYBRUSH, psStats) );
			/* Water speed */
			widgSetMinorBarSize(psWScreen, IDDES_PROPWATER,
									intCalcSpeed(TER_WATER, psStats));
		}
		else
		{
			/* Reset the shadow bars */
			widgSetMinorBarSize(psWScreen, IDDES_PROPROAD, 0);
			widgSetMinorBarSize(psWScreen, IDDES_PROPCOUNTRY, 0);
			widgSetMinorBarSize(psWScreen, IDDES_PROPWATER, 0);
		}
		break;
	case IDES_AIR:
		if (psStats)
		{
			/* Air speed - terrain type doesn't matter, use ROAD */
			widgSetMinorBarSize( psWScreen, IDDES_PROPAIR,
									intCalcSpeed(TER_ROAD, psStats) );
		}
		else
		{
			/* Reset the shadow bar */
			widgSetMinorBarSize(psWScreen, IDDES_PROPAIR, 0);
		}
		break;
	}

	if (psStats)
	{
		/* weight */
		//widgSetMinorBarSize(psWScreen, IDDES_PROPWEIGHT, psStats->weight);

        /* propulsion weight is a percentage of the body weight */
        if (sCurrDesign.asParts[COMP_BODY] != 0)
        {
       	    weight = psStats->weight * (asBodyStats + sCurrDesign.asParts[COMP_BODY])->
                weight / 100;
        }
        else
        {
            //if haven't got a body - can't calculate a value
            weight = 0;
        }
        widgSetMinorBarSize(psWScreen, IDDES_PROPWEIGHT, weight);
	}
	else
	{
		/* Reset the shadow bar */
		widgSetMinorBarSize(psWScreen, IDDES_PROPWEIGHT, 0);
	}
}


/* Return a program for a specific order */
/*static UDWORD intGetProgram(PROGRAM_ORDERS order)
{
	UDWORD		index;

	for (index = 0; index < numProgramStats; index++)
	{
		if (asProgramStats[index].order == (UDWORD)order)
		{
			break;
		}
	}

	if (index == numProgramStats)
	{
		return 0;
	}

	return index;
}*/


/* Check whether a droid template is valid */
static BOOL intValidTemplate(DROID_TEMPLATE *psTempl)
{
	UDWORD i;

	// set the weapon for a command droid
	if (psTempl->asParts[COMP_BRAIN] != 0)
	{
		psTempl->numWeaps = 1;
		psTempl->asWeaps[0] = 
			asBrainStats[psTempl->asParts[COMP_BRAIN]].psWeaponStat - asWeaponStats;
//			asCommandDroids[selectedPlayer][psTempl->asParts[COMP_BRAIN]].nWeapStat;
	}

	/* Check all the components have been set */
	if (psTempl->asParts[COMP_BODY] == 0)
	{
		return FALSE;
	}
	else if (psTempl->asParts[COMP_PROPULSION] == 0)
	{
		return FALSE;
	}

	// Check a turret has been installed
	if (psTempl->numWeaps == 0 &&
		psTempl->asParts[COMP_SENSOR] == 0 &&
		psTempl->asParts[COMP_ECM] == 0 &&
		psTempl->asParts[COMP_BRAIN] == 0 &&
		psTempl->asParts[COMP_REPAIRUNIT] == 0 &&
		psTempl->asParts[COMP_CONSTRUCT] == 0 )
	{
		return FALSE;
	}

	/* Check the weapons */
	for(i=0; i<psTempl->numWeaps; i++)
	{
		if (psTempl->asWeaps[i] == 0)
		{
			return FALSE;
		}
	}

    //can only have a weapon on a VTOL propulsion
    if (checkTemplateIsVtol(psTempl))
    {
        if (psTempl->numWeaps == 0)
        {
            return FALSE;
        }
    }

	if (psTempl->asParts[COMP_SENSOR] == 0)
	{
		/* Set the default Sensor */
		psTempl->asParts[COMP_SENSOR] = aDefaultSensor[selectedPlayer];
	}

	if (psTempl->asParts[COMP_ECM] == 0)
	{
		/* Set the default ECM */
		psTempl->asParts[COMP_ECM] = aDefaultECM[selectedPlayer];
	}

	if (psTempl->asParts[COMP_REPAIRUNIT] == 0)
	{
		/* Set the default Repair */
		psTempl->asParts[COMP_REPAIRUNIT] = aDefaultRepair[selectedPlayer];
	}

	psTempl->ref = REF_TEMPLATE_START;

	/* Set a default program */
	/*psTempl->numProgs = 1;
	psTempl->droidType = droidTemplateType(psTempl);
	switch ( psTempl->droidType )
	{
	case DROID_WEAPON:
	case DROID_PERSON:
	case DROID_CYBORG:
	case DROID_DEFAULT:
		psTempl->asProgs[0] = intGetProgram(ORDER_ATTACK);
		break;
	case DROID_SENSOR:
		psTempl->asProgs[0] = intGetProgram(ORDER_GUARD);
		break;
	case DROID_ECM:
		psTempl->asProgs[0] = intGetProgram(ORDER_GUARD);
		break;
	case DROID_CONSTRUCT:
		psTempl->asProgs[0] = intGetProgram(ORDER_BUILD);
		break;
	case DROID_REPAIR:
		psTempl->asProgs[0] = intGetProgram(ORDER_REPAIR);
		break;
	}*/

	/* Calculate build points */
	psTempl->buildPoints = calcTemplateBuild(psTempl);
	psTempl->powerPoints = calcTemplatePower(psTempl);

	//set the droidtype
	psTempl->droidType = droidTemplateType(psTempl);

	/* copy current name into template */
#ifndef HASH_NAMES
	strcpy( sCurrDesign.aName, aCurrName );
#endif

	return TRUE;
}


#ifdef WIN32
// ajl. above function is static. A quick wrapper for the net stuff
BOOL  MultiPlayValidTemplate(DROID_TEMPLATE *psTempl)
{
	return(intValidTemplate(psTempl) );
}
#endif

void desCreateDefaultTemplate( void )
{
	/* set current design to default */
	memcpy( &sCurrDesign, &sDefaultDesignTemplate, sizeof(DROID_TEMPLATE) );

	/* reset stats */
	intSetDesignStats(&sCurrDesign);
	widgDelete(psWScreen, IDDES_SYSTEMFORM);
	desSysMode = IDES_NOSYSTEM;
	CurrentStatsTemplate = (BASE_STATS *) &sCurrDesign;
}

/* Remove the design widgets from the widget screen */
void intRemoveDesign(void)
{
	//save the current design on exit if it is valid
	saveTemplate();

	newTemplate = FALSE;

	widgDelete(psWScreen, IDDES_POWERFORM);
#ifdef WIN32
	widgDelete(psWScreen, IDDES_NAMEBOX);
#endif
	widgDelete(psWScreen, IDDES_TEMPLFORM);
	widgDelete(psWScreen, IDDES_TEMPLBASE);
	widgDelete(psWScreen, IDDES_COMPFORM);
	widgDelete(psWScreen, IDDES_RIGHTBASE);

	widgDelete(psWScreen, IDDES_BODYFORM);
	widgDelete(psWScreen, IDDES_PROPFORM);
	widgDelete(psWScreen, IDDES_SYSTEMFORM);

	widgDelete(psWScreen, IDDES_FORM);
	widgDelete( psWScreen, IDDES_STATSFORM );

	resetDesignPauseState();

#ifdef PSX
	if(GetControllerType(0) == CON_MOUSE) {
		intAddMouseInterface();
	}
	RevealMissionTimer();
#endif
}

/* set flashing flag for button */
void intSetButtonFlash( UDWORD id, BOOL bFlash )
{
#ifdef FLASH_BUTTONS
	WIDGET	*psWidget = widgGetFromID( psWScreen, id );

	ASSERT((psWidget->type == WIDG_BUTTON,"intSetButtonFlash : Not a button"));

	if ( bFlash == TRUE )
	{
		psWidget->display = intDisplayButtonFlash;
	}
	else
	{
		psWidget->display = intDisplayButtonHilight;
	}
#endif
}

#ifndef HASH_NAMES
/*
 * desTemplateNameCustomised
 *
 * Checks whether user has customised template name : template not
 * customised if not complete or if generated name same as current.
 */
BOOL desTemplateNameCustomised( DROID_TEMPLATE *psTemplate )
{
	if ( (psTemplate->droidType == DROID_DEFAULT) ||
		 (strcmp( getTemplateName(psTemplate),
				  GetDefaultTemplateName(psTemplate) ) == 0) )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
#endif

/* checks whether to update name or has user already changed it */
void desUpdateDesignName( DROID_TEMPLATE *psTemplate, STRING *szCurrName )
{

}

/* Process return codes from the design screen */
void intProcessDesign(UDWORD id)
{
	DROID_TEMPLATE	*psTempl = NULL, *psCurr, *psPrev;
	//DROID_TEMPLATE	*psTempPrev;
	UDWORD			currID;
//	DES_COMPMODE	currCompMode;
	UDWORD			i;
#ifndef HASH_NAMES
	BOOL			bTemplateNameCustomised;
#endif

//19	#ifdef PSX
//19	if(id == IDDES_NAMEBOX) {
//19	#ifdef HASH_NAMES
//19			intAddStringEntry(SENTRY_EDITSTRING,"BAD_NAME");		// this needs fixing - sorry about that
//19	#else
//19			intAddStringEntry(SENTRY_EDITSTRING, getStatName(sCurrDesign) );
//19	#endif
//19			widgFocusLost( widgGetFromID(psWScreen, IDDES_NAMEBOX) );
//19		}
//19	#endif

#ifdef WIN32
//	if (pie_GetRenderEngine() == ENGINE_GLIDE)
//	{
		/* Dirty hack to allow screen dumps from the 3dfx during design!!! */
//		if(keyPressed(KEY_D))
//		{
//			CONPRINTF(ConsoleString,(ConsoleString,"Hackety hack - Alex has written screen dump to disk - %s",iV_ScreenDumpToDisk()));
//		}
//	}
#endif

	/* check template button pressed */
	if (id >= IDDES_TEMPLSTART && id <= IDDES_TEMPLEND)
	{
		/* if first template create blank design */
		if ( id == IDDES_TEMPLSTART )
		{
			desCreateDefaultTemplate();

			strncpy(aCurrName, strresGetString(psStringRes, STR_DES_NEWVEH),
				WIDG_MAXSTR-1);
#ifdef HASH_NAMES
//			sCurrDesign.NameHash=HashString(aCurrName);
			sCurrDesign.NameHash=0;	// As we are creating a new design the name must be NULL - This is needed for the save games
#else
			strcpy( sCurrDesign.aName, aCurrName );
#endif
//			strncpy(aCurrName, strresGetString(psStringRes, STR_DES_NEWVEH),
//				WIDG_MAXSTR-1);

			/* hide body and system component buttons */
			widgHide( psWScreen, IDDES_SYSTEMBUTTON );
//			widgHide( psWScreen, IDDES_BODYBUTTON );
			widgHide( psWScreen, IDDES_PROPBUTTON );

			/* set button render routines to flash */
			intSetButtonFlash( IDDES_SYSTEMBUTTON, TRUE );
			intSetButtonFlash( IDDES_BODYBUTTON,   TRUE );
			intSetButtonFlash( IDDES_PROPBUTTON,   TRUE );
		}
		else
		{
			/* Find the template for the new button */
			currID = IDDES_TEMPLSTART;
			for( i=0; i<MAXTEMPLATES; i++ )
			{
				psTempl = apsTemplateList[i];

				if (currID == id)
				{
					break;
				}
				currID ++;
			}

			ASSERT( (psTempl != NULL, "intProcessDesign: template not found!\n") );

			if ( psTempl != NULL )
			{
				/* Set the new template */
				memcpy(&sCurrDesign, psTempl, sizeof(DROID_TEMPLATE));
#ifdef HASH_NAMES
				strncpy(aCurrName, strresGetString(NULL,psTempl->NameHash), WIDG_MAXSTR-1);
#else
				//strcpy( sCurrDesign.aName, aCurrName );
				strncpy( aCurrName, getTemplateName(psTempl), WIDG_MAXSTR-1);
#endif
				/* reveal body and propulsion component buttons */
				widgReveal( psWScreen, IDDES_BODYBUTTON );
				widgReveal( psWScreen, IDDES_PROPBUTTON );
				widgReveal( psWScreen, IDDES_SYSTEMBUTTON );

				/* turn off button flashes */
				intSetButtonFlash( IDDES_SYSTEMBUTTON, FALSE );
				intSetButtonFlash( IDDES_BODYBUTTON,   FALSE );
				intSetButtonFlash( IDDES_PROPBUTTON,   FALSE );

				/* reset button states */
				widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, 0);
				widgSetButtonState(psWScreen, IDDES_BODYBUTTON,   0);
				widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   0);
			}

			intSetDesignMode(IDES_BODY);
		}

		/* reveal and flash body component button */
		widgReveal( psWScreen, IDDES_BODYBUTTON );

#ifdef PSX
		SetCurrentSnapID(&InterfaceSnap,IDDES_BODYBUTTON);
#endif

#ifdef FLASH_BUTTONS
		widgSetButtonState(psWScreen, IDDES_BODYBUTTON, WBUT_CLICKLOCK);
#endif
		/* reveal design form if not already on-screen */
		widgReveal( psWScreen, IDDES_FORM );

		/* Droid template button has been pressed - clear the old button */
		if (droidTemplID != 0)
		{
			widgSetButtonState(psWScreen, droidTemplID, 0);
		}

		intSetDesignStats(&sCurrDesign);

		/* show body stats only */
		widgReveal( psWScreen, IDDES_STATSFORM );
		widgReveal( psWScreen, IDDES_BODYFORM );
		widgHide(   psWScreen, IDDES_PROPFORM );
		widgHide(   psWScreen, IDDES_SYSTEMFORM );

		/*Update the Power bar stats as the power to build will have changed */
		intSetDesignPower(&sCurrDesign);
		/*Update the body points */
		intSetBodyPoints(&sCurrDesign);

		/* Lock the button */
		widgSetButtonState(psWScreen, id, WBUT_LOCK);
		droidTemplID = id;

		/* Update the component form */
//		currCompMode = desCompMode;
		widgDelete(psWScreen, IDDES_COMPFORM);
		widgDelete(psWScreen, IDDES_RIGHTBASE);
		desCompMode = IDES_NOCOMPONENT;
		intSetDesignMode(IDES_BODY);
	}
	else if (id >= IDDES_COMPSTART && id <= IDDES_COMPEND)
	{
		/* check whether can change template name */
#ifndef HASH_NAMES
		bTemplateNameCustomised = desTemplateNameCustomised( &sCurrDesign );
#endif

		/* Component stats button has been pressed - clear the old button */
		if (desCompID != 0)
		{
			widgSetButtonState(psWScreen, desCompID, 0);
		}

		/* Set the stats in the template */
		switch (desCompMode)
		{
		case IDES_BRAIN:
			/* // Calculate the index of the brain
			sCurrDesign.asParts[COMP_BRAIN] = 
				((COMMAND_DROID *)apsComponentList[id - IDDES_COMPSTART]) -
					asCommandDroids[selectedPlayer];
			// Reset the sensor, ECM and constructor and repair
			//	- defaults will be set when OK is hit
			sCurrDesign.asParts[COMP_SENSOR] = 0;
			sCurrDesign.asParts[COMP_ECM] = 0;
			sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
			sCurrDesign.numWeaps = 0;
			// Set the new stats on the display
			intSetSystemForm(apsComponentList[id - IDDES_COMPSTART]);
			// do the callback if in the tutorial
			if (bInTutorial)
			{
				eventFireCallbackTrigger(CALL_DESIGN_COMMAND);
			}*/
			break;
		case IDES_SYSTEM:
			break;
		case IDES_TURRET:
			/* Calculate the index of the component */
			sCurrDesign.asWeaps[0] = 
				((WEAPON_STATS *)apsComponentList[id - IDDES_COMPSTART]) -
				asWeaponStats;
			sCurrDesign.numWeaps = 1;
			/* Reset the sensor, ECM and constructor and repair
				- defaults will be set when OK is hit */
			sCurrDesign.asParts[COMP_SENSOR] = 0;
			sCurrDesign.asParts[COMP_ECM] = 0;
			sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
			sCurrDesign.asParts[COMP_BRAIN] = 0;
			/* Set the new stats on the display */
			intSetSystemForm(apsComponentList[id - IDDES_COMPSTART]);
			// do the callback if in the tutorial
			if (bInTutorial)
			{
				eventFireCallbackTrigger(CALL_DESIGN_WEAPON);
			}
			break;
		case IDES_BODY:
			/* reveal propulsion button if hidden */
			widgReveal( psWScreen, IDDES_PROPBUTTON );

			/* Calculate the index of the component */
			sCurrDesign.asParts[COMP_BODY] = 
				((BODY_STATS *)apsComponentList[id - IDDES_COMPSTART]) -
				asBodyStats;
			/* Set the new stats on the display */
			intSetBodyStats((BODY_STATS *)apsComponentList[id - IDDES_COMPSTART]);
			// do the callback if in the tutorial
			if (bInTutorial)
			{
				eventFireCallbackTrigger(CALL_DESIGN_BODY);
			}
			break;
		case IDES_PROPULSION:
			/* Calculate the index of the component */
			sCurrDesign.asParts[COMP_PROPULSION] = 
				((PROPULSION_STATS *)apsComponentList[id - IDDES_COMPSTART]) -
				asPropulsionStats;
			/* Set the new stats on the display */
			intSetPropulsionStats((PROPULSION_STATS *)apsComponentList[id - IDDES_COMPSTART]);
			
			//check that the weapon is valid for this propulsion
			if (!intCheckValidWeaponForProp())
			{
				//no way of allocating more than one weapon is there?
				if (sCurrDesign.numWeaps > 1)
				{
					ASSERT((FALSE, 
						"designScreen: More than one weapon on droid - how?"));
				}
				//not valid weapon so initialise the weapon stat
				sCurrDesign.asWeaps[0] = 0;
                //init all other stats as well!
                sCurrDesign.asParts[COMP_SENSOR] = 0;
                sCurrDesign.asParts[COMP_BRAIN] = 0;
                sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
                sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
                sCurrDesign.asParts[COMP_ECM] = 0;
				/* Reset the weapon stats on the display */
				intSetSystemForm((COMP_BASE_STATS *)(asWeaponStats +
								sCurrDesign.asWeaps[0]));
                intSetDesignMode(IDES_PROPULSION);
			}
			// do the callback if in the tutorial
			if (bInTutorial)
			{
				eventFireCallbackTrigger(CALL_DESIGN_PROPULSION);
			}

			break;
		}

		/* Lock the new button */
		widgSetButtonState(psWScreen, id, WBUT_LOCK);
		desCompID = id;

		/* Update the propulsion stats as the droid weight will have changed */
		intSetPropulsionStats(asPropulsionStats + sCurrDesign.asParts[COMP_PROPULSION]);

		/*Update the Power bar stats as the power to build will have changed */
		intSetDesignPower(&sCurrDesign);
		/*Update the body points */
		intSetBodyPoints(&sCurrDesign);

		/* update name if not customised */
#ifndef HASH_NAMES
		if ( bTemplateNameCustomised == FALSE )
		{
			strcpy( sCurrDesign.aName,
					GetDefaultTemplateName(&sCurrDesign) );
		}
#endif

		/* Update the name in the edit box */
		intSetEditBoxTextFromTemplate( &sCurrDesign );

		/* flash next button if design not complete */
		if ( intValidTemplate( &sCurrDesign ) == FALSE )
		{
			/* reset button states */
			widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, 0);
			widgSetButtonState(psWScreen, IDDES_BODYBUTTON,   0);
			widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   0);

#ifdef FLASH_BUTTONS
			switch (desCompMode)
			{
				case IDES_BODY:
					widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   WBUT_CLICKLOCK);
					break;
				case IDES_PROPULSION:
					widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON,   WBUT_CLICKLOCK);
					break;
				case IDES_BRAIN:
				case IDES_SYSTEM:
				case IDES_TURRET:
					widgSetButtonState(psWScreen, IDDES_BODYBUTTON, WBUT_CLICKLOCK);
					break;
			}
#endif
		}
	}
	else if (id >= IDDES_EXTRASYSSTART && id <= IDDES_EXTRASYSEND)
	{
		/* check whether can change template name */
#ifndef HASH_NAMES
		bTemplateNameCustomised = desTemplateNameCustomised( &sCurrDesign );
#endif

		// Extra component stats button has been pressed - clear the old button
		if (desCompID != 0)
		{
			widgSetButtonState(psWScreen, desCompID, 0);
		}

		// Now store the new stats
		switch (statType(apsExtraSysList[id - IDDES_EXTRASYSSTART]->ref))
		{
		case COMP_SENSOR:
			// Calculate the index of the component
			sCurrDesign.asParts[COMP_SENSOR] = 
				((SENSOR_STATS *)apsExtraSysList[id - IDDES_EXTRASYSSTART]) -
				asSensorStats;
			// Reset the ECM, constructor and weapon and repair
			//	- defaults will be set when OK is hit
			sCurrDesign.numWeaps = 0;
			sCurrDesign.asWeaps[0] = 0;
			sCurrDesign.asParts[COMP_ECM] = 0;
			sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
			sCurrDesign.asParts[COMP_BRAIN] = 0;
			// Set the new stats on the display
			intSetSystemForm(apsExtraSysList[id - IDDES_EXTRASYSSTART]);
			break;
		case COMP_ECM:
			// Calculate the index of the component
			sCurrDesign.asParts[COMP_ECM] = 
				((ECM_STATS *)apsExtraSysList[id - IDDES_EXTRASYSSTART]) -
				asECMStats;
			// Reset the Sensor, constructor and weapon and repair
			//	- defaults will be set when OK is hit
			sCurrDesign.numWeaps = 0;
			sCurrDesign.asWeaps[0] = 0;
			sCurrDesign.asParts[COMP_SENSOR] = 0;
			sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
			sCurrDesign.asParts[COMP_BRAIN] = 0;
			// Set the new stats on the display
			intSetSystemForm(apsExtraSysList[id - IDDES_EXTRASYSSTART]);
			break;
		case COMP_CONSTRUCT:
			// Calculate the index of the component and repair
			sCurrDesign.asParts[COMP_CONSTRUCT] = 
				((CONSTRUCT_STATS *)apsExtraSysList[id - IDDES_EXTRASYSSTART]) -
				asConstructStats;
			// Reset the Sensor, ECM and weapon
			//	- defaults will be set when OK is hit 
			sCurrDesign.numWeaps = 0;
			sCurrDesign.asWeaps[0] = 0;
			sCurrDesign.asParts[COMP_ECM] = 0;
			sCurrDesign.asParts[COMP_SENSOR] = 0;
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
			sCurrDesign.asParts[COMP_BRAIN] = 0;
			// Set the new stats on the display 
			intSetSystemForm(apsExtraSysList[id - IDDES_EXTRASYSSTART]);
			break;
		case COMP_REPAIRUNIT:
			// Calculate the index of the component
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 
				((REPAIR_STATS *)apsExtraSysList[id - IDDES_EXTRASYSSTART]) -
				asRepairStats;
			// Reset the Sensor, ECM and weapon and construct
			//	- defaults will be set when OK is hit 
			sCurrDesign.numWeaps = 0;
			sCurrDesign.asWeaps[0] = 0;
			sCurrDesign.asParts[COMP_ECM] = 0;
			sCurrDesign.asParts[COMP_SENSOR] = 0;
			sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
			sCurrDesign.asParts[COMP_BRAIN] = 0;
			// Set the new stats on the display 
			intSetSystemForm(apsExtraSysList[id - IDDES_EXTRASYSSTART]);
			break;
		case COMP_BRAIN:
			/* Calculate the index of the brain */
			sCurrDesign.asParts[COMP_BRAIN] = 
				((BRAIN_STATS *)apsExtraSysList[id - IDDES_EXTRASYSSTART]) -
					asBrainStats;
			/* Reset the sensor, ECM and constructor and repair
				- defaults will be set when OK is hit */
			sCurrDesign.asParts[COMP_SENSOR] = 0;
			sCurrDesign.asParts[COMP_ECM] = 0;
			sCurrDesign.asParts[COMP_CONSTRUCT] = 0;
			sCurrDesign.asParts[COMP_REPAIRUNIT] = 0;
			sCurrDesign.numWeaps = 0;
			/* Set the new stats on the display */
			intSetSystemForm(apsExtraSysList[id - IDDES_EXTRASYSSTART]);
			break;
		}
		// Lock the new button
		widgSetButtonState(psWScreen, id, WBUT_LOCK);
		desCompID = id;

		// Update the propulsion stats as the droid weight will have changed
		intSetPropulsionStats(asPropulsionStats + sCurrDesign.asParts[COMP_PROPULSION]);

		// Update the Power bar stats as the power to build will have changed
		intSetDesignPower(&sCurrDesign);
		// Update the body points
		intSetBodyPoints(&sCurrDesign);

		/* update name if not customised */
#ifndef HASH_NAMES
		if ( bTemplateNameCustomised == FALSE )
		{
			strcpy( sCurrDesign.aName,
					GetDefaultTemplateName(&sCurrDesign) );
		}
#endif
		/* Update the name in the edit box */
		intSetEditBoxTextFromTemplate( &sCurrDesign );

		// do the callback if in the tutorial
		if (bInTutorial)
		{
			if (statType(apsExtraSysList[id - IDDES_EXTRASYSSTART]->ref) == COMP_BRAIN)
			{
				eventFireCallbackTrigger(CALL_DESIGN_COMMAND);
			}
			else
			{
				eventFireCallbackTrigger(CALL_DESIGN_SYSTEM);
			}
		}
#ifdef FLASH_BUTTONS
		/* flash body button */
//		widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, 0);
//		widgSetButtonState(psWScreen, IDDES_BODYBUTTON, WBUT_CLICKLOCK);
//		widgSetButtonState(psWScreen, IDDES_PROPBUTTON, 0);
#endif
	}
	else
	{
		switch (id)
		{
			/* The four component clickable forms */
		case IDDES_WEAPONS:
			desCompID = 0;
			intSetDesignMode(IDES_TURRET);
			break;
		case IDDES_COMMAND:
			desCompID = 0;
			intSetDesignMode(IDES_BRAIN);
			break;
		case IDDES_SYSTEMS:
			desCompID = 0;
			intSetDesignMode(IDES_SYSTEM);
			break;
			/* The name edit box */
		case IDDES_NAMEBOX:
#ifdef HASH_NAMES
#else
			strncpy(sCurrDesign.aName, widgGetString(psWScreen, IDDES_NAMEBOX),
						DROID_MAXNAME);
			strncpy(aCurrName, sCurrDesign.aName,WIDG_MAXSTR-1);
#endif
			break;
		case IDDES_BIN:
			/* Find the template for the current button */
			currID = IDDES_TEMPLSTART+1;
			//psTempPrev = NULL;
			for( i=1; i<MAXTEMPLATES; i++ )
			{
				psTempl = apsTemplateList[i];
				if (currID == droidTemplID && psTempl != &sCurrDesign)
				{
					break;
				}
				currID ++;
				//psTempPrev = psTempl;
			}



			/* remove template if found */
			if ( psTempl )
			{

#ifdef WIN32
				if (bMultiPlayer)		//ajl. inform others of template destruction.
				{
					SendDestroyTemplate(psTempl);
				}
#endif

				/*CAN'T ASSUME THIS - there are some templates that don't get passed 
				into the design screen*/
				/* update player template list.
				if( psTempPrev )
				{
					psTempPrev->psNext = psTempl->psNext;
				}
				else
				{
					apsDroidTemplates[selectedPlayer] = psTempl->psNext;
				}*/

				//update player template list.
				{
					for (psCurr = apsDroidTemplates[selectedPlayer], psPrev = NULL; 
						psCurr != NULL; psCurr = psCurr->psNext)
					{
						if (psCurr == psTempl)
						{
							if (psPrev)
							{
								psPrev->psNext = psCurr->psNext;
							}
							else
							{
								apsDroidTemplates[selectedPlayer] = psCurr->psNext;
							}

							//quit looking cos found
							break;
						}
						psPrev = psCurr;
					}
				}

				// Delete the template.
                //before deleting the template, need to make sure not being used in production
                deleteTemplateFromProduction(psTempl, (UBYTE)selectedPlayer);
				HEAP_FREE(psTemplateHeap, psTempl);

				/* get previous template and set as current */
				psTempl = apsTemplateList[i-1];
				
				/* update local list */
				desSetupDesignTemplates();
				
				/* Now update the droid template form */
				newTemplate = FALSE;
				widgDelete(psWScreen, IDDES_TEMPLFORM);
				widgDelete(psWScreen, IDDES_TEMPLBASE);
				intAddTemplateForm( psTempl );

				/* Set the new template */
				memcpy(&sCurrDesign, psTempl, sizeof(DROID_TEMPLATE));
#ifdef HASH_NAMES
				strncpy(aCurrName, strresGetString(NULL,psTempl->NameHash), WIDG_MAXSTR-1);
#else
				//strcpy( sCurrDesign.aName, aCurrName );
				strncpy( aCurrName, getTemplateName(psTempl), WIDG_MAXSTR-1);
#endif
				intSetEditBoxTextFromTemplate( psTempl );

				intSetDesignStats(&sCurrDesign);

				/* show body stats only */
				widgReveal( psWScreen, IDDES_STATSFORM );
				widgReveal( psWScreen, IDDES_BODYFORM );
				widgHide(   psWScreen, IDDES_PROPFORM );
				widgHide(   psWScreen, IDDES_SYSTEMFORM );

				/*Update the Power bar stats as the power to build will have changed */
				intSetDesignPower(&sCurrDesign);
				/*Update the body points */
				intSetBodyPoints(&sCurrDesign);

				/* show correct body component highlight */
				widgDelete(psWScreen, IDDES_COMPFORM);
				widgDelete(psWScreen, IDDES_RIGHTBASE);
				desCompMode = IDES_NOCOMPONENT;
				intSetDesignMode(IDES_BODY);
			}
			break;
		case IDDES_SYSTEMBUTTON:
			// Add the correct component form
			switch (droidTemplateType(&sCurrDesign))
			{
			case DROID_COMMAND:
/*
				intSetDesignMode(IDES_BRAIN);
				break;
*/
			case DROID_SENSOR:
			case DROID_CONSTRUCT:
			case DROID_ECM:
			case DROID_REPAIR:
				intSetDesignMode(IDES_SYSTEM);
				break;
			default:
				intSetDesignMode(IDES_TURRET);
				break;
			}
			/* reveal components if not already onscreen */
			widgReveal( psWScreen, IDDES_STATSFORM );
			widgReveal( psWScreen, IDDES_RIGHTBASE );
			widgReveal( psWScreen, IDDES_SYSTEMFORM );
			widgHide(   psWScreen, IDDES_BODYFORM );
			widgHide(   psWScreen, IDDES_PROPFORM );

#ifdef FLASH_BUTTONS
			/* lock button if design complete */
			if ( intValidTemplate( &sCurrDesign ) == TRUE )
			{
				widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, WBUT_CLICKLOCK);
				widgSetButtonState(psWScreen, IDDES_BODYBUTTON,   0);
				widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   0);
			}
#endif

#ifdef PSX
			SetCurrentSnapID(&InterfaceSnap,IDDES_COMPSTART);
#endif

			break;
		case IDDES_BODYBUTTON:
			/* reveal components if not already onscreen */
			widgReveal( psWScreen, IDDES_RIGHTBASE );
			intSetDesignMode(IDES_BODY);

			widgReveal( psWScreen, IDDES_STATSFORM );
			widgHide(   psWScreen, IDDES_SYSTEMFORM );
			widgReveal( psWScreen, IDDES_BODYFORM );
			widgHide(   psWScreen, IDDES_PROPFORM );

#ifdef FLASH_BUTTONS
			/* lock button if design complete */
			if ( intValidTemplate( &sCurrDesign ) == TRUE )
			{
				widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, 0);
				widgSetButtonState(psWScreen, IDDES_BODYBUTTON,   WBUT_CLICKLOCK);
				widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   0);
			}
#endif

#ifdef PSX
			SetCurrentSnapID(&InterfaceSnap,IDDES_COMPSTART);
#endif

			break;
		case IDDES_PROPBUTTON:
			/* reveal components if not already onscreen */
			widgReveal( psWScreen, IDDES_RIGHTBASE );
			intSetDesignMode(IDES_PROPULSION);
			widgReveal( psWScreen, IDDES_STATSFORM );
			widgHide(   psWScreen, IDDES_SYSTEMFORM );
			widgHide(   psWScreen, IDDES_BODYFORM );
			widgReveal( psWScreen, IDDES_PROPFORM );

#ifdef FLASH_BUTTONS
			/* lock button if design complete */
			if ( intValidTemplate( &sCurrDesign ) == TRUE )
			{
				widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, 0);
				widgSetButtonState(psWScreen, IDDES_BODYBUTTON,   0);
				widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   WBUT_CLICKLOCK);
			}
#endif

#ifdef PSX
			SetCurrentSnapID(&InterfaceSnap,IDDES_COMPSTART);
#endif

			break;
		}
	}

	/* show body button if component button pressed and
	 * save template if valid
	 */
	if ( ( id >= IDDES_COMPSTART && id <= IDDES_COMPEND ) ||
		 ( id >= IDDES_EXTRASYSSTART && id <= IDDES_EXTRASYSEND ) )
	{
		/* reveal body button if hidden */
		widgReveal( psWScreen, IDDES_BODYBUTTON );

		/* save template if valid */
		if (saveTemplate())
		{
#ifdef SCRIPTS
			eventFireCallbackTrigger(CALL_DROIDDESIGNED);
#endif
		}

		switch ( desCompMode )
		{
			case IDES_BODY:
				widgReveal( psWScreen, IDDES_BODYFORM );
				widgHide(   psWScreen, IDDES_PROPFORM );
				widgHide(   psWScreen, IDDES_SYSTEMFORM );
#ifdef PSX
				SetCurrentSnapID(&InterfaceSnap,IDDES_PROPBUTTON);
#endif
				break;

			case IDES_PROPULSION:
				widgHide(   psWScreen, IDDES_BODYFORM );
				widgReveal( psWScreen, IDDES_PROPFORM );
				widgHide(   psWScreen, IDDES_SYSTEMFORM );
#ifdef PSX
				SetCurrentSnapID(&InterfaceSnap,IDDES_SYSTEMBUTTON);
#endif
				break;

			case IDES_BRAIN:
			case IDES_SYSTEM:
			case IDES_TURRET:
				widgHide(   psWScreen, IDDES_BODYFORM );
				widgHide(   psWScreen, IDDES_PROPFORM );
				widgReveal( psWScreen, IDDES_SYSTEMFORM );
#ifdef PSX
//				SetCurrentSnapID(&InterfaceSnap,IDDES_BODYBUTTON);
#endif
				break;
		}

		widgReveal( psWScreen, IDDES_STATSFORM );
#if 1
		/* switch automatically to next component type if initial design */
		if ( !intValidTemplate( &sCurrDesign ) )
		{
			/* show next component design screen */
			switch ( desCompMode )
			{
				case IDES_BODY:
					intSetDesignMode( IDES_PROPULSION );
					widgReveal(psWScreen, IDDES_PROPBUTTON);
					widgSetButtonState(psWScreen, IDDES_PROPBUTTON, WBUT_CLICKLOCK);
					break;

				case IDES_PROPULSION:
					intSetDesignMode( IDES_TURRET );
					widgReveal(psWScreen, IDDES_SYSTEMBUTTON);
					widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, WBUT_CLICKLOCK);
					break;

				case IDES_BRAIN:
				case IDES_SYSTEM:
				case IDES_TURRET:
					intSetDesignMode( IDES_BODY );
					break;
			}
		}
#endif

	}
	//save the template if the name gets edited
	if (id == IDDES_NAMEBOX)
	{
		saveTemplate();
	}

}


/* Set the shadow bar graphs for the design screen */
void intRunDesign(void)
{
	UDWORD				statID;
	COMP_BASE_STATS		*psStats;
	BOOL				templateButton;

#ifdef PSX
	intProcessTabs();
#endif

	/* Find out which button was hilited */
	templateButton = FALSE;
	statID = widgGetMouseOver(psWScreen);

// Somut around here is casuing a nasty crash.....
	/* If a component button is hilited get the stats for it */
	if (statID == desCompID)
	{
		/* The mouse is over the selected component - no shadow stats */
		psStats = NULL;
	}
	else if (statID >= IDDES_COMPSTART && statID <= IDDES_COMPEND)
	{
//DBPRINTF(("1 %p\n",psStats);
		psStats = apsComponentList[statID - IDDES_COMPSTART];
	}
	else if (statID >= IDDES_EXTRASYSSTART && statID <= IDDES_EXTRASYSEND)
	{
//DBPRINTF(("2 %p\n",psStats);
		psStats = apsExtraSysList[statID - IDDES_EXTRASYSSTART];
	}
	else if (statID >= IDDES_TEMPLSTART && statID <= IDDES_TEMPLEND)
	{
//DBPRINTF(("3 %d\n",statID);
		runTemplateShadowStats(statID);
		templateButton = TRUE;
		psStats = NULL;
//DBPRINTF(("4 %p\n",psStats);
	}
	else
	{
		/* No component button so reset the stats to nothing */
		psStats = NULL;
	}

	/* Now set the bar graphs for the stats - don't bother if over template 
	since setting them all!*/
	if (!templateButton)
	{
		switch (desCompMode)
		{
		case IDES_BRAIN:
			break;
		case IDES_SYSTEM:
		case IDES_TURRET:
			intSetSystemShadowStats(psStats);
			intSetBodyShadowStats(NULL);
			intSetPropulsionShadowStats(NULL);
			break;
		case IDES_BODY:
			intSetSystemShadowStats(NULL);
			intSetBodyShadowStats((BODY_STATS *)psStats);
			intSetPropulsionShadowStats(NULL);
			break;
		case IDES_PROPULSION:
			intSetSystemShadowStats(NULL);
			intSetBodyShadowStats(NULL);
			intSetPropulsionShadowStats((PROPULSION_STATS *)psStats);
			break;
		}

		//set the template shadow stats
		intSetTemplateBodyShadowStats(psStats);
		intSetTemplatePowerShadowStats(psStats);
	}
}



/*void intDisplayTemplateForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_TABFORM *Form = (W_TABFORM*)psWidget;
	UDWORD x0,y0,x1,y1;
	pColours;

	x0 = xOffset+Form->x;
	y0 = yOffset+Form->y;
	x1 = x0 + Form->width;
	y1 = y0 + Form->height;

	AdjustTabFormSize(Form,&x0,&y0,&x1,&y1);

	RenderWindowFrame(&FrameDesignLeft,x0,y0,x1-x0,y1-y0);
}*/


/*void intDisplayStatusForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_TABFORM *Form = (W_TABFORM*)psWidget;
	UDWORD x0,y0,x1,y1;
	pColours;

	x0 = xOffset+Form->x;
	y0 = yOffset+Form->y;
	x1 = x0 + Form->width;
	y1 = y0 + Form->height;

	AdjustTabFormSize(Form,&x0,&y0,&x1,&y1);

	RenderWindowFrame(&FrameDesignCenter,x0,y0,x1-x0,y1-y0);
}*/


/*void intDisplayComponentForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_TABFORM *Form = (W_TABFORM*)psWidget;
	UDWORD x0,y0,x1,y1;
	pColours;

	x0 = xOffset+Form->x;
	y0 = yOffset+Form->y;
	x1 = x0 + Form->width;
	y1 = y0 + Form->height;

	AdjustTabFormSize(Form,&x0,&y0,&x1,&y1);

	RenderWindowFrame(&FrameDesignRight,x0,y0,x1-x0,y1-y0);
}*/


extern void BoxBlueWash(UWORD x,UWORD y,UWORD w,UWORD h,BOOL Animate);

void intDisplayStatForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_CLICKFORM		*Form = (W_CLICKFORM*)psWidget;
	UWORD			x0, y0;
	static UDWORD	iRY = 45;
//	BOOL			Hilight = FALSE;
	BASE_STATS		*psStats;
	iVector			Rotation,Position;
	SWORD			templateRadius;
#ifdef PSX
	SWORD			OldBias;
#endif
	SDWORD			falseScale;

	UNUSEDPARAMETER(pColours);

	/* get stats from userdata pointer in widget stored in
	 * intSetSystemStats, intSetBodyStats, intSetPropulsionStats
	 */
	psStats = (BASE_STATS *) Form->pUserData;

	x0 = (UWORD)(xOffset+Form->x);
	y0 = (UWORD)(yOffset+Form->y);

	DrawBegin();

#ifdef WIN32
	iV_DrawImage(IntImages,(UWORD)(IMAGE_DES_STATBACKLEFT),x0,y0);
	iV_DrawImageRect(IntImages,IMAGE_DES_STATBACKMID,
				x0+iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKLEFT),y0,
				0,0,
				Form->width-iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKLEFT)-iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKRIGHT),
				iV_GetImageHeight(IntImages,IMAGE_DES_STATBACKMID) );
	iV_DrawImage(IntImages,IMAGE_DES_STATBACKRIGHT,
				x0+Form->width-iV_GetImageWidth(IntImages,(UWORD)(IMAGE_DES_STATBACKRIGHT)),y0);

	/* display current component */
	pie_SetGeometricOffset( (xOffset+psWidget->width/4),
							(yOffset+psWidget->height/2) );
	Rotation.x = -30;
	Rotation.y = iRY;
	Rotation.z = 0;

	/* inc rotation if highlighted */
	if ( Form->state & WCLICK_HILITE )
	{
		iRY += (BUTTONOBJ_ROTSPEED*frameTime2) / GAME_TICKS_PER_SEC;
		iRY %= 360;
	}

	templateRadius = (SWORD)(getComponentRadius(psStats));

	Position.x = 0;
	Position.y = -templateRadius / 4;
//	Position.z = templateRadius * 12;
	Position.z = BUTTON_DEPTH;

	//scale the object around the BUTTON_RADIUS so that half size objects are draw are draw 75% the size of normal objects
	falseScale = (DESIGN_COMPONENT_SCALE * COMPONENT_RADIUS) / templateRadius;
	falseScale = (falseScale/2) + (DESIGN_COMPONENT_SCALE/2);
	//display component in bottom design screen window
	displayComponentButton( psStats, &Rotation, &Position, TRUE, falseScale);
#else
	// needed for the clipping on the design screen
	OpenButtonRender(x0,y0,256,256);
/*
	iV_DrawImage(IntImages,(UWORD)(IMAGE_DES_STATBACKLEFT),x0,y0);
	iV_DrawImageRect(IntImages,IMAGE_DES_STATBACKMID,
				x0+iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKLEFT),y0,
				0,0,
				Form->width-iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKLEFT)-iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKRIGHT)+2,
				iV_GetImageHeight(IntImages,IMAGE_DES_STATBACKMID) );
	iV_DrawImage(IntImages,IMAGE_DES_STATBACKRIGHT,
				x0+Form->width-iV_GetImageWidth(IntImages,(UWORD)(IMAGE_DES_STATBACKRIGHT)),y0);
*/

#define MID1_WIDTH		114
#define MID3_WIDTH		130

	{
		int x = x0;

		iV_DrawImage(IntImages,(UWORD)(IMAGE_DES_STATBACKLEFT),x,y0);
		x += iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKLEFT);

		iV_DrawImageRect(IntImages,IMAGE_DES_STATBACKMID1,
					x,y0,
					0,0,MID1_WIDTH,
					iV_GetImageHeight(IntImages,IMAGE_DES_STATBACKMID1) );
		x += MID1_WIDTH;

		iV_DrawImage(IntImages,(UWORD)(IMAGE_DES_STATBACKMID2A),x,y0);
		x +=  iV_GetImageWidth(IntImages,IMAGE_DES_STATBACKMID2A);

		iV_DrawImageRect(IntImages,IMAGE_DES_STATBACKMID3,
					x,y0,
					0,0,MID3_WIDTH,
					iV_GetImageHeight(IntImages,IMAGE_DES_STATBACKMID3) );
		x += MID3_WIDTH;

		iV_DrawImage(IntImages,IMAGE_DES_STATBACKRIGHT,
					x0+Form->width-iV_GetImageWidth(IntImages,(UWORD)(IMAGE_DES_STATBACKRIGHT)),y0);
	}

//	iV_BoxFill(x0,y0,x0+Form->width,y0+Form->height,234);

	/* display current component */
	SetGeomOffset( XToPSX(xOffset+psWidget->width/4),YToPSX(yOffset+psWidget->height/2)+10 );

	Rotation.x = -30;
	Rotation.y = iRY;
	Rotation.z = 0;

	/* inc rotation if highlighted */
	if ( Form->state & WCLICK_HILITE )
	{
		iRY += (BUTTONOBJ_ROTSPEED*frameTime2) / GAME_TICKS_PER_SEC;
		iRY %= 360;
	}

	templateRadius = (SWORD)(getComponentDroidTemplateRadius((DROID_TEMPLATE*)
					CurrentStatsTemplate));

	Position.x = 0;
	Position.y = 0;
	Position.z = templateRadius * 10;
	
	SetIMDRenderingMode(USE_FIXEDZ,0);			// Render to a fixed OtZ.
	setComponentButtonOTIndex(OT2D_FARFARFORE);	// Set OtZ to render to.

	// Stop the renderer playing with the OTZ.
	OldBias = psxiv_GetZBias();		// Store the current Z Bias.
	psxiv_SetZBias(0);				// Don't want the renderer to add anything to the OtZ.
	psxiv_EnableZCheck(FALSE);		// Rendering over the 2d so don't check for this in the renderer.

	// Flush the current TPageID at this OT index.
	UpdateTPageID(0,OT2D_FARFARFORE);	

	displayComponentButton( psStats, &Rotation, &Position, TRUE, 100);

	psxiv_SetZBias(OldBias);			// Restore the renderers z bias.
	psxiv_EnableZCheck(TRUE);			// And re-enable OtZ range checks
	SetIMDRenderingMode(USE_MAXZ,0); 	// Set OT position calculation back to using the max Z value
	setComponentButtonOTIndex(ORDERING_BUTTONRENDERING);	// Restore draw depth for button rendering.

	CloseButtonRender();		// for clipping window
#endif

	DrawEnd();
}

/* Displays the 3D view of the droid in a window on the design form */
void intDisplayViewForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_FORM			*Form = (W_FORM*)psWidget;
	UDWORD			x0,y0,x1,y1;
	static UDWORD	iRY = 45;
	iVector			Rotation,Position;
	SWORD			templateRadius;
#ifdef PSX
	SWORD			OldBias;
#endif
	SDWORD			falseScale;

	UNUSEDPARAMETER(pColours);

	x0 = xOffset+Form->x;
	y0 = yOffset+Form->y;
	x1 = x0 + Form->width;
	y1 = y0 + Form->height;


	RenderWindowFrame(&FrameNormal,x0,y0,x1-x0,y1-y0);

	if(CurrentStatsTemplate) {

#ifdef WIN32
		pie_SetGeometricOffset(  (DES_CENTERFORMX+DES_3DVIEWX) + (DES_3DVIEWWIDTH/2),
								(DES_CENTERFORMY+DES_3DVIEWY) + (DES_3DVIEWHEIGHT/4) + 32);
#else
		SetGeomOffset( XToPSX( ((DES_CENTERFORMX+DES_3DVIEWX) + (DES_3DVIEWWIDTH/2)) ),
					   YToPSX( ((DES_CENTERFORMY+DES_3DVIEWY) + (DES_3DVIEWHEIGHT/2) + 32)) );
#endif

		Rotation.x = -30;
//		Rotation.y = ViewRotation;
		Rotation.y = iRY;
		Rotation.z = 0;

		/* inc rotation */
		iRY += (BUTTONOBJ_ROTSPEED*frameTime2) / GAME_TICKS_PER_SEC;
		iRY %= 360;

#ifdef WIN32
		//fixed depth scale the pie
		Position.x = 0;
		Position.y = -100;
		Position.z = BUTTON_DEPTH;

		templateRadius = (SWORD)(getComponentDroidTemplateRadius((DROID_TEMPLATE*)
			CurrentStatsTemplate));
		//scale the object around the OBJECT_RADIUS so that half size objects are draw are draw 75% the size of normal objects
		falseScale = (DESIGN_DROID_SCALE * OBJECT_RADIUS) / templateRadius;

		//display large droid view in the design screen
		displayComponentButtonTemplate((DROID_TEMPLATE*)&sCurrDesign,&Rotation,&Position,TRUE, falseScale);
#else// PSX
		templateRadius = (SWORD)(getComponentDroidTemplateRadius((DROID_TEMPLATE*)
			CurrentStatsTemplate));
		Position.x = 0;
		Position.y = 0;
		Position.z = templateRadius * 7;
#ifdef LIMITBUTZ
		if (Position.z > (INTERFACE_DEPTH- templateRadius))
		{
			Position.z = INTERFACE_DEPTH - templateRadius;
		}
#endif
		SetIMDRenderingMode(USE_FIXEDZ,0);			// Render to a fixed OtZ.
		setComponentButtonOTIndex(OT2D_FARFARFORE);	// Set OtZ to render to.

		// Stop the renderer playing with the OTZ.
		OldBias = psxiv_GetZBias();		// Store the current Z Bias.
		psxiv_SetZBias(0);				// Don't want the renderer to add anything to the OtZ.
		psxiv_EnableZCheck(FALSE);		// Rendering over the 2d so don't check for this in the renderer.

		// Flush the current TPageID at this OT index.
		UpdateTPageID(0,OT2D_FARFARFORE);	


//		if(((DROID_TEMPLATE*)&sCurrDesign)->droidType == DROID_DEFAULT) {
			compSetTransMode(TRUE,TRANSMODE_TRANSLUCENT);
//		}




		displayComponentButtonTemplate((DROID_TEMPLATE*)&sCurrDesign,&Rotation,&Position,TRUE, PSX_BUTTON_SCALE);

		compSetTransMode(FALSE,0);

		psxiv_SetZBias(OldBias);			// Restore the renderers z bias.
		psxiv_EnableZCheck(TRUE);			// And re-enable OtZ range checks
		SetIMDRenderingMode(USE_MAXZ,0); 	// Set OT position calculation back to using the max Z value
		setComponentButtonOTIndex(ORDERING_BUTTONRENDERING);	// Restore draw depth for button rendering.
#endif
	}

//	ViewRotation+=2;
}


void intDisplayTemplateButton(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	intDisplayStatsButton(psWidget, xOffset, yOffset, pColours);
}


void intDisplayComponentButton(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
//	iIMDShape *OldCurShape = CurrentStatsShape;
//	SWORD OldCurIndex = CurrentStatsIndex;
	BASE_STATS *OldCurStatsTemplate = CurrentStatsTemplate;

	intDisplayStatsButton(psWidget, xOffset, yOffset, pColours);

	CurrentStatsTemplate = OldCurStatsTemplate;
//	CurrentStatsShape = OldCurShape;
//	CurrentStatsIndex = OldCurIndex;
}

/* General display window for the design form  SOLID BACKGROUND - NOT TRANSPARENT*/
void intDisplayDesignForm(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_TABFORM *Form = (W_TABFORM*)psWidget;
	UDWORD x0,y0,x1,y1;
	UNUSEDPARAMETER(pColours);

	x0 = xOffset+Form->x;
	y0 = yOffset+Form->y;
	x1 = x0 + Form->width;
	y1 = y0 + Form->height;

	//AdjustTabFormSize(Form,&x0,&y0,&x1,&y1);

	//RenderWindowFrame(&FrameDesignView,x0,y0,x1-x0,y1-y0);
	RenderWindowFrame(&FrameNormal,x0,y0,x1-x0,y1-y0);
}


// NOTE!! if(when) this function is changed, please mail alexlee.
/* save the current Template if valid. Return TRUE if stored */
BOOL saveTemplate(void)
{
	DROID_TEMPLATE	*psTempl = NULL, *psPlayerTempl, *psPrevTempl;
	BOOL			stored = FALSE, bTemplateFound = FALSE;
	UDWORD			i, iCurrID;

	/* if first (New Design) button selected find empty template
	 * else find current button template
	 */
	if ( droidTemplID == IDDES_TEMPLSTART )
	{
		/* find empty template and point to that */
		for( i=1; i<MAXTEMPLATES; i++ )
		{
			psTempl = apsTemplateList[i];

			if ( psTempl == NULL )
			{
				bTemplateFound = TRUE;
				break;
			}
		}
	}
	else
	{
		/* Find the template for the current button */
		iCurrID = IDDES_TEMPLSTART + 1;
		for( i=1; i<MAXTEMPLATES; i++ )
		{
			psTempl = apsTemplateList[i];

			if ( iCurrID == droidTemplID )
			{
				bTemplateFound = TRUE;
				break;
			}
			iCurrID++;
		}
	}

	if ( bTemplateFound == TRUE && intValidTemplate( &sCurrDesign ) )
	{
		/* create new template if button is NULL,
		 * else store changes to existing template */
		if ( psTempl == NULL )
		{
			/* The design needs a new template in the list */
			if (!HEAP_ALLOC(psTemplateHeap, &psTempl))
			{
				DBPRINTF( ("saveTemplate: heap alloc failed\n") );
				return FALSE;
			}

			psTempl->ref = REF_TEMPLATE_START;
			newTemplate = TRUE;
			/* Add it to temp array */
			apsTemplateList[i] = psTempl;

			/* update player template list */
			psPlayerTempl = apsDroidTemplates[selectedPlayer];
			psPrevTempl = NULL;
			while ( psPlayerTempl != NULL )
			{
				psPrevTempl = psPlayerTempl;
				psPlayerTempl = psPlayerTempl->psNext;
			}
			if ( psPrevTempl == NULL )
			{
				apsDroidTemplates[selectedPlayer] = psTempl;
			}
			else
			{
				psPrevTempl->psNext = psTempl;
			}

			/* set button render routines to highlight, not flash */
			intSetButtonFlash( IDDES_SYSTEMBUTTON, FALSE );
			intSetButtonFlash( IDDES_BODYBUTTON,   FALSE );
			intSetButtonFlash( IDDES_PROPBUTTON,   FALSE );

			/* reset all button states */
			widgSetButtonState(psWScreen, IDDES_SYSTEMBUTTON, 0);
			widgSetButtonState(psWScreen, IDDES_BODYBUTTON,   0);
			widgSetButtonState(psWScreen, IDDES_PROPBUTTON,   0);
		}
		else
		{
			/* Get existing template */
			psTempl = apsTemplateList[i];
			newTemplate = FALSE;
            /*ANY change to the template affect the production - even if the 
            template is changed and then changed back again!*/
            deleteTemplateFromProduction(psTempl, (UBYTE)selectedPlayer);
		}

		/* Copy the template */
		memcpy(psTempl, &sCurrDesign, sizeof(DROID_TEMPLATE));
#ifdef HASH_NAMES
			// NameHash already copied
#else
		strncpy(psTempl->aName, aCurrName, DROID_MAXNAME);
		psTempl->aName[DROID_MAXNAME - 1] = 0;
#endif

		/* Now update the droid template form */
		widgDelete(psWScreen, IDDES_TEMPLFORM);
		widgDelete(psWScreen, IDDES_TEMPLBASE);
		intAddTemplateForm(psTempl);
		stored = TRUE;
	}

#ifdef WIN32
	if (stored)
	{
		psTempl->multiPlayerID = (objID<<3)|selectedPlayer;
		objID++; 
		if (bMultiPlayer)
		{
			sendTemplate(psTempl);
		}
	}
#endif

	return stored;
}


#ifdef PSX
void SetDesignWidgetName(char *Name)
{

//	widgSetString(psWScreen, IDDES_NAMEBOX, Name);
//	strncpy(sCurrDesign.pName, Name, WIDG_MAXSTR-1);
}
#endif

/*Function to set the shadow bars for all the stats when the mouse is over 
the Template buttons*/
void runTemplateShadowStats(UDWORD id)
{
	UDWORD			currID;
	DROID_TEMPLATE	*psTempl = NULL;
	COMP_BASE_STATS	*psStats;
	DROID_TYPE		templType;
	UDWORD			i;

	/* Find the template for the new button */
	//currID = IDDES_TEMPLSTART;
    //we're ignoring the Blank Design so start at the second button
    currID = IDDES_TEMPLSTART + 1;
	for( i=1; i<MAXTEMPLATES; i++ )
    {
		psTempl = apsTemplateList[i];
		if (currID == id)
		{
			break;
		}
		currID ++;
	}

	//if we're over a different template
	if (psTempl AND psTempl != &sCurrDesign)
	{
		/* Now set the bar graphs for the stats */
		intSetBodyShadowStats(asBodyStats + psTempl->asParts[COMP_BODY]);
		intSetPropulsionShadowStats(asPropulsionStats + psTempl->asParts[COMP_PROPULSION]);
		//only set the system shadow bar if the same type of droid
		psStats = NULL;
		templType = droidTemplateType(psTempl);
		if (templType == droidTemplateType(&sCurrDesign))
		{
			switch (templType)
			{
			case DROID_WEAPON:
				psStats = (COMP_BASE_STATS *)(asWeaponStats + psTempl->
					asWeaps[0]);
				break;
			case DROID_SENSOR:
				psStats = (COMP_BASE_STATS *)(asSensorStats + psTempl->
					asParts[COMP_SENSOR]);
				break;
			case DROID_ECM:
				psStats = (COMP_BASE_STATS *)(asECMStats + psTempl->
					asParts[COMP_ECM]);
				break;
			case DROID_CONSTRUCT:
				psStats = (COMP_BASE_STATS *)(asConstructStats + psTempl->
					asParts[COMP_CONSTRUCT]);
				break;
			case DROID_REPAIR:
				psStats = (COMP_BASE_STATS *)(asRepairStats + psTempl->
					asParts[COMP_REPAIRUNIT]);
				break;
			//default:
				//not interested in other types!
			}
		}

		if (psStats)
		{
			intSetSystemShadowStats(psStats);
		}
		//set the template shadow stats
		//intSetTemplateBodyShadowStats(psStats);
		//haven't got a stat so just do the code required here...
		widgSetMinorBarSize( psWScreen, IDDES_BODYPOINTS,
								calcTemplateBody(psTempl, (UBYTE)selectedPlayer) );

		//intSetTemplatePowerShadowStats(psStats);
		widgSetMinorBarSize( psWScreen, IDDES_POWERBAR,
								calcTemplatePower(psTempl) );
	}
}

/*sets which states need to be paused when the design screen is up*/
void setDesignPauseState(void)
{

#ifdef WIN32
	if (!bMultiPlayer)
	{
#endif

		gameTimeStop();
		setGameUpdatePause(TRUE);
		setScrollPause(TRUE);

#ifdef WIN32
	}
#endif

}

/*resets the pause states */
void resetDesignPauseState(void)
{
#ifdef WIN32
	if (!bMultiPlayer)
	{
#endif

		setGameUpdatePause(FALSE);
		setScrollPause(FALSE);
		gameTimeStart();

#ifdef WIN32
	}
#endif
}

/*this is called when a new propulsion type is added to the current design
to check the weapon is 'allowed'. Check if VTOL, the weapon is direct fire. 
Also check numVTOLattackRuns for the weapon is not zero - return TRUE if valid weapon*/
static BOOL intCheckValidWeaponForProp(void)
{
    return checkValidWeaponForProp(&sCurrDesign);
}

BOOL intAddDesign( BOOL bShowCentreScreen )
{
#ifdef PSX
	// If the stacks in the dcache then..
	if(SpInDCache()) {
		static BOOL ret;
		static BOOL _bShowCentreScreen;

		_bShowCentreScreen = bShowCentreScreen;

		// Set the stack pointer to point to the alternative stack which is'nt limited to 1k.
		SetSpAlt();
		ret = _intAddDesign(_bShowCentreScreen);
		SetSpAltNormal();

		return ret;
	}
#endif
	return _intAddDesign(bShowCentreScreen);
}


/* Set up the system clickable form of the design screen given a set of stats */
static BOOL intSetSystemForm(COMP_BASE_STATS *psStats)
{
#ifdef PSX
	// If the stacks in the dcache then..
	if(SpInDCache()) {
		static BOOL ret;
		static COMP_BASE_STATS *_psStats;

		_psStats = psStats;

		// Set the stack pointer to point to the alternative stack which is'nt limited to 1k.
		SetSpAlt();
		ret = _intSetSystemForm(_psStats);
		SetSpAltNormal();

		return ret;
	}
#endif
	return _intSetSystemForm(psStats);
}


static BOOL intAddTemplateForm(DROID_TEMPLATE *psSelected)
{
#ifdef PSX
	// If the stacks in the dcache then..
	if(SpInDCache()) {
		static BOOL ret;
		static DROID_TEMPLATE *_psSelected;

		_psSelected = psSelected;

		// Set the stack pointer to point to the alternative stack which is'nt limited to 1k.
		SetSpAlt();
		ret = _intAddTemplateForm(_psSelected);
		SetSpAltNormal();

		return ret;
	}
#endif
	return _intAddTemplateForm(psSelected);
}

//checks if the template has LIFT propulsion attached - returns TRUE if it does
BOOL checkTemplateIsVtol(DROID_TEMPLATE *psTemplate)
{
    if (asPropulsionStats[psTemplate->asParts[COMP_PROPULSION]].
        propulsionType == LIFT)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*goes thru' the list passed in reversing the order so the first entry becomes 
the last and the last entry becomes the first!*/
void reverseTemplateList(DROID_TEMPLATE **ppsList)
{
    DROID_TEMPLATE     *psPrev, *psNext, *psCurrent, *psObjList;

    //initialise the pointers
    psObjList = *ppsList;
    psPrev = psNext = NULL;
    psCurrent = psObjList;

    while(psCurrent != NULL)
    {
        psNext = psCurrent->psNext;
        psCurrent->psNext = psPrev;
        psPrev = psCurrent;
        psCurrent = psNext;
    }
    //set the list passed in to point to the new top
    *ppsList = psPrev;
}

//calculates the weapons ROF based on the fire pause and the salvos
static UWORD weaponROF(WEAPON_STATS *psStat)
{
    UWORD   rof = 0;

    //if there are salvos
    if (psStat->numRounds)
    {
        if (psStat->reloadTime != 0)
        {
            rof = (UWORD)(60 * GAME_TICKS_PER_SEC / psStat->reloadTime);
            //multiply by the number of salvos/shot
            rof = (UWORD)(rof * psStat->numRounds);
        }
    }
    if (!rof)
    {
        rof = (UWORD)weaponFirePause(psStat, (UBYTE)selectedPlayer);
        if (rof != 0)
        {
            rof = (UWORD)(60 * GAME_TICKS_PER_SEC / rof);
        }
        //else leave it at 0
    }
    return rof;
}