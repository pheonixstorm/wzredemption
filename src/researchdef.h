/*
 * ResearchDef.h
 *
 * Structure definitions for research
 *
 */
#ifndef _researchdef_h
#define _researchdef_h

/* Research struct type definitions */
typedef enum
{
	TC_MAJOR,
	TC_MINOR,
} TECH_CODE;

typedef struct research_stats
{
	STATS_BASE;
	UBYTE			techCode;
	//STRING			*pTechnologyName;	/* Text name of the group the research is a 
	//									   member of */
	TECH_LEVEL		techLevel;			/* technology level of the research topic */
//	STRING			*pSubGroupName;		/* Text name of the Subgroup to which the research belongs */
	UWORD       	subGroup;			/* Subgroup of the item - an iconID from 'Framer' to depict in the button*/

	UWORD			researchPoints;		/* Number of research points required to 
										   complete the research */
	UDWORD			researchPower;		/* Power cost to research */
	UBYTE			keyTopic;			/* Flag to indicate whether in single player 
										   this topic must be explicitly enabled*/
	UBYTE			storeCount;			/* used to load in the following lists*/
	UBYTE			numPRRequired;	
	//UDWORD			*pPRList;			/* List of research pre-requisites */
    //needs to be UWORD sized for Patches
    UWORD			*pPRList;			/* List of research pre-requisites */
    //UBYTE			*pPRList;			/* List of research pre-requisites */
	UBYTE			numStructures;
	//UDWORD			*pStructList;		/* List of structures that when built would 
    UWORD			*pStructList;		/* List of structures that when built would 
										   enable this research */
	UBYTE			numFunctions;
	struct _function	**pFunctionList; /* List of functions that can be performed 
										   on completion of research */
	UBYTE			numRedStructs;		
	//UDWORD			*pRedStructs;		/* List of Structures that become redundant */
    UWORD			*pRedStructs;		/* List of Structures that become redundant */
	UBYTE			numRedArtefacts;
	COMP_BASE_STATS	**pRedArtefacts;	/*List of Artefacts that become redundant */
	UBYTE			numStructResults;
	//UDWORD			*pStructureResults;	/*List of Structures that are possible after
    UWORD			*pStructureResults;	/*List of Structures that are possible after
										  this research */
	UBYTE			numArteResults;		
	COMP_BASE_STATS	**pArtefactResults;	/*List of Artefacts that are possible after
										  this research*/
	COMP_BASE_STATS	**pReplacedArtefacts;/*List of artefacts that are replaced by the above 
										  result - not necessarily any! 1 to 1 relation with 
										  above list */
	struct _viewdata	*pViewData;		/*data used to display a message in the 
										  Intelligence Screen*/
	UWORD			iconID;				/* the ID from 'Framer' for which graphic to draw in interface*/
    BASE_STATS      *psStat;            /* A stat used to define which graphic is 
                                           drawn instead of the two fields below*/
	struct	iIMDShape		*pIMD;		/* the IMD to draw for this research topic */
	struct	iIMDShape		*pIMD2;		/* the 2nd IMD for base plates/turrets*/
} RESEARCH;

typedef struct _player_research
{
	UDWORD		currentPoints;			// If the research has been suspended then this value contains the number of points generated at the suspension/cancel point
										// normally it is null
	UBYTE		ResearchStatus;			// Bit flags   ...  see below



//	UBYTE		possible;				/* Flag to specify whether the research is possible - so
//										   can enable topics vis scripts */
//	UBYTE		researched;				/* Flag to specify whether the research is 
//										   complete	*/
} PLAYER_RESEARCH;

#define STARTED_RESEARCH	0x01		// research in progress
#define CANCELLED_RESEARCH	0x02		// research has been canceled
#define RESEARCHED			0x04		// research is complete
#define RESBITS (STARTED_RESEARCH|CANCELLED_RESEARCH|RESEARCHED)
#define POSSIBLE			0x80		// is the research possible ... so can enable topics vis scripts


#define IsResearchPossible(x)   ((x)->ResearchStatus&POSSIBLE)
#define MakeResearchPossible(x)	((x)->ResearchStatus|=POSSIBLE)
#define IsResearchCompleted(x)   ((x)->ResearchStatus&RESEARCHED)

#define MakeResearchCompleted(x)	((x)->ResearchStatus=((x)->ResearchStatus&(~RESBITS))|RESEARCHED  )

#define IsResearchCancelled(x)   ((x)->ResearchStatus&CANCELLED_RESEARCH)
#define MakeResearchCancelled(x)	((x)->ResearchStatus=((x)->ResearchStatus&(~RESBITS))|CANCELLED_RESEARCH)

#define IsResearchStarted(x)   ((x)->ResearchStatus&STARTED_RESEARCH)
#define MakeResearchStarted(x)	((x)->ResearchStatus=((x)->ResearchStatus&(~RESBITS))|STARTED_RESEARCH  )

// clear all bits in the status except for the possible bit
#define ResetResearchStatus(x) ((x)->ResearchStatus&=(~RESBITS))
#endif

