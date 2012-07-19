/* 
	Display3D.c - draws the 3D terrain view. Both the 3D and pseudo-3D components:-
	textured tiles.

	  -------------------------------------------------------------------
	  -	Alex McLean & Jeremy Sallis, Pumpkin Studios, EIDOS INTERACTIVE -
	  -------------------------------------------------------------------
*/
/* Generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Includes direct access to render library */
#include "piedef.h"
#include "pieState.h"
#include "pieTexture.h"
#include "pieClip.h"
#include "piePalette.h"
#include "pieMatrix.h"
#include "pieMode.h"
#include "pieFunc.h"
#include "rendMode.h"
#include "bspfunc.h"
#include "E3Demo.h"	// DELETEME ?
#include "Loop.h"
#include "Atmos.h"
#include "RayCast.h"
#include "Levels.h"
#ifdef JEREMY
#include "groundMist.h"
#endif
/* Includes from PUMPKIN stuff */
#include "Frame.h"
#include "Map.h"
#include "Move.h"
#include "Visibility.h"
#include "Findpath.h"
#include "Disp2D.h"
#include "Geometry.h"
#include "GTime.h"
#include "resource.h"
#include "MessageDef.h"
#include "MiscImd.h"
#include "Effects.h"
#include "Edit3D.h"
#include "Feature.h"
#include "HCI.h"
#include "Display.h"
#include "intDisplay.h"
#include "Radar.h"
#include "Display3D.h"
#include "fractions.h"
#include "Lighting.h"
#include "Console.h"
#include "animobj.h"
#include "projectile.h"
#include "bucket3d.h"
#include "intelmap.h"
#include "mapdisplay.h"
#include "message.h"
#include "Component.h"
#include "Bridge.h"
#include "WarCAM.h"
#include "Script.h"
#include "ScriptTabs.h"
#include "ScriptExtern.h"
#include "ScriptCB.h"
#include "target.h"
#include "keymap.h"
#include "drive.h"
#include "fpath.h"
#include "gateway.h"
#include "Transporter.h"
#include "WarZoneConfig.h"
#include "audio.h"
#include "audio_id.h"
#include "action.h"
#include "KeyBind.h"
#include "Combat.h"
#include "order.h"

#include "Scores.h"
#ifdef ARROWS
#include "arrow.h"
#endif

#include "multiplay.h"

#include "environ.h"
#include "AdvVis.h"

#include "Texture.h"
//#ifdef THREEDFX
//#include "Glide.h"
//#endif

#include "anim_id.h"

#include "CmdDroid.h"

#define SPOTLIGHT

#define ENABLE_WATER			// Enable transparent water.
#define WATER_TILE 17			// ID of water tile.
#define BED_TILE 5				// ID of river bed tile.
//#define ADDITIVE_WATER	1		// Additive transparency on water ( 1 = YES ).

#if(ADDITIVE_WATER)
#define WATER_TRANS_MODE pie_ADDITIVE		// Transparency mode for water (pie_TRANSLUCENT or pie_ADDITIVE)
#define WATER_ALPHA_LEVEL 164 //was 164	// Amount to alpha blend water.
#else
#define WATER_TRANS_MODE pie_TRANSLUCENT	// Transparency mode for water (pie_TRANSLUCENT or pie_ADDITIVE)
#define WATER_ALPHA_LEVEL 160	// Amount to alpha blend water.
#endif
#define WATER_ZOFFSET 32		// Sorting offset for main water tile.
#define WATER_EDGE_ZOFFSET 64	// Sorting offset for water edge tiles.
#define WATER_DEPTH	127			// Amount to push terrain below water.


static UWORD WaterTileID = WATER_TILE;
static UWORD RiverBedTileID = BED_TILE;
static FRACT	waterRealValue = (FRACT)0;
static SDWORD	waterAlphaValue = 168;//jps 15Apr99 for d3d only
#define WAVE_SPEED 4
static SWORD vOffset = 1;
#define	MAX_FIRE_STAGE	32
static FRACT	separation=(FRACT)0;
static SDWORD	acceleration=0;
static SDWORD	heightSpeed=0;
static FRACT	aSep;
static SDWORD	aAccel = 0;
static SDWORD	aSpeed = 0;

UDWORD	barMode=BAR_FULL;


//remove this~!
UDWORD	getTargettingGfx( void );
void	drawDroidGroupNumber(DROID *psDroid);
void	toggleEnergyBars( void );
void	toggleReloadBarDisplay( void );
void	trackHeight( SDWORD desiredHeight );
void	setViewAngle(SDWORD angle);
void	setViewDistance(UDWORD dist);
void	getDefaultColours( void );
void renderSky(void);
void	showShadePalette( void );
/* Function prototypes:- */
void	scaleMatrix( UDWORD percent );
void	screenSlideDown( void );
void	locateMouse(void);
void	preprocessTiles(void);
//void	postprocessTiles(void);
BOOL	renderWallSection(STRUCTURE *psStructure);
void	buildTileTextures(void);
void	draw3dLine(iVector *src, iVector *dest, UBYTE col);
UDWORD	getSuggestedPitch			( void );
void	drawDragBox					( void );
void    setViewPos					( UDWORD x, UDWORD y,BOOL Pan );
void	calcScreenCoords			( DROID *psDroid );
void	calcFlagPosScreenCoords		( SDWORD *pX, SDWORD *pY, SDWORD *pR);
BOOL	clipDroid					( DROID *psDroid );
BOOL	clipXY						( SDWORD x, SDWORD y);
void	moveDroids					( void );
void	adjustTileHeight			( MAPTILE *psTile, SDWORD adjust );
BOOL	init3DView					( void );
int		makeTileTextures			( void );
int		remakeTileTextures			( void );
void	flipsAndRots				( int texture );
void	displayTerrain				( void );
void	draw3DScene					( void );
iIMDShape	*flattenImd(iIMDShape *imd, UDWORD structX, UDWORD structY, UDWORD direction);
void	RenderCompositeDroid		( UDWORD Index,iVector *Rotation,
									  iVector *Position,
									  iVector *TurretRotation,
									  DROID *psDroid,BOOL RotXYZ );
void	drawTiles					( iView *camera, iView *player );
void	display3DProjectiles		( void );

void	displaySprites				( void );
void	drawDroidSelections			( void );
void	drawStructureSelections		( void );
void	drawBuildingLines			( void );

void	displayAnimation( ANIM_OBJECT * psAnimObj, BOOL bHoldOnFirstFrame );
//void	assignSensorTarget( DROID *psDroid );
void	assignDestTarget( void );
void	processSensorTarget( void );
void	processDestinationTarget( void );
UDWORD	getWaterTileNum( void );
BOOL	eitherSelected(DROID *psDroid);
BOOL bRender3DOnly;
void	testEffect( void );
void	showDroidSensorRanges(void);
void	showSensorRange1(DROID *psDroid);
void	showSensorRange2(BASE_OBJECT *psObj);
void	debugToggleSensorDisplay( void );
BOOL	bSensorDisplay = FALSE;
BOOL	doWeDrawRadarBlips( void );
BOOL	doWeDrawProximitys( void );
void	drawDroidRank(DROID *psDroid);
void	drawDroidSensorLock(DROID *psDroid);
void	drawDroidCmndNo(DROID *psDroid);
static	void	addConstructionLine(DROID	*psDroid, STRUCTURE *psStructure);
static	void	doConstructionLines( void );
void	showDroidSelection( DROID *psDroid );
void	renderDefensiveStructure(STRUCTURE *psStructure);
void	drawDeliveryPointSelection(void);


//#ifdef ALEXM
//BOOL	bDrawBlips=TRUE;
//BOOL	bDrawProximitys=FALSE;
//#else
BOOL	bDrawBlips=TRUE;
BOOL	bDrawProximitys=TRUE;
//#endif
void	setBlipDraw(BOOL val);
void	setProximityDraw(BOOL val);
BOOL	godMode;
/* Info for Kev.. */
BOOL bDullInfo = FALSE;
UDWORD	texPage = 0;
/* Module variables - need to be wrapped up into specific structures*/
/* Is the scene spinning round - just for showcase stuff */
BOOL	spinScene = FALSE;
/* Initial 3D world origins */
UDWORD	mapX=45,mapY=80;
/* Have we made a selection by clicking the mouse - used for dragging etc */
BOOL	selectAttempt = FALSE;
/* Vectors that hold the player and camera directions and positions */
iView	player, camera;
/* Temporary rotation vectors to store rotations for droids etc */
iVector	imdRot,imdRot2;
/* How far away are we from the terrain */
UDWORD		distance = START_DISTANCE;//(DISTANCE - (DISTANCE/6));
/* Are we outlining the terrain tile triangles */
UDWORD		terrainOutline = FALSE;
/* Stores the screen coordinates of the transformed terrain tiles */
//PIEVERTEX		tileScreenInfo[LAND_YGRD][LAND_XGRD];

SVMESH tileScreenInfo[LAND_YGRD][LAND_XGRD];

/* Stores the tilepointers for rendered tiles */
TILE_BUCKET		tileIJ[LAND_YGRD][LAND_XGRD];
/* File size - used for any loads. Move to another specific file handling module? */
SDWORD		fileSize;
/* Stores the texture for a specific tile */
static	iTexture texturePage = {6, 64, 64, NULL};
/* Points for flipping the texture around if the tile is flipped or rotated */
POINT 	sP1,sP2,sP3,sP4;
POINT	*psP1,*psP2,*psP3,*psP4,*psPTemp;
/* Pointer to which tile the mouse is currently over */
MAPTILE	*tile3dOver = NULL;
/* Records the present X and Y values for the current mouse tile (in tiles */
SDWORD	mouseTileX,mouseTileY;
/* World coordinates that the mouse is over */
UDWORD	tile3dX,tile3dY;
/* Offsets for the screen being shrunk/expanded - how far in, how far down */
UDWORD	xOffset=CLIP_BORDER,yOffset=CLIP_BORDER;
/* Do we want the radar to be rendered */
BOOL	radarOnScreen=FALSE;
/* Are we highlighting a tile area - used for building placement */
BOOL	tileHighlight = TRUE;
/* Current cursur design */
UDWORD	cursor3D = IDC_DEFAULT;
/* Temporary values for the terrain render - top left corner of grid to be rendered */
int32 playerXTile, playerZTile, rx, rz;
/* Is gouraud shading currently enabled? */
BOOL	gouraudShading = TRUE;
/* Have we located the mouse? */
BOOL	mouseLocated = TRUE;
/* Mouse x and y - saves reading them every time we want to use them */
SDWORD	mX,mY;
/* Index so we know how to find the tile BEHIND the one currently being processed for any view angle */
UDWORD	stepIndex;
/* The box used for multiple selection - present screen coordinates */
/* The game palette */
iPalette	gamePal;
BOOL	done3dfxRadar = FALSE;
UDWORD	currentGameFrame;
UDWORD	numTiles = 0;
SDWORD	tileZ = 8000;
UDWORD	demoTextPage = 0;
BOOL	updateVideoCard = FALSE;
QUAD	dragQuad;
//BOOL	bScreenClose = FALSE;
//UDWORD	screenCloseState = SC_INACTIVE;
//UDWORD	closingTimeStart;
SDWORD	skyShift;
UDWORD	cameraHeight = 400;
//UDWORD	averageHeight;
// The maximum number of points for flattenImd
#define MAX_FLATTEN_POINTS	 255
static iVector	alteredPoints[MAX_FLATTEN_POINTS];
UBYTE	glowValue = 192;
//number of tiles visible
UDWORD	visibleXTiles;
UDWORD	visibleYTiles;
UDWORD	terrainMidX;
UDWORD	terrainMidY;
UDWORD	terrainMaxX;
UDWORD	terrainMaxY;

UDWORD	underwaterTile = WATER_TILE;
UDWORD	rubbleTile = 67;//WATER_TILE;

UDWORD geoOffset;
static	UDWORD	numTilesAveraged;
static	UDWORD	averageCentreTerrainHeight;
static	UDWORD	effectsProcessed = 0;

static	BOOL	bReloadBars = TRUE;
static	BOOL	bEnergyBars = TRUE;
static	BOOL	bTinyBars	= FALSE;
static	MAPTILE	edgeTile;
// THIS HAS GOT TO BE UPDATED EACH TIME
static char disclaimer[]="v e r s i o n g i v e n t o q a";
// THIS HAS GOT TO BE UPDATED EACH TIME

UDWORD	lastTargetAssignation = 0;
UDWORD	lastDestAssignation = 0;

BOOL	bSensorTargetting = FALSE;
BOOL	bDestTargetting = FALSE;
//DROID	*psSensorDroid = NULL;
BASE_OBJECT *psSensorObj = NULL;
UDWORD	destTargetX,destTargetY;
UDWORD	destTileX=0,destTileY=0;

#define	ONE_PERCENT		41	// 4096/100
#define	TARGET_TO_SENSOR_TIME	((4*(GAME_TICKS_PER_SEC))/5)
#define	DEST_TARGET_TIME	(GAME_TICKS_PER_SEC/4)


//this is used to 'highlight' the tiles when selecting a location for a structure
#define FOUNDATION_TEXTURE		22
#define EFFECT_DELIVERY_POINT_TRANSPARENCY		128

unsigned char	buildInfo[255];
UDWORD GetCameraDistance(void)
{
	return distance;		
}

/* Colour strobe values for the strobing drag selection box */
UBYTE	boxPulseColours[BOX_PULSE_SIZE] = {233,232,231,230,229,228,227,226,225,224};
UDWORD	lightLevel=12;
UDWORD	tCon,tIgn,tCal;

typedef struct	_defaultColours
{
UBYTE	red,green,blue,yellow,purple,white,black,cyan;
} DEF_COLOURS;

DEF_COLOURS	defaultColours;

SDWORD	pitch;

void	displayMultiChat( void )
{
UDWORD	pixelLength;
UDWORD	pixelHeight;

		pixelLength = iV_GetTextWidth(sTextToSend);
		pixelHeight = iV_GetTextLineSize();

		if(gameTime2%500<250)
		{
			iV_BoxFill(RET_X+pixelLength+3,474+E_H-(pixelHeight/4),RET_X+pixelLength+10,473+E_H,255);
		}

			/* GET RID OF THE MAGIC NUMBERS BELOW */
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_SetSwirlyBoxes(TRUE);
			iV_UniTransBoxFill(RET_X+1,474+E_H-pixelHeight,RET_X+1+pixelLength+2,473+E_H,0x000000ff,128);
			pie_SetSwirlyBoxes(FALSE);
		}
		else
		{
			iV_TransBoxFill(RET_X+1,474+E_H-pixelHeight,RET_X+1+pixelLength+2,473+E_H);
		}

		iV_DrawText(sTextToSend,RET_X+3,469+E_H);
}

// Optimisation to stop it being calculated every frame
static SDWORD	gridCentreX,gridCentreZ,gridVarCalls;
SDWORD	getCentreX( void )
{
	gridVarCalls++;
	return(gridCentreX);
}

SDWORD	getCentreZ( void )
{
	return(gridCentreZ);
}



/* Render the 3D world */
void draw3DScene( void )
{
//SDWORD	angle;
BOOL		bPlayerHasHQ = FALSE;


  	/* Set the droids on-screen display coordinates for selection later */
	mX = mouseX();
	mY = mouseY();

	// the world centre - used for decaying lighting etc
   	gridCentreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
	gridCentreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );


	/* What frame number are we on? */
	currentGameFrame = frameGetFrameNumber();


	/* Lock the surface */
	pie_GlobalRenderBegin();//only begins scene if it wasn't already begun

	pie_LocalRenderBegin();

	/* Build the drag quad */
	if(dragBox3D.status == DRAG_RELEASED)
	{
		dragQuad.coords[0].x = dragBox3D.x1; // TOP LEFT
		dragQuad.coords[0].y = dragBox3D.y1;

		dragQuad.coords[1].x = dragBox3D.x2; // TOP RIGHT
		dragQuad.coords[1].y = dragBox3D.y1;

		dragQuad.coords[2].x = dragBox3D.x2; // BOTTOM RIGHT
		dragQuad.coords[2].y = dragBox3D.y2;

		dragQuad.coords[3].x = dragBox3D.x1; // BOTTOM LEFT
		dragQuad.coords[3].y = dragBox3D.y2;
	}

   	displayTerrain();
	updateLightLevels();	
	drawDroidSelections();
	/* Show the selected delivery point */
//	drawDeliveryPointSelection(0);

	drawStructureSelections();
//	drawBuildingLines();

	bPlayerHasHQ = radarCheckForHQ(selectedPlayer);

//	if(radarOnScreen AND (bPlayerHasHQ || (bMultiPlayer && (game.type == DMATCH)) ))
	if(radarOnScreen AND bPlayerHasHQ)
	{
		pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
		pie_SetFogStatus(FALSE);
		drawRadar();
		if(doWeDrawRadarBlips())
		{
#ifndef RADAR_ROT
			drawRadarBlips();
#endif
		}
		pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
		pie_SetFogStatus(TRUE);
	}


	/* Unlock the surface */
	pie_LocalRenderEnd();

	if(!bRender3DOnly) 
	{

		if(gameStats)
		{
#ifdef DISP2D
			showGameStats();
#endif
		}

	/* Ensure that any text messages are displayed at bottom of screen */
		if (pie_Hardware())
		{
			pie_SetFogStatus(FALSE);
		}
		displayConsoleMessages();
//		if(getWarCamStatus())
//		{
//			dispWarCamLogo();
//		}
	}
//	scoreDataToScreen();
	pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_OFF);
	pie_SetFogStatus(FALSE);
	iV_SetTextColour(-1);

//	if(widgetsOn)
//	{
//		iV_DrawText("Warzone 2100 - Pumpkin Studios - QA(4) ",190,470);
//	}

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
 	/* Dont remove this folks!!!! */
	if(!bAllowOtherKeyPresses)
	{
		displayMultiChat();
	}
	else
	{
		if(!gamePaused)
		{
			iV_DrawText("Developed by Pumpkin Studios",RET_X+3,467+E_H);
			iV_DrawText("Published by EIDOS Interactive",pie_GetVideoBufferWidth()-196,467+E_H);
		}
	}

	

	/*
	if(mousePressed(MOUSE_LMB))
	{
		{
			if(apsDroidLists[0])
			{
				iVector	pos;
				UDWORD	i;
				pos.x = apsDroidLists[0]->x;
				pos.z = apsDroidLists[0]->y;
				pos.y = map_Height(pos.x,pos.z);
				addEffect(&pos,EFFECT_SAT_LASER,SAT_LASER_STANDARD,FALSE,NULL,0);
			}

		}
	}
	*/
 //----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
 	if(getDebugMappingStatus() AND !demoGetStatus() AND !gamePaused())
 	{
 		iV_DrawText("DEBUG ",RET_X+134,440+E_H);
  	}
 	else
 	{
#ifdef DEBUG
		if(!gamePaused())
		{
 			iV_DrawText(getLevelName(),RET_X+134,420+E_H);
 			getAsciiTime(buildInfo,gameTime);
 			iV_DrawText(buildInfo,RET_X+134,434+E_H);
		}
#endif
 	}

	if(pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		if(getSelectedGroup()<UBYTE_MAX)
		{
	 		sprintf(buildInfo,"%d", getSelectedGroup());
			iV_DrawText(buildInfo,mX-17,mY+4);
		}
		else if(getSelectedCommander()<UBYTE_MAX)
		{
	 		sprintf(buildInfo,"*%d", getSelectedCommander());
			iV_DrawText(buildInfo,mX-25,mY+4);
		}
	}
  	while(player.r.y>DEG(360))
	{
		player.r.y-=DEG(360);
	}
	
	/* If we don't have an active camera track, then track terrain height! */
	if(!getWarCamStatus())
	{
	/*	player.p.y = averageCentreTerrainHeight; */
	/* Move the autonomous camera if necessary */
 		trackHeight(2*averageCentreTerrainHeight);
	}
	else
	{
		processWarCam();
	}

		if(demoGetStatus())
		{
			flushConsoleMessages();
			setConsolePermanence(TRUE,TRUE);
	  		permitNewConsoleMessages(TRUE);

			addConsoleMessage("Warzone 2100 : Pumpkin Studios ",RIGHT_JUSTIFY);
	  		permitNewConsoleMessages(FALSE);
		}

  //	sprintf(buildInfo,"WallDrag from %d,%d to %d,%d", wallDrag.x1,wallDrag.y1,wallDrag.x2,wallDrag.y2);
   //	iV_DrawText(buildInfo,100,180);
	/*
 	sprintf(buildInfo,"Gridvar calls : %d", gridVarCalls);
	iV_DrawText(buildInfo,100,180);

	sprintf(buildInfo,"Instructions saved : %d", gridVarCalls*24);
	iV_DrawText(buildInfo,100,200);
		gridVarCalls = 0;
	*/

#ifdef ALEXM
      	sprintf(buildInfo,"Skipped effects : %d", getNumSkippedEffects());
	iV_DrawText(buildInfo,100,200);
	sprintf(buildInfo,"Miss Count : %d", getMissCount());
	iV_DrawText(buildInfo,100,220);
	sprintf(buildInfo,"Even effects : %d", getNumEvenEffects());
	iV_DrawText(buildInfo,100,240);
#endif

 //	sprintf(buildInfo,"Average Grid Height : %d", averageCentreTerrainHeight);
 //	iV_DrawText(buildInfo,100,240);
 //	sprintf(buildInfo,"Height : %d", player.p.y);
 //	iV_DrawText(buildInfo,100,260);



	processDemoCam();
	processSensorTarget();
	processDestinationTarget();
	
	testEffect();

	if(bSensorDisplay)
	{
		showDroidSensorRanges();
	}

}

/* Draws the 3D textured terrain */
void displayTerrain(void)

{
//	SDWORD	x,y;
	tileZ = 8000;

//	x = ((visibleXTiles/2)*128);
//	y = ((visibleYTiles/2)*128);

//	x += player.p.x;
//	y += player.p.z;
	camera.p.z = distance;
	camera.p.y = 0;
	camera.p.x = 0;

	/* SetUpClipping window - to below the backdrop */
	if (pie_Hardware())
	{
  		pie_Set2DClip(xOffset,yOffset,psRendSurface->width-xOffset,psRendSurface->height-yOffset);
	}
	else
	{
  		pie_Set2DClip(xOffset,yOffset,psRendSurface->width-xOffset-1,psRendSurface->height-yOffset-1);
	}

	/* Render the sky here */
//	if(!bScreenClose)
//	{
//		renderSky();
//	}



	/* Set 3D world origins */
	pie_SetGeometricOffset((iV_SCREEN_WIDTH>>1),geoOffset);

	/* We haven't yet located which tile mouse is over */
	mouseLocated = FALSE;

	numTiles = 0;

	/* Setup tiles */
	preprocessTiles();

	/* Now, draw the terrain */
	drawTiles(&camera,&player);

	/* Show the drag Box if necessary */
	drawDragBox();

	/* Have we released the drag box? */
	if(dragBox3D.status == DRAG_RELEASED)
	{
		dragBox3D.status = DRAG_INACTIVE;
	}
}


// Parameter is the vector to store the camera position
void CalcBSPCameraPos(iVector *NewBSPCamera)
{
		OBJPOS Camera;

		/* The 128's should really be TILE_UNITS - sort this out later ! */
		Camera.x = player.p.x+ (terrainMidX*128);
		Camera.y = player.p.y;
		Camera.z = player.p.z+ (terrainMidY*128);

		Camera.pitch = (SWORD)player.r.x;
		Camera.yaw = (SWORD)player.r.y;

		GetRealCameraPos(&Camera,GetCameraDistance(),NewBSPCamera);
}

/***************************************************************************/
BOOL	doWeDrawRadarBlips( void )
{
	return(bDrawBlips);
}
/***************************************************************************/
BOOL	doWeDrawProximitys( void )
{
	return(bDrawProximitys);
}
/***************************************************************************/
void	setBlipDraw(BOOL val)
{
	bDrawBlips = val;
}
/***************************************************************************/
void	setProximityDraw(BOOL val)
{
	bDrawProximitys = val;
}
/***************************************************************************/


void drawTiles(iView *camera, iView *player)
{
	SDWORD	i,j;
	iVector tileXYZ;
	SDWORD	zMax;
	iVector BSPCamera;
	MAPTILE	*psTile; 	
	UDWORD	specular;
	UDWORD	tilesRejected;
	UDWORD	edgeX,edgeY;
	BOOL IsWaterTile;
	BOOL PushedDown;
	UBYTE TileIllum;
	UWORD TextNum;
	UDWORD	shiftVal;
	UDWORD	altVal;
	FRACT	fraction;
	UDWORD	realX,realY;
	BOOL	bEdgeTile;

	// Animate the water texture, just cycles the V coordinate through half the tiles height.

   	
	if(!gamePaused())
	{
		fraction = MAKEFRACT(frameTime2)/GAME_TICKS_PER_SEC;
   		waterRealValue += (fraction*WAVE_SPEED);
   		vOffset = (SWORD)MAKEINT(waterRealValue);
   		if(vOffset >= 64/2) 
   		{
   			vOffset = 0;											
   			waterRealValue = (FRACT)0;
   		}
	}
	/* Is the scene spinning? - showcase demo stuff */
	if (spinScene)
		player->r.y+=DEG(3);

	/* ---------------------------------------------------------------- */
	/* Do boundary and extent checking                                  */
	/* ---------------------------------------------------------------- */
	/* Get the mid point of the grid */
	terrainMidX = (visibleXTiles>>1);
	terrainMidY = (visibleYTiles>>1);

	CalcBSPCameraPos(&BSPCamera);
	SetBSPCameraPos(BSPCamera.x,BSPCamera.y,BSPCamera.z);

	/* Find our position in tile coordinates */
	playerXTile = player->p.x >> TILE_SHIFT;
	playerZTile = player->p.z >> TILE_SHIFT;

  	/* Get the x,z translation components */
 	rx = (player->p.x) & (TILE_UNITS-1);
 	rz = (player->p.z) & (TILE_UNITS-1);


	/* ---------------------------------------------------------------- */
	/* Set up the geometry                                              */
	/* ---------------------------------------------------------------- */

	/* ---------------------------------------------------------------- */
	/* Push identity matrix onto stack */
	pie_MatBegin();
	// Now, scale the world according to what resolution we're running in
	scaleMatrix(pie_GetResScalingFactor());
	// Now, scale the world according to what resolution we're running in
	/* Set the camera position */
	pie_MATTRANS(camera->p.x,camera->p.y,camera->p.z);
	/* Rotate for the player */
	pie_MatRotZ(player->r.z);
	pie_MatRotX(player->r.x);
	pie_MatRotY(player->r.y);
	  /* Translate */
	pie_TRANSLATE(-rx,-player->p.y,rz);
	
	/* ---------------------------------------------------------------- */
	/* Rotate and project all the tiles within the grid                 */
	/* ---------------------------------------------------------------- */
	/*	We track the height here - so make sure we get the average heights
		of the tiles in the grid
	*/
	averageCentreTerrainHeight = 0;
	numTilesAveraged = 0;
	for (i=0; i<(SDWORD)visibleYTiles+1; i++) 
	{
		/* Go through the x's */
		for (j=0; j<(SDWORD)visibleXTiles+1; j++) 
		{
			if(pie_Hardware())
			{
				tileScreenInfo[i][j].bWater = FALSE;
			}
			if( (playerXTile+j < 0) OR 
				(playerZTile+i < 0)	OR
				(playerXTile+j > (SDWORD)(mapWidth-1)) OR
				(playerZTile+i > (SDWORD)(mapHeight-1)) )
			{
				edgeX = playerXTile+j;
				edgeY = playerZTile+i;
				if(playerXTile+j < 0 ) edgeX = 0;
				else
				if(playerXTile+j > (SDWORD)(mapWidth-1) ) edgeX = mapWidth-1;
				if(playerZTile+i < 0 ) edgeY = 0;
				else
				if(playerZTile+i > (SDWORD)(mapHeight-1) ) edgeY = mapHeight-1;

				tileXYZ.x = ((j-terrainMidX)<<TILE_SHIFT);
				tileXYZ.y = 0;//map_TileHeight(edgeX,edgeY);
				tileXYZ.z = ((terrainMidY-i)<<TILE_SHIFT);
  				tileScreenInfo[i][j].sz = pie_RotProj(&tileXYZ,(iPoint *)&tileScreenInfo[i][j].sx);
			  
			   	if (pie_GetFogEnabled())
			  	{
			  		if(pie_Hardware())
			  		{
			  			tileScreenInfo[i][j].light.argb = 0xff030303;
			  			tileScreenInfo[i][j].specular.argb = pie_GetFogColour();
			  		}
			  		else
					{
			  			tileScreenInfo[i][j].light.argb = 0x00000003;
			  			tileScreenInfo[i][j].specular.argb = 0;
			  		}
			  	}
			  	else
			  	{
			  		tileScreenInfo[i][j].light.argb = lightDoFogAndIllumination(mapTile(edgeX,edgeY)->illumination,rx-tileXYZ.x,rz - ((i-terrainMidY)<<TILE_SHIFT),&specular);
			  	}

					
				if( (playerXTile+j <-1 ) OR 
				(playerZTile+i <-1)	OR
				(playerXTile+j > (SDWORD)(mapWidth-1)) OR
				(playerZTile+i > (SDWORD)(mapHeight-1)) )
				{
					tileScreenInfo[i][j].drawInfo = FALSE;
				}
				else
				{
					tileScreenInfo[i][j].drawInfo = TRUE;
				}


			}
			else
			{
				tileScreenInfo[i][j].drawInfo = TRUE;
				
				psTile = mapTile(playerXTile+j,playerZTile+i);
				/* Get a pointer to the tile at this location */
				tileXYZ.x = ((j-terrainMidX)<<TILE_SHIFT);
				if(pie_Hardware())
				{
					if(TERRAIN_TYPE(psTile)==TER_WATER)
					{
						tileScreenInfo[i][j].bWater = TRUE;
					}
				}
				tileXYZ.y = map_TileHeight(playerXTile+j, playerZTile+i);
				tileXYZ.z = ((terrainMidY-i)<<TILE_SHIFT);

				/* Is it in the centre and therefore worth averaging height over? */
				if(i > MIN_TILE_Y AND i < MAX_TILE_Y AND j > MIN_TILE_X AND j < MAX_TILE_X)
				{
					averageCentreTerrainHeight += tileXYZ.y;
					numTilesAveraged++;
				}
				realX = playerXTile+j;
				realY = playerZTile+i;
				bEdgeTile = FALSE;
				if(realX<=1 OR realY<=1 OR realX >= mapWidth-2 OR realY >= mapHeight-2)
				{
					bEdgeTile = TRUE;
				}
				
				if(getRevealStatus())
				{
					if(godMode)
					{
						TileIllum = psTile->illumination;
					}
					else
					{
						TileIllum = (psTile->level == UBYTE_MAX ? 1 : psTile->level);//avGetTileLevel(realX,realY);
					}
				}
				else
				{
						TileIllum = psTile->illumination;
				}
			  
				
				if(bDisplaySensorRange)
				{
					TileIllum = psTile->inRange;
				}

#ifdef ENABLE_WATER
				if (pie_Hardware())
				{
					TextNum = (UWORD)(psTile->texture & TILE_NUMMASK);
					IsWaterTile = (TERRAIN_TYPE(psTile) == TER_WATER);
					// If it's the main water tile then..
					PushedDown = FALSE;
					if( TextNum == WaterTileID AND !bEdgeTile) 
					{
						// Push the terrain down for the river bed.
						PushedDown = TRUE;
					 	shiftVal = WATER_DEPTH + ((3*environGetData(playerXTile+j,playerZTile+i))/2);
						altVal = 0;//environGetValue(playerXTile+j,playerZTile+i);
						tileXYZ.y -= (shiftVal+altVal);
						// And darken it.
						TileIllum = (UBYTE)((TileIllum*3)/4);
					}
				}
#endif
				tileScreenInfo[i][j].sz = pie_RotProj(&tileXYZ,(iPoint *)&tileScreenInfo[i][j].sx);

				tileScreenInfo[i][j].light.argb = lightDoFogAndIllumination(TileIllum,rx-tileXYZ.x,rz - ((i-terrainMidY)<<TILE_SHIFT),&specular);

				if (pie_GetRenderEngine() == ENGINE_D3D)
			 	{
					tileScreenInfo[i][j].specular.argb = specular;
		   		}

#ifdef ENABLE_WATER
				if (pie_Hardware())
				{
					// If it's any water tile..
					if(IsWaterTile) {
						// If it's the main water tile then bring it back up because it was pushed down
						// for the river bed calc.
						if(PushedDown) { //TextNum == WaterTileID) {
							tileXYZ.y += (shiftVal + (2*altVal));
						}

						// Transform it into the wx,wy mesh members.
						tileScreenInfo[i][j].wz = pie_RotProj(&tileXYZ,(iPoint *)&tileScreenInfo[i][j].wx);
						tileScreenInfo[i][j].wlight.argb = lightDoFogAndIllumination(
							TileIllum, rx-tileXYZ.x,		  // cos altval can go to 20
							rz - ((i-terrainMidY)<<TILE_SHIFT), &specular);
					} else {
						// If it was'nt a water tile then need to ensure wx,wy are valid because
						// a water tile might be sharing verticies with it.
						tileScreenInfo[i][j].wx = tileScreenInfo[i][j].sx;
						tileScreenInfo[i][j].wy = tileScreenInfo[i][j].sy;
						tileScreenInfo[i][j].wz = tileScreenInfo[i][j].sz;
					}
				}
#endif
			}
		}
	}

	/* Work out the average height */
	if(numTilesAveraged)	// might not be if off map
	{
		averageCentreTerrainHeight/=numTilesAveraged;
	}
	else
	{
		averageCentreTerrainHeight = 128 * ELEVATION_SCALE;
	}
	/* This is done here as effects can light the terrain - pause mode problems though */
	processEffects();
	atmosUpdateSystem();
	if(waterOnMap())
	{
//		environUpdate();
	}
	if(getRevealStatus())
	{
		avUpdateTiles();
	}


//	doBuildingLights();
	/* ---------------------------------------------------------------- */
	/* Draw all the tiles or add them to bucket sort                     */
	/* ---------------------------------------------------------------- */
	tilesRejected = 0;
	for (i= 0; i < (SDWORD)visibleYTiles; i++) 

	{	
		for (j= 0; j < (SDWORD)visibleXTiles; j++) 
		{
#ifndef BUCKET
//bucket?
		drawTexturedTile(i,j);
#else

		if(tileScreenInfo[i][j].drawInfo == TRUE)
			{

				tileIJ[i][j].i = i;
				tileIJ[i][j].j = j;
				//get distance of furthest corner
				zMax = pie_MAX(tileScreenInfo[i][j].sz, tileScreenInfo[i + 1][j].sz);
				zMax = pie_MAX(zMax, tileScreenInfo[i + 1][j + 1].sz);
				zMax = pie_MAX(zMax, tileScreenInfo[i][j + 1].sz);
				tileIJ[i][j].depth = zMax;
				if((UDWORD)i>mapHeight OR (UDWORD)j>mapWidth)
				{
					ASSERT((FALSE,"Weirdy tile coords"));
				}
				bucketAddTypeToList(RENDER_TILE, &tileIJ[i][j]);
				if (pie_Hardware()) {
					bucketAddTypeToList(RENDER_WATERTILE, &tileIJ[i][j]);

				}
			}
			else
			{
				tilesRejected++;
			}
#endif //bucket
		}
	}

	targetOpenList((BASE_OBJECT*)driveGetDriven());

	/* ---------------------------------------------------------------- */
	/* Now display all the static objects                               */
	/* ---------------------------------------------------------------- */
	displayStaticObjects();//bucket render implemented
	displayFeatures();//bucket render implemented
	displayDynamicObjects();//bucket render implemented
	if(doWeDrawProximitys())
	{
		displayProximityMsgs(); //bucket render implemented
	}
	displayDelivPoints();	//bucket render implemented
	display3DProjectiles();//bucket render implemented

	drawEffects();
	atmosDrawParticles();
#ifdef BUCKET
	bucketRenderCurrentList();
#endif
#ifdef ARROWS
	arrowDrawAll();
#endif

	targetCloseList();

	if(driveModeActive()) {
		// If were in driving mode then mark the current target.
		BASE_OBJECT *psObj = targetGetCurrent();
		if(psObj != NULL) {
			targetMarkCurrent();
		}
	}
	if(!gamePaused())
	{
		doConstructionLines();
	}

	/* Clear the matrix stack */
	iV_MatrixEnd();
	locateMouse();
}


BOOL	init3DView(void)
{
//	barMode = BAR_FULL;		now from registry.

	// the world centre - used for decaying lighting etc
	gridCentreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
	gridCentreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );


	pie_SetGammaValue(gamma);

	edgeTile.texture = 0;

	bEnergyBars = TRUE;

	/* Base Level */
	geoOffset = 192;

	//set up how many tiles to draw
	visibleXTiles = VISIBLE_XTILES;
	visibleYTiles = VISIBLE_YTILES;

	/* There are no drag boxes */
	dragBox3D.status = DRAG_INACTIVE;

	/* Arbitrary choice - from direct read! */
	theSun.x = 0;
	theSun.y = -3441;
	theSun.z = 2619;

	/* Make sure and change these to comply with map.c */
	imdRot.x = -35;
	/* Maximum map size */
	terrainMaxX = 128;
	terrainMaxY = 128;
//	terrainSizeX = distance/100;
//	terrainSizeY = distance/100;
//	terrainMidX = (terrainSizeX>>1);
//	terrainMidY = terrainSizeY>>1;
   
	/* Get all the init stuff out of here? */
	initWarCam();

	/* Init the game messaging system */
	initConsoleMessages();

	/* Initialise the effects system */
  //	initEffectsSystem();
	atmosInitSystem();

	initDemoCamera();

	/* HACK -  remove, although function of some form still necessary */
//	initSmoke();

	/* Set up the sine table for the bullets */
	initBulletTable();

	/* Set up light values */
   //	initLighting();

	/* Build our shade table for gouraud shading - 256*16 values with best match from 256 colour table */
	iV_PaletteShadeTableCreate();
	getDefaultColours();

	/* No initial rotations */
	imdRot2.x = 0;
	imdRot.y = 0;
	imdRot2.z = 0;

	/* Set up the player */
/*	player.p.y = 0;
	player.p.x = mapWidth/2*TILE_UNITS;
	player.p.z = mapHeight/2*TILE_UNITS;

	setViewAngle(-30);
	player.r.y = DEG(-45); */

	bRender3DOnly = FALSE;

	targetInitialise();

	/* Set up the fog tbale for the 3dfx */
//	pie_SetFogTable(0x00000000,65536,65536);
	return(TRUE);
	CONPRINTF(ConsoleString,(ConsoleString, "This build : %s, %s",__TIME__,__DATE__));

	demoProcessTilesIn();
}


// set the view position from save game
void disp3d_setView(iView *newView)
{
	memcpy(&player,newView,sizeof(iView));	
}
 
// get the view position for save game
void disp3d_getView(iView *newView)
{
	memcpy(newView,&player,sizeof(iView));	
}

/* John's routine - deals with flipping around the vertex ordering for source textures
   when flips and rotations are being done */
void	flipsAndRots(int texture)
{

/* Store the source rect as four points */
	sP1.x = 1;
	sP1.y = 1;
	sP2.x = 63;
	sP2.y = 1;
	sP3.x = 63;
	sP3.y = 63;
	sP4.x = 1;
	sP4.y = 63;

	/* Store pointers to the points */
	psP1 = &sP1;
	psP2 = &sP2;
	psP3 = &sP3;
	psP4 = &sP4;
	
	
	if (texture & TILE_XFLIP)
	{
		psPTemp = psP1;
		psP1 = psP2;
		psP2 = psPTemp;
		psPTemp = psP3;
		psP3 = psP4;
		psP4 = psPTemp;
	}
	if (texture & TILE_YFLIP)
	{
 		psPTemp = psP1;
		psP1 = psP4;
		psP4 = psPTemp;
		psPTemp = psP2;
		psP2 = psP3;
		psP3 = psPTemp;
	}
	
	switch ((texture & TILE_ROTMASK) >> TILE_ROTSHIFT)
	{
	case 1:
 		psPTemp = psP1;
		psP1 = psP4;
		psP4 = psP3;
		psP3 = psP2;
		psP2 = psPTemp;
		break;
	case 2:
		psPTemp = psP1;
		psP1 = psP3;
		psP3 = psPTemp;
		psPTemp = psP4;
		psP4 = psP2;
		psP2 = psPTemp;
		break;
	case 3:
		psPTemp = psP1;
		psP1 = psP2;
		psP2 = psP3;
		psP3 = psP4;
		psP4 = psPTemp;
		break;
	}
}




/* Establishes whether it's worth trying to render a droid - is it actually on the grid? */
BOOL clipDroid(DROID *psDroid)
{
	if (psDroid->x>=(UDWORD)player.p.x AND psDroid->x<(UDWORD)player.p.x+(visibleXTiles*TILE_UNITS) AND
		psDroid->y>=(UDWORD)player.p.z AND psDroid->y<(UDWORD)player.p.z+(visibleYTiles*TILE_UNITS))
		return(TRUE);
	else
		return(FALSE);
}

/* Clips anything - not necessarily a droid */
BOOL clipXY(SDWORD x, SDWORD y)
{
	if (x > (SDWORD)player.p.x AND  x < (SDWORD)(player.p.x+(visibleXTiles*
		TILE_UNITS)) AND
		y > (SDWORD)player.p.z AND y < (SDWORD)(player.p.z+(visibleYTiles*TILE_UNITS)))
		return(TRUE);
	else
		return(FALSE);
}


/*	Get the onscreen corrdinates of a Object Position so we can draw a 'button' in 
the Intelligence screen.  VERY similar to above function*/
void	calcFlagPosScreenCoords(SDWORD *pX, SDWORD *pY, SDWORD *pR)
{
	SDWORD	centX,centY,centZ;
	SDWORD	cX,cY;
	UDWORD	radius;

	/* Ensure correct context */
	pie_SETUP_ROTATE_PROJECT;

	/* Get it's absolute dimensions */
	centX = centY = centZ = 0;
	/* How big a box do we want - will ultimately be calculated using xmax, ymax, zmax etc */
	radius = 22;

	/* Pop matrices and get the screen coordinates for last point*/
	pie_ROTATE_PROJECT(centX,centY,centZ,cX,cY);

	/*store the coords*/
	*pX = cX;
	*pY = cY;
	*pR = radius;
}


/* Renders the bullets and their effects in 3D */
void display3DProjectiles( void )
{
	PROJ_OBJECT		*psObj;

	psObj = proj_GetFirst();

	while ( psObj != NULL )
	{
		switch(psObj->state)
		{
		case PROJ_INFLIGHT:			
			// if source or destination is visible
//			if(   ((psObj->psSource != NULL) && psObj->psSource->visible[selectedPlayer])
//			   || ((psObj->psDest != NULL)   && psObj->psDest->visible[selectedPlayer]  )  )
  			if(gfxVisible(psObj))

//			if(GFX_VISIBLE(psObj))
			{

				/* don't display first frame of trajectory (projectile on firing object) */
				if ( gameTime != psObj->born )
				{
					/* Draw a bullet at psObj->x for X coord
										psObj->y for Z coord
										whatever for Y (height) coord - arcing ? 
					*/
	#ifndef BUCKET
					renderProjectile(psObj);
	#else
					/* these guys get drawn last */
					if(psObj->psWStats->weaponSubClass == WSC_ROCKET OR 
						psObj->psWStats->weaponSubClass == WSC_MISSILE OR
						psObj->psWStats->weaponSubClass == WSC_COMMAND OR
						psObj->psWStats->weaponSubClass == WSC_SLOWMISSILE OR
						psObj->psWStats->weaponSubClass == WSC_SLOWROCKET OR
						psObj->psWStats->weaponSubClass == WSC_ENERGY OR
                        psObj->psWStats->weaponSubClass == WSC_EMP)
					{
						bucketAddTypeToList(RENDER_PROJECTILE_TRANSPARENT, psObj);
					}
					else
					{
						bucketAddTypeToList(RENDER_PROJECTILE, psObj);
					}
	#endif
				}
			}
			break;
			
		case PROJ_IMPACT:
			break;

		case PROJ_POSTIMPACT:
			break;

		default:
			break;
		}	/* end switch */
		psObj = proj_GetNext();
	}
}	/* end of function display3DProjectiles */


void	renderProjectile(PROJ_OBJECT *psCurr)
{
	WEAPON_STATS	*psStats;
	iVector			dv;
	iIMDShape		*pIMD;
	UDWORD			brightness, specular;
//	SDWORD		centreX, centreZ;
	
	psStats = psCurr->psWStats;
	/* Reject flame or command since they have interim drawn fx */
	if(psStats->weaponSubClass == WSC_FLAME OR 
        psStats->weaponSubClass == WSC_COMMAND OR // OR psStats->weaponSubClass == WSC_ENERGY)
		psStats->weaponSubClass == WSC_ELECTRONIC OR
        psStats->weaponSubClass == WSC_EMP OR
        (bMultiPlayer AND psStats->weaponSubClass == WSC_LAS_SAT))
//		OR psStats->weaponSubClass == WSC_ROCKET)
	{
		/* We don't do projectiles from these guys, cos there's an effect instead */
		return;
	}


	//the weapon stats holds the reference to which graphic to use
	/*Need to draw the graphic depending on what the projectile is doing - hitting target, 
	missing target, in flight etc - JUST DO IN FLIGHT FOR NOW! */
	pIMD = psStats->pInFlightGraphic;

   	if (clipXY(psCurr->x,psCurr->y))
	{
		/* Get bullet's x coord */
	   	dv.x = (psCurr->x - player.p.x) - terrainMidX*TILE_UNITS;

		/* Get it's y coord (z coord in the 3d world */
	 	dv.z = terrainMidY*TILE_UNITS - (psCurr->y - player.p.z);

		/* What's the present height of the bullet? */
		dv.y = psCurr->z;
		/* Set up the matrix */
		iV_MatrixBegin();

		/* Translate to the correct position */
   		iV_TRANSLATE(dv.x,dv.y,dv.z);
		/* Get the x,z translation components */
   		rx = player.p.x & (TILE_UNITS-1);
		rz = player.p.z & (TILE_UNITS-1);
 
		/* Translate */
		iV_TRANSLATE(rx,0,-rz);

		/* Rotate it to the direction it's facing */
		imdRot2.y = DEG(psCurr->direction);
		iV_MatrixRotateY(-imdRot2.y);

	   	/* pitch it */
   		imdRot2.x = DEG(psCurr->pitch);
	   	iV_MatrixRotateX(imdRot2.x);

		/* Spin the bullet around - remove later */
//		centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//		centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

		brightness = (UDWORD)lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX()-psCurr->x,getCentreZ()-psCurr->y, &specular);
	  	if(psStats->weaponSubClass == WSC_ROCKET OR psStats->weaponSubClass == WSC_MISSILE OR
	  	psStats->weaponSubClass == WSC_SLOWROCKET OR psStats->weaponSubClass == WSC_SLOWMISSILE)
		{
	   	  	pie_Draw3DShape(pIMD, 0, 0, brightness, 0, pie_ADDITIVE, 164);

//			pie_Draw3DShape(pIMD, 0, 0, brightness, specular, pie_NO_BILINEAR, 0);
		}
		else
		{
			pie_Draw3DShape(pIMD, 0, 0, brightness, specular, pie_NO_BILINEAR, 0);
		}

		iV_MatrixEnd();
	}
  	/* Flush matrices */
}

void
renderAnimComponent( COMPONENT_OBJECT *psObj )
{
	iVector		dv;
	SDWORD		posX, posY, posZ, iPlayer;
	BASE_OBJECT	*psParentObj = (BASE_OBJECT *) psObj->psParent;
	DROID		*psDroid;
	STRUCTURE	*psStructure;
	UDWORD		brightness, specular;
//	SDWORD		centreX, centreZ;

	ASSERT( (PTRVALID(psParentObj, sizeof(SIMPLE_OBJECT)),
		"renderAnimComponent: invalid parent object pointer") );

	/* only draw visible bits */
	if( (psParentObj->type == OBJ_DROID) AND !godMode AND !demoGetStatus())
	{
		if( ((DROID*)psParentObj)->visible[selectedPlayer] != UBYTE_MAX)
		{
			return;
		}
	}

	posX = psParentObj->x + psObj->position.x;
	posY = psParentObj->y + psObj->position.y;
	posZ = psParentObj->z + psObj->position.z;

//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

	/* render */
	if( clipXY( posX, posY ) )
	{
		psParentObj->sDisplay.frameNumber = currentGameFrame;
		/* Push the indentity matrix */
		iV_MatrixBegin();

		/* get parent object translation */
		dv.x = (psParentObj->x - player.p.x) - terrainMidX*TILE_UNITS;
		dv.z = terrainMidY*TILE_UNITS - (psParentObj->y - player.p.z);
		dv.y = psParentObj->z;
		
		/* parent object translation */
		iV_TRANSLATE(dv.x,dv.y,dv.z);

		/* Get the x,z translation components */
		rx = player.p.x & (TILE_UNITS-1);
		rz = player.p.z & (TILE_UNITS-1);

		/* Translate */
		iV_TRANSLATE(rx,0,-rz);

		/* parent object rotations */
		imdRot2.y = DEG(psParentObj->direction);
		iV_MatrixRotateY(-imdRot2.y);
   		imdRot2.x = DEG(psParentObj->pitch);
	   	iV_MatrixRotateX(imdRot2.x);

		/* object (animation) translations - ivis z and y flipped */
		iV_TRANSLATE( psObj->position.x, psObj->position.z, psObj->position.y );

		/* object (animation) rotations */
		iV_MatrixRotateY( -psObj->orientation.z );
		iV_MatrixRotateZ( -psObj->orientation.y );
		iV_MatrixRotateX( -psObj->orientation.x );

		/* Set frame numbers - look into this later?? FIXME!!!!!!!! */
		if( psParentObj->type == OBJ_DROID )
		{
			psDroid = (DROID *) psParentObj;
			if ( psDroid->droidType == DROID_PERSON )
			{
				iPlayer = psParentObj->player-6;
				scaleMatrix(75);
			}
			else
			{
				iPlayer = getPlayerColour(psParentObj->player);
			}

			/* Get the onscreen coordinates so we can draw a bounding box */
			calcScreenCoords( psDroid );
			targetAdd((BASE_OBJECT*)psDroid);
		}
		else
		{
			iPlayer = getPlayerColour(psParentObj->player);
		}

		//brightness and fog calculation
		if (psParentObj->type == OBJ_STRUCTURE)
		{
			psStructure = (STRUCTURE*)psParentObj;
			brightness = 200 - (100-PERCENT( psStructure->body , 
			//		psStructure->baseBodyPoints ));
					structureBody(psStructure)));
			{
				SDWORD sX,sY;
				pie_SETUP_ROTATE_PROJECT;
				pie_ROTATE_PROJECT(0,0,0,sX,sY);
				psStructure->sDisplay.screenX = sX;
				psStructure->sDisplay.screenY = sY;
			}
			targetAdd((BASE_OBJECT*)psStructure);
		}
		else
		{
			brightness = pie_MAX_BRIGHT_LEVEL;
		}

		if(getRevealStatus() && !godMode)
		{
			brightness = avGetObjLightLevel((BASE_OBJECT*)psParentObj,brightness);
		}
		brightness = (UDWORD)lightDoFogAndIllumination((UBYTE)brightness,getCentreX()-posX,getCentreZ()-posY, &specular);

		pie_Draw3DShape(psObj->psShape, 0, iPlayer, brightness, specular, pie_NO_BILINEAR, 0);

		/* clear stack */
		iV_MatrixEnd();
	}
}

/*	Renders ONE terrain tile and any droids, structures, features that are on it. Sorts the objects though, so's there
	only drawn once and drawn at the appropriate time so tles aren't drawn over them */
void	drawTexturedTile(UDWORD	i, UDWORD j)
{
UDWORD	tileNumber;
UDWORD	renderFlag;
//UDWORD	n;
iVertex p[4];
//iVertex clip[iV_POLY_MAX_POINTS];
MAPTILE	*psTile;
BOOL	tileOutlined = FALSE;
UDWORD	realX,realY;
UDWORD		topL,botL,topR,botR;
iPoint	offset;
BOOL	bEdgeTile;

	bEdgeTile = FALSE;
	/* Get the actual tile to render */
	realX = playerXTile+j;
	realY = playerZTile+i;

	/* Get a pointer to the tile we're going to render */
	if( (realX<0) OR
		(realY<0) OR
		(realX>mapWidth-2) OR
		(realY>mapHeight-2) )
	{
		psTile = &edgeTile;
		bEdgeTile = TRUE;
		CLEAR_TILE_HIGHLIGHT(psTile);
	}
	else
	{
		psTile = mapTile(realX,realY);
	}

	if(!TILE_DRAW(psTile))
	{
		/* Bomb out if we're not supposed to draw this tile! */
		return;
	}

// tiles are always visible for now - john.
//	if ( TEST_TILE_VISIBLE(selectedPlayer, psTile) OR godMode)
		{

			numTiles++;

			if(bEdgeTile)
			{
				topL = botL = botR = topR = 0;
			}
			else
			{
				//penumbra
				topL = tileScreenInfo[i+0][j+0].light.argb;
				botL = tileScreenInfo[i+1][j+0].light.argb;
				botR = tileScreenInfo[i+1][j+1].light.argb;
				topR = tileScreenInfo[i+0][j+1].light.argb;
				//no penumbra
				/*
				topL = mapTile(realX,realY)->illumination;
				botL = mapTile(realX,realY+1)->illumination;
				botR = mapTile(realX+1,realY+1)->illumination;
				topR = mapTile(realX+1,realY)->illumination;
				*/
			}
		 		/* get the appropriate tile texture */
			if(TILE_HIGHLIGHT(psTile))
			{
				CLEAR_TILE_HIGHLIGHT(psTile);
				//tileNumber = psTile->texture; 
				tileNumber = FOUNDATION_TEXTURE;
				tileOutlined = TRUE;
			}
			else
			{
				tileNumber = psTile->texture; 
			}
			if (pie_Hardware())
			{
				pie_SetTexturePage(tileTexInfo[tileNumber & TILE_NUMMASK].texPage);
			}
			else
			{
				texturePage.bmp = tilesRAW[tileNumber & TILE_NUMMASK];
			}

//temp
//			pie_DrawTile(&tileScreenInfo[0][0],&tileScreenInfo[0][1],&tileScreenInfo[1][0],
//				&tileScreenInfo[1][1],tileTexInfo[tileNumber & 0xff].texPage);
//temp
		
			

			/* Check for flipped and rotated tiles */
   			flipsAndRots(tileNumber & ~TILE_NUMMASK);

			if(TRI_FLIPPED(psTile))
			{
			 	/* Get the screen coordinates to render into for the texturer */
 		   		p[0].x = tileScreenInfo[i+0][j+0].sx; p[0].y = tileScreenInfo[i+0][j+0].sy; p[0].z = tileScreenInfo[i+0][j+0].sz;
  		   		p[1].x = tileScreenInfo[i+0][j+1].sx; p[1].y = tileScreenInfo[i+0][j+1].sy; p[1].z = tileScreenInfo[i+0][j+1].sz;
  		   		p[2].x = tileScreenInfo[i+1][j+0].sx; p[2].y = tileScreenInfo[i+1][j+0].sy; p[2].z = tileScreenInfo[i+1][j+0].sz;
		   
				/* Get the U,V values for the indexing into the texture */
				p[0].u = psP1->x; p[0].v=psP1->y;
   				p[1].u = psP2->x; p[1].v=psP2->y;
   				p[2].u = psP4->x; p[2].v=psP4->y;

				/* Get the intensity values	for shading */
		 	   	p[0].g = (UBYTE)topL;
		 		p[1].g = (UBYTE)topR;
				p[2].g = (UBYTE)botL;
			}
			else
			{
				/* Get the screen coordinates to render into for the texturer */
 		   		p[0].x = tileScreenInfo[i+0][j+0].sx; p[0].y = tileScreenInfo[i+0][j+0].sy; p[0].z = tileScreenInfo[i+0][j+0].sz;
  		   		p[1].x = tileScreenInfo[i+0][j+1].sx; p[1].y = tileScreenInfo[i+0][j+1].sy; p[1].z = tileScreenInfo[i+0][j+1].sz;
  		   		p[2].x = tileScreenInfo[i+1][j+1].sx; p[2].y = tileScreenInfo[i+1][j+1].sy; p[2].z = tileScreenInfo[i+1][j+1].sz;
		   
				/* Get the U,V values for the indexing into the texture */
				p[0].u = psP1->x; p[0].v=psP1->y;
   				p[1].u = psP2->x; p[1].v=psP2->y;
   				p[2].u = psP3->x; p[2].v=psP3->y;

				/* Get the intensity values	for shading */
		 		p[0].g = (UBYTE)topL;
		 		p[1].g = (UBYTE)topR;
				p[2].g = (UBYTE)botR;

			}

			renderFlag = 0;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			offset.x = (tileTexInfo[tileNumber & TILE_NUMMASK].xOffset * 64); 
			offset.y = (tileTexInfo[tileNumber & TILE_NUMMASK].yOffset * 64); 
		}
			pie_DrawTriangle(p, &texturePage, renderFlag, &offset);	
		 
			if(TRI_FLIPPED(psTile))
			{
				/* Set up the texel coordinates */
   				p[0].x = tileScreenInfo[i+0][j+1].sx; p[0].y = tileScreenInfo[i+0][j+1].sy; p[0].z = tileScreenInfo[i+0][j+1].sz;
   				p[1].x = tileScreenInfo[i+1][j+1].sx; p[1].y = tileScreenInfo[i+1][j+1].sy; p[1].z = tileScreenInfo[i+1][j+1].sz;
   				p[2].x = tileScreenInfo[i+1][j+0].sx; p[2].y = tileScreenInfo[i+1][j+0].sy; p[2].z = tileScreenInfo[i+1][j+0].sz;
		
				/* Set up U,V */
				p[0].u = psP2->x; p[0].v=psP2->y;
   				p[1].u = psP3->x; p[1].v=psP3->y;
   				p[2].u = psP4->x; p[2].v=psP4->y;

				/* Set up shading vars */
   				p[0].g = (UBYTE)topR;
				p[1].g = (UBYTE)botR;
		  		p[2].g = (UBYTE)botL;
			}
			else
			{
				/* Set up the texel coordinates */
				p[0].x = tileScreenInfo[i+0][j+0].sx; p[0].y = tileScreenInfo[i+0][j+0].sy; p[0].z = tileScreenInfo[i+0][j+0].sz;
   				p[1].x = tileScreenInfo[i+1][j+1].sx; p[1].y = tileScreenInfo[i+1][j+1].sy; p[1].z = tileScreenInfo[i+1][j+1].sz;
   				p[2].x = tileScreenInfo[i+1][j+0].sx; p[2].y = tileScreenInfo[i+1][j+0].sy; p[2].z = tileScreenInfo[i+1][j+0].sz;
		
				/* Set up U,V */
				p[0].u = psP1->x; p[0].v=psP1->y;
   				p[1].u = psP3->x; p[1].v=psP3->y;
   				p[2].u = psP4->x; p[2].v=psP4->y;

				/* Set up shading vars */
   				p[0].g = (UBYTE)topL;
				p[1].g = (UBYTE)botR;
		  		p[2].g = (UBYTE)botL;

			}

			pie_DrawTriangle(p, &texturePage, renderFlag, &offset);	
			// end tile-draw

			// -------------------------------------------------------------------------
				if (terrainOutline OR tileOutlined)
				{
					/*iV_Line(tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,
    					tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,255);
    				iV_Line(tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,
    					tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,255);
    				iV_Line(tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,
    					tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,255);
    				iV_Line(tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,
    					tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,255);*/
					 iV_Line(tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,
    					tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,
						outlineColour3D);
    				iV_Line(tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,
    					tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,
						outlineColour3D);
    				iV_Line(tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,
    					tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,
						outlineColour3D);
    				iV_Line(tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,
    					tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,
						outlineColour3D);
/*
   						if(TRI_FLIPPED(psTile))
						{
							iV_Line(tileScreenInfo[i+0][j+0].x,tileScreenInfo[i+0][j+0].y,
    							tileScreenInfo[i+1][j+1].x,tileScreenInfo[i+1][j+1].y,255);
						}
						else
						{
							iV_Line(tileScreenInfo[i+0][j+1].x,tileScreenInfo[i+0][j+1].y,
    						tileScreenInfo[i+1][j+0].x,tileScreenInfo[i+1][j+0].y,255);
						}
*/
				}
   		}
 }

/* Draw the buildings */
void displayStaticObjects( void )
{
	STRUCTURE	*psStructure;
	UDWORD		clan;
	UDWORD		test = 0;
	ANIM_OBJECT	*psAnimObj;

	/* Go through all the players */
	for (clan = 0; clan < MAX_PLAYERS; clan++)
	{
		/* Now go all buildings for that player */
		for(psStructure = apsStructLists[clan]; psStructure != NULL;
			psStructure = psStructure->psNext)
		{
			test++;
			/* Worth rendering the structure? */
			if(clipXY(psStructure->x,psStructure->y))
			{
				//don't use #ifndef BUCKET for now - need to do it this way for renderMapToBuffer
				if (!doBucket)
				{
					//over-ride the BUCKET def
					renderStructure(psStructure);
				}
				else
				{
					if ( psStructure->pStructureType->type == REF_RESOURCE_EXTRACTOR &&
						psStructure->psCurAnim == NULL &&
						(psStructure->currentBuildPts > (SDWORD)psStructure->pStructureType->buildPoints) )
					{
						psStructure->psCurAnim = animObj_Add( psStructure, ID_ANIM_DERIK, 0, 0 );
					}

					if ( psStructure->psCurAnim == NULL ||
							psStructure->psCurAnim->bVisible == FALSE ||
							(psAnimObj = animObj_Find( psStructure,
							psStructure->psCurAnim->uwID )) == NULL )
					{
						bucketAddTypeToList(RENDER_STRUCTURE, psStructure);
					}
					else
					{
						if ( psStructure->visible[selectedPlayer] OR godMode )
						{
							//check not a resource extractors
							if (psStructure->pStructureType->type != 
								REF_RESOURCE_EXTRACTOR)
							{
								displayAnimation( psAnimObj, FALSE );
							}
							//check that a power gen exists before animationg res extrac
							//else if (getPowerGenExists(psStructure->player))
							/*check the building is active*/
							else if (((RES_EXTRACTOR *)psStructure->
								pFunctionality)->active)
							{
								displayAnimation( psAnimObj, FALSE );
								if(selectedPlayer == psStructure->player)
								{
									audio_PlayObjStaticTrack( (void *) psStructure, ID_SOUND_OIL_PUMP_2 );
								}
							}
							else
							{
								/* hold anim on first frame */
								displayAnimation( psAnimObj, TRUE );
								audio_StopObjTrack( (void *) psStructure, ID_SOUND_OIL_PUMP_2 );
							}
						}
					}
				}
			}
		}
	}
}

//draw Factory Delivery Points
void displayDelivPoints(void)
{
	FLAG_POSITION	*psDelivPoint;

	//only do the selected players'
	/* go through all DPs for that player */
	for(psDelivPoint = apsFlagPosLists[selectedPlayer]; psDelivPoint != NULL;
		psDelivPoint = psDelivPoint->psNext)
	{
		if (clipXY(psDelivPoint->coords.x, psDelivPoint->coords.y))
		{
			if (!doBucket)
			{
				renderDeliveryPoint(psDelivPoint);
			}
			else
			{
				bucketAddTypeToList(RENDER_DELIVPOINT, psDelivPoint);
			}
		}
	}
}

/* Draw the features */
void displayFeatures( void )
{
FEATURE	*psFeature;
UDWORD		clan;

		/* player can only be 0 for the features */
		clan = 0;

		/* Go through all the features */
		for(psFeature = apsFeatureLists[clan]; psFeature != NULL;
			psFeature = psFeature->psNext)
		{
			/* Is the feature worth rendering? */
			if(clipXY(psFeature->x,psFeature->y))
			{
				//don't use #ifndef BUCKET for now - need to do it this way for renderMapToBuffer
				if (!doBucket)
				{
					//over-ride the BUCKET def
					renderFeature(psFeature);
				}
				else
				{
					bucketAddTypeToList(RENDER_FEATURE, psFeature);
				}
	 		}
		}
 }

/* Draw the Proximity messages for the **SELECTED PLAYER ONLY***/
void displayProximityMsgs( void )
{
	PROXIMITY_DISPLAY	*psProxDisp;
	VIEW_PROXIMITY		*pViewProximity;
	UDWORD				x, y;

	/* Go through all the proximity Displays*/
	for (psProxDisp = apsProxDisp[selectedPlayer]; psProxDisp != NULL; 
		psProxDisp = psProxDisp->psNext)
	{
	  	if(!((VIEW_PROXIMITY*)psProxDisp->psMessage->read))
		{
			if (psProxDisp->type == POS_PROXDATA)
			{
				pViewProximity = (VIEW_PROXIMITY*)((VIEWDATA *)psProxDisp->psMessage->
					pViewData)->pData;
				x = pViewProximity->x;
				y = pViewProximity->y;
			}
			else
			{
				x = ((BASE_OBJECT *)psProxDisp->psMessage->pViewData)->x;
				y = ((BASE_OBJECT *)psProxDisp->psMessage->pViewData)->y;
			}
			/* Is the Message worth rendering? */
			//if(clipXY(pViewProximity->x,pViewProximity->y))
			if(clipXY(x,y))
			{
				//don't use #ifndef BUCKET for now - need to do it this way for renderMapToBuffer
				if (!doBucket)
				{
					//over-ride the BUCKET def
					renderProximityMsg(psProxDisp);
				}
				else
				{
					bucketAddTypeToList(RENDER_PROXMSG, psProxDisp);
				}
			}
		}
	}
}

void
displayAnimation( ANIM_OBJECT * psAnimObj, BOOL bHoldOnFirstFrame )
{
	UWORD				i,uwFrame;
	VECTOR3D			vecPos, vecRot, vecScale;
	COMPONENT_OBJECT	*psComp;

	for ( i=0; i<psAnimObj->psAnim->uwObj; i++ )
	{
		if ( bHoldOnFirstFrame == TRUE )
		{
			uwFrame = 0;
			vecPos.x = vecPos.y = vecPos.z = 0;
			vecRot.x = vecRot.y = vecRot.z = 0;
			vecScale.x = vecScale.y = vecScale.z = 0;
		}
		else
		{
			uwFrame = animObj_GetFrame3D( psAnimObj, i,
										&vecPos, &vecRot, &vecScale );
		}

		if ( uwFrame != ANIM_DELAYED )
		{
			if ( psAnimObj->psAnim->animType == ANIM_3D_TRANS )
			{
				psComp = &psAnimObj->apComponents[i];
			}
			else
			{
				psComp = &psAnimObj->apComponents[uwFrame];
			}

			psComp->position.x = vecPos.x;
			psComp->position.y = vecPos.y;
			psComp->position.z = vecPos.z;

			psComp->orientation.x = vecRot.x;
			psComp->orientation.y = vecRot.y;
			psComp->orientation.z = vecRot.z;

			bucketAddTypeToList( RENDER_ANIMATION, psComp );
		}
	}
}

/* Draw the droids */
void displayDynamicObjects( void )
{
	DROID		*psDroid;
	ANIM_OBJECT	*psAnimObj;
	UDWORD		clan;

	/* Need to go through all the droid lists */
	for(clan = 0; clan < MAX_PLAYERS; clan++)
	{
		for(psDroid = apsDroidLists[clan]; psDroid != NULL;
			psDroid = psDroid->psNext)
		{
			/* Find out whether the droid is worth rendering */
				if(clipXY(psDroid->x,psDroid->y))
				{
					/* No point in adding it if you can't see it? */
					if(psDroid->visible[selectedPlayer] OR godMode OR demoGetStatus())
					{
					 	psDroid->sDisplay.frameNumber = currentGameFrame;
						//don't use #ifndef BUCKET for now - need to do it this way for renderMapToBuffer
						if (!doBucket)
						{
							//over-ride the BUCKET def
							renderDroid( (DROID *) psDroid);
						}
						else
						{
							/* draw droid even if animating (still need to draw weapons) */
							bucketAddTypeToList(RENDER_DROID, psDroid);
//							bucketAddTypeToList(RENDER_SHADOW, psDroid);

							/* draw anim if visible */
							if ( psDroid->psCurAnim != NULL &&
								 psDroid->psCurAnim->bVisible == TRUE &&
								 (psAnimObj = animObj_Find( psDroid,
								  psDroid->psCurAnim->uwID )) != NULL )
							{
								displayAnimation( psAnimObj, FALSE );
							}
						}
						//showDroidSelection(psDroid);
					}
				} // end clipDroid
		} // end for
	} // end for clan
} // end Fn

/* Sets the player's position and view angle - defaults player rotations as well */
void setViewPos(UDWORD x, UDWORD y,BOOL Pan)
{
//BOOL	changed = FALSE;
SDWORD midX,midY;


	/* Find centre of grid thats actually DRAWN */
	midX = x-(visibleXTiles/2);
	midY = y-(visibleYTiles/2);


	player.p.x = midX*TILE_UNITS;
	player.p.z = midY*TILE_UNITS;
	player.r.z = 0;

	if(getWarCamStatus())
	{
		camToggleStatus();
	}

	SetRadarStrobe(midX,midY);
	scroll();
}

void getPlayerPos(SDWORD *px, SDWORD *py)
{
	*px = player.p.x + (visibleXTiles/2)*TILE_UNITS;
	*py = player.p.z + (visibleYTiles/2)*TILE_UNITS;
}

void setPlayerPos(SDWORD x, SDWORD y)
{
	SDWORD midX,midY;


	ASSERT(((x > 0) && (x < (SDWORD)(mapWidth*TILE_UNITS)) &&
			(y > 0) && (y < (SDWORD)(mapHeight*TILE_UNITS)),
		"setPlayerPos: position off map"));

	// Find centre of grid thats actually DRAWN
	midX = (x>>TILE_SHIFT)-(visibleXTiles/2);
	midY = (y>>TILE_SHIFT)-(visibleYTiles/2);

	player.p.x = midX*TILE_UNITS;
	player.p.z = midY*TILE_UNITS;
	player.r.z = 0;

	SetRadarStrobe(midX,midY);
}


void	setViewAngle(SDWORD angle)
{
	player.r.x = DEG(360 + angle);
}


UDWORD getViewDistance(VOID)
{
	return distance;
}

void	setViewDistance(UDWORD dist)
{
	dist = distance;
}


void	renderFeature(FEATURE *psFeature)
{
UDWORD		featX,featY;	
SDWORD		rotation;
//SDWORD		centreX,centreZ;
UDWORD		brightness, specular;
iVector		dv;
iVector		*vecTemp;
BOOL		bForceDraw;


//	if(psFeature->psStats->subType == FEAT_BUILD_WRECK)
//	{
//		return;//don't draw 'em	
//	}

	bForceDraw = ( !getRevealStatus() AND psFeature->psStats->visibleAtStart);

	if (psFeature->visible[selectedPlayer] OR godMode OR demoGetStatus() OR bForceDraw)
	{
		psFeature->sDisplay.frameNumber = currentGameFrame;
		/* Get it's x and y coordinates so we don't have to deref. struct later */
		featX = psFeature->x;
		featY = psFeature->y;
		/* Daft hack to get around the oild derrick issue */
		if(!TILE_HAS_FEATURE(mapTile(featX>>TILE_SHIFT,featY>>TILE_SHIFT)))
		{
			return;
		}
		dv.x = (featX - player.p.x) - terrainMidX*TILE_UNITS;
		dv.z = terrainMidY*TILE_UNITS - (featY - player.p.z);


		/* features sits at the height of the tile it's centre is on */
		dv.y = psFeature->z;
		SetBSPObjectPos(featX,dv.y,featY);	// world x,y,z coord of structure ... this is needed for the BSP code

		/* Push the indentity matrix */
		iV_MatrixBegin();

		/* Translate the feature  - N.B. We can also do rotations here should we require
		   buildings to face different ways - Don't know if this is necessary - should be IMO */
		iV_TRANSLATE(dv.x,dv.y,dv.z);
		/* Get the x,z translation components */
		rx = player.p.x & (TILE_UNITS-1);
		rz = player.p.z & (TILE_UNITS-1);

		/* Translate */
		iV_TRANSLATE(rx,0,-rz);
		rotation = DEG(psFeature->direction);

		iV_MatrixRotateY(-rotation);


//		centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//		centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
		brightness = 200; //? HUH?

//		if(psFeature->sDisplay.imd->ymax>300)
		if(psFeature->psStats->subType == FEAT_SKYSCRAPER)
		{
			objectShimmy((BASE_OBJECT*)psFeature);
		}

		if(godMode OR demoGetStatus() OR bForceDraw)
		{
			brightness = 200;
		}
		else if(getRevealStatus())
		{
			brightness = avGetObjLightLevel((BASE_OBJECT*)psFeature,brightness);
		}

		brightness = lightDoFogAndIllumination(brightness,getCentreX()-featX,getCentreZ()-featY, &specular);
		if(psFeature->psStats->subType == FEAT_OIL_RESOURCE)
		{
			vecTemp = psFeature->sDisplay.imd->points;
			flattenImd(psFeature->sDisplay.imd,psFeature->x,psFeature->y,0);
			/* currentGameFrame/2 set anim running - GJ hack */
			pie_Draw3DShape(psFeature->sDisplay.imd, currentGameFrame/2, 0, brightness, specular, 0, 0);
			psFeature->sDisplay.imd->points = vecTemp;
		}
		else
		{
			pie_Draw3DShape(psFeature->sDisplay.imd, 0, 0, brightness, specular, 0,0);//pie_TRANSLUCENT, psFeature->visible[selectedPlayer]);
		}

		{
			SDWORD sX,sY;
			pie_SETUP_ROTATE_PROJECT;
			pie_ROTATE_PROJECT(0,0,0,sX,sY);
			psFeature->sDisplay.screenX = sX;
			psFeature->sDisplay.screenY = sY;
			targetAdd((BASE_OBJECT*)psFeature);
		}

		iV_MatrixEnd();
	}
}

void renderProximityMsg(PROXIMITY_DISPLAY *psProxDisp)
{
	UDWORD			msgX,msgY;	
	iVector			dv;
	VIEW_PROXIMITY	*pViewProximity = NULL;
	SDWORD			x, y, r;
	iIMDShape		*proxImd;
//	SDWORD		centreX,centreZ;
	UDWORD		brightness, specular;

	//store the frame number for when deciding what has been clicked on
	psProxDisp->frameNumber = currentGameFrame;

	/* Get it's x and y coordinates so we don't have to deref. struct later */
	if (psProxDisp->type == POS_PROXDATA)
	{
		pViewProximity = (VIEW_PROXIMITY*)((VIEWDATA *)psProxDisp->psMessage->
			pViewData)->pData;
		if (pViewProximity)
		{
			msgX = pViewProximity->x;
			msgY = pViewProximity->y;
			/* message sits at the height specified at input*/
			dv.y = pViewProximity->z + 64;
		}
	}
	else if (psProxDisp->type == POS_PROXOBJ)
	{
		msgX = ((BASE_OBJECT *)psProxDisp->psMessage->pViewData)->x;
		msgY = ((BASE_OBJECT *)psProxDisp->psMessage->pViewData)->y;
		/* message sits at the height specified at input*/
		dv.y = ((BASE_OBJECT *)psProxDisp->psMessage->pViewData)->z + 64;
	}
	else
	{
		ASSERT((FALSE,"Buggered proximity message type"));
	}
//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX()-msgX,getCentreZ()-msgY, &specular);

	dv.x = (msgX - player.p.x) - terrainMidX*TILE_UNITS;
	dv.z = terrainMidY*TILE_UNITS - (msgY - player.p.z);
		
	/* Push the indentity matrix */
	iV_MatrixBegin();

	/* Translate the message */
	iV_TRANSLATE(dv.x,dv.y,dv.z);
	/* Get the x,z translation components */
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Translate */
	iV_TRANSLATE(rx,0,-rz);
	//get the appropriate IMD
	if (pViewProximity)
	{
		switch(pViewProximity->proxType)
		{
		case PROX_ENEMY:
			proxImd = getImdFromIndex(MI_BLIP_ENEMY);
			break;
		case PROX_RESOURCE:
			proxImd = getImdFromIndex(MI_BLIP_RESOURCE);
			break;
		case PROX_ARTEFACT:
			proxImd = getImdFromIndex(MI_BLIP_ARTEFACT);
			break;
		default:
			ASSERT((FALSE,"Buggered proximity message type"));
			break;
		}
	}
	else
	{
		//object Proximity displays are for oil resources and artefacts
		ASSERT(( ((BASE_OBJECT *)psProxDisp->psMessage->pViewData)->type == 
			OBJ_FEATURE, "renderProximityMsg: invalid feature" ));

		if (((FEATURE *)psProxDisp->psMessage->pViewData)->psStats->subType == 
			FEAT_OIL_RESOURCE)
		{
			//resource
			proxImd = getImdFromIndex(MI_BLIP_RESOURCE);
		}
		else
		{
			//artefact
			proxImd = getImdFromIndex(MI_BLIP_ARTEFACT);
		}
	}
	
//	iV_MatrixRotateY(DEG(gameTime/10));
	iV_MatrixRotateY(-player.r.y);
	iV_MatrixRotateX(-player.r.x);

	if(!gamePaused())
	{
		pie_Draw3DShape(proxImd, getTimeValueRange(1000,4), 0, brightness, specular, pie_ADDITIVE, 192);
	}
	else
	{
		pie_Draw3DShape(proxImd, 0, 0, brightness, specular, pie_ADDITIVE, 192);
	}
	
	//get the screen coords for determining when clicked on
	calcFlagPosScreenCoords(&x, &y, &r);
	psProxDisp->screenX = x;
	psProxDisp->screenY = y;
	psProxDisp->screenR = r;
	//storeProximityScreenCoords(psMessage, x, y);

	iV_MatrixEnd();
}

#define STRUCTURE_ANIM_RATE 4

#define ELEC_DAMAGE_DURATION	(GAME_TICKS_PER_SEC/5)
void	renderStructure(STRUCTURE *psStructure)
{
//MAPTILE		*psTile;
SDWORD			structX,structY;	
SDWORD			sX,sY;
iIMDShape		*baseImd,*strImd,*mountImd,*weaponImd,*flashImd;
SDWORD			rotation;
SDWORD			frame;
SDWORD			playerFrame;
SDWORD			animFrame;
UDWORD			nWeaponStat;
//UDWORD			stage;
UDWORD			buildingBrightness, specular;
//SDWORD			centreX,centreZ;
iVector			dv;
SDWORD			i;
iIMDShape		*lImd,*imd;
iVector			*temp;
SDWORD			brightVar;
BOOL			bHitByElectronic = FALSE;
SDWORD			yVar;
iIMDShape		*pRepImd;
REPAIR_FACILITY		*psRepairFac = NULL;
	
	//REF_DEFENSE no longer drawn as sloping wall
	//if(psStructure->pStructureType->type>=REF_WALL AND
	//	psStructure->pStructureType->type<=REF_TOWER4)
	if(psStructure->pStructureType->type == REF_WALL OR
		psStructure->pStructureType->type == REF_WALLCORNER)
	{
		renderWallSection(psStructure);
		return;
	}

	if(psStructure->pStructureType->type == REF_DEFENSE)
	{
		renderDefensiveStructure(psStructure);
		return;
	}

	// -------------------------------------------------------------------------------
	/* Power stations and factories have pulsing lights  */
	if(psStructure->sDisplay.imd->numFrames > 0)
	{
        /*OK, so we've got a hack for a new structure - its a 2x2 wall but 
        we've called it a BLAST_DOOR cos we don't want it to use the wallDrag code
        So its got clan colour trim and not really an anim - these HACKS just keep
        coming back to haunt us hey? - AB 02/09/99*/
        if (bMultiPlayer AND psStructure->pStructureType->type == REF_BLASTDOOR)
        {
            animFrame = getPlayerColour(psStructure->player);;
        }
        else
        {
		    //calculate an animation frame
		    animFrame = (gameTime%4000)/1000;//(gameTime * STRUCTURE_ANIM_RATE)/GAME_TICKS_PER_SEC;//one frame per second
    		//psStructure->sDisplay.animFrame = animFrame;
        }
	}
	else
	{
		animFrame = 0;
	}
	playerFrame =getPlayerColour(psStructure->player);// psStructure->player

  	// -------------------------------------------------------------------------------

  
	if(psStructure->visible[selectedPlayer] OR godMode OR demoGetStatus())
	{
		psStructure->sDisplay.frameNumber = currentGameFrame;
		/* Get it's x and y coordinates so we don't have to deref. struct later */
		structX = psStructure->x;
		structY = psStructure->y;
		dv.x = (structX - player.p.x) - terrainMidX*TILE_UNITS;
		dv.z = terrainMidY*TILE_UNITS - (structY - player.p.z);
			
 		dv.y = map_TileHeight(structX>>TILE_SHIFT, structY>>TILE_SHIFT);  
		SetBSPObjectPos(structX,dv.y,structY);	// world x,y,z coord of structure ... this is needed for the BSP code
		/* Push the indentity matrix */
		iV_MatrixBegin();

		/* Translate the building  - N.B. We can also do rotations here should we require
		   buildings to face different ways - Don't know if this is necessary - should be IMO */
		iV_TRANSLATE(dv.x,dv.y,dv.z);
		/* Get the x,z translation components */
		rx = player.p.x & (TILE_UNITS-1);
		rz = player.p.z & (TILE_UNITS-1);
		
		/* Translate */
		iV_TRANSLATE(rx,0,-rz);
		/* OK - here is where we establish which IMD to draw for the building - luckily static objects,
		buildings in other words are NOT made up of components - much quicker! */
	
	
		rotation = DEG(psStructure->direction);
		iV_MatrixRotateY(-rotation);
//		centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//		centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

		bHitByElectronic = FALSE;
		if( (gameTime2-psStructure->timeLastHit < ELEC_DAMAGE_DURATION) AND 
				(psStructure->lastHitWeapon == WSC_ELECTRONIC) )
		{
			bHitByElectronic = TRUE;
		}

	   	buildingBrightness = 200 - (100-PERCENT( psStructure->body , structureBody(psStructure)));

		if(psStructure->selected)
		{
			if(!gamePaused())
			{
				brightVar = getStaticTimeValueRange(990,110);
				if(brightVar>55) brightVar = 110-brightVar;
			}
			else
			{
				brightVar = 55;
			}

			buildingBrightness = 200+brightVar;
		}
		
		if(godMode OR demoGetStatus())
		{
			buildingBrightness = buildingBrightness;
		}
		else
			if(getRevealStatus())
		{
			buildingBrightness = avGetObjLightLevel((BASE_OBJECT*)psStructure,buildingBrightness);
		}
		buildingBrightness = lightDoFogAndIllumination((UBYTE)buildingBrightness,getCentreX()-structX,getCentreZ()-structY, &specular);

		/* Draw the building's base first */
		baseImd = psStructure->pStructureType->pBaseIMD;

		if(baseImd!=NULL)
		{
			pie_Draw3DShape(baseImd, 0, 0, buildingBrightness, specular, 0,0);
		}						
		
		// override
		if(bHitByElectronic)
		{
			buildingBrightness = 150;
		}

		imd = psStructure->sDisplay.imd;

	   	if(imd!=NULL AND bHitByElectronic)
		{
			// Get a copy of the points 
			memcpy(alteredPoints,imd->points,imd->npoints*sizeof(iVector));
			for(i=0; i<imd->npoints; i++)
			{
				yVar = (10-rand()%20);
				alteredPoints[i].x +=yVar - (rand()%2*yVar);
				alteredPoints[i].z +=yVar - (rand()%2*yVar);
			}
			temp = imd->points;
			imd->points = alteredPoints;
		}

	   


		//first check if partially built - ANOTHER HACK!
		if ( (psStructure->status == SS_BEING_BUILT ) OR
			 (psStructure->status == SS_BEING_DEMOLISHED ) OR
			 (psStructure->status == SS_BEING_BUILT AND psStructure->pStructureType->type == REF_RESOURCE_EXTRACTOR) )
		{
			pie_Draw3DShape(imd, 0, playerFrame, 
			buildingBrightness, specular, pie_HEIGHT_SCALED, 
			(SDWORD)(structHeightScale(psStructure) * pie_RAISE_SCALE));
			if(bHitByElectronic)
			{
				imd->points = temp;
			}

		}
		else if(psStructure->status == SS_BUILT)
		{
			/* They're already built!!!! */
			pie_Draw3DShape(imd, animFrame, 0, buildingBrightness, specular, 0,0);
			if(bHitByElectronic)
			{
				imd->points = temp;
			}

			if(psStructure->sDisplay.imd->nconnectors == 1)
			{
				weaponImd = NULL;
				mountImd = NULL;
				flashImd = NULL;
				strImd = psStructure->sDisplay.imd;
			   
                if (psStructure->asWeaps[0].nStat > 0)
				{
					nWeaponStat = psStructure->asWeaps[0].nStat;
					weaponImd =  asWeaponStats[nWeaponStat].pIMD;
					mountImd =  asWeaponStats[nWeaponStat].pMountGraphic;
					flashImd =  asWeaponStats[nWeaponStat].pMuzzleGraphic;
				}

				if (weaponImd == NULL)
				{
					//check for ECM
					if (psStructure->pStructureType->pECM != NULL)
					{
						weaponImd =  psStructure->pStructureType->pECM->pIMD;
						mountImd =  psStructure->pStructureType->pECM->pMountGraphic;
						flashImd = NULL;
					}
				}

				if (weaponImd == NULL)
				{
					//check for sensor
					if (psStructure->pStructureType->pSensor != NULL)
					{
						weaponImd =  psStructure->pStructureType->pSensor->pIMD;
						/* No recoil for sensors */
						psStructure->asWeaps[0].recoilValue = 0;
						mountImd  =  psStructure->pStructureType->pSensor->pMountGraphic;
						flashImd = NULL;
					}
				}

				//draw Weapon/ECM/Sensor for structure
				if(weaponImd != NULL)
				{
					iV_MatrixBegin();
					iV_TRANSLATE(strImd->connectors->x,strImd->connectors->z,strImd->connectors->y);
					pie_MatRotY(DEG(-((SDWORD)psStructure->turretRotation)));
					if (mountImd != NULL)
					{
						pie_TRANSLATE(0,0,psStructure->asWeaps[0].recoilValue/3);

						pie_Draw3DShape(mountImd, animFrame, 0, buildingBrightness, specular, 0,0);//pie_TRANSLUCENT, psStructure->visible[selectedPlayer]);
						if(mountImd->nconnectors)
						{
							iV_TRANSLATE(mountImd->connectors->x,mountImd->connectors->z,mountImd->connectors->y);
						}
					}
					iV_MatrixRotateX(DEG(psStructure->turretPitch));
					pie_TRANSLATE(0,0,psStructure->asWeaps[0].recoilValue);

					pie_Draw3DShape(weaponImd, playerFrame, 0, buildingBrightness, specular, 0,0);//pie_TRANSLUCENT, psStructure->visible[selectedPlayer]);
				 	if(psStructure->pStructureType->type == REF_REPAIR_FACILITY)
					{
						psRepairFac = (REPAIR_FACILITY*)psStructure->pFunctionality;
                        //draw repair flash if the Repair Facility has a target which it has started work on
						if(weaponImd->nconnectors AND psRepairFac->psObj!=NULL
                            AND psRepairFac->psObj->type == OBJ_DROID AND
                            ((DROID *)psRepairFac->psObj)->action == DACTION_WAITDURINGREPAIR )
						{
							iV_TRANSLATE(weaponImd->connectors->x,weaponImd->connectors->z-12,weaponImd->connectors->y);
							pRepImd = getImdFromIndex(MI_FLAME);
					   	   
							pie_MatRotY(DEG((SDWORD)psStructure->turretRotation));

							iV_MatrixRotateY(-player.r.y);         
							iV_MatrixRotateX(-player.r.x);    
						   	/* Dither on software */
							if(pie_GetRenderEngine() == ENGINE_4101)
							{
								pie_SetDitherStatus(TRUE);
							}
							pie_Draw3DShape(pRepImd, getStaticTimeValueRange(100,pRepImd->numFrames), 0, buildingBrightness, 0, pie_ADDITIVE, 192);
						   		/* Dither on software */
							if(pie_GetRenderEngine() == ENGINE_4101)
							{
								pie_SetDitherStatus(FALSE);
							}

					  		iV_MatrixRotateX(player.r.x);   
					  		iV_MatrixRotateY(player.r.y);         
							pie_MatRotY(DEG((SDWORD)psStructure->turretRotation));
							
						}
					}
					//we have a droid weapon so do we draw a muzzle flash
					else if( weaponImd->nconnectors AND psStructure->visible[selectedPlayer]>(UBYTE_MAX/2))
					{
						/* Now we need to move to the end fo the barrel */
						pie_TRANSLATE( weaponImd->connectors[0].x,
									   weaponImd->connectors[0].z,
									   weaponImd->connectors[0].y  );
						//and draw the muzzle flash
						//animate for the duration of the flash only
						if(flashImd)
						{
							//assume no clan colours formuzzle effects
						  	if ((flashImd->numFrames == 0) || (flashImd->animInterval <= 0))//no anim so display one frame for a fixed time
							{
						  		if (gameTime < (psStructure->asWeaps->lastFired + BASE_MUZZLE_FLASH_DURATION))
								{
									pie_Draw3DShape(flashImd, 0, 0, buildingBrightness, specular, pie_ADDITIVE, 12);//muzzle flash
								}
							}
							
							else
							{
							   	frame = (gameTime - psStructure->asWeaps->lastFired)/flashImd->animInterval;
								if (frame < flashImd->numFrames)
								{
									pie_Draw3DShape(flashImd, frame, 0, buildingBrightness, specular, pie_ADDITIVE, 12);//muzzle flash
								}
							}
							
						}
					}
					iV_MatrixEnd();
				}
			}
			else if(psStructure->sDisplay.imd->nconnectors > 1)// add some lights if we have the connectors for it
			{
				for (i = 0; i < psStructure->sDisplay.imd->nconnectors; i++)
				{
					iV_MatrixBegin();
					iV_TRANSLATE(psStructure->sDisplay.imd->connectors->x,psStructure->sDisplay.imd->connectors->z,psStructure->sDisplay.imd->connectors->y);
					lImd = getImdFromIndex(MI_LANDING);
					pie_Draw3DShape(lImd, getStaticTimeValueRange(1024,lImd->numFrames), 0, buildingBrightness, specular, 0,0);//pie_TRANSLUCENT, psStructure->visible[selectedPlayer]);
					iV_MatrixEnd();
				}
			}
			else //its a baba machine gun
			{
			    if (psStructure->asWeaps[0].nStat > 0)
				{
					flashImd = NULL;
					strImd = psStructure->sDisplay.imd;
					//get an imd to draw on the connector priority is weapon, ECM, sensor
					//check for weapon
					nWeaponStat = psStructure->asWeaps[0].nStat;
					flashImd =  asWeaponStats[nWeaponStat].pMuzzleGraphic;
					//draw Weapon/ECM/Sensor for structure
					if(flashImd != NULL)
					{
						iV_MatrixBegin();
						if (strImd->ymax > 80)//babatower
						{
							iV_TRANSLATE(0,80,0);
							pie_MatRotY(DEG(-((SDWORD)psStructure->turretRotation)));
							iV_TRANSLATE(0,0,-20);
						}
						else//baba bunker
						{
							iV_TRANSLATE(0,10,0);
							pie_MatRotY(DEG(-((SDWORD)psStructure->turretRotation)));
							iV_TRANSLATE(0,0,-40);
						}
						iV_MatrixRotateX(DEG(psStructure->turretPitch));
						//and draw the muzzle flash
						//animate for the duration of the flash only
						//assume no clan colours formuzzle effects
						if ((flashImd->numFrames == 0) || (flashImd->animInterval <= 0))//no anim so display one frame for a fixed time
						{
							if (gameTime < (psStructure->asWeaps->lastFired + BASE_MUZZLE_FLASH_DURATION))
							{
								pie_Draw3DShape(flashImd, 0, 0, buildingBrightness, specular, 0, 0);//muzzle flash
							}
						}
						else
						{
							frame = (gameTime - psStructure->asWeaps->lastFired)/flashImd->animInterval;
							if (frame < flashImd->numFrames)
							{
								pie_Draw3DShape(flashImd, 0, 0, buildingBrightness, specular, 0, 0);//muzzle flash
							}
						}
						iV_MatrixEnd();
					}
				}
			}
		}	
		{
			pie_SETUP_ROTATE_PROJECT;
			pie_ROTATE_PROJECT(0,0,0,sX,sY);
			psStructure->sDisplay.screenX = sX;
			psStructure->sDisplay.screenY = sY;
		}
		iV_MatrixEnd();

		targetAdd((BASE_OBJECT*)psStructure);
	}
}

//---
void	renderDefensiveStructure(STRUCTURE *psStructure)
{

SDWORD			structX,structY;	
SDWORD			sX,sY;
iIMDShape		*strImd,*mountImd,*weaponImd,*flashImd;
SDWORD			rotation;
SDWORD			frame;
SDWORD			playerFrame;
SDWORD			animFrame;
UDWORD			nWeaponStat;
UDWORD			brightness;
UDWORD			buildingBrightness, specular;
//SDWORD			centreX,centreZ;
iVector			dv;
SDWORD			i;
iIMDShape		*imd,*lImd;
iVector			*temp;
SDWORD			pointHeight,strHeight,shift;
SDWORD			brightVar;
	
  	playerFrame =getPlayerColour(psStructure->player);// psStructure->player
   	animFrame = playerFrame;
	// -------------------------------------------------------------------------------
	if(psStructure->visible[selectedPlayer] OR godMode OR demoGetStatus())
	{
  		/* Mark it as having been drawn */
		psStructure->sDisplay.frameNumber = currentGameFrame;
		/* Get it's x and y coordinates so we don't have to deref. struct later */
		structX = psStructure->x;
		structY = psStructure->y;
		/* Get coordinates of world centre - hmmm. optimise this out? */
//		centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//		centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
		// Play with the imd so its flattened
		imd = psStructure->sDisplay.imd;
		if(imd!=NULL)
		{
			// Get a copy of the points 
			memcpy(alteredPoints,imd->points,imd->npoints*sizeof(iVector));
			// Get the height of the centre point for reference 
			strHeight = psStructure->z;//map_Height(structX,structY) + 64;
		   //	 Now we got through the shape looking for vertices on the edge 
			for(i=0; i<imd->npoints; i++)
			{
				if(alteredPoints[i].y<=0)
				{
					pointHeight = map_Height(structX+alteredPoints[i].x,structY-alteredPoints[i].z);
					shift = strHeight - pointHeight;
					alteredPoints[i].y -= shift;
				}
			}
		}

		dv.x = (structX - player.p.x) - terrainMidX*TILE_UNITS;
		dv.z = terrainMidY*TILE_UNITS - (structY - player.p.z);
		dv.y = psStructure->z;//map_TileHeight(structX>>TILE_SHIFT, structY>>TILE_SHIFT)+64;  
		
		SetBSPObjectPos(structX,dv.y,structY);	// world x,y,z coord of structure ... this is needed for the BSP code
		/* Push the indentity matrix */
		iV_MatrixBegin();

		/* Translate the building  - N.B. We can also do rotations here should we require
		   buildings to face different ways - Don't know if this is necessary - should be IMO */
		iV_TRANSLATE(dv.x,dv.y,dv.z);
		/* Get the x,z translation components */
		rx = player.p.x & (TILE_UNITS-1);
		rz = player.p.z & (TILE_UNITS-1);
		
		/* Translate */
		iV_TRANSLATE(rx,0,-rz);
	   	
		rotation = DEG(psStructure->direction);
		iV_MatrixRotateY(-rotation);
	   
		/* Get the buildings brightness level - proportional to how damaged it is */
		buildingBrightness = 200 - (100-PERCENT( psStructure->body , structureBody(psStructure)));
		/* If it's selected, then it's brighter */
		if(psStructure->selected)
		{
			if(!gamePaused())
			{
				brightVar = getStaticTimeValueRange(990,110);
				if(brightVar>55) brightVar = 110-brightVar;
			}
			else
			{
				brightVar = 55;
			}

			buildingBrightness = 200 + brightVar;
		}
		if(godMode OR demoGetStatus())
		{
			buildingBrightness = buildingBrightness;
		}
		else
			if(getRevealStatus())
		{
			buildingBrightness = avGetObjLightLevel((BASE_OBJECT*)psStructure,buildingBrightness);
		}
		brightness = lightDoFogAndIllumination((UBYTE)buildingBrightness,getCentreX()-structX,getCentreZ()-structY, &specular);

		//first check if partially built - ANOTHER HACK!
		if ( (psStructure->status == SS_BEING_BUILT ) OR (psStructure->status == SS_BEING_DEMOLISHED ) )
		{
			temp = imd->points;
			imd->points = alteredPoints;
			pie_Draw3DShape(imd, 0, playerFrame, 
			brightness, specular, pie_HEIGHT_SCALED, 
			(SDWORD)(structHeightScale(psStructure) * pie_RAISE_SCALE));
			imd->points = temp;

		}
		else if(psStructure->status == SS_BUILT)
		{
			temp = imd->points;
			imd->points = alteredPoints;
			pie_Draw3DShape(imd, animFrame, 0, brightness, specular, 0,0);
			imd->points = temp;

			// It might have weapons on it
			if(psStructure->sDisplay.imd->nconnectors == 1)
			{
				weaponImd = NULL;//weapon is gun ecm or sensor
				mountImd = NULL;
				flashImd = NULL;
				strImd = psStructure->sDisplay.imd;
				//get an imd to draw on the connector priority is weapon, ECM, sensor
				//check for weapon
				if (psStructure->asWeaps[0].nStat > 0)
				{
					nWeaponStat = psStructure->asWeaps[0].nStat;
					weaponImd =  asWeaponStats[nWeaponStat].pIMD;
					mountImd =  asWeaponStats[nWeaponStat].pMountGraphic;
					flashImd =  asWeaponStats[nWeaponStat].pMuzzleGraphic;
				}

				if (weaponImd == NULL)
				{
					//check for ECM
					if (psStructure->pStructureType->pECM != NULL)
					{
						weaponImd =  psStructure->pStructureType->pECM->pIMD;
						mountImd =  psStructure->pStructureType->pECM->pMountGraphic;
						flashImd = NULL;
					}
				}

				if (weaponImd == NULL)
				{
					//check for sensor
					if (psStructure->pStructureType->pSensor != NULL)
					{
						weaponImd =  psStructure->pStructureType->pSensor->pIMD;
						/* No recoil for sensors */
						psStructure->asWeaps[0].recoilValue = 0;
						mountImd  =  psStructure->pStructureType->pSensor->pMountGraphic;
						flashImd = NULL;
					}
				}

				//draw Weapon/ECM/Sensor for structure
				if(weaponImd != NULL)
				{
					iV_MatrixBegin();
					iV_TRANSLATE(strImd->connectors->x,strImd->connectors->z,strImd->connectors->y);
					pie_MatRotY(DEG(-((SDWORD)psStructure->turretRotation)));
					if (mountImd != NULL)
					{
						pie_TRANSLATE(0,0,psStructure->asWeaps[0].recoilValue/3);

						pie_Draw3DShape(mountImd, animFrame, 0, brightness, specular, 0,0);
						if(mountImd->nconnectors)
						{
							iV_TRANSLATE(mountImd->connectors->x,mountImd->connectors->z,mountImd->connectors->y);
						}
					}
					iV_MatrixRotateX(DEG(psStructure->turretPitch));
					pie_TRANSLATE(0,0,psStructure->asWeaps[0].recoilValue);

					pie_Draw3DShape(weaponImd, animFrame, 0, brightness, specular, 0,0);
					//we have a droid weapon so do we draw a muzzle flash
					if( weaponImd->nconnectors AND psStructure->visible[selectedPlayer]>(UBYTE_MAX/2))
					{
						/* Now we need to move to the end fo the barrel */
						pie_TRANSLATE( weaponImd->connectors[0].x,
									   weaponImd->connectors[0].z,
									   weaponImd->connectors[0].y  );
						//and draw the muzzle flash
						//animate for the duration of the flash only
						if(flashImd)
						{
							//assume no clan colours formuzzle effects
						 	if ((flashImd->numFrames == 0) || (flashImd->animInterval <= 0))//no anim so display one frame for a fixed time
							{
						  		if (gameTime < (psStructure->asWeaps->lastFired + BASE_MUZZLE_FLASH_DURATION))
								{
									pie_Draw3DShape(flashImd, 0, 0, brightness, specular, pie_ADDITIVE, 128);//muzzle flash
								}
							}
							
							else
							{
								frame = (gameTime - psStructure->asWeaps->lastFired)/flashImd->animInterval;
								if (frame < flashImd->numFrames)
								{
									pie_Draw3DShape(flashImd, frame, 0, brightness, specular, pie_ADDITIVE, 20);//muzzle flash
								}
							}
							
						}
					}
					iV_MatrixEnd();
				}
			}
			else if(psStructure->sDisplay.imd->nconnectors > 1)// add some lights if we have the connectors for it
			{
				for (i = 0; i < psStructure->sDisplay.imd->nconnectors; i++)
				{
					iV_MatrixBegin();
					iV_TRANSLATE(psStructure->sDisplay.imd->connectors->x,psStructure->sDisplay.imd->connectors->z,psStructure->sDisplay.imd->connectors->y);
					lImd = getImdFromIndex(MI_LANDING);
					pie_Draw3DShape(lImd, getStaticTimeValueRange(1024,lImd->numFrames), 0, brightness, specular, 0,0);//pie_TRANSLUCENT, psStructure->visible[selectedPlayer]);
					iV_MatrixEnd();
				}
			}
			else //its a baba machine gun
			{
                if (psStructure->asWeaps[0].nStat > 0)
				{
					flashImd = NULL;
					strImd = psStructure->sDisplay.imd;
					//get an imd to draw on the connector priority is weapon, ECM, sensor
					//check for weapon
					nWeaponStat = psStructure->asWeaps[0].nStat;
					flashImd =  asWeaponStats[nWeaponStat].pMuzzleGraphic;
					//draw Weapon/ECM/Sensor for structure
					if(flashImd != NULL)
					{
						iV_MatrixBegin();
						//horrendous hack
						if (strImd->ymax > 80)//babatower
						{
							iV_TRANSLATE(0,80,0);
							pie_MatRotY(DEG(-((SDWORD)psStructure->turretRotation)));
							iV_TRANSLATE(0,0,-20);
						}
						else//baba bunker
						{
							iV_TRANSLATE(0,10,0);
							pie_MatRotY(DEG(-((SDWORD)psStructure->turretRotation)));
							iV_TRANSLATE(0,0,-40);
						}
						iV_MatrixRotateX(DEG(psStructure->turretPitch));
						//and draw the muzzle flash
						//animate for the duration of the flash only
						//assume no clan colours formuzzle effects
						if ((flashImd->numFrames == 0) || (flashImd->animInterval <= 0))//no anim so display one frame for a fixed time
						{
							if (gameTime < (psStructure->asWeaps->lastFired + BASE_MUZZLE_FLASH_DURATION))
							{
								pie_Draw3DShape(flashImd, 0, 0, brightness, specular, 0, 0);//muzzle flash
							}
						}
						else
						{
							frame = (gameTime - psStructure->asWeaps->lastFired)/flashImd->animInterval;
							if (frame < flashImd->numFrames)
							{
								pie_Draw3DShape(flashImd, 0, 0, brightness, specular, 0, 0);//muzzle flash
							}
						}
						iV_MatrixEnd();
					}
				}
			}
		}
		imd = psStructure->sDisplay.imd;
		temp = imd->points;

		{
			pie_SETUP_ROTATE_PROJECT;
			pie_ROTATE_PROJECT(0,0,0,sX,sY);
			psStructure->sDisplay.screenX = sX;
			psStructure->sDisplay.screenY = sY;
		}
		iV_MatrixEnd();

		targetAdd((BASE_OBJECT*)psStructure);
	}
}


//---

/*draw the delivery points */
void	renderDeliveryPoint(FLAG_POSITION *psPosition)
{
	iVector			dv;
	SDWORD			x, y, r;
	iVector			*temp;
//	SDWORD			centreX, centreZ;
	SDWORD			buildingBrightness, specular;
	//store the frame number for when deciding what has been clicked on
	psPosition->frameNumber = currentGameFrame;

	dv.x = (psPosition->coords.x - player.p.x) - terrainMidX*TILE_UNITS;
	dv.z = terrainMidY*TILE_UNITS - (psPosition->coords.y - player.p.z);
	dv.y = psPosition->coords.z;  

	// world x,y,z coord of deliv point ... this is needed for the BSP code
	//SetBSPObjectPos(posX,dv.y,posY);	
	
	/* Push the indentity matrix */
	iV_MatrixBegin();

	iV_TRANSLATE(dv.x,dv.y,dv.z);

	/* Get the x,z translation components */
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Translate */
	iV_TRANSLATE(rx,0,-rz);

	//quick check for invalid data
	//ASSERT((psPosition->factoryType < NUM_FACTORY_TYPES AND 
    ASSERT((psPosition->factoryType < NUM_FLAG_TYPES AND 
		psPosition->factoryInc < MAX_FACTORY, "Invalid assembly point"));

	if(!psPosition->selected)
	{
		temp = pAssemblyPointIMDs[psPosition->factoryType][psPosition->factoryInc]->points;
		flattenImd(pAssemblyPointIMDs[psPosition->factoryType][psPosition->factoryInc],
			psPosition->coords.x, psPosition->coords.y,0);
	}

	scaleMatrix(50);				//they are all big now so make this one smaller too

//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

	buildingBrightness = pie_MAX_BRIGHT_LEVEL;

	buildingBrightness = lightDoFogAndIllumination((UBYTE)buildingBrightness,
		getCentreX()-psPosition->coords.x,getCentreZ()-psPosition->coords.y, (UDWORD*)&specular);

//	pie_Draw3DShape(pAssemblyPointIMDs[psPosition->factoryInc], 0, 0, 
	//	buildingBrightness, 0, pie_TRANSLUCENT | pie_NO_BILINEAR, EFFECT_DELIVERY_POINT_TRANSPARENCY);
	pie_Draw3DShape(pAssemblyPointIMDs[psPosition->factoryType][psPosition->factoryInc], 
		0, 0, buildingBrightness, specular, pie_NO_BILINEAR, 0);

	if(!psPosition->selected)
	{
		pAssemblyPointIMDs[psPosition->factoryType][psPosition->factoryInc]->points = temp;
	}

	//get the screen coords for the DP
	calcFlagPosScreenCoords(&x, &y, &r);
	psPosition->screenX = x;
	psPosition->screenY = y;
	psPosition->screenR = r;

	iV_MatrixEnd();
}

BOOL	renderWallSection(STRUCTURE *psStructure)
{
	SDWORD			structX,structY;	
//	SDWORD			centreX,centreZ;
	UDWORD			brightness;
	iIMDShape		*imd;
	SDWORD			rotation;
	iVector			dv;
	UDWORD			i;
	UDWORD			centreHeight;
	UDWORD			pointHeight;
	iVector			*temp;
	SDWORD			shift;
	UDWORD			buildingBrightness, specular;
	SDWORD			sX,sY;
	SDWORD			brightVar;

         
	if(psStructure->visible[selectedPlayer] OR godMode OR demoGetStatus())
	{
		psStructure->sDisplay.frameNumber = currentGameFrame;
		/* Get it's x and y coordinates so we don't have to deref. struct later */
		structX = psStructure->x;
		structY = psStructure->y;
 //		centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
 //		centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
		buildingBrightness = 200 - (100-PERCENT( psStructure->body , structureBody(psStructure)));

		if(psStructure->selected)
		{
			if(!gamePaused())
			{
				brightVar = getStaticTimeValueRange(990,110);
				if(brightVar>55) brightVar = 110-brightVar;
			}
			else
			{
				brightVar = 55;
			}


			buildingBrightness = 200 + brightVar;
		}

		if(godMode OR demoGetStatus())
		{
			/* NOP */
		}
		else if(getRevealStatus())
		{
			buildingBrightness = avGetObjLightLevel((BASE_OBJECT*)psStructure,buildingBrightness);
		}

		brightness = lightDoFogAndIllumination((UBYTE)buildingBrightness,getCentreX()-structX,getCentreZ()-structY, &specular);

	//	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,centreX-structX,centreZ-structY);
		/* 
		Right, now the tricky bit, we need to bugger about with the coordinates of the imd to make it
		fit tightly to the ground and to neighbours. 
		*/
		imd = psStructure->pStructureType->pBaseIMD;
		if(imd!=NULL)
		{
			// Get a copy of the points 
			memcpy(alteredPoints,imd->points,imd->npoints*sizeof(iVector));
			// Get the height of the centre point for reference 
			centreHeight = map_Height(structX,structY);
		   //	 Now we got through the shape looking for vertices on the edge 
			for(i=0; i<(UDWORD)imd->npoints; i++)
			{
				pointHeight = map_Height(structX+alteredPoints[i].x,structY-alteredPoints[i].z);
				shift = centreHeight - pointHeight;
				alteredPoints[i].y -= shift;
			}
		}
		/* Establish where it is in the world */
		dv.x = (structX - player.p.x) - terrainMidX*TILE_UNITS;
		dv.z = terrainMidY*TILE_UNITS - (structY - player.p.z);
		dv.y = map_Height(structX, structY);  

		SetBSPObjectPos(structX,dv.y,structY);	// world x,y,z coord of structure ... this is needed for the BSP code

		/* Push the indentity matrix */
		iV_MatrixBegin();

		/* Translate */
		iV_TRANSLATE(dv.x,dv.y,dv.z);

		/* Get the x,z translation components */
		rx = player.p.x & (TILE_UNITS-1);
		rz = player.p.z & (TILE_UNITS-1);

		/* Translate */
		iV_TRANSLATE(rx,0,-rz);
			
		rotation = DEG(psStructure->direction);
		iV_MatrixRotateY(-rotation);
	//	objectShimmy((BASE_OBJECT*)psStructure);
		if(imd!=NULL)
		{
			// Make the imd pointer to the vertex list point to ours 
			temp = imd->points;
			imd->points = alteredPoints;
			// Actually render it 
			if (pie_Hardware())
			{
				pie_Draw3DShape(imd, 0, getPlayerColour(psStructure->player), brightness, specular, 0, 0);
			}
			else
			{
	 			pie_Draw3DShape(imd, 0,getPlayerColour( psStructure->player), brightness, specular, 0, 0);
			}
			imd->points = temp;
		}

		imd = psStructure->sDisplay.imd;
		temp = imd->points;

		flattenImd(imd,structX,structY,psStructure->direction);

		/* Actually render it */
		if ( (psStructure->status == SS_BEING_BUILT ) OR
			 (psStructure->status == SS_BEING_DEMOLISHED ) OR
			 (psStructure->status == SS_BEING_BUILT AND psStructure->pStructureType->type == REF_RESOURCE_EXTRACTOR) )
		{
			pie_Draw3DShape(psStructure->sDisplay.imd, 0,getPlayerColour(psStructure->player) , 
				brightness, specular, pie_HEIGHT_SCALED, 
				(SDWORD)(structHeightScale(psStructure) * pie_RAISE_SCALE));
		}
		else if(psStructure->status == SS_BUILT)
		{
			pie_Draw3DShape(imd, 0, getPlayerColour(psStructure->player), brightness, specular, 0, 0);
			/*
			if (pie_Hardware())
			{
  				pie_Draw3DShape(imd, 0,getPlayerColour( psStructure->player), brightness, specular, 0,0);//pie_TRANSLUCENT, psStructure->visible[selectedPlayer]);
			}
			else
			{
  				pie_Draw3DShape(imd, 0, getPlayerColour(psStructure->player), brightness, specular, 0, 0);
			}
			*/
		}
		
		imd->points = temp;
		{
			// Macro definition declares variables so this needs to be bracketed and indented.
			pie_SETUP_ROTATE_PROJECT;
			pie_ROTATE_PROJECT(0,0,0,sX,sY);
			psStructure->sDisplay.screenX = sX;
			psStructure->sDisplay.screenY = sY;
		}
		iV_MatrixEnd();

		return(TRUE);
	}
}

/* renderShadow: draws shadow under droid */
void renderShadow( DROID *psDroid, iIMDShape *psShadowIMD )
{
	iVector			dv;
	iVector			*pVecTemp;
	SDWORD			shadowScale;
	UDWORD brightness, specular;
//	SDWORD centreX, centreZ;
  

	dv.x = (psDroid->x - player.p.x) - terrainMidX*TILE_UNITS;
	if(psDroid->droidType == DROID_TRANSPORTER)
	{
		dv.x -= bobTransporterHeight()/2;
	}
	dv.z = terrainMidY*TILE_UNITS - (psDroid->y - player.p.z);
	dv.y = map_Height(psDroid->x, psDroid->y);  

	/* Push the indentity matrix */
	iV_MatrixBegin();

	iV_TRANSLATE(dv.x,dv.y,dv.z);

	/* Get the x,z translation components */
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Translate */
	iV_TRANSLATE(rx,0,-rz);

	if(psDroid->droidType == DROID_TRANSPORTER)
	{
		iV_MatrixRotateY( DEG(-psDroid->direction));
	}

	pVecTemp = psShadowIMD->points;
	if(psDroid->droidType == DROID_TRANSPORTER)
	{
		flattenImd( psShadowIMD, psDroid->x, psDroid->y, 0);
		shadowScale = 100-(psDroid->z/100);
		if(shadowScale < 50) shadowScale = 50;
	}
	else
	{
	   	pie_MatRotY(DEG(-psDroid->direction));
		pie_MatRotX(DEG(psDroid->pitch));
		pie_MatRotZ(DEG(psDroid->roll));
	}
   
	// set up lighting
//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

	brightness = (UDWORD)lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX()-psDroid->x,getCentreZ()-psDroid->y, &specular);

	pie_Draw3DShape( psShadowIMD, 0, 0, brightness, specular, pie_TRANSLUCENT, 128);
	psShadowIMD->points = pVecTemp;

	iV_MatrixEnd();
}

/* Draw the droids */
void renderDroid( DROID *psDroid )
{
//PROPULSION_STATS	*psPropStats;
//	ASSERT((psDroid->x != 0 && psDroid->y != 0,
//		"moveUpdateUnit: unit at (0,0)"));


   //	psPropStats = asPropulsionStats + psDroid->asBits[COMP_PROPULSION].nStat;
   //	ASSERT( (PTRVALID(psPropStats, sizeof(PROPULSION_STATS)),
	//		"moveUpdateDroid: invalid propulsion stats pointer") );

	/*
	if ( psPropStats->propulsionType == LIFT )
	{
		if ( psDroid->droidType != DROID_TRANSPORTER )
		{
			renderShadow( psDroid, getImdFromIndex(MI_SHADOW) );
		}
	
	}
	*/
	displayComponentObject( (BASE_OBJECT *) psDroid);
	targetAdd((BASE_OBJECT*)psDroid);
	return;
} // end Fn


/* Draws the strobing 3D drag box that is used for multiple selection */
void	drawDragBox( void )
{
	if(dragBox3D.status == DRAG_DRAGGING AND buildState == BUILD3D_NONE)
	{
		if(gameTime - dragBox3D.lastTime > BOX_PULSE_SPEED)
		{
		 	dragBox3D.boxColourIndex++;
			if(dragBox3D.boxColourIndex>=BOX_PULSE_SIZE)
			{
				dragBox3D.boxColourIndex = 0;
			}
			dragBox3D.lastTime = gameTime;
		}
		pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_OFF);
	   	iV_Box(dragBox3D.x1+dragBox3D.boxColourIndex/2,dragBox3D.y1+dragBox3D.boxColourIndex/2,
				mX-dragBox3D.boxColourIndex/2,mY-dragBox3D.boxColourIndex/2,
				boxPulseColours[dragBox3D.boxColourIndex]);
		if (war_GetTranslucent())
		{
		//	pie_doWeirdBoxFX(dragBox3D.x1+dragBox3D.boxColourIndex/2+1,dragBox3D.y1+dragBox3D.boxColourIndex/2+1,
  		//			(mX-dragBox3D.boxColourIndex/2),(mY-dragBox3D.boxColourIndex/2));
  			pie_UniTransBoxFill(dragBox3D.x1+dragBox3D.boxColourIndex/2+1,dragBox3D.y1+dragBox3D.boxColourIndex/2+1,
  					(mX-dragBox3D.boxColourIndex/2)-1,(mY-dragBox3D.boxColourIndex/2)-1,
  					0x00ffffff,16);
		}
		pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
	}
}


// display reload bars for structures and droids
void drawWeaponReloadBar(BASE_OBJECT *psObj, WEAPON *psWeap)
{
	WEAPON_STATS	*psStats;
	BOOL			bSalvo;
	UDWORD			firingStage, interval, damLevel;
	SDWORD			scrX,scrY, scrR, scale;
	STRUCTURE		*psStruct;

	/* ****************/
	// display unit resistance instead of reload!
	FRACT mulH;
	DROID *psDroid;

	if (ctrlShiftDown() && (psObj->type == OBJ_DROID))
    {
		psDroid = (DROID*)psObj;
		scrX = psObj->sDisplay.screenX;
		scrY = psObj->sDisplay.screenY;
		scrR = psObj->sDisplay.screenR;
		scrY += scrR + 2;

		if (psDroid->resistance)
        {
			mulH = MAKEFRACT(psDroid->resistance) / MAKEFRACT(droidResistance(psDroid));
        }
        else
        {
			mulH = 100;
        }
		firingStage = MAKEINT(mulH);
		firingStage = ((((2*scrR)*10000)/100)*firingStage)/10000;			
		if(firingStage >= (UDWORD)(2*scrR))
		{
			firingStage = (2*scrR) - 1;
		}
		if (pie_Hardware())
		{
			pie_BoxFill(scrX - scrR-1, 6+scrY + 0, scrX - scrR +(2*scrR),    6+scrY+3, 0x00020202);
			pie_BoxFill(scrX - scrR,   6+scrY + 1, scrX - scrR +firingStage, 6+scrY+2, 0x00ffffff);
		}
		else
		{
			pie_BoxFillIndex(scrX - scrR-1, 6+scrY + 0, scrX - scrR +(2*scrR),   6+scrY+4, 1);
			pie_BoxFillIndex(scrX - scrR,   6+scrY + 1, scrX - scrR +firingStage,6+scrY+3, 255);
		}
		return;
	}
	/* ******** ********/


	if (psWeap->nStat == 0)
	{
		// no weapon
		return;
	}

	psStats = asWeaponStats + psWeap->nStat;

	/* Justifiable only when greater than a one second reload 
		or intra salvo time  */
	bSalvo = FALSE;
	if(psStats->numRounds > 1)
	{
		bSalvo = TRUE;
	}
	if( (bSalvo AND (psStats->reloadTime > GAME_TICKS_PER_SEC)) OR
		(psStats->firePause > GAME_TICKS_PER_SEC) OR
		((psObj->type == OBJ_DROID) && vtolDroid((DROID *)psObj)) )
	{
		if ((psObj->type == OBJ_DROID) &&
			vtolDroid((DROID *)psObj))
		{
			//deal with VTOLs
			firingStage = getNumAttackRuns((DROID *)psObj) - ((DROID *)psObj)->sMove.iAttackRuns;
    		//compare with max value
			interval = getNumAttackRuns((DROID *)psObj);
		}
		else
		{
			firingStage = gameTime - psWeap->lastFired;
			if (bSalvo)
			{
				interval = psStats->reloadTime;
			}
			else
			{
				interval = weaponFirePause(psStats, psObj->player);
			}

            //we haven't calculated the damLevel yet! DOH!
			/*if (damLevel < HEAVY_DAMAGE_LEVEL)
			{
				interval += interval;
			}*/
		}

		scrX = psObj->sDisplay.screenX;
		scrY = psObj->sDisplay.screenY;
		scrR = psObj->sDisplay.screenR;
		switch (psObj->type)
		{
		case OBJ_DROID:
			damLevel = PERCENT(((DROID *)psObj)->body, ((DROID *)psObj)->originalBody);
			scrY += scrR + 2;
			break;
		case OBJ_STRUCTURE:
			psStruct = (STRUCTURE *)psObj;
			damLevel = PERCENT(psStruct->body, structureBody(psStruct));
			scale = max(psStruct->pStructureType->baseWidth,psStruct->pStructureType->baseBreadth);
			scrY += scale * 10 - 1;
			scrR = scale * 20;
			break;
		default:
			ASSERT((FALSE, "drawWeaponReloadBars: invalid object type"));
			damLevel = 100;
			break;
		}

        //now we know what it is!!
        if (damLevel < HEAVY_DAMAGE_LEVEL)
		{
			interval += interval;
		}

		if(firingStage < interval)
		{
			/* Get a percentage */
			firingStage = PERCENT(firingStage,interval);

			/* Scale it into an appropriate range */
			firingStage = ((((2*scrR)*10000)/100)*firingStage)/10000;			
			if(firingStage >= (UDWORD)(2*scrR))
			{
				firingStage = (2*scrR) - 1;
			}
			if (pie_Hardware())
			{
				/* Power bars */
				pie_BoxFill(scrX - scrR-1, 6+scrY + 0, scrX - scrR +(2*scrR),    6+scrY+3, 0x00020202);
				pie_BoxFill(scrX - scrR,   6+scrY + 1, scrX - scrR +firingStage, 6+scrY+2, 0x00ffffff);
			}
			else
			{
				/* Power bars */
				pie_BoxFillIndex(scrX - scrR-1, 6+scrY + 0, scrX - scrR +(2*scrR),   6+scrY+4, 1);
				pie_BoxFillIndex(scrX - scrR,   6+scrY + 1, scrX - scrR +firingStage,6+scrY+3, 255);
			}
		}
	}
}


void	drawStructureSelections( void )
{
STRUCTURE	*psStruct;
SDWORD		scrX,scrY,scrR;
UDWORD		longPowerCol;
UBYTE		powerCol;
UDWORD		health,width;
UDWORD		scale;
UDWORD		i;
BASE_OBJECT	*psClickedOn;
BOOL		bMouseOverStructure = FALSE;
BOOL		bMouseOverOwnStructure = FALSE;
FRACT		mulH;

	psClickedOn = mouseTarget();
	if(psClickedOn!=NULL AND psClickedOn->type == OBJ_STRUCTURE)
	{
		bMouseOverStructure = TRUE;
		if(psClickedOn->player == selectedPlayer)
		{
 			bMouseOverOwnStructure = TRUE;
		}
	}
	pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
	pie_SetFogStatus(FALSE);
	/* Go thru' all the buildings */
	for(psStruct = apsStructLists[selectedPlayer]; psStruct; psStruct = psStruct->psNext)
	{
		if(clipXY(psStruct->x,psStruct->y))
		{
			/* If it's selected */
			if( (psStruct->selected) 
				OR (bMouseOverOwnStructure AND (psStruct==(STRUCTURE*)psClickedOn) 
										   AND (((STRUCTURE*)psClickedOn)->status==SS_BUILT) )
										   /* If it was clipped - reject it */
										   AND psStruct->sDisplay.frameNumber == currentGameFrame)
			{
			//----
				scale = max(psStruct->pStructureType->baseWidth,psStruct->pStructureType->baseBreadth);
				width = scale*20;
				scrX = psStruct->sDisplay.screenX;
				scrY = psStruct->sDisplay.screenY + (scale*10);
				scrR = width;
				//health = PERCENT(psStruct->body, psStruct->baseBodyPoints);
                if (ctrlShiftDown())
                {
                    //show resistance values if CTRL/SHIFT depressed
                    UDWORD  resistance = structureResistance(
                        psStruct->pStructureType, psStruct->player);
                    if (resistance)
                    {
                        health = PERCENT(psStruct->resistance, resistance);
                    }
                    else
                    {
                        health = 100;
                    }
                }
                else
                {
                    //show body points
				    health = PERCENT(psStruct->body, structureBody(psStruct));
                }
				if(health>100) health =100;

				if(pie_Hardware())
				{
					if(health>REPAIRLEV_HIGH) longPowerCol = 0x0000ff00;//green
					else if(health>=REPAIRLEV_LOW) longPowerCol = 0x00ffff00;//yellow
					else longPowerCol = 0x00ff0000;//red
				}
				else
				{
					if(health>REPAIRLEV_HIGH) powerCol = COL_GREEN;
					else if(health>REPAIRLEV_LOW) powerCol = COL_YELLOW;
					else powerCol = COL_RED;
				}
				mulH = MAKEFRACT(health)/100;
				mulH*=MAKEFRACT(width);
				health = MAKEINT(mulH);
//				health = (((width*10000)/100)*health)/10000;
				if(health>width) health = width;
				health*=2;
				if(pie_Hardware())
				{
					pie_BoxFill(scrX-scrR-1, scrY-1, scrX+scrR+1, scrY+2, 0x00020202);
					pie_BoxFill(scrX-scrR, scrY, scrX-scrR+health, scrY+1, longPowerCol);
				}
				else
				{
					pie_BoxFillIndex(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+6,1);
					pie_BoxFillIndex(scrX - scrR,scrY + scrR+3,scrX - scrR+health,scrY+scrR+5,powerCol);
				}

				drawWeaponReloadBar((BASE_OBJECT *)psStruct, psStruct->asWeaps);
			}
			else
			{
				if(psStruct->status == SS_BEING_BUILT AND psStruct->sDisplay.frameNumber == currentGameFrame)
				{
					scale = max(psStruct->pStructureType->baseWidth,psStruct->pStructureType->baseBreadth);
					width = scale*20;
					scrX = psStruct->sDisplay.screenX;
					scrY = psStruct->sDisplay.screenY + (scale*10);
					scrR = width;
	//				health = PERCENT(psStruct->body, psStruct->baseBodyPoints);
		   			health =  PERCENT(psStruct->currentBuildPts , 
						psStruct->pStructureType->buildPoints);
					if(health>=100) health = 100;	// belt and braces
						if(pie_GetRenderEngine() == ENGINE_GLIDE)
						{
							longPowerCol = 0x00ffff00;
						}
						else
						{
							powerCol = COL_YELLOW;
						}
						mulH = MAKEFRACT(health)/100;
						mulH*=MAKEFRACT(width);
						health = MAKEINT(mulH);
//						health = (((width*10000)/100)*health)/10000;
						if(health>width) health = width;
						health*=2;
						if(pie_GetRenderEngine() == ENGINE_GLIDE)
						{
							pie_BoxFill(scrX-scrR-1, scrY-1, scrX+scrR+1, scrY+2, 0x00020202);
							pie_BoxFill(scrX-scrR-1, scrY, scrX-scrR+health, scrY+1, longPowerCol);
						}
						else
						{
//							pie_BoxFillIndex(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+6,1);
//							pie_BoxFillIndex(scrX - scrR,scrY + scrR+3,scrX - scrR+health,scrY+scrR+5,powerCol);
							pie_BoxFillIndex(scrX - scrR-1,scrY-1,scrX + scrR+1,scrY+2,1);
							pie_BoxFillIndex(scrX - scrR,scrY ,scrX - scrR+health,scrY+1,powerCol);
						}
					}
				}
					//----
			}
	}

	for(i=0; i<MAX_PLAYERS; i++)
	{
		/* Go thru' all the buildings */
		for(psStruct = apsStructLists[i]; psStruct; psStruct = psStruct->psNext)
		{
			if(i!=selectedPlayer)		// only see enemy buildings being targetted, not yours!
			{
				if(clipXY(psStruct->x,psStruct->y))
				{
					/* If it's targetted and on-screen */
					if(psStruct->targetted) 
					{
						if(psStruct->sDisplay.frameNumber == currentGameFrame)

						{
							//health = PERCENT(psStruct->body, psStruct->baseBodyPoints);
	//						health = PERCENT(psStruct->body, structureBody(psStruct));
							psStruct->targetted = 0;
							scrX = psStruct->sDisplay.screenX;
							scrY = psStruct->sDisplay.screenY - (psStruct->sDisplay.imd->ymax/4);
							iV_DrawTransImage(IntImages,getTargettingGfx(),scrX,scrY);

							/*
							scrR = (gameTime%1000)/50;
							if(health>REPAIRLEV_HIGH) powerCol = COL_GREEN;
							else if(health>REPAIRLEV_LOW) powerCol = COL_YELLOW;
							else powerCol = COL_RED;

							iV_Line(scrX-scrR,scrY,scrX+scrR,scrY,255);//powerCol);
							iV_Line(scrX,scrY-scrR,scrX,scrY+scrR,255);//powerCol);
							*/
						}
					}
				}
			}
		}
	}

	if(bMouseOverStructure AND !bMouseOverOwnStructure)
	{
		if(mouseDown(MOUSE_RMB))
		{
			psStruct = (STRUCTURE*)psClickedOn;
			if(psStruct->status==SS_BUILT)
			{
			//----
				scale = max(psStruct->pStructureType->baseWidth,psStruct->pStructureType->baseBreadth);
				width = scale*20;
				scrX = psStruct->sDisplay.screenX;
				scrY = psStruct->sDisplay.screenY + (scale*10);
				scrR = width;
				//health = PERCENT(psStruct->body, psStruct->baseBodyPoints);
                if (ctrlShiftDown())
                {
                    //show resistance values if CTRL/SHIFT depressed
                    UDWORD  resistance = structureResistance(
                        psStruct->pStructureType, psStruct->player);
                    if (resistance)
                    {
                        health = PERCENT(psStruct->resistance, resistance);
                    }
                    else
                    {
                        health = 100;
                    }
                }
                else
                {
                    //show body points
    				health = PERCENT(psStruct->body, structureBody(psStruct));
                }
				if(pie_Hardware())
				{
					if(health>REPAIRLEV_HIGH) longPowerCol = 0x0000ff00;//green
					else if(health>REPAIRLEV_LOW) longPowerCol = 0x00ffff00;//yellow
					else longPowerCol = 0x00ff0000;//red
				}
				else
				{
					if(health>REPAIRLEV_HIGH) powerCol = COL_GREEN;
					else if(health>REPAIRLEV_LOW) powerCol = COL_YELLOW;
					else powerCol = COL_RED;
				}
				health = (((width*10000)/100)*health)/10000;
				health*=2;
				if(pie_Hardware())
				{
					pie_BoxFill(scrX-scrR-1, scrY-1, scrX+scrR+1, scrY+2, 0x00020202);
					pie_BoxFill(scrX-scrR, scrY, scrX-scrR+health, scrY+1, longPowerCol);
				}
				else
				{
//					pie_BoxFillIndex(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+6,1);
//					pie_BoxFillIndex(scrX - scrR,scrY + scrR+3,scrX - scrR+health,scrY+scrR+5,powerCol);
					pie_BoxFillIndex(scrX - scrR-1,scrY-1,scrX + scrR+1,scrY+2,1);
					pie_BoxFillIndex(scrX - scrR,scrY,scrX - scrR+health,scrY+1,powerCol);

				}


			}
			else if(psStruct->status == SS_BEING_BUILT)
			{
				scale = max(psStruct->pStructureType->baseWidth,psStruct->pStructureType->baseBreadth);
				width = scale*20;
				scrX = psStruct->sDisplay.screenX;
				scrY = psStruct->sDisplay.screenY + (scale*10);
				scrR = width;
//				health = PERCENT(psStruct->body, psStruct->baseBodyPoints);
			   	health =  PERCENT(psStruct->currentBuildPts , psStruct->pStructureType->buildPoints);
				if(pie_GetRenderEngine() == ENGINE_GLIDE)
				{
					longPowerCol = 0x0000ff00;
				}
				else
				{
					powerCol = COL_GREEN;
				}
				health = (((width*10000)/100)*health)/10000;
				health*=2;
				if(pie_GetRenderEngine() == ENGINE_GLIDE)
				{
					pie_BoxFill(scrX-scrR-1, scrY-1, scrX+scrR+1, scrY+2, 0x00020202);
					pie_BoxFill(scrX-scrR-1, scrY, scrX-scrR+health, scrY+1, longPowerCol);
				}
				else
				{
//					pie_BoxFillIndex(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+6,1);
//					pie_BoxFillIndex(scrX - scrR,scrY + scrR+3,scrX - scrR+health,scrY+scrR+5,powerCol);
					pie_BoxFillIndex(scrX - scrR-1,scrY-1,scrX + scrR+1,scrY+2,1);
					pie_BoxFillIndex(scrX - scrR-1,scrY,scrX - scrR+health,scrY+1,powerCol);
				}
			}
			//----
		}
	}

	pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
}

UDWORD	getTargettingGfx( void )
{
UDWORD	index;

   	index = getTimeValueRange(1000,10);

	switch(index)
	{
	case	0:
	case	1:
	case	2:
		return(IMAGE_TARGET1+index);
		break;
	default:
		if(index & 0x01)
		{
			return(IMAGE_TARGET4);
		}
		else
		{
			return(IMAGE_TARGET5);
		}
		break;
	}
}

BOOL	eitherSelected(DROID *psDroid)
{
BOOL			retVal;
BASE_OBJECT		*psObj;

	retVal = FALSE;
	if(psDroid->selected)
	{
		retVal = TRUE;
	}

	if(psDroid->psGroup)
	{
		if(psDroid->psGroup->psCommander)
		{
			if(psDroid->psGroup->psCommander->selected)
			{
				retVal = TRUE;
			}
		}
	}

	if (orderStateObj(psDroid, DORDER_FIRESUPPORT, &psObj))
	{
		if (psObj != NULL && psObj->selected)
		{
			retVal = TRUE;
		}
	}
	return(retVal);
}

void	drawDroidPowerBar(DROID *psDroid)
{
	return;
}

void	drawDroidRanking(DROID *psDroid)
{
	return;
}

void	drawDroidReloadBar(DROID *psDroid)
{
	return;
}

BOOL	doHighlight( DROID *psDroid )
{
	return(TRUE);
}

void	showDroidSelection( DROID *psDroid )
{
	/* First, let's see if we need to - Get out if it's not selected and no appropriate to highlight */
	if(!psDroid->selected AND !doHighlight(psDroid))
	{
		/* No need - so get out now */
		return;
	}

	/* Right, we're going to process this one, so first do power bars */
	drawDroidPowerBar(psDroid);

	/* Now, display it's rank */
	drawDroidRanking(psDroid);

	/* And the reload bars... */
	drawDroidReloadBar(psDroid);
}

void	drawDeliveryPointSelection(void)
{
FLAG_POSITION	*psDelivPoint;
SDWORD			scrX,scrY,scrR;

	//draw the selected Delivery Point if any
	for(psDelivPoint = apsFlagPosLists[selectedPlayer]; psDelivPoint; psDelivPoint = 
		psDelivPoint->psNext)
	{
		if(psDelivPoint->selected AND psDelivPoint->frameNumber == currentGameFrame)
		{
			scrX = psDelivPoint->screenX;
			scrY = psDelivPoint->screenY;
			scrR = psDelivPoint->screenR;
			if (pie_Hardware())
			{
				/* Three DFX clips properly right now - not sure if software does */
				if((scrX+scrR)>0 AND (scrY+scrR)>0 AND (scrX-scrR)<DISP_WIDTH 
					AND (scrY-scrR)<DISP_HEIGHT)
				{
					iV_Box(scrX - scrR, scrY - scrR, scrX + scrR, scrY + scrR, 110);
				}
			}
			else
			{
				if( ((SDWORD)(scrX - scrR) > 0) AND (scrX + scrR < DISP_WIDTH) AND 
						((SDWORD)(scrY - scrR) > 0) AND (scrY + scrR < DISP_HEIGHT) )
				{
					iV_Box(scrX - scrR, scrY - scrR, scrX + scrR, scrY + scrR, 110);
				}
			}
		}
	}
}

void	drawDroidSelections( void )
{
SDWORD			scrX,scrY,scrR;
DROID			*psDroid;
UDWORD			damage;
UBYTE			powerCol;
UDWORD			longPowerCol;
UBYTE			boxCol;
UDWORD			longBoxCol;
BASE_OBJECT		*psClickedOn;
BOOL			bMouseOverDroid = FALSE;
BOOL			bMouseOverOwnDroid = FALSE;
BOOL			bBeingTracked;
UDWORD			i,index;
FEATURE			*psFeature;
FRACT			mulH;

	psClickedOn = mouseTarget();
	if(psClickedOn!=NULL AND psClickedOn->type == OBJ_DROID)
	{
		bMouseOverDroid = TRUE;
		if(psClickedOn->player == selectedPlayer AND !psClickedOn->selected)
		{
			bMouseOverOwnDroid = TRUE;
		}
	}

	switch(barMode)
	{
	case BAR_FULL:
		bEnergyBars = TRUE;
		bTinyBars = FALSE;
		break;
	case BAR_BASIC:
		bEnergyBars = FALSE;
		bTinyBars = FALSE;
		break;
	case BAR_DOT:
		bEnergyBars = FALSE;
		bTinyBars = TRUE;
		break;
	default:
		ASSERT((FALSE,"Invalid energy bar display value"));
		break;
	}

	pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);   
	pie_SetFogStatus(FALSE);                             
	for(psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
	{
		bBeingTracked = FALSE;
		/* If it's selected and on screen or it's the one the mouse is over OR*/
		// ABSOLUTELY MAD LOGICAL EXPRESSION!!! :-)
		if( ( eitherSelected(psDroid) AND psDroid->sDisplay.frameNumber == currentGameFrame) OR
			( bMouseOverOwnDroid AND (psDroid == (DROID*)psClickedOn)) OR 
			( droidUnderRepair(psDroid) AND psDroid->sDisplay.frameNumber == currentGameFrame) )	
		{
//show resistance values if CTRL/SHIFT depressed (now done in reload bar)
//            if (ctrlShiftDown())
//          {
//                if (psDroid->resistance)
//                {
//                    damage = PERCENT(psDroid->resistance, droidResistance(psDroid));
//                }
//                else
//                {
//                    damage = 100;
//                }
//            }
//            else
//            {
			    damage = PERCENT(psDroid->body,psDroid->originalBody);
//            }

			if(pie_Hardware())
			{
			 	if(damage>REPAIRLEV_HIGH) longPowerCol = 0x0000ff00;//green
				else if(damage>REPAIRLEV_LOW) longPowerCol = 0x00ffff00;//yellow
				else longPowerCol = 0x00ff0000;//red
			}
			else
			{
				if(damage>REPAIRLEV_HIGH) powerCol = defaultColours.green;
				else if(damage>REPAIRLEV_LOW) powerCol = defaultColours.yellow;
				else powerCol = defaultColours.red;
			}
//show resistance values if CTRL/SHIFT depressed(now done in reload bar)
//            if (ctrlShiftDown())
//          {
//                if (psDroid->resistance)
//                {
//                    mulH = MAKEFRACT(psDroid->resistance) / MAKEFRACT(droidResistance(psDroid));
//                }
//                else
//                {
//                    mulH = 100;
//                }
//            }
//            else
//            {
			    mulH = MAKEFRACT(psDroid->body) / MAKEFRACT(psDroid->originalBody);
//            }
			damage = MAKEINT(mulH*MAKEFRACT(psDroid->sDisplay.screenR));// (((psDroid->sDisplay.screenR*10000)/100)*damage)/10000;
			if(damage>psDroid->sDisplay.screenR) damage = psDroid->sDisplay.screenR;

			damage *=2;
			scrX = psDroid->sDisplay.screenX;
			scrY = psDroid->sDisplay.screenY;
			scrR = psDroid->sDisplay.screenR;

			/* Yeah, yeah yeah - hardcoded palette entries - need to change to #defined colour names */
			/* Three DFX clips properly right now - not sure if software does */
//			if((scrX+scrR)>0 AND (scrY+scrR)>0 AND (scrX-scrR)<DISP_WIDTH
//				AND (scrY-scrR)<DISP_HEIGHT)
			{
				if(!driveModeActive() || driveIsDriven(psDroid)) {
					boxCol = defaultColours.white;
					longBoxCol = 0x00ffffff;
				} else {
					boxCol = defaultColours.green;
					longBoxCol = 0x0000ff00;
				}

				if(psDroid->selected)
				{

					/* Selection Lines */
					if (pie_Hardware())
					{
						if(bEnergyBars)
						{
						 	pie_BoxFill(scrX-scrR, scrY+scrR, scrX-scrR+1, scrY+scrR - 7, longBoxCol);
							pie_BoxFill(scrX-scrR, scrY+scrR, scrX-scrR+7, scrY+scrR+1,longBoxCol);
							pie_BoxFill(scrX+scrR-7, scrY+scrR, scrX+scrR, scrY+scrR+1,longBoxCol);
							pie_BoxFill(scrX+scrR, scrY+scrR+1, scrX+scrR+1, scrY+scrR-7,longBoxCol);
						}
						else
						{
							if(bTinyBars)
							{
								pie_BoxFill(scrX-scrR-3, scrY-3, scrX-scrR+3, scrY+3, 0x00010101);
								pie_BoxFill(scrX-scrR-2, scrY-2, scrX-scrR+2, scrY+2, longPowerCol);
							}
							else
							{
								pie_BoxFill(scrX-scrR, scrY+scrR, scrX-scrR+1, scrY+scrR - 7, longPowerCol);
								pie_BoxFill(scrX-scrR, scrY+scrR, scrX-scrR+7, scrY+scrR+1,longPowerCol);
								pie_BoxFill(scrX+scrR-7, scrY+scrR, scrX+scrR, scrY+scrR+1,longPowerCol);
								pie_BoxFill(scrX+scrR, scrY+scrR+1, scrX+scrR+1, scrY+scrR-7,longPowerCol);
							}
						}
					}
					else
					{
						if(bEnergyBars)
						{
						 	pie_BoxFillIndex(scrX-scrR, scrY+scrR-7, scrX-scrR, scrY+scrR, boxCol);
							pie_BoxFillIndex(scrX-scrR, scrY+scrR, scrX-scrR+7, scrY+scrR, boxCol);
							pie_BoxFillIndex(scrX+scrR-7, scrY+scrR, scrX+scrR, scrY+scrR, boxCol);
							pie_BoxFillIndex(scrX+scrR, scrY+scrR-7, scrX+scrR, scrY+scrR+1, boxCol);
						}
						else
						{
							if(bTinyBars)
							{
								pie_BoxFillIndex(scrX-scrR-3, scrY-3, scrX-scrR+3, scrY+3, 1);
								pie_BoxFillIndex(scrX-scrR-2, scrY-2, scrX-scrR+2, scrY+2, powerCol);
							}
							else
							{
								pie_BoxFillIndex(scrX-scrR, scrY+scrR-7, scrX-scrR, scrY+scrR, powerCol);
								pie_BoxFillIndex(scrX-scrR, scrY+scrR, scrX-scrR+7, scrY+scrR, powerCol);
								pie_BoxFillIndex(scrX+scrR-7, scrY+scrR, scrX+scrR, scrY+scrR, powerCol);
								pie_BoxFillIndex(scrX+scrR, scrY+scrR-7, scrX+scrR, scrY+scrR+1, powerCol);
							}
						}
					}
				}
				if(bEnergyBars)
				{
					if (pie_Hardware())
					{
						/* Power bars */
						pie_BoxFill(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+5,0x00020202);
						pie_BoxFill(scrX - scrR,scrY + scrR+3,scrX - scrR+damage,scrY+scrR+4,longPowerCol);
					}
					else
					{
						/* Power bars */
						pie_BoxFillIndex(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+5,1);
						pie_BoxFillIndex(scrX - scrR,scrY + scrR+3,scrX - scrR+damage,scrY+scrR+4,powerCol);
					}
				}
			
				/* Write the droid rank out */
				if((scrX+scrR)>0 AND (scrY+scrR)>0 AND (scrX-scrR)<DISP_WIDTH
					AND (scrY-scrR)<DISP_HEIGHT)
				{
					drawDroidRank(psDroid);
					drawDroidSensorLock(psDroid);

					if ((psDroid->droidType == DROID_COMMAND) ||
						(psDroid->psGroup != NULL && psDroid->psGroup->type == GT_COMMAND))
					{
						drawDroidCmndNo(psDroid);
					}
					else if(psDroid->group!=UBYTE_MAX)
					{
						drawDroidGroupNumber(psDroid);
					}
				}
	
			}

			if (bReloadBars)
			{
				drawWeaponReloadBar((BASE_OBJECT *)psDroid, psDroid->asWeaps);
			}

		}
	}


  
	/* Are we over an enemy droid */
	if(bMouseOverDroid AND !bMouseOverOwnDroid)
	{
		if(mouseDown(MOUSE_RMB))
		{
			if(psClickedOn->player!=selectedPlayer AND psClickedOn->sDisplay.frameNumber == currentGameFrame)
			{
				psDroid = (DROID*)psClickedOn;
                //show resistance values if CTRL/SHIFT depressed
                if (ctrlShiftDown())
                {
                    if (psDroid->resistance)
                    {
                        damage = PERCENT(psDroid->resistance, droidResistance(psDroid));
                    }
                    else
                    {
                        damage = 100;
                    }
                }
                else
                {
	    			damage = PERCENT(psDroid->body,psDroid->originalBody);
                }
    
				if(pie_Hardware())
				{
			 		if(damage>REPAIRLEV_HIGH) longPowerCol = 0x0000ff00;//green
					else if(damage>REPAIRLEV_LOW) longPowerCol = 0x00ffff00;//yellow
					else longPowerCol = 0x00ff0000;//red
				}
				else
				{
					if(damage>REPAIRLEV_HIGH) powerCol = defaultColours.green;
					else if(damage>REPAIRLEV_LOW) powerCol = defaultColours.yellow;
					else powerCol = defaultColours.red;
				}
		  	    //show resistance values if CTRL/SHIFT depressed
                if (ctrlShiftDown())
                {
                    if (psDroid->resistance)
                    {
                        mulH = MAKEFRACT(psDroid->resistance) / MAKEFRACT(droidResistance(psDroid));
                    }
                    else
                    {
                        mulH = 100;
                    }
                }
                else
                {
    				mulH = MAKEFRACT(psDroid->body) / MAKEFRACT(psDroid->originalBody);
                }
			    damage = MAKEINT(mulH*MAKEFRACT(psDroid->sDisplay.screenR));// (((psDroid->sDisplay.screenR*10000)/100)*damage)/10000;
//			    damage = MAKEINT(MAKEFRACT(psDroid->body) / MAKEFRACT(psDroid->originalBody));// (((psDroid->sDisplay.screenR*10000)/100)*damage)/10000;
//				damage = (((psDroid->sDisplay.screenR*10000)/100)*damage)/10000;
				if(damage>psDroid->sDisplay.screenR) damage = psDroid->sDisplay.screenR;
				damage *=2;
				scrX = psDroid->sDisplay.screenX;
				scrY = psDroid->sDisplay.screenY;
				scrR = psDroid->sDisplay.screenR;

				/* Yeah, yeah yeah - hardcoded palette entries - need to change to #defined colour names */
				/* Three DFX clips properly right now - not sure if software does */
				if((scrX+scrR)>0 AND (scrY+scrR)>0 AND (scrX-scrR)<DISP_WIDTH AND (scrY-scrR)<DISP_HEIGHT)
				{
					if(!driveModeActive() || driveIsDriven(psDroid)) {
						boxCol = defaultColours.white;
						longBoxCol = 0x00ffffff;
					} else {
						boxCol = defaultColours.green;
						longBoxCol = 0x0000ff00;
					}

                    //we always want to show the enemy health/resistance as energyBar - AB 18/06/99
					//if(bEnergyBars)
					{
						if (pie_Hardware())
						{
							/* Power bars */
							pie_BoxFill(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+5,0x00020202);
							pie_BoxFill(scrX - scrR,scrY + scrR+3,scrX - scrR+damage,scrY+scrR+4,longPowerCol);
						}
						else
						{
							/* Power bars */
							pie_BoxFillIndex(scrX - scrR-1,scrY + scrR+2,scrX + scrR+1,scrY+scrR+6,1);
							pie_BoxFillIndex(scrX - scrR,scrY + scrR+3,scrX - scrR+damage,scrY+scrR+5,powerCol);
						}
					}
				}
			}
		}
	}

	for(i=0; i<MAX_PLAYERS; i++)
	{
		/* Go thru' all the droidss */
		for(psDroid = apsDroidLists[i]; psDroid; psDroid = psDroid->psNext)
		{
			if(i!=selectedPlayer AND !psDroid->died AND psDroid->sDisplay.frameNumber == currentGameFrame)		
			{
				/* If it's selected */
				if(psDroid->bTargetted AND (psDroid->visible[selectedPlayer] == UBYTE_MAX))
				{
					psDroid->bTargetted = FALSE;
					scrX = psDroid->sDisplay.screenX;
					scrY = psDroid->sDisplay.screenY - 8;
					index = IMAGE_BLUE1+getTimeValueRange(1020,5);
					iV_DrawTransImage(IntImages,index,scrX,scrY);
				}
			}
		}
	}

	for(psFeature = apsFeatureLists[0]; psFeature; psFeature = psFeature->psNext)
	{
		if(!psFeature->died AND psFeature->sDisplay.frameNumber == currentGameFrame)
		{
			if(psFeature->bTargetted)
			{
				psFeature->bTargetted = FALSE;
				scrX = psFeature->sDisplay.screenX;
				scrY = psFeature->sDisplay.screenY - (psFeature->sDisplay.imd->ymax/4);
				iV_DrawTransImage(IntImages,getTargettingGfx(),scrX,scrY);
			}
		}
	}


	pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
}
/* ---------------------------------------------------------------------------- */
void	drawBuildingLines( void )
{
iVector	first,second;

if(buildState == BUILD3D_VALID OR buildState == BUILD3D_POS)
	{
		pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
		pie_SetFogStatus(FALSE);
		first.x = 1000;//buildSite.xTL * 128;
		first.y = 116;
		first.z = 1000;//buildSite.yTL * 128;

		second.x = 3000;//mouseTileX<<TILE_SHIFT;//buildSite.xBR * 128;
		second.y = 116;
		second.z = 3000;//mouseTileY<<TILE_SHIFT;//buildSite.yBR * 128;

		draw3dLine(&first,&second,rand()%255);

		pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
	}

}
/* ---------------------------------------------------------------------------- */
#define GN_X_OFFSET	(28)
#define GN_Y_OFFSET (17)
void	drawDroidGroupNumber(DROID *psDroid)
{
UWORD	id;
UDWORD	id2;
BOOL	bDraw;
SDWORD	xShift,yShift;

	bDraw = TRUE;

	id = id2 = UDWORD_MAX;

	/* Is the unit in a group? */
	if(psDroid->psGroup AND psDroid->psGroup->type == GT_COMMAND)
	{
		id2 = IMAGE_GN_STAR;

	}
	//else
	{
		switch(psDroid->group)
		{
		case 0:
			id = IMAGE_GN_0;
			break;
		case 1:
			id = IMAGE_GN_1;
			break;
		case 2:
			id = IMAGE_GN_2;
			break;
		case 3:
			id = IMAGE_GN_3;
			break;
		case 4:
			id = IMAGE_GN_4;
			break;
		case 5:
			id = IMAGE_GN_5;
			break;
		case 6:
			id = IMAGE_GN_6;
			break;
		case 7:
			id = IMAGE_GN_7;
			break;
		case 8:
			id = IMAGE_GN_8;
			break;
		case 9:
			id = IMAGE_GN_9;
			break;
		default:
			bDraw = FALSE;
			break;
		}
	}
	if(bDraw)
	{
		xShift = GN_X_OFFSET;  // yeah yeah, I know
		yShift = GN_Y_OFFSET;
		xShift = ((xShift*pie_GetResScalingFactor())/100);
		yShift = ((yShift*pie_GetResScalingFactor())/100);
		iV_DrawTransImage(IntImages,id,psDroid->sDisplay.screenX-xShift,psDroid->sDisplay.screenY+yShift);
		if(id2!=UDWORD_MAX)
		{
			iV_DrawTransImage(IntImages,id2,psDroid->sDisplay.screenX-xShift,psDroid->sDisplay.screenY+yShift-8);
		}
	}
}
/* ---------------------------------------------------------------------------- */
void	drawDroidCmndNo(DROID *psDroid)
{
UWORD	id;
UDWORD	id2;
BOOL	bDraw;
SDWORD	xShift,yShift, index;

	bDraw = TRUE;

	id = id2 = UDWORD_MAX;

	id2 = IMAGE_GN_STAR;
	index = SDWORD_MAX;
	if (psDroid->droidType == DROID_COMMAND)
	{
		index = cmdDroidGetIndex(psDroid);
	}
	else if (psDroid->psGroup && psDroid->psGroup->type == GT_COMMAND)
	{
		index = cmdDroidGetIndex(psDroid->psGroup->psCommander);
	}
	switch(index)
	{
	case 1:
		id = IMAGE_GN_1;
		break;
	case 2:
		id = IMAGE_GN_2;
		break;
	case 3:
		id = IMAGE_GN_3;
		break;
	case 4:
		id = IMAGE_GN_4;
		break;
	case 5:
		id = IMAGE_GN_5;
		break;
	case 6:
		id = IMAGE_GN_6;
		break;
	case 7:
		id = IMAGE_GN_7;
		break;
	case 8:
		id = IMAGE_GN_8;
		break;
	case 9:
		id = IMAGE_GN_9;
		break;
	default:
		bDraw = FALSE;
		break;
	}

	if(bDraw)
	{
		xShift = GN_X_OFFSET;  // yeah yeah, I know
		yShift = GN_Y_OFFSET;
		xShift = ((xShift*pie_GetResScalingFactor())/100);
		yShift = ((yShift*pie_GetResScalingFactor())/100);
		iV_DrawTransImage(IntImages,id2,psDroid->sDisplay.screenX-xShift-6,psDroid->sDisplay.screenY+yShift);
		iV_DrawTransImage(IntImages,id,psDroid->sDisplay.screenX-xShift,psDroid->sDisplay.screenY+yShift);
	}
}
/* ---------------------------------------------------------------------------- */


/* ---------------------------------------------------------------------------- */
void	draw3dLine(iVector *src, iVector *dest, UBYTE col)
{
iVector	null,vec;
iPoint	srcS,destS;

	null.x = null.y = null.z = 0;
	vec.x = (src->x - player.p.x) - terrainMidX*TILE_UNITS;
	vec.z = terrainMidY*TILE_UNITS - (src->z - player.p.z);
	vec.y = src->y;

	pie_MatBegin();

	/* Translate */
	pie_TRANSLATE(vec.x,vec.y,vec.z);
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Translate */
	pie_TRANSLATE(rx,0,-rz);

	/* Project - no rotation being done */
	pie_RotProj(&null,&srcS);
	pie_MatEnd();

	vec.x = (dest->x - player.p.x) - terrainMidX*TILE_UNITS;
	vec.z = terrainMidY*TILE_UNITS - (dest->z - player.p.z);
	vec.y = dest->y;

	iV_MatrixBegin();

	/* Translate */
	pie_TRANSLATE(vec.x,vec.y,vec.z);
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Translate */
	pie_TRANSLATE(rx,0,-rz);

	/* Project - no rotation being done */
	pie_RotProj(&null,&destS);
	pie_MatEnd();

	iV_Line(srcS.x,srcS.y,destS.x,destS.y,col);
}

/*	Get the onscreen corrdinates of a droid - so we can draw a bounding box - this need to be severely
	speeded up and the accuracy increased to allow variable size bouding boxes */
void	calcScreenCoords(DROID *psDroid)
{
//BOOL	setMouse = FALSE;
SDWORD	centX,centY,centZ;
SDWORD	cX,cY;
iIMDShape	*imd;
UDWORD	radius;
POINT	pt;

	/* Ennsure correct context */
	pie_SETUP_ROTATE_PROJECT;

	/* which IMD are we using? */
	imd = psDroid->sDisplay.imd;


	/* Get it's absolute dimensions */
	centX = centY = centZ = 0;

	/* How big a box do we want - will ultimately be calculated using xmax, ymax, zmax etc */
	if(psDroid->droidType == DROID_TRANSPORTER)
	{
		radius = 45;
	}
	else
	{
		radius = 22;

	}

	radius = ((radius * pie_GetResScalingFactor())/100);

	/* Pop matrices and get the screen corrdinates */
	pie_ROTATE_PROJECT(centX,centY,centZ,cX,cY);

	/* Deselect all the droids if we've released the drag box */
	if(dragBox3D.status == DRAG_RELEASED)
	{
		pt.x = cX;
		pt.y = cY;
		if(inQuad(&pt,&dragQuad) AND psDroid->player == selectedPlayer)
		{	   	
			//don't allow Transporter Droids to be selected here
            //unless we're in multiPlayer mode!!!!
			if (psDroid->droidType != DROID_TRANSPORTER OR bMultiPlayer)
			{
				dealWithDroidSelect(psDroid, TRUE);
/*				psDroid->selected = TRUE;
				if (bInTutorial)
				{
					psCBSelectedDroid = psDroid;
					eventFireCallbackTrigger(CALL_DROID_SELECTED);
					psCBSelectedDroid = NULL;
				}*/
			}
		}
	}
	cY-=4;
	/* Store away the screen coordinates so we can select the droids without doing a trasform */
	psDroid->sDisplay.screenX = cX;
	psDroid->sDisplay.screenY = cY;
	psDroid->sDisplay.screenR = radius;
}

void	preprocessTiles(void)
{
UDWORD	i,j;
UDWORD	left,right,up,down, size;
DROID   *psDroid;
//BASE_OBJECT *psObj;
SDWORD  order;
//UDWORD	tileCount;

	/* Set up the highlights if we're putting down a wall */
	if(wallDrag.status == DRAG_PLACING OR wallDrag.status == DRAG_DRAGGING)
	{
		/* Ensure the start point is always shown */
		SET_TILE_HIGHLIGHT(mapTile(wallDrag.x1,wallDrag.y1));
		if( (wallDrag.x1 == wallDrag.x2) OR (wallDrag.y1 == wallDrag.y2) )
		{
			/* First process the ones inside the wall dragging area */
			left = min(wallDrag.x1,wallDrag.x2);
			right = max(wallDrag.x1,wallDrag.x2) + 1;
			up = min(wallDrag.y1,wallDrag.y2);
			down = max(wallDrag.y1,wallDrag.y2) + 1;

			for(i=left; i<right; i++)
			{
				for(j=up; j<down; j++)
				{
					SET_TILE_HIGHLIGHT(mapTile(i,j));
				}
			}
		}
	}
	else
	/* Only bother if we're placing a building */
	if(buildState == BUILD3D_VALID OR buildState == BUILD3D_POS)
	{
	/* Now do the ones inside the building highlight */
		left = buildSite.xTL;
		right = buildSite.xBR+1;
		up = buildSite.yTL;
		down = buildSite.yBR+1;
		
		for(i=left; i<right; i++)
		{
			for(j=up; j<down; j++)
			{
				SET_TILE_HIGHLIGHT(mapTile(i,j));
//				tileCount++;
//				averageHeight+=map_TileHeight(i,j);
			}
		}
    }

    //don't display until we're releasing this feature in an update!
#ifndef DISABLE_BUILD_QUEUE
    if (intBuildSelectMode())
    {
        //and there may be multiple building sites that need highlighting - AB 26/04/99
        if (ctrlShiftDown())
        {
            //this just highlights the current interface selected unit
            //psObj = getCurrentSelected();
            //if (psObj AND psObj->type == OBJ_DROID)
            
            //this highlights ALL constructor units' build sites
            for (psDroid = apsDroidLists[selectedPlayer]; psDroid; psDroid = psDroid->psNext)
            {
                //psDroid = (DROID *)psObj;
                if (psDroid->droidType == DROID_CONSTRUCT OR
                    psDroid->droidType == DROID_CYBORG_CONSTRUCT)
                {
                    //draw the current build site if its a line of structures
                    if (psDroid->order == DORDER_LINEBUILD)
                    {
                            left = psDroid->orderX >> TILE_SHIFT;
                            right = (psDroid->orderX2 >> TILE_SHIFT) + 1;
                            if (left > right)
                            {
                                size = left;
                                left = right;
                                right = size;
                            }
                            up = psDroid->orderY >> TILE_SHIFT;
                            down = (psDroid->orderY2 >> TILE_SHIFT) + 1;
                            if (up > down)
                            {
                                size = up;
                                up = down;
                                down = size;
                            }
                            //hilight the tiles
    		                for(i=left; i<right; i++)
	    	                {
		    	                for(j=up; j<down; j++)
			                    {
				                    SET_TILE_HIGHLIGHT(mapTile(i,j));
			                    }
                            }
                    }
                    //now look thru' the list of orders to see if more building sites
                    for (order = 0; order < psDroid->listSize; order++)
                    {
                        if (psDroid->asOrderList[order].order == DORDER_BUILD)
                        {
                            //set up coords for tiles
                            size = ((STRUCTURE_STATS *)psDroid->asOrderList[order].
                                psOrderTarget)->baseWidth;
                            left = ((psDroid->asOrderList[order].x) >> TILE_SHIFT) - size/2;
                            right = left + size;
                            size = ((STRUCTURE_STATS *)psDroid->asOrderList[order].
                                psOrderTarget)->baseBreadth;
                            up = ((psDroid->asOrderList[order].y) >> TILE_SHIFT) - size/2;
                            down = up + size;
                            //hilight the tiles
    		                for(i=left; i<right; i++)
	    	                {
		    	                for(j=up; j<down; j++)
			                    {
				                    SET_TILE_HIGHLIGHT(mapTile(i,j));
			                    }
                            }
		                }
                        else if (psDroid->asOrderList[order].order == DORDER_LINEBUILD)
                        {
                            //need to highlight the length of the wall
                            left = psDroid->asOrderList[order].x >> TILE_SHIFT;
                            right = psDroid->asOrderList[order].x2 >> TILE_SHIFT;
                            if (left > right)
                            {
                                size = left;
                                left = right;
                                right = size;
                            }
                            up = psDroid->asOrderList[order].y >> TILE_SHIFT;
                            down = psDroid->asOrderList[order].y2 >> TILE_SHIFT;
                            if (up > down)
                            {
                                size = up;
                                up = down;
                                down = size;
                            }
                            //hilight the tiles
    		                for(i=left; i<=right; i++)
	    	                {
		    	                for(j=up; j<=down; j++)
			                    {
				                    SET_TILE_HIGHLIGHT(mapTile(i,j));
			                    }
                            }
                        }
                    }
                }
            }
        }
	}
#endif
//	if(tileCount)
//	{													  

//		averageHeight = averageHeight/tileCount;
//	}
}

//this never gets called!
/*void	postprocessTiles(void)
{
UDWORD	i,j;
UDWORD	left,right,up,down;
MAPTILE	*psTile;

	// Clear the highlights if we're putting down a wall
	if(wallDrag.status == DRAG_PLACING OR wallDrag.status == DRAG_DRAGGING)
	{
		// Only show valid walls
		if( (wallDrag.x1 == wallDrag.x2) OR (wallDrag.y1 == wallDrag.y2) )
		{
			// First process the ones inside the wall dragging area
			left = min(wallDrag.x1,wallDrag.x2);
			right = max(wallDrag.x1,wallDrag.x2);
			up = min(wallDrag.y1,wallDrag.y2);
			down = max(wallDrag.y1,wallDrag.y2);

			for(i=left; i<right; i++)
			{
				for(j=up; j<down; j++)
				{
					psTile = mapTile(i,j);
					CLEAR_TILE_HIGHLIGHT(psTile);
				}
			}
		}
	}
	// Now do the ones inside the building highlight
	left = buildSite.xTL;
	right = buildSite.xBR;
	up = buildSite.yTL;
	down = buildSite.yBR;
	
	for(i=left; i<right; i++)
	{
		for(j=up; j<down; j++)
		{
			psTile = mapTile(i,j);
			CLEAR_TILE_HIGHLIGHT(psTile);
		}
	}
}*/

/* This is slow - speed it up */

void	locateMouse(void)
{
UDWORD	i,j;
POINT	pt;
QUAD	quad;
SDWORD	nearestZ = SDWORD_MAX;
SDWORD	tileZ;
BOOL	bWaterTile;
//UDWORD	bX,bY;

	pt.x = mX;
	pt.y = mY;
	for(i=0; i<visibleXTiles; i++)
	{
		for(j=0; j<visibleYTiles; j++)
		{
			bWaterTile = tileScreenInfo[i][j].bWater;

			tileZ = (bWaterTile ? tileScreenInfo[i][j].wz : tileScreenInfo[i][j].sz);
			if(tileZ<=nearestZ)
			{
				quad.coords[0].x = (bWaterTile ? tileScreenInfo[i][j].wx : tileScreenInfo[i][j].sx);
				quad.coords[0].y = (bWaterTile ? tileScreenInfo[i][j].wy : tileScreenInfo[i][j].sy);
   						
				quad.coords[1].x = (bWaterTile ? tileScreenInfo[i][j+1].wx : tileScreenInfo[i][j+1].sx);
				quad.coords[1].y = (bWaterTile ? tileScreenInfo[i][j+1].wy : tileScreenInfo[i][j+1].sy);
   				
				quad.coords[2].x = (bWaterTile ? tileScreenInfo[i+1][j+1].wx : tileScreenInfo[i+1][j+1].sx);
				quad.coords[2].y = (bWaterTile ? tileScreenInfo[i+1][j+1].wy :tileScreenInfo[i+1][j+1].sy);
   			 						
				quad.coords[3].x = (bWaterTile ? tileScreenInfo[i+1][j].wx: tileScreenInfo[i+1][j].sx);
				quad.coords[3].y = (bWaterTile ? tileScreenInfo[i+1][j].wy: tileScreenInfo[i+1][j].sy);
   				
				/* We've got a match for our mouse coords */
				if(inQuad(&pt,&quad))
				{
					mouseTileX = playerXTile+j;
					mouseTileY = playerZTile+i;
					if(mouseTileX<0) mouseTileX = 0;
					if(mouseTileX>mapWidth-1) mouseTileX = mapWidth-1;
					if(mouseTileY<0) mouseTileY = 0;
					if(mouseTileY>mapHeight-1) mouseTileY = mapHeight-1;

 					tile3dX = playerXTile+j;
					tile3dY = playerZTile+i;
					if( (tile3dX > 0) AND
						(tile3dY > 0) AND
						(tile3dX <= mapWidth-1) AND
						(tile3dY <= mapHeight-1) )

					tile3dOver = mapTile(tile3dX,tile3dY);
					/* Store away z value */
					nearestZ = tileZ;
				}
			} 
		}
	}
 
   
}


/*
This needs to be dumped into IVIS - shouldn't be in here - Alex
*/
void	scaleMatrix( UDWORD percent )
{
SDWORD	scaleFactor;

	if(percent == 100)
	{
		return;
	}

	scaleFactor = percent * ONE_PERCENT;

	psMatrix->a = (psMatrix->a * scaleFactor) / 4096;
	psMatrix->b = (psMatrix->b * scaleFactor) / 4096;
	psMatrix->c = (psMatrix->c * scaleFactor) / 4096;

	psMatrix->d = (psMatrix->d * scaleFactor) / 4096;
	psMatrix->e = (psMatrix->e * scaleFactor) / 4096;
	psMatrix->f = (psMatrix->f * scaleFactor) / 4096;

	psMatrix->g = (psMatrix->g * scaleFactor) / 4096;
	psMatrix->h = (psMatrix->h * scaleFactor) / 4096;
	psMatrix->i = (psMatrix->i * scaleFactor) / 4096;
}

/*
HACK for IAN VISIT, but works....
*/
void renderSky(void)
{
	SDWORD index,width;
	skyShift+=2;

	index= (player.r.y + skyShift)/ 20;

	while (index > 0)
	{
		index -=256;
	}

	while (index < 640)
	{
		if(index >=384)
		{
			width = 640-index;
		}
		else
		{
			width = 256;
		}
		if (pie_Hardware())
		{
			iV_UniBitmapDepth(texPage,0,0,width,128,index,0,width,192,200,65000);
		}
		index += 256;
	}

	index= (player.r.y+(skyShift/2))/ 15;

	while (index > 0)
	{
		index -=256;
	}

	while (index < 640)
	{
		if(index >=384)
		{
			width = 640-index;
		}
		else
		{
			width = 256;
		}
		if (pie_Hardware())
		{
			iV_UniBitmapDepth(texPage,0,128,width,127,index,0,width,192,200,64000);
		}
		index += 256;
	}
}

/* Flattens an imd to the landscape and handles 4 different rotations */
iIMDShape	*flattenImd(iIMDShape *imd, UDWORD structX, UDWORD structY, UDWORD direction)
{
UDWORD	i;
UDWORD	pointHeight,centreHeight;
SDWORD	shift;

	// CHECK WHETHER THE NUMBER OF POINTS IN THE IMD WILL FIT IN THE ARRAY

	ASSERT((imd->npoints < MAX_FLATTEN_POINTS,
		"flattenImd: too many points in the PIE to flatten it"));

	/* Get a copy of the points */
   	memcpy(alteredPoints,imd->points,imd->npoints*sizeof(iVector));
   	/* Get the height of the centre point for reference */
   	centreHeight = map_Height(structX,structY);
  	/* Now we go through the shape looking for vertices on the edge */
  	/* Flip reference coords if we're on a vertical wall */

	/* Little hack below 'cos sometimes they're not exactly 90 degree alligned. */
	direction /= 90;
	direction *= 90;

	switch(direction)
  	{
  	case 0:
  		for(i=0; i<(UDWORD)imd->npoints; i++)
    		{
				if(abs(alteredPoints[i].x) >= 63 OR abs(alteredPoints[i].z)>=63)
				{
  					pointHeight = map_Height(structX+alteredPoints[i].x,structY-alteredPoints[i].z);
    					shift = centreHeight - pointHeight;
						alteredPoints[i].y -= (shift-4);
				}
    		}
  		break;
  	case 90:
  		for(i=0; i<(UDWORD)imd->npoints; i++)
    		{
				if(abs(alteredPoints[i].x) >= 63 OR abs(alteredPoints[i].z)>=63)
				{
  					pointHeight = map_Height(structX-alteredPoints[i].z,structY-alteredPoints[i].x);
    				shift = centreHeight - pointHeight;
  					alteredPoints[i].y -= (shift-4);
				}
    		}
  		break;

  	case 180:
  		for(i=0; i<(UDWORD)imd->npoints; i++)
    		{
				if(abs(alteredPoints[i].x) >= 63 OR abs(alteredPoints[i].z)>=63)
				{
  					pointHeight = map_Height(structX-alteredPoints[i].x,structY+alteredPoints[i].z);
    					shift = centreHeight - pointHeight;
  					alteredPoints[i].y -= (shift-4);
				}
    		}
  		break;
  	case 270:
  		for(i=0; i<(UDWORD)imd->npoints; i++)
    		{
				if(abs(alteredPoints[i].x) >= 63 OR abs(alteredPoints[i].z)>=63)
				{
  					pointHeight = map_Height(structX+alteredPoints[i].z,structY+alteredPoints[i].x);
    					shift = centreHeight - pointHeight;
  					alteredPoints[i].y -= (shift-4);
				}
    		}
  		break;

  	default:
  		DBERROR((FALSE,"Weirdy direction for a structure in renderWall"));
  		break;
  	}
/*
			if (psStructure-> direction == 90)
			{
				for(i=0; i<imd->npoints; i++)
		  		{
						pointHeight = map_Height(structX-alteredPoints[i].z,structY-alteredPoints[i].x);
		  				shift = centreHeight - pointHeight;
						alteredPoints[i].y -= shift;
		  		}
			}
			else
			{
				for(i=0; i<imd->npoints; i++)
		  		{
						pointHeight = map_Height(structX+alteredPoints[i].x,structY-alteredPoints[i].z);
		  				shift = centreHeight - pointHeight;
						alteredPoints[i].y -= shift;
		  		}
			}
*/
			imd->points = alteredPoints;
			return(imd);
}

void	getDefaultColours( void )
{
defaultColours.red = iV_PaletteNearestColour(255,0,0);
defaultColours.green = iV_PaletteNearestColour(0,255,0);
defaultColours.blue = iV_PaletteNearestColour(0,0,255);
defaultColours.yellow = iV_PaletteNearestColour(255,255,0);
defaultColours.purple = iV_PaletteNearestColour(255,0,255);
defaultColours.cyan = iV_PaletteNearestColour(0,255,255);
defaultColours.black = iV_PaletteNearestColour(0,0,0);
defaultColours.white = iV_PaletteNearestColour(255,255,255);
}

#ifdef JOHN
//#define SHOW_ZONES
//#define SHOW_GATEWAYS
#endif

// -------------------------------------------------------------------------------------
/* New improved (and much faster) tile drawer */
// -------------------------------------------------------------------------------------
void	drawTerrainTile(UDWORD i, UDWORD j)//hardware only
{
SDWORD	actualX,actualY;
MAPTILE	*psTile;
BOOL	bOutlined;
UDWORD	tileNumber;
UDWORD	renderFlag;
iPoint	offset;
PIEVERTEX aVrts[3];
BYTE	oldColours[4];
UDWORD	oldColoursWord[4];
#if defined(SHOW_ZONES) || defined(SHOW_GATEWAYS)
SDWORD	zone;
#endif

	/* Get the correct tile index for the x coordinate */
	actualX = playerXTile + j;
	/* Get the correct tile index for the y coordinate */
	actualY = playerZTile + i;

   //	ASSERT((actualX<mapWidth,"X Coordinate invalid in tile draw"));
   //	ASSERT((actualY<mapWidth,"Y Coordinate invalid in tile draw"));


#ifdef SHOW_ZONES
	zone = 0;
#endif

	/* Let's just get out now if we're not supposed to draw it */
	if( (actualX<0) OR
		(actualY<0) OR
		(actualX>mapWidth-1) OR
		(actualY>mapHeight-1) )
	{
		psTile = &edgeTile;
		CLEAR_TILE_HIGHLIGHT(psTile);
	}
	else
	{
		psTile = mapTile(actualX,actualY);
#ifdef SHOW_ZONES
		if (!fpathBlockingTile(actualX,actualY) ||
			TERRAIN_TYPE(psTile) == TER_WATER)
		{
			zone = gwGetZone(actualX,actualY);
		}
#endif
#ifdef SHOW_GATEWAYS
		if (psTile->tileInfoBits & BITS_GATEWAY)
		{
			zone  = gwGetZone(actualX,actualY);
		}
#endif
	}

	if(!TILE_DRAW(psTile))
	{
		/* This tile isn't being drawn */
		return;
	}
	/* what tile texture number is it? */
	tileNumber = psTile->texture;

	// If it's a water tile then force it to be the river bed texture.
	if(TERRAIN_TYPE(psTile) == TER_WATER) {
		tileNumber = RiverBedTileID;
	}

#if defined(SHOW_ZONES)
	if (zone != 0)
	{
		tileNumber = zone;
	}
#elif defined(SHOW_GATEWAYS)
	if (psTile->tileInfoBits & BITS_GATEWAY)
	{
		tileNumber = 55;//zone;
	}
#endif


	/* Is the tile highlighted? Perhaps because there's a building foundation on it */
	bOutlined = FALSE;
	if(TILE_HIGHLIGHT(psTile))
	{
		/* Clear it for next time round */
		CLEAR_TILE_HIGHLIGHT(psTile);
		bOutlined = TRUE;
		//set tilenumber
//		tileNumber = FOUNDATION_TEXTURE;
		if( (i<LAND_XGRD-1) AND (j<(LAND_YGRD-1)) )		// FIXME
		{
			if(outlineColour3D == outlineOK3D)
			{
		 		

				oldColoursWord[0] = tileScreenInfo[i+0][j+0].light.argb;
				oldColoursWord[1] = tileScreenInfo[i+0][j+1].light.argb;
				oldColoursWord[2] = tileScreenInfo[i+1][j+1].light.argb;
				oldColoursWord[3] = tileScreenInfo[i+1][j+0].light.argb;
				/*
				tileScreenInfo[i+0][j+0].light.argb = 0x00f0f0f0;
				tileScreenInfo[i+0][j+1].light.argb = 0x00f0f0f0;
				tileScreenInfo[i+1][j+1].light.argb = 0x00f0f0f0;
				tileScreenInfo[i+1][j+0].light.argb = 0x00f0f0f0;
				*/
				tileScreenInfo[i+0][j+0].light.byte.b = 255;
				tileScreenInfo[i+0][j+1].light.byte.b = 255;
				tileScreenInfo[i+1][j+1].light.byte.b = 255;
				tileScreenInfo[i+1][j+0].light.byte.b = 255;

				tileScreenInfo[i+0][j+0].light.byte.g = 255;
				tileScreenInfo[i+0][j+1].light.byte.g = 255;
				tileScreenInfo[i+1][j+1].light.byte.g = 255;
				tileScreenInfo[i+1][j+0].light.byte.g = 255;

				tileScreenInfo[i+0][j+0].light.byte.r = 255;
				tileScreenInfo[i+0][j+1].light.byte.r = 255;
				tileScreenInfo[i+1][j+1].light.byte.r = 255;
				tileScreenInfo[i+1][j+0].light.byte.r = 255;
			}
			else
			{
				oldColours[0] = tileScreenInfo[i+0][j+0].light.byte.r;
				oldColours[1] = tileScreenInfo[i+0][j+1].light.byte.r;
				oldColours[2] = tileScreenInfo[i+1][j+1].light.byte.r;
				oldColours[3] = tileScreenInfo[i+1][j+0].light.byte.r;
				tileScreenInfo[i+0][j+0].light.byte.r = 255;
				tileScreenInfo[i+0][j+1].light.byte.r = 255;
				tileScreenInfo[i+1][j+1].light.byte.r = 255;
				tileScreenInfo[i+1][j+0].light.byte.r = 255;
			}
		}
	}
	/*
	else if(TILE_IN_SENSORRANGE(psTile))
	{
		if(tileScreenInfo[i+0][j+0].light.byte.g < 128) 
		{
			tileScreenInfo[i+0][j+0].light.byte.g *= 2;
		}
		else
		{
			tileScreenInfo[i+0][j+0].light.byte.g =255;
		}

		if(tileScreenInfo[i+0][j+1].light.byte.g < 128) 
		{
			tileScreenInfo[i+0][j+1].light.byte.g *= 2;
		}
		else
		{
			tileScreenInfo[i+0][j+1].light.byte.g =255;
		}

		if(tileScreenInfo[i+1][j+1].light.byte.g < 128) 
		{
			tileScreenInfo[i+1][j+1].light.byte.g *= 2;
		}
		else
		{
			tileScreenInfo[i+1][j+1].light.byte.g =255;
		}
		if(tileScreenInfo[i+1][j+0].light.byte.g < 128) 
		{
			tileScreenInfo[i+1][j+0].light.byte.g *= 2;
		}
		else
		{
			tileScreenInfo[i+1][j+0].light.byte.g =255;
		}

	}
	*/
	/* Get the right texture page */
#if 0
	/* Software is just an address */
	texturePage.bmp = tilesRAW[tileNumber & TILE_NUMMASK];
#endif
	/* 3dfx is pre stored and indexed */
	pie_SetTexturePage(tileTexInfo[tileNumber & TILE_NUMMASK].texPage);


	/* set up the texture size info */
	renderFlag = 0;
	offset.x = (tileTexInfo[tileNumber & TILE_NUMMASK].xOffset * 64);
	offset.y = (tileTexInfo[tileNumber & TILE_NUMMASK].yOffset * 64);

	/* Check for rotations and flips - this sets up the coordinates for texturing */
	flipsAndRots(tileNumber & ~TILE_NUMMASK);

	tileScreenInfo[i+0][j+0].tu = (UWORD)(psP1->x + offset.x);
	tileScreenInfo[i+0][j+0].tv = (UWORD)(psP1->y + offset.y);

	tileScreenInfo[i+0][j+1].tu = (UWORD)(psP2->x + offset.x);
	tileScreenInfo[i+0][j+1].tv = (UWORD)(psP2->y + offset.y);
		
	tileScreenInfo[i+1][j+1].tu = (UWORD)(psP3->x + offset.x);
	tileScreenInfo[i+1][j+1].tv = (UWORD)(psP3->y + offset.y);

	tileScreenInfo[i+1][j+0].tu = (UWORD)(psP4->x + offset.x);
	tileScreenInfo[i+1][j+0].tv = (UWORD)(psP4->y + offset.y);

	/* The first triangle */
	if(TRI_FLIPPED(psTile))
	{
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle((PIEVERTEX*)&tileScreenInfo[i+0][j+0],
								(PIEVERTEX*)&tileScreenInfo[i+0][j+1],
								(PIEVERTEX*)&tileScreenInfo[i+1][j+0],
								&texturePage,0,0);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
			memcpy(&aVrts[1],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
			memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}
	else
	{
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle((PIEVERTEX*)&tileScreenInfo[i+0][j+0],
								(PIEVERTEX*)&tileScreenInfo[i+0][j+1],
								(PIEVERTEX*)&tileScreenInfo[i+1][j+1],
				&texturePage,0,0);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
			memcpy(&aVrts[1],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
			memcpy(&aVrts[2],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}

	/* The second triangle */
	if(TRI_FLIPPED(psTile))
	{
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle((PIEVERTEX*)&tileScreenInfo[i+0][j+1],
								(PIEVERTEX*)&tileScreenInfo[i+1][j+1],
								(PIEVERTEX*)&tileScreenInfo[i+1][j+0],
				&texturePage,0,0);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			memcpy(&aVrts[0],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
			memcpy(&aVrts[1],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
			memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}
	else
	{
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle((PIEVERTEX*)&tileScreenInfo[i+0][j+0],
								(PIEVERTEX*)&tileScreenInfo[i+1][j+1],
								(PIEVERTEX*)&tileScreenInfo[i+1][j+0],
				&texturePage,0,0);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
			memcpy(&aVrts[1],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
			memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}

	/* Outline the tile if necessary */
	if(terrainOutline)
	{
    	/*iV_Line(tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,
   			tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,255);
   		iV_Line(tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,
   			tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,255);
   		iV_Line(tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,
   			tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,255);
   		iV_Line(tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,
   		tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,255);*/
		
      	iV_Line(tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,
   			tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,255);
   		iV_Line(tileScreenInfo[i+0][j+1].sx,tileScreenInfo[i+0][j+1].sy,
   			tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,255);
   		iV_Line(tileScreenInfo[i+1][j+1].sx,tileScreenInfo[i+1][j+1].sy,
   			tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,255);
   		iV_Line(tileScreenInfo[i+1][j+0].sx,tileScreenInfo[i+1][j+0].sy,
   			tileScreenInfo[i+0][j+0].sx,tileScreenInfo[i+0][j+0].sy,255);
	}

	if(bOutlined)
	{
		if(outlineColour3D == outlineOK3D)
		{
			tileScreenInfo[i+0][j+0].light.argb = oldColoursWord[0];
			tileScreenInfo[i+0][j+1].light.argb = oldColoursWord[1];
			tileScreenInfo[i+1][j+1].light.argb = oldColoursWord[2];
			tileScreenInfo[i+1][j+0].light.argb = oldColoursWord[3];
		}
		else
		{
			tileScreenInfo[i+0][j+0].light.byte.r = oldColours[0];
			tileScreenInfo[i+0][j+1].light.byte.r = oldColours[1];
			tileScreenInfo[i+1][j+1].light.byte.r = oldColours[2];
			tileScreenInfo[i+1][j+0].light.byte.r = oldColours[3];
		}
	}
}


// Render a water edge tile.
//
void	drawTerrainWEdgeTile(UDWORD i, UDWORD j)
{
	UDWORD	actualX,actualY;
	MAPTILE	*psTile;
	//BOOL	bOutlined;
	UDWORD	tileNumber;
	UDWORD	renderFlag;
	iPoint	offset;
	PIEVERTEX aVrts[3];


	/* Get the correct tile index for the x coordinate */
	actualX = playerXTile + j;
	/* Get the correct tile index for the y coordinate */
	actualY = playerZTile + i;

	/* Let's just get out now if we're not supposed to draw it */
	if( (actualX<0) OR
		(actualY<0) OR
		(actualX>mapWidth-1) OR
		(actualY>mapHeight-1) )
	{
		psTile = &edgeTile;
		CLEAR_TILE_HIGHLIGHT(psTile);
	}
	else
	{
		psTile = mapTile(actualX,actualY);
	}

	/* what tile texture number is it? */
	tileNumber = psTile->texture;

	/* 3dfx is pre stored and indexed */
	pie_SetTexturePage(tileTexInfo[tileNumber & TILE_NUMMASK].texPage);


	/* set up the texture size info */
	renderFlag = 0;
	offset.x = (tileTexInfo[tileNumber & TILE_NUMMASK].xOffset * 64);
	offset.y = (tileTexInfo[tileNumber & TILE_NUMMASK].yOffset * 64);

	/* Check for rotations and flips - this sets up the coordinates for texturing */
	flipsAndRots(tileNumber & ~TILE_NUMMASK);

	tileScreenInfo[i+0][j+0].tu = (UWORD)(psP1->x + offset.x);
	tileScreenInfo[i+0][j+0].tv = (UWORD)(psP1->y + offset.y);

	tileScreenInfo[i+0][j+1].tu = (UWORD)(psP2->x + offset.x);
	tileScreenInfo[i+0][j+1].tv = (UWORD)(psP2->y + offset.y);
		
	tileScreenInfo[i+1][j+1].tu = (UWORD)(psP3->x + offset.x);
	tileScreenInfo[i+1][j+1].tv = (UWORD)(psP3->y + offset.y);

	tileScreenInfo[i+1][j+0].tu = (UWORD)(psP4->x + offset.x);
	tileScreenInfo[i+1][j+0].tv = (UWORD)(psP4->y + offset.y);

	/* The first triangle */
	if(TRI_FLIPPED(psTile))
	{
   		memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
   		aVrts[0].sx = tileScreenInfo[i+0][j+0].wx;
   		aVrts[0].sy = tileScreenInfo[i+0][j+0].wy;
   		aVrts[0].sz = tileScreenInfo[i+0][j+0].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[1],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
   		aVrts[1].sx = tileScreenInfo[i+0][j+1].wx;
   		aVrts[1].sy = tileScreenInfo[i+0][j+1].wy;
   		aVrts[1].sz = tileScreenInfo[i+0][j+1].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
   		aVrts[2].sx = tileScreenInfo[i+1][j+0].wx;
   		aVrts[2].sy = tileScreenInfo[i+1][j+0].wy;
   		aVrts[2].sz = tileScreenInfo[i+1][j+0].wz - WATER_EDGE_ZOFFSET;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle(&aVrts[0],&aVrts[1],&aVrts[2],
								&texturePage,0,pie_ADDITIVE);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}
	else
	{
   		memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
   		aVrts[0].sx = tileScreenInfo[i+0][j+0].wx;
   		aVrts[0].sy = tileScreenInfo[i+0][j+0].wy;
   		aVrts[0].sz = tileScreenInfo[i+0][j+0].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[1],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
   		aVrts[1].sx = tileScreenInfo[i+0][j+1].wx;
   		aVrts[1].sy = tileScreenInfo[i+0][j+1].wy;
   		aVrts[1].sz = tileScreenInfo[i+0][j+1].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[2],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
   		aVrts[2].sx = tileScreenInfo[i+1][j+1].wx;
   		aVrts[2].sy = tileScreenInfo[i+1][j+1].wy;
   		aVrts[2].sz = tileScreenInfo[i+1][j+1].wz - WATER_EDGE_ZOFFSET;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle(&aVrts[0],&aVrts[1],&aVrts[2],
								&texturePage,0,pie_ADDITIVE);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}

	/* The second triangle */
	if(TRI_FLIPPED(psTile))
	{
   		memcpy(&aVrts[0],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
   		aVrts[0].sx = tileScreenInfo[i+0][j+1].wx;
   		aVrts[0].sy = tileScreenInfo[i+0][j+1].wy;
   		aVrts[0].sz = tileScreenInfo[i+0][j+1].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[1],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
   		aVrts[1].sx = tileScreenInfo[i+1][j+1].wx;
   		aVrts[1].sy = tileScreenInfo[i+1][j+1].wy;
   		aVrts[1].sz = tileScreenInfo[i+1][j+1].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
   		aVrts[2].sx = tileScreenInfo[i+1][j+0].wx;
   		aVrts[2].sy = tileScreenInfo[i+1][j+0].wy;
   		aVrts[2].sz = tileScreenInfo[i+1][j+0].wz - WATER_EDGE_ZOFFSET;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle(&aVrts[0],&aVrts[1],&aVrts[2],
								&texturePage,0,pie_ADDITIVE);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}
	else
	{
   		memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
   		aVrts[0].sx = tileScreenInfo[i+0][j+0].wx;
   		aVrts[0].sy = tileScreenInfo[i+0][j+0].wy;
   		aVrts[0].sz = tileScreenInfo[i+0][j+0].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[1],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
   		aVrts[1].sx = tileScreenInfo[i+1][j+1].wx;
   		aVrts[1].sy = tileScreenInfo[i+1][j+1].wy;
   		aVrts[1].sz = tileScreenInfo[i+1][j+1].wz - WATER_EDGE_ZOFFSET;
   		memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
   		aVrts[2].sx = tileScreenInfo[i+1][j+0].wx;
   		aVrts[2].sy = tileScreenInfo[i+1][j+0].wy;
   		aVrts[2].sz = tileScreenInfo[i+1][j+0].wz - WATER_EDGE_ZOFFSET;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle(&aVrts[0],&aVrts[1],&aVrts[2],
								&texturePage,0,pie_ADDITIVE);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, NULL);
		}
	}
}


// Render a water tile.
//
void drawTerrainWaterTile(UDWORD i, UDWORD j)	//hardware only
{
	UDWORD	actualX,actualY;
	MAPTILE	*psTile;
	//BOOL	bOutlined;
	UDWORD	tileNumber;
	//UDWORD	renderFlag;
	iPoint	offset;
	PIEVERTEX aVrts[3];


	/* Get the correct tile index for the x coordinate */
	actualX = playerXTile + j;
	/* Get the correct tile index for the y coordinate */
	actualY = playerZTile + i;

	/* Let's just get out now if we're not supposed to draw it */
	if( (actualX<0) OR
		(actualY<0) OR
		(actualX>mapWidth-1) OR
		(actualY>mapHeight-1) )
	{
		return;
//		psTile = &edgeTile;
//		CLEAR_TILE_HIGHLIGHT(psTile);
	}

	psTile = mapTile(actualX,actualY);

	// If it's a water tile then draw the water
	if(TERRAIN_TYPE(psTile) == TER_WATER) {
		tileNumber = getWaterTileNum();
		// Draw the main water tile.
	
		/* 3dfx is pre stored and indexed */
		pie_SetTexturePage(tileTexInfo[tileNumber & TILE_NUMMASK].texPage);

		offset.x = (tileTexInfo[tileNumber & TILE_NUMMASK].xOffset * 64);
		offset.y = (tileTexInfo[tileNumber & TILE_NUMMASK].yOffset * 64);

		tileScreenInfo[i+0][j+0].tu = (UWORD)(offset.x + 1);
		tileScreenInfo[i+0][j+0].tv = (UWORD)(offset.y + vOffset);

		tileScreenInfo[i+0][j+1].tu = (UWORD)(offset.x + 63);
		tileScreenInfo[i+0][j+1].tv = (UWORD)(offset.y + vOffset);
			
		tileScreenInfo[i+1][j+1].tu = (UWORD)(offset.x + 63);
		tileScreenInfo[i+1][j+1].tv = (UWORD)(offset.y + 31 + vOffset);

		tileScreenInfo[i+1][j+0].tu = (UWORD)(offset.x + 1);
		tileScreenInfo[i+1][j+0].tv = (UWORD)(offset.y + 31 + vOffset);

		memcpy(&aVrts[0],&tileScreenInfo[i+0][j+0],sizeof(PIEVERTEX));
		aVrts[0].sx = tileScreenInfo[i+0][j+0].wx;
		aVrts[0].sy = tileScreenInfo[i+0][j+0].wy;
		aVrts[0].sz = tileScreenInfo[i+0][j+0].wz - WATER_ZOFFSET;
		aVrts[0].light = tileScreenInfo[i+0][j+0].wlight;
		aVrts[0].light.byte.a = pie_ByteScale((UBYTE)WATER_ALPHA_LEVEL,
			(UBYTE)(255-tileScreenInfo[i+0][j+0].light.byte.a));

		memcpy(&aVrts[1],&tileScreenInfo[i+0][j+1],sizeof(PIEVERTEX));
		aVrts[1].sx = tileScreenInfo[i+0][j+1].wx;
		aVrts[1].sy = tileScreenInfo[i+0][j+1].wy;
		aVrts[1].sz = tileScreenInfo[i+0][j+1].wz - WATER_ZOFFSET;
		aVrts[1].light = tileScreenInfo[i+0][j+1].wlight;
		aVrts[1].light.byte.a = pie_ByteScale((UBYTE)WATER_ALPHA_LEVEL,
			(UBYTE)(255-tileScreenInfo[i+0][j+1].light.byte.a));

		memcpy(&aVrts[2],&tileScreenInfo[i+1][j+1],sizeof(PIEVERTEX));
		aVrts[2].sx = tileScreenInfo[i+1][j+1].wx;
		aVrts[2].sy = tileScreenInfo[i+1][j+1].wy;
		aVrts[2].sz = tileScreenInfo[i+1][j+1].wz - WATER_ZOFFSET;
		aVrts[2].light = tileScreenInfo[i+1][j+1].wlight;
		aVrts[2].light.byte.a = pie_ByteScale((UBYTE)WATER_ALPHA_LEVEL,
			(UBYTE)(255-tileScreenInfo[i+1][j+1].light.byte.a));

		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle(&aVrts[0],&aVrts[1],&aVrts[2],
								&texturePage,WATER_TRANS_MODE,WATER_ALPHA_LEVEL);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, &waterAlphaValue);//jps 15 apr99
		}


		memcpy(&aVrts[1],&aVrts[2],sizeof(PIEVERTEX));

		memcpy(&aVrts[2],&tileScreenInfo[i+1][j+0],sizeof(PIEVERTEX));
		aVrts[2].sx = tileScreenInfo[i+1][j+0].wx;
		aVrts[2].sy = tileScreenInfo[i+1][j+0].wy;
		aVrts[2].sz = tileScreenInfo[i+1][j+0].wz - WATER_ZOFFSET;
		aVrts[2].light = tileScreenInfo[i+1][j+0].wlight;
		aVrts[2].light.byte.a = pie_ByteScale((UBYTE)WATER_ALPHA_LEVEL,
			(UBYTE)(255-tileScreenInfo[i+1][j+0].light.byte.a));

		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			pie_DrawFastTriangle(&aVrts[0],&aVrts[1],&aVrts[2],
								&texturePage,WATER_TRANS_MODE,WATER_ALPHA_LEVEL);
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			pie_DrawPoly(3, aVrts, tileTexInfo[tileNumber & TILE_NUMMASK].texPage, &waterAlphaValue);//jps 15 apr99
		}

		if( (psTile->texture & TILE_NUMMASK) != WaterTileID) {
			drawTerrainWEdgeTile(i,j);
		}
	}
}




// -------------------------------------------------------------------------------------
UDWORD	getSuggestedPitch( void )
{
UDWORD	worldAngle;
UDWORD	xPos,yPos;
SDWORD	pitch;

	worldAngle = (UDWORD) ((UDWORD)player.r.y/DEG_1)%360;
 	/* Now, we need to track angle too - to avoid near z clip! */

 	xPos = (player.p.x + ((visibleXTiles/2)*TILE_UNITS));
 	yPos = (player.p.z + ((visibleYTiles/2)*TILE_UNITS));
// 	getBestPitchToEdgeOfGrid(xPos,yPos,360-worldAngle,&pitch);
 	getPitchToHighestPoint(xPos,yPos,360-worldAngle,0,&pitch);

	if(pitch<abs(MAX_PLAYER_X_ANGLE)) pitch = abs(MAX_PLAYER_X_ANGLE);
	if(pitch>abs(MIN_PLAYER_X_ANGLE)) pitch = abs(MIN_PLAYER_X_ANGLE);
	
	return(pitch);
}
// -------------------------------------------------------------------------------------
void	trackHeight( SDWORD desiredHeight )
{
FRACT	fraction;
UDWORD	pitch;
SDWORD	angConcern;
UDWORD	desPitch;

		/* What fraction of a second did last game loop take */
		fraction = (MAKEFRACT(frameTime2) / (FRACT)GAME_TICKS_PER_SEC);

		/* How far are we from desired hieght? */
		separation = (FRACT)(desiredHeight - player.p.y);

		/* Work out accelertion... */
		acceleration = MAKEINT(((ACCEL_CONSTANT*2)*separation - (VELOCITY_CONSTANT)*(FRACT)heightSpeed));

		/* ...and now speed */
		heightSpeed += MAKEINT(((FRACT)acceleration * fraction));

		/* Adjust the height accordingly */
		player.p.y += MAKEINT(((FRACT)heightSpeed * fraction));

		/* Now do auto pitch as well, but only if we're not using mouselook and not tracking */
		if(!getWarCamStatus() AND !getRotActive())
		{
			/* Get the suggested pitch */
			pitch = getSuggestedPitch();

			/* Make sure this isn't negative */
			while(player.r.x<0)
			{
				player.r.x+=DEG(360);
			}
		   
			/* Or too much */
			while(player.r.x > DEG(360))
			{
				player.r.x -= DEG(360);
			}
			
			/* What's the desired pitch from the player */
			desPitch = (360-getDesiredPitch());

			/* Do nothing if we're within 2 degrees of optimum */
			if(abs(pitch-desPitch) < 2) // near enough
			{
					/*NOP*/		  
			}	

			/* Force adjust if too low - stops near z clip */
			else if(pitch>desPitch)
			{
				angConcern = DEG(360-pitch);
				aSep = (FRACT)(angConcern-player.r.x);
				aAccel = MAKEINT((((ACCEL_CONSTANT))*aSep - (VELOCITY_CONSTANT)*(FRACT)aSpeed));
				aSpeed += MAKEINT(((FRACT)aAccel * fraction));
				player.r.x += MAKEINT(((FRACT)aSpeed * fraction));
			}
			else 
			{
				/* Else, move towards player's last selected pitch */
				angConcern = DEG(360-desPitch);
				aSep = (FRACT)(angConcern-player.r.x);
				aAccel = MAKEINT((((ACCEL_CONSTANT))*aSep - (VELOCITY_CONSTANT)*(FRACT)aSpeed));
				aSpeed += MAKEINT(((FRACT)aAccel * fraction));
				player.r.x += MAKEINT(((FRACT)aSpeed * fraction));
			}

//			flushConsoleMessages();
//			CONPRINTF(ConsoleString,(ConsoleString,"Player.r.x : %d",player.r.x/182));
//			CONPRINTF(ConsoleString,(ConsoleString,"Pitch : %d",pitch));
		}

}
// -------------------------------------------------------------------------------------
void	toggleEnergyBars( void )
{
	if(++barMode>BAR_DOT)
	{
		barMode = BAR_FULL;
	}
//	bEnergyBars = !bEnergyBars;
}
// -------------------------------------------------------------------------------------
void	toggleReloadBarDisplay( void )
{
	bReloadBars = !bReloadBars;
}
// -------------------------------------------------------------------------------------
//void	assignSensorTarget( DROID *psDroid )
void assignSensorTarget( BASE_OBJECT *psObj )
{
	bSensorTargetting = TRUE;
	lastTargetAssignation = gameTime2;
	//psSensorDroid = psDroid;
	psSensorObj = psObj;
}

// -------------------------------------------------------------------------------------
void	assignDestTarget( void )
{
	bDestTargetting = TRUE;
	lastDestAssignation = gameTime2;
	destTargetX = mouseX();
	destTargetY = mouseY();
	destTileX = mouseTileX;
	destTileY = mouseTileY;
}
// -------------------------------------------------------------------------------------
void	processSensorTarget( void )
{
	SWORD x,y;
	SWORD offset;
	SWORD x0,y0,x1,y1;
	UDWORD	index;


	if(bSensorTargetting)
	{
		if( (gameTime2 - lastTargetAssignation) < TARGET_TO_SENSOR_TIME)
		{
			//if(!psSensorDroid->died)
			if(!psSensorObj->died AND psSensorObj->sDisplay.frameNumber == currentGameFrame)
			{
				x = /*mouseX();*/(SWORD)psSensorObj->sDisplay.screenX;
				y = (SWORD)psSensorObj->sDisplay.screenY;
				if(!gamePaused())
				{
					index = IMAGE_BLUE1+getStaticTimeValueRange(1020,5);
				}
				else
				{
					index = IMAGE_BLUE1;
				}
				iV_DrawTransImage(IntImages,index,x,y);

				offset = (SWORD)(12+ ((TARGET_TO_SENSOR_TIME)-(gameTime2-
					lastTargetAssignation))/2);

				x0 = (SWORD)(x-offset);
				y0 = (SWORD)(y-offset);
				x1 = (SWORD)(x+offset);
				y1 = (SWORD)(y+offset);

				
				iV_Line(x0,y0,x0+8,y0,COL_WHITE);
				iV_Line(x0,y0,x0,y0+8,COL_WHITE);

				iV_Line(x1,y0,x1-8,y0,COL_WHITE);
				iV_Line(x1,y0,x1,y0+8,COL_WHITE);

				iV_Line(x1,y1,x1-8,y1,COL_WHITE);
				iV_Line(x1,y1,x1,y1-8,COL_WHITE);

				iV_Line(x0,y1,x0+8,y1,COL_WHITE);
				iV_Line(x0,y1,x0,y1-8,COL_WHITE);
			}
			else
			{
				bSensorTargetting = FALSE;
			}
		}
		else
		{
			bSensorTargetting = FALSE;
		}
	}

}
// -------------------------------------------------------------------------------------
void	processDestinationTarget( void )
{
	SWORD x,y;
	SWORD offset;
	SWORD x0,y0,x1,y1;


	if(bDestTargetting)
	{
		if( (gameTime2 - lastDestAssignation) < DEST_TARGET_TIME)
		{
				x = (SWORD)destTargetX;
				y = (SWORD)destTargetY;

				offset = (SWORD)(((DEST_TARGET_TIME)-(gameTime2-lastDestAssignation))/2);

				x0 = (SWORD)(x-offset);
				y0 = (SWORD)(y-offset);
				x1 = (SWORD)(x+offset);
				y1 = (SWORD)(y+offset);

				iV_BoxFill(x0,y0,x0+2,y0+2,COL_WHITE);
		//		iV_Line(x0,y0,x0,y0+2,COL_WHITE);

				iV_BoxFill(x1-2,y0-2,x1,y0,COL_WHITE);
		//		iV_Line(x1,y0,x1,y0+2,COL_WHITE);

				iV_BoxFill(x1-2,y1-2,x1,y1,COL_WHITE);
		//		iV_Line(x1,y1,x1,y1-2,COL_WHITE);

				iV_BoxFill(x0,y1,x0+2,y1+2,COL_WHITE);
		//		iV_Line(x0,y1,x0,y1-2,COL_WHITE);
		}
		else
		{
			bDestTargetting = FALSE;
		}
	}
}
// -------------------------------------------------------------------------------------
void	setEnergyBarDisplay( BOOL val)
{
	bEnergyBars = val;
}
// -------------------------------------------------------------------------------------
void	setUnderwaterTile(UDWORD num)
{
	underwaterTile = num;
}
// -------------------------------------------------------------------------------------
void	setRubbleTile(UDWORD num)
{
	rubbleTile = num;
}
// -------------------------------------------------------------------------------------
UDWORD	getWaterTileNum( void )
{
	return(underwaterTile);
}
// -------------------------------------------------------------------------------------
UDWORD	getRubbleTileNum( void )
{
	return(rubbleTile);
}
// -------------------------------------------------------------------------------------

UDWORD	lastSpinVal;

void	testEffect2( UDWORD player )
{
	SDWORD	val;
	SDWORD	radius;
	UDWORD	angle;
	STRUCTURE	*psStructure;
	SDWORD	xDif,yDif;
	iVector	pos;
	UDWORD	numConnected;
	POWER_GEN	*psPowerGen;
	DROID	*psDroid;
	UDWORD	gameDiv;
	UDWORD	i;
	REARM_PAD	*psReArmPad;
	BASE_OBJECT			*psChosenObj = NULL;
	UWORD	bFXSize;


	for(psStructure = apsStructLists[player]; psStructure; psStructure = psStructure->psNext)
	{
		if(psStructure->status == SS_BUILT)
		{
			if(psStructure->pStructureType->type == REF_POWER_GEN AND psStructure->visible[selectedPlayer])
			{
				psPowerGen = (POWER_GEN *)psStructure->pFunctionality;
				numConnected = 0;
				for (i = 0; i < NUM_POWER_MODULES; i++)
				{
					if (psPowerGen->apResExtractors[i])
					{
						numConnected++;
					}
				}
	   			/* No effect if nothing connected */
				if(!numConnected)
				{
					//return;
                    //keep looking for another!
                    continue;
				}
		
				else switch(numConnected)
				{	
				case 1:
				case 2:
					gameDiv = 1440;
					val = 4;
					break;
				case 3:
				case 4:
				default:
					gameDiv = 1080;
					val = 3;	  // really fast!!!
					break;
				}

	   		
				angle = gameTime2%gameDiv;
				val = angle/val;   
				
				/* New addition - it shows how many are connected... */
				for(i=0 ;i<numConnected; i++)
				{
		 			radius = 32 - (i*2);	// around the spire
					xDif = radius * (SIN(DEG(val)));
					yDif = radius * (COS(DEG(val)));

					xDif = xDif/4096;	 // cos it's fixed point
					yDif = yDif/4096;

					pos.x = psStructure->x + xDif; 
					pos.z = psStructure->y + yDif;
					pos.y = map_Height(pos.x,pos.z) + 64 + (i*20);	// 64 up to get to base of spire
					effectGiveAuxVar(50);	// half normal plasma size...
					addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LASER,FALSE,NULL,0);

					pos.x = psStructure->x - xDif; 
					pos.z = psStructure->y - yDif;
//					pos.y = map_Height(pos.x,pos.z) + 64 + (i*20);	// 64 up to get to base of spire
					effectGiveAuxVar(50);	// half normal plasma size...
					
					addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LASER,FALSE,NULL,0);
				}
			}
			/* Might be a re-arm pad! */
			else if(psStructure->pStructureType->type == REF_REARM_PAD 
				AND psStructure->visible[selectedPlayer] )
			{
			 	psReArmPad = (REARM_PAD *) psStructure->pFunctionality;
				psChosenObj = psReArmPad->psObj;
				if(psChosenObj!=NULL)
				{
					if((((DROID*)psChosenObj)->visible[selectedPlayer]))
					{
				   		bFXSize = 0;
						psDroid = (DROID*) psChosenObj;
						if(!psDroid->died AND psDroid->action == DACTION_WAITDURINGREARM )
						{
				   			bFXSize = 30;

						}
				  		/* Then it's repairing...? */
						if(!gamePaused())
						{
							val = lastSpinVal = getTimeValueRange(720,360);	// grab an angle - 4 seconds cyclic
						}
						else
						{
							val = lastSpinVal;
						}
						radius = psStructure->sDisplay.imd->radius;	
						xDif = radius * (SIN(DEG(val)));
						yDif = radius * (COS(DEG(val)));
						xDif = xDif/4096;	 // cos it's fixed point
						yDif = yDif/4096;
						pos.x = psStructure->x + xDif; 
						pos.z = psStructure->y + yDif;
						pos.y = map_Height(pos.x,pos.z) + psStructure->sDisplay.imd->ymax; 
						effectGiveAuxVar(30+bFXSize);	// half normal plasma size...
						addEffect(&pos,EFFECT_EXPLOSION, EXPLOSION_TYPE_LASER,FALSE,NULL,0);
						pos.x = psStructure->x - xDif; 
						pos.z = psStructure->y - yDif;	// buildings are level!
		//				pos.y = map_Height(pos.x,pos.z) + psStructure->sDisplay->ymax; 
						effectGiveAuxVar(30+bFXSize);	// half normal plasma size...
						addEffect(&pos,EFFECT_EXPLOSION, EXPLOSION_TYPE_LASER,FALSE,NULL,0);
					}
				}	   	

			}
		}

	}
}


void	testEffect( void )
{
	UDWORD	i;

	/* Hardware only effect, and then only if you've got additive! */
	if((pie_Hardware() AND war_GetAdditive()) )
	{
		/* Only do for player 0 power stations */

		if(bMultiPlayer)
		{
			for(i=0;i<MAX_PLAYERS;i++)
			{
				 if(isHumanPlayer(i) AND apsStructLists[i] )
				 {
					testEffect2(i);
				 }

			}
		}
		else if(apsStructLists[0])
		{
			testEffect2(0);
		}
	}
}


void	showDroidSensorRanges(void)
{
DROID		*psDroid;
STRUCTURE	*psStruct;

	for(psDroid= apsDroidLists[selectedPlayer]; psDroid; psDroid=psDroid->psNext)
	{
//		if(psDroid->selected)
		{
			showSensorRange2((BASE_OBJECT*)psDroid);
		}
	}

	for(psStruct = apsStructLists[selectedPlayer]; psStruct; psStruct = psStruct->psNext)
	{
//		if(psStruct->selected)
		{
			showSensorRange2((BASE_OBJECT*)psStruct);
		}
	}
}

void	showSensorRange1(DROID *psDroid)
{
SDWORD	val;
SDWORD	radius;
UDWORD	angle;
SDWORD	xDif,yDif;
UDWORD	sensorRange;
iVector	pos;

	angle = gameTime%3600;
	val = angle/10;   
	sensorRange = asSensorStats[psDroid->asBits[COMP_SENSOR].nStat].range;
	radius = sensorRange;
	xDif = radius * (SIN(DEG(val)));
	yDif = radius * (COS(DEG(val)));

	xDif = xDif/4096;	 // cos it's fixed point
	yDif = yDif/4096;
   	pos.x = psDroid->x - xDif; 
	pos.z = psDroid->y - yDif;
	pos.y = map_Height(pos.x,pos.z)+ 16;	// 64 up to get to base of spire
	effectGiveAuxVar(80);	// half normal plasma size...
	addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LASER,FALSE,NULL,0);
}

void	showSensorRange2(BASE_OBJECT *psObj)
{
SDWORD	radius;
SDWORD	xDif,yDif;
UDWORD	sensorRange;
iVector	pos;
UDWORD	i;
DROID	*psDroid;
STRUCTURE	*psStruct;
BOOL	bBuilding=FALSE;

	for(i=0; i<360; i++)
	{
		if(psObj->type == OBJ_DROID)
		{
		 	psDroid = (DROID*)psObj;
			sensorRange = asSensorStats[psDroid->asBits[COMP_SENSOR].nStat].range;
		}
		else
		{
			psStruct = (STRUCTURE*)psObj;
			sensorRange = psStruct->sensorRange;
			bBuilding = TRUE;
		}

		radius = sensorRange;
		xDif = radius * (SIN(DEG(i)));
		yDif = radius * (COS(DEG(i)));

		xDif = xDif/4096;	 // cos it's fixed point
		yDif = yDif/4096;
   		pos.x = psObj->x - xDif; 
		pos.z = psObj->y - yDif;
		pos.y = map_Height(pos.x,pos.z)+ 16;	// 64 up to get to base of spire
		effectGiveAuxVar(80);	// half normal plasma size...
		if(bBuilding)
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}
		else
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LASER,FALSE,NULL,0);
		}
	}
}

void debugToggleSensorDisplay( void )
{
	if(bSensorDisplay)
	{
		bSensorDisplay = FALSE;
	}
	else
	{
		bSensorDisplay = TRUE;
	}
}

/*returns the graphic ID for a droid rank*/
UDWORD  getDroidRankGraphic(DROID *psDroid)
{
    UDWORD gfxId; 
	/* Not found yet */
	gfxId = UDWORD_MAX;

//#ifdef JOHN
	/* Establish the numerical value of the droid's rank */
	switch(getDroidLevel(psDroid))
	{
		case 0:
			break;
		case 1:
			gfxId = IMAGE_LEV_0;
			break;
		case 2:
			gfxId = IMAGE_LEV_1;
			break;
		case 3:
			gfxId = IMAGE_LEV_2;
			break;
		case 4:
			gfxId = IMAGE_LEV_3;
			break;
		case 5:
			gfxId = IMAGE_LEV_4;
			break;
		case 6:
			gfxId = IMAGE_LEV_5;
			break;
		case 7:
			gfxId = IMAGE_LEV_6;
			break;
		case 8:
			gfxId = IMAGE_LEV_7;
			break;
		default:
			ASSERT((FALSE, "Weird droid level in drawDroidRank"));
		break;
	}
//#else
  	/*
	switch(getDroidLevel(psDroid))
	{
		case 0:
//			gfxId = IMAGE_GN_0;	// Unexperienced 
			break;
		case 1:
			gfxId = IMAGE_GN_1;
			break;
		case 2:
			gfxId = IMAGE_GN_2;
			break;
		case 3:
			gfxId = IMAGE_GN_3;
			break;
		case 4:
			gfxId = IMAGE_GN_4;
			break;
		case 5:
			gfxId = IMAGE_GN_5;
			break;
		case 6:
			gfxId = IMAGE_GN_6;	// Experienced
			break;
		case 7:
			gfxId = IMAGE_GN_7;
			break;
		case 8:
			gfxId = IMAGE_GN_8;
			break;
		default:
			ASSERT((FALSE, "Weird droid level in drawDroidRank"));
		break;
	}
	*/
//#endif


/*	John's routing debug code
	switch (psDroid->sMove.Status)
	{
		case MOVEINACTIVE:
			gfxId = IMAGE_GN_0;	// Unexperienced 
			break;
		case MOVENAVIGATE:
			gfxId = IMAGE_GN_1;
			break;
		case MOVETURN:
			gfxId = IMAGE_GN_2;
			break;
		case MOVEPAUSE:
			gfxId = IMAGE_GN_3;
			break;
		case MOVEPOINTTOPOINT:
			gfxId = IMAGE_GN_4;
			break;
		case MOVEROUTE:
			gfxId = IMAGE_GN_5;
			break;
		case MOVEWAITROUTE:
			gfxId = IMAGE_GN_6;	// Experienced
			break;
		case MOVESHUFFLE:
			gfxId = IMAGE_GN_7;
			break;
		case MOVEROUTESHUFFLE:
			gfxId = IMAGE_GN_8;
			break;
		default:
		break;
	}*/

    return gfxId;
}

/*	DOES : Assumes matrix context set and that z-buffer write is force enabled (Always). 
	Will render a graphic depiction of the droid's present rank.
	BY : Alex McLean.
*/
void	drawDroidRank(DROID *psDroid)
{
//UDWORD	droidLevel;
UDWORD	gfxId;

    gfxId = getDroidRankGraphic(psDroid);

 	/* Did we get one? - We should have... */
	if(gfxId!=UDWORD_MAX)
	{
		/* Render the rank graphic at the correct location */ // remove hardcoded numbers?!
		iV_DrawTransImage(IntImages,(UWORD)gfxId,psDroid->sDisplay.screenX+20,psDroid->sDisplay.screenY+8);
	}
}

/*	DOES : Assumes matrix context set and that z-buffer write is force enabled (Always). 
	Will render a graphic depiction of the droid's present rank.
*/
void	drawDroidSensorLock(DROID *psDroid)
{
	//if on fire support duty - must be locked to a Sensor Droid/Structure
	if (orderState(psDroid, DORDER_FIRESUPPORT))
	{
		/* Render the sensor graphic at the correct location - which is what?!*/
		iV_DrawTransImage(IntImages,IMAGE_GN_STAR,psDroid->sDisplay.screenX+20,
			psDroid->sDisplay.screenY-20);
	}
}

static	void	doConstructionLines( void )
{
DROID	*psDroid;
UDWORD	i;

//	pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
  //	pie_SetFogStatus(FALSE);

	for(i=0; i<MAX_PLAYERS; i++)
	{
		for(psDroid= apsDroidLists[i]; psDroid; psDroid = psDroid->psNext)
		{
			if(clipXY(psDroid->x,psDroid->y))
			{

				if( (psDroid->visible[selectedPlayer]==UBYTE_MAX) AND 
 					(psDroid->sMove.Status != MOVESHUFFLE) ) 
				{
					if(psDroid->action == DACTION_BUILD)
					{
						if(psDroid->psTarget)
						{
							if(psDroid->psTarget->type == OBJ_STRUCTURE)
							{
								addConstructionLine(psDroid, (STRUCTURE*)psDroid->psTarget);
							}
						}
					}	
					
					else if ((psDroid->action == DACTION_DEMOLISH) OR
							(psDroid->action == DACTION_REPAIR) OR
							(psDroid->action == DACTION_CLEARWRECK) OR
							(psDroid->action == DACTION_RESTORE))
					{
						if(psDroid->psActionTarget)
						{
							if(psDroid->psActionTarget->type == OBJ_STRUCTURE)
							{
								addConstructionLine(psDroid, (STRUCTURE*)psDroid->psActionTarget);
							}
						}
					}
				}

			}
		}
	}
   	pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);

}

static	void	addConstructionLine(DROID	*psDroid, STRUCTURE *psStructure)
{
PIEVERTEX	pts[3];
iPoint	pt1,pt2,pt3;
UDWORD	pt1Z,pt2Z,pt3Z;
iVector each;	
iVector	*point;
UDWORD	pointIndex;
SDWORD	realY;
iVector	null,vec;
SDWORD	rx,rz;
UDWORD	colour;
UDWORD	specular;
UDWORD	trans;

trans = 0; //Defining the variable trans to eleminate a runtime debug error
//SDWORD	centreX,centreZ;

	null.x = null.y = null.z = 0;
	each.x = psDroid->x;
	each.y = psDroid->z + 24;
	each.z = psDroid->y;

	vec.x = (each.x - player.p.x) - terrainMidX*TILE_UNITS;
	vec.z = terrainMidY*TILE_UNITS - (each.z - player.p.z);
	vec.y = each.y;

	pie_MatBegin();
	pie_TRANSLATE(vec.x,vec.y,vec.z);
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);
	pie_TRANSLATE(rx,0,-rz);
	pt1Z = pie_RotProj(&null,&pt1);
	pie_MatEnd();

	pointIndex = rand()%(psStructure->sDisplay.imd->npoints-1);
	point = &(psStructure->sDisplay.imd->points[pointIndex]);

	each.x = psStructure->x + point->x;
	realY = MAKEINT((structHeightScale(psStructure) * point->y));
	each.y = psStructure->z + realY;
	each.z = psStructure->y - point->z;

	if(ONEINEIGHT)
	{
		effectSetSize(30);
  //	 	if(rand()%2)
  //		{
			addEffect(&each,EFFECT_EXPLOSION,EXPLOSION_TYPE_SPECIFIED,TRUE,getImdFromIndex(MI_PLASMA),0);
  //		}
  //		else
  //		{
  //			addEffect(&each,EFFECT_SMOKE,SMOKE_TYPE_DRIFTING,FALSE,NULL,0);
  //		}
	}

	vec.x = (each.x - player.p.x) - terrainMidX*TILE_UNITS;
	vec.z = terrainMidY*TILE_UNITS - (each.z - player.p.z);
	vec.y = each.y;

	pie_MatBegin();
	pie_TRANSLATE(vec.x,vec.y,vec.z);
	rx = player.p.x & (TILE_UNITS-1);

	rz = player.p.z & (TILE_UNITS-1);
	pie_TRANSLATE(rx,0,-rz);
	pt2Z = pie_RotProj(&null,&pt2);
	pie_MatEnd();

	pointIndex = rand()%(psStructure->sDisplay.imd->npoints-1);
	point = &(psStructure->sDisplay.imd->points[pointIndex]);

	each.x = psStructure->x + point->x;
	realY = MAKEINT((structHeightScale(psStructure) * point->y));
	each.y = psStructure->z + realY;
	each.z = psStructure->y - point->z;

	vec.x = (each.x - player.p.x) - terrainMidX*TILE_UNITS;
	vec.z = terrainMidY*TILE_UNITS - (each.z - player.p.z);
	vec.y = each.y;

	pie_MatBegin();
	pie_TRANSLATE(vec.x,vec.y,vec.z);
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);
	pie_TRANSLATE(rx,0,-rz);
	pt3Z = pie_RotProj(&null,&pt3);
	pie_MatEnd();

//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

  //	colour = lightDoFogAndIllumination(255,centreX - psDroid->x, centreZ - psDroid->z,&specular);
//	   rx-tileXYZ.x,rz - ((i-terrainMidY)<<TILE_SHIFT),&specular);

	// set the colour
	colour = UBYTE_MAX;
	colour = lightDoFogAndIllumination(colour,getCentreX() - psDroid->x, getCentreZ() - psDroid->y,&specular);

	if	(pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		if (war_GetFog())
		{
			trans = colour >> 24;//alpha 
			trans &= 0xff;
			trans = UBYTE_MAX - trans;
		}
		else
		{
			trans = colour & 0xff;
		}
		trans >>= 1;//divide by 2
		if (	(psDroid->action == DACTION_DEMOLISH) OR
			(psDroid->action == DACTION_CLEARWRECK) )
		{
			colour = 0x00ff0000;//red
		}
		else
		{
			colour = 0x000000ff;//blue
		}
		pts[0].light.argb = 0x00000000;
		pts[1].light.argb = 0x00000000;
		pts[2].light.argb = 0x00000000;
	}
	else
	{
		colour &= 0xff;
		if (	(psDroid->action == DACTION_DEMOLISH) OR
		(psDroid->action == DACTION_CLEARWRECK) )
		{
			colour <<= 16;//red
		}
		pts[0].light.argb = 0xff000000;
		pts[1].light.argb = 0xff000000;
		pts[2].light.argb = 0xff000000;
	}


  	pts[0].sx = pt1.x;
	pts[0].sy = pt1.y;
	pts[0].sz = pt1Z;
	pts[0].tu = 0;
	pts[0].tv = 0;
	pts[0].specular.argb = colour;

	pts[1].sx = pt2.x;
	pts[1].sy = pt2.y;
	pts[1].sz = pt2Z;
	pts[1].tu = 0;
	pts[1].tv = 0;
	pts[1].specular.argb = 0;
	
	pts[2].sx = pt3.x;
	pts[2].sy = pt3.y;
	pts[2].sz = pt3Z;
	pts[2].tu = 0;
	pts[2].tv = 0;
	pts[2].specular.argb = 0;

 
/* Only do if at least one point is on-screen */
	if( ((pts[0].sx>0 AND pts[0].sx<DISP_WIDTH) AND	(pts[0].sy>0 AND pts[0].sy<DISP_HEIGHT)) OR
	   ( (pts[1].sx>0 AND pts[1].sx<DISP_WIDTH) AND	(pts[1].sy>0 AND pts[1].sy<DISP_HEIGHT)) OR
		((pts[2].sx>0 AND pts[2].sx<DISP_WIDTH) AND (pts[2].sy>0 AND pts[2].sy<DISP_HEIGHT)) )
	{
		pie_TransColouredTriangle((PIEVERTEX*)&pts,colour,trans);
	}
	return;

/*

	pts[0].sx = psDroid->x;
	pts[0].sy = psDroid->z + 24;
	pts[0].sz = psDroid->y;

	pointIndex = rand()%(psStructure->sDisplay.imd->npoints-1);
	point = &(psStructure->sDisplay.imd->points[pointIndex]);

	pts[1].sx = psStructure->x + point->x;
	realY = MAKEINT((structHeightScale(psStructure) * point->y));
	pts[1].sy = psStructure->z + realY;
	pts[1].sz = psStructure->y - point->z;

	pointIndex = rand()%(psStructure->sDisplay.imd->npoints-1);
	point = &(psStructure->sDisplay.imd->points[pointIndex]);

	pts[2].sx = psStructure->x + point->x;
	realY = MAKEINT((structHeightScale(psStructure) * point->y));
	pts[2].sy = psStructure->z + realY;
	pts[2].sz = psStructure->y - point->z;

	pie_TransColouredTriangle((PIEVERTEX*)&pts,0x00ff0000,128);

//	draw3dLine(&src,&dest,COL_WHITE);

//	if(rand()%5==1)
//	{	
//		effectGiveAuxVar(50);
//		addEffect(&dest,EFFECT_EXPLOSION,EXPLOSION_TYPE_LASER,FALSE,NULL,0);
//	}
	point = &(psStructure->sDisplay.imd->points[pointIndex+1]);

	src.x = psStructure->x + point->x;
	realY = MAKEINT((structHeightScale(psStructure) * point->y));
	src.y = psStructure->z + realY;
	src.z = psStructure->y - point->z;
	draw3dLine(&src,&dest,COL_GREEN);
	*/
}

/*
UDWORD	getSelectionBoxSize( void )
{
FRACT	mul;
UDWORD	dif;
UDWORD	step;
UDWORD	min;
UDWORD	final;

	dif = DISTANCE - MINDISTANCE;
	step = distance - MINDISTANCE;

	mul = MAKEFRACT(step)/MAKEFRACT(dif);
}
*/