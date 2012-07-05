/*
 * Loop.c
 *
 * The main game loop
 *
 */

#include <stdio.h>

/* loop position printf's */
//#define DEBUG_GROUP1
#include "Frame.h"
#include "Loop.h"
#include "rendmode.h"
#include "pieState.h" //ivis render code
#include "pieMode.h"
#include "vid.h" //ivis render code
#include "Objects.h"
#include "Display.h"
#include "Map.h"
#include "Disp2D.h"
#include "HCI.h"
#include "audio.h"
#include "ingameop.h"
#include "Player.h"
#include "GTime.h"
#include "MiscImd.h"
#include "Effects.h"
#include "Radar.h"
#include "projectile.h"
#include "Console.h"
#include "Power.h"
#include "animobj.h"
#include "Message.h"
#include "bucket3d.h"
#include "Display3d.h"
#include "3dfxFunc.h"
#include "MultiPlay.h" //ajl
#include "Script.h"
#include "ScriptTabs.h"
#include "Levels.h"
#include "Visibility.h"
#include "multimenu.h"
#include "IntelMap.h"
#include "loadsave.h"
#include "game.h"
#include "text.h"

#include "IntImage.h"
#include "Resource.h"
#include "SeqDisp.h"
#include "cdAudio.h"
#include "Mission.h"
#include "WarCam.h"
#include "Lighting.h"
#include "MapGrid.h"
#include "Edit3D.h"
#include "drive.h"
#include "target.h"
#include "csnap.h"
#include "fpath.h"
#include "ScriptExtern.h"
#include "Cluster.h"
#include "mixer.h"
#include "cmdDroid.h"
#include "keyBind.h"
#include "wrappers.h"
#include "PowerCrypt.h"

#ifdef DEBUG
#include "objMem.h"
#endif

#define MISSION_COMPLETE_DELAY	4000

/*
 * Global variables
 */
SDWORD loopPieCount;
SDWORD loopTileCount;
SDWORD loopPolyCount;
SDWORD loopStateChanges;
/*
 * local variables
 */

static BOOL paused=FALSE;
static BOOL video=FALSE;
static BOOL bQuitVideo=FALSE;
static	SDWORD clearCount = 0;
static BOOL bSoftVideoPalette = FALSE;

//holds which pause is valid at any one time
typedef struct _pause_state
{
	unsigned gameUpdatePause	: 1;
	unsigned audioPause			: 1;
	unsigned scriptPause		: 1;
	unsigned scrollPause		: 1;
	unsigned consolePause		: 1;
} PAUSE_STATE;

static PAUSE_STATE	pauseState;
static	UDWORD	numDroids[MAX_PLAYERS];
static	UDWORD	numMissionDroids[MAX_PLAYERS];
static	UDWORD	numTransporterDroids[MAX_PLAYERS];
static	UDWORD	numCommandDroids[MAX_PLAYERS];
static	UDWORD	numConstructorDroids[MAX_PLAYERS];
// flag to signal a quick exit from the game
static BOOL fastExit;
static SDWORD videoMode;

static SDWORD	g_iGlobalVol;

LOOP_MISSION_STATE		loopMissionState = LMS_NORMAL;

// this is set by scrStartMission to say what type of new level is to be started
SDWORD	nextMissionType = LDS_NONE;//MISSION_NONE;

 /* Force 3D display */
UDWORD	mcTime;
BOOL	display3D = TRUE;
extern BOOL		godMode;

BOOL	gamePaused( void );
void	setGamePauseStatus( BOOL val );
void	setGameUpdatePause(BOOL state);
void	setAudioPause(BOOL state);
void	setScriptPause(BOOL state);
void	setScrollPause(BOOL state);


// signal a fast exit from the game
void loopFastExit(void)
{
	fastExit = TRUE;
}


/* The main game loop */
GAMECODE gameLoop(void)
{
	DROID		*psCurr, *psNext;
	STRUCTURE	*psCBuilding, *psNBuilding;
//	BOOL		bPlayerHasHQ = FALSE;
	FEATURE		*psCFeat, *psNFeat;
	UDWORD		i,widgval;
	BOOL		quitting=FALSE;
	INT_RETVAL	intRetVal;
	CLEAR_MODE	clearMode;
//	DumpVRAM();	// use mouse to scroll through vram

//	dumpimdpoints();

//	heapIntegrityCheck(psDroidHeap);

//JPS 24 feb???
	if (fogStatus & FOG_BACKGROUND)
	{
		clearMode = CLEAR_FOG;//screen clear to fog colour D3D
		if (loopMissionState == LMS_SAVECONTINUE)
		{
			pie_SetFogStatus(FALSE);
			clearMode = CLEAR_BLACK;
		}
	}
	else
	{
		clearMode = CLEAR_BLACK;//force to black 3DFX
	}
	pie_ScreenFlip(clearMode);//gameloopflip
//JPS 24 feb???

	
	fastExit = FALSE;

	pie_GlobalRenderBegin();

	HandleClosingWindows();	// Needs to be done outside the pause case.

	audio_Update();

	if (!paused)
	{
		if (!scriptPaused())
		{
#ifdef SCRIPTS
			/* Update the event system */
			if (!bInTutorial)
			{
				eventProcessTriggers(gameTime/SCR_TICKRATE);
			}
			else
			{
				eventProcessTriggers(gameTime2/SCR_TICKRATE);
			}
#endif
		}

		/* Run the in game interface and see if it grabbed any mouse clicks */
		DBP1(("loop: Run Widgets Update\n"));
	  	if( (!rotActive) && getWidgetsStatus() &&
			(dragBox3D.status != DRAG_DRAGGING) &&
			(wallDrag.status != DRAG_DRAGGING) ) 
		{
			intRetVal = intRunWidgets();
		}
		else
		{
			intRetVal = INT_NONE;
		}

        //don't process the object lists if paused or about to quit to the front end
		if (!(gameUpdatePaused() OR intRetVal == INT_QUIT))
		{
			if( (dragBox3D.status != DRAG_DRAGGING) && (wallDrag.status 
				!= DRAG_DRAGGING)) 
			{
				if( (intRetVal == INT_INTERCEPT) 
					|| (radarOnScreen && CoordInRadar(mouseX(),mouseY()) AND 
                    getHQExists(selectedPlayer)) )
				{
					pie_SetMouse(IntImages,IMAGE_CURSOR_DEFAULT);
					frameSetCursorFromRes(IDC_DEFAULT);
					//if( (intRetVal != INT_FULLSCREENPAUSE) && (
					//	intRetVal != INT_INTELPAUSE) ) 
					//{
						intRetVal = INT_INTERCEPT;
					//}
				}
			}
		//}

//		intRetVal = INT_NONE;


		// we need two versions of the loop conditions, since PSX doesn't have
		// multiplayer stuff.

		// Don't update the game world if the design screen is up and single player game
		//if (((intRetVal != INT_FULLSCREENPAUSE) || bMultiPlayer) AND ((intRetVal != 
		//	INT_INTELPAUSE) || bMultiPlayer))

#ifdef DEBUG
			// check all flag positions for duplicate delivery points
			checkFactoryFlags();
#endif
			//handles callbacks for positioning of DP's - like PSX
			process3DBuilding();

			// Update the base movement stuff
			moveUpdateBaseSpeed();

			// Update the visibility change stuff
			visUpdateLevel();

			// do the grid garbage collection
			gridGarbageCollect();

			//update the findpath system
			fpathUpdate();

			// update the cluster system
			clusterUpdate();

			// update the command droids
			cmdDroidUpdate();

			DBP1(("loop: Object Update\n"));

			/* Update the AI for a player */
			for(i=0; i<MAX_PLAYERS; i++)
			{
				playerUpdate(i);
			}

			if(getDrivingStatus())
			{
				driveUpdate();
			}

			//ajl. get the incoming netgame messages and process them.
			if(bMultiPlayer)
			{
				multiPlayerLoop();
//		RecvMessage();
			}
			for(i=0; i<MAX_PLAYERS; i++)
			{
				//update the current power available for a player
				updatePlayerPower(i);

                //this is a check cos there is a problem with the power but not sure where!!
                powerCheck(TRUE, (UBYTE)i);

				//spread the power out...done in aiUpdateStructure now
				//spreadPower((UBYTE)i);

				//set the flag for each player
				//setPowerGenExists(FALSE, i);
				setHQExists(FALSE, i);
                setSatUplinkExists(FALSE, i);

				numCommandDroids[i] = 0;
				numConstructorDroids[i] = 0;
				numDroids[i]=0;
				numTransporterDroids[i]=0;

				for(psCurr = apsDroidLists[i]; psCurr; psCurr = psNext)
				{
					/* Copy the next pointer - not 100% sure if the droid could get destroyed
					   but this covers us anyway */
					psNext = psCurr->psNext;
					droidUpdate(psCurr);

					// update the droid counts
					numDroids[i]++;
					switch (psCurr->droidType)
					{
					case DROID_COMMAND:
						numCommandDroids[i] += 1;
						break;
					case DROID_CONSTRUCT:
                    case DROID_CYBORG_CONSTRUCT:
						numConstructorDroids[i] += 1;
						break;
					case DROID_TRANSPORTER:
						if( (psCurr->psGroup != NULL) )
						{
							numTransporterDroids[i] += psCurr->psGroup->refCount-1;
						}
						break;
					default:
						break;
					}

				}
				
				numMissionDroids[i]=0;
				for(psCurr = mission.apsDroidLists[i]; psCurr; psCurr = psNext)
				{
					/* Copy the next pointer - not 100% sure if the droid could
					get destroyed but this covers us anyway */
					psNext = psCurr->psNext;
					missionDroidUpdate(psCurr);
					numMissionDroids[i]++;
					switch (psCurr->droidType)
					{
					case DROID_COMMAND:
						numCommandDroids[i] += 1;
						break;
					case DROID_CONSTRUCT:
                    case DROID_CYBORG_CONSTRUCT:
						numConstructorDroids[i] += 1;
						break;
					case DROID_TRANSPORTER:
						if( (psCurr->psGroup != NULL) )
						{
							numTransporterDroids[i] += psCurr->psGroup->refCount-1;
						}
						break;
					default:
						break;
					}
				}
				for(psCurr = apsLimboDroids[i]; psCurr; psCurr = psNext)
				{
					/* Copy the next pointer - not 100% sure if the droid could
					get destroyed but this covers us anyway */
					psNext = psCurr->psNext;

					// count the type of units
					switch (psCurr->droidType)
					{
					case DROID_COMMAND:
						numCommandDroids[i] += 1;
						break;
					case DROID_CONSTRUCT:
                    case DROID_CYBORG_CONSTRUCT:
						numConstructorDroids[i] += 1;
						break;
					default:
						break;
					}
				}

                /*set this up AFTER droidUpdate so that if trying to building a 
                new one, we know whether one exists already*/
                setLasSatExists(FALSE, i);
				for (psCBuilding = apsStructLists[i]; psCBuilding; psCBuilding = psNBuilding)
				{
					/* Copy the next pointer - not 100% sure if the structure could get destroyed
					   but this covers us anyway */
					psNBuilding = psCBuilding->psNext;
					structureUpdate(psCBuilding);
					//set animation flag
					/*if (psCBuilding->pStructureType->type == REF_POWER_GEN AND
						psCBuilding->status == SS_BUILT)
					{
						setPowerGenExists(TRUE, i);
					}*/
					if (psCBuilding->pStructureType->type == REF_HQ AND
						psCBuilding->status == SS_BUILT)
					{
						setHQExists(TRUE, i);
					}
					if (psCBuilding->pStructureType->type == REF_SAT_UPLINK AND
						psCBuilding->status == SS_BUILT)
					{
						setSatUplinkExists(TRUE, i);
					}
                    //don't wait for the Las Sat to be built - can't build another if one is partially built
                    if (asWeaponStats[psCBuilding->asWeaps[0].nStat].
                        weaponSubClass == WSC_LAS_SAT)
                    {
                        setLasSatExists(TRUE, i);
                    }
				}
				for (psCBuilding = mission.apsStructLists[i]; psCBuilding; 
						psCBuilding = psNBuilding)
				{
					/* Copy the next pointer - not 100% sure if the structure could get destroyed
					   but this covers us anyway It shouldn't do since its not even on the map!*/
					psNBuilding = psCBuilding->psNext;
					missionStructureUpdate(psCBuilding);
					if (psCBuilding->pStructureType->type == REF_HQ AND
						psCBuilding->status == SS_BUILT)
					{
						setHQExists(TRUE, i);
					}
					if (psCBuilding->pStructureType->type == REF_SAT_UPLINK AND
						psCBuilding->status == SS_BUILT)
					{
						setSatUplinkExists(TRUE, i);
					}
                    //don't wait for the Las Sat to be built - can't build another if one is partially built
                    if (asWeaponStats[psCBuilding->asWeaps[0].nStat].
                        weaponSubClass == WSC_LAS_SAT)
                    {
                        setLasSatExists(TRUE, i);
                    }
				}
                //this is a check cos there is a problem with the power but not sure where!!
                powerCheck(FALSE, (UBYTE)i);
			}

			pwrcUpdate();

			missionTimerUpdate();

			proj_UpdateAll();

			for(psCFeat = apsFeatureLists[0]; psCFeat; psCFeat = psNFeat)
			{
				psNFeat = psCFeat->psNext;
				featureUpdate(psCFeat);
			}

			
			DBP1(("loop: Smoke/Explosion Update\n"));

			/* Ensure smoke drifts up! */
//			raiseSmoke();
//			updateGravitons();

			/* update animations */
			animObj_Update();

			/* Raise and increase frames of explosions */
//			updateExplosions();
			/* Update all the temporary world effects */
//			processEffects();

//			flushConsoleMessages();
//			clustDisplay();

		//}
		// Don't update the game world if the design screen is up and single player game
		//if (((intRetVal != INT_FULLSCREENPAUSE ) || bMultiPlayer) AND ((intRetVal != 
		//	INT_INTELNOSCROLL) || bMultiPlayer))
		//{
			//not any more!
			//need to be able to scroll and have radar still in Intelligence Screen - 
			//but only if 3D View is not up
			/*if(!getWarCamStatus())
			{
				scroll();
			}*/
		//}
		// Don't update the game world if the design screen is up and single player game
		//if ((intRetVal != INT_FULLSCREENPAUSE ) || bMultiPlayer) 
		//{
//			DBP1(("Radar update \n"));
//			/* Make radar line sweep and colour cycle */
//			updateRadar();
		//}

		// Don't update the game world if the design screen is up and single player game
		//if ((intRetVal != INT_FULLSCREENPAUSE AND intRetVal != 
		//	INT_INTELPAUSE) || bMultiPlayer)
		//{
			DBP1(("loop: Objmem Update\n"));

			objmemUpdate();
			
			DBP1(("loop: audio Update\n"));

		}
		if (!consolePaused())
		{
			/* Process all the console messages */
			updateConsoleMessages();
		}
		if (!scrollPaused())
		{
			if(!getWarCamStatus()) //this could set scrollPause?
			{
			 	if(dragBox3D.status != DRAG_DRAGGING 
					&& intMode		!= INT_INGAMEOP )
				{
					scroll();
				}
			}
		}
	}
	else//paused
	{
		intRetVal = INT_NONE;
		if (video)
		{
			bQuitVideo = !seq_UpdateFullScreenVideo(NULL);
		}
		if(dragBox3D.status != DRAG_DRAGGING)
		{
			scroll();
		}
#ifdef WIN32
		if(InGameOpUp)		// ingame options menu up, run it!
		{
			intRunInGameOptions();
//			processFrontendSnap(FALSE);
			widgval = widgRunScreen(psWScreen);
			intProcessInGameOptions(widgval);
			if(widgval == INTINGAMEOP_QUIT_CONFIRM)
			{
				if(gamePaused())
				{
					kf_TogglePauseMode();
				}
				intRetVal = INT_QUIT;
			}
		}

		if(bLoadSaveUp)
		{
			if(runLoadSave(TRUE))// check for file name.
			{
				if(strlen(sRequestResult))
				{
					DBPRINTF(("Returned %s",sRequestResult));
					if(bRequestLoad)
					{
						loopMissionState = LMS_LOADGAME;
						strcpy(saveGameName, sRequestResult);
					}
					else
					{
						if (saveInMissionRes())
						{
							if (saveGame(sRequestResult, GTYPE_SAVE_START))
							{
								addConsoleMessage(strresGetString(psStringRes, STR_GAME_SAVED), LEFT_JUSTIFY);
							}
							else
							{
								ASSERT((FALSE,"Mission Results: saveGame Failed"));
								deleteSaveGame(sRequestResult);
							}
						}
						else if (bMultiPlayer || saveMidMission())
						{
							if (saveGame(sRequestResult, GTYPE_SAVE_MIDMISSION))//mid mission from [esc] menu
							{
								addConsoleMessage(strresGetString(psStringRes, STR_GAME_SAVED), LEFT_JUSTIFY);
							}
							else
							{
								ASSERT((FALSE,"Mid Mission: saveGame Failed"));
								deleteSaveGame(sRequestResult);
							}
      					}
						else
						{
							ASSERT((FALSE, "Attempt to save game with incorrect load/save mode"));
						}
					}
				}
			}
		}
#endif

	}

	/* Check for quit */
//	if (keyPressed(KEY_ESC) || intRetVal == INT_QUIT)
	if ((intRetVal == INT_QUIT) || fastExit)
	{
		if (!video)
		{
			//quitting from the game to the front end
			//so get a new backdrop
			quitting = TRUE;
			if (pie_GetRenderEngine() == ENGINE_GLIDE)
			{
#ifdef COVERMOUNT
				pie_LoadBackDrop(SCREEN_COVERMOUNT,TRUE);
#else
				pie_LoadBackDrop(SCREEN_RANDOMBDROP,TRUE);
#endif
			}
			else
			{
#ifdef COVERMOUNT
				pie_LoadBackDrop(SCREEN_COVERMOUNT,FALSE);
#else
				pie_LoadBackDrop(SCREEN_RANDOMBDROP,FALSE);
#endif
			}
		}
		else //if in video mode esc kill video
		{
			bQuitVideo = TRUE;
		}
	}
	if  (!video)
	{
		//if (!quitting && intRetVal != INT_FULLSCREENPAUSE)
		if (!quitting)
		{
			if (!gameUpdatePaused())
			{
				if (display3D)
				{
					/*bPlayerHasHQ=FALSE;
					for (psStructure = apsStructLists[selectedPlayer]; psStructure AND 
						!bPlayerHasHQ; psStructure = psStructure->psNext)
					{
						if (psStructure->pStructureType->type == REF_HQ)
						{
							bPlayerHasHQ = TRUE;
						}
					}
					*/
				  //	bPlayerHasHQ = radarCheckForHQ(selectedPlayer);

					if( //(intRetVal != INT_INTELPAUSE) &&
						(dragBox3D.status != DRAG_DRAGGING) &&
						(wallDrag.status != DRAG_DRAGGING))
					{
#ifndef NON_INTERACT
						ProcessRadarInput();
#endif
					}
					processInput();

					//no key clicks or in Intelligence Screen
	//				if (intRetVal == INT_INTELPAUSE)
					if (intRetVal == INT_NONE && !InGameOpUp)// OR intRetVal == INT_INTELPAUSE)
					{
						DBP1(("loop: 3D input\n"));
						//quitting = processInput();
						//don't want to handle the mouse input here when in intelligence screen
						//if (intRetVal != INT_INTELPAUSE)
						//{
							processMouseClickInput();
						//}
					}
					DBP1(("loop: display3D\n"));		
					downloadAtStartOfFrame();
					displayWorld();
				}
				else
				{
					//no key clicks or in Intelligence Screen
					if (intRetVal == INT_NONE)// OR intRetVal == INT_INTELPAUSE)
					{
						DBP1(("loop: 2D input\n"));
#ifdef DISP2D
						quitting = process2DInput();
#endif
					}
					DBP1(("loop: display2D\n"));
#ifdef DISP2D
					display2DWorld();
#endif
				}
			}
			/* Display the in game interface */
//			if(widgetsOn OR forceWidgetsOn)
//			{
			pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
			pie_SetFogStatus(FALSE);

#ifdef WIN32
			if(bMultiPlayer)
			{
//				if((game.type == DMATCH) && !MultiMenuUp)
//				{
//					intDisplayMiniMultiMenu();
//				}

				if(bDisplayMultiJoiningStatus)
				{
					intDisplayMultiJoiningStatus(bDisplayMultiJoiningStatus);
					setWidgetsStatus(FALSE);
				}
			}
#endif
			if(getWidgetsStatus())
			{
				intDisplayWidgets();
			}
			pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
			pie_SetFogStatus(TRUE);
//			}

			if(pie_GetRenderEngine() == ENGINE_GLIDE)
			{
				if(!driveModeActive()) {
					pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
					pie_SetFogStatus(FALSE);
					pie_DrawMouse(mouseX(),mouseY());
					pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
					pie_SetFogStatus(TRUE);
				} else {
					// If were in driving mode then put the cursor over the current target.
					BASE_OBJECT *psObj = targetGetCurrent();
					if(psObj != NULL) {
						SetMousePos(0,psObj->sDisplay.screenX,psObj->sDisplay.screenY);
						pie_DrawMouse(psObj->sDisplay.screenX,psObj->sDisplay.screenY);
					}
				}
			}
	//#endif
		}										   
		/*else if (!quitting)
		{
			// Display the in game interface 
	//#ifdef PSX
	//		DrawMousePointer(mouseX(),mouseY());	// add the mouse pointer as a primative
	//#endif

			DBP1(("loop: Display widgets\n"));
			if(widgetsOn)
			{
				pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
				pie_SetFogStatus(FALSE);

				intDisplayWidgets();

				pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
				pie_SetFogStatus(TRUE);
			}

			if(pie_GetRenderEngine() == ENGINE_GLIDE)
			{
				pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
				pie_SetFogStatus(FALSE);
				pie_DrawMouse(mouseX(),mouseY());
				pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
				pie_SetFogStatus(TRUE);
			}
		}*/
	}

	DBP1(("loop: key presses\n"));


	/* Check for toggling video playbackmode */
	if (bQuitVideo)
	{
		if (!video)
		{
		}
		else
		{
			seq_StopFullScreenVideo();
			bQuitVideo = FALSE;
		}
	}

	/* Check for pause */
//	if (!video)
//	{
//		if (keyPressed(KEY_F12) && !paused)
//		{
//			paused = TRUE;
//			gameTimeStop();
	//		addGameMessage("Game Status : PAUSED",1000, TRUE);
//			addConsoleMessage("Game has been paused",DEFAULT_JUSTIFY);
//		}
//		else if (keyPressed(KEY_F12) && paused)
//		{
//			paused = FALSE;
//			gameTimeStart();
//		}
//	}		// ALL THIS GUBBINS DONE IN A PROPER KEYMAPPING NOW (A DEBUG ONE THOUGH!).

	DBP1(("loop: flip\n"));
	
	pie_GetResetCounts(&loopPieCount, &loopTileCount, &loopPolyCount, &loopStateChanges);


	if (fogStatus & FOG_BACKGROUND)
	{
		pie_GlobalRenderEnd(FALSE);//screen clear to fog colour 3DFX
		clearMode = CLEAR_FOG;//screen clear to fog colour D3D
		if (loopMissionState == LMS_SAVECONTINUE)
		{
			pie_SetFogStatus(FALSE);
			clearMode = CLEAR_BLACK;
		}
	}
	else
	{
		pie_GlobalRenderEnd(TRUE);//force to black 3DFX
		clearMode = CLEAR_BLACK;//force to black 3DFX
	}

	if(!quitting && !fastExit)
	{

//JPS 24 feb???		pie_ScreenFlip(clearMode);//gameloopflip
	 //	if(pie_Hardware())
	 //	{
			/* Needs to be handled! */
	 //	}
	//	else
		{
			/* Check for toggling display mode */
			if ((keyDown(KEY_LALT) || keyDown(KEY_RALT)) &&
				keyPressed(KEY_RETURN) AND pie_GetRenderEngine()!=ENGINE_GLIDE)
			{
				screenToggleMode();
		#ifdef DISP2D
				disp2DModeChange();
		#endif
				dispModeChange();
			}
		}
	}

/*	if(missionComplete)
	{
		if (gameTime>(mcTime+MISSION_COMPLETE_DELAY))
		{
			quitting = TRUE;
		}
	}*/

	// deal with the mission state
	switch (loopMissionState)
	{
	case LMS_CLEAROBJECTS:
		missionDestroyObjects();
        setScriptPause(TRUE);
		loopMissionState = LMS_SETUPMISSION;
		break;

	case LMS_NORMAL:
		// default
		break;
	case LMS_SETUPMISSION:
        setScriptPause(FALSE);
		if (!setUpMission(nextMissionType))
		{
			return GAMECODE_QUITGAME;
		}
		break;
	case LMS_SAVECONTINUE:
		// just wait for this to be changed when the new mission starts
		clearMode = CLEAR_BLACK;
		break;
	case LMS_NEWLEVEL:
		//nextMissionType = MISSION_NONE;
		nextMissionType = LDS_NONE;
		return GAMECODE_NEWLEVEL;
		break;
	case LMS_LOADGAME:
		return GAMECODE_LOADGAME;
		break;
	default:
		ASSERT((FALSE, "unknown loopMissionState"));
		break;
	}

	if (fastExit)
	{
		pie_SetFogStatus(FALSE);
		pie_ScreenFlip(CLEAR_BLACK);//gameloopflip
		pie_ScreenFlip(CLEAR_BLACK);//gameloopflip
	 //	if(pie_Hardware())
	 //	{
			/* Needs to be handled! */
	//	}
	//	else
		{
			/* Check for toggling display mode */
			if ((keyDown(KEY_LALT) || keyDown(KEY_RALT)) &&
				keyPressed(KEY_RETURN) AND pie_GetRenderEngine()!=ENGINE_GLIDE)
			{
				screenToggleMode();
		#ifdef DISP2D
				disp2DModeChange();
		#endif
				dispModeChange();
			}
		}
		return GAMECODE_FASTEXIT;
	}
	else if (quitting)
	{
		pie_SetFogStatus(FALSE);
		pie_ScreenFlip(CLEAR_BLACK);//gameloopflip
		pie_ScreenFlip(CLEAR_BLACK);//gameloopflip
	  //	if(pie_Hardware())
	   //	{
			/* Needs to be handled! */
	  //	}
	  //	else
		{
			/* Check for toggling display mode */
			if ((keyDown(KEY_LALT) || keyDown(KEY_RALT)) &&
				keyPressed(KEY_RETURN) AND pie_GetRenderEngine()!=ENGINE_GLIDE)
			{
				screenToggleMode();
		#ifdef DISP2D
				disp2DModeChange();
		#endif
				dispModeChange();
			}
		}
		return GAMECODE_QUITGAME;
	}
	else if (video)
	{
		audio_StopAll();
		return GAMECODE_PLAYVIDEO;
	}
	
	/*
	if( (intMode == INT_NORMAL) AND (forceWidgetsOn == TRUE) )
	{
		forceWidgetsOn = FALSE;
	}
	*/
	return GAMECODE_CONTINUE;
}

/* The video playback loop */
GAMECODE videoLoop(void)
{
	BOOL	bVolKilled = FALSE;

	CLEAR_MODE bClear;
static BOOL bActiveBackDrop = FALSE;

#ifdef DEBUG
	screen_GetBackDrop();//test only remove JPS feb26
#endif

	pie_GlobalRenderBegin();

	bClear = CLEAR_OFF;

	if (video)
	{
		bQuitVideo = !seq_UpdateFullScreenVideo(&bClear);
	}

	if ( (keyPressed(KEY_ESC) || bQuitVideo) && !seq_AnySeqLeft() )
	{
		/* zero volume before video quit - restore later */
		g_iGlobalVol = sound_GetGlobalVolume();
		mixer_SetWavVolume( 0 );
		bVolKilled = TRUE;
	}

	//toggling display mode disabled in video mode
	// Check for quit
	if (keyPressed(KEY_ESC))// || bQuitVideo)
	{
		seq_StopFullScreenVideo();
		bQuitVideo = FALSE;

		// remove the intelligence screen if necessary
		if (messageIsImmediate())
		{
			intResetScreen(TRUE);
			setMessageImmediate(FALSE);
		}
		//Script callback for video over
#ifdef SCRIPTS
        //don't do the callback if we're playing the win/lose video
        if (!getScriptWinLoseVideo())
        {
		    eventFireCallbackTrigger(CALL_VIDEO_QUIT);
        }
        else
        {
            displayGameOver(getScriptWinLoseVideo() == PLAY_WIN);
        }
#endif
//		clearCount = 0;
		pie_ScreenFlip(CLEAR_BLACK);// videoloopflip extra mar10
		if(pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_ScreenFlip(CLEAR_BLACK);// videoloopflip extra mar10
		}
		if (bActiveBackDrop)
		{
 			screen_RestartBackDrop();
		}
		// remove the intelligence screen if necessary
		/*if (messageIsImmediate())
		{
			intResetScreen(TRUE);
		}*/
	}

	//if the video finished...
	if (bQuitVideo)
	{
		seq_StopFullScreenVideo();
		bQuitVideo = FALSE;

		//set the next video off - if any
		if (seq_AnySeqLeft())
		{
			pie_ScreenFlip(CLEAR_BLACK);// videoloopflip extra mar10
			if(pie_GetRenderEngine() == ENGINE_GLIDE)
			{
				pie_ScreenFlip(CLEAR_BLACK);// videoloopflip extra mar10
			}
			if (bActiveBackDrop)
			{
				screen_RestartBackDrop();
			}
		 	//bClear = CLEAR_BLACK;
			seq_StartNextFullScreenVideo();
		}
		else
		{
			// remove the intelligence screen if necessary
			if (messageIsImmediate())
			{
				intResetScreen(TRUE);
				setMessageImmediate(FALSE);
			}
#ifdef SCRIPTS
            //don't do the callback if we're playing the win/lose video
            if (!getScriptWinLoseVideo())
            {
			    eventFireCallbackTrigger(CALL_VIDEO_QUIT);
            }
            else
            {
                displayGameOver(getScriptWinLoseVideo() == PLAY_WIN);
            }
#endif
//		    clearCount = 0;
			pie_ScreenFlip(CLEAR_BLACK);// videoloopflip extra mar10
			if(pie_GetRenderEngine() == ENGINE_GLIDE)
			{
				pie_ScreenFlip(CLEAR_BLACK);// videoloopflip extra mar10
			}
    		if (bActiveBackDrop)
	    	{
		    	screen_RestartBackDrop();
		    }
			// remove the intelligence screen if necessary
			/*if (messageIsImmediate())
			{
				intResetScreen(TRUE);
			}*/
		}
	}

	pie_GlobalRenderEnd(TRUE);//force to black

	if( clearCount < 1)
	{
		bClear = CLEAR_BLACK;
		if (screen_GetBackDrop() != NULL)
		{
			bActiveBackDrop = TRUE;
			screen_StopBackDrop();
		}
		else
		{
			bActiveBackDrop = FALSE;
			screen_StopBackDrop();
		}
	}
	else if( clearCount < 2)
	{
		bClear = CLEAR_BLACK;
	}

	clearCount++;

	pie_ScreenFlip(bClear);// videoloopflip

	/* restore volume after video quit */
	if ( bVolKilled == TRUE )
	{
#ifdef WIN32
		mixer_SetWavVolume( g_iGlobalVol );
#else
		sound_SetGlobalVolume( g_iGlobalVol );
#endif
	}
	
	return GAMECODE_CONTINUE;
}

void loop_SetVideoPlaybackMode(void)
{
#ifdef DEBUG
	screen_GetBackDrop();//test only remove JPS feb26
#endif
	videoMode += 1;
	paused = TRUE;
	video = TRUE;
	clearCount = 0;
	gameTimeStop();
	pie_SetFogStatus(FALSE);
	audio_StopAll();
	if(!pie_Hardware())
	{
		screenToggleVideoPlaybackMode();
		if (!bSoftVideoPalette)
		{
			pal_Make16BitPalette();//now we are in 16bit mode
			bSoftVideoPalette = TRUE;
		}
	}
}

void loop_ClearVideoPlaybackMode(void)
{
	videoMode -=1;
	paused = FALSE;
	video = FALSE;
	gameTimeStart();
//	pie_SetFogStatus(TRUE);
	if(!pie_Hardware())
	{
		screenToggleVideoPlaybackMode();
	}
	cdAudio_Resume();
	ASSERT((videoMode == 0,"loop_ClearVideoPlaybackMode: out of sync."));
}

SDWORD loop_GetVideoMode(void)
{
	return videoMode;
}

BOOL loop_GetVideoStatus(void)
{
	return video;
}

BOOL	gamePaused( void )
{
	return(paused);
}

void	setGamePauseStatus( BOOL val )
{
	paused = val;
}

BOOL gameUpdatePaused(void)
{
	return pauseState.gameUpdatePause;
}
BOOL audioPaused(void)
{
	return pauseState.audioPause;
}
BOOL scriptPaused(void)
{
	return pauseState.scriptPause;
}
BOOL scrollPaused(void)
{
	return pauseState.scrollPause;
}
BOOL consolePaused(void)
{
	return pauseState.consolePause;
}

void setGameUpdatePause(BOOL state)
{
	pauseState.gameUpdatePause = state;
	if (state)
	{
	   	screen_RestartBackDrop();
	}
	else
	{
		screen_StopBackDrop();
	}
}
void setAudioPause(BOOL state)
{
	pauseState.audioPause = state;
}
void setScriptPause(BOOL state)
{
	pauseState.scriptPause = state;
}
void setScrollPause(BOOL state)
{
	pauseState.scrollPause = state;
}
void setConsolePause(BOOL state)
{
	pauseState.consolePause = state;
}

//set all the pause states to the state value
void setAllPauseStates(BOOL state)
{
	setGameUpdatePause(state);
	setAudioPause(state);
	setScriptPause(state);
	setScrollPause(state);
	setConsolePause(state);
}

UDWORD	getNumDroids(UDWORD player)
{
	return(numDroids[player]);
}

UDWORD	getNumTransporterDroids(UDWORD player)
{
	return(numTransporterDroids[player]);
}

UDWORD	getNumMissionDroids(UDWORD player)
{
	return(numMissionDroids[player]);
}

UDWORD	getNumCommandDroids(UDWORD player)
{
	return numCommandDroids[player];
}

UDWORD	getNumConstructorDroids(UDWORD player)
{
	return numConstructorDroids[player];
}


// increase the droid counts - used by update factory to keep the counts in sync
void incNumDroids(UDWORD player)
{
	numDroids[player] += 1;
}
void incNumCommandDroids(UDWORD player)
{
	numCommandDroids[player] += 1;
}
void incNumConstructorDroids(UDWORD player)
{
	numConstructorDroids[player] += 1;
}

