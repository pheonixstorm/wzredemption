/*
 * Stats.h
 *
 * Interface to the common stats module
 *
 */
#ifndef _stats_h
#define _stats_h

#include "objectdef.h"
/**************************************************************************************
 *
 * Function prototypes and data storage for the stats
 */

/* The stores for the different stats */
extern BODY_STATS			*asBodyStats;
extern BRAIN_STATS			*asBrainStats;
//extern POWER_STATS			*asPowerStats;
extern PROPULSION_STATS		*asPropulsionStats;
extern SENSOR_STATS			*asSensorStats;
extern ECM_STATS			*asECMStats;
//extern ARMOUR_STATS			*asArmourStats;
extern REPAIR_STATS			*asRepairStats;
//extern PROGRAM_STATS		*asProgramStats;
extern WEAPON_STATS			*asWeaponStats;
extern CONSTRUCT_STATS		*asConstructStats;

extern PROPULSION_TYPES		*asPropulsionTypes;
extern TERRAIN_TABLE		*asTerrainTable;
extern SPECIAL_ABILITY		*asSpecialAbility;

//used to hold the modifiers cross refd by weapon effect and propulsion type
extern WEAPON_MODIFIER		asWeaponModifier[WE_NUMEFFECTS][NUM_PROP_TYPES];

//used to hold the current upgrade level per player per weapon subclass
extern WEAPON_UPGRADE		asWeaponUpgrade[MAX_PLAYERS][NUM_WEAPON_SUBCLASS];
extern SENSOR_UPGRADE		asSensorUpgrade[MAX_PLAYERS];
extern ECM_UPGRADE			asECMUpgrade[MAX_PLAYERS];
extern REPAIR_UPGRADE		asRepairUpgrade[MAX_PLAYERS];
extern CONSTRUCTOR_UPGRADE	asConstUpgrade[MAX_PLAYERS];
//body upgrades are possible for droids and/or cyborgs
#define		DROID_BODY_UPGRADE	0
#define		CYBORG_BODY_UPGRADE	1
#define		BODY_TYPE		2
extern BODY_UPGRADE			asBodyUpgrade[MAX_PLAYERS][BODY_TYPE];

/* The number of different stats stored */
extern UDWORD		numBodyStats;
extern UDWORD		numBrainStats;
//extern UDWORD		numPowerStats;
extern UDWORD		numPropulsionStats;
extern UDWORD		numSensorStats;
extern UDWORD		numECMStats;
//extern UDWORD		numArmourStats;
extern UDWORD		numRepairStats;
extern UDWORD		numProgramStats;
extern UDWORD		numWeaponStats;
extern UDWORD		numConstructStats;

//extern UDWORD		numPropulsionTypes;
extern UDWORD		numTerrainTypes;
extern UDWORD		numSpecialAbility;

/* What number the ref numbers start at for each type of stat */
#define REF_BODY_START			0x010000
#define REF_BRAIN_START			0x020000
//#define REF_POWER_START			0x030000
#define REF_PROPULSION_START	0x040000
#define REF_SENSOR_START		0x050000
#define REF_ECM_START			0x060000
//#define REF_ARMOUR_START		0x070000
#define REF_REPAIR_START		0x080000
//#define REF_PROGRAM_START		0x090000
#define REF_WEAPON_START		0x0a0000
#define REF_RESEARCH_START		0x0b0000
#define REF_TEMPLATE_START		0x0c0000
#define REF_STRUCTURE_START		0x0d0000
#define REF_FUNCTION_START		0x0e0000
#define REF_CONSTRUCT_START		0x0f0000
#define REF_FEATURE_START		0x100000

/* The maximum number of refs for a type of stat */
#define REF_RANGE				0x010000


//stores for each players component states - see below
extern UBYTE		*apCompLists[MAX_PLAYERS][COMP_NUMCOMPONENTS];

//store for each players Structure states
extern UBYTE		*apStructTypeLists[MAX_PLAYERS];

//flags to fill apCompLists and apStructTypeLists
#define AVAILABLE				0x01		//this item can be used to design droids
#define UNAVAILABLE				0x02		//the player does not know about this item
#define FOUND					0x04		//this item has been found, but is unresearched

/*******************************************************************************
*		Allocate stats functions
*******************************************************************************/
/* Allocate Weapon stats */
extern BOOL statsAllocWeapons(UDWORD numEntries);

/*Allocate Armour stats*/
//extern BOOL statsAllocArmour(UDWORD numEntries);

/*Allocate Body stats*/
extern BOOL statsAllocBody(UDWORD numEntries);

/*Allocate Brain stats*/
extern BOOL statsAllocBrain(UDWORD numEntries);

/*Allocate Power stats*/
//extern BOOL statsAllocPower(UDWORD numEntries);

/*Allocate Propulsion stats*/
extern BOOL statsAllocPropulsion(UDWORD numEntries);

/*Allocate Sensor stats*/
extern BOOL statsAllocSensor(UDWORD numEntries);

/*Allocate Ecm Stats*/
extern BOOL statsAllocECM(UDWORD numEntries);

/*Allocate Repair Stats*/
extern BOOL statsAllocRepair(UDWORD numEntries);

/*Allocate Program Stats*/
extern BOOL statsAllocProgram(UDWORD numEntries);

/*Allocate Construct Stats*/
extern BOOL statsAllocConstruct(UDWORD numEntries);

/*******************************************************************************
*		Load stats functions
*******************************************************************************/
/* Return the number of newlines in a file buffer */
extern UDWORD numCR(UBYTE *pFileBuffer, UDWORD fileSize);

/*Load the weapon stats from the file exported from Access*/
extern BOOL loadWeaponStats(SBYTE *pWeaponData, UDWORD bufferSize);

/*Load the armour stats from the file exported from Access*/
//extern BOOL loadArmourStats(void);

/*Load the body stats from the file exported from Access*/
extern BOOL loadBodyStats(SBYTE *pBodyData, UDWORD bufferSize);

/*Load the brain stats from the file exported from Access*/
extern BOOL loadBrainStats(SBYTE *pBrainData, UDWORD bufferSize);

/*Load the power stats from the file exported from Access*/
//extern BOOL loadPowerStats(void);

/*Load the propulsion stats from the file exported from Access*/
extern BOOL loadPropulsionStats(SBYTE *pPropulsionData, UDWORD bufferSize);

/*Load the sensor stats from the file exported from Access*/
extern BOOL loadSensorStats(SBYTE *pSensorData, UDWORD bufferSize);

/*Load the ecm stats from the file exported from Access*/
extern BOOL loadECMStats(SBYTE *pECMData, UDWORD bufferSize);

/*Load the repair stats from the file exported from Access*/
extern BOOL loadRepairStats(SBYTE *pRepairData, UDWORD bufferSize);

/*Load the program stats from the file exported from Access*/
extern BOOL loadProgramStats(SBYTE *pProgramData, UDWORD bufferSize);

/*Load the construct stats from the file exported from Access*/
extern BOOL loadConstructStats(SBYTE *pConstructData, UDWORD bufferSize);

/*Load the Propulsion Types from the file exported from Access*/
extern BOOL loadPropulsionTypes(SBYTE *pPropTypeData, UDWORD bufferSize);

/*Load the propulsion sounds from the file exported from Access*/
extern BOOL loadPropulsionSounds(SBYTE *pSoundData, UDWORD bufferSize);

/*Load the Terrain Table from the file exported from Access*/
extern BOOL loadTerrainTable(SBYTE *pTerrainTableData, UDWORD bufferSize);

/*Load the Special Ability stats from the file exported from Access*/
extern BOOL loadSpecialAbility(SBYTE *pSAbilityData, UDWORD bufferSize);

/* load the IMDs to use for each body-propulsion combination */
extern BOOL loadBodyPropulsionIMDs(SBYTE *pData, UDWORD bufferSize);

/*Load the weapon sounds from the file exported from Access*/
extern BOOL loadWeaponSounds(SBYTE *pSoundData, UDWORD bufferSize);

/*Load the Weapon Effect Modifiers from the file exported from Access*/
extern BOOL loadWeaponModifiers(SBYTE *pWeapModData, UDWORD bufferSize);
/*******************************************************************************
*		Set stats functions
*******************************************************************************/
/* Set the stats for a particular weapon type
 * The function uses the ref number in the stats structure to
 * index the correct array entry
 */
extern void statsSetWeapon(WEAPON_STATS	*psStats, UDWORD index);

/*Set the stats for a particular armour type*/
//extern void statsSetArmour(ARMOUR_STATS	*psStats, UDWORD index);

/*Set the stats for a particular body type*/
extern void statsSetBody(BODY_STATS	*psStats, UDWORD index);

/*Set the stats for a particular brain type*/
extern void statsSetBrain(BRAIN_STATS	*psStats, UDWORD index);

/*Set the stats for a particular power type*/
//extern void statsSetPower(POWER_STATS	*psStats, UDWORD index);

/*Set the stats for a particular propulsion type*/
extern void statsSetPropulsion(PROPULSION_STATS	*psStats, UDWORD index);

/*Set the stats for a particular sensor type*/
extern void statsSetSensor(SENSOR_STATS	*psStats, UDWORD index);

/*Set the stats for a particular ecm type*/
extern void statsSetECM(ECM_STATS	*psStats, UDWORD index);

/*Set the stats for a particular repair type*/
extern void statsSetRepair(REPAIR_STATS	*psStats, UDWORD index);

/*Set the stats for a particular program type*/
//extern void statsSetProgram(PROGRAM_STATS	*psStats, UDWORD index);

/*Set the stats for a particular construct type*/
extern void statsSetConstruct(CONSTRUCT_STATS	*psStats, UDWORD index);

/*******************************************************************************
*		Get stats functions
*******************************************************************************/
extern WEAPON_STATS *statsGetWeapon(UDWORD ref);
//extern ARMOUR_STATS *statsGetArmour(UDWORD ref);
extern BODY_STATS *statsGetBody(UDWORD ref);
extern BRAIN_STATS *statsGetBrain(UDWORD ref);
//extern POWER_STATS *statsGetPower(UDWORD ref);
extern PROPULSION_STATS *statsGetPropulsion(UDWORD ref);
extern SENSOR_STATS *statsGetSensor(UDWORD ref);
extern ECM_STATS *statsGetECM(UDWORD ref);
extern REPAIR_STATS *statsGetRepair(UDWORD ref);
//extern PROGRAM_STATS *statsGetProgram(UDWORD ref);
extern CONSTRUCT_STATS *statsGetConstruct(UDWORD ref);

/*******************************************************************************
*		Generic stats functions
*******************************************************************************/
/*Load the stats from the Access database*/
//extern BOOL loadStats(void);

/*calls the STATS_DEALLOC macro for each set of stats*/
extern BOOL statsShutDown(void);

/*Deallocate the stats passed in as parameter */
extern void statsDealloc(COMP_BASE_STATS* pStats, UDWORD listSize, 
						 UDWORD structureSize);

extern void deallocPropulsionTypes(void);
extern void deallocTerrainTypes(void);
extern void deallocTerrainTable(void);
extern void deallocSpecialAbility(void);

extern void storeSpeedFactor(UDWORD terrainType, UDWORD propulsionType, UDWORD speedFactor);
extern UDWORD getSpeedFactor(UDWORD terrainType, UDWORD propulsionType);
//return the type of stat this ref refers to!
extern UDWORD statType(UDWORD ref);
//return the REF_START value of this type of stat
extern UDWORD statRefStart(UDWORD stat);
/*Returns the component type based on the string - used for reading in data */
extern UDWORD componentType(char* pType);
//get the component Inc for a stat based on the name
extern SDWORD	getCompFromName(UDWORD compType, STRING *pName);
//get the component Inc for a stat based on the Resource name held in Names.txt
extern SDWORD	getCompFromResName(UDWORD compType, STRING *pName);
/*sets the tech level for the stat passed in */
extern BOOL setTechLevel(BASE_STATS *psStats, STRING *pLevel);
/*returns the weapon sub class based on the string name passed in */
extern SDWORD	getWeaponSubClass(STRING *pSubClass);
/*either gets the name associated with the resource (if one) or allocates space and copies pName*/
extern BOOL allocateName(STRING **ppStore, STRING *pName);
//converts the name read in from Access into the name which is used in the Stat lists (or ignores it)
extern BOOL getResourceName(STRING *pName);
/*return the name to display for the interface - valid for OBJECTS and STATS*/
extern STRING* getName(STRING *pNameID);
/*sets the store to the body size based on the name passed in - returns FALSE 
if doesn't compare with any*/
extern BOOL getBodySize(STRING *pSize, UBYTE *pStore);

// Pass in a stat and get its name
extern STRING* getStatName(void * pStat);

/*returns the propulsion type based on the string name passed in */
extern UBYTE	getPropulsionType(STRING *pType);
/*returns the weapon effect based on the string name passed in */
extern UBYTE	getWeaponEffect(STRING *pWeaponEffect);

/*Access functions for the upgradeable stats of a weapon*/
extern UDWORD	weaponFirePause(WEAPON_STATS *psStats, UBYTE player);
extern UDWORD	weaponShortHit(WEAPON_STATS *psStats, UBYTE player);
extern UDWORD	weaponLongHit(WEAPON_STATS *psStats, UBYTE player);
extern UDWORD	weaponDamage(WEAPON_STATS *psStats, UBYTE player);
extern UDWORD	weaponRadDamage(WEAPON_STATS *psStats, UBYTE player);
extern UDWORD	weaponIncenDamage(WEAPON_STATS *psStats, UBYTE player);
extern UDWORD	weaponRadiusHit(WEAPON_STATS *psStats, UBYTE player);
/*Access functions for the upgradeable stats of a sensor*/
extern UDWORD	sensorPower(SENSOR_STATS *psStats, UBYTE player);
extern UDWORD	sensorRange(SENSOR_STATS *psStats, UBYTE player);
/*Access functions for the upgradeable stats of a ECM*/
extern UDWORD	ecmPower(ECM_STATS *psStats, UBYTE player);
/*Access functions for the upgradeable stats of a repair*/
extern UDWORD	repairPoints(REPAIR_STATS *psStats, UBYTE player);
/*Access functions for the upgradeable stats of a constructor*/
extern UDWORD	constructorPoints(CONSTRUCT_STATS *psStats, UBYTE player);
/*Access functions for the upgradeable stats of a body*/
extern UDWORD	bodyPower(BODY_STATS *psStats, UBYTE player, UBYTE bodyType);
extern UDWORD	bodyArmour(BODY_STATS *psStats, UBYTE player, UBYTE bodyType, 
				   WEAPON_CLASS weaponClass);
/*dummy function for John*/
extern void brainAvailable(BRAIN_STATS *psStat);

//Access functions for the max values to be used in the Design Screen
extern UDWORD getMaxComponentWeight(void);
extern UDWORD getMaxBodyArmour(void);
extern UDWORD getMaxBodyPower(void);
extern UDWORD getMaxBodyPoints(void);
extern UDWORD getMaxSensorRange(void);
extern UDWORD getMaxSensorPower(void);
extern UDWORD getMaxECMPower(void);
extern UDWORD getMaxConstPoints(void);
extern UDWORD getMaxRepairPoints(void);
extern UDWORD getMaxWeaponRange(void);
extern UDWORD getMaxWeaponDamage(void);
extern UDWORD getMaxPropulsionSpeed(void);

#endif

