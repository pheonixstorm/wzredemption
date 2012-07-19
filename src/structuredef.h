/*
 * StructureDef.h
 *
 * Structure definitions for structures
 *
 */
#ifndef _structuredef_h
#define _structuredef_h

#define NUM_FACTORY_MODULES	2
#define NUM_RESEARCH_MODULES 4
#define NUM_POWER_MODULES 4

#define	REF_ANY	255	// Used to indicate any kind of building when calling intGotoNextStructureType()

/* Defines for indexing an appropriate IMD object given a buildings purpose. */
enum
{
REF_HQ,
REF_FACTORY,	
REF_FACTORY_MODULE,//draw as factory 2	
REF_POWER_GEN,
REF_POWER_MODULE,
REF_RESOURCE_EXTRACTOR,
REF_DEFENSE,
REF_WALL,
REF_WALLCORNER,				//corner wall - no gun
REF_BLASTDOOR,
REF_RESEARCH,	
REF_RESEARCH_MODULE,	
REF_REPAIR_FACILITY,
REF_COMMAND_CONTROL,		//control centre for command droids
REF_BRIDGE,
REF_DEMOLISH,			//the demolish structure type - should only be one stat with this type
REF_CYBORG_FACTORY,
REF_VTOL_FACTORY,
REF_LAB,
REF_REARM_PAD,
REF_MISSILE_SILO,
REF_SAT_UPLINK,         //added for updates - AB 8/6/99

//REF_WALLH,     //the following are needed for the demo
//REF_WALLV,		
//REF_CORNER1,
//REF_CORNER2,
//REF_CORNER3,
//REF_CORNER4,
//REF_GATE1,
//REF_GATE2,
//REF_GATE3,
//REF_GATE4,
//REF_TOWER1,
//REF_TOWER2,
//REF_TOWER3,
//REF_TOWER4,
//REF_VANH,
//REF_VANV,
//REF_JEEP,
//REF_TANKERH,
//REF_TANKERV,

NUM_DIFF_BUILDINGS,		//need to keep a count of how many types for IMD loading
};

typedef enum _position_type
{
	POS_DELIVERY,		//Delivery Points NOT wayPoints
	POS_PROXDATA,	//proximity messages that are data generated
	POS_PROXOBJ,	//proximity messages that are in game generated
	POS_TEMPDELIVERY //SAVE ONLY delivery point for factory currently assigned to commander
} POSITION_TYPE;

#define POSITION_OBJ \
	POSITION_TYPE	type;				/*the type of position obj - FlagPos or ProxDisp*/ \
	UDWORD			frameNumber;		/*when the Position was last drawn*/ \
	UDWORD			screenX;			/*screen coords and radius of Position imd */ \
	UDWORD			screenY; \
	UDWORD			screenR; \
	UDWORD			player;				/*which player the Position belongs to*/ \
	BOOL			selected			/*flag to indicate whether the Position 
										is to be highlighted*/

typedef struct _object_position
{
	POSITION_OBJ;
} OBJECT_POSITION;

typedef struct _flag_position
{
	POSITION_OBJ;
	iVector		coords;							//the world coords of the Position
	//UDWORD		frameNumber;					//when the Position was last drawn
	//UDWORD		screenX, screenY, screenR;		//screen coords and radius of Position imd
	//UDWORD		player;							//which player the Position belongs to
	//BOOL		selected;						//flag to indicate whether the 
												//Position is to be highlighted
	UBYTE		factoryInc;						//indicates whether the first, second etc factory
	UBYTE		factoryType;					//indicates whether standard, cyborg or vtol factory
//	UBYTE		factorySub;						//sub value. needed to order production points.
//	UBYTE		primary;
	struct _flag_position	*psNext;
} FLAG_POSITION;


#ifdef DEMO
#define NUM_DEMO_STRUCTS	12
#endif

//only allowed one weapon per structure (more memory for Tim) 
#define STRUCT_MAXWEAPS		1

typedef enum _struct_strength
{
	STRENGTH_SOFT,
	STRENGTH_MEDIUM,
	STRENGTH_HARD,
	STRENGTH_BUNKER,

	NUM_STRUCT_STRENGTH,
} STRUCT_STRENGTH;

#define INVALID_STRENGTH	(NUM_STRUCT_STRENGTH + 1)

typedef UWORD STRUCTSTRENGTH_MODIFIER;

//this structure is used to hold the permenant stats for each type of building
typedef struct _structure_stats
{
	STATS_BASE;						/* basic stats */ 
	UDWORD		type;				/* the type of structure */
	TECH_LEVEL	techLevel;			/* technology level of the structure */
	STRUCT_STRENGTH	strength;		/* strength against the weapon effects */
	UDWORD		terrainType;		/*The type of terrain the structure has to be 
									  built next to - may be none*/
	UDWORD		baseWidth;			/*The width of the base in tiles*/
	UDWORD		baseBreadth;		/*The breadth of the base in tiles*/
	UDWORD		foundationType;		/*The type of foundation for the structure*/
	UDWORD		buildPoints;		/*The number of build points required to build
									  the structure*/
	UDWORD		height;				/*The height above/below the terrain - negative
									  values denote below the terrain*/
	UDWORD		armourValue;		/*The armour value for the structure - can be 
									  upgraded */
	UDWORD		bodyPoints;			/*The structure's body points - A structure goes
									  off-line when 50% of its body points are lost*/
	UDWORD		repairSystem;		/*The repair system points are added to the body
									  points until fully restored . The points are 
									  then added to the Armour Points*/
	UDWORD		powerToBuild;		/*How much power the structure requires to build*/ 
    //NOT USED ANYMORE - AB 24/01/99
	/*UDWORD		minimumPower;		The minimum power requirement to start building
								      the structure*/
	UDWORD		resistance;			/*The number used to determine whether a 
									  structure can resist an enemy takeover - 
									  0 = cannot be attacked electrically*/
    //NOT USED ANYMORE - AB 24/01/99
	/*UDWORD		quantityLimit;		The maximum number that a player can have - 
									  0 = no limit 1 = only 1 allowed etc*/
	UDWORD		sizeModifier;		/*The larger the target, the easier to hit*/
	struct 	iIMDShape	*pIMD;		/*The IMD to draw for this structure */
	struct 	iIMDShape	*pBaseIMD;	/*The base IMD to draw for this structure */
	struct _ecm_stats	*pECM;		/*Which ECM is standard for the structure - 
									  if any*/
	struct _sensor_stats *pSensor;	/*Which Sensor is standard for the structure - 
									  if any*/
    //NOT USED ANYMORE - AB 24/01/99
	/*UDWORD		weaponSlots;		/Number of weapons that can be attached to the
									  building/
	UDWORD		numWeaps;			/Number of weapons for default /
	SDWORD		defaultWeap;		/The default weapon/
    
	struct _weapon_stats **asWeapList;		/List of pointers to default weapons/
    */
    struct _weapon_stats    *psWeapStat;    //can only have one weapon now

	UDWORD		numFuncs;			/*Number of functions for default*/
	SDWORD		defaultFunc;		/*The default function*/
	struct _function	**asFuncList;		/*List of pointers to allowable functions - 
									  unalterable*/
} STRUCTURE_STATS;

typedef enum _struct_states
{
	SS_BEING_BUILT,
	SS_BUILT,
	SS_BEING_DEMOLISHED,
} STRUCT_STATES;

//this is sizeof(FACTORY) the largest at present 11-2-99 - increased AB 22-04-99
#define	MAX_FUNCTIONALITY_SIZE	40

typedef UBYTE	FUNCTIONALITY[MAX_FUNCTIONALITY_SIZE];

typedef struct _research_facility
{
	struct _base_stats	*psSubject;		/* the subject the structure is working on*/
	UDWORD		capacity;				/* Number of upgrade modules added*/
	UDWORD		timeStarted;			/* The time the building started on the subject*/
	UDWORD		researchPoints;			/* Research Points produced per research cycle*/
	UDWORD		timeToResearch;			/* Time taken to research the topic*/
	struct _base_stats	*psBestTopic;	/* The topic with the most research points 
										   that was last performed*/
	UDWORD		powerAccrued;			/* used to keep track of power before 
										   researching a topic*/
	UDWORD		timeStartHold;		    /* The time the research facility was put on hold*/

} RESEARCH_FACILITY;

typedef struct _factory
{

	UBYTE				capacity;			/* The max size of body the factory 
											   can produce*/
	UBYTE				quantity;			/* The number of droids to produce OR for 
											   selectedPlayer, how many loops to perform*/
	UBYTE				loopsPerformed;		/* how many times the loop has been performed*/
	//struct _propulsion_types*	propulsionType;		
	//UBYTE				propulsionType;		/* The type of propulsion the facility 
	//										   can produce*/
	UBYTE				productionOutput;	/* Droid Build Points Produced Per 
											   Build Cycle*/
	UDWORD				powerAccrued;		/* used to keep track of power before building a droid*/
	BASE_STATS			*psSubject;			/* the subject the structure is working on */
	UDWORD				timeStarted;		/* The time the building started on the subject*/
	UDWORD				timeToBuild;		/* Time taken to build one droid */
	UDWORD				timeStartHold;		/* The time the factory was put on hold*/
	FLAG_POSITION		*psAssemblyPoint;	/* Place for the new droids to assemble at */
	struct _formation	*psFormation;		// formation for the droids that are produced
	struct _droid		*psCommander;	    // command droid to produce droids for (if any)
    UDWORD              secondaryOrder;     // secondary order state for all units coming out of the factory
                                            // added AB 22/04/99
	
    //these are no longer required - yipee!
	// The group the droids produced by this factory belong to - used for Missions
	//struct _droid_group		*psGroup;
	//struct _droid			*psGrpNext;

} FACTORY;

typedef struct _res_extractor
{
	UDWORD				power;				/*The max amount of power that can be extracted*/
	UDWORD				timeLastUpdated;	/*time the Res Extr last got points*/
	BOOL				active;				/*indicates when the extractor is on ie digging up oil*/
	struct _structure	*psPowerGen;		/*owning power generator*/
} RES_EXTRACTOR;

typedef struct _power_gen
{
	UDWORD				power;				/*The max power that can be used - NOT USED 21/04/98*/
	UDWORD				multiplier;			/*Factor to multiply output by - percentage*/
	UDWORD				capacity;			/* Number of upgrade modules added*/

	//struct _structure	*apResExtractors[NUM_POWER_MODULES + 1];/*pointers to the res ext
	struct _structure	*apResExtractors[NUM_POWER_MODULES];/*pointers to the res ext
																associated with this gen*/
} POWER_GEN;

typedef struct REPAIR_FACILITY
{
	UDWORD				power;				/* Power used in repairing */
	UDWORD				timeStarted;		/* Time repair started on current object */
	BASE_OBJECT			*psObj;				/* Object being repaired */
	UDWORD				powerAccrued;		/* used to keep track of power before 
											   repairing a droid */
	FLAG_POSITION		*psDeliveryPoint;	/* Place for the repaired droids to assemble
                                               at */
    UDWORD              currentPtsAdded;    /* stores the amount of body points added to the unit
                                               that is being worked on */

	// The group the droids to be repaired by this facility belong to
	struct _droid_group		*psGroup;
	struct _droid			*psGrpNext;
} REPAIR_FACILITY;

typedef struct _rearm_pad
{
	UDWORD				reArmPoints;		/* rearm points per cycle				 */
	UDWORD				timeStarted;		/* Time reArm started on current object	 */
	BASE_OBJECT			*psObj;				/* Object being rearmed		             */
    UDWORD              currentPtsAdded;    /* stores the amount of body points added to the unit
                                               that is being worked on */
} REARM_PAD;


//this structure is used whenever an instance of a building is required in game
typedef struct _structure
{
	/* The common structure elements for all objects */
	BASE_ELEMENTS(struct _structure);

//	UDWORD		ref;
	STRUCTURE_STATS*	pStructureType;		/* pointer to the structure stats for this 
											   type of building */	
	UBYTE		status;						/* defines whether the structure is being 
											   built, doing nothing or performing a function*/
	//SDWORD		currentBuildPts;			/* the build points currently assigned to this 
    SWORD		currentBuildPts;			/* the build points currently assigned to this 
											   structure */
    SWORD       currentPowerAccrued;        /* the power accrued for building this structure*/
	UWORD		body;						/* current body points */
	//UDWORD		body;						/* current body points */
	//UDWORD		baseBodyPoints;				/* undamaged body points */
	UWORD		armour;						/* current armour points */
	//UDWORD		armour;						/* current armour points */
	//SDWORD		resistance;					/* current resistance points 
	SWORD		resistance;					/* current resistance points 
											   0 = cannot be attacked electrically*/
	UDWORD		lastResistance;				/* time the resistance was last increased*/
	//UDWORD		repair;						/* current repair points */

// repair doesn't seem to be used anywhere ... I'll take it out for the mo. and remove it from the new savegame stuff
// talk to me if you are having problems with this
//	UWORD		repair;						/* current repair points */

	/* The other structure data.  These are all derived from the functions
	 * but stored here for easy access - will need to add more for variable stuff!
	 */
	//the sensor stats need to be stored since the actual sensor stat can change with research
	UWORD		sensorRange;
	UWORD		sensorPower;
	/*UDWORD		turretRotation;				// weapon, ECM and sensor direction and pitch
	UDWORD		turretRotRate;				// weapon, ECM and sensor direction and pitch
	UDWORD		turretPitch;				// weapon, ECM and sensor direction and pitch*/
	UWORD		turretRotation;				// weapon, ECM and sensor direction and pitch
	//UWORD		turretRotRate;				// weapon, ECM and sensor direction and pitch - THIS IS A CONSTANT
	UWORD		turretPitch;				// weapon, ECM and sensor direction and pitch

	UDWORD		timeLastHit;				//the time the structure was last attacked
	UDWORD		lastHitWeapon;
	UWORD		radarX;
	UWORD		radarY;
	//the ecm power needs to be stored since the actual ecm stat can change with research
	UWORD		ecmPower;
	//FRACT		heightScale;	

	FUNCTIONALITY	*pFunctionality;		/* pointer to structure that contains fields
											   necessary for functionality */
	/* The weapons on the structure */
	//UWORD		numWeaps;
	UBYTE		targetted;
	WEAPON		asWeaps[STRUCT_MAXWEAPS];
	BASE_OBJECT	*psTarget;

	/* anim data */
	ANIM_OBJECT	*psCurAnim;

} STRUCTURE;

#define LOTS_OF	255						/*highest number the limit can be set to */
typedef struct _structure_limits
{
	UBYTE		limit;				/* the number allowed to be built */
	UBYTE		currentQuantity;	/* the number of the type currently 
												   built per player*/
	UBYTE		globalLimit;		// multiplayer only. sets the max value selectable (limits changed by player)
} STRUCTURE_LIMITS;


//the three different types of factory (currently) - FACTORY, CYBORG_FACTORY, VTOL_FACTORY
// added repair facilities as they need an assebly point as well
#define NUM_FACTORY_TYPES	3
#define FACTORY_FLAG		0
#define CYBORG_FLAG			1
#define VTOL_FLAG			2
#define REPAIR_FLAG			3
//seperate the numfactory from numflag
#define NUM_FLAG_TYPES      4

//this is used for module graphics - factory and vtol factory
#define NUM_FACMOD_TYPES	2

typedef struct _production_run
{
	UBYTE						quantity;			//number to build
	UBYTE						built;				//number built on current run
	struct _droid_template		*psTemplate;		//template to build
} PRODUCTION_RUN;

/* structure stats which can be upgraded by research*/
typedef struct _structure_upgrade
{
	UWORD			armour;
	UWORD			body;
	UWORD			resistance;
} STRUCTURE_UPGRADE;

/* wall/Defence structure stats which can be upgraded by research*/
typedef struct _wallDefence_upgrade
{
	UWORD			armour;
	UWORD			body;
} WALLDEFENCE_UPGRADE;

typedef struct _upgrade
{
	UWORD		modifier;		//% to increase the stat by
} UPGRADE;

typedef UPGRADE		RESEARCH_UPGRADE;
typedef UPGRADE		PRODUCTION_UPGRADE;
typedef UPGRADE		REPAIR_FACILITY_UPGRADE;
typedef UPGRADE		POWER_UPGRADE;
typedef UPGRADE		REARM_UPGRADE;

#endif
