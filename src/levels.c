/*
 * Levels.c
 *
 * Control the data loading for game levels
 *
 */

#include "ctype.h"

// levLoadData printf's
#define DEBUG_GROUP0
#include "Frame.h"
#include "Init.h"
#include "Objects.h"
#include "HCI.h"
#include "Levels.h"
#include "Mission.h"
#include "LevelInt.h"
#include "Game.h"
#include "Lighting.h"
#include "pieState.h"
#include "Data.h"
#include "MultiWDG.h"

//#ifdef DEBUG
#include "script.h"
#include "ScriptTabs.h"
//#endif

// minimum type number for a type instruction
#define MULTI_TYPE_START	10


// block ID number start for the current level data (as opposed to a dataset)
#define CURRENT_DATAID		LEVEL_MAXFILES

static	UBYTE	currentLevelName[32];

// the current level descriptions
LEVEL_DATASET	*psLevels;

// the currently loaded data set
LEVEL_DATASET	*psBaseData;
LEVEL_DATASET	*psCurrLevel;

// dummy level data for single WRF loads
LEVEL_DATASET	sSingleWRF;

// return values from the lexer
STRING *pLevToken;
SDWORD levVal;
SDWORD levelLoadType;
// modes for the parser
enum
{
	LP_START,		// no input received
	LP_LEVEL,		// level token received
	LP_LEVELDONE,	// defined a level waiting for players/type/data
	LP_PLAYERS,		// players token received
	LP_TYPE,		// type token received
	LP_DATASET,		// dataset token received
	LP_WAITDATA,	// defining level data, waiting for data token
	LP_DATA,		// data token received
	LP_GAME,		// game token received
};

/*// the current data file to parse
static UBYTE	*pDataFile;
static SDWORD	dataFileSize;

// the current position in the data file
static UBYTE	*pDataPtr;
static SDWORD	levLine;

// the token buffer
#define TOKEN_MAX	255
static STRING	aTokenBuff[TOKEN_MAX];
*/

// initialise the level system
BOOL levInitialise(void)
{
	psLevels = NULL;
	psBaseData = NULL;
	psCurrLevel = NULL;
	
	return TRUE;
}

SDWORD getLevelLoadType(void)
{
	return levelLoadType;
}

// shutdown the level system
void levShutDown(void)
{
	LEVEL_DATASET	*psNext;
	SDWORD			i;

	while (psLevels)
	{
		FREE(psLevels->pName);
		for(i=0; i<LEVEL_MAXFILES; i++)
		{
			if (psLevels->apDataFiles[i] != NULL)
			{
				FREE(psLevels->apDataFiles[i]);
			}
		}
		psNext = psLevels->psNext;
		FREE(psLevels);
		psLevels = psNext;
	}
}


// error report function for the level parser
void levError(STRING *pError)
{
	char	*pText;
	int		line;

	levGetErrorData(&line, &pText);

#ifdef DEBUG
	ASSERT((FALSE, "Level File parse error:\n%s at line %d text %s\n", pError, line, pText));
#else
	DBERROR(("Level File parse error:\n%s at line %d text %s\n", pError, line, pText));
#endif
}

// find the level dataset
BOOL levFindDataSet(STRING *pName, LEVEL_DATASET **ppsDataSet)
{
	LEVEL_DATASET	*psNewLevel;

	for(psNewLevel = psLevels; psNewLevel; psNewLevel = psNewLevel->psNext)
	{
		if (psNewLevel->pName != NULL)
		{
			if (strcmp(psNewLevel->pName, pName) == 0)
			{
				*ppsDataSet = psNewLevel;
				return TRUE;
			}
		}
	}

	return FALSE;
}

// parse a level description data file
BOOL levParse(UBYTE *pBuffer, SDWORD size)
{
	SDWORD			token, state, currData=0;
	LEVEL_DATASET	*psDataSet = NULL;
	LEVEL_DATASET	*psFoundData;

	levSetInputBuffer(pBuffer, size);

	state = LP_START;
	token = lev_lex();
	while (token != 0)
	{
		switch (token)
		{
		case LTK_LEVEL:
		case LTK_CAMPAIGN:
		case LTK_CAMSTART:
		case LTK_CAMCHANGE:
		case LTK_EXPAND:
		case LTK_BETWEEN:
		case LTK_MKEEP:
		case LTK_MCLEAR:
        case LTK_EXPAND_LIMBO:
        case LTK_MKEEP_LIMBO:
			if (state == LP_START || state == LP_WAITDATA)
			{
				// start a new level data set
				psDataSet = MALLOC(sizeof(LEVEL_DATASET));
				if (!psDataSet)
				{
					levError("Out of memory");
					return FALSE;
				}
				memset(psDataSet, 0, sizeof(LEVEL_DATASET));
				psDataSet->players = 1;
				psDataSet->game = -1;
				LIST_ADDEND(psLevels, psDataSet, LEVEL_DATASET);
				currData = 0;

				// set the dataset type
				switch (token)
				{
				case LTK_LEVEL:
					psDataSet->type = LDS_COMPLETE;
					break;
				case LTK_CAMPAIGN:
					psDataSet->type = LDS_CAMPAIGN;
					break;
				case LTK_CAMSTART:
					psDataSet->type = LDS_CAMSTART;
					break;
				case LTK_BETWEEN:
					psDataSet->type = LDS_BETWEEN;
					break;
				case LTK_MKEEP:
					psDataSet->type = LDS_MKEEP;
					break;
#ifndef COVERMOUNT
				case LTK_CAMCHANGE:
					psDataSet->type = LDS_CAMCHANGE;
					break;
				case LTK_EXPAND:
					psDataSet->type = LDS_EXPAND;
					break;
				case LTK_MCLEAR:
					psDataSet->type = LDS_MCLEAR;
					break;
				case LTK_EXPAND_LIMBO:
					psDataSet->type = LDS_EXPAND_LIMBO;
					break;
				case LTK_MKEEP_LIMBO:
					psDataSet->type = LDS_MKEEP_LIMBO;
					break;
#endif
				default:
					ASSERT((FALSE,"eh?"));
					break;
				}
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			state = LP_LEVEL;
			break;
		case LTK_PLAYERS:
			if (state == LP_LEVELDONE &&
				(psDataSet->type == LDS_COMPLETE || psDataSet->type >= MULTI_TYPE_START))
			{
				state = LP_PLAYERS;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		case LTK_TYPE:
			if (state == LP_LEVELDONE && psDataSet->type == LDS_COMPLETE)
			{
				state = LP_TYPE;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		case LTK_INTEGER:
			if (state == LP_PLAYERS)
			{
				psDataSet->players = (SWORD)levVal;
			}
			else if (state == LP_TYPE)
			{
				if (levVal < MULTI_TYPE_START)
				{
					levError("invalid type number");
					return FALSE;
				}

				psDataSet->type = (SWORD)levVal;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			state = LP_LEVELDONE;
			break;
		case LTK_DATASET:
			if (state == LP_LEVELDONE && psDataSet->type != LDS_COMPLETE)
			{
				state = LP_DATASET;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		case LTK_DATA:
			if (state == LP_WAITDATA)
			{
				state = LP_DATA;
			}
			else if (state == LP_LEVELDONE)
			{
				if (psDataSet->type == LDS_CAMSTART ||
					psDataSet->type == LDS_MKEEP 
#ifndef COVERMOUNT
					||psDataSet->type == LDS_CAMCHANGE ||
					psDataSet->type == LDS_EXPAND ||
					psDataSet->type == LDS_MCLEAR ||
                    psDataSet->type == LDS_EXPAND_LIMBO ||
                    psDataSet->type == LDS_MKEEP_LIMBO
#endif
					)
				{
					levError("Missing dataset command");
					return FALSE;
				}
				state = LP_DATA;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		case LTK_GAME:
			if ((state == LP_WAITDATA || state == LP_LEVELDONE) &&
				psDataSet->game == -1 && psDataSet->type != LDS_CAMPAIGN)
			{
				state = LP_GAME;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		case LTK_IDENT:
			if (state == LP_LEVEL)
			{
#ifndef COVERMOUNT
				if (psDataSet->type == LDS_CAMCHANGE)
				{
					// campaign change dataset - need to find the full data set
					if (!levFindDataSet(pLevToken, &psFoundData))
					{
						levError("Cannot find full data set for camchange");
						return FALSE;
					}

					if (psFoundData->type != LDS_CAMSTART)
					{
						levError("Invalid data set name for cam change");
						return FALSE;
					}
					psFoundData->psChange = psDataSet;
				}
#endif
				// store the level name
				psDataSet->pName = MALLOC(strlen(pLevToken) + 1);
				if (!psDataSet->pName)
				{
					levError("Out of memory");
					return FALSE;
				}
				strcpy(psDataSet->pName, pLevToken);
				state = LP_LEVELDONE;
			}
			else if (state == LP_DATASET)
			{
				// find the dataset
				if (!levFindDataSet(pLevToken, &psDataSet->psBaseData))
				{
					levError("Unknown dataset");
					return FALSE;
				}
				state = LP_WAITDATA;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		case LTK_STRING:
			if (state == LP_DATA || state == LP_GAME)
			{
				if (currData >= LEVEL_MAXFILES)
				{
					levError("Too many data files");
					return FALSE;
				}

				// note the game index if necessary
				if (state == LP_GAME)
				{
					psDataSet->game = (SWORD)currData;
				}

				// store the data name
				psDataSet->apDataFiles[currData] = MALLOC(strlen(pLevToken) + 1);
				if (!psDataSet->apDataFiles[currData])
				{
					levError("Out of memory");
					return FALSE;
				}
				resToLower(pLevToken);
				strcpy(psDataSet->apDataFiles[currData], pLevToken);

				currData += 1;
				state = LP_WAITDATA;
			}
			else
			{
				levError("Syntax Error");
				return FALSE;
			}
			break;
		default:
			levError("Unexpected token");
			break;
		}

		// get the next token
		token = lev_lex();
	}

	if (state != LP_WAITDATA || currData == 0)
	{
		levError("Unexpected end of file");
		return FALSE;
	}
	
	return TRUE;
}


// free the data for the current mission
BOOL levReleaseMissionData(void)
{
	SDWORD i;

	// release old data if any was loaded
	if (psCurrLevel != NULL)
	{
		if (!stageThreeShutDown())
		{
			return FALSE;
		}

		if ((psCurrLevel->type == LDS_COMPLETE ||
			 psCurrLevel->type >= MULTI_TYPE_START) && psCurrLevel->game == -1)
		{
			BLOCK_RESET(psMissionHeap);
		}

		// free up the old data
		for(i=LEVEL_MAXFILES-1; i >= 0; i--)
		{
			if (i == psCurrLevel->game)
			{
				BLOCK_RESET(psMissionHeap);
				if (psCurrLevel->psBaseData == NULL)
				{
					if (!stageTwoShutDown())
					{
						return FALSE;
					}
				}
			}
			else// if (psCurrLevel->apDataFiles[i])
			{
                
				resReleaseBlockData(i + CURRENT_DATAID);
			}
		}
//#ifndef COVERMOUNT
		if (psCurrLevel->type == LDS_BETWEEN)
		{
			BLOCK_RESET(psMissionHeap);
		}
//#endif

	}

	return TRUE;
}



// free the currently loaded dataset
BOOL levReleaseAll(void)
{
	SDWORD i;

	// release old data if any was loaded
	if (psCurrLevel != NULL)
	{
		if (!levReleaseMissionData())
		{
			return FALSE;
		}

		// release the game data
		if (psCurrLevel->psBaseData != NULL)
		{
			if (!stageTwoShutDown())
			{
				return FALSE;
			}
		}


		if (psCurrLevel->psBaseData)
		{
			for(i=LEVEL_MAXFILES-1; i >= 0; i--)
			{
				if (psCurrLevel->psBaseData->apDataFiles[i])
				{
					resReleaseBlockData(i);
				}
			}
		}

		if (!stageOneShutDown())
		{
			return FALSE;
		}

		BLOCK_RESET(psGameHeap);
	}

	psCurrLevel=NULL;

	return TRUE;
}

// load up a single wrf file
BOOL levLoadSingleWRF(STRING *pName)
{
	// free the old data
	levReleaseAll();

	// create the dummy level data
	memset(&sSingleWRF, 0, sizeof(LEVEL_DATASET));
	sSingleWRF.pName = pName;

	// load up the WRF
	if (!stageOneInitialise())
	{
		return FALSE;
	}
	BLOCK_RESET(psGameHeap);
	memSetBlockHeap(psGameHeap);
	// load the data
	DBPRINTF(("Loading %s ...\n", pName));
	if (!resLoad(pName, 0,
				 DisplayBuffer, displayBufferSize,
				 psGameHeap))
	{
		return FALSE;
	}

	BLOCK_RESET(psMissionHeap);
	memSetBlockHeap(psMissionHeap);

	if (!stageThreeInitialise())
	{
		return FALSE;
	}

	psCurrLevel = &sSingleWRF;

	return TRUE;
}


// load up the base data set for a level (used by savegames)
BOOL levLoadBaseData(STRING *pName)
{
	LEVEL_DATASET	*psNewLevel, *psBaseData;
	SDWORD			i;

	DBPRINTF(("Loading base data for level %s\n", pName));

	// find the level dataset
	if (!levFindDataSet(pName, &psNewLevel))
	{
		DBERROR(("levLoadBaseData: couldn't find level data"));
		return FALSE;
	}

	if (psNewLevel->type != LDS_CAMSTART &&
		psNewLevel->type != LDS_MKEEP 
#ifndef COVERMOUNT
		&& psNewLevel->type != LDS_EXPAND &&
		psNewLevel->type != LDS_MCLEAR &&
        psNewLevel->type != LDS_EXPAND_LIMBO &&
        psNewLevel->type != LDS_MKEEP_LIMBO
#endif
		)
	{
		DBERROR(("levLoadBaseData: incorect level type"));
		return FALSE;
	}

	// clear all the old data
	levReleaseAll();

	// basic game data is loaded in the game heap
	memSetBlockHeap(psGameHeap);

	// initialise
	BLOCK_RESET(psGameHeap);
	if (!stageOneInitialise())
	{
		return FALSE;
	}

	// load up the base dataset
	psBaseData = psNewLevel->psBaseData;
	for(i=0; i<LEVEL_MAXFILES; i++)
	{
		if (psBaseData->apDataFiles[i])
		{
			// load the data
			DBPRINTF(("Loading %s ...\n", psBaseData->apDataFiles[i]));
			if (!resLoad(psBaseData->apDataFiles[i], i,
						 DisplayBuffer, displayBufferSize,
						 psGameHeap))
			{
				return FALSE;
			}
		}
	}

	psCurrLevel = psNewLevel;

	return TRUE;
}

UBYTE	*getLevelName( void )
{
	return(currentLevelName);
}


// load up the data for a level
BOOL levLoadData(STRING *pName, STRING *pSaveName, SDWORD saveType)
{
	LEVEL_DATASET	*psNewLevel, *psBaseData, *psChangeLevel;
	SDWORD			i;
	BLOCK_HEAP		*psCurrHeap;
    BOOL            bCamChangeSaveGame;

	DBPRINTF(("Loading level %s\n", pName));

	// reset fog
//	fogStatus = 0;
//	pie_EnableFog(FALSE);//removed, always set by script or save game
	
	levelLoadType = saveType;

	// find the level dataset
	if (!levFindDataSet(pName, &psNewLevel))
	{
		DBMB(("levLoadData: dataset %s not found - trying to load as WRF", pName));
		return levLoadSingleWRF(pName);
	}

	/* Keep a copy of the present level name */
	strcpy(currentLevelName,pName);

    bCamChangeSaveGame = FALSE;
    if (pSaveName AND saveType == GTYPE_SAVE_START)
    {
        if (psNewLevel->psChange != NULL)
        {
            bCamChangeSaveGame = TRUE;
        }
    }

	// select the change dataset if there is one
    psChangeLevel = NULL;
	if (((psNewLevel->psChange != NULL) && (psCurrLevel != NULL)) OR bCamChangeSaveGame)
	{
        //store the level name
		DBP0(("levLoadData: Found CAMCHANGE dataset\n"));
        psChangeLevel = psNewLevel;
		psNewLevel = psNewLevel->psChange;
	}

	// ensure the correct dataset is loaded
	if (psNewLevel->type == LDS_CAMPAIGN)
	{
		DBERROR(("levLoadData: Cannot load a campaign dataset (%s)", psNewLevel->pName));
		return FALSE;
	}
	else
	{
		if (psCurrLevel != NULL)
		{
			if ((psCurrLevel->psBaseData != psNewLevel->psBaseData) ||
				(psCurrLevel->type < LDS_NONE && psNewLevel->type  >= LDS_NONE) ||
				(psCurrLevel->type >= LDS_NONE && psNewLevel->type  < LDS_NONE))
			{
				// there is a dataset loaded but it isn't the correct one
				DBP0(("levLoadData: Incorrect base dataset loaded - levReleaseAll()\n"));
				levReleaseAll();	// this sets psCurrLevel to NULL
			}
		}

		// setup the correct dataset to load if necessary
		if (psCurrLevel == NULL)
		{
#ifdef DEBUG_GROUP0
			if (psNewLevel->psBaseData != NULL)
			{
				DBP0(("levLoadData: Setting base dataset to load: %s\n", psNewLevel->psBaseData->pName));
			}
#endif
			psBaseData = psNewLevel->psBaseData;
		}
		else
		{
			DBP0(("levLoadData: No base dataset to load\n"));
			psBaseData = NULL;
		}
	}

	// if this is a single player level - disable the multiple WDG
	if (psNewLevel->type < LDS_NONE)
	{
		wdgDisableAddonWDG();
	}

	// reset the old mission data if necessary
	if (psCurrLevel != NULL)
	{
		DBP0(("levLoadData: reseting old mission data\n"));
		if (!gameReset())
		{
			return FALSE;
		}
		if (!levReleaseMissionData())
		{
			return FALSE;
		}
	}

	// need to free the current map and droids etc for a save game
	if ((psBaseData == NULL) &&
		(pSaveName != NULL))
	{
		if (!saveGameReset())
		{
			return FALSE;
		}
	}


	// basic game data is loaded in the game heap
	DBP0(("levLoadData: Setting game heap\n"));
	memSetBlockHeap(psGameHeap);

	// initialise if necessary
	if (psNewLevel->type == LDS_COMPLETE || //psNewLevel->type >= MULTI_TYPE_START ||
		psBaseData != NULL)
	{
		DBP0(("levLoadData: reset game heap\n"));
		BLOCK_RESET(psGameHeap);
		if (!stageOneInitialise())
		{
			return FALSE;
		}
	}

	// load up a base dataset if necessary
	if (psBaseData != NULL)
	{
		DBP0(("levLoadData: loading base dataset %s\n", psBaseData->pName));
		for(i=0; i<LEVEL_MAXFILES; i++)
		{
			if (psBaseData->apDataFiles[i])
			{
				// load the data
				DBPRINTF(("Loading %s ...\n", psBaseData->apDataFiles[i]));
				if (!resLoad(psBaseData->apDataFiles[i], i,
							 DisplayBuffer, displayBufferSize,
							 psGameHeap))
				{
					return FALSE;
				}
			}
		}
	}
#ifndef COVERMOUNT
	if (psNewLevel->type == LDS_CAMCHANGE)
	{
		if (!campaignReset())
		{
			return FALSE;
		}
	}
#endif
	if (psNewLevel->game == -1)  //no .gam file to load - BETWEEN missions (for Editor games only)
	{
//#ifndef COVERMOUNT

		ASSERT((psNewLevel->type == LDS_BETWEEN,
			"levLoadData: only BETWEEN missions do not need a .gam file"));
//#endif
		DBP0(("levLoadData: no .gam file for level: BETWEEN mission\n"));
		if (pSaveName != NULL)
		{
			if (psBaseData != NULL)
			{
				if (!stageTwoInitialise())
				{
					return FALSE;
				}
			}

			DBP0(("levLoadData: setting map heap\n"));
			BLOCK_RESET(psMapHeap);
			memSetBlockHeap(psMapHeap);

            //set the mission type before the saveGame data is loaded
			if (saveType == GTYPE_SAVE_MIDMISSION)
			{
				DBP0(("levLoadData: init mission stuff\n"));
				if (!startMissionSave(psNewLevel->type))
				{
					return FALSE;
				}

				DBP0(("levLoadData: dataSetSaveFlag\n"));
				dataSetSaveFlag();
			}

            DBP0(("levLoadData: loading savegame: %s\n", pSaveName));
			if (!loadGame(pSaveName, FALSE, TRUE,TRUE))
			{
				return FALSE;
			}

			if (!newMapInitialise())
			{
				return FALSE;
			}
		}

		if ((pSaveName == NULL) ||
			(saveType == GTYPE_SAVE_START))
		{
			DBP0(("levLoadData: start mission - no .gam\n"));
			if (!startMission(psNewLevel->type, NULL))
			{
				return FALSE;
			}
		}

		DBP0(("levLoadData: setting mission heap\n"));
		BLOCK_RESET(psMissionHeap);
		memSetBlockHeap(psMissionHeap);
	}

    //we need to load up the save game data here for a camchange
    if (bCamChangeSaveGame)
    {
		DBP0(("levLoadData: no .gam file for level: BETWEEN mission\n"));
		if (pSaveName != NULL)
		{
			if (psBaseData != NULL)
			{
				if (!stageTwoInitialise())
				{
					return FALSE;
				}
			}

			DBP0(("levLoadData: setting map heap\n"));
			BLOCK_RESET(psMapHeap);
			memSetBlockHeap(psMapHeap);

            DBP0(("levLoadData: loading savegame: %s\n", pSaveName));
			if (!loadGame(pSaveName, FALSE, TRUE,TRUE))
			{
				return FALSE;
			}

    		if (!campaignReset())
	    	{
		    	return FALSE;
		    }

            //we now need to go to the next level
            //psNewLevel = psChangeLevel;
            //psChangeLevel = NULL;

            //stageTwoShutDown??
            //block_reset??
        }
    }


	// load the new data
	DBP0(("levLoadData: loading mission dataset: %s\n", psNewLevel->pName));
	psCurrHeap = memGetBlockHeap();
	for(i=0; i<LEVEL_MAXFILES; i++)
	{
		if (psNewLevel->game == i)
		{
			// do some more initialising if necessary
			if (psNewLevel->type == LDS_COMPLETE || psNewLevel->type >= MULTI_TYPE_START ||
				(psBaseData != NULL AND !bCamChangeSaveGame))
			{
iV_Reset(FALSE);//unload font, to avoid crash on 8th load... ajl 15/sep/99
				if (!stageTwoInitialise())
				{
					return FALSE;
				}

				DBP0(("levLoadData: setting map heap\n"));
				BLOCK_RESET(psMapHeap);
				memSetBlockHeap(psMapHeap);
				psCurrHeap = psMapHeap;
			}

			// missions with a seperate map have to use the mission heap now
			if ((psNewLevel->type == LDS_MKEEP 
#ifndef COVERMOUNT
				 ||psNewLevel->type == LDS_MCLEAR
                 ||psNewLevel->type == LDS_MKEEP_LIMBO
#endif
				  ) &&
				pSaveName == NULL)
			{
				DBP0(("levLoadData: setting mission heap\n"));
				BLOCK_RESET(psMissionHeap);
				memSetBlockHeap(psMissionHeap);
				psCurrHeap = psMissionHeap;
			}

			// load a savegame if there is one - but not if already done so
			if (pSaveName != NULL AND !bCamChangeSaveGame)
			{
				// make sure the map gets loaded into the right heap
				DBP0(("levLoadData: setting map heap\n"));
				BLOCK_RESET(psMapHeap);
				memSetBlockHeap(psMapHeap);
				psCurrHeap = psMapHeap;

                //set the mission type before the saveGame data is loaded
				if (saveType == GTYPE_SAVE_MIDMISSION)
				{
					DBP0(("levLoadData: init mission stuff\n"));
					if (!startMissionSave(psNewLevel->type))
					{
						return FALSE;
					}

					DBP0(("levLoadData: dataSetSaveFlag\n"));
					dataSetSaveFlag();
				}

                DBP0(("levLoadData: loading save game %s\n", pSaveName));
				if (!loadGame(pSaveName, FALSE, TRUE,TRUE))
				{
					return FALSE;
				}

/*				if (saveType == GTYPE_SAVE_START)
				{
					// do not load any more data
					break;
				}*/
			}

			if ((pSaveName == NULL) ||
				(saveType == GTYPE_SAVE_START))
			{
				// load the game
				DBPRINTF(("Loading scenario file %s ...", psNewLevel->apDataFiles[i]));
				switch (psNewLevel->type)
				{
				case LDS_COMPLETE:
				case LDS_CAMSTART:
					DBPRINTF(("COMPLETE / CAMSTART\n"));
					//if (!startMission(MISSION_CAMPSTART, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_CAMSTART, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
				case LDS_BETWEEN:
					DBPRINTF(("BETWEEN\n"));
					if (!startMission(LDS_BETWEEN, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
				
				case LDS_MKEEP:
					DBPRINTF(("MKEEP\n"));
					//if (!startMission(MISSION_OFFKEEP, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_MKEEP, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
#ifndef COVERMOUNT
				case LDS_CAMCHANGE:
					DBPRINTF(("CAMCHANGE\n"));
					//if (!startMission(MISSION_CAMPSTART, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_CAMCHANGE, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
			
				case LDS_EXPAND:
					DBPRINTF(("EXPAND\n"));
					//if (!startMission(MISSION_CAMPEXPAND, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_EXPAND, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
				case LDS_EXPAND_LIMBO:
					DBPRINTF(("EXPAND_LIMBO\n"));
					//if (!startMission(MISSION_CAMPEXPAND, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_EXPAND_LIMBO, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
			
				case LDS_MCLEAR:
					DBPRINTF(("MCLEAR\n"));
					//if (!startMission(MISSION_OFFCLEAR, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_MCLEAR, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
				case LDS_MKEEP_LIMBO:
					DBPRINTF(("MKEEP_LIMBO\n"));
					//if (!startMission(MISSION_OFFKEEP, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_MKEEP_LIMBO, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
#endif
				default:
					ASSERT((psNewLevel->type >= MULTI_TYPE_START,
						"levLoadData: Unexpected mission type"));
					DBPRINTF(("MULTIPLAYER\n"));
					//if (!startMission(MISSION_CAMPSTART, psNewLevel->apDataFiles[i]))
					if (!startMission(LDS_CAMSTART, psNewLevel->apDataFiles[i]))
					{
						return FALSE;
					}
					break;
				}
			}

			// set the view position if necessary
			if ((pSaveName != NULL)
				 ||(psNewLevel->type != LDS_BETWEEN)
#ifndef COVERMOUNT
				 &&(psNewLevel->type != LDS_EXPAND) 
				 &&(psNewLevel->type != LDS_EXPAND_LIMBO)
#endif
				)
			{
				if (!newMapInitialise())
				{
					return FALSE;
				}
			}

			// set the mission heap now if it isn't already being used
			if (memGetBlockHeap() != psMissionHeap)
			{
				DBP0(("levLoadData: setting mission heap\n"));
				BLOCK_RESET(psMissionHeap);
				memSetBlockHeap(psMissionHeap);
			}
			psCurrHeap = psMissionHeap;
		}
		else if (psNewLevel->apDataFiles[i])
		{
			// load the data
			DBPRINTF(("Loading %s ...\n", psNewLevel->apDataFiles[i]));
			if (!resLoad(psNewLevel->apDataFiles[i], i + CURRENT_DATAID,
						 DisplayBuffer, displayBufferSize,
						 psCurrHeap))
			{
				return FALSE;
			}
		}
	}

	dataClearSaveFlag();

	// set the mission heap now if it isn't already being used
	if (memGetBlockHeap() != psMissionHeap)
	{
		DBP0(("levLoadData: setting mission heap\n"));
		BLOCK_RESET(psMissionHeap);
		memSetBlockHeap(psMissionHeap);
		psCurrHeap = psMissionHeap;
	}


	//if (pSaveName != NULL && saveType == GTYPE_SAVE_MIDMISSION)
    if (pSaveName != NULL)
	{
		//load MidMission Extras
		if (!loadMissionExtras(pSaveName, psNewLevel->type))
		{
			return FALSE;
		}
    }

    if (pSaveName != NULL && saveType == GTYPE_SAVE_MIDMISSION)
    {
		//load script stuff
		// load the event system state here for a save game
		DBP0(("levLoadData: loading script system state\n"));
		if (!loadScriptState(pSaveName))
		{
			return FALSE;
		}
	}

	if (!stageThreeInitialise())
	{
		return FALSE;
	}

//want to test with release build too
//#ifdef DEBUG
    //this enables us to to start cam2/cam3 without going via a save game and get the extra droids
    //in from the script-controlled Transporters
    if (!pSaveName AND psNewLevel->type == LDS_CAMSTART)
    {
        eventFireCallbackTrigger(CALL_NO_REINFORCEMENTS_LEFT);
    }
//#endif

    //restore the level name for comparisons on next mission load up
    if (psChangeLevel == NULL)
    {
        psCurrLevel = psNewLevel;
    }
    else
    {
        psCurrLevel = psChangeLevel;
    }


    return TRUE;
}
