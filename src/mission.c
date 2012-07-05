/*
 * mission.c
 *
 * all the stuff relevant to a mission
 */
#include "stdio.h"
#include "mission.h"
#include "GTime.h"
#include "game.h"
#include "Projectile.h"
#include "Power.h"
#include "Structure.h"
#include "message.h"
#include "Research.h"
#include "HCI.h"
#include "Player.h"
#include "Order.h"
#include "Action.h"
#include "display3D.h"
#include "pieState.h"
#include "Effects.h"
#include "radar.h"
#include "resource.h"		// for mousecursors
#include "Transporter.h"
#include "Group.h"
#include "text.h"
#include "frontend.h"		// for displaytextoption.
#include "csnap.h"			// cursor snapping
#include "intDisplay.h"
#include "display.h"
#include "loadsave.h"
#include "Script.h"
#include "ScriptTabs.h"
#include "audio.h"
#include "audio_id.h"
#include "CmdDroid.h"
#include "WarCam.h"
#include "Wrappers.h"
#include "Widget.h"
#include "WidgInt.h"
#include "Console.h"
#include "droid.h"
#ifdef WIN32
#include "data.h"
#include "multiplay.h"
#include "rendmode.h"		// for downloadbuffer
#include "pieFunc.h"
#include "pieBlitFunc.h"
#include "environ.h"
#endif
#include "Loop.h"
#include "Levels.h"
#include "visibility.h"
#include "MapGrid.h"
#include "Cluster.h"
#include "Gateway.h"
#include "Selection.h"
#include "Scores.h"
#include "KeyMap.h"

#ifdef WIN32
#include "cdspan.h"
#include "cdaudio.h"
#endif
//#include "texture.h"	   // ffs 
#ifdef WIN32
#include "Texture.h"
#endif
#ifdef PSX
#include "rendmode.h"
#include "csnap.h"
#include "initpsx.h"
#include "Primatives.h"
#include "DCache.h"
#include "VPad.h"
#include "CtrlPSX.h"
//extern CURSORSNAP InterfaceSnap;
#endif
extern CURSORSNAP InterfaceSnap;
//DEFINES**************
//#define		IDTIMER_FORM			11000		// has to be in the header..boohoo
//#define		IDTIMER_DISPLAY			11001
//#define		IDMISSIONRES_FORM		11002		// has to be in the header..boohoo

#define		IDMISSIONRES_TXT		11004
#define     IDMISSIONRES_LOAD		11005
//#define     IDMISSIONRES_SAVE		11006
//#define     IDMISSIONRES_QUIT		11007			//has to be in the header. bummer.
#define     IDMISSIONRES_CONTINUE	11008

//#define		IDTRANSTIMER_FORM		11009		//form for transporter timer
//#define		IDTRANTIMER_DISPLAY		11010		//timer display for transporter timer
//#define		IDTRANTIMER_BUTTON		11012		//transporter button on timer display

#define		IDMISSIONRES_BACKFORM	11013
#define		IDMISSIONRES_TITLE		11014

/*mission timer position*/
#ifdef WIN32

#define		TIMER_X					(568 + E_W)
//#define		TIMER_Y					22
//#define		TIMER_WIDTH				38
//#define		TIMER_HEIGHT			20
#define		TIMER_LABELX			15
#define		TIMER_LABELY			0
/*Transporter Timer form position */
#define		TRAN_FORM_X				STAT_X
#define		TRAN_FORM_Y				TIMER_Y
#define		TRAN_FORM_WIDTH			75
#define		TRAN_FORM_HEIGHT		25

#else
// Mission timer.
#define		TIMER_X					(320+32)
#define		TIMER_Y					(20+32)
#define		TIMER_WIDTH				56
#define		TIMER_HEIGHT			20
#define		TIMER_LABELX			15
#define		TIMER_LABELY			0
// Transporter loading.
#define		TRAN_FORM_X				(320-64)
#define		TRAN_FORM_Y				(20+32)
#define		TRAN_FORM_WIDTH			84
#define		TRAN_FORM_HEIGHT		25

#endif

/*Transporter Timer position */
#define		TRAN_TIMER_X			4
#define		TRAN_TIMER_Y			TIMER_LABELY
#define		TRAN_TIMER_WIDTH		25
//#define		TRAN_TIMER_HEIGHT		TRAN_FORM_HEIGHT
/*Transporter Timer button position */
//#define		TRAN_BUTTON_X			(TRAN_TIMER_X + TRAN_TIMER_WIDTH + TRAN_TIMER_X)
//#define		TRAN_BUTTON_Y			TRAN_TIMER_Y
//#define		TRAN_BUTTON_WIDTH		20
//#define		TRAN_BUTTON_HEIGHT		20



#ifdef WIN32

#define		MISSION_1_X				5						// pos of text options in box.
#define		MISSION_1_Y				15
#define		MISSION_2_X				5
#define		MISSION_2_Y				35
#define		MISSION_3_X				5
#define		MISSION_3_Y				55

#define		MISSION_TEXT_W			MISSIONRES_W-10
#define		MISSION_TEXT_H			16

#else	// PSX version

#define		MISSION_1_X				5						// pos of text options in box.
#define		MISSION_1_Y				(15+8)
#ifdef COVERMOUNT
 #define		MISSION_2_X				5
 #define		MISSION_2_Y				(MISSION_1_Y+16)
#else
 #define		MISSION_2_X				5
 #define		MISSION_2_Y				(MISSION_1_Y+48)
#endif
#define		MISSION_3_X				5
#define		MISSION_3_Y				(MISSION_1_Y+48*2)
#define		MISSION_TEXT_W			MISSIONRES_W-10
#define		MISSION_TEXT_H			16

#endif

//these are used for mission countdown
#ifdef WIN32
#define TEN_MINUTES         (10 * 60 * GAME_TICKS_PER_SEC)
#define FIVE_MINUTES        (5 * 60 * GAME_TICKS_PER_SEC)
#define FOUR_MINUTES        (4 * 60 * GAME_TICKS_PER_SEC)
#define THREE_MINUTES       (3 * 60 * GAME_TICKS_PER_SEC)
#define TWO_MINUTES         (2 * 60 * GAME_TICKS_PER_SEC)
#define ONE_MINUTE          (1 * 60 * GAME_TICKS_PER_SEC)
#define NOT_PLAYED_ONE          0x01
#define NOT_PLAYED_TWO          0x02
#define NOT_PLAYED_THREE        0x04
#define NOT_PLAYED_FIVE         0x08
#define NOT_PLAYED_TEN          0x10
#define NOT_PLAYED_ACTIVATED    0x20
#endif

//EXTERNALS*************
MISSION		mission;

BOOL		offWorldKeepLists;

// Set by scrFlyInTransporter. True if were currenly tracking the transporter.
BOOL		bTrackingTransporter = FALSE;

/*lists of droids that are held seperate over several missions. There should 
only be selectedPlayer's droids but have possibility for MAX_PLAYERS - 
also saves writing out list functions to cater for just one player*/
DROID       *apsLimboDroids[MAX_PLAYERS];

#ifdef PSX
UDWORD getLevelLoadFlags(void);
BOOL	bDisplayScores;
static BOOL BackdropActive = FALSE;
#endif

/**********TEST************/
static  UDWORD      addCount = 0;

//#ifdef COVERMOUNT
//BOOL		DemoStart;
//BOOL		DemoExpand;
//#endif

//STATICS***************
//Where the Transporter lands for player 0 (sLandingZone[0]), and the rest are 
//a list of areas that cannot be built on, used for landing the enemy transporters
static LANDING_ZONE		sLandingZone[MAX_NOGO_AREAS];

//flag to indicate when the droids in a Transporter are flown to safety and not the next mission
static BOOL             bDroidsToSafety;

/* mission result holder */
static BOOL				g_bMissionResult;

// return positions for vtols
POINT	asVTOLReturnPos[MAX_PLAYERS];

#ifdef WIN32
static UBYTE   missionCountDown;
//flag to indicate whether the coded mission countdown is played
static UBYTE   bPlayCountDown;
#endif

//FUNCTIONS**************
static void addLandingLights( UDWORD x, UDWORD y);
static BOOL startMissionOffClear(STRING *pGame);
static BOOL startMissionOffKeep(STRING *pGame);
static BOOL startMissionCampaignStart(STRING *pGame);
static BOOL startMissionCampaignChange(STRING *pGame);
static BOOL startMissionCampaignExpand(STRING *pGame);
static BOOL startMissionCampaignExpandLimbo(STRING *pGame);
static BOOL startMissionBetween(void);
static void endMissionCamChange(void);
static void endMissionOffClear(void);
static void endMissionOffKeep(void);
static void endMissionOffKeepLimbo(void);
static void endMissionExpandLimbo(void);

static void saveMissionData(void);
static void restoreMissionData(void);
static void saveCampaignData(void);
static void aiUpdateMissionStructure(STRUCTURE *psStructure);
static void missionResetDroids();
static void saveMissionLimboData(void);
static void restoreMissionLimboData(void);
static void processMissionLimbo(void);

static void intUpdateMissionTimer(struct _widget *psWidget, struct _w_context *psContext);
static BOOL intAddMissionTimer(void);
static void intUpdateTransporterTimer(struct _widget *psWidget, struct _w_context *psContext);
static void adjustMissionPower(void);
static void saveMissionPower(void);
static UDWORD getHomeLandingX(void);
static UDWORD getHomeLandingY(void);
void swapMissionPointers(void);
static void fillTimeDisplay(STRING	*psText, UDWORD time, BOOL bHours);
static void processPreviousCampDroids(void);
static BOOL intAddTransporterTimer(void);
static void clearCampaignUnits(void);

static void emptyTransporters(BOOL bOffWorld);

//result screen functions
BOOL intAddMissionResult(BOOL result, BOOL bPlaySucess);
//void intRemoveMissionResult			(void);
//void intRemoveMissionResultNoAnim	(void);
//void intRunMissionResult			(void);
//void intProcessMissionResult		(UDWORD id);

BOOL MissionResUp		= FALSE;
BOOL ClosingMissionRes	= FALSE;

static SDWORD		g_iReinforceTime = 0;
static DROID_GROUP	*g_CurrentScriptGroup = NULL;

/* Which campaign are we dealing with? */
static	UDWORD	camNumber = 1;

//static iSprite *pMissionBackDrop; //pointer to backdrop piccy

#ifdef PSX

void HideMissionTimer(void)
{
	if(widgGetFromID(psWScreen,IDTIMER_FORM)) {
		widgHide(psWScreen,IDTIMER_FORM);
	}
}


void RevealMissionTimer(void)
{
	if(widgGetFromID(psWScreen,IDTIMER_FORM)) {
		widgReveal(psWScreen,IDTIMER_FORM);
	}
}

#endif



//returns TRUE if on an off world mission
BOOL missionIsOffworld(void)
{
	return ((mission.type == LDS_MKEEP)
#ifndef COVERMOUNT

			|| (mission.type == LDS_MCLEAR) 
			|| (mission.type == LDS_MKEEP_LIMBO)
#endif
			);
}

//returns TRUE if the correct type of mission for reinforcements
BOOL missionForReInforcements(void)
{
    if ( (mission.type == LDS_CAMSTART)
		OR missionIsOffworld() 
#ifndef COVERMOUNT
		OR (mission.type == LDS_CAMCHANGE) 
#endif
		)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

//returns TRUE if the correct type of mission and a reinforcement time has been set
BOOL missionCanReEnforce(void)
{
//	return missionIsOffworld() && (mission.ETA >= 0);
    if (mission.ETA >= 0)
    {
        //added CAMSTART for when load up a save game on Cam2A - AB 17/12/98
        //if (missionIsOffworld() OR (mission.type == LDS_CAMCHANGE) OR 
        //    (mission.type == LDS_CAMSTART))
        if (missionForReInforcements())
        {
            return TRUE;
        }
    }
    return FALSE;
}

//returns TRUE if the mission is a Limbo Expand mission
BOOL missionLimboExpand(void)
{
#ifndef COVERMOUNT
    if (mission.type == LDS_EXPAND_LIMBO)
    {
        return TRUE;
    }
    else
#endif
    {
        return FALSE;
    }
}


//mission initialisation game code
void initMission(void)
{
	UDWORD		inc;
	
DBPRINTF(("***Init Mission ***\n"));
	//mission.type = MISSION_NONE;
	mission.type = LDS_NONE;
	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		mission.apsStructLists[inc] = NULL;
		mission.apsDroidLists[inc] = NULL;
		mission.apsFeatureLists[inc] = NULL;
		//mission.apsProxDisp[inc] = NULL;
		mission.apsFlagPosLists[inc] = NULL;
        apsLimboDroids[inc] = NULL;
	}
	offWorldKeepLists = FALSE;
	mission.time = -1;
#ifdef WIN32			// ffs ab
    setMissionCountDown();
#endif
	mission.ETA = -1;
	mission.startTime = 0;
	mission.psGateways = NULL;
	mission.apRLEZones = NULL;
	mission.gwNumZones = 0;
	mission.mapHeight = 0;
	mission.mapWidth = 0;
	mission.aNumEquiv = NULL;
	mission.apEquivZones = NULL;
	mission.aZoneReachable = NULL;

	//init all the landing zones
	for (inc = 0; inc < MAX_NOGO_AREAS; inc++)
	{
		sLandingZone[inc].x1 = sLandingZone[inc].y1 = sLandingZone[inc].x2 = 
			sLandingZone[inc].y2 = 0;
	}

	// init the vtol return pos
	memset(asVTOLReturnPos, 0, sizeof(POINT)*MAX_PLAYERS);

    bDroidsToSafety = FALSE;

#ifdef WIN32
    setPlayCountDown(TRUE);
#endif

    //start as not cheating!
    mission.cheatTime = 0;

//#ifdef COVERMOUNT
//	DemoStart = TRUE;
//	DemoExpand = TRUE;
//#endif
}

// reset the vtol landing pos
void resetVTOLLandingPos(void)
{
	memset(asVTOLReturnPos, 0, sizeof(POINT)*MAX_PLAYERS);
}

//this is called everytime the game is quit
void releaseMission(void)
{
    /*mission.apsDroidLists may contain some droids that have been transferred 
    from one campaign to the next*/
	freeAllMissionDroids();

    /*apsLimboDroids may contain some droids that have been saved
    at the end of one mission and not yet used*/
	freeAllLimboDroids();
}

//called to shut down when mid-mission on an offWorld map
BOOL missionShutDown(void)
{
	UDWORD		inc;

	//if (mission.type == MISSION_OFFKEEP OR mission.type == MISSION_OFFCLEAR)
	if (missionIsOffworld()) //mission.type == LDS_MKEEP OR mission.type == LDS_MCLEAR)
	{
		//clear out the audio
		audio_StopAll();

		freeAllDroids();
		freeAllStructs();
		freeAllFeatures();
		releaseAllProxDisp();
		gwShutDown();
		//flag positions go with structs
//		FREE(psMapTiles);
//		mapFreeTilesAndStrips();
		//FREE(aMapLinePoints);

		for (inc = 0; inc < MAX_PLAYERS; inc++)
		{
			apsDroidLists[inc] = mission.apsDroidLists[inc];
            mission.apsDroidLists[inc] = NULL;
			apsStructLists[inc] = mission.apsStructLists[inc];
            mission.apsStructLists[inc] = NULL;
			apsFeatureLists[inc] = mission.apsFeatureLists[inc];
            mission.apsFeatureLists[inc] = NULL;
			//apsProxDisp[inc] = mission.apsProxDisp[inc];
			apsFlagPosLists[inc] = mission.apsFlagPosLists[inc];
            mission.apsFlagPosLists[inc] = NULL;
		}
		
		psMapTiles = mission.psMapTiles;
		aMapLinePoints = mission.aMapLinePoints;
		mapWidth = mission.mapWidth;
		mapHeight = mission.mapHeight;
		psGateways = mission.psGateways;
		apRLEZones = mission.apRLEZones;
		gwNumZones = mission.gwNumZones;
		aNumEquiv = mission.aNumEquiv;
		apEquivZones = mission.apEquivZones;
		aZoneReachable = mission.aZoneReachable;
	}

	// sorry if this breaks something - but it looks like it's what should happen - John
	mission.type = LDS_NONE;
	
	return TRUE;
}

#ifdef WIN32
/*on the PC - sets the countdown played flag*/
void setMissionCountDown(void)
{
	SDWORD		timeRemaining;

	timeRemaining = mission.time - (gameTime - mission.startTime);
	if (timeRemaining < 0)
	{
		timeRemaining = 0;
	}

    //need to init the countdown played each time the mission time is changed
    missionCountDown = NOT_PLAYED_ONE | NOT_PLAYED_TWO | NOT_PLAYED_THREE | 
        NOT_PLAYED_FIVE | NOT_PLAYED_TEN | NOT_PLAYED_ACTIVATED;

    if (timeRemaining < TEN_MINUTES - 1)
    {
        missionCountDown &= ~NOT_PLAYED_TEN;
    }
    if (timeRemaining < FIVE_MINUTES - 1)
    {
        missionCountDown &= ~NOT_PLAYED_FIVE;
    }
    if (timeRemaining < THREE_MINUTES - 1)
    {
        missionCountDown &= ~NOT_PLAYED_THREE;
    }
    if (timeRemaining < TWO_MINUTES - 1)
    {
        missionCountDown &= ~NOT_PLAYED_TWO;
    }
    if (timeRemaining < ONE_MINUTE - 1)
    {
        missionCountDown &= ~NOT_PLAYED_ONE;
    }
}
#endif

//BOOL startMission(MISSION_TYPE missionType, STRING *pGame)
BOOL startMission(LEVEL_TYPE missionType, STRING *pGame)
{
	BOOL	loaded = TRUE;

	/* Player has (obviously) not failed at the start */
	setPlayerHasLost(FALSE);
	setPlayerHasWon(FALSE);

    /*and win/lose video is obvioulsy not playing*/
    setScriptWinLoseVideo(PLAY_NONE);

    //HACKY HACK HACK
    //this inits the flag so that 'reinforcements have arrived' message isn't played for the first transporter load
    initFirstTransporterFlag();

//	if (missionType == LDS_CAMSTART)
//	{
		//only load up one campaign start
//		if (!DemoStart)
//		{
//			DBERROR(("Unable to load mission"));
//			return FALSE;
//		}
//		DemoStart = FALSE;
//	}
//	else if (missionType == LDS_MKEEP)
//	{
		//only load up one other mission
//		if (!DemoExpand)
//		{
//			DBERROR(("Unable to load mission"));
//			return FALSE;
//		}
//		DemoExpand = FALSE;
//	}
//	else if(missionType == LDS_BETWEEN)
//	{
//		// do nothing.
//	}
//	else
//	{ 
//		//don't want to load up any other type of mission
//		DBERROR(("Unable to load mission"));
//		return FALSE;
//	}

	//if (mission.type != MISSION_NONE)
	if (mission.type != LDS_NONE)
	{
		/*mission type gets set to none when you have returned from a mission 
		so don't want to go another mission when already on one! - so ignore*/
		DBMB(("Already on a mission"));
		return TRUE;
	}

	// reset the cluster stuff
	clustInitialise();
	initEffectsSystem();

	//load the game file for all types of mission except a Between Mission
	//if (missionType != MISSION_BETWEEN)
//#ifndef COVERMOUNT
	if (missionType != LDS_BETWEEN)
//#endif
	{
		loadGameInit(pGame,TRUE);
	}

	//all proximity messages are removed between missions now
	releaseAllProxDisp();

	switch (missionType)
	{
		case LDS_CAMSTART:
		{
			if (!startMissionCampaignStart(pGame))
			{
				loaded = FALSE;
			}
			break;
		}

		case LDS_MKEEP:
#ifndef COVERMOUNT
		case LDS_MKEEP_LIMBO:
#endif
		{
			if (!startMissionOffKeep(pGame))
			{
				loaded = FALSE;
			}
			break;
		}
		case LDS_BETWEEN:
		{
			//do anything?
			if (!startMissionBetween())
			{
				loaded = FALSE;
			}
			break;
		}
#ifndef COVERMOUNT
		case LDS_CAMCHANGE:
		{
			/*if (getCampaignNumber() == 1)
			{
				//play the cam 2 video
				seq_ClearSeqList();
			#ifdef WIN32
				seq_AddSeqToList("CAM2\\c002.rpl",NULL,"CAM2\\c002.txa",FALSE);
			#else
				seq_AddSeqToList("CAM2\\C002.STR","1656f");
			#endif
				seq_StartNextFullScreenVideo();
			}
			else
			{
				//play the cam 3 video
				seq_ClearSeqList();
			#ifdef WIN32
				seq_AddSeqToList("CAM2\\cam2out.rpl",NULL,NULL,FALSE);
				seq_AddSeqToList("CAM3\\c003.rpl",NULL,"CAM3\\c003.txa",FALSE);
			#else
				seq_AddSeqToList("CAM3\\C003.STR","1656f");
			#endif
				seq_StartNextFullScreenVideo();
			}*/

			if (!startMissionCampaignChange(pGame))
			{
				loaded = FALSE;
			}
			break;
		}
		case LDS_EXPAND:
		{
			if (!startMissionCampaignExpand(pGame))
			{
				loaded = FALSE;
			}
			break;
		}
		case LDS_EXPAND_LIMBO:
		{
			if (!startMissionCampaignExpandLimbo(pGame))
			{
				loaded = FALSE;
			}
			break;
		}
		case LDS_MCLEAR:
		{
			if (!startMissionOffClear(pGame))
			{
				loaded = FALSE;
			}
			break;
		}
	

#endif
		default:
		{
			//error!
			DBERROR(("Unknown Mission Type"));
			loaded = FALSE;
		}
	}

	if (!loaded)
	{
		DBERROR(("Unable to load mission file"));
		return FALSE;
	}

	mission.type = missionType;

    if (missionIsOffworld())
    {
		//add what power have got from the home base
		adjustMissionPower();
    }

	if (missionCanReEnforce())
	{
		//add mission timer - if necessary
		addMissionTimerInterface();

		//add Transporter Timer
		addTransporterTimerInterface();
	}

	scoreInitSystem();

    //add proximity messages for all untapped VISIBLE oil resources
    addOilResourceProximities();

	return TRUE;
}


// initialise the mission stuff for a save game
BOOL startMissionSave(SDWORD missionType)
{
	mission.type = missionType;

	return TRUE;
}


/*checks the time has been set and then adds the timer if not already on 
the display*/
void addMissionTimerInterface(void)
{
	//don't add if the timer hasn't been set
	if (mission.time < 0)
	{
		return;
	}

	//check timer is not already on the screen
	if (!widgGetFromID(psWScreen, IDTIMER_FORM))
	{
		intAddMissionTimer();
	}
}

/*checks that the timer has been set and that a Transporter exists before 
adding the timer button*/
void addTransporterTimerInterface(void)
{
	DROID	        *psDroid, *psTransporter;
    BOOL            bAddInterface = FALSE;
#ifdef WIN32
    W_CLICKFORM     *psForm;
#endif

	//check if reinforcements are allowed
	if (mission.ETA >= 0)
	{
		//check the player has at least one Transporter back at base
		psDroid = psTransporter = NULL;
		for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != 
			NULL; psDroid = psDroid->psNext)
		{
			if (psDroid->droidType == DROID_TRANSPORTER)
			{
				psTransporter = psDroid;
				break;
			}
		}
        if (psDroid)
        {
            //don't bother checking for reinforcements - always add it if you've got a Transporter
            //check the player has some reinforcements back at home
            /*psDroid = NULL;
            for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != 
                NULL; psDroid = psDroid->psNext)
            {
                if (psDroid->droidType != DROID_TRANSPORTER)
                {
                    break;
                }
            }
            //if reinforcements available
            if (psDroid)*/
            {
                bAddInterface = TRUE;
#ifdef WIN32
	    		//check timer is not already on the screen
		    	if (!widgGetFromID(psWScreen, IDTRANTIMER_BUTTON))
    			{
	    			intAddTransporterTimer();
		    	}
#else
	    		//check timer is not already on the screen
		    	if (!widgGetFromID(psWScreen, IDTIMER_FORM))
    			{
	    			intAddMissionTimer();
		    	}
#endif
				//set the data for the transporter timer
				widgSetUserData(psWScreen, IDTRANTIMER_DISPLAY, (void*)psTransporter);
#ifdef WIN32
                //lock the button if necessary
                if (transporterFlying(psTransporter))
                {
                    //disable the form so can't add any more droids into the transporter
                    psForm = (W_CLICKFORM*)widgGetFromID(psWScreen,IDTRANTIMER_BUTTON);
                    if (psForm)
                    {
                        formSetClickState(psForm, WBUT_LOCK);
                    }
                }
#endif
		    }
	    }
    }
    //if criteria not met
    if (!bAddInterface)
	{
    	//make sure its not there!
		intRemoveTransporterTimer();
	}
}

#define OFFSCREEN_RADIUS	600
#define OFFSCREEN_HEIGHT	600

/* get offscreen point */
void missionGetOffScreenPoint( UWORD iX, UWORD iY,
						UWORD *piOffX, UWORD *piOffY, UWORD *piOffZ )
{
//	UDWORD	iMapWidth  = GetWidthOfMap()  << TILE_SHIFT,
//			iMapHeight = GetHeightOfMap() << TILE_SHIFT;
	SDWORD	iTestX = ((SDWORD)iX) - OFFSCREEN_RADIUS,
			iTestY = ((SDWORD)iY) - OFFSCREEN_RADIUS;

	if ( iTestX > 0 )
	{
		*piOffX = (UWORD) iTestX;
	}
	else
	{
		*piOffX = (UWORD) (((SDWORD)iX) - OFFSCREEN_RADIUS);
	}

	if ( iTestY > 0 )
	{
		*piOffY = (UWORD) iTestY;
	}
	else
	{
		*piOffY = (UWORD) (((SDWORD)iY) - OFFSCREEN_RADIUS);
	}

	*piOffZ = (UWORD) (map_Height( iX, iY ) + OFFSCREEN_HEIGHT);
}

#define	EDGE_SIZE	1

/* pick nearest map edge to point */
void missionGetNearestCorner( UWORD iX, UWORD iY, UWORD *piOffX, UWORD *piOffY )
{
	UDWORD	iMidX = (scrollMinX + scrollMaxX)/2,
			iMidY = (scrollMinY + scrollMaxY)/2;

	if ( ((UDWORD)(iX>>TILE_SHIFT)) < iMidX )
	{
		*piOffX = (UWORD) ((scrollMinX << TILE_SHIFT) + (EDGE_SIZE*TILE_UNITS));
	}
	else
	{
		*piOffX = (UWORD) ((scrollMaxX << TILE_SHIFT) - (EDGE_SIZE*TILE_UNITS));
	}
	if ( ((UDWORD)(iY>>TILE_SHIFT)) < iMidY )
	{
		*piOffY = (UWORD) ((scrollMinY << TILE_SHIFT) + (EDGE_SIZE*TILE_UNITS));
	}
	else
	{
		*piOffY = (UWORD) ((scrollMaxY << TILE_SHIFT) - (EDGE_SIZE*TILE_UNITS));
	}
}

/* fly in transporters at start of level */
void missionFlyTransportersIn( SDWORD iPlayer, BOOL bTrackTransporter )
{
	DROID	*psTransporter, *psNext;
	UWORD	iX, iY, iZ;
	SDWORD	iLandX, iLandY, iDx, iDy;
	FRACT_D	fR;

	bTrackingTransporter = bTrackTransporter;

	iLandX = getLandingX(iPlayer);
	iLandY = getLandingY(iPlayer);
	missionGetTransporterEntry( iPlayer, &iX, &iY );
	iZ = (UWORD) (map_Height( iX, iY ) + OFFSCREEN_HEIGHT);

	psNext = NULL;
	//get the droids for the mission
	for (psTransporter = mission.apsDroidLists[iPlayer]; psTransporter != 
		NULL; psTransporter = psNext)
	{
		psNext = psTransporter->psNext;
		if (psTransporter->droidType == DROID_TRANSPORTER)
		{
            //check that this transporter actually contains some droids
            if (psTransporter->psGroup AND psTransporter->psGroup->refCount > 1)
            {
			    //remove out of stored list and add to current Droid list
                if (droidRemove(psTransporter, mission.apsDroidLists))
                {
                    //don't want to add it unless managed to remove it from the previous list
			        addDroid(psTransporter, apsDroidLists);
                }
                
			    /* set start position */
			    psTransporter->x = iX;
			    psTransporter->y = iY;
			    psTransporter->z = iZ;
                
			    /* set start direction */
			    iDx = iLandX - iX;
			    iDy = iLandY - iY;
                
#ifdef WIN32    
			    fR = (FRACT_D) atan2(iDx, iDy);
			    if ( fR < 0.0 )
			    {
			    	fR += (FRACT_D) (2*PI);
			    }
			    psTransporter->direction = (UWORD)( RAD_TO_DEG(fR) );
#else           
			    {
			    	SWORD direction;
			    	direction = angle_PSX2World(ratan2(iDx, iDy));
			    	if (direction < 0) direction+=360;
			    	psTransporter->direction = (UWORD) direction;
			    }
#endif          
                
				// Camera track requested and it's the selected player.
			    if ( ( bTrackTransporter == TRUE ) && (iPlayer == (SDWORD)selectedPlayer) )
			    {
			    	/* deselect all droids */
			    	selDroidDeselect( selectedPlayer );
                
    		    	if ( getWarCamStatus() )
		        	{
			        	camToggleStatus();
			        }
                
	    	    	/* select transporter */
		        	psTransporter->selected = TRUE;
			        camToggleStatus();
			    }
                
                //little hack to ensure all Transporters are fully repaired by time enter world
                psTransporter->body = psTransporter->originalBody;
                
			    /* set fly-in order */
			    orderDroidLoc( psTransporter, DORDER_TRANSPORTIN,
			    				iLandX, iLandY );
#ifdef WIN32
				audio_PlayObjDynamicTrack( psTransporter, ID_SOUND_BLIMP_FLIGHT,
									moveCheckDroidMovingAndVisible );
#endif
                //only want to fly one transporter in at a time now - AB 14/01/99
                break;
            }
		}
	}
}

/*Saves the necessary data when moving from a home base Mission to an OffWorld
mission*/
void saveMissionData(void)
{
	UDWORD			inc;
	DROID			*psDroid;
	STRUCTURE		*psStruct, *psStructBeingBuilt;
	BOOL			bRepairExists;

	//clear out the audio
	audio_StopAll();

	//save the mission data
	mission.psMapTiles = psMapTiles;
	mission.aMapLinePoints = aMapLinePoints;
	mission.mapWidth = mapWidth;
	mission.mapHeight = mapHeight;
	mission.scrollMinX = scrollMinX;
	mission.scrollMinY = scrollMinY;
	mission.scrollMaxX = scrollMaxX;
	mission.scrollMaxY = scrollMaxY;
	mission.psGateways = psGateways;
	psGateways = NULL;
	mission.apRLEZones = apRLEZones;
	apRLEZones = NULL;
	mission.gwNumZones = gwNumZones;
	gwNumZones = 0;
	mission.aNumEquiv = aNumEquiv;
	aNumEquiv = NULL;
	mission.apEquivZones = apEquivZones;
	apEquivZones = NULL;
	mission.aZoneReachable = aZoneReachable;
	aZoneReachable = NULL;
    //save the selectedPlayer's LZ
    mission.homeLZ_X = getLandingX(selectedPlayer);
    mission.homeLZ_Y = getLandingY(selectedPlayer);

	bRepairExists = FALSE;
	//set any structures currently being built to completed for the selected player
	for (psStruct = apsStructLists[selectedPlayer]; psStruct != NULL; psStruct =
		psStruct->psNext)
	{
		if (psStruct->status == SS_BEING_BUILT)
		{
			//find a droid working on it
			for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; 
				psDroid = psDroid->psNext)
			{
				if (orderStateObj(psDroid, DORDER_BUILD, (BASE_OBJECT **)&psStructBeingBuilt))
				{
					if (psStructBeingBuilt == psStruct)
					{
                        //check there is enough power to complete
                        //inc = psStruct->pStructureType->powerToBuild - psStruct->currentPowerAccrued;
                        inc = structPowerToBuild(psStruct) - psStruct->currentPowerAccrued;
                        if (inc > 0)
                        {
                            //not accrued enough power, so check if there is enough available
                            if (checkPower(selectedPlayer, inc, FALSE))
                            {
                                //enough - so use it and set to complete
                                usePower(selectedPlayer, inc);
                                buildingComplete(psStruct);
                            }
                        }
                        else
                        {
                            //enough power or more than enough! - either way, set to complete
						    buildingComplete(psStruct);
                        }
						//don't bother looking for any other droids working on it
						break;
					}
				}
			}
		}
		//check if have a completed repair facility on home world
		if (psStruct->pStructureType->type == REF_REPAIR_FACILITY AND
			psStruct->status == SS_BUILT)
		{
			bRepairExists = TRUE;
		}
	}

	//repair all droids back at home base if have a repair facility
	if (bRepairExists)
	{
		for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; 
			psDroid = psDroid->psNext)
		{
			if (droidIsDamaged(psDroid))
			{
				psDroid->body = psDroid->originalBody;
			}
		}
	}

	//clear droid orders for all droids except constructors still building
	for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; 
			psDroid = psDroid->psNext)
	{
		if (orderStateObj(psDroid, DORDER_BUILD, (BASE_OBJECT **)&psStructBeingBuilt))
		{
			if (psStructBeingBuilt->status == SS_BUILT)
			{
				orderDroid(psDroid, DORDER_STOP);
			}
		}
		else 
		{
			orderDroid(psDroid, DORDER_STOP);
		}
	}
    
	//THIS HAPPENS AT THE END OF THE CAMCHANGE MISSION NOW - AB 22/12/98
    //before copy the pointers over check selectedPlayer's mission.droids since 
    //there might be some from the previous camapign
    //processPreviousCampDroids();


	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		mission.apsStructLists[inc] = apsStructLists[inc];
		mission.apsDroidLists[inc] = apsDroidLists[inc];
		mission.apsFeatureLists[inc] = apsFeatureLists[inc];
		//mission.apsProxDisp[inc] = apsProxDisp[inc];
		mission.apsFlagPosLists[inc] = apsFlagPosLists[inc];
	}

	mission.playerX = player.p.x;
	mission.playerY = player.p.z;

	//save the power settings
	saveMissionPower();

	//reset before loading in the new game
	//resetFactoryNumFlag();

	//init before loading in the new game
	initFactoryNumFlag();

	//clear all the effects from the map
	initEffectsSystem();

	resetRadarRedraw();

#ifdef PSX
	intInitObjectCycle();

	// Ensure drive GUI disabled and control enabled.
	driveEnableControl();
	driveDisableInterface();
#endif
}

/*
	This routine frees the memory for the offworld mission map (in the call to mapShutdown)

	- so when this routine is called we must still be set to the offworld map data 
	i.e. We shoudn't have called SwapMissionPointers()

*/
void restoreMissionData(void)
{
	UDWORD		inc;
	BASE_OBJECT	*psObj;

	//clear out the audio
	audio_StopAll();

	//clear all the lists
	//clearPlayerPower();
	proj_FreeAllProjectiles();
	freeAllDroids();
	freeAllStructs();
	freeAllFeatures();
	gwShutDown();
	mapShutdown();
//	FREE(psMapTiles);
//	mapFreeTilesAndStrips();
	//FREE(aMapLinePoints);
	//releaseAllProxDisp();
	//flag positions go with structs

	//restore the game pointers
	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		apsDroidLists[inc] = mission.apsDroidLists[inc];
		mission.apsDroidLists[inc] = NULL;
		for(psObj = (BASE_OBJECT *)apsDroidLists[inc]; psObj; psObj=psObj->psNext)
		{
			gridAddObject(psObj);
            //make sure the died flag is not set
            psObj->died = FALSE;
		}

		apsStructLists[inc] = mission.apsStructLists[inc];
		mission.apsStructLists[inc] = NULL;
		for(psObj = (BASE_OBJECT *)apsStructLists[inc]; psObj; psObj=psObj->psNext)
		{
			gridAddObject(psObj);
		}

		apsFeatureLists[inc] = mission.apsFeatureLists[inc];
		mission.apsFeatureLists[inc] = NULL;
		for(psObj = (BASE_OBJECT *)apsFeatureLists[inc]; psObj; psObj=psObj->psNext)
		{
			gridAddObject(psObj);
		}

		apsFlagPosLists[inc] = mission.apsFlagPosLists[inc];
		mission.apsFlagPosLists[inc] = NULL;
		//apsProxDisp[inc] = mission.apsProxDisp[inc];
		//mission.apsProxDisp[inc] = NULL;
		//asPower[inc]->usedPower = mission.usedPower[inc];
		//init the next structure to be powered
		asPower[inc]->psLastPowered = NULL;
	}
	//swap mission data over

#ifdef PSX
	// if we are at the end of a campaign we don't want to restore anything !
	if((getLevelLoadFlags() & LDF_CAMEND) == 0)
	{ 

		psMapTiles = mission.psMapTiles;
	}
#else
	psMapTiles = mission.psMapTiles;
#endif
	aMapLinePoints = mission.aMapLinePoints;
	mapWidth = mission.mapWidth;
	mapHeight = mission.mapHeight;
	scrollMinX = mission.scrollMinX;
	scrollMinY = mission.scrollMinY;
	scrollMaxX = mission.scrollMaxX;
	scrollMaxY = mission.scrollMaxY;
	psGateways = mission.psGateways;
	apRLEZones = mission.apRLEZones;
	gwNumZones = mission.gwNumZones;
	aNumEquiv = mission.aNumEquiv;
	apEquivZones = mission.apEquivZones;
	aZoneReachable = mission.aZoneReachable;
	//and clear the mission pointers
	mission.psMapTiles	= NULL;
	mission.aMapLinePoints = NULL;
	mission.mapWidth	= 0;
	mission.mapHeight	= 0;
	mission.scrollMinX	= 0;
	mission.scrollMinY	= 0;
	mission.scrollMaxX	= 0;
	mission.scrollMaxY	= 0;
	mission.psGateways	= NULL;
	mission.apRLEZones	= NULL;
	mission.gwNumZones	= 0;
	mission.aNumEquiv	= NULL;
	mission.apEquivZones = NULL;
	mission.aZoneReachable = NULL;

	//reset the current structure lists
	setCurrentStructQuantity(FALSE);

	//initPlayerPower();

	initFactoryNumFlag();
	resetFactoryNumFlag();

	//terrain types? - hopefully not! otherwise we have to load in the terrain texture pages.
	
	//reset the game time
	//gameTimeReset(mission.startTime);

	offWorldKeepLists = FALSE;

	resetRadarRedraw();

#ifdef WIN32
    //reset the environ map back to the homebase settings
    environReset();
#endif

	//intSetMapPos(mission.playerX, mission.playerY);
}

/*Saves the necessary data when moving from one mission to a limbo expand Mission*/
void saveMissionLimboData(void)
{
    DROID           *psDroid, *psNext;
	//UDWORD			droidX, droidY;
	//PICKTILE		pickRes;
    STRUCTURE           *psStruct;

	//clear out the audio
	audio_StopAll();

    //before copy the pointers over check selectedPlayer's mission.droids since 
    //there might be some from the previous camapign
    processPreviousCampDroids();

    //only need to keep the selectedPlayer's droid's separate
	//mission.apsDroidLists[selectedPlayer] = apsDroidLists[selectedPlayer];
    //apsDroidLists[selectedPlayer] = NULL;
    //move droids properly - does all the clean up code
    for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = psNext)
    {
        psNext = psDroid->psNext;
        if (droidRemove(psDroid, apsDroidLists))
        {
    		addDroid(psDroid, mission.apsDroidLists);
        }
    }
    apsDroidLists[selectedPlayer] = NULL;
    
    //this is happening in a separate function now so can be called once the mission has started
    /*apsDroidLists[selectedPlayer] = apsLimboDroids[selectedPlayer];
    apsLimboDroids[selectedPlayer] = NULL;

    //set up location for each of the droids
    for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = 
        psDroid->psNext)
    {
		droidX = getLandingX(LIMBO_LANDING) >> TILE_SHIFT;
		droidY = getLandingY(LIMBO_LANDING) >> TILE_SHIFT;
		pickRes = pickHalfATile(&droidX, &droidY,LOOK_FOR_EMPTY_TILE);
		if (pickRes == NO_FREE_TILE )
		{
			ASSERT((FALSE, "saveMissionLimboData: Unable to find a free location"));
		}
		psDroid->x = (UWORD)(droidX << TILE_SHIFT);
		psDroid->y = (UWORD)(droidY << TILE_SHIFT);
		if (pickRes == HALF_FREE_TILE )
		{
			psDroid->x += TILE_UNITS;
			psDroid->y += TILE_UNITS;
		}
		psDroid->z = map_Height(psDroid->x, psDroid->y);
		updateDroidOrientation(psDroid);
		//psDroid->lastTile = mapTile(psDroid->x >> TILE_SHIFT, 
		//	psDroid->y >> TILE_SHIFT);
		psDroid->selected = FALSE;
        //this is mainly for VTOLs
        psDroid->psBaseStruct = NULL;
		psDroid->cluster = 0;
		//initialise the movement data
		initDroidMovement(psDroid);
        //make sure the died flag is not set
        psDroid->died = FALSE;

    }*/

    //any selectedPlayer's factories/research need to be put on holdProduction/holdresearch
    for (psStruct = apsStructLists[selectedPlayer]; psStruct != NULL; psStruct = 
        psStruct->psNext)
    {
        if (StructIsFactory(psStruct))
        {
            holdProduction(psStruct);
        }
        else if (psStruct->pStructureType->type == REF_RESEARCH)
        {
            holdResearch(psStruct);
        }
    }

#ifdef PSX
	intInitObjectCycle();

	// Ensure drive GUI disabled and control enabled.
	driveEnableControl();
	driveDisableInterface();
#endif
}

//this is called via a script function to place the Limbo droids once the mission has started
void placeLimboDroids(void)
{
    DROID           *psDroid, *psNext;
	UDWORD			droidX, droidY;
	PICKTILE		pickRes;

    //copy the droids across for the selected Player
    for (psDroid = apsLimboDroids[selectedPlayer]; psDroid != NULL; 
        psDroid = psNext)
    {
        psNext = psDroid->psNext;
        if (droidRemove(psDroid, apsLimboDroids))
        {
    		addDroid(psDroid, apsDroidLists);
    		//KILL OFF TRANSPORTER - should never be one but....
	    	if (psDroid->droidType == DROID_TRANSPORTER)
		    {
			    vanishDroid(psDroid);
                continue;
		    }
            //set up location for each of the droids
	    	droidX = getLandingX(LIMBO_LANDING) >> TILE_SHIFT;
		    droidY = getLandingY(LIMBO_LANDING) >> TILE_SHIFT;
		    pickRes = pickHalfATile(&droidX, &droidY,LOOK_FOR_EMPTY_TILE);
		    if (pickRes == NO_FREE_TILE )
		    {
			    ASSERT((FALSE, "placeLimboUnits: Unable to find a free location"));
		    }
		    psDroid->x = (UWORD)(droidX << TILE_SHIFT);
		    psDroid->y = (UWORD)(droidY << TILE_SHIFT);
		    if (pickRes == HALF_FREE_TILE )
		    {
			    psDroid->x += TILE_UNITS;
			    psDroid->y += TILE_UNITS;
		    }
		    psDroid->z = map_Height(psDroid->x, psDroid->y);
		    updateDroidOrientation(psDroid);
		    //psDroid->lastTile = mapTile(psDroid->x >> TILE_SHIFT, 
		    //	psDroid->y >> TILE_SHIFT);
		    psDroid->selected = FALSE;
            //this is mainly for VTOLs
            psDroid->psBaseStruct = NULL;
		    psDroid->cluster = 0;
		    //initialise the movement data
		    initDroidMovement(psDroid);
            //make sure the died flag is not set
            psDroid->died = FALSE;
        }
        else
        {
            ASSERT((FALSE, "placeLimboUnits: Unable to remove unit from Limbo list"));
        }
    }
}

/*restores the necessary data on completion of a Limbo Expand mission*/
void restoreMissionLimboData(void)
{
    DROID           *psDroid, *psNext;

    /*the droids stored in the mission droid list need to be added back 
    into the current droid list*/
    for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != NULL; 
        psDroid = psNext)
    {
        psNext = psDroid->psNext;
		//remove out of stored list and add to current Droid list
		if (droidRemove(psDroid, mission.apsDroidLists))
        {
    		addDroid(psDroid, apsDroidLists);
	    	psDroid->cluster = 0;
            gridAddObject((BASE_OBJECT *)psDroid);
    		//reset droid orders
	    	orderDroid(psDroid, DORDER_STOP);
            //the location of the droid should be valid!
        }
    }
    ASSERT((mission.apsDroidLists[selectedPlayer] == NULL,
        "restoreMissionLimboData: list should be empty")); 
}

/*Saves the necessary data when moving from one campaign to the start of the 
next - saves out the list of droids for the selected player*/
void saveCampaignData(void)
{
	UBYTE		inc;
	DROID		*psDroid, *psNext, *psSafeDroid, *psNextSafe, *psCurr, *psCurrNext;

    //if the droids have been moved to safety then get any Transporters that exist
    if (getDroidsToSafetyFlag())
    {
        //move any Transporters into the mission list
		psDroid = apsDroidLists[selectedPlayer];
		while(psDroid != NULL) 
		{
            psNext = psDroid->psNext;
            if (psDroid->droidType == DROID_TRANSPORTER)
            {
                /*Now - we want to empty the transporter and make it turn up 
                with the first ten out of the previous mission*/

                //we want to make sure they are full
                /*if (calcRemainingCapacity(psDroid))
                //before we move the droid into the mission list - check to see if it's empty
                //if (psDroid->psGroup AND psDroid->psGroup->refCount == 1)
                {
                    //fill it with droids from the mission list
                    for (psSafeDroid = mission.apsDroidLists[selectedPlayer]; 
                        psSafeDroid != NULL; psSafeDroid = psNextSafe)
                    {
                        psNextSafe = psSafeDroid->psNext;
                        //add to the Transporter, checking for when full
	                    if (checkTransporterSpace(psDroid, psSafeDroid))
	                    {
		                    if (droidRemove(psSafeDroid, mission.apsDroidLists))
                            {
    	                        grpJoin(psDroid->psGroup, psSafeDroid);
                            }
                        }
                        else
                        {
                            //setting this will cause the loop to end
                            psNextSafe = NULL;
                        }
                    }
                }*/
                //empty the transporter into the mission list
                ASSERT((psDroid->psGroup != NULL, 
                    "saveCampaignData: Transporter does not have a group"));

        		for (psCurr = psDroid->psGroup->psList; psCurr != NULL AND psCurr != 
                    psDroid; psCurr = psCurrNext)
		        {
			        psCurrNext = psCurr->psGrpNext;
                    //remove it from the transporter group
                    grpLeave( psDroid->psGroup, psCurr);
 					//cam change add droid
					psCurr->x = INVALID_XY;
					psCurr->y = INVALID_XY;
                   //add it back into current droid lists
			        addDroid(psCurr, mission.apsDroidLists);
                }
                //remove the transporter from the current list
			    if (droidRemove(psDroid, apsDroidLists))
                {
					//cam change add droid
					psDroid->x = INVALID_XY;
					psDroid->y = INVALID_XY;
			        addDroid(psDroid, mission.apsDroidLists);
                }
            }
		    psDroid = psNext;
        }
    }
    else
    {
    	//reserve the droids for selected player for start of next campaign
	    mission.apsDroidLists[selectedPlayer] = apsDroidLists[selectedPlayer];
        apsDroidLists[selectedPlayer] = NULL;
    	psDroid = mission.apsDroidLists[selectedPlayer];
		while(psDroid != NULL) 
		{
			//cam change add droid
			psDroid->x = INVALID_XY;
			psDroid->y = INVALID_XY;
            psDroid = psDroid->psNext;
		}
	}

    //if the droids have been moved to safety then get any Transporters that exist
    if (getDroidsToSafetyFlag())
    {
        /*now that every unit for the selected player has been moved into the 
        mission list - reverse it and fill the transporter with the first ten units*/
        reverseObjectList((BASE_OBJECT**)&mission.apsDroidLists[selectedPlayer]);

        //find the *first* transporter
        for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != NULL; 
            psDroid = psDroid->psNext)
        {
            if (psDroid->droidType == DROID_TRANSPORTER)
            {
                //fill it with droids from the mission list
                for (psSafeDroid = mission.apsDroidLists[selectedPlayer]; psSafeDroid != 
                    NULL; psSafeDroid = psNextSafe)
                {
                    psNextSafe = psSafeDroid->psNext;
                    if (psSafeDroid != psDroid)
                    {
                        //add to the Transporter, checking for when full
	                    if (checkTransporterSpace(psDroid, psSafeDroid))
	                    {
		                    if (droidRemove(psSafeDroid, mission.apsDroidLists))
                            {
    	                        grpJoin(psDroid->psGroup, psSafeDroid);
                            }
                        }
                        else
                        {
                            //setting this will cause the loop to end
                            psNextSafe = NULL;
                        }
                    }
                }
                //only want to fill one transporter
                break;
            }
        }
    }

	//clear all other droids
	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		psDroid = apsDroidLists[inc];

		while(psDroid != NULL) 
		{
			psNext = psDroid->psNext;
			vanishDroid(psDroid);
			psDroid = psNext;
		}
	}

	//clear out the audio
	audio_StopAll();

	//clear all other memory
	freeAllStructs();
	freeAllFeatures();
}


//start an off world mission - clearing the object lists
BOOL startMissionOffClear(STRING *pGame)
{
	saveMissionData();

	//load in the new game clearing the lists
	if (!loadGame(pGame, !KEEPOBJECTS, !FREEMEM,FALSE))
	{
		return FALSE;
	}

	//call after everything has been loaded up - done on stageThreeInit
	//gridReset();

	offWorldKeepLists = FALSE;
	intResetPreviousObj();

	//this gets set when the timer is added in scriptFuncs
	//mission.startTime = gameTime;
#ifdef WIN32			// ffs ab
    //the message should have been played at the between stage
    missionCountDown &= ~NOT_PLAYED_ACTIVATED;
#endif
	return TRUE;
}

//start an off world mission - keeping the object lists
BOOL startMissionOffKeep(STRING *pGame)
{
	saveMissionData();

	//load in the new game clearing the lists
	if (!loadGame(pGame, !KEEPOBJECTS, !FREEMEM,FALSE))
	{
		return FALSE;
	}

	//call after everything has been loaded up - done on stageThreeInit
	//gridReset();

	offWorldKeepLists = TRUE;
	intResetPreviousObj();

	//this gets set when the timer is added in scriptFuncs
	//mission.startTime = gameTime;
#ifdef WIN32		// ffs ab
    //the message should have been played at the between stage
    missionCountDown &= ~NOT_PLAYED_ACTIVATED;
#endif
    return TRUE;
}

BOOL startMissionCampaignStart(STRING *pGame)
{
    //clear out all intelligence screen messages
    freeMessages();

    //check no units left with any settings that are invalid
    clearCampaignUnits();

    //load in the new game details
	if (!loadGame(pGame, !KEEPOBJECTS, FREEMEM, FALSE))
	{
		return FALSE;
	}

	//call after everything has been loaded up - done on stageThreeInit
	//gridReset();

	offWorldKeepLists = FALSE;

	return TRUE;
}

BOOL startMissionCampaignChange(STRING *pGame)
{
    //clear out all intelligence screen messages
    freeMessages();

    //check no units left with any settings that are invalid
    clearCampaignUnits();

    //clear out the production run between campaigns
    changeProductionPlayer((UBYTE)selectedPlayer);

    saveCampaignData();

	//load in the new game details
	if (!loadGame(pGame, !KEEPOBJECTS, !FREEMEM, FALSE))
	{
		return FALSE;
	}

	offWorldKeepLists = FALSE;
    intResetPreviousObj();

	return TRUE;
}

BOOL startMissionCampaignExpand(STRING *pGame)
{
	//load in the new game details
	if (!loadGame(pGame, KEEPOBJECTS, !FREEMEM, FALSE))
	{
		return FALSE;
	}

	//call after everything has been loaded up - done on stageThreeInit
	//gridReset();

	offWorldKeepLists = FALSE;
	return TRUE;
}

BOOL startMissionCampaignExpandLimbo(STRING *pGame)
{
    saveMissionLimboData();

	//load in the new game details
	if (!loadGame(pGame, KEEPOBJECTS, !FREEMEM, FALSE))
	{
		return FALSE;
	}

    	//call after everything has been loaded up - done on stageThreeInit
	//gridReset();

	offWorldKeepLists = FALSE;

	return TRUE;
}

BOOL startMissionBetween(void)
{
	offWorldKeepLists = FALSE;

	return TRUE;
}

//check no units left with any settings that are invalid
void clearCampaignUnits(void)
{
    DROID       *psDroid;

    for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = 
        psDroid->psNext)
    {
        orderDroid(psDroid, DORDER_STOP);
        psDroid->psBaseStruct = NULL;
    }
}

/*This deals with droids at the end of an offworld mission*/
void processMission()
{
	DROID			*psNext;
	DROID			*psDroid;
	UDWORD			droidX, droidY;
	PICKTILE		pickRes;

	//and the rest on the mission map  - for now?
	for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = psNext)
	{
		psNext = psDroid->psNext;
		//reset order - do this to all the droids that are returning from offWorld
		orderDroid(psDroid, DORDER_STOP);

		//remove out of stored list and add to current Droid list
		if (droidRemove(psDroid, apsDroidLists))
        {
		    addDroid(psDroid, mission.apsDroidLists);
		    // just remove the droid from the grid cos
	    	//   there is only one grid which gets reset when we go back to the
    		//   campaign map
		    //gridRemoveObject((BASE_OBJECT *)psDroid); - happens in droidRemove()
		    //set the x/y for now
	    	//psDroid->x = getHomeLandingX() + 256;
    		//psDroid->y = getHomeLandingY() + 256;
		    //droidX = getLandingX(psDroid->player) >> TILE_SHIFT; //getHomeLandingX() >> TILE_SHIFT;
	    	//droidY = getLandingY(psDroid->player) >> TILE_SHIFT; //getHomeLandingY() >> TILE_SHIFT;
            droidX = getHomeLandingX();
            droidY = getHomeLandingY();
		    //swap the droid and map pointers
    		swapMissionPointers();

    		pickRes = pickHalfATile(&droidX, &droidY,LOOK_FOR_EMPTY_TILE);
	    	if (pickRes == NO_FREE_TILE )
		    {
			    ASSERT((FALSE, "processMission: Unable to find a free location"));
    		}
	    	psDroid->x = (UWORD)(droidX << TILE_SHIFT);
		    psDroid->y = (UWORD)(droidY << TILE_SHIFT);
    		if (pickRes == HALF_FREE_TILE )
	    	{
		    	psDroid->x += TILE_UNITS;
			    psDroid->y += TILE_UNITS;
		    }
		    psDroid->z = map_Height(psDroid->x, psDroid->y);
		    updateDroidOrientation(psDroid);
/*		    psDroid->lastTile = mapTile(psDroid->x >> TILE_SHIFT, 
			psDroid->y >> TILE_SHIFT);
*/
	    	//swap the droid and map pointers back again
    		swapMissionPointers();
    		psDroid->selected = FALSE;
            //this is mainly for VTOLs
            psDroid->psBaseStruct = NULL;
	    	psDroid->cluster = 0;
		    //initialise the movement data
    		initDroidMovement(psDroid);

	    	//orderSelectedLoc(psDroid->player, psDroid->x + 3 * TILE_UNITS, 
		    //	psDroid->y + 3 * TILE_UNITS);
        }
	}
}


#ifdef WIN32
#define MAXLIMBODROIDS (999)
#else
#define MAXLIMBODROIDS (10)
#endif

/*This deals with droids at the end of an offworld Limbo mission*/
void processMissionLimbo(void)
{
	DROID			*psNext, *psDroid;
	UDWORD	numDroidsAddedToLimboList=0;

	//all droids (for selectedPlayer only) are placed into the limbo list
	for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = psNext)
	{
		psNext = psDroid->psNext;
		//KILL OFF TRANSPORTER - should never be one but....
		if (psDroid->droidType == DROID_TRANSPORTER)
		{
			vanishDroid(psDroid);
		}
        else
        {
			if (numDroidsAddedToLimboList >= MAXLIMBODROIDS)		// any room in limbo list
			{
				vanishDroid(psDroid);
			}
			else
			{


	    		//remove out of stored list and add to current Droid list
		    	if (droidRemove(psDroid, apsDroidLists))
	            {
				    //limbo list invalidate XY
				    psDroid->x = INVALID_XY;
				    psDroid->y = INVALID_XY;
			        addDroid(psDroid, apsLimboDroids);
	                //this is mainly for VTOLs
	                psDroid->psBaseStruct = NULL;
	                psDroid->cluster = 0;
				    orderDroid(psDroid, DORDER_STOP);	 

					numDroidsAddedToLimboList++;
					
	            }


			}
        }
	}
}

/*switch the pointers for the map and droid lists so that droid placement
 and orientation can occur on the map they will appear on*/
void swapMissionPointers(void)
{
	void		*pVoid;
	void		**ppVoid;
	UDWORD		udwTemp, inc;

	//swap psMapTiles
	pVoid = (void*)psMapTiles;
	psMapTiles = mission.psMapTiles;
	mission.psMapTiles = (MAPTILE *)pVoid;
    //swap map sizes
	udwTemp = mapWidth;
	mapWidth = mission.mapWidth;
	mission.mapWidth = udwTemp;
	udwTemp = mapHeight;
	mapHeight = mission.mapHeight;
	mission.mapHeight = udwTemp;
	//swap gateway zones
	pVoid = (void*)psGateways;
	psGateways = mission.psGateways;
	mission.psGateways = (struct _gateway *)pVoid;
	ppVoid = (void**)apRLEZones;
	apRLEZones = mission.apRLEZones;
	mission.apRLEZones = (UBYTE **)ppVoid;
	udwTemp = (UDWORD)gwNumZones;
	gwNumZones = mission.gwNumZones;
	mission.gwNumZones = (SDWORD)udwTemp;
	pVoid = (void*)aNumEquiv;
	aNumEquiv = mission.aNumEquiv;
	mission.aNumEquiv = (UBYTE *)pVoid;
	ppVoid = (void**)apEquivZones;
	apEquivZones = mission.apEquivZones;
	mission.apEquivZones = (UBYTE **)ppVoid;
	pVoid = (void*)aZoneReachable;
	aZoneReachable = mission.aZoneReachable;
	mission.aZoneReachable = (UBYTE *)pVoid;
    //swap scroll limits
	udwTemp = scrollMinX;
	scrollMinX = mission.scrollMinX;
	mission.scrollMinX = udwTemp;
	udwTemp = scrollMinY;
	scrollMinY = mission.scrollMinY;
	mission.scrollMinY = udwTemp;
	udwTemp = scrollMaxX;
	scrollMaxX = mission.scrollMaxX;
	mission.scrollMaxX = udwTemp;
	udwTemp = scrollMaxY;
	scrollMaxY = mission.scrollMaxY;
	mission.scrollMaxY = udwTemp;
	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		pVoid = (void*)apsDroidLists[inc];
		apsDroidLists[inc] = mission.apsDroidLists[inc];
		mission.apsDroidLists[inc] = (DROID *)pVoid;
		pVoid = (void*)apsStructLists[inc];
		apsStructLists[inc] = mission.apsStructLists[inc];
		mission.apsStructLists[inc] = (STRUCTURE *)pVoid;
		pVoid = (void*)apsFeatureLists[inc];
		apsFeatureLists[inc] = mission.apsFeatureLists[inc];
		mission.apsFeatureLists[inc] = (FEATURE *)pVoid;
		pVoid = (void*)apsFlagPosLists[inc];
		apsFlagPosLists[inc] = mission.apsFlagPosLists[inc];
		mission.apsFlagPosLists[inc] = (FLAG_POSITION *)pVoid;
	}

/*
	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		udwTemp = iTranspEntryTileX[inc];
		iTranspEntryTileX[inc] = mission.iTranspEntryTileX[inc];
		mission.iTranspEntryTileX[inc] = udwTemp;
		udwTemp = iTranspEntryTileY[inc];
		iTranspEntryTileY[inc] = mission.iTranspEntryTileY[inc];
		mission.iTranspEntryTileY[inc] = udwTemp;
		udwTemp = iTranspExitTileX[inc];
		iTranspExitTileX[inc] = mission.iTranspExitTileX[inc];
		mission.iTranspExitTileX[inc] = udwTemp;
		udwTemp = iTranspExitTileY[inc];
		iTranspExitTileY[inc] = mission.iTranspExitTileY[inc];
		mission.iTranspExitTileY[inc] = udwTemp;
	}
*/
	// NOTE: none of the gateway pointers are swapped at the moment
	// which isn't a problem for the current usage - might need to be
	// added later
/* stuff to add
	UDWORD				type;							//defines which start and end functions to use - see levels_type in levels.h
	//struct _proximity_display	*apsProxDisp[MAX_PLAYERS];
	FLAG_POSITION				*apsFlagPosLists[MAX_PLAYERS];
	PLAYER_POWER				asPower[MAX_PLAYERS];

//stuff for save game
	UDWORD				startTime;			//time the mission started
	SDWORD				time;				//how long the mission can last
											// < 0 = no limit
	SDWORD				ETA;				//time taken for reinforcements to arrive
											// < 0 = none allowed
    UWORD               homeLZ_X;           //selectedPlayer's LZ x and y
    UWORD               homeLZ_Y;
	SDWORD				playerX;			//original view position
	SDWORD				playerY;
*/
}

void endMission(void)
{
	if (mission.type == LDS_NONE)
	{
		//can't go back any further!!
		DBMB(("Already returned from mission"));
		return;
	}

	switch (mission.type)
	{
		case LDS_CAMSTART:
        {
            //any transporters that are flying in need to be emptied
            emptyTransporters(FALSE);
            //when loading in a save game mid cam2a or cam3a it is loaded as a camstart
            endMissionCamChange();
			/*
				This is called at the end of every campaign mission
			*/
#ifdef PSX
			SetScrollLimitsTilesVisible();
#endif
			break;
        }
		case LDS_MKEEP:
		{
            //any transporters that are flying in need to be emptied
            emptyTransporters(TRUE);
			endMissionOffKeep();
			break;
		}
#ifndef COVERMOUNT
		case LDS_EXPAND:
#endif
		case LDS_BETWEEN:
		{
			/*
				This is called at the end of every campaign mission
			*/
#ifdef PSX
			SetScrollLimitsTilesVisible();
#endif
			break;
		}
#ifndef COVERMOUNT
		case LDS_CAMCHANGE:
        {
            //any transporters that are flying in need to be emptied
            emptyTransporters(FALSE);
            endMissionCamChange();
            break;
        }
        /* left in so can skip the mission for testing...*/
        case LDS_EXPAND_LIMBO:
        {
            //shouldn't be any transporters on this mission but...who knows?
            endMissionExpandLimbo();
            break;
        }
		case LDS_MCLEAR:
		{
            //any transporters that are flying in need to be emptied
            emptyTransporters(TRUE);
			endMissionOffClear();
			break;
		}
	
		case LDS_MKEEP_LIMBO:
		{
            //any transporters that are flying in need to be emptied
            emptyTransporters(TRUE);
			endMissionOffKeepLimbo();
			break;
		}
#endif
		default:
		{
			//error!
			DBERROR(("Unknown Mission Type"));
		}
	}
	
	if (missionCanReEnforce()) //mission.type == LDS_MCLEAR OR mission.type == LDS_MKEEP)
	{
		intRemoveMissionTimer();
		intRemoveTransporterTimer();
	}

	//at end of mission always do this
	intRemoveTransporterLaunch();

    //and this...
    //make sure the cheat time is not set for the next mission
    mission.cheatTime = 0;

#ifdef WIN32
    //reset the bSetPlayCountDown flag
    setPlayCountDown(TRUE);
#endif

	//mission.type = MISSION_NONE;
	mission.type = LDS_NONE;

	// reset the transporters
	initTransporters();
}

void endMissionCamChange(void)
{
    //get any droids remaining from the previous campaign
    processPreviousCampDroids();
}

void endMissionOffClear(void)
{
	processMission();
	//called in setUpMission now - AB 6/4/98
	//intAddMissionResult(result);// ajl. fire up interface.	
	restoreMissionData();

	//reset variables in Droids
	missionResetDroids();
}

void endMissionOffKeep(void)
{
	processMission();
	//called in setUpMission now - AB 6/4/98
	//intAddMissionResult(result);// ajl. fire up interface.	
	restoreMissionData();

	//reset variables in Droids
	missionResetDroids();
}

/*In this case any droids remaining (for selectedPlayer) go into a limbo list
for use in a future mission (expand type) */
void endMissionOffKeepLimbo(void)
{
    //save any droids left 'alive'
	processMissionLimbo();

    //set the lists back to the home base lists
    restoreMissionData();

	//reset variables in Droids
	missionResetDroids();
}

//This happens MID_MISSION now! but is left here in case the scripts fail but somehow get here...?
/*The selectedPlayer's droids which were separated at the start of the 
mission need to merged back into the list*/
void endMissionExpandLimbo(void)
{
    restoreMissionLimboData();
}


//this is called mid Limbo mission via the script
void resetLimboMission(void)
{
#ifndef COVERMOUNT
    //add the units that were moved into the mission list at the start of the mission
    restoreMissionLimboData();
    //set the mission type to plain old expand...
    mission.type = LDS_EXPAND;
#endif
}

/* The AI update routine for all Structures left back at base during a Mission*/
void aiUpdateMissionStructure(STRUCTURE *psStructure)
{
	BASE_STATS			*pSubject = NULL;
	UDWORD				pointsToAdd;
	PLAYER_RESEARCH		*pPlayerRes = asPlayerResList[psStructure->player];
	UDWORD				structureMode = 0;
	DROID				*psNewDroid;
	UBYTE				Quantity;
	FACTORY				*psFactory;
	RESEARCH_FACILITY	*psResFacility;
#ifdef INCLUDE_FACTORYLISTS
	DROID_TEMPLATE		*psNextTemplate;
#endif

	ASSERT((PTRVALID(psStructure, sizeof(STRUCTURE)),
		"aiUpdateMissionStructure: invalid Structure pointer"));

	ASSERT(((psStructure->pStructureType->type == REF_FACTORY  OR
		psStructure->pStructureType->type == REF_CYBORG_FACTORY  OR
		psStructure->pStructureType->type == REF_VTOL_FACTORY  OR
		psStructure->pStructureType->type == REF_RESEARCH), 
		"aiUpdateMissionStructure: Structure is not a Factory or Research Facility"));

	//only interested if the Structure "does" something!
	if (psStructure->pFunctionality == NULL)
	{
		return;
	}

	//check if any power available
	if ((asPower[psStructure->player]->currentPower > POWER_PER_CYCLE) OR 
		(!powerCalculated))
	{
		//check if this structure is due some power
		if (getLastPowered((BASE_OBJECT *)psStructure))
		{
			//get some power if necessary
			if (accruePower((BASE_OBJECT *)psStructure))
			{
				updateLastPowered((BASE_OBJECT *)psStructure, psStructure->player);
			}
		}
	}
	//determine the Subject
	switch (psStructure->pStructureType->type)
	{
		case REF_RESEARCH:
		{
			pSubject = ((RESEARCH_FACILITY*)psStructure->pFunctionality)->psSubject;
			structureMode = REF_RESEARCH;
			break;
		}
		case REF_FACTORY:
		case REF_CYBORG_FACTORY:
		case REF_VTOL_FACTORY:
		{
			pSubject = ((FACTORY*)psStructure->pFunctionality)->psSubject;
			structureMode = REF_FACTORY;
	        //check here to see if the factory's commander has died
            if (((FACTORY*)psStructure->pFunctionality)->psCommander AND
                ((FACTORY*)psStructure->pFunctionality)->psCommander->died)
            {
                //remove the commander from the factory
                assignFactoryCommandDroid(psStructure, NULL);
            }
		    break;
		}
	}

	if (pSubject != NULL)
	{
		//if subject is research...
		if (structureMode == REF_RESEARCH)
		{
			psResFacility = (RESEARCH_FACILITY*)psStructure->pFunctionality;

			//if on hold don't do anything
			if (psResFacility->timeStartHold)
			{
				return;
			}

            pPlayerRes += (pSubject->ref - REF_RESEARCH_START);
			//check research has not already been completed by another structure
			if (IsResearchCompleted(pPlayerRes)==0)
			{
				//check to see if enough power to research has accrued
				if (psResFacility->powerAccrued < ((RESEARCH *)pSubject)->researchPower)
				{
					//wait until enough power
					return;
				}

				if (psResFacility->timeStarted == ACTION_START_TIME)
				{
					//set the time started
					psResFacility->timeStarted = gameTime;
				}

				pointsToAdd = (psResFacility->researchPoints * (gameTime - 
                    psResFacility->timeStarted)) / GAME_TICKS_PER_SEC; 

				//check if Research is complete
#ifdef WIN32
				//if ((pointsToAdd + pPlayerRes->currentPoints) > psResFacility->
				//	timeToResearch)
                if ((pointsToAdd + pPlayerRes->currentPoints) > (
                    (RESEARCH *)pSubject)->researchPoints)
#else
				//if (pointsToAdd > psResFacility->timeToResearch)
                if (pointsToAdd > ((RESEARCH *)pSubject)->researchPoints)
#endif
				{
					//store the last topic researched - if its the best
					if (psResFacility->psBestTopic == NULL)
					{
						psResFacility->psBestTopic = psResFacility->psSubject;
					}
					else
					{
						if (((RESEARCH *)psResFacility->psSubject)->researchPoints > 
							((RESEARCH *)psResFacility->psBestTopic)->researchPoints)
						{
							psResFacility->psSubject = psResFacility->psSubject;
						}
					}
					psResFacility->psSubject = NULL;
					intResearchFinished(psStructure);
					researchResult(pSubject->ref - REF_RESEARCH_START, 
						psStructure->player, TRUE);
					//check if this result has enabled another topic
					intCheckResearchButton();
				}
			}
			else
			{
				//cancel this Structure's research since now complete
				psResFacility->psSubject = NULL;
				intResearchFinished(psStructure);
			}
		}
		//check for manufacture
		else if (structureMode == REF_FACTORY)
		{
			psFactory = (FACTORY *)psStructure->pFunctionality;
			Quantity = psFactory->quantity;

			//if on hold don't do anything
			if (psFactory->timeStartHold)
			{
				return;
			}

//			if (psFactory->timeStarted == ACTION_START_TIME)
//			{
//				// also need to check if a command droid's group is full
//				if ( ( psFactory->psCommander != NULL ) &&
//					 ( grpNumMembers( psFactory->psCommander->psGroup ) >=
//							cmdDroidMaxGroup( psFactory->psCommander ) ) )
//				{
//					return;
//				}
//			}

			if(CheckHaltOnMaxUnitsReached(psStructure) == TRUE) {
				return;
			}

			//check enough power has accrued to build the droid
			if (psFactory->powerAccrued < ((DROID_TEMPLATE *)pSubject)->
					powerPoints)
			{
				//wait until enough power
				return;
			}

			/*must be enough power so subtract that required to build*/
			if (psFactory->timeStarted == ACTION_START_TIME)
			{
				//set the time started
				psFactory->timeStarted = gameTime;
			}

			pointsToAdd = (gameTime - psFactory->timeStarted) / 
				GAME_TICKS_PER_SEC;

			//check for manufacture to be complete
			if (pointsToAdd > psFactory->timeToBuild)
			{
				//build droid - store in mission list
				psNewDroid = buildMissionDroid((DROID_TEMPLATE *)pSubject, 
					psStructure->x, psStructure->y, psStructure->player);

				if (!psNewDroid)
				{
					//if couldn't build then cancel the production
					Quantity = 0;
					psFactory->psSubject = NULL;
					intManufactureFinished(psStructure);
				}
				else
				{
					if (psStructure->player == selectedPlayer)
					{
						intRefreshScreen();	// update the interface.
					}

                    //store the factory as the droid's baseStructure instead
					//add it to the factory group
					/*if (!psFactory->psGroup)
					{
						//create the factory group
						if (!grpCreate(&psFactory->psGroup))
						{
							DBPRINTF(("missionUpdateStructure: unable to create group\n"));
						}
						else
						{
							grpJoin(psFactory->psGroup, psNewDroid);
						}
					}
					else
					{
						grpJoin(psFactory->psGroup, psNewDroid);
					}*/
                    psNewDroid->psBaseStruct = psStructure;

					//reset the start time
					psFactory->timeStarted = ACTION_START_TIME;
					psFactory->powerAccrued = 0;

#ifdef INCLUDE_FACTORYLISTS
					//next bit for productionPlayer only 
					if (productionPlayer == psStructure->player)
					{
						psNextTemplate = factoryProdUpdate(psStructure,
							(DROID_TEMPLATE *)pSubject);
						if (psNextTemplate)
						{
							structSetManufacture(psStructure, psNextTemplate,Quantity);
							return;
						}
						else 
						{
							//nothing more to manufacture - reset the Subject and Tab on HCI Form
							psFactory->psSubject = NULL;
							intManufactureFinished(psStructure);
						}
					}
					else
#endif
					{
						//decrement the quantity to manufacture if not set to infinity
						if (Quantity != NON_STOP_PRODUCTION)
						{
							psFactory->quantity--;
							Quantity--;
						}
	
						// If quantity not 0 then kick of another manufacture.
						if(Quantity) 
						{	
							// Manufacture another.
							structSetManufacture(psStructure, (DROID_TEMPLATE*)pSubject,Quantity);
							//playerNewDroid(psNewDroid);
							return;
						}
						else 
						{
							//when quantity = 0, reset the Subject and Tab on HCI Form
							psFactory->psSubject = NULL;
							intManufactureFinished(psStructure);
						}
					}
				}
			}
		}
	}
}

/* The update routine for all Structures left back at base during a Mission*/
void missionStructureUpdate(STRUCTURE *psBuilding)
{

	ASSERT((PTRVALID(psBuilding, sizeof(STRUCTURE)),
		"structureUpdate: Invalid Structure pointer"));

	//update the manufacture/research of the building
//	if (psBuilding->pStructureType->type == REF_FACTORY OR
//		psBuilding->pStructureType->type == REF_CYBORG_FACTORY OR
//		psBuilding->pStructureType->type == REF_VTOL_FACTORY OR
//		psBuilding->pStructureType->type == REF_RESEARCH)
	if(StructIsFactory(psBuilding) OR
		psBuilding->pStructureType->type == REF_RESEARCH)
	{
		if (psBuilding->status == SS_BUILT)
		{
			aiUpdateMissionStructure(psBuilding);
		}
	}
}

/* The update routine for all droids left back at home base
Only interested in Transporters at present*/
void missionDroidUpdate(DROID *psDroid)
{
	ASSERT((PTRVALID(psDroid, sizeof(DROID)),
		"unitUpdate: Invalid unit pointer"));

    /*This is required for Transporters that are moved offWorld so the 
    saveGame doesn't try to set their position in the map - especially important
    for endCam2 where there isn't a valid map!*/
    if (psDroid->droidType == DROID_TRANSPORTER)
    {
	    psDroid->x = INVALID_XY;
	    psDroid->y = INVALID_XY;
    }

	//ignore all droids except Transporters
	if ( (psDroid->droidType != DROID_TRANSPORTER)           ||
		 !(orderState(psDroid, DORDER_TRANSPORTOUT)          ||
		   orderState(psDroid, DORDER_TRANSPORTIN)           ||
		   orderState(psDroid, DORDER_TRANSPORTRETURN))         )
	{
		return;
	}

	// NO ai update droid

	// update the droids order
	orderUpdateDroid(psDroid);

	// update the action of the droid
	actionUpdateDroid(psDroid);

	//NO move update
}

//reset variables in Droids such as order and position
void missionResetDroids()
{
	UDWORD			player;
	DROID			*psDroid, *psNext;
	STRUCTURE		*psStruct;
	FACTORY			*psFactory;
//	UDWORD			mapX, mapY;
	BOOL			placed;
	UDWORD			x, y;
	PICKTILE		pickRes;

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		for (psDroid = apsDroidLists[player]; psDroid != NULL; psDroid = psNext)
		{
			psNext = psDroid->psNext;

			//reset order - unless constructor droid that is mid-build
            //if (psDroid->droidType == DROID_CONSTRUCT AND orderStateObj(psDroid, 
            if ((psDroid->droidType == DROID_CONSTRUCT OR psDroid->droidType == 
                DROID_CYBORG_CONSTRUCT) AND orderStateObj(psDroid, 
                    DORDER_BUILD, (BASE_OBJECT **)&psStruct))
            {
                //need to set the action time to ignore the previous mission time
                psDroid->actionStarted = gameTime;
            }
            else
            {
			    orderDroid(psDroid, DORDER_STOP);
            }
			
			//KILL OFF TRANSPORTER
			if (psDroid->droidType == DROID_TRANSPORTER)
			{
				vanishDroid(psDroid);
			}
		}
	}

    //don't need to implement this hack now since using pickATile - oh what a wonderful routine...
	//only need to look through the selected players list
	/*need to put all the droids that were built whilst off on the mission into
	a temp list so can add them back into the world one by one so that the
	path blocking routines do not fail*/
	/*for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = 
		psNext)
	{
		psNext = psDroid->psNext;
		//for all droids that have never left home base
		if (psDroid->x == INVALID_XY AND psDroid->y == INVALID_XY)
		{
			if (droidRemove(psDroid, apsDroidLists))
            {
			    addDroid(psDroid, mission.apsDroidLists);
            }
		}
	}*/

	for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = 
		psDroid->psNext)
	//for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != NULL; 
	//	psDroid = psNext)
	{
		psNext = psDroid->psNext;
		//for all droids that have never left home base
		if (psDroid->x == INVALID_XY AND psDroid->y == INVALID_XY)
		{
            //this is now stored as the baseStruct when the droid is built...
			//find which factory produced the droid
			/*psFactory = NULL;
			for (psStruct = apsStructLists[psDroid->player]; psStruct != 
				NULL; psStruct = psStruct->psNext)
			{
				if(StructIsFactory(psStruct))
				{
					if (((FACTORY *)psStruct->pFunctionality)->psGroup == 
						psDroid->psGroup)
					{
						//found the right factory
						psFactory = (FACTORY *)psStruct->pFunctionality;
						break;
					}
				}
			}*/
            psStruct = psDroid->psBaseStruct;
            if (psStruct AND StructIsFactory(psStruct))
            {
                psFactory = (FACTORY *)psStruct->pFunctionality;
            }
			placed = FALSE;
			//find a location next to the factory
			if (psStruct)
			{
				/*placed = placeDroid(psStruct, &x, &y);
				if (placed)
				{
					psDroid->x = (UWORD)(x << TILE_SHIFT);
					psDroid->y = (UWORD)(y << TILE_SHIFT);
				}
				else
				{
					psStruct = NULL;
				}*/
                //use pickATile now - yipeee!
                //use factory DP if one
                if (psFactory->psAssemblyPoint)
                {
                    x = (UWORD)(psFactory->psAssemblyPoint->coords.x >> TILE_SHIFT);
                    y = (UWORD)(psFactory->psAssemblyPoint->coords.y >> TILE_SHIFT);
                }
                else
                {
                    x = (UWORD)(psStruct->x >> TILE_SHIFT);
                    y = (UWORD)(psStruct->y >> TILE_SHIFT);
                }
		        pickRes = pickHalfATile(&x, &y,LOOK_FOR_EMPTY_TILE);
		        if (pickRes == NO_FREE_TILE )
		        {
			        ASSERT((FALSE, "missionResetUnits: Unable to find a free location"));
                    psStruct = NULL;
		        }
                else
                {
    		        psDroid->x = (UWORD)(x << TILE_SHIFT);
	    	        psDroid->y = (UWORD)(y << TILE_SHIFT);
		            if (pickRes == HALF_FREE_TILE )
		            {
			            psDroid->x += TILE_UNITS;
			            psDroid->y += TILE_UNITS;
		            }
                    placed = TRUE;
                }
			}
			//if couldn't find the factory - hmmm
			if (!psStruct)
			{
				//just stick them near the HQ
				for (psStruct = apsStructLists[psDroid->player]; psStruct != 
					NULL; psStruct = psStruct->psNext)
				{
					if (psStruct->pStructureType->type == REF_HQ)
					{
						/*psDroid->x = (UWORD)(psStruct->x + 128);
						psDroid->y = (UWORD)(psStruct->y + 128);
						placed = TRUE;
						break;*/
                        //use pickATile again...
                        x = (UWORD)(psStruct->x >> TILE_SHIFT);
                        y = (UWORD)(psStruct->y >> TILE_SHIFT);
		                pickRes = pickHalfATile(&x, &y,LOOK_FOR_EMPTY_TILE);
		                if (pickRes == NO_FREE_TILE )
		                {
			                ASSERT((FALSE, "missionResetUnits: Unable to find a free location"));
                            psStruct = NULL;
		                }
                        else
                        {
    		                psDroid->x = (UWORD)(x << TILE_SHIFT);
	    	                psDroid->y = (UWORD)(y << TILE_SHIFT);
		                    if (pickRes == HALF_FREE_TILE )
		                    {
			                    psDroid->x += TILE_UNITS;
			                    psDroid->y += TILE_UNITS;
		                    }
                            placed = TRUE;
                        }
                        break;
					}				
				}
			}
			if (placed)
			{
                //don't need to do this since using pickATile now...
				//add them to the current list
				/*if (droidRemove(psDroid, mission.apsDroidLists))
                {
				    addDroid(psDroid, apsDroidLists);
    				if (psFactory)
	    			{
		    			//order the droid to the factory DP
			    		orderSelectedLoc(psDroid->player, 
				    		psFactory->psAssemblyPoint->coords.x,
					    	psFactory->psAssemblyPoint->coords.y);
    				}
	    			else
		    		{
			    		//order them to move to an arbitary new location!
				    	orderSelectedLoc(psDroid->player, psDroid->x + 3 * TILE_UNITS, 
					    	psDroid->y + 3 * TILE_UNITS);
				    }*/
				    //do all the things in build droid that never did when it was built!
				    // check the droid is a reasonable distance from the edge of the map
				    if (psDroid->x <= TILE_UNITS || psDroid->x >= (mapWidth * 
					    TILE_UNITS) - TILE_UNITS ||	psDroid->y <= TILE_UNITS || 
					    psDroid->y >= (mapHeight * TILE_UNITS) - TILE_UNITS)
				    {
					    DBMB(("missionResetUnits: unit too close to edge of map - removing"));
						vanishDroid(psDroid);
					    continue;
				    }
					// Check there is nothing on the map
				    /*if ( TILE_OCCUPIED(mapTile(x >> TILE_SHIFT, y >> TILE_SHIFT)) )
				    {
					    DBPRINTF(("missionResetDroids: tile occupied\n");
	    `				removeDroid(psDroid, apsDroidLists);
					    droidRelease(psDroid);
					    HEAP_FREE(psDroidHeap, psDroid);
				    }*/
/*				    mapX = psDroid->x >> TILE_SHIFT;
				    mapY = psDroid->y >> TILE_SHIFT;
				    psDroid->lastTile = mapTile(mapX,mapY);
*/
				    //set droid height
				    psDroid->z = map_Height(psDroid->x, psDroid->y);
				
				    // People always stand upright 
				    //if(psDroid->droidType != DROID_PERSON AND psDroid->type != DROID_CYBORG)
                    if(psDroid->droidType != DROID_PERSON AND !cyborgDroid(psDroid))
				    {
					    updateDroidOrientation(psDroid);
				    }
				    visTilesUpdate((BASE_OBJECT *)psDroid,FALSE);
				    //reset the selected flag
				    psDroid->selected = FALSE;
                //}
			}
			else
			{
				//can't put it down so get rid of this droid!!
				ASSERT((FALSE,"missionResetUnits: can't place unit - cancel to continue"));
				vanishDroid(psDroid);
			}
		}
	}
}

/*unloads the Transporter passed into the mission at the specified x/y
goingHome = TRUE when returning from an off World mission*/
void unloadTransporter(DROID *psTransporter, UDWORD x, UDWORD y, BOOL goingHome)
{
	DROID		*psDroid, *psNext;
	DROID		**ppCurrentList, **ppStoredList;
	UDWORD		droidX, droidY;
	UWORD		iX, iY;
	DROID_GROUP	*psGroup;

	if (goingHome)
	{
		ppCurrentList = mission.apsDroidLists;
		ppStoredList = apsDroidLists;
	}
	else
	{
		ppCurrentList = apsDroidLists;
		ppStoredList = mission.apsDroidLists;
	}

	//look through the stored list of droids to see if there any transporters
	/*for (psTransporter = ppStoredList[selectedPlayer]; psTransporter != NULL; psTransporter = 
		psTransporter->psNext)
	{
		if (psTransporter->droidType == DROID_TRANSPORTER)
		{
			//remove out of stored list and add to current Droid list
			removeDroid(psTransporter, ppStoredList);
			addDroid(psTransporter, ppCurrentList);
			//need to put the Transporter down at a specified location
			psTransporter->x = x;
			psTransporter->y = y;
		}
	}*/

	//unload all the droids from within the current Transporter
	if (psTransporter->droidType == DROID_TRANSPORTER)
	{
		// If the scripts asked for transporter tracking then clear the "tracking a transporter" flag
		// since the transporters landed and unloaded now.
		if(psTransporter->player == selectedPlayer) {
			bTrackingTransporter = FALSE;
		}

		// reset the transporter cluster
		psTransporter->cluster = 0;
		for (psDroid = psTransporter->psGroup->psList; psDroid != NULL 
				AND psDroid != psTransporter; psDroid = psNext)
		{
			psNext = psDroid->psGrpNext;

			//add it back into current droid lists
			addDroid(psDroid, ppCurrentList);
			//hack in the starting point!
			/*psDroid->x = x;
			psDroid->y = y;
			if (!goingHome)
			{
				//send the droids off to a starting location
				orderSelectedLoc(psDroid->player, x + 3*TILE_UNITS, y + 3*TILE_UNITS);
			}*/
			//starting point...based around the value passed in
			droidX = x >> TILE_SHIFT;
			droidY = y >> TILE_SHIFT;
			if (goingHome)
			{
				//swap the droid and map pointers
				swapMissionPointers();
			}
			if (!pickATileGen(&droidX, &droidY,LOOK_FOR_EMPTY_TILE,zonedPAT))
			{
				ASSERT((FALSE, "unloadTransporter: Unable to find a valid location"));
			}
			psDroid->x = (UWORD)(droidX << TILE_SHIFT);
			psDroid->y = (UWORD)(droidY << TILE_SHIFT);
			psDroid->z = map_Height(psDroid->x, psDroid->y);
			updateDroidOrientation(psDroid);
			// a commander needs to get it's group back
			if (psDroid->droidType == DROID_COMMAND)
			{
				if (grpCreate(&psGroup))
				{
					grpJoin(psGroup, psDroid);
				}
				clearCommandDroidFactory(psDroid);
			}

			//initialise the movement data
			initDroidMovement(psDroid);
			//reset droid orders
			orderDroid(psDroid, DORDER_STOP);
			gridAddObject((BASE_OBJECT *)psDroid);
			psDroid->selected = FALSE;
            //this is mainly for VTOLs
            psDroid->psBaseStruct = NULL;
			psDroid->cluster = 0;
			if (goingHome)
			{
				//swap the droid and map pointers
				swapMissionPointers();
			}

            //inform all other players
            if (bMultiPlayer)
            {
                sendDroidDisEmbark(psDroid);
            }
		}

		/* trigger script callback detailing group about to disembark */
		transporterSetScriptCurrent( psTransporter );
		eventFireCallbackTrigger(CALL_TRANSPORTER_LANDED);
		transporterSetScriptCurrent( NULL );

		/* remove droids from transporter group if not already transferred to script group */
		for (psDroid = psTransporter->psGroup->psList; psDroid != NULL 
				AND psDroid != psTransporter; psDroid = psNext)
		{
			psNext = psDroid->psGrpNext;
			grpLeave( psTransporter->psGroup, psDroid );
		}
	}

    //don't do this in multiPlayer
    if (!bMultiPlayer)
    {
	    //send all transporter Droids back to home base if off world
	    if (!goingHome)
	    {
		    /* stop the camera following the transporter */
		    psTransporter->selected = FALSE;

		    /* send transporter offworld */
		    missionGetTransporterExit( psTransporter->player, &iX, &iY );
		    orderDroidLoc(psTransporter, DORDER_TRANSPORTRETURN, iX, iY );
            //set the launch time so the transporter doesn't just disappear for CAMSTART/CAMCHANGE
            transporterSetLaunchTime(gameTime);
        }
    }
}

void missionMoveTransporterOffWorld( DROID *psTransporter )
{
    W_CLICKFORM     *psForm;
    DROID           *psDroid;

	if (psTransporter->droidType == DROID_TRANSPORTER)
	{
    	/* trigger script callback */
	    transporterSetScriptCurrent( psTransporter );
		eventFireCallbackTrigger(CALL_TRANSPORTER_OFFMAP);
		transporterSetScriptCurrent( NULL );

		//gridRemoveObject( (BASE_OBJECT *) psTransporter ); - these happen in droidRemove()
		//clustRemoveObject( (BASE_OBJECT *) psTransporter );
		if (droidRemove(psTransporter, apsDroidLists))
        {
		    addDroid(psTransporter, mission.apsDroidLists);
        }

        //stop the droid moving - the moveUpdate happens AFTER the orderUpdate and can cause problems if the droid moves from one tile to another
        moveReallyStopDroid(psTransporter);

	    //if offworld mission, then add the timer
		//if (mission.type == LDS_MKEEP OR mission.type == LDS_MCLEAR)
        if (missionCanReEnforce() AND psTransporter->player == selectedPlayer)
	    {
		    addTransporterTimerInterface();
   			//set the data for the transporter timer label
    		widgSetUserData(psWScreen, IDTRANTIMER_DISPLAY, (void*)psTransporter);
#ifdef WIN32
            //make sure the button is enabled
            psForm = (W_CLICKFORM*)widgGetFromID(psWScreen,IDTRANTIMER_BUTTON);
            if (psForm)
            {
                formSetClickState(psForm, WCLICK_NORMAL);
            }
#endif
		}
        //need a callback for when all the selectedPlayers' reinforcements have been delivered
        if (psTransporter->player == selectedPlayer)
        {
            psDroid = NULL;
            for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != 
                NULL; psDroid = psDroid->psNext)
            {
                if (psDroid->droidType != DROID_TRANSPORTER)
                {
                    break;
                }
            }
            if (psDroid == NULL)
            {
            	eventFireCallbackTrigger(CALL_NO_REINFORCEMENTS_LEFT);
            }
        }
	}
	else
	{
		DBPRINTF( ("missionMoveTransporterOffWorld: droid type not transporter!\n") );
	}
}


void intReAddMissionTimer(void)
{
	if (widgGetFromID(psWScreen,IDTIMER_FORM) != NULL) {
		void *UserData;

		// Need to preserve widgets UserData.
		UserData = widgGetUserData(psWScreen, IDTRANTIMER_DISPLAY);

		intRemoveMissionTimer();
		intAddMissionTimer();

   		widgSetUserData(psWScreen, IDTRANTIMER_DISPLAY, UserData);
	}
}


//add the Mission timer into the top  right hand corner of the screen
BOOL intAddMissionTimer(void)
{
	W_FORMINIT		sFormInit;
	W_LABINIT		sLabInit;

	//check to see if it exists already
	if (widgGetFromID(psWScreen,IDTIMER_FORM) != NULL) 
	{	
		return TRUE;
	}

	// Add the background
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
#endif
	sFormInit.formID = 0;
	sFormInit.id = IDTIMER_FORM;
	sFormInit.style = WFORM_PLAIN;
#ifdef PSX
	sFormInit.width = iV_GetImageWidth(IntImages,IMAGE_MISSIONINFO);
	sFormInit.height = iV_GetImageHeight(IntImages,IMAGE_MISSIONINFO);
//	sFormInit.x = 320-(WidthToPSX(sFormInit.width/2));
	sFormInit.x = 320-155/2;	//iV_GetImageWidth(IntImages,IMAGE_MISSIONINFO)/2;
//	sFormInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_MISSION_CLOCK,IMAGE_MISSION_CLOCK_UP);;
	sFormInit.pUserData = (void*)IMAGE_MISSIONINFO;
	sFormInit.pDisplay = intDisplayImage;
	if(GetControllerType(0) == CON_MOUSE) {
		sFormInit.y = TIMER_Y+32;
	} else {
		sFormInit.y = TIMER_Y;
	}
#else
	sFormInit.width = iV_GetImageWidth(IntImages,IMAGE_MISSION_CLOCK);//TIMER_WIDTH;
	sFormInit.height = iV_GetImageHeight(IntImages,IMAGE_MISSION_CLOCK);//TIMER_HEIGHT;
	sFormInit.x = (SWORD)(RADTLX + RADWIDTH - sFormInit.width);
	sFormInit.y = (SWORD)TIMER_Y;
	sFormInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_MISSION_CLOCK,IMAGE_MISSION_CLOCK_UP);;
	sFormInit.pDisplay = intDisplayMissionClock;
#endif

	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FORE);
#endif
	//add labels for the time display
	memset(&sLabInit,0,sizeof(W_LABINIT));
	sLabInit.formID = IDTIMER_FORM;
	sLabInit.id = IDTIMER_DISPLAY;
	sLabInit.style = WLAB_PLAIN | WIDG_HIDDEN;
	sLabInit.x = TIMER_LABELX;
	sLabInit.y = TIMER_LABELY;
	sLabInit.width = sFormInit.width;//TIMER_WIDTH;
	sLabInit.height = sFormInit.height;//TIMER_HEIGHT;
	sLabInit.pText = "00:00:00";
	sLabInit.FontID = WFont;
	sLabInit.pCallback = intUpdateMissionTimer;
#ifdef PSX
	sLabInit.pDisplay = intDisplayTime;
#endif
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return FALSE;
	}

#ifdef PSX
	//add labels for the time display
	memset(&sLabInit,0,sizeof(W_LABINIT));
	sLabInit.formID = IDTIMER_FORM;
	sLabInit.id = IDTRANTIMER_DISPLAY;
	sLabInit.style = WLAB_PLAIN | WIDG_HIDDEN;
	sLabInit.x = TIMER_LABELX+44+32+22;
	sLabInit.y = TIMER_LABELY;
	sLabInit.width = TRAN_TIMER_WIDTH;
	sLabInit.height = sFormInit.height;//TRAN_TIMER_HEIGHT;
	sLabInit.pText = "00:00:00";
	sLabInit.FontID = WFont;
	sLabInit.pCallback = intUpdateTransporterTimer;
	sLabInit.pDisplay = intDisplayTime;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return FALSE;
	}
#endif

	return TRUE;
}

//add the Transporter timer into the top left hand corner of the screen
BOOL intAddTransporterTimer(void)
{
#ifdef WIN32
	W_FORMINIT		sFormInit;
	W_LABINIT		sLabInit;

    //make sure that Transporter Launch button isn't up as well
    intRemoveTransporterLaunch();

	//check to see if it exists already
	if (widgGetFromID(psWScreen, IDTRANTIMER_BUTTON) != NULL) 
	{	
		return TRUE;
	}

	// Add the button form - clickable
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = 0;
	sFormInit.id = IDTRANTIMER_BUTTON;
	sFormInit.style = WFORM_CLICKABLE | WFORM_NOCLICKMOVE;
	sFormInit.x = TRAN_FORM_X;
	sFormInit.y = TRAN_FORM_Y;
	sFormInit.width = iV_GetImageWidth(IntImages,IMAGE_TRANSETA_UP);
	sFormInit.height = iV_GetImageHeight(IntImages,IMAGE_TRANSETA_UP);
	sFormInit.pTip = strresGetString(psStringRes, STR_INT_TRANSPORTER);
	sFormInit.pDisplay = intDisplayImageHilight;
	sFormInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_TRANSETA_DOWN,
		IMAGE_TRANSETA_UP);

	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	//add labels for the time display
	memset(&sLabInit,0,sizeof(W_LABINIT));
	sLabInit.formID = IDTRANTIMER_BUTTON;
	sLabInit.id = IDTRANTIMER_DISPLAY;
	sLabInit.style = WLAB_PLAIN | WIDG_HIDDEN;
	sLabInit.x = TRAN_TIMER_X;
	sLabInit.y = TRAN_TIMER_Y;
	sLabInit.width = TRAN_TIMER_WIDTH;
	sLabInit.height = sFormInit.height;
	sLabInit.FontID = WFont;
	sLabInit.pCallback = intUpdateTransporterTimer;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return FALSE;
	}

	//add the capacity label
	memset(&sLabInit,0,sizeof(W_LABINIT));
	sLabInit.formID = IDTRANTIMER_BUTTON;
	sLabInit.id = IDTRANS_CAPACITY;
	sLabInit.style = WLAB_PLAIN;
	sLabInit.x = 65;
	sLabInit.y = 1;
	sLabInit.width = 16;
	sLabInit.height = 16;
	sLabInit.pText = "00/10";
	sLabInit.FontID = WFont;
	sLabInit.pCallback = intUpdateTransCapacity;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return FALSE;
	}

#endif
	return TRUE;
}


/*
//add the Transporter timer into the top left hand corner of the screen
BOOL intAddTransporterTimer(void)
{
#ifdef WIN32
	W_FORMINIT		sFormInit;
	W_LABINIT		sLabInit;
	W_BUTINIT		sButInit;

	// Add the background - invisible since the button image caters for this
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
//#ifdef PSX
//	WidgSetOTIndex(OT2D_FARBACK);
//#endif
	sFormInit.formID = 0;
	sFormInit.id = IDTRANSTIMER_FORM;
	sFormInit.style = WFORM_PLAIN | WFORM_INVISIBLE;
	sFormInit.x = TRAN_FORM_X;
	sFormInit.y = TRAN_FORM_Y;
	sFormInit.width = iV_GetImageWidth(IntImages,IMAGE_TRANSETA_UP);//TRAN_FORM_WIDTH;
	sFormInit.height = iV_GetImageHeight(IntImages,IMAGE_TRANSETA_UP);//TRAN_FORM_HEIGHT;
	//sFormInit.pDisplay = intDisplayPlainForm;

	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

//#ifdef PSX
//	WidgSetOTIndex(OT2D_BACK);
//#endif

	//add labels for the time display
	memset(&sLabInit,0,sizeof(W_LABINIT));
	sLabInit.formID = IDTRANSTIMER_FORM;
	sLabInit.id = IDTRANTIMER_DISPLAY;
	sLabInit.style = WLAB_PLAIN | WIDG_HIDDEN;
	sLabInit.x = TRAN_TIMER_X;
	sLabInit.y = TRAN_TIMER_Y;
	sLabInit.width = TRAN_TIMER_WIDTH;
	sLabInit.height = sFormInit.height;//TRAN_TIMER_HEIGHT;
	//sLabInit.pText = "00.00";
	//if (mission.ETA < 0)
	//{
	//	sLabInit.pText = "00:00";
	//}
	//else
	//{
	//	fillTimeDisplay(sLabInit.pText, mission.ETA);
	}
	sLabInit.FontID = WFont;
	sLabInit.pCallback = intUpdateTransporterTimer;
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return FALSE;
	}

//#ifdef PSX
//	WidgSetOTIndex(OT2D_FARBACK);
//#endif
	//set up button data 
	memset(&sButInit, 0, sizeof(W_BUTINIT));
	sButInit.formID = IDTRANSTIMER_FORM;
	sButInit.id = IDTRANTIMER_BUTTON;
	sButInit.x = 0;//TRAN_FORM_X;
	sButInit.y = 0;//TRAN_FORM_Y;
	sButInit.width = sFormInit.width;
	sButInit.height = sFormInit.height;
	sButInit.FontID = WFont;
	sButInit.style = WBUT_PLAIN;
	//sButInit.pText = "T";
	sButInit.pTip = strresGetString(psStringRes, STR_INT_TRANSPORTER);
	sButInit.pDisplay = intDisplayImageHilight;
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_TRANSETA_DOWN,
		IMAGE_TRANSETA_UP);
//#ifdef PSX
//	AddCursorSnap(&InterfaceSnap,
//					sFormInit.x+sButInit.x+sButInit.width/2,
//					sFormInit.y+sButInit.y+sButInit.height/2,sButInit.formID);
//#endif
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

#endif
	return TRUE;
}
*/

void missionSetReinforcementTime( UDWORD iTime )
{
	g_iReinforceTime = iTime;
}

UDWORD  missionGetReinforcementTime(void)
{
    return g_iReinforceTime;
}

//fills in a hours(if bHours = TRUE), minutes and seconds display for a given time in 1000th sec
void fillTimeDisplay(STRING *psText, UDWORD time, BOOL bHours)
{
	UDWORD		calcTime, inc;

    inc = 0;

    //this is only for the transporter timer - never have hours!
    if (time == LZ_COMPROMISED_TIME)
    {
        psText[inc++] = (UBYTE)('-');
        psText[inc++] = (UBYTE)('-');
	    //seperator
	    psText[inc++] = (UBYTE)(':');
        psText[inc++] = (UBYTE)('-');
        psText[inc++] = (UBYTE)('-');
	    //terminate the timer text
	    psText[inc] = '\0';
    }
    else
    {
#ifdef PSX
		if(bHours && (time == 0) ) {
	        psText[inc++] = (UBYTE)('-');
	        psText[inc++] = (UBYTE)('-');
		    psText[inc++] = (UBYTE)(':');
	        psText[inc++] = (UBYTE)('-');
	        psText[inc++] = (UBYTE)('-');
		    psText[inc++] = (UBYTE)(':');
	        psText[inc++] = (UBYTE)('-');
	        psText[inc++] = (UBYTE)('-');
		    //terminate the timer text
		    psText[inc] = '\0';
		} else {
#endif
	        if (bHours)
	        {
	            //hours
		        calcTime = time / (60*60*GAME_TICKS_PER_SEC);
		        psText[inc++] = (UBYTE)('0'+ calcTime / 10);
	    	    psText[inc++] = (UBYTE)('0'+ calcTime % 10);
		        time -= (calcTime * (60*60*GAME_TICKS_PER_SEC));
	    	    //seperator
		        psText[inc++] = (UBYTE)(':');
	        }
		    //minutes
		    calcTime = time / (60*GAME_TICKS_PER_SEC);
		    psText[inc++] = (UBYTE)('0'+ calcTime / 10);
		    psText[inc++] = (UBYTE)('0'+ calcTime % 10);
		    time -= (calcTime * (60*GAME_TICKS_PER_SEC));
		    //seperator
		    psText[inc++] = (UBYTE)(':');
		    //seconds
		    calcTime = time / GAME_TICKS_PER_SEC;
		    psText[inc++] = (UBYTE)('0'+ calcTime / 10);
		    psText[inc++] = (UBYTE)('0'+ calcTime % 10);
		    //terminate the timer text
		    psText[inc] = '\0';
#ifdef PSX
		}
#endif
    }
}


//update function for the mission timer
void intUpdateMissionTimer(struct _widget *psWidget, struct _w_context *psContext)
{
	W_LABEL		*Label = (W_LABEL*)psWidget;
	UDWORD		timeElapsed;//, calcTime;
	SDWORD		timeRemaining;

	UNUSEDPARAMETER(psContext);

    //take into account cheating with the mission timer
    //timeElapsed = gameTime - mission.startTime;

    //if the cheatTime has been set, then don't want the timer to countdown until stop cheating
    if (mission.cheatTime)
    {
        timeElapsed = mission.cheatTime - mission.startTime;
    }
    else
    {
	    timeElapsed = gameTime - mission.startTime;
    }
	//check not gone over more than one hour - the mission should have been aborted?
	//if (timeElapsed > 60*60*GAME_TICKS_PER_SEC)
	//check not gone over more than 99 mins - the mission should have been aborted?
    //check not gone over more than 5 hours - arbitary number of hours
    if (timeElapsed > 5*60*60*GAME_TICKS_PER_SEC)
	{
		ASSERT((FALSE,"You've taken too long for this mission!"));
		return;
	}

	timeRemaining = mission.time - timeElapsed;
	if (timeRemaining < 0)
	{
		timeRemaining = 0;
	}

	fillTimeDisplay(Label->aText, timeRemaining, TRUE);

    //make sure its visible
    Label->style &= ~WIDG_HIDDEN;

#ifdef WIN32
    //make timer flash if time remaining < 5 minutes
    if (timeRemaining < FIVE_MINUTES)
    {
        flashMissionButton(IDTIMER_FORM);
    }
    //stop timer from flashing when gets to < 4 minutes
    if (timeRemaining < FOUR_MINUTES)
    {
        stopMissionButtonFlash(IDTIMER_FORM);
    }
    //play audio the first time the timed mission is started
    if (timeRemaining AND (missionCountDown & NOT_PLAYED_ACTIVATED))
    {
        audio_QueueTrack(ID_SOUND_MISSION_TIMER_ACTIVATED);
        missionCountDown &= ~NOT_PLAYED_ACTIVATED;
    }
    //play some audio for mission countdown - start at 10 minutes remaining
    if (getPlayCountDown() AND timeRemaining < TEN_MINUTES)
    {
        if (timeRemaining < TEN_MINUTES AND (missionCountDown & NOT_PLAYED_TEN))
        {
            audio_QueueTrack(ID_SOUND_10_MINUTES_REMAINING);
            missionCountDown &= ~NOT_PLAYED_TEN;
        }
        else if (timeRemaining < FIVE_MINUTES AND (missionCountDown & NOT_PLAYED_FIVE))
        {
            audio_QueueTrack(ID_SOUND_5_MINUTES_REMAINING);
            missionCountDown &= ~NOT_PLAYED_FIVE;
        }
        else if (timeRemaining < THREE_MINUTES AND (missionCountDown & NOT_PLAYED_THREE))
        {
            audio_QueueTrack(ID_SOUND_3_MINUTES_REMAINING);
            missionCountDown &= ~NOT_PLAYED_THREE;
        }
        else if (timeRemaining < TWO_MINUTES AND (missionCountDown & NOT_PLAYED_TWO))
        {
            audio_QueueTrack(ID_SOUND_2_MINUTES_REMAINING);
            missionCountDown &= ~NOT_PLAYED_TWO;
        }
        else if (timeRemaining < ONE_MINUTE AND (missionCountDown & NOT_PLAYED_ONE))
        {
            audio_QueueTrack(ID_SOUND_1_MINUTE_REMAINING);
            missionCountDown &= ~NOT_PLAYED_ONE;
        }
    }

#endif
}

#define	TRANSPORTER_REINFORCE_LEADIN	10*GAME_TICKS_PER_SEC

//update function for the transporter timer
void intUpdateTransporterTimer(struct _widget *psWidget, struct _w_context *psContext)
{
	W_LABEL		*Label = (W_LABEL*)psWidget;
	DROID		*psTransporter;
	SDWORD		timeRemaining;
	SDWORD		ETA;

	UNUSEDPARAMETER(psContext);

	ETA = mission.ETA;
	if(ETA < 0) {
		ETA = 0;
	}

	// Get the object associated with this widget.
	psTransporter = (DROID *)Label->pUserData;
	if (psTransporter != NULL)
	{
		ASSERT((PTRVALID(psTransporter, sizeof(DROID)),
			"intUpdateTransporterTimer: invalid Droid pointer"));

		if (psTransporter->action == DACTION_TRANSPORTIN ||
			psTransporter->action == DACTION_TRANSPORTWAITTOFLYIN )
		{
            if (mission.ETA == LZ_COMPROMISED_TIME)
            {
                timeRemaining = LZ_COMPROMISED_TIME;
            }
            else
            {
			    timeRemaining = mission.ETA - (gameTime - g_iReinforceTime);
			    if (timeRemaining < 0)
			    {
				    timeRemaining = 0;
			    }
			    if (timeRemaining < TRANSPORTER_REINFORCE_LEADIN )
			    {
				    // arrived: tell the transporter to move to the new onworld
				    // location if not already doing so
				    if ( psTransporter->action == DACTION_TRANSPORTWAITTOFLYIN )
				    {
					    missionFlyTransportersIn( selectedPlayer, FALSE );
					    eventFireCallbackTrigger( CALL_TRANSPORTER_REINFORCE );
				    }
			    }
            }
			fillTimeDisplay(Label->aText, timeRemaining, FALSE);
		} else {
			fillTimeDisplay(Label->aText, ETA, FALSE);
		}
	}
	else
	{
		if(missionCanReEnforce()) { // ((mission.type == LDS_MKEEP) || (mission.type == LDS_MCLEAR)) & (mission.ETA >= 0) ) {
			fillTimeDisplay(Label->aText, ETA, FALSE);
		} else {
#ifdef WIN32
			fillTimeDisplay(Label->aText, 0, FALSE);
#else
			fillTimeDisplay(Label->aText, LZ_COMPROMISED_TIME, FALSE);
#endif
		}
	}

	//minutes
	/*calcTime = timeRemaining / (60*GAME_TICKS_PER_SEC);
	Label->aText[0] = (UBYTE)('0'+ calcTime / 10);
	Label->aText[1] = (UBYTE)('0'+ calcTime % 10);
	timeElapsed -= calcTime * (60*GAME_TICKS_PER_SEC);
	//seperator
	Label->aText[3] = (UBYTE)(':');
	//seconds
	calcTime = timeRemaining / GAME_TICKS_PER_SEC;
	Label->aText[3] = (UBYTE)('0'+ calcTime / 10);
	Label->aText[4] = (UBYTE)('0'+ calcTime % 10);*/

	Label->style &= ~WIDG_HIDDEN;
}

/* Remove the Mission Timer widgets from the screen*/
void intRemoveMissionTimer(void)
{
	// Check it's up.
	if(widgGetFromID(psWScreen,IDTIMER_FORM) != NULL)
	{
		//and remove it.
		widgDelete(psWScreen, IDTIMER_FORM);
	}
}

/* Remove the Transporter Timer widgets from the screen*/
void intRemoveTransporterTimer(void)
{
#ifdef WIN32
	//remove main screen
	if(widgGetFromID(psWScreen,IDTRANTIMER_BUTTON) != NULL) 
	{	
    	widgDelete(psWScreen, IDTRANTIMER_BUTTON);
    }
#endif
}


// ////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////
// mission result functions for the interface.


#ifdef WIN32
void intDisplayMissionBackDrop(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	UNUSEDPARAMETER(pColours);
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		// need to do some funky rejigging of the buffer to get upto 16bit+alpha
//		pie_DownLoadBufferToScreen( pMissionBackDrop->bmp,psWidget->x+xOffset,psWidget->y+yOffset,psWidget->width, psWidget->height,1280);
	}
	else
	{
		UNUSEDPARAMETER(yOffset);
		UNUSEDPARAMETER(xOffset);
		UNUSEDPARAMETER(psWidget);
//		iV_DownloadDisplayBuffer(pMissionBackDrop->bmp);
	}
	scoreDataToScreen();
}

#else	// PSX version.

#define ENABLE_BACKDROPS
#define USEPRIMBUFFERS	TRUE

//UBYTE *OldCacheStart;
//UDWORD OldCacheSize;
//UBYTE *BDropData = NULL;
extern void UpdateVRAM(void);

void intDisplayMissionBackDrop(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	UNUSEDPARAMETER(pColours);
	UNUSEDPARAMETER(yOffset);
	UNUSEDPARAMETER(xOffset);
	UNUSEDPARAMETER(psWidget);

	if(bDisplayScores) {
		scoreDataToScreen();
	}
}


static UBYTE *CurrentBackdrop = NULL;
static BOOL	CurrentBackdropIsValid = FALSE;
static BOOL CurrentIsPrimBuffer = FALSE;

void UpdateBackdrop(void)
{
#ifdef ENABLE_BACKDROPS
	if(CurrentBackdrop) {
		TIM_IMAGE TimInfo;
		RECT CurrentImage;

		OpenTIM((void*)CurrentBackdrop);
		ReadTIM(&TimInfo);

		CurrentImage.x=0;
		CurrentImage.y=GetDBIndex()*GetDisplayHeight();
		CurrentImage.w=TimInfo.prect->w;
		CurrentImage.h=TimInfo.prect->h;

		DrawSync(0);
		LoadImage(&CurrentImage,TimInfo.paddr);
		DrawSync(0);
	}
#endif
}


//void DisplayBackdrop(void)
//{
//	if(CurrentBackdrop) {
//		TIM_IMAGE TimInfo;
//		RECT CurrentImage;
//
//		OpenTIM((void*)CurrentBackdrop);
//		ReadTIM(&TimInfo);
//
//		CurrentImage.x=0;
//		CurrentImage.y=0;
//		CurrentImage.w=TimInfo.prect->w;
//		CurrentImage.h=TimInfo.prect->h;
//
//		DrawSync(0);
//		VSync(0);
//		LoadImage(&CurrentImage,TimInfo.paddr);
//		DrawSync(0);
//
//		CurrentImage.y=GetDisplayHeight();
//
//		DrawSync(0);
//		VSync(0);
//		LoadImage(&CurrentImage,TimInfo.paddr);
//		DrawSync(0);
//	}
//}



BOOL GetBackdropActive(void)
{
	return BackdropActive;
}


BOOL IsBackdropLoaded(void)
{
	return CurrentBackdropIsValid;
}

void StartBackdropDisplay(void)
{
	SetDisplaySize(DISPLAY_WIDTH,DISPLAY_HEIGHT);		// flip to lowres for this screen - needed for end of level (was staying in hi-res for results screen)
#ifdef ENABLE_BACKDROPS
	SetVRAMUpdateCallback(UpdateBackdrop);
#endif
	BackdropActive = TRUE;
}


void StopBackdropDisplay(void)
{
#ifdef ENABLE_BACKDROPS
	SetVRAMUpdateCallback(NULL);
#endif
	BackdropActive = FALSE;
}

extern BOOL SetWDGCache(UBYTE *CacheStart, UDWORD CacheSize);
extern BOOL GetWDGCache(UBYTE **CacheStart,UDWORD *CacheSize);
extern BOOL FreeCurrentWDG(void);


#ifdef DISPLAYMODE_PAL
#define BDROPDIR "bdropspal"
#else
#define BDROPDIR "bdrops"
#endif

UBYTE LastBackdropFname[32];

UBYTE *GetLastBackdropName(void)
{
	return LastBackdropFname;
}

// Load a backdrop bitmap from BDROP.WDG
//
// Filename 		backdrop to load.
// UsePrimBuffer	If this is TRUE then the backdrop is stored at the begining of the primitive
//					buffer. In this case you must be carefull that the primitive buffer is not in use
//					when this function is called.
// 					If UsePrimBuffer = FALSE then memory is allocated for the bitmap.
//
// Assumes bitmap to be loaded is a 320 x 256 x 16 bit TIM.
// Assumes WDG cache points to begining of primative buffers.
// Assumes primative buffers are big enought to hold the bitmap.
//
UBYTE *LoadBackdrop(char *FileName,BOOL UsePrimBuffer)
{
	BOOL Result;
	UBYTE *FileData = NULL;
	UDWORD FileSize;
	UBYTE fname[32];
	UBYTE AllocateMode;
	
#ifdef ENABLE_BACKDROPS

	if(CurrentBackdropIsValid) 
	{
		UnloadBackdrop();
	}
	CurrentIsPrimBuffer = UsePrimBuffer;

	DrawSync(0);		// Ensure anything being drawn has finished before continueing.

	// we need to take a backup copy of the backdrop name so that we can restore it if needed
	strcpy(LastBackdropFname,FileName);
	sprintf(fname,"%s\\%s",BDROPDIR,FileName);

/*
	if(UsePrimBuffer==TRUE) 
	{
		// Allocate memory at the beggining of the primative buffer for our picture.
		// This means that the primitive buffer will be smaller but still usable for
		// drawing GPU stuff.
		FileData=AllocInPrimBuffers(320*256*2+128);
		// This will also invalidate the file cache .. and it will need to be reloaded
	}
*/

	DBPRINTF(("Loading backdrop %s useprimbuffer=%d\n",fname,UsePrimBuffer));
	if (UsePrimBuffer==TRUE)
	{
//		AllocateMode=WDG_USESUPPLIED;
		AllocateMode=WDG_RETURNCACHE;	// return the pointer in the cache where the file is stored
	}
	else
	{
		AllocateMode=WDG_ALLOCATEMEM;
	}

	FILE_InvalidateCache();			// This line MUST be included so that we make sure that the backdrop is loaded into cache
	Result = loadFileFromWDG(fname, &FileData, &FileSize, AllocateMode);
	if(Result != TRUE) 
	{
		// Backdrop not in WDG loading from mem
		if (!loadFile(fname, &FileData, &FileSize))
		{
			DBPRINTF(("Error loading %s from WDG (%d)\n",fname,Result));
			FileData = NULL;
		}

	}


	if(UsePrimBuffer==TRUE) 
	{
		UBYTE *FileStart;		// Where in the cache is the file stored
		// Allocate memory at the beggining of the primative buffer for our picture.
		// This means that the primitive buffer will be smaller but still usable for
		// drawing GPU stuff.
#define BACKDROPSIZE (320*256*2+128)

		FileStart=FileData;	 //
		FileData=AllocInPrimBuffers(BACKDROPSIZE);

//		// Fill the source bitmap for testing.
//		{
//			UDWORD i;
//			for(i=20; i<BACKDROPSIZE; i++) {
//				FileStart[i] = 0x1f;
//			}
//		}

		// memmove must be used because both are in the primative buffer area ... it should get sorted out though
DBPRINTF(("filedata=%p filestart=%p size=%p\n",FileData,FileStart,BACKDROPSIZE));
		memmove(FileData,FileStart,BACKDROPSIZE);	   // copy the data from the cache into the start of the primative buffer
		FILE_InvalidateCache();
	}


	CurrentBackdrop = FileData;
	CurrentBackdropIsValid = TRUE;

	DBPRINTF(("OK\n",FileName));
#endif
	return FileData;
}


// Unload a backdrop bitmap.
// if the bitmap was loaded with UsePrimBuffer = TRUE then call
// this function with UsePrimBuffer = TRUE.
//
BOOL UnloadBackdrop(void)
{
#ifdef ENABLE_BACKDROPS
	if(CurrentBackdropIsValid) {
		DBPRINTF(("Unloading backdrop\n"));
		if(CurrentIsPrimBuffer) {
			DrawSync(0);		// Ensure anything being drawn has finished before continueing.
			ResetPrimBuffers();
		} else {
			if(CurrentBackdrop) {
				FREE(CurrentBackdrop);
			}
		}

		CurrentBackdropIsValid = FALSE;
	}
#endif
	return TRUE;
}

#endif

void missionResetInGameState( void )
{
	//stop the game if in single player mode
	setMissionPauseState();

	// reset the input state
	resetInput();

	// Add the background
	// get rid of reticule etc..
	intResetScreen(FALSE);
	//intHidePowerBar();
	forceHidePowerBar();
	intRemoveReticule();
	intRemoveMissionTimer();

#ifdef PSX
	if(GetControllerType(0) == CON_MOUSE) {
		// If were using a mouse then set the cursor to arrow
		pie_SetMouse(IntImages,IMAGE_CURSOR_DEFAULT);
	} else {
		// Otherwise, ensure the cursor is hidden and mouse movement is disabled.
		EnableMouseDraw(FALSE);
		MouseMovement(FALSE);
	}
#endif
}

static BOOL _intAddMissionResult(BOOL result, BOOL bPlaySuccess)
{
	W_FORMINIT		sFormInit;
	W_LABINIT		sLabInit;
	W_BUTINIT		sButInit;
	UDWORD			fileSize=0;

	missionResetInGameState();

	memset(&sFormInit, 0, sizeof(W_FORMINIT));


#ifdef WIN32

	// add some funky beats
	cdAudio_PlayTrack(2);	// 2= frontend music.


	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		pie_LoadBackDrop(SCREEN_MISSIONEND,TRUE);
	}
	else
	{
		pie_LoadBackDrop(SCREEN_MISSIONEND,FALSE);
	}

#else

	LoadBackdrop("frontend.tim",USEPRIMBUFFERS);
	StartBackdropDisplay();
#endif

	sFormInit.formID		= 0;
	sFormInit.id			= IDMISSIONRES_BACKFORM;
	sFormInit.style			= WFORM_PLAIN;
	sFormInit.x				= (SWORD)(0 + D_W);
	sFormInit.y				= (SWORD)(0 + D_H);
	sFormInit.width			= 640;
	sFormInit.height		= 480;
	sFormInit.pDisplay		= intDisplayMissionBackDrop;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}
#ifdef PSX
	bDisplayScores = TRUE;
#endif
//#endif

	// TITLE
	sFormInit.formID		= IDMISSIONRES_BACKFORM;
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
//	sFormInit.formID		= 0;
#endif
	sFormInit.id			= IDMISSIONRES_TITLE;
	sFormInit.style			= WFORM_PLAIN;
	sFormInit.x				= MISSIONRES_TITLE_X;
	sFormInit.y				= MISSIONRES_TITLE_Y;
	sFormInit.width			= MISSIONRES_TITLE_W;
	sFormInit.height		= MISSIONRES_TITLE_H;
	sFormInit.disableChildren = TRUE;
	sFormInit.pDisplay		= intOpenPlainForm;	//intDisplayPlainForm;
// removed for more space
//#ifndef WIN32
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}
//#endif

	// add form 
	sFormInit.formID		= IDMISSIONRES_BACKFORM;
#ifdef PSX
	WidgSetOTIndex(OT2D_BACK);
//	sFormInit.formID		= 0;
#endif
	sFormInit.id			= IDMISSIONRES_FORM;
	sFormInit.style			= WFORM_PLAIN;
	sFormInit.x				= MISSIONRES_X;
	sFormInit.y				= MISSIONRES_Y;
	sFormInit.width			= MISSIONRES_W;
	sFormInit.height		= MISSIONRES_H;
	sFormInit.disableChildren = TRUE;
	sFormInit.pDisplay		= intOpenPlainForm;	//intDisplayPlainForm;
	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

#ifdef PSX
	WidgSetOTIndex(OT2D_FORE);
#endif
	// description of success/fail
	memset(&sLabInit,0,sizeof(W_LABINIT));
	sLabInit.formID = IDMISSIONRES_TITLE;
	sLabInit.id = IDMISSIONRES_TXT;
	sLabInit.style = WLAB_PLAIN | WLAB_ALIGNCENTRE;
	sLabInit.x = 0;
	sLabInit.y = 12;
	sLabInit.width = MISSIONRES_TITLE_W;
	sLabInit.height = 16;
	if(result)
	{	
#ifdef WIN32
        //don't bother adding the text if haven't played the audio
        if (bPlaySuccess)
        {
		    sLabInit.pText = strresGetString(psStringRes,STR_MR_OBJECTIVE_ACHIEVED);//"Objective Achieved";
        }
#else	// might want to do this on PC as well. Apparently not!
		// If the've won the game then..
		if(testPlayerHasWon()) {
			// In fastplay say "Practice Complete"
			if(GetInFastPlay()) {
	  			sLabInit.pText = strresGetString(psStringRes,STR_MR_PRACTICE_COMPLETE);
			} else {
				// In the real game say "Victory"
	  			sLabInit.pText = strresGetString(psStringRes,STR_MR_VICTORY);
			}
		} else {
  			sLabInit.pText = strresGetString(psStringRes,STR_MR_OBJECTIVE_ACHIEVED);//"Objective Achieved";
		}
#endif
	}
	else
	{
	  	sLabInit.pText = strresGetString(psStringRes,STR_MR_OBJECTIVE_FAILED);//"Objective Failed;
	}
	sLabInit.FontID = WFont;
//removed for more space
//#ifndef WIN32
	if (!widgAddLabel(psWScreen, &sLabInit))
	{
		return FALSE;
	}
//#endif
	// options.
	memset(&sButInit,0,sizeof(W_BUTINIT));
	sButInit.formID		= IDMISSIONRES_FORM;
	sButInit.style		= WBUT_PLAIN | WBUT_TXTCENTRE;
	sButInit.width		= MISSION_TEXT_W;
	sButInit.height		= MISSION_TEXT_H;
	sButInit.FontID		= WFont;
	sButInit.pTip		= NULL;
	sButInit.pDisplay	= displayTextOption;
    //if won or in debug mode
	if(result OR getDebugMappingStatus())
	{
		//continue
		sButInit.x			= MISSION_2_X;
        // Won the game, so display "Quit to main menu"
		if(testPlayerHasWon()) 
        {		
			sButInit.id			= IDMISSIONRES_QUIT;
			sButInit.y			= MISSION_2_Y-8;
			sButInit.pText		= strresGetString(psStringRes,STR_MR_QUIT_TO_MAIN);
			widgAddButton(psWScreen, &sButInit);
			intSetCurrentCursorPosition(&InterfaceSnap,sButInit.id);
		} 
        else 
        {
			// Finished the mission, so display "Continue Game"
			if(!GetInFastPlay())	// If in fast play then no save option so move up a bit.
			{
				sButInit.y			= MISSION_2_Y;
			} else {
				sButInit.y			= MISSION_2_Y-16;
			}
			sButInit.id			= IDMISSIONRES_CONTINUE;
			sButInit.pText		= strresGetString(psStringRes,STR_MR_CONTINUE);//"Continue Game";
			widgAddButton(psWScreen, &sButInit);
			intSetCurrentCursorPosition(&InterfaceSnap,sButInit.id);
		}

#ifndef COVERMOUNT
		/* Only add save option if in the game for real, ie, not fastplay. 
        And the player hasn't just completed the whole game
        Don't add save option if just lost and in debug mode*/
		if(!GetInFastPlay() AND !testPlayerHasWon() AND 
            !(testPlayerHasLost() AND getDebugMappingStatus()))
		{
			//save
			sButInit.id			= IDMISSIONRES_SAVE;
			sButInit.x			= MISSION_1_X;
			sButInit.y			= MISSION_1_Y;
			sButInit.pText		= strresGetString(psStringRes,STR_MR_SAVE_GAME);//"Save Game";
			widgAddButton(psWScreen, &sButInit);
			intSetCurrentCursorPosition(&InterfaceSnap,sButInit.id);
		}
#endif


	}
	else
	{
#ifdef WIN32
#ifndef COVERMOUNT
		//load
		sButInit.id			= IDMISSIONRES_LOAD;
		sButInit.x			= MISSION_1_X;
		sButInit.y			= MISSION_1_Y;
		sButInit.pText		= strresGetString(psStringRes,STR_MR_LOAD_GAME);//"Load Saved Game";
		widgAddButton(psWScreen, &sButInit);
		intSetCurrentCursorPosition(&InterfaceSnap,sButInit.id);
#endif		//quit
		sButInit.id			= IDMISSIONRES_QUIT;
		sButInit.x			= MISSION_2_X;
		sButInit.y			= MISSION_2_Y;
		sButInit.pText		= strresGetString(psStringRes,STR_MR_QUIT_TO_MAIN);//"Quit to Main Menu";
		widgAddButton(psWScreen, &sButInit);
#else	// No load option on Playstation.
		//quit
		sButInit.id			= IDMISSIONRES_QUIT;
		sButInit.x			= MISSION_2_X;
		sButInit.y			= MISSION_2_Y-22;
		sButInit.pText		= strresGetString(psStringRes,STR_MR_QUIT_TO_MAIN);//"Quit to Main Menu";
		widgAddButton(psWScreen, &sButInit);
		intSetCurrentCursorPosition(&InterfaceSnap,sButInit.id);
#endif
	}

	intMode		= INT_MISSIONRES;
	MissionResUp = TRUE;

#ifdef WIN32
	/* play result audio */
	if ( result == TRUE AND bPlaySuccess)
	{
		audio_QueueTrack( ID_SOUND_OBJECTIVE_ACCOMPLISHED );
	}
#endif

	return TRUE;
}


BOOL intAddMissionResult(BOOL result, BOOL bPlaySuccess)
{
	/* save result */
	g_bMissionResult = result;

#ifdef PSX
	// If the stacks in the dcache then..
	if(SpInDCache()) {
		static BOOL ret;
		// Set the stack pointer to point to the alternative stack which is'nt limited to 1k.
		SetSpAlt();
		// Only one parameter so it will be in a register not on the stack so
		// we don't need to copy into a static.
		ret = _intAddMissionResult(result, bPlaySuccess);
		SetSpAltNormal();
		return ret;
	}
#endif
	return _intAddMissionResult(result, bPlaySuccess);
}




void intRemoveMissionResultNoAnim(void)
{
	widgDelete(psWScreen, IDMISSIONRES_TITLE);
	widgDelete(psWScreen, IDMISSIONRES_FORM);
	widgDelete(psWScreen, IDMISSIONRES_BACKFORM);		

#ifdef WIN32
	 cdAudio_Stop();
#else
	StopBackdropDisplay();
	UnloadBackdrop();
	DBPRINTF(("Backdrop released\n"));
#endif

	MissionResUp	 	= FALSE;
	ClosingMissionRes   = FALSE;
	intMode				= INT_NORMAL;

	//reset the pauses
	resetMissionPauseState();

#ifdef PSX
	CancelInterfaceSnap();
	if(GetControllerType(0) == CON_MOUSE) {
		intAddReticule();
	}
#else
	// add back the reticule and power bar.
	intAddReticule();
#endif
	intShowPowerBar();

//	EnableMouseDraw(TRUE);
//	MouseMovement(TRUE);
}


void intRunMissionResult()
{						
#ifdef PSX						//cursor snap stuff.
	ProcessCursorSnap();
#else
	processFrontendSnap(FALSE);
	pie_SetMouse(IntImages,IMAGE_CURSOR_DEFAULT);
	frameSetCursorFromRes(IDC_DEFAULT);
#endif

#ifdef WIN32
	if(bLoadSaveUp)
	{
		if(runLoadSave(FALSE))// check for file name.
		{
			if(strlen(sRequestResult))
			{
				DBPRINTF(("Returned %s",sRequestResult));
#ifdef WIN32 // bRequestLoad is not set up correctly on the psx ... so we'll take it out this check (we will always save)

				if(bRequestLoad)
				{
//					loadGame(		);
				}
				else
#endif
				{
					saveGame(sRequestResult,GTYPE_SAVE_START);
		            addConsoleMessage(strresGetString(psStringRes, STR_GAME_SAVED), LEFT_JUSTIFY);		
				}
			}
		}
	}
#endif

}

void missionCDCancelPressed( void )
{
	intAddMissionResult( g_bMissionResult, TRUE );
}

void missionContineButtonPressed( void )
{
	//SHOULDN'T BE ABLE TO BE ANY OTHER TYPE AT PRESENT!
	// start the next mission if necessary
	// otherwise wait for the Launch button to be pressed
	//if (nextMissionType == MISSION_CAMPSTART OR nextMissionType == 
	//	MISSION_CAMPEXPAND OR nextMissionType == MISSION_BETWEEN)

	if (nextMissionType == LDS_CAMSTART
		OR nextMissionType == LDS_BETWEEN 
#ifndef COVERMOUNT
		OR nextMissionType == LDS_EXPAND 
		OR nextMissionType == LDS_EXPAND_LIMBO
#endif
	)
	{
        //if we're moving from cam2-cam3?
		launchMission();
	}
	widgDelete(psWScreen,IDMISSIONRES_FORM);	//close option box.
	//if (nextMissionType == MISSION_OFFKEEP OR nextMissionType == MISSION_OFFCLEAR)
	/*if (nextMissionType == MISSION_BETWEEN)
	{
		intRemoveMissionResultNoAnim();
	}*/
//	intRemoveMissionResultNoAnim();
    

    //just being paranoid here - definately don't want this in the final build
#ifdef DEBUG
    //in the final build we won't be able to get here beciase the CONTINUE button will not be available
    //if we're in debug mode and we've lost it is preferred if we just return to the game
    if (getDebugMappingStatus() AND testPlayerHasLost())
    {
        intRemoveMissionResultNoAnim();
    }
#endif

#ifdef PSX
	widgDelete(psWScreen, IDMISSIONRES_TITLE);
	widgDelete(psWScreen, IDMISSIONRES_BACKFORM);
	// We must release the backdrop here on the PSX as it uses the
	// primitave buffer which is neaded for WDG loading.
	StopBackdropDisplay();
	UnloadBackdrop();
	DBPRINTF(("Backdrop released2\n"));
	// Clear the screen.
//	ClearDisplayBuffers();
//	initLoadingScreen(FALSE,FALSE);
 #ifdef LOADINGBACKDROPS
	AddLoadingBackdrop(TRUE);
 #else
	initLoadingScreen(FALSE,FALSE);
 #endif
#endif
}

void intProcessMissionResult(UDWORD id)
{
	W_BUTINIT	sButInit;
#ifdef WIN32
	CD_INDEX	CDrequired;
#endif

#ifdef WIN32
	/* GJ to TC - this call processes the CD change widget box */
	if ( !cdspan_ProcessCDChange(id) )
#endif
	switch(id)
	{

#ifdef WIN32
	case IDMISSIONRES_LOAD:
		// throw up some filerequester
		addLoadSave(LOAD_MISSIONEND,"savegame\\","gam",strresGetString(psStringRes,STR_MR_LOAD_GAME)/*"Load Game"*/);
		break;
	case IDMISSIONRES_SAVE:
		addLoadSave(SAVE_MISSIONEND,"savegame\\","gam",strresGetString(psStringRes,STR_MR_SAVE_GAME)/*"Save Game"*/);

		if (widgGetFromID(psWScreen, IDMISSIONRES_QUIT) == NULL)
		{
			//Add Quit Button now save has been pressed
			memset(&sButInit,0,sizeof(W_BUTINIT));
			sButInit.formID		= IDMISSIONRES_FORM;
			sButInit.style		= WBUT_PLAIN | WBUT_TXTCENTRE;
			sButInit.width		= MISSION_TEXT_W;
			sButInit.height		= MISSION_TEXT_H;
			sButInit.FontID		= WFont;
			sButInit.pTip		= NULL;
			sButInit.pDisplay	= displayTextOption;
			sButInit.id			= IDMISSIONRES_QUIT;
			sButInit.x			= MISSION_3_X;
			sButInit.y			= MISSION_3_Y;
			sButInit.pText		= strresGetString(psStringRes,STR_MR_QUIT_TO_MAIN);
			widgAddButton(psWScreen, &sButInit);
		}
		break;
#else
//	case IDMISSIONRES_LOAD:
//		// throw up some filerequester
//		addLoadSave(TRUE,"savegame\\","gam",strresGetString(psStringRes,STR_MR_LOAD_GAME)/*"Load Game"*/);
//		break;
	case IDMISSIONRES_SAVE:
#if(1)
		widgDelete(psWScreen, IDMISSIONRES_TITLE);
		widgDelete(psWScreen, IDMISSIONRES_FORM);
		widgDelete(psWScreen, IDMISSIONRES_BACKFORM);		
		StopBackdropDisplay();
		UnloadBackdrop();
		MissionResUp	 	= FALSE;
		ClosingMissionRes   = FALSE;
		intMode				= INT_NORMAL;

		intDoLoadSave(TRUE);

		resetMissionPauseState();		// Don't ask.
		intAddMissionResult(TRUE, TRUE);
#else
		addLoadSave(FALSE,"savegame\\","gam",strresGetString(psStringRes,STR_MR_SAVE_GAME)/*"Save Game"*/);
#endif
		break;
#endif

	case IDMISSIONRES_QUIT:
#ifdef PSX
	bDisplayScores = FALSE;
#endif
		// catered for by hci.c.
		break;

	case IDMISSIONRES_CONTINUE:
#ifdef WIN32
		if(bLoadSaveUp)
		{
			closeLoadSave();				// close save interface if it's up.
		}

		/* check correct CD in drive */
		CDrequired = getCDForCampaign( getCampaignNumber() );
		if ( cdspan_CheckCDPresent( CDrequired ) )
#endif
		{
			missionContineButtonPressed();
		}
#ifdef WIN32
		else
		{
			widgDelete(psWScreen, IDMISSIONRES_TITLE);
			widgDelete(psWScreen, IDMISSIONRES_FORM);
			widgDelete(psWScreen, IDMISSIONRES_BACKFORM);		
			showChangeCDBox( psWScreen, CDrequired,
				missionContineButtonPressed, missionCDCancelPressed );
		}
#endif	
#ifdef PSX
		bDisplayScores = FALSE;
#endif
		break;

	default:
		break;
	}
}

// end of interface stuff.
// ////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////

/*builds a droid back at the home base whilst on a mission - stored in a list made
available to the transporter interface*/
DROID * buildMissionDroid(DROID_TEMPLATE *psTempl, UDWORD x, UDWORD y, 
						  UDWORD player)
{
	DROID		*psNewDroid;

	psNewDroid = buildDroid(psTempl, x << TILE_SHIFT, y << TILE_SHIFT, 
		player, TRUE);
	if (!psNewDroid)
	{
		return NULL;
	}
	//addDroid(psNewDroid, mission.apsBuiltDroids);
	addDroid(psNewDroid, mission.apsDroidLists);
	//set its x/y to impossible values so can detect when return from mission
	psNewDroid->x = INVALID_XY;
	psNewDroid->y = INVALID_XY;

	//set all the droids to selected from when return
	psNewDroid->selected = TRUE;
	
	return psNewDroid;
}

//this causes the new mission data to be loaded up - only if startMission has been called
void launchMission(void)
{
	//if (mission.type == MISSION_NONE)
	if (mission.type == LDS_NONE)
	{
		// tell the loop that a new level has to be loaded up
		loopMissionState = LMS_NEWLEVEL;
	}
	else
	{
		DBMB(("Start Mission has not been called"));
	}
}

#ifdef WIN32
void intCDOK( void )
{
//#ifdef PSX
//		// Clear the screen.
//		initLoadingScreen(FALSE,FALSE);
//#endif
		resetMissionPauseState();
		intAddReticule();
		intShowPowerBar();

		launchMission();
}

void intCDCancel( void )
{
	/* do nothing - dealt with in HCI */
}
#endif

//sets up the game to start a new mission
//BOOL setUpMission(MISSION_TYPE type)
BOOL setUpMission(UDWORD type)
{
/*#ifdef WIN32
	CD_INDEX	CDrequired;
#endif*/

	//MISSION_TYPE	oldMission;

	//UNUSEDPARAMETER(type);

	//close the interface
	intResetScreen(TRUE);

	//oldMission = mission.type;
	/*the last mission must have been successful otherwise endgame would have 
	been called*/
	endMission();
	
	//release the level data for the previous mission
	if (!levReleaseMissionData())
	{
		return FALSE;
	}

	//if (type == MISSION_OFFCLEAR OR type == MISSION_OFFKEEP)
	if ( type == LDS_CAMSTART )
	{
#ifdef PSX
//		intCDOK();
		intAddMissionResult(TRUE, TRUE);	// Assume you've succeded if you get here.
		loopMissionState = LMS_SAVECONTINUE;
#else
        //this cannot be called here since we need to be able to save the game at the end of cam1 and cam2
		/*CDrequired = getCDForCampaign( getCampaignNumber() );
		if ( cdspan_CheckCDPresent(CDrequired) )*/
		{
            //another one of those lovely hacks!!
            BOOL    bPlaySuccess = TRUE;

            //we don't want the 'mission accomplished' audio/text message at end of cam1
            if (getCampaignNumber() == 2)
            {
                bPlaySuccess = FALSE;
            }
    		//give the option of save/continue
	    	if (!intAddMissionResult(TRUE, bPlaySuccess))
		    {
			    return FALSE;
		    }
		    loopMissionState = LMS_SAVECONTINUE;
			//intCDOK(); - do this later - in missionContineButtonPressed() to be exact
		}
		/*else
		{
			if(!getWidgetsStatus())
			{
				setWidgetsStatus(TRUE);
				intResetScreen(FALSE);
			}
			missionResetInGameState();
			addCDChangeInterface( CDrequired, intCDOK, intCDCancel );
			loopMissionState = LMS_SAVECONTINUE;
		}*/
#endif
	}
	else if (type == LDS_MKEEP  
#ifndef COVERMOUNT
		OR type == LDS_MCLEAR
		OR type == LDS_MKEEP_LIMBO
#endif
		)
	{
 #ifdef PSX
	// Clear the screen.
  #ifdef LOADINGBACKDROPS
	AddLoadingBackdrop(TRUE);
  #else
	initLoadingScreen(FALSE,FALSE);
  #endif
//		ClearDisplayBuffers();
 #endif
		launchMission();

	}
	else
	{
		if(!getWidgetsStatus())
		{
			setWidgetsStatus(TRUE);
			intResetScreen(FALSE);
		}
		//give the option of save/continue
		if (!intAddMissionResult(TRUE, TRUE))
		{
			return FALSE;
		}
		loopMissionState = LMS_SAVECONTINUE;
	}

	//if current mission is 'between' then don't give option to save/continue again
	/*if (oldMission != MISSION_BETWEEN)
	{
		//give the option of save/continue
		if (!intAddMissionResult(TRUE))
		{
			return FALSE;
		}
	}*/

	return TRUE;
}

//save the power settings before loading in the new map data
void saveMissionPower(void)
{
	UDWORD	inc;

	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		//mission.asPower[inc].initialPower = asPower[inc]->initialPower;
		mission.asPower[inc].extractedPower = asPower[inc]->extractedPower;
		mission.asPower[inc].currentPower = asPower[inc]->currentPower;
	}
}

//add the power from the home base to the current power levels for the mission map
void adjustMissionPower(void)
{
	UDWORD	inc;

	for (inc = 0; inc < MAX_PLAYERS; inc++)
	{
		//asPower[inc]->initialPower += mission.asPower[inc].initialPower;
		asPower[inc]->extractedPower += mission.asPower[inc].extractedPower;
		asPower[inc]->currentPower += mission.asPower[inc].currentPower;
	}
}

/*sets the appropriate pause states for when the interface is up but the 
game needs to be paused*/
void setMissionPauseState(void)
{
#ifdef WIN32
	if (!bMultiPlayer)
	{
#endif

		gameTimeStop();
		setGameUpdatePause(TRUE);
		setAudioPause(TRUE);
		setScriptPause(TRUE);
		setConsolePause(TRUE);

#ifdef WIN32
	}
#endif
}

/*resets the pause states */
void resetMissionPauseState(void)
{
#ifdef WIN32
	if (!bMultiPlayer)
	{
#endif

		setGameUpdatePause(FALSE);
		setAudioPause(FALSE);
		setScriptPause(FALSE);
		setConsolePause(FALSE);
		gameTimeStart();

#ifdef WIN32
	}
#endif
}

//gets the coords for a no go area
LANDING_ZONE* getLandingZone(SDWORD i)
{
	ASSERT(((i >= 0) && (i < MAX_NOGO_AREAS), "getLandingZone out of range."));
	return &sLandingZone[i];
}

/*Initialises all the nogo areas to 0 - DOESN'T INIT THE LIMBO AREA because we 
have to set this up in the mission BEFORE*/
void initNoGoAreas(void)
{
	UBYTE	i;

	for (i = 0; i < MAX_NOGO_AREAS; i++)
	{
        if (i != LIMBO_LANDING)
        {
		    sLandingZone[i].x1 = sLandingZone[i].y1 = sLandingZone[i].x2 = 
			    sLandingZone[i].y2 = 0;
        }
	}
}

//sets the coords for the Transporter to land (for player 0 - selectedPlayer)
void setLandingZone(UBYTE x1, UBYTE y1, UBYTE x2, UBYTE y2)
{
	//quick check that x2 > x1 and y2 > y1
 	if (x2 < x1)
	{
		sLandingZone[0].x1 = x2;
		sLandingZone[0].x2 = x1;
	}
	else
	{
		sLandingZone[0].x1 = x1;
		sLandingZone[0].x2 = x2;
	}
	if (y2 < y1)
	{
		sLandingZone[0].y1 = y2;
		sLandingZone[0].y2 = y1;
	}
	else
	{
		sLandingZone[0].y1 = y1;
		sLandingZone[0].y2 = y2;
	}

#ifdef WIN32
		if(pie_Hardware())
		{
			addLandingLights(getLandingX(0)+64,getLandingY(0)+64);
		}
#else
		addLandingLights(getLandingX(0)+64,getLandingY(0)+64);
#endif
}

//sets the coords for a no go area
void setNoGoArea(UBYTE x1, UBYTE y1, UBYTE x2, UBYTE y2, UBYTE area)
{
	//quick check that x2 > x1 and y2 > y1
	if (x2 < x1)
	{
		sLandingZone[area].x1 = x2;
		sLandingZone[area].x2 = x1;
	}
	else
	{
		sLandingZone[area].x1 = x1;
		sLandingZone[area].x2 = x2;
	}
	if (y2 < y1)
	{
		sLandingZone[area].y1 = y2;
		sLandingZone[area].y2 = y1;
	}
	else
	{
		sLandingZone[area].y1 = y1;
		sLandingZone[area].y2 = y2;
	}

#ifdef WIN32
		if(area==0 AND pie_Hardware())
		{
			addLandingLights(getLandingX(area)+64,getLandingY(area)+64);
		}
#else
		if(area == 0) {
			addLandingLights(getLandingX(area)+64,getLandingY(area)+64);
		}
#endif

}

void addLandingLights( UDWORD x, UDWORD y)
{
iVector	pos;

	pos.x = x;
	pos.z = y;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_MIDDLE);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,TRUE); //middle
	pos.x=x+128;
	pos.z=y+128;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_OUTER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE); //outer
	pos.x=x+128;
	pos.z=y-128;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_OUTER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE);
	pos.x=x-128;
	pos.z=y+128;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_OUTER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE);
	pos.x=x-128;
	pos.z=y-128;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_OUTER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE);
	pos.x=x+64;
	pos.z=y+64;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_INNER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE); // inner
	pos.x=x+64;
	pos.z=y-64;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_INNER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE);
	pos.x=x-64;
	pos.z=y+64;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_INNER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE);
	pos.x=x-64;
	pos.z=y-64;
	pos.y = map_Height(pos.x,pos.z) +16;
	effectSetLandLightSpec(LL_INNER);
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LAND_LIGHT,FALSE,NULL,FALSE);
}

/*	checks the x,y passed in are not within the boundary of any Landing Zone
	x and y in tile coords*/
BOOL withinLandingZone(UDWORD x, UDWORD y)
{
	UDWORD		inc;

	ASSERT(( x < mapWidth, "withinLandingZone: x coord bigger than mapWidth"));
	ASSERT(( y < mapHeight, "withinLandingZone: y coord bigger than mapHeight"));


	for (inc = 0; inc < MAX_NOGO_AREAS; inc++)
	{
		if ((x >= (UDWORD)sLandingZone[inc].x1 AND x <= (UDWORD)sLandingZone[inc].x2) AND 
			(y >= (UDWORD)sLandingZone[inc].y1 AND y <= (UDWORD)sLandingZone[inc].y2))
		{
			return TRUE;
		}
	}
	return FALSE;
}

//returns the x coord for where the Transporter can land (for player 0)
UWORD getLandingX( SDWORD iPlayer )
{
	ASSERT( (iPlayer<MAX_NOGO_AREAS, "getLandingX: player %i out of range") );
	return (UWORD)((sLandingZone[iPlayer].x1 + (sLandingZone[iPlayer].x2 - 
		sLandingZone[iPlayer].x1)/2) << TILE_SHIFT);
}

//returns the y coord for where the Transporter can land
UWORD getLandingY( SDWORD iPlayer )
{
	ASSERT( (iPlayer<MAX_NOGO_AREAS, "getLandingY: player %i out of range") );
	return (UWORD)((sLandingZone[iPlayer].y1 + (sLandingZone[iPlayer].y2 - 
		sLandingZone[iPlayer].y1)/2) << TILE_SHIFT);
}

//returns the x coord for where the Transporter can land back at home base
UDWORD getHomeLandingX(void)
{
	//return ((mission.homeLZ.x1 + (mission.homeLZ.x2 - 
	//	mission.homeLZ.x1)/2) << TILE_SHIFT);
    return (mission.homeLZ_X >> TILE_SHIFT);
}

//returns the y coord for where the Transporter can land back at home base
UDWORD getHomeLandingY(void)
{
	//return ((mission.homeLZ.y1 + (mission.homeLZ.y2 - 
	//	mission.homeLZ.y1)/2) << TILE_SHIFT);
    return (mission.homeLZ_Y >> TILE_SHIFT);
}

void missionSetTransporterEntry( SDWORD iPlayer, SDWORD iEntryTileX, SDWORD iEntryTileY )
{
	ASSERT( (iPlayer<MAX_PLAYERS, "missionSetTransporterEntry: player %i too high", iPlayer) );

	if( (iEntryTileX > scrollMinX) && (iEntryTileX < scrollMaxX) )
	{
		mission.iTranspEntryTileX[iPlayer] = (UWORD) iEntryTileX;
	}
	else
	{
		DBPRINTF( ("missionSetTransporterEntry: entry point x %i outside scroll limits %i->%i\n",
					iEntryTileX, scrollMinX, scrollMaxX ) );
		mission.iTranspEntryTileX[iPlayer] = (UWORD) (scrollMinX + EDGE_SIZE);
	}

	if( (iEntryTileY > scrollMinY) && (iEntryTileY < scrollMaxY) )
	{
		mission.iTranspEntryTileY[iPlayer] = (UWORD) iEntryTileY;
	}
	else
	{
		DBPRINTF( ("missionSetTransporterEntry: entry point y %i outside scroll limits %i->%i\n",
					iEntryTileY, scrollMinY, scrollMaxY ) );
		mission.iTranspEntryTileY[iPlayer] = (UWORD) (scrollMinY + EDGE_SIZE);
	}
}

void missionSetTransporterExit( SDWORD iPlayer, SDWORD iExitTileX, SDWORD iExitTileY )
{
	ASSERT( (iPlayer<MAX_PLAYERS, "missionSetTransporterExit: player %i too high", iPlayer) );

	if( (iExitTileX > scrollMinX) && (iExitTileX < scrollMaxX) )
	{
		mission.iTranspExitTileX[iPlayer] = (UWORD) iExitTileX;
	}
	else
	{
		DBPRINTF( ("missionSetTransporterExit: entry point x %i outside scroll limits %i->%i\n",
					iExitTileX, scrollMinX, scrollMaxX ) );
		mission.iTranspExitTileX[iPlayer] = (UWORD) (scrollMinX + EDGE_SIZE);
	}

	if( (iExitTileY > scrollMinY) && (iExitTileY < scrollMaxY) )
	{
		mission.iTranspExitTileY[iPlayer] = (UWORD) iExitTileY;
	}
	else
	{
		DBPRINTF( ("missionSetTransporterExit: entry point y %i outside scroll limits %i->%i\n",
					iExitTileY, scrollMinY, scrollMaxY ) );
		mission.iTranspExitTileY[iPlayer] = (UWORD) (scrollMinY + EDGE_SIZE);
	}
}

void missionGetTransporterEntry( SDWORD iPlayer, UWORD *iX, UWORD *iY )
{
	ASSERT( (iPlayer<MAX_PLAYERS, "missionGetTransporterEntry: player %i too high", iPlayer) );

	*iX = (UWORD) (mission.iTranspEntryTileX[iPlayer] << TILE_SHIFT);
	*iY = (UWORD) (mission.iTranspEntryTileY[iPlayer] << TILE_SHIFT);
}

void missionGetTransporterExit( SDWORD iPlayer, UWORD *iX, UWORD *iY )
{
	ASSERT( (iPlayer<MAX_PLAYERS, "missionGetTransporterExit: player %i too high", iPlayer) );

	*iX = (UWORD) (mission.iTranspExitTileX[iPlayer] << TILE_SHIFT);
	*iY = (UWORD) (mission.iTranspExitTileY[iPlayer] << TILE_SHIFT);
}

/*update routine for mission details */
void missionTimerUpdate(void)
{
    //don't bother with the time check if have 'cheated'
    if (!mission.cheatTime)
    {
        //Want a mission timer on all types of missions now - AB 26/01/99
	    //only interested in off world missions (so far!) and if timer has been set
	    if (mission.time >= 0 ) //AND (
            //mission.type == LDS_MKEEP OR mission.type == LDS_MKEEP_LIMBO OR 
            //mission.type == LDS_MCLEAR OR mission.type == LDS_BETWEEN))
	    {
		    //check if time is up
		    if ((SDWORD)(gameTime - mission.startTime) > mission.time)
		    {
			    //the script can call the end game cos have failed!
			    eventFireCallbackTrigger(CALL_MISSION_TIME);
		    }
	    }
    }
}


// Remove any objects left ie walls,structures and droids that are not the selected player.
//
void missionDestroyObjects(void)
{
	DROID *psDroid;
	STRUCTURE *psStruct;
	UBYTE Player;

	DBPRINTF(("missionDestroyObjects\n"));
	for(Player = 0; Player < MAX_PLAYERS; Player++) {
		if(Player != selectedPlayer) {

			psDroid = apsDroidLists[Player];

			while(psDroid != NULL) {
				DROID *psNext = psDroid->psNext;
				removeDroidBase(psDroid);
//				droidRemove(psDroid, apsDroidLists);
//				droidRelease(psDroid);
//				HEAP_FREE(psDroidHeap, psDroid);
				psDroid = psNext;
			}

            //clear out the mission lists as well to make sure no Tranporters exist
            apsDroidLists[Player] = mission.apsDroidLists[Player];
			psDroid = apsDroidLists[Player];

			while(psDroid != NULL) {
				DROID *psNext = psDroid->psNext;
                //make sure its died flag is not set since we've swapped the apsDroidList pointers over
                psDroid->died = FALSE;
				removeDroidBase(psDroid);
//				droidRemove(psDroid, apsDroidLists);
//				droidRelease(psDroid);
//				HEAP_FREE(psDroidHeap, psDroid);
				psDroid = psNext;
			}
            mission.apsDroidLists[Player] = NULL;

			psStruct = apsStructLists[Player];

			while(psStruct != NULL) {
				STRUCTURE *psNext = psStruct->psNext;
				removeStruct(psStruct, TRUE);
				psStruct = psNext;
			}
		}
	}

	gameTime++;	// Wonderfull hack to ensure objects destroyed above get free'ed up by objmemUpdate.

	objmemUpdate();	// Not sure why but we need to call this after freeing up the droids. List house keeping?
}

void processPreviousCampDroids(void)
{
    DROID           *psDroid, *psNext;
	//UDWORD			droidX, droidY;
    //BOOL            bPlaced;

    //see if any are left
    if (mission.apsDroidLists[selectedPlayer])
    {
        for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != NULL;
            psDroid = psNext)
        {
            psNext = psDroid->psNext;
            //We want to kill off all droids now! - AB 27/01/99
		    //KILL OFF TRANSPORTER
    		//if (psDroid->droidType == DROID_TRANSPORTER)
	    	{
		    	if (droidRemove(psDroid, mission.apsDroidLists))
                {
		            addDroid(psDroid, apsDroidLists);
                    vanishDroid(psDroid);
                }
	    	}
            /*else
            {
		        //remove out of stored list and add to current Droid list
		        if (droidRemove(psDroid, mission.apsDroidLists))
                {
		            addDroid(psDroid, apsDroidLists);
		            //set the x/y
		            droidX = getLandingX(psDroid->player) >> TILE_SHIFT;
		            droidY = getLandingY(psDroid->player) >> TILE_SHIFT;
                    bPlaced = pickATileGen(&droidX, &droidY,LOOK_FOR_EMPTY_TILE,normalPAT);
                    if (!bPlaced)
		            {
			            ASSERT((FALSE, "processPreviousCampDroids: Unable to find a free location \
                            cancel to continue"));
                        vanishDroid(psDroid);
		            }
                    else
                    {
    		            psDroid->x = (UWORD)(droidX << TILE_SHIFT);
	    	            psDroid->y = (UWORD)(droidY << TILE_SHIFT);
		                psDroid->z = map_Height(psDroid->x, psDroid->y);
		                updateDroidOrientation(psDroid);
		                //psDroid->lastTile = mapTile(psDroid->x >> TILE_SHIFT, 
			            //    psDroid->y >> TILE_SHIFT);

					    psDroid->selected = FALSE;
		                psDroid->cluster = 0;
		                gridAddObject((BASE_OBJECT *)psDroid);
		                //initialise the movement data
		                initDroidMovement(psDroid);
                    }
                }
            }*/
        }
    }
}

//access functions for droidsToSafety flag - so we don't have to end the mission when a Transporter fly's off world
void setDroidsToSafetyFlag(BOOL set)
{
    bDroidsToSafety = set;
}
BOOL getDroidsToSafetyFlag(void)
{
    return bDroidsToSafety;
}

#ifdef WIN32
//access functions for bPlayCountDown flag - TRUE = play coded mission count down
void setPlayCountDown(UBYTE set)
{
    bPlayCountDown = set;
}
BOOL getPlayCountDown(void)
{
    return bPlayCountDown;
}
#endif

//checks to see if the player has any droids (except Transporters left)
BOOL missionDroidsRemaining(UDWORD player)
{
    DROID   *psDroid;
    BOOL    bDroidsRemaining = FALSE;
    for (psDroid = apsDroidLists[player]; psDroid != NULL; psDroid = psDroid->psNext)
    {
        if (psDroid->droidType != DROID_TRANSPORTER)
        {
            bDroidsRemaining = TRUE;
            //don't bother looking for more
            break;
        }
    }

    return bDroidsRemaining;
}

/*called when a Transporter gets to the edge of the world and the droids are 
being flown to safety. The droids inside the Transporter are placed into the 
mission list for later use*/
void moveDroidsToSafety(DROID *psTransporter)
{
    DROID       *psDroid, *psNext;

    ASSERT((psTransporter->droidType == DROID_TRANSPORTER, 
        "moveUnitsToSafety: unit not a Transporter"));

    //move droids out of Transporter into mission list
	for (psDroid = psTransporter->psGroup->psList; psDroid != NULL 
			AND psDroid != psTransporter; psDroid = psNext)
	{
		psNext = psDroid->psGrpNext;
		grpLeave(psTransporter->psGroup, psDroid);
		//cam change add droid
		psDroid->x = INVALID_XY;
		psDroid->y = INVALID_XY;
		addDroid(psDroid, mission.apsDroidLists);
    }
    //move the transporter into the mission list also
    if (droidRemove(psTransporter, apsDroidLists))
    {
		//cam change add droid - done in missionDroidUpdate()
		//psDroid->x = INVALID_XY;
		//psDroid->y = INVALID_XY;
        addDroid(psTransporter, mission.apsDroidLists);
    }
}

void clearMissionWidgets(void)
{
    //remove any widgets that are up due to the missions
    if (mission.time > 0)
    {
        intRemoveMissionTimer();
    }
#ifdef WIN32
    if (missionCanReEnforce())
    {
        intRemoveTransporterTimer();
    }
#endif
    intRemoveTransporterLaunch();
}

void resetMissionWidgets(void)
{
    DROID       *psDroid;

    //add back any widgets that should be up due to the missions
    if (mission.time > 0)
    {
        intAddMissionTimer();
        //make sure its not flashing when added
        stopMissionButtonFlash(IDTIMER_FORM);
    }
#ifdef WIN32
    if (missionCanReEnforce())
    {
        addTransporterTimerInterface();
    }
    //if not a reinforceable mission and a transporter exists, then add the launch button
    //if (!missionCanReEnforce())
    //check not a typical reinforcement mission
    else if (!missionForReInforcements())
    {
        for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = 
            psDroid->psNext)
        {
            if (psDroid->droidType == DROID_TRANSPORTER)
            {
                intAddTransporterLaunch(psDroid);
                break;
            }
        }
        /*if we got to the end without adding a transporter - there might be 
        one sitting in the mission list which is waiting to come back in*/
        if (!psDroid)
        {
            for (psDroid = mission.apsDroidLists[selectedPlayer]; psDroid != NULL; 
                psDroid = psDroid->psNext)
            {
                if (psDroid->droidType == DROID_TRANSPORTER AND 
                    psDroid->action == DACTION_TRANSPORTWAITTOFLYIN)
                {
                    intAddTransporterLaunch(psDroid);
                    break;
                }
            }
        }
    }
#endif
}

void	setCampaignNumber( UDWORD number )
{
	ASSERT((number<4,"Campaign Number too high!"));
	camNumber = number;
}

UDWORD	getCampaignNumber( void )
{
	return(camNumber);
}

/*deals with any selectedPlayer's transporters that are flying in when the 
mission ends. bOffWorld is TRUE if the Mission is currenly offWorld*/
void emptyTransporters(BOOL bOffWorld)
{
    DROID       *psTransporter, *psDroid, *psNext, *psNextTrans;

    //see if there are any Transporters in the world
    for (psTransporter = apsDroidLists[selectedPlayer]; psTransporter != NULL; 
        psTransporter = psNextTrans)
    {
        psNextTrans = psTransporter->psNext;
        if (psTransporter->droidType == DROID_TRANSPORTER)
        {
            //if flying in, empty the contents
            if (orderState(psTransporter, DORDER_TRANSPORTIN))
            {
                /* if we're offWorld, all we need to do is put the droids into the apsDroidList
                and processMission() will assign them a location etc */
                if (bOffWorld)
                {
 		            for (psDroid = psTransporter->psGroup->psList; psDroid != NULL 
				            AND psDroid != psTransporter; psDroid = psNext)
		            {
			            psNext = psDroid->psGrpNext;
                        //take it out of the Transporter group
                        grpLeave(psTransporter->psGroup, psDroid);
			            //add it back into current droid lists
			            addDroid(psDroid, apsDroidLists);
                    }
                }
                /* we're not offWorld so add to mission.apsDroidList to be 
                processed by the endMission function */
                else
                {
 		            for (psDroid = psTransporter->psGroup->psList; psDroid != NULL 
				            AND psDroid != psTransporter; psDroid = psNext)
		            {
			            psNext = psDroid->psGrpNext;
                        //take it out of the Transporter group
                        grpLeave(psTransporter->psGroup, psDroid);
			            //add it back into current droid lists
			            addDroid(psDroid, mission.apsDroidLists);
                    }
                }
                //now kill off the Transporter
				vanishDroid(psDroid);
           }
        }
    }
    //deal with any transporters that are waiting to come over
    for (psTransporter = mission.apsDroidLists[selectedPlayer]; psTransporter != 
        NULL; psTransporter = psTransporter->psNext)
    {
        if (psTransporter->droidType == DROID_TRANSPORTER)
        {
            //for each droid within the transporter...
 		    for (psDroid = psTransporter->psGroup->psList; psDroid != NULL 
				    AND psDroid != psTransporter; psDroid = psNext)
		    {
			    psNext = psDroid->psGrpNext;
                //take it out of the Transporter group
                grpLeave(psTransporter->psGroup, psDroid);
			    //add it back into mission droid lists
			    addDroid(psDroid, mission.apsDroidLists);
            }
        }
        //don't need to destory the transporter here - it is dealt with by the endMission process
    }
}

/*bCheating = TRUE == start of cheat 
bCheating = FALSE == end of cheat */
void setMissionCheatTime(BOOL bCheating)
{
    if (bCheating)
    {
        mission.cheatTime = gameTime;
    }
    else
    {
        //adjust the mission start time for the duration of the cheat!
        mission.startTime += gameTime - mission.cheatTime;
        mission.cheatTime = 0;
    }
}
