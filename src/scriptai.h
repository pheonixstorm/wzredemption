/*
 * ScriptAI.h
 *
 * Script functions to support the AI system
 *
 */
#ifndef _scriptai_h
#define _scriptai_h

// Add a droid to a group
extern BOOL scrGroupAddDroid(void);

// Add droids in an area to a group
extern BOOL scrGroupAddArea(void);

// Add groupless droids in an area to a group 
extern BOOL scrGroupAddAreaNoGroup(void);

// Move the droids from one group to another
extern BOOL scrGroupAddGroup(void);

// check if a droid is a member of a group
extern BOOL scrGroupMember(void);

// return number of idle droids in group.
extern BOOL scrIdleGroup(void);

// initialise iterating a groups members
extern BOOL scrInitIterateGroup(void);

// iterate through a groups members
extern BOOL scrIterateGroup(void);

// remove a droid from a group
extern BOOL scrDroidLeaveGroup(void);

// Give a group an order
extern BOOL scrOrderGroup(void);

// Give a group an order to a location
extern BOOL scrOrderGroupLoc(void);

// Give a group an order to an object
extern BOOL scrOrderGroupObj(void);

// Give a Droid an order
extern BOOL scrOrderDroid(void);

// Give a Droid an order to a location
extern BOOL scrOrderDroidLoc(void);

// Give a Droid an order to an object
extern BOOL scrOrderDroidObj(void);

// Give a Droid an order with a stat
extern BOOL scrOrderDroidStatsLoc(void);

// set the secondary state for a droid
extern BOOL scrSetDroidSecondary(void);

// set the secondary state for a droid
extern BOOL scrSetGroupSecondary(void);

// initialise iterating a cluster
extern BOOL scrInitIterateCluster(void);

// iterate a cluster
extern BOOL scrIterateCluster(void);

// add a droid to a commander
extern BOOL scrCmdDroidAddDroid(void);


// types for structure targets
typedef enum _scr_struct_tar
{
	// normal structure types
	SCR_ST_HQ					= 0x00000001,
	SCR_ST_FACTORY				= 0x00000002,
	SCR_ST_POWER_GEN			= 0x00000004,
	SCR_ST_RESOURCE_EXTRACTOR	= 0x00000008,
	SCR_ST_WALL					= 0x00000010,
	SCR_ST_RESEARCH				= 0x00000020,
	SCR_ST_REPAIR_FACILITY		= 0x00000040,
	SCR_ST_COMMAND_CONTROL		= 0x00000080,
	SCR_ST_CYBORG_FACTORY		= 0x00000100,
	SCR_ST_VTOL_FACTORY			= 0x00000200,
	SCR_ST_REARM_PAD			= 0x00000400,
	SCR_ST_SENSOR				= 0x00000800,

	// defensive structure types
	SCR_ST_DEF_GROUND			= 0x00001000,
	SCR_ST_DEF_AIR				= 0x00002000,
	SCR_ST_DEF_IDF				= 0x00004000,
	SCR_ST_DEF_ALL				= 0x00007000,
} SCR_STRUCT_TAR;


typedef enum _scr_droid_tar
{
	// turret types
	SCR_DT_COMMAND				= 0x00000001,
	SCR_DT_SENSOR				= 0x00000002,
	SCR_DT_CONSTRUCT			= 0x00000004,
	SCR_DT_REPAIR				= 0x00000008,
	SCR_DT_WEAP_GROUND			= 0x00000010,
	SCR_DT_WEAP_AIR				= 0x00000020,
	SCR_DT_WEAP_IDF				= 0x00000040,
	SCR_DT_WEAP_ALL				= 0x00000070,

	// body types
	SCR_DT_LIGHT				= 0x00000080,
	SCR_DT_MEDIUM				= 0x00000100,
	SCR_DT_HEAVY				= 0x00000200,
	SCR_DT_SUPER_HEAVY			= 0x00000400,

	// propulsion
	SCR_DT_TRACK				= 0x00000800,
	SCR_DT_HTRACK				= 0x00001000,
	SCR_DT_WHEEL				= 0x00002000,
	SCR_DT_LEGS					= 0x00004000,
	SCR_DT_GROUND				= 0x00007800,
	SCR_DT_VTOL					= 0x00008000,
	SCR_DT_HOVER				= 0x00010000,

} SCR_DROID_TAR;


// reset the structure preferences
BOOL scrResetStructTargets(void);
// reset the droid preferences
BOOL scrResetDroidTargets(void);
// set prefered structure target types
BOOL scrSetStructTarPref(void);
// set structure target ignore types
BOOL scrSetStructTarIgnore(void);
// set prefered droid target types
BOOL scrSetDroidTarPref(void);
// set droid target ignore types
BOOL scrSetDroidTarIgnore(void);
// get a structure target in an area using the preferences
BOOL scrStructTargetInArea(void);
// get a structure target on the map using the preferences
BOOL scrStructTargetOnMap(void);
// get a droid target in an area using the preferences
BOOL scrDroidTargetInArea(void);
// get a droid target on the map using the preferences
BOOL scrDroidTargetOnMap(void);
// get a target from a cluster using the preferences
BOOL scrTargetInCluster(void);

// Skirmish funcs may99

// choose and do research
BOOL scrSkDoResearch(void);

// find the human players
BOOL scrSkLocateEnemy(void);

// check a template
BOOL scrSkCanBuildTemplate(void);

// check for vtol availability
BOOL scrSkVtolEnableCheck(void);

// check capacity
BOOL scrSkGetFactoryCapacity(void);

// help/hinder player.
BOOL scrSkDifficultyModifier(void);

// pick good spots.
BOOL scrSkDefenseLocation(void);

// line build.
//BOOL scrSkOrderDroidLineBuild(void);



#endif




