#ifndef _component_h
#define _component_h

/*
	Header file for component.c 
	Pumpkin Studios, EIDOS Interactive. 
*/

extern UBYTE PlayerColour[MAX_PLAYERS];// = {0,1,2,3,4,5,6,7}
extern void initPlayerColours(void);
extern BOOL setPlayerColour(UDWORD player, UDWORD col);
extern UBYTE getPlayerColour(UDWORD pl);

extern UDWORD getComponentDroidRadius(DROID *psDroid);
extern UDWORD getComponentDroidTemplateRadius(DROID_TEMPLATE *psDroid);
extern UDWORD getComponentRadius(BASE_STATS *psComponent);
extern UDWORD getResearchRadius(BASE_STATS *Stat);
extern UDWORD getStructureSize(STRUCTURE *psStructure);
extern UDWORD getStructureStatSize(STRUCTURE_STATS *Stats);

#define OBJECT_RADIUS				(128)
#define COMPONENT_RADIUS			(64)
#define DESIGN_DROID_SCALE			(200)
#define DESIGN_COMPONENT_SCALE		(150)
#define RESEARCH_COMPONENT_SCALE	(300)
#define COMP_BUT_SCALE				(100)
#define DROID_BUT_SCALE				(72)
#define SMALL_STRUCT_SCALE			(55)
#define MED_STRUCT_SCALE			(25)//reduced from 30 to fit command centre in window
#define LARGE_STRUCT_SCALE			(25)

#define TOWER_HEIGHT    100
extern UDWORD getStructureStatHeight(STRUCTURE_STATS *psStat);

extern void displayIMDButton(iIMDShape *IMDShape,
					     iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void displayStructureButton(STRUCTURE *psStructure,
						     iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void displayStructureStatButton(STRUCTURE_STATS *Stats,UDWORD Player,
						     iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void displayComponentButton(BASE_STATS *Stat,
						     iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void displayResearchButton(BASE_STATS *Stat,
						     iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void displayComponentButtonTemplate(DROID_TEMPLATE *psTemplate,
									iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void displayComponentButtonObject(DROID *psDroid,
								  iVector *Rotation,iVector *Position,BOOL RotXYZ, SDWORD scale);
extern void	displayComponentObject(BASE_OBJECT *psObj);

extern void	compPersonToBits(DROID *psDroid);

/* Pass in the stats you're interested in and the COMPONENT - double reference, but works */
#define PART_IMD(STATS,DROID,COMPONENT,PLAYER)	(STATS[DROID->asBits[COMPONENT].nStat].pIMD)


/* Get the chassis imd */
#define BODY_IMD(DROID,PLAYER)	(asBodyStats[DROID->asBits[COMP_BODY].nStat].pIMD)
/* Get the brain imd */
#define BRAIN_IMD(DROID,PLAYER)	(asBrainStats[DROID->asBits[COMP_BRAIN].nStat].pIMD)
/* Get the weapon imd */
#define WEAPON_IMD(DROID,PLAYER)	(asWeaponStats[DROID->asWeaps[0].nStat].pIMD)
/* Get the propulsion imd  THIS IS A LITTLE MORE COMPLICATED NOW!*/
//#define PROPULSION_IMD(DROID,PLAYER)	(asPropulsionStats[DROID->asBits[COMP_PROPULSION].nStat].pIMD[PLAYER])
/* Get the sensor imd */
#define SENSOR_IMD(DROID,PLAYER)	(asSensorStats[DROID->asBits[COMP_SENSOR].nStat].pIMD)
/* Get an ECM imd!?! */
#define ECM_IMD(DROID,PLAYER)	(asECMStats[DROID->asBits[COMP_ECM].nStat].pIMD)
/* Get an Repair imd!?! */
#define REPAIR_IMD(DROID,PLAYER)	(asRepairStats[DROID->asBits[COMP_REPAIRUNIT].nStat].pIMD)
/* Get a construct imd */
#define CONSTRUCT_IMD(DROID,PLAYER)	(asConstructStats[DROID->asBits[COMP_CONSTRUCT].nStat].pIMD)
/* Get a weapon mount imd*/
#define WEAPON_MOUNT_IMD(DROID,PLAYER)	(asWeaponStats[DROID->asWeaps[0].nStat].pMountGraphic)
/* Get a sensor mount imd*/
#define SENSOR_MOUNT_IMD(DROID,PLAYER)	(asSensorStats[DROID->asBits[COMP_SENSOR].nStat].pMountGraphic)
/* Get a construct mount imd*/
#define CONSTRUCT_MOUNT_IMD(DROID,PLAYER)	(asConstructStats[DROID->asBits[COMP_CONSTRUCT].nStat].pMountGraphic)
/* Get a ecm mount imd*/
#define ECM_MOUNT_IMD(DROID,PLAYER)	(asECMStats[DROID->asBits[COMP_ECM].nStat].pMountGraphic)
/* Get a repair mount imd*/
#define REPAIR_MOUNT_IMD(DROID,PLAYER)	(asRepairStats[DROID->asBits[COMP_REPAIRUNIT].nStat].pMountGraphic)
/* Get a muzzle flash pie*/
#define MUZZLE_FLASH_PIE(DROID,PLAYER)	(asWeaponStats[DROID->asWeaps[0].nStat].pMuzzleGraphic)

/* Don't know what these might be? */
//#define PROGRAM_IMD(DROID,PLAYER)	(DROID->asProgs[0]->pIMD)
extern BOOL		leftFirst;
extern UDWORD	droidScale;	//testing only - remove when decided
extern void	updateLightLevels(void);
extern SDWORD	rescaleButtonObject(SDWORD radius, SDWORD baseScale,SDWORD baseRadius);

extern void	destroyFXDroid(DROID	*psDroid);

#endif