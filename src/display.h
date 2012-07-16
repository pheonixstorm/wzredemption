/*
 * Display.h
 *
 * Definitions for the display system structures and routines.
 *
 */



#ifndef _display_h
#define _display_h

#include "base.h"
#include "Structure.h"

/* Initialise the display system */
extern BOOL dispInitialise(void);

extern void shakeStart(void);

/* Tidy up after a mode change */
extern BOOL dispModeChange();

/* Process the user input. This just processes the key input and jumping around the radar*/
//extern BOOL processInput(void);

extern void ProcessRadarInput(void);

extern void processInput(void);
/*don't want to do any of these whilst in the Intelligence Screen*/
extern void processMouseClickInput(void);

extern void	scroll(void);

extern BOOL DrawnInLastFrame(SDWORD Frame);

BOOL LoadLevelGraphics(UBYTE LevelNumber);

// Clear all selections.
extern void clearSel(void);
// Clear all selections and stop driver mode.
extern void clearSelection(void);
// deal with selecting a droid
extern void dealWithDroidSelect(DROID *psDroid, BOOL bDragBox);

extern BOOL	buildingDamaged(STRUCTURE *psStructure);
extern	void	setInvertMouseStatus( BOOL val );
extern BOOL		getInvertMouseStatus( void );

extern	BOOL	getRadarJumpStatus( void );

extern	void	setRadarJump(BOOL	val);


/* Do the 3D display */
extern void displayWorld(void);

// Illumination value for standard light level "as the artist drew it" ... not darker, not lighter
#define ILLUMINATION_NONE (13)

//#define MAX_SCROLL_SPEED	1600
//#define SCROLL_SPEED_ACCEL	800

#define MAX_SCROLL_SPEED (800+scroll_speed_accel)	// make max speed dependant on accel chosen.

extern UDWORD scroll_speed_accel;			// now user modifyable.


#define DRAG_INACTIVE 0
#define DRAG_DRAGGING 1
#define DRAG_RELEASED 2
#define DRAG_PLACING  3

#define BOX_PULSE_SPEED	50

struct	_dragBox 
{
UDWORD	x1;
UDWORD	y1;
UDWORD	x2;
UDWORD	y2;
UDWORD	status;
UDWORD	lastTime;
UDWORD	boxColourIndex;
};

extern struct	_dragBox dragBox3D,wallDrag;

typedef enum _pointer
{
MP_ATTACH = 99,
MP_ATTACK,
MP_BRIDGE,
MP_BUILD,
MP_EMBARK,
MP_FIX,
MP_GUARD,
MP_JAM,
MP_MOVE,
MP_PICKUP,
MP_REPAIR,
MP_SELECT,
MP_LOCKON,
MP_MENSELECT,
MP_BOMB
} MOUSE_POINTER;

typedef enum _selectionTypes
{
SC_DROID_CONSTRUCT,
SC_DROID_DIRECT,
SC_DROID_INDIRECT,
SC_DROID_CLOSE,
SC_DROID_SENSOR,
SC_DROID_ECM,
SC_DROID_BRIDGE,
SC_DROID_RECOVERY,
SC_DROID_COMMAND,
SC_DROID_BOMBER,
SC_DROID_TRANSPORTER,
SC_DROID_DEMOLISH,
SC_DROID_REPAIR,
SC_INVALID,

} SELECTION_TYPE;

typedef enum _targets
{
MT_TERRAIN,
MT_RESOURCE,
MT_BLOCKING,
MT_RIVER,
MT_TRENCH,
MT_OWNSTRDAM,
MT_OWNSTROK,
MT_OWNSTRINCOMP,
MT_REPAIR,
MT_REPAIRDAM,
MT_ENEMYSTR,
MT_TRANDROID,
MT_OWNDROID,
MT_OWNDROIDDAM,
MT_ENEMYDROID,
MT_COMMAND,
MT_ARTIFACT,
MT_DAMFEATURE,
MT_SENSOR,
MT_WRECKFEATURE,
MT_CONSTRUCT,
MT_SENSORSTRUCT,
MT_SENSORSTRUCTDAM,

MT_NOTARGET		//leave as last one
} MOUSE_TARGET;


extern BOOL		mouseAtEdge;
extern BOOL		edgeOfMap;
extern BOOL		gameStats;
extern BOOL		bigBlueInWorld;
extern BOOL		missionComplete;
extern BOOL		godMode;
extern UWORD	RadarZoomLevel;



// reset the input state
void resetInput(void);

void ResetMouseBoundryConditions(void);
BOOL IsMouseAtBottom(void);
BOOL IsMouseAtTop(void);
BOOL IsMouseAtRight(void);
BOOL IsMouseAtLeft(void);
BOOL CheckObjInScrollLimits(UWORD *xPos,UWORD *zPos);
BOOL CheckInScrollLimits(SDWORD *xPos,SDWORD *zPos);
extern BOOL CheckScrollLimits(void);
//extern BOOL	widgetsOn;
extern BOOL	rotActive;
extern float	gamma;
//extern BOOL	forceWidgetsOn;

BASE_OBJECT	*mouseTarget( void );

extern PALETTEENTRY	gamePalette[255];

BOOL StartObjectOrbit(BASE_OBJECT *psObj);
void CancelObjectOrbit(void);

extern void FinishDeliveryPosition(UDWORD xPos,UDWORD yPos,void *UserData);
extern void CancelDeliveryRepos(void);
extern void StartDeliveryPosition(OBJECT_POSITION *psLocation,BOOL driveActive);
extern BOOL GetDeliveryRepos(UDWORD *xPos,UDWORD *yPos);
extern BOOL DeliveryReposValid(void);

extern void StartTacticalScroll(BOOL driveActive);
extern void StartTacticalScrollObj(BOOL driveActive,BASE_OBJECT *psObj);
extern void CancelTacticalScroll(void);
extern void MoveTacticalScroll(SDWORD xVel,SDWORD yVel);
extern BOOL	getRotActive( void );
extern SDWORD	getDesiredPitch( void );
extern void	setDesiredPitch(SDWORD pitch);

#define MAX_PLAYER_X_ANGLE	(-14)
#define MIN_PLAYER_X_ANGLE	(-50)

#define	HIDDEN_FRONTEND_WIDTH	(640)
#define	HIDDEN_FRONTEND_HEIGHT	(480)

//access function for bSensorAssigned variable
extern void setSensorAssigned(void);
extern void	setShakeStatus( BOOL val );
extern BOOL	getShakeStatus( void );

extern void	displayInitVars(void);

extern DROID *constructorDroidSelected(UDWORD player);

void BeepMessage(UDWORD StringID);
void AddDerrickBurningMessage(void);

// check whether the queue order keys are pressed
extern BOOL ctrlShiftDown(void);

#endif

