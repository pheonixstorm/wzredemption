#include "frame.h"
#include "objects.h"
#include "base.h"
#include "map.h"
#include "warcam.h"
#include "warzoneconfig.h"
#include "console.h"
#include "objects.h"
#include "display.h"
#include "mapdisplay.h"
#include "display3d.h"
#include "edit3d.h"
#include "keybind.h"
#include "mechanics.h"
#include "audio.h"
#include "audio_id.h"
#include "lighting.h"
#include "power.h"
#include "hci.h"
#include "oprint.h"
#include "wrappers.h"
#include "ingameop.h"
#include "effects.h"
#include "component.h"
#include "geometry.h"
#include "radar.h"
#include "cheat.h"
#include "netplay.h"
#include "multiplay.h"
#include "multimenu.h"
#include "atmos.h"
#include "dglide.h"
#include "raycast.h"
#include "advvis.h"
#include "game.h"
#include "difficulty.h"

#include "intorder.h"
#include "widget.h"
#include "widgint.h"
#include "bar.h"
#include "form.h"
#include "label.h"
#include "button.h"
#include "order.h"
#include "rendmode.h"
#include "piestate.h"
#include "piematrix.h"

#include "keymap.h"
#include "loop.h"
#include "script.h"
#include "scripttabs.h"
#include "scriptextern.h"
#include "mission.h"
#include "mapgrid.h"
#include "order.h"
#include "drive.h"
#include "text.h"
#include "selection.h"
#include "difficulty.h"

#define	MAP_ZOOM_RATE	(500)


#define PITCH_SCALING	(360*DEG_1)
#define	SECS_PER_PITCH	2
#define MAP_PITCH_RATE	(SPIN_SCALING/SECS_PER_SPIN)

#define MAX_TYPING_LENGTH	80



BOOL		bAllowOtherKeyPresses = TRUE;
extern BOOL	bAllowDebugMode;
STRUCTURE	*psOldRE = NULL;
extern		void shakeStop(void);
STRING	sTextToSend[MAX_CONSOLE_STRING_LENGTH];	

int fogCol = 0;//start in nicks mode

/* Support functions to minimise code size */
void	kfsf_SelectAllSameProp	( PROPULSION_TYPE propType );
void	kfsf_SelectAllSameName	( STRING *droidName );
void	kfsf_SetSelectedDroidsState( SECONDARY_ORDER sec, SECONDARY_STATE State );
/*	
	KeyBind.c
	Holds all the functions that can be mapped to a key.
	All functions at the moment must be 'void func(void)'.
	Alex McLean, Pumpkin Studios, EIDOS Interactive.
*/

// --------------------------------------------------------------------------
void	kf_ToggleMissionTimer( void )
{
	if(mission.cheatTime)
	{
		setMissionCheatTime(FALSE);
	}
	else
	{
		setMissionCheatTime(TRUE);
	}
}
// --------------------------------------------------------------------------
void	kf_ToggleRadarJump( void )
{
	if(getRadarJumpStatus())
	{
		setRadarJump(FALSE);
	}
	else
	{
		setRadarJump(TRUE);
	}
}
// --------------------------------------------------------------------------
void	kf_NoFaults( void )
{
	audio_QueueTrack( ID_SOUND_NOFAULTS );
}
// --------------------------------------------------------------------------

void	kf_ToggleDimension( void )
{
		display3D = FALSE;
		intSetMapPos(player.p.x + ((visibleXTiles/2) << TILE_SHIFT),
					 player.p.z + ((visibleYTiles/2) << TILE_SHIFT));
}

// --------------------------------------------------------------------------

/* Halves all the heights of the map tiles */
void	kf_HalveHeights( void )
{
UDWORD	i,j;
MAPTILE	*psTile;

  		for (i=0; i<mapWidth; i++)
		{
			for (j=0; j<mapHeight; j++)
			{
				psTile = mapTile(i,j);
				psTile->height/=2;;
			}
		}
}

// --------------------------------------------------------------------------
void	kf_FaceNorth(void)
{
	player.r.y = 0;
	if(getWarCamStatus())
	{
		camToggleStatus();
	}
}
// --------------------------------------------------------------------------
void	kf_FaceSouth(void)
{
	player.r.y = DEG(180);
	if(getWarCamStatus())
	{
		camToggleStatus();
	}
}
// --------------------------------------------------------------------------
void	kf_FaceEast(void)
{
	player.r.y = DEG(90);
	if(getWarCamStatus())
	{
		camToggleStatus();
	}
}
// --------------------------------------------------------------------------
void	kf_FaceWest(void)
{
	player.r.y = DEG(270);
	if(getWarCamStatus())
	{
		camToggleStatus();
	}
}
// --------------------------------------------------------------------------

/* Writes out debug info about all the selected droids */
void	kf_DebugDroidInfo( void )
{
DROID	*psDroid;

	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid=psDroid->psNext)
			{
				if (psDroid->selected)
				{
					printDroidInfo(psDroid);
				}
			}
}

// --------------------------------------------------------------------------
//
///* Prints out the date and time of the build of the game */
void	kf_BuildInfo( void )
{
 	CONPRINTF(ConsoleString,(ConsoleString,"Built at %s on %s",__TIME__,__DATE__));
}

// --------------------------------------------------------------------------

/* Toggles whether the windows surface gets updated */
void	kf_UpdateWindow( void )
{
    updateVideoCard = !updateVideoCard;
    addConsoleMessage("Windows surface update toggled",DEFAULT_JUSTIFY);
}

// --------------------------------------------------------------------------
void	kf_ToggleConsoleDrop( void )
{
	if(!bInTutorial)
	{
		toggleConsoleDrop();
	}
}
// --------------------------------------------------------------------------
void	kf_SetKillerLevel( void )  // FIXME This needs to be changed so tough and killer difficulty can be used later.
{
	if(!bMultiPlayer || (NetPlay.bComms == 0))
	{
		setDifficultyLevel(DL_KILLER);
		addConsoleMessage("Hard as nails!!!",LEFT_JUSTIFY);
	}
}
// --------------------------------------------------------------------------
void	kf_SetEasyLevel( void )
{
	if(!bMultiPlayer|| (NetPlay.bComms == 0))
	{
		setDifficultyLevel(DL_EASY);
		addConsoleMessage("Takings thing easy!",LEFT_JUSTIFY);
	}
}

// --------------------------------------------------------------------------
void	kf_UpThePower( void )
{
	if(!bMultiPlayer|| (NetPlay.bComms == 0))
	{
		asPower[selectedPlayer]->currentPower+=1000;
		addConsoleMessage("1000 big ones!!!",LEFT_JUSTIFY);
	}
}

// --------------------------------------------------------------------------
void	kf_MaxPower( void )
{
	if(!bMultiPlayer|| (NetPlay.bComms == 0))
	{
		asPower[selectedPlayer]->currentPower=SDWORD_MAX;
		addConsoleMessage("Max Power!!",LEFT_JUSTIFY);
	}
}

// --------------------------------------------------------------------------
void	kf_SetNormalLevel( void )
{
	if(!bMultiPlayer|| (NetPlay.bComms == 0))
	{
		setDifficultyLevel(DL_NORMAL);
		addConsoleMessage("Back to normality!",LEFT_JUSTIFY);
	}
}
// --------------------------------------------------------------------------
void	kf_SetHardLevel( void )
{
	if(!bMultiPlayer|| (NetPlay.bComms == 0))
	{
		setDifficultyLevel(DL_HARD);
		addConsoleMessage("Getting tricky!",LEFT_JUSTIFY);
	}
}
// --------------------------------------------------------------------------
void	kf_SetToughUnitsLevel( void )
{
	if(!bMultiPlayer|| (NetPlay.bComms == 0))
	{
		setDifficultyLevel(DL_TOUGH);
		addConsoleMessage("Twice as nice!",LEFT_JUSTIFY);
	}
}
// --------------------------------------------------------------------------

/* Writes out the frame rate */
void	kf_FrameRate( void )
{
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		if(weHave3DNow())
		{
		CONPRINTF(ConsoleString,(ConsoleString,"GLIDE (With AMD 3DNow!) fps - %d; PIEs - %d; polys - %d; Terr. polys - %d; States %d",
				frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount,loopStateChanges));
		}
		else
		{
 	 	 	CONPRINTF(ConsoleString,(ConsoleString,"GLIDE fps - %d; PIEs - %d; polys - %d; Terr. polys - %d; States %d",
				frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount,loopStateChanges));
		}
//		ASSERT((war_GetFog(),"Fog is Disabled"));
//		ASSERT((war_GetTranslucent(),"Transparency is Disabled"));
//		ASSERT((war_GetAdditive(),"Additive Transpaency is Disabled"));
		DBPRINTF(("GLIDE fps - %d; PIEs - %d; polys - %d; Terr. polys - %d; States %d;",
			frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount,loopStateChanges));
	}
	else if (pie_GetRenderEngine() == ENGINE_D3D)
	{
		if(weHave3DNow())
		{
			CONPRINTF(ConsoleString,(ConsoleString,"DIRECT3D (With AMD 3DNow!) fps %d; PIEs %d; polys %d; Terr. polys %d; States %d",
				frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount,loopStateChanges));
		}
		else
		{
 	 	   	CONPRINTF(ConsoleString,(ConsoleString,"DIRECT3D fps %d; PIEs %d; polys %d; Terr. polys %d; States %d",
				frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount,loopStateChanges));
		}

	}
	else
	{	
		if(weHave3DNow())
		{
			CONPRINTF(ConsoleString,(ConsoleString,"SOFTWARE (With AMD 3DNow!) fps - %d; pie's - %d; polys - %d; Terr. polys - %d;",
				frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount));
		}
		else
		{
			CONPRINTF(ConsoleString,(ConsoleString,"SOFTWARE fps - %d; pie's - %d; polys - %d; Terr. polys - %d;",
				frameGetFrameRate(),loopPieCount,loopPolyCount,loopTileCount));
		}
	}
		if (bMultiPlayer)
		{

			CONPRINTF(ConsoleString,(ConsoleString,
						"NETWORK:  Bytes: s-%d r-%d  Packets: s-%d r-%d",
						NETgetBytesSent(),
						NETgetBytesRecvd(),
						NETgetPacketsSent(),
						NETgetPacketsRecvd() ));
						
		}
		gameStats = !gameStats;

		CONPRINTF(ConsoleString,(ConsoleString,"Built at %s on %s",__TIME__,__DATE__));
//		addConsoleMessage("Game statistics display toggled",DEFAULT_JUSTIFY);
}

// --------------------------------------------------------------------------

// display the total number of objects in the world
void kf_ShowNumObjects( void )
{
	SDWORD		i, droids, structures, features;
	DROID		*psDroid;
	STRUCTURE	*psStruct;
	FEATURE		*psFeat;

	droids = 0;
	structures = 0;
	features = 0;
	for (i=0; i<MAX_PLAYERS; i++)
	{
		for(psDroid = apsDroidLists[i]; psDroid; psDroid = psDroid->psNext)
		{
			droids += 1;
		}

		for(psStruct = apsStructLists[i]; psStruct; psStruct = psStruct->psNext)
		{
			structures += 1;
		}
	}

	for(psFeat=apsFeatureLists[0]; psFeat; psFeat = psFeat->psNext)
	{
		features += 1;
	}

	CONPRINTF(ConsoleString,(ConsoleString, "Num Droids: %d  Num Structures: %d  Num Features: %d",
				droids, structures, features));
}

// --------------------------------------------------------------------------

/* Toggles radar on off */
void	kf_ToggleRadar( void )
{
  		radarOnScreen = !radarOnScreen;
//		addConsoleMessage("Radar display toggled",DEFAULT_JUSTIFY);
}

// --------------------------------------------------------------------------

/* Toggles the outline around the map tiles */
void	kf_ToggleOutline( void )
{
		if(terrainOutline)
		{
			terrainOutline = FALSE;
		}
		else
		{
			terrainOutline = TRUE;
		}
//		addConsoleMessage("Tile outline display toggled",DEFAULT_JUSTIFY);
}

// --------------------------------------------------------------------------

/* Toggles infinite power on/off */
void	kf_TogglePower( void )
{
#ifndef DEBUG
if(bMultiPlayer)
{
	return;
}
#endif
		powerCalculated = !powerCalculated;
		if (powerCalculated)
		{
			addConsoleMessage("Infinite power disabled",DEFAULT_JUSTIFY);
			powerCalc(TRUE);
		}
		else
		{
			addConsoleMessage("Infinite power enabled",DEFAULT_JUSTIFY);
		}
}

// --------------------------------------------------------------------------

/* Recalculates the lighting values for a tile */
void	kf_RecalcLighting( void )
{
		//initLighting();
        initLighting(0, 0, mapWidth, mapHeight);
		addConsoleMessage("Lighting values for all tiles recalculated",DEFAULT_JUSTIFY);
}

// --------------------------------------------------------------------------

/* Raises the 3dfx gamma value */
void	kf_RaiseGamma( void )
{
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		if(gamma<(float)5.0)
		{
			gamma = gamma+(float)0.1;
			pie_SetGammaValue(gamma);
			addConsoleMessage("Gamma correction altered",DEFAULT_JUSTIFY);
		}
		else
		{
			gamma = (float)0.2;
		}
	}
}

// --------------------------------------------------------------------------

/* Lowers the threedfx gamma value */
void	kf_LowerGamma( void )
{
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		if(gamma>(float)0.2)
		{
			gamma = gamma-(float)0.1;
			pie_SetGammaValue(gamma);
			addConsoleMessage("Gamma correction lowered",DEFAULT_JUSTIFY);
		}
		else
		{
			addConsoleMessage("Gamma correction at MINIMUM",DEFAULT_JUSTIFY);
		}
	}
}	

// --------------------------------------------------------------------------

/* Sends the 3dfx screen buffer to disk */
void	kf_ScreenDump( void )
{
	if(pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		CONPRINTF(ConsoleString,(ConsoleString,"3dfx 24 bit raw screen dump written to working directory : %s",iV_ScreenDumpToDisk()));
	}
	else
	{
		CONPRINTF(ConsoleString,(ConsoleString,"Screen dump function presently only works on 3dfx based cards."));
	}
}

// --------------------------------------------------------------------------

/* Make all functions available */
void	kf_AllAvailable( void )
{

#ifndef DEBUG
if(bMultiPlayer && (NetPlay.bComms != 0) )
{
	return;
}
#endif


//		addConsoleMessage("All items made available",DEFAULT_JUSTIFY);
		makeAllAvailable();

}

// --------------------------------------------------------------------------

/* Flips the cut of a tile */
void	kf_TriFlip( void )
{
iVector	pos;
//MAPTILE	*psTile;
//		psTile = mapTile(mouseTileX,mouseTileY);
//		TOGGLE_TRIFLIP(psTile);
//		addConsoleMessage("Triangle flip status toggled",DEFAULT_JUSTIFY);
		pos.x = mouseTileX*TILE_UNITS + TILE_UNITS/2;
		pos.z = mouseTileY*TILE_UNITS + TILE_UNITS/2;
		pos.y = map_Height(pos.x,pos.x);
		effectGiveAuxVar(50);
		effectGiveAuxVarSec(10000);

		addEffect(&pos,EFFECT_FIRE,FIRE_TYPE_LOCALISED,FALSE,NULL,0); 

}

// --------------------------------------------------------------------------
void	kf_ToggleBackgroundFog( void )
{
	static BOOL bEnabled  = TRUE;//start in nicks mode
	
		if (bEnabled)//true, so go to false
		{
			bEnabled = FALSE;
			fogStatus &= FOG_FLAGS-FOG_BACKGROUND;//clear lowest bit of 3
			if (fogStatus == 0)
			{
				pie_SetFogStatus(FALSE);
				pie_EnableFog(FALSE);
			}
		}
		else
		{
			bEnabled = TRUE;
			if (fogStatus == 0)
			{
				pie_EnableFog(TRUE);
			}
			fogStatus |= FOG_BACKGROUND;//set lowest bit of 3
		}
}

extern void	kf_ToggleDistanceFog( void )
{
	static BOOL bEnabled  = TRUE;//start in nicks mode
	
		if (bEnabled)//true, so go to false
		{
			bEnabled = FALSE;
			fogStatus &= FOG_FLAGS-FOG_DISTANCE;//clear middle bit of 3
			if (fogStatus == 0)
			{
				pie_SetFogStatus(FALSE);
				pie_EnableFog(FALSE);
			}
		}
		else
		{
			bEnabled = TRUE;
			if (fogStatus == 0)
			{
				pie_EnableFog(TRUE);
			}
			fogStatus |= FOG_DISTANCE;//set lowest bit of 3
		}
}

void	kf_ToggleMistFog( void )
{
	static BOOL bEnabled  = TRUE;//start in nicks mode
	
		if (bEnabled)//true, so go to false
		{
			bEnabled = FALSE;
			fogStatus &= FOG_FLAGS-FOG_GROUND;//clear highest bit of 3
			if (fogStatus == 0)
			{
				pie_SetFogStatus(FALSE);
				pie_EnableFog(FALSE);
			}
		}
		else
		{
			bEnabled = TRUE;
			if (fogStatus == 0)
			{
				pie_EnableFog(TRUE);
			}
			fogStatus |= FOG_GROUND;//set highest bit of 3
		}
}

void	kf_ToggleFogColour( void )
{
	fogCol++;
	if (fogCol>4)
		fogCol = 0;
	switch(fogCol)
	{
	case 1:
			pie_SetFogColour(0x00c9920f);//nicks colour Urban 
			break;
	case 2:
			pie_SetFogColour(0x00b6e1ec);//nicks colour Rockies 182,225,236
			  break;
	case 3:
			pie_SetFogColour(0x00101040);//haze
			  break;
	case 4:
			pie_SetFogColour(0x00000000);//black
			  break;
	case 0:
	default:
			pie_SetFogColour(0x00B08f5f);//nicks colour Arizona 
			//pie_SetFogColour(0x0078684f);//(nicks colour + 404040)/2
		break;
	}
}

void	kf_ToggleFog( void )
{
	static BOOL fogEnabled = FALSE;
	
		if (fogEnabled)
		{
			fogEnabled = FALSE;
			pie_SetFogStatus(FALSE);
			pie_EnableFog(fogEnabled);
//			addConsoleMessage("Fog Off",DEFAULT_JUSTIFY);
		}
		else
		{
			fogEnabled = TRUE;
			pie_EnableFog(fogEnabled);
//			addConsoleMessage("Fog On",DEFAULT_JUSTIFY);
		}
}

// --------------------------------------------------------------------------

/* Toggles fog on/off */
void	kf_ToggleWidgets( void )
{
//	 	widgetsOn = !widgetsOn;
	if(getWidgetsStatus())
	{
		setWidgetsStatus(FALSE);
	}
	else
	{
		setWidgetsStatus(TRUE);
	}
//	addConsoleMessage("Widgets display toggled",DEFAULT_JUSTIFY);
}

// --------------------------------------------------------------------------

/* Toggle camera on/off */
void	kf_ToggleCamera( void )
{
		if(getWarCamStatus() == FALSE) {
			shakeStop();	// Ensure screen shake stopped before starting camera mode.
			setDrivingStatus(FALSE);
		}
		camToggleStatus();
}

// --------------------------------------------------------------------------

/* Simulates a close down */
/*
void	kf_SimCloseDown( void )
{
  		bScreenClose = TRUE;
		audio_PlayTrack( ID_SOUND_THX_SHUTDOWN );

		closingTimeStart = gameTime;
//		widgetsOn = FALSE;
		spinScene = TRUE;
		radarOnScreen = FALSE;
		screenCloseState = SC_CLOSING_DOWN;
}
*/
// --------------------------------------------------------------------------

/* Toggles on/off gouraud shading */
void	kf_ToggleGouraud( void )
{
 	gouraudShading = !gouraudShading;
 	addConsoleMessage("Gouraud shading toggled",DEFAULT_JUSTIFY);
	texPage++;
}

// --------------------------------------------------------------------------

/* Raises the tile under the mouse */
void	kf_RaiseTile( void )
{
	raiseTile(mouseTileX,mouseTileY);
}

// --------------------------------------------------------------------------

/* Lowers the tile under the mouse */
void	kf_LowerTile( void )
{
//	lowerTile(mouseTileX,mouseTileY);
	selNextSpecifiedBuilding(REF_FACTORY);
}

// --------------------------------------------------------------------------

/* Quick game exit */
void	kf_SystemClose( void )
{
	if(pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		grSstControl(GR_CONTROL_DEACTIVATE);
	}

//	ExitProcess(4);
	loopFastExit();
}

// --------------------------------------------------------------------------
/* Zooms out from display */
void	kf_ZoomOut( void )
{
FRACT	fraction;
FRACT	zoomInterval;

	fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
	zoomInterval = fraction * MAP_ZOOM_RATE;
	distance += MAKEINT(zoomInterval);
	/* If we're debugging, limit to a bit more */
	

#ifndef JOHN
	{
		if(distance>DISTANCE)
		{
			distance = DISTANCE;
		}
	}
#endif

}

// --------------------------------------------------------------------------
void	kf_RadarZoomIn( void )
{
	if(RadarZoomLevel < MAX_RADARZOOM) 
	{
		RadarZoomLevel++;
	   	SetRadarZoom(RadarZoomLevel);
		audio_PlayTrack( ID_SOUND_BUTTON_CLICK_5 );
   	}
	else	// at maximum already
	{
		audio_PlayTrack( ID_SOUND_BUILD_FAIL );
	}	

}
// --------------------------------------------------------------------------
void	kf_RadarZoomOut( void )
{
	if(RadarZoomLevel)
	{
		RadarZoomLevel--;
	   	SetRadarZoom(RadarZoomLevel);
		audio_PlayTrack( ID_SOUND_BUTTON_CLICK_5 );
	}
	else	// at minimum already
	{
		audio_PlayTrack( ID_SOUND_BUILD_FAIL );
	}
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
/* Zooms in the map */
void	kf_ZoomIn( void )
{
FRACT	fraction;
FRACT	zoomInterval;

	fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
	zoomInterval = fraction * MAP_ZOOM_RATE;
	distance -= MAKEINT(zoomInterval);
  	
	if( distance< MINDISTANCE)
	{
		distance = MINDISTANCE;
	}
	
}

// --------------------------------------------------------------------------
void kf_MaxScrollLimits( void )
{
	scrollMinX = scrollMinY = 0;
	scrollMaxX = mapWidth;
	scrollMaxY = mapHeight;
}


// --------------------------------------------------------------------------
// Shrink the screen down
/*
void	kf_ShrinkScreen( void )
{
	// nearest multiple of 8 plus 1 
	if (xOffset<73)
	{
		xOffset+=8;
  		distance+=170;
		if (yOffset<200)
		{
			yOffset+=8;
		}
	}
}
*/
// --------------------------------------------------------------------------
// Expand the screen
/*
void	kf_ExpandScreen( void )
{
	if(xOffset)
	{
   		if (distance>DISTANCE)
   		{
   			distance-=170;
   		}
   		xOffset-=8;
   		if(yOffset)
   		{
   			yOffset-=8;
   		}
	}
}
*/
// --------------------------------------------------------------------------
/* Spins the world round left */
void	kf_RotateLeft( void )
{
FRACT	fraction;
FRACT	rotAmount;

	fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
	rotAmount = fraction * MAP_SPIN_RATE;
	player.r.y += MAKEINT(rotAmount);
}

// --------------------------------------------------------------------------
/* Spins the world right */
void	kf_RotateRight( void )
{
FRACT	fraction;
FRACT	rotAmount;

	fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
	rotAmount = fraction * MAP_SPIN_RATE;
	player.r.y -= MAKEINT(rotAmount);
	if(player.r.y<0)
	{
		player.r.y+= DEG(360);
	}
}
			
// --------------------------------------------------------------------------
/* Pitches camera back */
void	kf_PitchBack( void )
{
FRACT	fraction;
FRACT	pitchAmount;

//#ifdef ALEXM
//SDWORD	pitch;
//SDWORD	angConcern;
//#endif

	fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
	pitchAmount = fraction * MAP_PITCH_RATE;

//#ifdef ALEXM
//	pitch = getSuggestedPitch();
//	angConcern = DEG(360-pitch);
//
//	if(player.r.x < angConcern) 
//	{
//#endif

	player.r.x+= MAKEINT(pitchAmount);

//#ifdef ALEXM
//	}
//#endif
//#ifdef ALEXM
//	if(getDebugMappingStatus() == FALSE)
//#endif

//	{
 	if(player.r.x>DEG(360+MAX_PLAYER_X_ANGLE))
  	{
   		player.r.x = DEG(360+MAX_PLAYER_X_ANGLE);
   	}
//	}
	setDesiredPitch(player.r.x/DEG_1);
}

// --------------------------------------------------------------------------
/* Pitches camera foward */
void	kf_PitchForward( void )
{
FRACT	fraction;
FRACT	pitchAmount;

	fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
	pitchAmount = fraction * MAP_PITCH_RATE;
	player.r.x-= MAKEINT(pitchAmount);
//#ifdef ALEXM
//	if(getDebugMappingStatus() == FALSE)
//#endif
//	{
	if(player.r.x <DEG(360+MIN_PLAYER_X_ANGLE))
	{
		player.r.x = DEG(360+MIN_PLAYER_X_ANGLE);
	}
//	}
	setDesiredPitch(player.r.x/DEG_1);
}

// --------------------------------------------------------------------------
/* Resets pitch to default */
void	kf_ResetPitch( void )
{
 	player.r.x = DEG(360-20);
	distance = START_DISTANCE;
}

// --------------------------------------------------------------------------
/* Dumps all the keyboard mappings to the console display */
void	kf_ShowMappings( void )
{
	keyShowMappings();
}

// --------------------------------------------------------------------------
/*If this is performed twice then it changes the productionPlayer*/
void	kf_SelectPlayer( void )
{
    UDWORD	playerNumber, prevPlayer;
#ifndef DEBUG
if(bMultiPlayer && (NetPlay.bComms != 0) )
{
	return;
}
#endif
    //store the current player
    prevPlayer = selectedPlayer;

	playerNumber = (getLastSubKey()-KEY_F1);
  	if(playerNumber >= 10)
	{
		selectedPlayer = 0;
	}
	else
	{
		selectedPlayer = playerNumber;
	}
   //	godMode = TRUE;

    if (prevPlayer == selectedPlayer)
    {
        changeProductionPlayer((UBYTE)selectedPlayer);
    }

}
// --------------------------------------------------------------------------

/* Selects the player's groups 1..9 */
void	kf_SelectGrouping( void )
{
UDWORD	groupNumber;
BOOL	bAlreadySelected;
DROID	*psDroid;
BOOL	Selected;

	bAlreadySelected = FALSE;
	groupNumber = (getLastSubKey()-KEY_1) + 1;	
	for(psDroid = apsDroidLists[selectedPlayer]; psDroid!=NULL; psDroid = psDroid->psNext)
	{
		/* Wipe out the ones in the wrong group */
		if(psDroid->selected AND psDroid->group!=groupNumber)
		{
			psDroid->selected = FALSE;
		}
		/* Get the right ones */
		if(psDroid->group == groupNumber)
		{
			if(psDroid->selected)
			{
				bAlreadySelected = TRUE;
			}
		}
	}
	if(bAlreadySelected)
	{
		Selected = activateGroupAndMove(selectedPlayer,groupNumber);
	}
	else
	{
		Selected = activateGroup(selectedPlayer,groupNumber);
	}

	// Tell the driving system that the selection may have changed.
	driveSelectionChanged();
	/* play group audio but only if they wern't already selected - AM */
	if ( Selected AND !bAlreadySelected)
	{
		audio_QueueTrack( ID_SOUND_GROUP_0+groupNumber );
		audio_QueueTrack( ID_SOUND_REPORTING );
		audio_QueueTrack( ID_SOUND_RADIOCLICK_1+(rand()%6) );
	}
}


// --------------------------------------------------------------------------

void	kf_AssignGrouping( void )
{
UDWORD	groupNumber;

	groupNumber = (getLastSubKey()-KEY_1) + 1;	
	assignDroidsToGroup(selectedPlayer,groupNumber);
}

// --------------------------------------------------------------------------

void	kf_SelectCommander( void )
{
SDWORD	cmdNumber;

	cmdNumber = (getLastSubKey()-KEY_1) + 1;	

	// now select the appropriate commander
	selCommander(cmdNumber);
}


// --------------------------------------------------------------------------
void	kf_SelectMoveGrouping( void )
{
UDWORD	groupNumber;

	groupNumber = (getLastSubKey()-KEY_1) + 1;
	activateGroupAndMove(selectedPlayer,groupNumber);
}
// --------------------------------------------------------------------------
void	kf_ToggleDroidInfo( void )
{
	camToggleInfo();
}
// --------------------------------------------------------------------------
void	kf_addInGameOptions( void )
{
		setWidgetsStatus(TRUE);
		intAddInGameOptions();
}
// --------------------------------------------------------------------------
/* Tell the scripts to start a mission*/
void	kf_AddMissionOffWorld( void )
{
#ifndef DEBUG
if(bMultiPlayer)
{
	return;
}
#endif
	game_SetValidityKey(VALIDITYKEY_CTRL_M);
	eventFireCallbackTrigger(CALL_MISSION_START);
}
// --------------------------------------------------------------------------
/* Tell the scripts to end a mission*/
void	kf_EndMissionOffWorld( void )
{
#ifndef DEBUG
if(bMultiPlayer)
{
	return;
}
#endif
	eventFireCallbackTrigger(CALL_MISSION_END);
}
// --------------------------------------------------------------------------
/* Initialise the player power levels*/
void	kf_NewPlayerPower( void )
{
#ifndef DEBUG
if(bMultiPlayer)
{
	return;
}
#endif
	newGameInitPower();
}

// --------------------------------------------------------------------------
// Display multiplayer guff.
void	kf_addMultiMenu(void)
{
	if(bMultiPlayer)
	{
		intAddMultiMenu();
	}
}

// --------------------------------------------------------------------------
// start/stop capturing audio for multiplayer
void kf_multiAudioStart(void)
{
	if(bMultiPlayer							// multiplayer game
		&& game.bytesPerSec==IPXBYTESPERSEC	// ipx type
		&& !NetPlay.bCaptureInUse)			// noone else talking.
	{
		NETstartAudioCapture();
	}
	return;
}

void kf_multiAudioStop(void)
{	
	if(bMultiPlayer 
		&& game.bytesPerSec==IPXBYTESPERSEC)
	{	
		NETstopAudioCapture();
	}
	return;
}
// --------------------------------------------------------------------------

void	kf_JumpToMapMarker( void )
{
UDWORD	entry;
	if(!getRadarTrackingStatus())
	{
		entry = getLastSubKey();
//		CONPRINTF(ConsoleString,(ConsoleString,"Restoring map position %d:%d",getMarkerX(entry),getMarkerY(entry)));
		player.p.x = getMarkerX(entry);
		player.p.z = getMarkerY(entry);
		player.r.y = getMarkerSpin(entry);
		/* A fix to stop the camera continuing when marker code is called */
		if(getWarCamStatus())
		{
			camToggleStatus();
		}
	}
}


// --------------------------------------------------------------------------
/* Raises the G Offset */
void	kf_UpGeoOffset( void )
{
	geoOffset++;
}
// --------------------------------------------------------------------------
/* Lowers the geoOffset */
void	kf_DownGeoOffset( void )
{
	geoOffset--;
}
// --------------------------------------------------------------------------
/* Ups the droid scale */
void	kf_UpDroidScale( void )
{
	droidScale++;
}
// --------------------------------------------------------------------------
/* Lowers the droid scale */
void	kf_DownDroidScale( void )
{
	if(droidScale>2)
	{
		droidScale--;
	}
}
// --------------------------------------------------------------------------
/* Toggles the power bar display on and off*/
void	kf_TogglePowerBar( void )
{
	togglePowerBar();
}
// --------------------------------------------------------------------------
/* Toggles whether we process debug key mappings */
void	kf_ToggleDebugMappings( void )
{
#ifndef DEBUG
if(bMultiPlayer && (NetPlay.bComms != 0) )
{
	return;
}
#endif

#ifndef DEBUG
	if(bAllowDebugMode)
#endif
	{
		if(getDebugMappingStatus())
		{
			processDebugMappings(FALSE);
			CONPRINTF(ConsoleString,(ConsoleString,"ALL Debug Key Mappings - DISALLOWED"));
		}
		else
		{
			game_SetValidityKey(VALIDITYKEY_CHEAT_MODE);
			processDebugMappings(TRUE);
			CONPRINTF(ConsoleString,(ConsoleString,"ALL Debug Key Mappings - PERMITTED"));
			CONPRINTF(ConsoleString,(ConsoleString,"DISCLAIMER: YOU HAVE NOW CHEATED"));
		}
		if(bMultiPlayer)
		{
			sendTextMessage("Presses Debug. CHEAT",TRUE);
		}
	}
}
// --------------------------------------------------------------------------


void	kf_ToggleGodMode( void )
{
#ifndef DEBUG
if(bMultiPlayer && (NetPlay.bComms != 0) )
{
	return;
}
#endif
	if(godMode)
	{
		godMode = FALSE;
//		setDifficultyLevel(getDifficultyLevel());
		CONPRINTF(ConsoleString,(ConsoleString,"God Mode OFF"));
	}
	else
	{
		godMode = TRUE;
//		setModifiers(FRACTCONST(1000,100),FRACTCONST(100,1000));
		CONPRINTF(ConsoleString,(ConsoleString,"God Mode ON"));
	}

}
// --------------------------------------------------------------------------
/* Aligns the view to north - some people can't handle the world spinning */
void	kf_SeekNorth( void )
{
	player.r.y = 0;
	if(getWarCamStatus())
	{
		camToggleStatus();
	}
	CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_NORTH)));
}

// --------------------------------------------------------------------------
void	kf_TogglePauseMode( void )
{

	if(bMultiPlayer && (NetPlay.bComms != 0) )
	{
		return;
	}
	/* Is the game running? */
	if(gamePaused() == FALSE)
	{
		/* Then pause it */
		setGamePauseStatus(TRUE);		
		setConsolePause(TRUE);
		setScriptPause(TRUE);
		setAudioPause(TRUE);
		/* And stop the clock */
		gameTimeStop();
		addConsoleMessage(strresGetString(psStringRes,STR_MISC_PAUSED),CENTRE_JUSTIFY);

	}
	else
	{
		/* Else get it going again */
		setGamePauseStatus(FALSE);
		setConsolePause(FALSE);
		setScriptPause(FALSE);
		setAudioPause(FALSE);
		/* And start the clock again */
		gameTimeStart();
	}
}

// --------------------------------------------------------------------------
// finish all the research for the selected player
void	kf_FinishResearch( void )
{
	STRUCTURE	*psCurr;

	//for (psCurr=apsStructLists[selectedPlayer]; psCurr; psCurr = psCurr->psNext)
	for (psCurr=interfaceStructList(); psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->pStructureType->type == REF_RESEARCH)
		{
			//((RESEARCH_FACILITY *)psCurr->pFunctionality)->timeStarted = 0;
			((RESEARCH_FACILITY *)psCurr->pFunctionality)->timeStarted = gameTime + 100000;
			//set power accrued to high value so that will trigger straight away
			((RESEARCH_FACILITY *)psCurr->pFunctionality)->powerAccrued = 10000;
		}
	}
}

// --------------------------------------------------------------------------
//void	kf_ToggleRadarAllign( void )
//{
//	toggleRadarAllignment();
//	addConsoleMessage("Radar allignment toggled",LEFT_JUSTIFY);
//}

// --------------------------------------------------------------------------
void	kf_ToggleEnergyBars( void )
{
	toggleEnergyBars();
	CONPRINTF(ConsoleString,(ConsoleString, strresGetString(psStringRes,STR_GAM_ENERGY ) ));
}
// --------------------------------------------------------------------------
void	kf_ToggleReloadBars( void )
{
	toggleReloadBarDisplay();
	CONPRINTF(ConsoleString,(ConsoleString, strresGetString(psStringRes,STR_GAM_ENERGY ) ));
}

// --------------------------------------------------------------------------
void	kf_ChooseOptions( void )
{
//	if(!widgetsOn) widgetsOn = TRUE;
	intResetScreen(TRUE);
	setWidgetsStatus(TRUE);
	intAddOptions();
}

// --------------------------------------------------------------------------
void	kf_ToggleBlips( void )
{
	if(doWeDrawRadarBlips())
	{
		setBlipDraw(FALSE);
	}
	else
	{
		setBlipDraw(TRUE);
	}
}
// --------------------------------------------------------------------------
void	kf_ToggleProximitys( void )
{
	if(doWeDrawProximitys())
	{
		setProximityDraw(FALSE);
	}
	else
	{
		setProximityDraw(TRUE);
	}
}
// --------------------------------------------------------------------------
void	kf_JumpToResourceExtractor( void )
{
STRUCTURE	*psStruct;
SDWORD	xJump,yJump;

	psStruct = getRExtractor(psOldRE);
	if(psStruct)
	{
		xJump = (psStruct->x - ((visibleXTiles/2)*TILE_UNITS));
		yJump = (psStruct->y - ((visibleYTiles/2)*TILE_UNITS));
		player.p.x = xJump;
		player.p.z = yJump;
		player.r.y = 0; // face north
		setViewPos(psStruct->x>>TILE_SHIFT,psStruct->y>>TILE_SHIFT,TRUE);
		psOldRE = psStruct;
	}
	else
	{
		addConsoleMessage(strresGetString(psStringRes,STR_GAM_RESNOTFOUND),LEFT_JUSTIFY);
	}

}
// --------------------------------------------------------------------------
void	kf_JumpToRepairUnits( void )
{
	selNextSpecifiedUnit( DROID_REPAIR );
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void	kf_JumpToConstructorUnits( void )
{
	selNextSpecifiedUnit( DROID_CONSTRUCT );
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void	kf_JumpToSensorUnits( void )
{
	selNextSpecifiedUnit( DROID_SENSOR );
}
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void	kf_JumpToCommandUnits( void )
{
	selNextSpecifiedUnit( DROID_COMMAND );
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
void	kf_JumpToUnassignedUnits( void )
{
	selNextUnassignedUnit();
}
// --------------------------------------------------------------------------


void	kf_ToggleOverlays( void )
{
		/* Make sure they're both the same */
//		radarOnScreen = widgetsOn;
		/* Flip their states */
//		radarOnScreen = !radarOnScreen;

	if(getWidgetsStatus())
	{
		setWidgetsStatus(FALSE);
	}
	else
	{
		setWidgetsStatus(TRUE);
	}

}

void	kf_SensorDisplayOn( void )
{
//	debugToggleSensorDisplay();
	startSensorDisplay();
}

void	kf_SensorDisplayOff( void )
{
	stopSensorDisplay();
}


// --------------------------------------------------------------------------
/*
#define IDRET_OPTIONS		2		// option button
#define IDRET_BUILD			3		// build button
#define IDRET_MANUFACTURE	4		// manufacture button
#define IDRET_RESEARCH		5		// research button
#define IDRET_INTEL_MAP		6		// intelligence map button
#define IDRET_DESIGN		7		// design droids button
#define IDRET_CANCEL		8		// central cancel button
*/
// --------------------------------------------------------------------------
void	kf_ChooseCommand( void )
{
	if (intCheckReticuleButEnabled(IDRET_COMMAND))
	{
		setKeyButtonMapping(IDRET_COMMAND);
	}
/*
WIDGET *psWidg;
W_BUTTON *psButton;

	psWidg = widgGetFromID(psWScreen,IDRET_COMMAND);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
	*/
}
// --------------------------------------------------------------------------
void	kf_ChooseManufacture( void )
{
	if (intCheckReticuleButEnabled(IDRET_MANUFACTURE))
	{
		setKeyButtonMapping(IDRET_MANUFACTURE);
	}

	/*
WIDGET *psWidg;
W_BUTTON *psButton;

	psWidg = widgGetFromID(psWScreen,IDRET_MANUFACTURE);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
	*/

}
// --------------------------------------------------------------------------
void	kf_ChooseResearch( void )
{
	if (intCheckReticuleButEnabled(IDRET_RESEARCH))
	{
		setKeyButtonMapping(IDRET_RESEARCH);
	}

	/*
WIDGET *psWidg;
W_BUTTON *psButton;
	psWidg = widgGetFromID(psWScreen,IDRET_RESEARCH);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
	 */
}
// --------------------------------------------------------------------------
void	kf_ChooseBuild( void )
{
	if (intCheckReticuleButEnabled(IDRET_BUILD))
	{
		setKeyButtonMapping(IDRET_BUILD);
	}

	/*
WIDGET *psWidg;
W_BUTTON *psButton;
	psWidg = widgGetFromID(psWScreen,IDRET_BUILD);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
	*/
}

// --------------------------------------------------------------------------
void	kf_ChooseDesign( void )
{
	if (intCheckReticuleButEnabled(IDRET_DESIGN))
	{
		setKeyButtonMapping(IDRET_DESIGN);
	}

	/*
WIDGET *psWidg;
W_BUTTON *psButton;
	psWidg = widgGetFromID(psWScreen,IDRET_DESIGN);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
	*/
}
// --------------------------------------------------------------------------
void	kf_ChooseIntelligence( void )
{
	if (intCheckReticuleButEnabled(IDRET_INTEL_MAP))
	{
		setKeyButtonMapping(IDRET_INTEL_MAP);
	}

	/*
WIDGET *psWidg;
W_BUTTON *psButton;
	psWidg = widgGetFromID(psWScreen,IDRET_INTEL_MAP);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
	*/

}
// --------------------------------------------------------------------------

void	kf_ChooseCancel( void )
{
	setKeyButtonMapping(IDRET_CANCEL);

/*
WIDGET *psWidg;
W_BUTTON *psButton;
	psWidg = widgGetFromID(psWScreen,IDRET_CANCEL);
	psButton = (W_BUTTON*)psWidg;
	buttonClicked(psButton,WKEY_PRIMARY);
  */

}
// --------------------------------------------------------------------------
void	kf_ToggleDrivingMode( void )
{
	/* No point unless we're tracking */
	if(getWarCamStatus())
	{
		if(getDrivingStatus())
		{
			StopDriverMode();
		}
		else
		{
			if(	(driveModeActive() == FALSE) && !bMultiPlayer)
			{
				StartDriverMode( NULL );
			}
		}
	}

}

BOOL	bMovePause = FALSE;

// --------------------------------------------------------------------------
void	kf_MovePause( void )
{
	
	if(!bMultiPlayer)	// can't do it in multiplay
	{
		if(!bMovePause)
		{
			/* Then pause it */
			setGamePauseStatus(TRUE);		
			setConsolePause(TRUE);
			setScriptPause(TRUE);
			setAudioPause(TRUE);
			/* And stop the clock */
			gameTimeStop();
			setWidgetsStatus(FALSE);
			radarOnScreen = FALSE;
			bMovePause = TRUE;
		}
		else
		{
			setWidgetsStatus(TRUE);
			radarOnScreen = TRUE;
			/* Else get it going again */
			setGamePauseStatus(FALSE);
			setConsolePause(FALSE);
			setScriptPause(FALSE);
			setAudioPause(FALSE);
			/* And start the clock again */
			gameTimeStart();
			bMovePause = FALSE;
		}
	}
}
// --------------------------------------------------------------------------
void	kf_MoveToLastMessagePos( void )
{
	SDWORD	iX, iY, iZ;

	if ( audio_GetPreviousQueueTrackPos( &iX, &iY, &iZ ) )
	{
// Should use requestRadarTrack but the camera gets jammed so use setViewpos - GJ
//		requestRadarTrack( iX, iY );
		setViewPos( iX>>TILE_SHIFT, iY>>TILE_SHIFT, TRUE );
	}
}
// --------------------------------------------------------------------------
/* Makes it snow if it's not snowing and stops it if it is */
void	kf_ToggleWeather( void )
{
	if(atmosGetWeatherType() == WT_NONE)
	{
		atmosSetWeatherType(WT_SNOWING);
		addConsoleMessage("Oh, the weather outside is frightful... SNOW",LEFT_JUSTIFY);

	}
	else if(atmosGetWeatherType() == WT_SNOWING)
	{
		atmosSetWeatherType(WT_RAINING);
		addConsoleMessage("Singing in the rain, I'm singing in the rain... RAIN",LEFT_JUSTIFY);
	}
	else
	{
		atmosInitSystem();
		atmosSetWeatherType(WT_NONE);
		addConsoleMessage("Forecast : Clear skies for all areas... NO WEATHER",LEFT_JUSTIFY);
	}
}

// --------------------------------------------------------------------------
void	kf_SelectNextFactory(void)
{
	selNextSpecifiedBuilding(REF_FACTORY);
}
// --------------------------------------------------------------------------
void	kf_SelectNextResearch(void)
{
	selNextSpecifiedBuilding(REF_RESEARCH);
}
// --------------------------------------------------------------------------
void	kf_SelectNextPowerStation(void)
{
	selNextSpecifiedBuilding(REF_POWER_GEN);
}
// --------------------------------------------------------------------------
void	kf_SelectNextCyborgFactory(void)
{
	selNextSpecifiedBuilding(REF_CYBORG_FACTORY);
}
// --------------------------------------------------------------------------


void	kf_KillEnemy( void )
{
UDWORD	player;
DROID	*psCDroid,*psNDroid;
STRUCTURE	*psCStruct, *psNStruct;


	for(player = 0; player<MAX_PLAYERS AND !bMultiPlayer; player++)
	{
		if(player!=selectedPlayer)
		{
		 	// wipe out all the droids
			for(psCDroid=apsDroidLists[player]; psCDroid; psCDroid=psNDroid)
			{
				psNDroid = psCDroid->psNext;
				destroyDroid(psCDroid);
			}
			// wipe out all their structures
		  //	for(psCStruct=apsStructLists[player]; psCStruct; psCStruct=psNStruct)
		  //	{
		  //		psNStruct = psCStruct->psNext;
		  //		destroyStruct(psCStruct);
		  //	}
		}
	}
}

// kill all the selected objects
void kf_KillSelected(void)
{
	DROID		*psCDroid, *psNDroid;
	STRUCTURE	*psCStruct, *psNStruct;

#ifndef DEBUG
if(bMultiPlayer)
{
	return;
}
#endif

	for(psCDroid=apsDroidLists[selectedPlayer]; psCDroid; psCDroid=psNDroid)
	{
		psNDroid = psCDroid->psNext;
		if (psCDroid->selected)
		{
//		  	removeDroid(psCDroid);
			destroyDroid(psCDroid);
		}
	}
	for(psCStruct=apsStructLists[selectedPlayer]; psCStruct; psCStruct=psNStruct)
	{
		psNStruct = psCStruct->psNext;
		if (psCStruct->selected)
		{
			destroyStruct(psCStruct);
		}
	}
}

// --------------------------------------------------------------------------
// display the grid info for all the selected objects
void kf_ShowGridInfo(void)
{
	DROID		*psCDroid, *psNDroid;
	STRUCTURE	*psCStruct, *psNStruct;

	for(psCDroid=apsDroidLists[selectedPlayer]; psCDroid; psCDroid=psNDroid)
	{
		psNDroid = psCDroid->psNext;
		if (psCDroid->selected)
		{
			gridDisplayCoverage((BASE_OBJECT *)psCDroid);
		}
	}
	for(psCStruct=apsStructLists[selectedPlayer]; psCStruct; psCStruct=psNStruct)
	{
		psNStruct = psCStruct->psNext;
		if (psCStruct->selected)
		{
			gridDisplayCoverage((BASE_OBJECT *)psCStruct);
		}
	}
}


// --------------------------------------------------------------------------
void kf_GiveTemplateSet(void)
{
	addTemplateSet(4,0);
	addTemplateSet(4,1);
	addTemplateSet(4,2);
	addTemplateSet(4,3);
}

// --------------------------------------------------------------------------
// Chat message. NOTE THIS FUNCTION CAN DISABLE ALL OTHER KEYPRESSES
void kf_SendTextMessage(void)
{
	CHAR	ch;									

	if(/*bMultiPlayer || */!bAllowOtherKeyPresses OR getCheatCodeStatus()) 
	{
		if(bAllowOtherKeyPresses)									// just starting.
		{
			bAllowOtherKeyPresses = FALSE;
			strcpy(sTextToSend,"");
			inputClearBuffer();										
		}

		ch = (CHAR)inputGetKey();
		while(ch != 0)												// in progress
		{
			// Kill if they hit return - it maxes out console or it's more than one line long
		   	if((ch == INPBUF_CR) || (strlen(sTextToSend)>=MAX_CONSOLE_STRING_LENGTH-16) // Prefixes with ERROR: and terminates with '?'
				OR iV_GetTextWidth(sTextToSend) > (DISP_WIDTH-64))// sendit
		   //	if((ch == INPBUF_CR) || (strlen(sTextToSend)==MAX_TYPING_LENGTH) 
			{
				bAllowOtherKeyPresses = TRUE;
			 //	flushConsoleMessages();					
				if(bMultiPlayer && NetPlay.bComms)
				{
					sendTextMessage(sTextToSend,FALSE);
				}
				else if(getCheatCodeStatus())
				{
					(void) attemptCheatCode(sTextToSend);
				}
				return;
			}
			else if(ch == INPBUF_BKSPACE )							// delete 
			{
				if(sTextToSend[0] != '\0')							// cant delete nothing!
				{
					sTextToSend[strlen(sTextToSend)-1]= '\0';
				}
			}
			else if(ch == INPBUF_ESC)								//abort.
			{
				bAllowOtherKeyPresses = TRUE;
			 //	flushConsoleMessages();						
				return;
			}		
			else							 						// display
			{
				sprintf(sTextToSend,"%s%c\0",sTextToSend,ch);
			}	

			ch = (CHAR)inputGetKey();
		}

		// macro store stuff
		if(keyPressed(KEY_F1)){
			if(keyDown(KEY_LCTRL)){
				strcpy(ingame.phrases[0],sTextToSend );
			}else{
				strcpy(sTextToSend,ingame.phrases[0]);
				bAllowOtherKeyPresses = TRUE;
			 //	flushConsoleMessages();					
				sendTextMessage(sTextToSend,FALSE);
				return;
			}
		}
		if(keyPressed(KEY_F2)){
			if(keyDown(KEY_LCTRL)){
				strcpy(ingame.phrases[1],sTextToSend );
			}else{
				strcpy(sTextToSend,ingame.phrases[1]);
				bAllowOtherKeyPresses = TRUE;
			//	flushConsoleMessages();					
				sendTextMessage(sTextToSend,FALSE);			
				return;
			}
		}
		if(keyPressed(KEY_F3)){
			if(keyDown(KEY_LCTRL)){
				strcpy(ingame.phrases[2],sTextToSend );
			}else{
				strcpy(sTextToSend,ingame.phrases[2]);
				bAllowOtherKeyPresses = TRUE;
			//	flushConsoleMessages();					
				sendTextMessage(sTextToSend,FALSE);
				return;
			}
		}
		if(keyPressed(KEY_F4)){
			if(keyDown(KEY_LCTRL)){
				strcpy(ingame.phrases[3],sTextToSend );
			}else{
				strcpy(sTextToSend,ingame.phrases[3]);
				bAllowOtherKeyPresses = TRUE;
			//	flushConsoleMessages();					
				sendTextMessage(sTextToSend,FALSE);
				return;
			}
		}
		if(keyPressed(KEY_F5)){
			if(keyDown(KEY_LCTRL)){
				strcpy(ingame.phrases[4],sTextToSend );			
			}else{
				strcpy(sTextToSend,ingame.phrases[4]);
				bAllowOtherKeyPresses = TRUE;
			 //	flushConsoleMessages();					
				sendTextMessage(sTextToSend,FALSE);
				return;
			}
		}
		

//		flushConsoleMessages();								//clear
//		addConsoleMessage(sTextToSend,DEFAULT_JUSTIFY);		//display
//		iV_DrawText(sTextToSend,16+D_W,RADTLY+D_H-16);
		return;
	}
}
// --------------------------------------------------------------------------
void	kf_ToggleConsole( void )
{
	if(getConsoleDisplayStatus())
	{
		enableConsoleDisplay(FALSE);
	}
	else
	{
		enableConsoleDisplay(TRUE);
	}
}
// --------------------------------------------------------------------------
void	kf_SelectAllOnScreenUnits( void )
{

	selDroidSelection(selectedPlayer, DS_ALL_UNITS, DST_UNUSED, TRUE);

/*
DROID	*psDroid;
UDWORD	dX,dY;

	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		if (DrawnInLastFrame(psDroid->sDisplay.frameNumber)==TRUE)
		{
			dX = psDroid->sDisplay.screenX;
			dY = psDroid->sDisplay.screenY;
			if(dX>0 AND dY>0 AND dX<DISP_WIDTH AND dY<DISP_HEIGHT)
			{
				psDroid->selected = TRUE;
			}
		}
	}
*/
}
// --------------------------------------------------------------------------
void	kf_SelectAllUnits( void )
{
 
	selDroidSelection(selectedPlayer, DS_ALL_UNITS, DST_UNUSED, FALSE);

/*
DROID	*psDroid;
	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		psDroid->selected = TRUE;
	}
*/
}
// --------------------------------------------------------------------------
void	kf_SelectAllVTOLs( void )
{
  //	kfsf_SelectAllSameProp(LIFT);
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_VTOL,FALSE);
}
// --------------------------------------------------------------------------
void	kf_SelectAllHovers( void )
{
//	kfsf_SelectAllSameProp(HOVER);
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_HOVER,FALSE);
}
// --------------------------------------------------------------------------
void	kf_SelectAllWheeled( void )
{
//	kfsf_SelectAllSameProp(WHEELED);
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_WHEELED,FALSE);
}
// --------------------------------------------------------------------------
void	kf_SelectAllTracked( void )
{
//	kfsf_SelectAllSameProp(TRACKED);
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_TRACKED,FALSE);
}
// --------------------------------------------------------------------------
void	kf_SelectAllHalfTracked( void )
{
//	kfsf_SelectAllSameProp(HALF_TRACKED);
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_HALF_TRACKED,FALSE);
}

// --------------------------------------------------------------------------
void	kf_SelectAllDamaged( void )
{
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_ALL_DAMAGED,FALSE);
/*
DROID	*psDroid;
UDWORD	damage;

	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		damage = PERCENT(psDroid->body,psDroid->originalBody);
		if(damage<REPAIRLEV_LOW)
		{
			psDroid->selected = TRUE;
		}
	}
*/
}
// --------------------------------------------------------------------------
void	kf_SelectAllCombatUnits( void )
{
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_ALL_COMBAT,FALSE);

/*
DROID	*psDroid;
	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		if(psDroid->numWeaps)
		{
			psDroid->selected = TRUE;
		}	
	}
*/
}
// --------------------------------------------------------------------------
void	kfsf_SelectAllSameProp( PROPULSION_TYPE propType )
{
    UNUSEDPARAMETER(propType);
	/*
PROPULSION_STATS	*psPropStats;
DROID	*psDroid;

	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		if(!psDroid->selected)
		{
			psPropStats = asPropulsionStats + psDroid->asBits[COMP_PROPULSION].nStat;
			ASSERT( (PTRVALID(psPropStats, sizeof(PROPULSION_STATS)),
					"moveUpdateDroid: invalid propulsion stats pointer") );
			if ( psPropStats->propulsionType == propType )
			{
				psDroid->selected = TRUE;
			}
		}
	}
	*/
}
// --------------------------------------------------------------------------
// this is worst case (size of apsDroidLists[selectedPlayer] squared).
// --------------------------------------------------------------------------
void	kf_SelectAllSameType( void )
{
	selDroidSelection(selectedPlayer,DS_BY_TYPE,DST_ALL_SAME,FALSE);
	/*
DROID	*psDroid;
//PROPULSION_STATS	*psPropStats;

	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		if(psDroid->selected)
		{
//			psPropStats = asPropulsionStats + psDroid->asBits[COMP_PROPULSION].nStat;
//			kfsf_SelectAllSameProp(psPropStats->propulsionType);	// non optimal - multiple assertion!?
			kfsf_SelectAllSameName(psDroid->aName);
		}
	}
	*/
}
// --------------------------------------------------------------------------
void	kfsf_SelectAllSameName( STRING *droidName )
{
    UNUSEDPARAMETER(droidName);

	/*
DROID	*psDroid;
	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		// already selected - ignore
		if(!psDroid->selected)
		{
			if(!strcmp(droidName,psDroid->aName))
			{
				psDroid->selected = TRUE;
			}
		}
	}
	*/
}
// --------------------------------------------------------------------------
void	kf_SetDroidRangeShort( void )
{
	kfsf_SetSelectedDroidsState(DSO_ATTACK_RANGE,DSS_ARANGE_SHORT);
}
// --------------------------------------------------------------------------
void	kf_SetDroidRangeDefault( void )
{
	kfsf_SetSelectedDroidsState(DSO_ATTACK_RANGE,DSS_ARANGE_DEFAULT);
}
// --------------------------------------------------------------------------
void	kf_SetDroidRangeLong( void )
{
	kfsf_SetSelectedDroidsState(DSO_ATTACK_RANGE,DSS_ARANGE_LONG);
}

// --------------------------------------------------------------------------
void	kf_SetDroidRetreatMedium( void )
{
	kfsf_SetSelectedDroidsState(DSO_REPAIR_LEVEL,DSS_REPLEV_LOW);
}
// --------------------------------------------------------------------------
void	kf_SetDroidRetreatHeavy( void )
{
	kfsf_SetSelectedDroidsState(DSO_REPAIR_LEVEL,DSS_REPLEV_HIGH);
}
// --------------------------------------------------------------------------
void	kf_SetDroidRetreatNever( void )
{
	kfsf_SetSelectedDroidsState(DSO_REPAIR_LEVEL,DSS_REPLEV_NEVER);
}
// --------------------------------------------------------------------------
void	kf_SetDroidAttackAtWill( void )
{
	kfsf_SetSelectedDroidsState(DSO_ATTACK_LEVEL,DSS_ALEV_ALWAYS);
}
// --------------------------------------------------------------------------
void	kf_SetDroidAttackReturn( void )
{
	kfsf_SetSelectedDroidsState(DSO_ATTACK_LEVEL,DSS_ALEV_ATTACKED);
}
// --------------------------------------------------------------------------
void	kf_SetDroidAttackCease( void )
{
	kfsf_SetSelectedDroidsState(DSO_ATTACK_LEVEL,DSS_ALEV_NEVER);
}

// --------------------------------------------------------------------------
void	kf_SetDroidMoveHold( void )
{
	kfsf_SetSelectedDroidsState(DSO_HALTTYPE,DSS_HALT_HOLD);
}
// --------------------------------------------------------------------------
void	kf_SetDroidMovePursue( void )
{
	kfsf_SetSelectedDroidsState(DSO_HALTTYPE,DSS_HALT_PERSUE);	// ASK?
}
// --------------------------------------------------------------------------
void	kf_SetDroidMovePatrol( void )
{
	kfsf_SetSelectedDroidsState(DSO_PATROL,DSS_PATROL_SET);	// ASK
}
// --------------------------------------------------------------------------
void	kf_SetDroidReturnToBase( void )
{
	kfsf_SetSelectedDroidsState(DSO_RETURN_TO_LOC,DSS_RTL_BASE);
}
// --------------------------------------------------------------------------
void	kf_SetDroidGoForRepair( void )
{
	kfsf_SetSelectedDroidsState(DSO_RETURN_TO_LOC,DSS_RTL_REPAIR);
}
// --------------------------------------------------------------------------
void	kf_SetDroidRecycle( void )
{
	kfsf_SetSelectedDroidsState(DSO_RECYCLE,DSS_RECYCLE_SET);
}
// --------------------------------------------------------------------------
void	kf_ToggleVisibility( void )
{
	if(getRevealStatus())
	{
		setRevealStatus(FALSE);
	}
	else
	{
		setRevealStatus(TRUE);
	}

}
// --------------------------------------------------------------------------
void	kfsf_SetSelectedDroidsState( SECONDARY_ORDER sec, SECONDARY_STATE state )
{
DROID	*psDroid;

	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		if(psDroid->selected)
		{
			secondarySetState(psDroid,sec,state);
			/* Kick him out of group if he's going for repair */	  // Now done in secondarySetState
		   //	if ((sec == DSO_RETURN_TO_LOC) && (state == DSS_RTL_REPAIR))
		   //	{
		   //		psDroid->group = UBYTE_MAX;
		   //		psDroid->selected = FALSE;
		   //	}
		}
	}
}
// --------------------------------------------------------------------------
void	kf_TriggerRayCast( void )
{
DROID	*psDroid;
BOOL	found;
DROID	*psOther;

	found = FALSE;
	for(psDroid = apsDroidLists[selectedPlayer]; psDroid AND !found; 
		psDroid = psDroid->psNext)
		{
			if(psDroid->selected)
			{
				found = TRUE;
				psOther = psDroid;
			}
			/* NOP */
		}

	if(found)
	{
//		 getBlockHeightDirToEdgeOfGrid(UDWORD x, UDWORD y, UBYTE direction, UDWORD *height, UDWORD *dist)
   //		getBlockHeightDirToEdgeOfGrid(psOther->x,psOther->y,psOther->direction,&height,&dist);
//		getBlockHeightDirToEdgeOfGrid(mouseTileX*TILE_UNITS,mouseTileY*TILE_UNITS,getTestAngle(),&height,&dist);
	}
}
// --------------------------------------------------------------------------

void	kf_ScatterDroids( void )
{
	// to be written!
	addConsoleMessage("Scatter droids - not written yet!",LEFT_JUSTIFY);
}// --------------------------------------------------------------------------
void	kf_CentreOnBase( void )
{
STRUCTURE	*psStruct;
BOOL		bGotHQ;
UDWORD		xJump,yJump;

	/* Got through our buildings */
	for(psStruct = apsStructLists[selectedPlayer],bGotHQ = FALSE;	// start
	psStruct AND !bGotHQ;											// terminate
	psStruct = psStruct->psNext)									// iteration
	{
		/* Have we got a HQ? */
		if(psStruct->pStructureType->type == REF_HQ)
		{
			bGotHQ = TRUE;
			xJump = (psStruct->x - ((visibleXTiles/2)*TILE_UNITS));
			yJump = (psStruct->y - ((visibleYTiles/2)*TILE_UNITS));
		}
	}

	/* If we found it, then jump to it! */
	if(bGotHQ)
	{
		addConsoleMessage(strresGetString(psStringRes,STR_GAM_GOHQ),LEFT_JUSTIFY);
		player.p.x = xJump;
		player.p.z = yJump;
		player.r.y = 0; // face north
		/* A fix to stop the camera continuing when marker code is called */
		if(getWarCamStatus())
		{
			camToggleStatus();
		}
	}
	else
	{
		addConsoleMessage(strresGetString(psStringRes,STR_GAM_NOHQ),LEFT_JUSTIFY);
	}
}

// --------------------------------------------------------------------------

void kf_ToggleFormationSpeedLimiting( void )
{
	if(bMultiPlayer)
	{
		return;
	}
	if ( moveFormationSpeedLimitingOn() )
	{
		addConsoleMessage(strresGetString(psStringRes,STR_GAM_FORMATION_OFF),LEFT_JUSTIFY);
	}
	else
	{
		addConsoleMessage(strresGetString(psStringRes,STR_GAM_FORMATION_ON),LEFT_JUSTIFY);
	}
	moveToggleFormationSpeedLimiting();
}

// --------------------------------------------------------------------------
void	kf_RightOrderMenu( void )
{
DROID	*psDroid,*psGotOne;
BOOL	bFound;

	// if menu open, then close it!
	if (widgGetFromID(psWScreen,IDORDER_FORM) != NULL)
	{
		intRemoveOrder();	// close the screen.
		return;
	}


	for(psDroid = apsDroidLists[selectedPlayer],bFound = FALSE;
		psDroid AND !bFound; psDroid = psDroid->psNext)
	{
		if(psDroid->selected)// AND droidOnScreen(psDroid,0))
		{
			bFound = TRUE;
			psGotOne = psDroid;
		}	
	}
	if(bFound)
	{
		intResetScreen(TRUE);
		intObjectSelected((BASE_OBJECT*)psGotOne);
	}
}
// --------------------------------------------------------------------------
void kf_ScriptTest( void )
{
	UBYTE	*pBuffer;
	UDWORD	size;

	eventSaveState(1,&pBuffer, &size);

	eventReset();

	eventLoadState(pBuffer, size, TRUE);

	FREE(pBuffer);
}
// --------------------------------------------------------------------------
void kf_TriggerShockWave( void )
{
iVector	pos;

		pos.x = mouseTileX*TILE_UNITS + TILE_UNITS/2;
		pos.z = mouseTileY*TILE_UNITS + TILE_UNITS/2;
		pos.y = map_Height(pos.x,pos.z) + SHOCK_WAVE_HEIGHT; 

		addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SHOCKWAVE,FALSE,NULL,0);
}
// --------------------------------------------------------------------------
void	kf_ToggleMouseInvert( void )
{
	if(getInvertMouseStatus())
	{
		setInvertMouseStatus(FALSE);
	}
	else
	{
		setInvertMouseStatus(TRUE);
	}
}
// --------------------------------------------------------------------------
void	kf_ToggleShakeStatus( void )
{
	if(getShakeStatus())
	{
		setShakeStatus(FALSE);
	}
	else
	{
		setShakeStatus(TRUE);
	}

}
// --------------------------------------------------------------------------

void kf_SpeedUp( void )
{
	FRACT	mod, fast1,fast2;

	if (getDebugMappingStatus())
	{
		fast1 = FRACTCONST(3,2);
		fast2 = FRACTCONST(2,1);
	}
	else
	{
		fast1 = FRACTCONST(5,4);
		fast2 = FRACTCONST(3,2);
	}

	if ( (!bMultiPlayer || (NetPlay.bComms==0) )  && !bInTutorial)
	{
		// get the current modifier
		gameTimeGetMod(&mod);

		// increase it
		if (mod < FRACTCONST(1,2))
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_SPEED_UP),FRACTCONST(1,2)));
			mod = FRACTCONST(1,2);
		}
		else if (mod < FRACTCONST(1,1))
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_NORMAL_SPEED)));
			mod = FRACTCONST(1,1);
		}
		else if (mod < fast1)
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_SPEED_UP),fast1));
			mod = fast1;
		}
		else if (mod < fast2)
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_SPEED_UP),fast2));
			mod = fast2;
		}
		gameTimeSetMod(mod);
	}
}

void kf_SlowDown( void )
{
	FRACT	mod, fast1,fast2;

	if (getDebugMappingStatus())
	{
		fast1 = FRACTCONST(3,2);
		fast2 = FRACTCONST(2,1);
	}
	else
	{
		fast1 = FRACTCONST(5,4);
		fast2 = FRACTCONST(3,2);
	}

	if ( (!bMultiPlayer || (NetPlay.bComms==0)) && !bInTutorial )
	{
		// get the current modifier
		gameTimeGetMod(&mod);

		// decrease it
		if (mod > fast1)
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_SLOW_DOWN),fast1));
			mod = fast1;
		}
		else if (mod > FRACTCONST(1,1))
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_NORMAL_SPEED)));
			mod = FRACTCONST(1,1);
		}
		else if (mod > FRACTCONST(1,2))
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_SLOW_DOWN),FRACTCONST(1,2)));
			mod = FRACTCONST(1,2);
		}
		else if (mod > FRACTCONST(1,3))
		{
			CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_SLOW_DOWN),FRACTCONST(1,3)));
			mod = FRACTCONST(1,3);
		}
		gameTimeSetMod(mod);
	}
}

void kf_NormalSpeed( void )
{
	if ( (!bMultiPlayer || (NetPlay.bComms == 0)) && !bInTutorial)
	{
		CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_NORMAL_SPEED)));
		gameTimeResetMod();
	}
}

// --------------------------------------------------------------------------

void kf_ToggleReopenBuildMenu( void )
{
	intReopenBuild( !intGetReopenBuild() );

	if (intGetReopenBuild())
	{
		CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_BUILD_REOPEN)));
	}
	else
	{
		CONPRINTF(ConsoleString,(ConsoleString,strresGetString(psStringRes,STR_GAM_BUILD_NO_REOPEN)));
	}
}

// --------------------------------------------------------------------------
