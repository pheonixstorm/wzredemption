/*
 * Structure.h
 *
 * Definitions for the structures.
 *
 */
#ifndef _structure_h
#define _structure_h

#include "objectdef.h"
#include "structuredef.h"

// how long to wait between CALL_STRUCT_ATTACKED's - plus how long to flash on radar for
#define ATTACK_CB_PAUSE		5000

#define	STRUCTURE_DAMAGE_SCALING	400
// This should really be logarithmic
#define	CALC_STRUCTURE_SMOKE_INTERVAL(x) ((((100-x)+10)/10) * STRUCTURE_DAMAGE_SCALING)
#define	STRUCTURE_HEIGHT_SCATTER	(rand()%300)

/* explosion data for when a structure is blown up - used by features as well*/
#define	FLAME_MAX_ANIMS		4
#define FLAME_MAX_OFFSET	50
#define FLAME_MIN_DELAY		2000
#define FLAME_MAX_DELAY		8000
#define FLAME_CYCLES_MAX	10

#define NON_STOP_PRODUCTION	(STAT_SLDSTOPS + 1)
//production loop max
#define INFINITE_PRODUCTION	 9//10

/*This should correspond to the structLimits! */
#define	MAX_FACTORY			5
#define MAX_PROD_RUN	20

//used to flag when the Factory is ready to start building
#define ACTION_START_TIME	0

//distance that VTOLs can be away from the reArm pad
#define REARM_DIST			(TILE_UNITS*2)

extern iIMDShape * factoryModuleIMDs[NUM_FACTORY_MODULES][NUM_FACMOD_TYPES];
extern iIMDShape * researchModuleIMDs[NUM_RESEARCH_MODULES];
extern iIMDShape * powerModuleIMDs[NUM_POWER_MODULES];

extern PRODUCTION_RUN		asProductionRun[NUM_FACTORY_TYPES][MAX_FACTORY][MAX_PROD_RUN];

//Value is stored for easy access to this structure stat
extern UDWORD	factoryModuleStat;
extern UDWORD	powerModuleStat;
extern UDWORD	researchModuleStat;

// the structure that was last hit
extern STRUCTURE	*psLastStructHit;

//stores which player the production list has been set up for
extern SBYTE         productionPlayer;

//holder for all StructureStats
extern STRUCTURE_STATS		*asStructureStats;
extern UDWORD				numStructureStats;
extern STRUCTURE_LIMITS		*asStructLimits[MAX_PLAYERS];
//holds the upgrades attained through research for structure stats
extern STRUCTURE_UPGRADE	asStructureUpgrade[MAX_PLAYERS];
extern WALLDEFENCE_UPGRADE	asWallDefenceUpgrade[MAX_PLAYERS];
//holds the upgrades for the functionality of structures through research
extern RESEARCH_UPGRADE	asResearchUpgrade[MAX_PLAYERS];
extern POWER_UPGRADE		asPowerUpgrade[MAX_PLAYERS];
extern REPAIR_FACILITY_UPGRADE	asRepairFacUpgrade[MAX_PLAYERS];
extern PRODUCTION_UPGRADE	asProductionUpgrade[MAX_PLAYERS][NUM_FACTORY_TYPES];
extern REARM_UPGRADE		asReArmUpgrade[MAX_PLAYERS];

//used to hold the modifiers cross refd by weapon effect and structureStrength
extern STRUCTSTRENGTH_MODIFIER		asStructStrengthModifier[WE_NUMEFFECTS][
													NUM_STRUCT_STRENGTH];

extern BOOL IsPlayerDroidLimitReached(UDWORD PlayerNumber);
extern BOOL IsPlayerStructureLimitReached(UDWORD PlayerNumber);
extern BOOL CheckHaltOnMaxUnitsReached(STRUCTURE *psStructure);

extern BOOL loadStructureStats(SBYTE *pStructData, UDWORD bufferSize);
extern BOOL loadStructureWeapons(SBYTE *pWeaponData, UDWORD bufferSize);
extern BOOL loadStructureFunctions(SBYTE *pFunctionData, UDWORD bufferSize);
/*Load the Structure Strength Modifiers from the file exported from Access*/
extern BOOL loadStructureStrengthModifiers(SBYTE *pStrengthModData, UDWORD bufferSize);

extern BOOL	structureStatsShutDown(void);

extern BOOL structureDamage(STRUCTURE *psStructure, UDWORD damage, 
                            UDWORD weaponClass, UDWORD weaponSubClass);

/* Set the type of droid for a factory to build */
extern BOOL structSetManufacture(STRUCTURE *psStruct, DROID_TEMPLATE *psTempl,
								 UBYTE quantity);

//temp test function for creating structures at the start of the game
extern void createTestStructures(void);

// Set the tile no draw flags for a structure
extern void setStructTileDraw(STRUCTURE *psStruct);

//builds a specified structure at a given location
extern STRUCTURE* buildStructure(STRUCTURE_STATS* pStructureType, UDWORD x, UDWORD y, 
						  UDWORD player,BOOL FromSave);

/* The main update routine for all Structures */
void structureUpdate(STRUCTURE *psBuilding);

/* Release all resources associated with a structure */
void structureRelease(STRUCTURE *psBuilding);

/* Remove a structure and free it's memory */
extern BOOL destroyStruct(STRUCTURE *psDel);

// remove a structure from a game without any visible effects
// bDestroy = TRUE if the object is to be destroyed
// (for example used to change the type of wall at a location)
BOOL removeStruct(STRUCTURE *psDel, BOOL bDestroy);

//fills the list with Structures that can be built
extern UDWORD fillStructureList(STRUCTURE_STATS **ppList, UDWORD selectedPlayer, 
						 UDWORD limit);
/* checks that the location is a valid one to build on and sets the outline colour
x and y in tile-coords*/
extern BOOL validLocation(BASE_STATS *psStats, UDWORD x, UDWORD y, UDWORD player, 
                          BOOL bCheckBuildQueue);

/* for a new structure, find a location along an edge which the droid can get
to and return this as the destination for the droid */
//extern BOOL getDroidDestination(STRUCTURE_STATS *psPositionStats, UDWORD structX, 
//	UDWORD structY, UDWORD * pDroidX, UDWORD *pDroidY);
/*for a structure or feature, find a location along an edge which the droid can get
to and return this as the destination for the droid*/
extern BOOL getDroidDestination(BASE_STATS *psPositionStats, UDWORD structX, 
	UDWORD structY, UDWORD * pDroidX, UDWORD *pDroidY);
/* check along the width of a structure for an empty space */
extern BOOL checkWidth(UDWORD maxRange, UDWORD x, UDWORD y, UDWORD *pDroidX, UDWORD *pDroidY);

/* check along the length of a structure for an empty space */
extern BOOL checkLength(UDWORD maxRange, UDWORD x, UDWORD y, UDWORD *pDroidX, UDWORD *pDroidY);

extern SWORD buildFoundation(STRUCTURE_STATS *psStructStats, UDWORD x, UDWORD y);

//initialise the structure limits structure
extern void initStructLimits(void);
/* set the current number of structures of each type built */
extern void setCurrentStructQuantity(BOOL displayError);
/* get a stat inc based on the name */
extern SDWORD getStructStatFromName(STRING *pName);
/*check to see if the structure is 'doing' anything  - return TRUE if idle*/
extern BOOL  structureIdle(STRUCTURE *psBuilding);
/*checks to see if any structure exists of a specified type with a specified status */
extern BOOL checkStructureStatus( STRUCTURE_STATS *psStats, UDWORD player, UDWORD status);
/*sets the point new droids go to - x/y in world coords for a Factory*/
extern void setAssemblyPoint(FLAG_POSITION *psAssemblyPoint, UDWORD x, UDWORD y, 
                             UDWORD player, BOOL bCheck);
//extern void createAssemblyPoint(STRUCTURE* psStruct);

/* consider delivery points when selected by player*/
extern void processDeliveryPoint(UDWORD player, UDWORD x, UDWORD y);

/*called when a structure has been built - checks through the list of callbacks 
for the scripts*/
extern void structureCompletedCallback(STRUCTURE_STATS *psStructType);

/*initialises the flag before a new data set is loaded up*/
extern void initFactoryNumFlag(void);
//called at start of missions
extern void resetFactoryNumFlag(void);

/* get demolish stat */
extern STRUCTURE_STATS * structGetDemolishStat( void );

/*find a location near to the factory to start the droid of*/
extern BOOL placeDroid(STRUCTURE *psStructure, UDWORD *droidX, UDWORD *droidY);

/*sets the flag to indicate a Power Generator Exists - so do Oil Derrick anim*/
//extern void setPowerGenExists(BOOL state, UDWORD player);
/*returns teh status of the flag*/
//extern BOOL getPowerGenExists(UDWORD player);

/*sets the flag to indicate a HQ Exists - so draw Radar*/
extern void setHQExists(BOOL state, UDWORD player);
/*returns the status of the flag*/
extern BOOL getHQExists(UDWORD player);
/*sets the flag to indicate a SatUplink Exists - so draw everything!*/
extern void setSatUplinkExists(BOOL state, UDWORD player);
/*returns the status of the flag*/
extern BOOL getSatUplinkExists(UDWORD player);
/*sets the flag to indicate a Las Sat Exists - ONLY EVER WANT ONE*/
extern void setLasSatExists(BOOL state, UDWORD player);
/*returns the status of the flag*/
extern BOOL getLasSatExists(UDWORD player);

extern BOOL calcStructureMuzzleLocation(STRUCTURE *psStructure, iVector *muzzle);

/*this is called whenever a structure has finished building*/
extern void buildingComplete(STRUCTURE *psBuilding);
// these functions are used in game.c inplace of  building complete
extern void checkForResExtractors(STRUCTURE *psPowerGen);
extern void checkForPowerGen(STRUCTURE *psPowerGen);

/*Looks through the players list of structures to see if a HQ exists - will look
through the list of structures at Home Base when on an offWorld mission map*/
extern BOOL radarCheckForHQ(UDWORD player);

// Set the command droid that factory production should go to
//struct _command_droid;
extern void assignFactoryCommandDroid(STRUCTURE *psStruct, struct _droid *psCommander);

// remove all factories from a command droid
void clearCommandDroidFactory(DROID *psDroid);

/*for a given structure, return a pointer to its module stat */
extern STRUCTURE_STATS* getModuleStat(STRUCTURE *psStruct);

/*called when a Res extractor is destroyed or runs out of power or is disconnected
adjusts the owning Power Gen so that it can link to a different Res Extractor if one
is available*/
extern void releaseResExtractor(STRUCTURE *psRelease);

/*called when a Power Gen is destroyed or is disconnected
adjusts the associated Res Extractors so that they can link to different Power 
Gens if any are available*/
extern void releasePowerGen(STRUCTURE *psRelease);

//print some info at the top of the screen dependant on the structure
extern void printStructureInfo(STRUCTURE *psStructure);

/*Checks the template type against the factory type - returns FALSE 
if not a good combination!*/
extern BOOL validTemplateForFactory(DROID_TEMPLATE *psTemplate, STRUCTURE *psFactory);

/*calculates the damage caused to the resistance levels of structures*/
//extern BOOL electronicDamage(STRUCTURE *psStructure, UDWORD damage, UBYTE attackPlayer);
//electronic damage can be targetted at droids as well as structures now - AB 5/11/98
extern BOOL electronicDamage(BASE_OBJECT *psTarget, UDWORD damage, UBYTE attackPlayer);

/* EW works differently in multiplayer mode compared with single player.*/
extern BOOL validStructResistance(STRUCTURE *psStruct);

/*checks to see if a specific structure type exists -as opposed to a structure 
stat type*/
extern BOOL checkSpecificStructExists(UDWORD structInc, UDWORD player);

/*Access functions for the upgradeable stats of a structure*/
extern UDWORD	structureBody(STRUCTURE * psStruct);
extern UDWORD	structureArmour(STRUCTURE_STATS *psStats, UBYTE player);
extern UDWORD	structureResistance(STRUCTURE_STATS *psStats, UBYTE player);
/*this returns the Base Body points of a structure - regardless of upgrade*/
extern UDWORD	structureBaseBody(STRUCTURE *psStructure);

extern void hqReward(UBYTE losingPlayer, UBYTE rewardPlayer);

// Is a structure a factory of somekind?
extern BOOL StructIsFactory(STRUCTURE *Struct);

// Is a flag a factory delivery point?
extern BOOL FlagIsFactory(FLAG_POSITION *psCurrFlag);

// Find a factories corresonding delivery point.
extern FLAG_POSITION *FindFactoryDelivery(STRUCTURE *Struct);

//Find the factory associated with the delivery point - returns NULL if none exist
extern STRUCTURE	*findDeliveryFactory(FLAG_POSITION *psDelPoint);

/*this is called when a factory produces a droid. The Template returned is the next 
one to build - if any*/
extern DROID_TEMPLATE * factoryProdUpdate(STRUCTURE *psStructure, DROID_TEMPLATE *psTemplate);

//increment the production run for this type
extern void factoryProdAdjust(STRUCTURE *psStructure, DROID_TEMPLATE *psTemplate, BOOL add);

//returns the quantity of a specific template in the production list
extern UDWORD	getProductionQuantity(STRUCTURE *psStructure, DROID_TEMPLATE *psTemplate);
/*returns the quantity of a specific template in the production list that 
have already been built*/
extern UDWORD	getProductionBuilt(STRUCTURE *psStructure, DROID_TEMPLATE *psTemplate);

//looks through a players production list to see if a command droid is being built
extern UBYTE checkProductionForCommand(UBYTE player);

//check that delivery points haven't been put down in invalid location
extern void checkDeliveryPoints(UDWORD version);

//adjust the loop quantity for this factory
extern void factoryLoopAdjust(STRUCTURE *psStruct, BOOL add);

/*cancels the production run for the factory and returns any power that was 
accrued but not used*/
extern void cancelProduction(STRUCTURE *psBuilding);

/*set a factory's production run to hold*/
extern void holdProduction(STRUCTURE *psBuilding);

/*release a factory's production run from hold*/
extern void releaseProduction(STRUCTURE *psBuilding);

/*This function is called after a game is loaded so that any resource extractors 
that are active are initialised for when to start*/
extern void checkResExtractorsActive(void);

// Count number of factories assignable to a command droid.
extern UWORD countAssignableFactories(UBYTE player,UWORD FactoryType);

/*Used for determining how much of the structure to draw as being built or demolished*/
extern FRACT structHeightScale(STRUCTURE *psStruct);

/*compares the structure sensor type with the droid weapon type to see if the 
FIRE_SUPPORT order can be assigned*/
extern BOOL structSensorDroidWeapon(STRUCTURE *psStruct, DROID *psDroid);

/*checks if the structure has a Counter Battery sensor attached - returns 
TRUE if it has*/
extern BOOL structCBSensor(STRUCTURE *psStruct);
/*checks if the structure has a Standard Turret sensor attached - returns 
TRUE if it has*/
extern BOOL structStandardSensor(STRUCTURE *psStruct);

/*checks if the structure has a VTOL Intercept sensor attached - returns 
TRUE if it has*/
extern BOOL structVTOLSensor(STRUCTURE *psStruct);

/*checks if the structure has a VTOL Counter Battery sensor attached - returns 
TRUE if it has*/
extern BOOL structVTOLCBSensor(STRUCTURE *psStruct);

// return the nearest rearm pad
// if bClear is true it tries to find the nearest clear rearm pad in
// the same cluster as psTarget
// psTarget can be NULL
STRUCTURE *	findNearestReArmPad(DROID *psDroid, STRUCTURE *psTarget, BOOL bClear);

// check whether a rearm pad is clear
BOOL clearRearmPad(STRUCTURE *psStruct);

// clear a rearm pad for a vtol to land on it
void ensureRearmPadClear(STRUCTURE *psStruct, DROID *psDroid);

// return whether a rearm pad has a vtol on it
BOOL vtolOnRearmPad(STRUCTURE *psStruct, DROID *psDroid);

/* Just returns true if the structure's present body points aren't as high as the original*/
extern BOOL	structIsDamaged(STRUCTURE *psStruct);

// give a structure from one player to another - used in Electronic Warfare
extern STRUCTURE * giftSingleStructure(STRUCTURE *psStructure, UBYTE attackPlayer, BOOL bFromScript);

/*Initialise the production list and set up the production player*/
extern void changeProductionPlayer(UBYTE player);

// La!
extern BOOL IsStatExpansionModule(STRUCTURE_STATS *psStats);

/*checks that the structure stats have loaded up as expected - must be done after 
all StructureStats parts have been loaded*/
extern BOOL checkStructureStats(void);

/*returns the power cost to build this structure*/
extern UDWORD  structPowerToBuild(STRUCTURE *psStruct);

extern UDWORD getMaxDroids(UDWORD PlayerNumber);

// check whether a factory of a certain number and type exists
extern BOOL checkFactoryExists(UDWORD player, UDWORD factoryType, UDWORD inc);

extern BOOL	ptInStructure(STRUCTURE *psStruct, UDWORD x, UDWORD y);

/*checks the structure passed in is a Las Sat structure which is currently 
selected - returns TRUE if valid*/
extern BOOL lasSatStructSelected(STRUCTURE *psStruct);

#endif

