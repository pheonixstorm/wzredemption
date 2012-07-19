/*
 * ScriptObj.c
 *
 * Object access functions for the script library
 *
 */

#include "Frame.h"
#include "Objects.h"

#include "Script.h"
#include "ScriptTabs.h"
#include "ScriptObj.h"
#include "Group.h"
#include "GTime.h"
#include "Cluster.h"
#include "MessageDef.h"
#include "Message.h"
#include "ResearchDef.h"
#include "Audio.h"
#include "Multiplay.h"
#include "Text.h"
#include "Levels.h"
#include "ScriptVals.h"
#include "Research.h"

// Get values from a base object
BOOL scrBaseObjGet(UDWORD index)
{
//	INTERP_VAL		sVal;
	INTERP_TYPE		type;
	BASE_OBJECT		*psObj;
	SDWORD			val;
	DROID			*psDroid;
	STRUCTURE		*psStruct;
	FEATURE			*psFeature;

	if (!stackPopParams(1, ST_BASEOBJECT, &psObj))
	{
		return FALSE;
	}

	// Check this is a valid pointer
	if (psObj == NULL )
	{
		ASSERT((FALSE, "scrBaseObjGet: was passed an invalid pointer"));
		return FALSE;
	}
	// Check this is a valid pointer
	if (psObj->type != OBJ_DROID && psObj->type != OBJ_STRUCTURE && psObj->type != OBJ_FEATURE) 
	{
		ASSERT((FALSE, "scrBaseObjGet: invalid object"));
		return FALSE;
	}

	// set the type and return value
	switch (index)
	{
	case OBJID_POSX:
		type = VAL_INT;
		val = (SDWORD)psObj->x;
		break;
	case OBJID_POSY:
		type = VAL_INT;
		val = (SDWORD)psObj->y;
		break;
	case OBJID_POSZ:
		type = VAL_INT;
		val = (SDWORD)psObj->z;
		break;
	case OBJID_ID:
		type = VAL_INT;
		val = (SDWORD)psObj->id;
		break;
	case OBJID_PLAYER:
		type = VAL_INT;
		val = (SDWORD)psObj->player;
		break;
	case OBJID_TYPE:
		type = VAL_INT;
		val = (SDWORD)psObj->type;
		break;
	case OBJID_ORDER:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: order only valid for a droid"));
			return FALSE;
		}
		type = VAL_INT;
		val = (SDWORD)((DROID *)psObj)->order;
		if ((val == DORDER_GUARD) && (((DROID *)psObj)->psTarget == NULL))
		{
			val = DORDER_NONE;
		}
		break;
	case OBJID_ORDERX:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: order only valid for a droid"));
			return FALSE;
		}
		type = VAL_INT;
		val = (SDWORD)((DROID *)psObj)->orderX;
		break;
	case OBJID_ORDERY:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: order only valid for a droid"));
			return FALSE;
		}
		type = VAL_INT;
		val = (SDWORD)((DROID *)psObj)->orderY;
		break;
	case OBJID_DROIDTYPE:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: droidType only valid for a droid"));
			return FALSE;
		}
		type = VAL_INT;
		val = (SDWORD)((DROID *)psObj)->droidType;
		break;
	case OBJID_CLUSTERID:
		if (psObj->type == OBJ_FEATURE)
		{
			ASSERT((FALSE,"scrBaseObjGet: clusterID not valid for features"));
			return FALSE;
		}
		type = VAL_INT;
		val = clustGetClusterID(psObj);
		break;
	case OBJID_HEALTH:
		switch (psObj->type)
		{
		case OBJ_DROID:
			psDroid = (DROID *)psObj;
			type = VAL_INT;
			val = psDroid->body * 100 / psDroid->originalBody;
			break;
		case OBJ_FEATURE:
			psFeature = (FEATURE *)psObj;
			type = VAL_INT;
			if (psFeature->psStats->damageable)
			{
				val = psFeature->body * 100 / psFeature->psStats->body;
			}
			else
			{
				val = 100;
			}
			break;
		case OBJ_STRUCTURE:
			psStruct = (STRUCTURE *)psObj;
			type = VAL_INT;
			//val = psStruct->body * 100 / psStruct->baseBodyPoints;
			val = psStruct->body * 100 / structureBody(psStruct);
			break;
		}
		break;
	case OBJID_BODY:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: body only valid for a droid"));
			return FALSE;
		}
		type = ST_BODY;
		val = ((DROID *)psObj)->asBits[COMP_BODY].nStat;
		break;
	case OBJID_PROPULSION:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: propulsion only valid for a droid"));
			return FALSE;
		}
		type = ST_PROPULSION;
		val = ((DROID *)psObj)->asBits[COMP_PROPULSION].nStat;
		break;
	case OBJID_WEAPON:
		if (psObj->type != OBJ_DROID)
		{
			ASSERT((FALSE,"scrBaseObjGet: weapon only valid for a droid"));
			return FALSE;
		}
		type = ST_WEAPON;
		//if (((DROID *)psObj)->numWeaps == 0)
        if (((DROID *)psObj)->asWeaps[0].nStat == 0)
		{
			val = 0;
		}
		else
		{
			val = ((DROID *)psObj)->asWeaps[0].nStat;
		}
		break;
	case OBJID_STRUCTSTAT:
		if (psObj->type != OBJ_STRUCTURE)
		{
			ASSERT((FALSE,"scrBaseObjGet: stat only valid for a structure"));
			return FALSE;
		}
		type = ST_STRUCTURESTAT;
		val = ((STRUCTURE *)psObj)->pStructureType - asStructureStats;
		break;
	default:
		ASSERT((FALSE, "scrBaseObjGet: unknown variable index"));
		return FALSE;
		break;
	}

	// Return the value
	if (!stackPushResult(type, val))
	{
		return FALSE;
	}

	return TRUE;
}


// Set values from a base object
BOOL scrBaseObjSet(UDWORD index)
{
	index = index;

	return TRUE;
}


// convert a base object to a droid if it is the right type
BOOL scrObjToDroid(void)
{
	BASE_OBJECT		*psObj;

	if (!stackPopParams(1, ST_BASEOBJECT, &psObj))
	{
		return FALSE;
	}

	// return NULL if not a droid
	if (psObj->type != OBJ_DROID)
	{
		psObj = NULL;
	}

	if (!stackPushResult(ST_DROID, (SDWORD)psObj))
	{
		return FALSE;
	}

	return TRUE;
}


// convert a base object to a structure if it is the right type
BOOL scrObjToStructure(void)
{
	BASE_OBJECT		*psObj;

	if (!stackPopParams(1, ST_BASEOBJECT, &psObj))
	{
		return FALSE;
	}

	// return NULL if not a droid
	if (psObj->type != OBJ_STRUCTURE)
	{
		psObj = NULL;
	}

	if (!stackPushResult(ST_STRUCTURE, (SDWORD)psObj))
	{
		return FALSE;
	}

	return TRUE;
}


// convert a base object to a feature if it is the right type
BOOL scrObjToFeature(void)
{
	BASE_OBJECT		*psObj;

	if (!stackPopParams(1, ST_BASEOBJECT, &psObj))
	{
		return FALSE;
	}

	// return NULL if not a droid
	if (psObj->type != OBJ_FEATURE)
	{
		psObj = NULL;
	}

	if (!stackPushResult(ST_FEATURE, (SDWORD)psObj))
	{
		return FALSE;
	}

	return TRUE;
}


// cache all the possible values for the last group to try
// to speed up access
static DROID_GROUP	*psScrLastGroup;
static SDWORD		lgX,lgY, lgMembers, lgHealth;
static UDWORD		lgGameTime;

// Get values from a group
BOOL scrGroupObjGet(UDWORD index)
{
	INTERP_TYPE		type;
	DROID_GROUP		*psGroup;
	SDWORD			val;
	DROID			*psCurr;

	if (!stackPopParams(1, ST_GROUP, &psGroup))
	{
		return FALSE;
	}

	// recalculate the values if necessary
	if (lgGameTime != gameTime || psScrLastGroup != psGroup)
	{
		lgGameTime = gameTime;
		psScrLastGroup = psGroup;
		lgMembers = 0;
		lgHealth = 0;
		lgX = lgY = 0;
		for(psCurr = psGroup->psList; psCurr; psCurr = psCurr->psGrpNext)
		{
			lgMembers += 1;
			lgX += (SDWORD)psCurr->x;
			lgY += (SDWORD)psCurr->y;
			lgHealth += (SDWORD)((100 * psCurr->body)/psCurr->originalBody);
		}
		if (lgMembers > 0)
		{
			lgX = lgX / lgMembers;
			lgY = lgY / lgMembers;
			lgHealth = lgHealth / lgMembers;
		}
	}

	// set the type and return value
	switch (index)
	{
	case GROUPID_POSX:
		type = VAL_INT;
		val = lgX;
		break;
	case GROUPID_POSY:
		type = VAL_INT;
		val = lgY;
		break;
	case GROUPID_MEMBERS:
		type = VAL_INT;
		val = lgMembers;
		break;
	case GROUPID_HEALTH:
		type = VAL_INT;
		val = lgHealth;
		break;
	default:
		ASSERT((FALSE, "scrGroupObjGet: unknown variable index"));
		return FALSE;
		break;
	}

	// Return the value
	if (!stackPushResult(type, val))
	{
		return FALSE;
	}

	return TRUE;
}

// get the name from a stat pointer
STRING	*scrGetStatName(INTERP_TYPE type, UDWORD data)
{
	STRING	*pName = NULL;

	switch (type)
	{
	case ST_STRUCTURESTAT:
		if (data < numStructureStats)
		{
			pName = asStructureStats[data].pName;
		}
		break;
	case ST_FEATURESTAT:
		if (data < numFeatureStats)
		{
			pName = asFeatureStats[data].pName;
		}
		break;
	case ST_BODY:
		if (data < numBodyStats)
		{
			pName = asBodyStats[data].pName;
		}
		break;
	case ST_PROPULSION:
		if (data < numPropulsionStats)
		{
			pName = asPropulsionStats[data].pName;
		}
		break;
	case ST_ECM:
		if (data < numECMStats)
		{
			pName = asECMStats[data].pName;
		}
		break;
	case ST_SENSOR:
		if (data < numSensorStats)
		{
			pName = asSensorStats[data].pName;
		}
		break;
	case ST_CONSTRUCT:
		if (data < numConstructStats)
		{
			pName = asConstructStats[data].pName;
		}
		break;
	case ST_WEAPON:
		if (data < numWeaponStats)
		{
			pName = asWeaponStats[data].pName;
		}
		break;
	case ST_REPAIR:
		if (data < numRepairStats)
		{
			pName = asRepairStats[data].pName;
		}
		break;
	case ST_BRAIN:
		if (data < numBrainStats)
		{
			pName = asBrainStats[data].pName;
		}
		break;
	case ST_BASESTATS:
	case ST_COMPONENT:
		// should never have variables of this type
		break;
	}

	if (pName == NULL)
	{
		DBERROR(("scrGetStatName: cannot get name for a base stat"));
	}

	return pName;
}

// default value save routine
BOOL scrValDefSave(INTERP_TYPE type, UDWORD data, UBYTE *pBuffer, UDWORD *pSize)
{
	VIEWDATA	*psIntMessage;
	STRING		*pName;
	RESEARCH	*psResearch;
	UBYTE		*pPos;
	DROID		*psCDroid;
	SDWORD		members;
	DROID_GROUP	*psGroup;
	BASE_OBJECT	*psObj;
	switch (type)
	{
	case ST_INTMESSAGE:
		// save the name
		psIntMessage = (VIEWDATA *)data;
		if (psIntMessage != NULL)
		{
			if (pBuffer)
			{
				strcpy((char *)pBuffer, psIntMessage->pName);
			}
			*pSize = strlen(psIntMessage->pName)+1;
		}
		else
		{
			if (pBuffer)
			{
				*((UBYTE *)pBuffer) = 0;
			}
			*pSize = 1;
		}
		break;
	case ST_BASEOBJECT:
	case ST_DROID:
	case ST_STRUCTURE:
	case ST_FEATURE:
		// just save the id
		if (pBuffer)
		{
			if (data == 0)
			{
				*((UDWORD*)pBuffer) = UDWORD_MAX;
			}
			else
			{
				*((UDWORD*)pBuffer) = ((BASE_OBJECT *)data)->id;
#ifdef _DEBUG
				psObj = getBaseObjFromId(((BASE_OBJECT *)data)->id);
				ASSERT((psObj == (BASE_OBJECT *)data,"scrValDefSave failed to find object, continue"));
#endif
			}
		}
		*pSize = sizeof(UDWORD);
		break;
	case ST_BASESTATS:
	case ST_COMPONENT:
	case ST_FEATURESTAT:
	case ST_STRUCTURESTAT:
	case ST_BODY:
	case ST_PROPULSION:
	case ST_ECM:
	case ST_SENSOR:
	case ST_CONSTRUCT:
	case ST_WEAPON:
	case ST_REPAIR:
	case ST_BRAIN:
		pName = scrGetStatName(type, data);
		if (pName != NULL)
		{
			if (pBuffer)
			{
				strcpy((char *)pBuffer, pName);
			}
			*pSize = strlen(pName) + 1;
		}
		else
		{
			return FALSE;
		}
		break;
	case ST_TEMPLATE:
		if (pBuffer)
		{
			if (data == 0)
			{
				*((UDWORD*)pBuffer) = UDWORD_MAX;
			}
			else
			{
				*((UDWORD*)pBuffer) = ((DROID_TEMPLATE *)data)->multiPlayerID;
			}
		}
		*pSize = sizeof(UDWORD);
		break;
	case ST_TEXTSTRING:
		if (pBuffer)
		{
			if (data == 0)
			{
				*((UDWORD*)pBuffer) = UDWORD_MAX;
			}
			else
			{
				*((UDWORD*)pBuffer) = strresGetIDfromString(psStringRes, (STRING *)data);
			}
		}
		*pSize = sizeof(UDWORD);
		break;
	case ST_LEVEL:
		if (data != 0)
		{
			if (pBuffer)
			{
				strcpy((char *)pBuffer, (STRING *)data);
			}
			*pSize = strlen((STRING *)data)+1;
		}
		else
		{
			if (pBuffer)
			{
				*((UBYTE *)pBuffer) = 0;
			}
			*pSize = 1;
		}
		break;
	case ST_RESEARCH:
		psResearch = (RESEARCH *)data;
		if (psResearch != NULL)
		{
			if (pBuffer)
			{
				strcpy((char *)pBuffer, psResearch->pName);
			}
			*pSize = strlen(psResearch->pName)+1;
		}
		else
		{
			if (pBuffer)
			{
				*((UBYTE *)pBuffer) = 0;
			}
			*pSize = 1;
		}
		break;
	case ST_GROUP:
		if (data != 0)
		{
			members = grpNumMembers((DROID_GROUP *)data);
		}
		else
		{
			members = 0;
		}
		if (pBuffer != NULL)
		{
			pPos = pBuffer;
			psGroup = (DROID_GROUP *)data;

			// store the run data
			*((SDWORD *)pPos) = psGroup->sRunData.sPos.x;
			pPos += sizeof(SDWORD);
			*((SDWORD *)pPos) = psGroup->sRunData.sPos.y;
			pPos += sizeof(SDWORD);
			*((SDWORD *)pPos) = psGroup->sRunData.forceLevel;
			pPos += sizeof(SDWORD);
			*((SDWORD *)pPos) = psGroup->sRunData.leadership;
			pPos += sizeof(SDWORD);
			*((SDWORD *)pPos) = psGroup->sRunData.healthLevel;
			pPos += sizeof(SDWORD);

			// now store the droids
			for(psCDroid=((DROID_GROUP *)data)->psList; psCDroid; psCDroid=psCDroid->psGrpNext)
			{
				checkValidId(psCDroid->id);
				*((UDWORD *)pPos) = psCDroid->id;
				pPos += sizeof(UDWORD);
			}
		}
		*pSize = sizeof(UDWORD) * members + sizeof(SDWORD) * 5;
		break;
	case ST_SOUND:
/*		pName = sound_GetTrackName((SDWORD)data);
		if (pName == NULL)
		{
			DBERROR(("scrValDefSave: couldn't get sound track name"));
			return FALSE;
		}
		if (pBuffer)
		{
			strcpy((char *)pBuffer, pName);
		}
		*pSize = strlen((char *)pName) + 1;*/
		if (pBuffer)
		{
			*((UDWORD *) pBuffer) = sound_GetTrackHashName((SDWORD)data);
		}
		*pSize = sizeof(UDWORD);
		break;
	case ST_STRUCTUREID:
	case ST_DROIDID:
	default:
		ASSERT(( (type == ST_STRUCTUREID) ||
				 (type == ST_DROIDID),
				"scrValDefSave: unknown script variable type for save"));
		// just save the variable contents directly
		if (pBuffer)
		{
			*((UDWORD *)pBuffer) = data;
		}
		*pSize = sizeof(UDWORD);
		break;
	}
	return TRUE;
}

// default value load routine
BOOL scrValDefLoad(SDWORD version, INTERP_TYPE type, UBYTE *pBuffer, UDWORD size, UDWORD *pData)
{
	UBYTE			*pPos;
	DROID			*psCDroid;
	SDWORD			index, members;
	UDWORD			id;
	LEVEL_DATASET	*psLevel;
	DROID_GROUP		*psGroup;

	switch (type)
	{
	case ST_INTMESSAGE:
		if ((size == 1) &&
			(*pBuffer == 0))
		{
			*pData = 0;
		}
		else
		{
			*pData = (UDWORD)getViewData((STRING *)pBuffer);
			if (*pData == 0)
			{
				return FALSE;
			}
		}
		break;
	case ST_BASEOBJECT:
	case ST_DROID:
	case ST_STRUCTURE:
	case ST_FEATURE:
		id = *((UDWORD *)pBuffer);
		if (id == UDWORD_MAX)
		{
			*pData = 0;
		}
		else
		{
			if (!scrvGetBaseObj(*((UDWORD*)pBuffer), (BASE_OBJECT **)pData))
			{
				DBERROR(("scrValDefLoad: couldn't find object id %d", *((UDWORD*)pBuffer)));
			}
		}
		break;
	case ST_BASESTATS:
	case ST_COMPONENT:
		break;
	case ST_STRUCTURESTAT:
		index = getStructStatFromName((char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find structure stat %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_FEATURESTAT:
		index = getFeatureStatFromName((char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find feature stat %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_BODY:
		index = getCompFromResName(COMP_BODY, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find body component %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_PROPULSION:
		index = getCompFromResName(COMP_PROPULSION, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find propulsion component %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_ECM:
		index = getCompFromResName(COMP_ECM, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find ECM component %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_SENSOR:
		index = getCompFromResName(COMP_SENSOR, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find sensor component %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_CONSTRUCT:
		index = getCompFromResName(COMP_CONSTRUCT, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find constructor component %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_WEAPON:
		index = getCompFromResName(COMP_WEAPON, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find weapon %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_REPAIR:
		index = getCompFromResName(COMP_REPAIRUNIT, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find repair component %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_BRAIN:
		index = getCompFromResName(COMP_BRAIN, (char *)pBuffer);
		if (index == -1)
		{
			DBERROR(("scrValDefLoad: couldn't find repair brain %s", pBuffer));
			index = 0;
		}
		*pData = (UDWORD)index;
		break;
	case ST_TEMPLATE:
		id = *((UDWORD *)pBuffer);
		if (id == UDWORD_MAX)
		{
			*((DROID_TEMPLATE **)pData) = NULL;
		}
		else
		{
			*((DROID_TEMPLATE **)pData) = IdToTemplate(id, ANYPLAYER);
			if (*pData == 0)
			{
				DBERROR(("scrValDefLoad: couldn't find template id %d", *((UDWORD *)pBuffer)));
			}
		}
		break;
	case ST_TEXTSTRING:
		if (*((UDWORD *)pBuffer) == UDWORD_MAX)
		{
			*pData = 0;
		}
		else
		{
			*pData = (UDWORD)(strresGetString(psStringRes, *((UDWORD *)pBuffer)));
		}
		break;
	case ST_LEVEL:
		if ((size == 1) &&
			(*pBuffer == 0))
		{
			*pData = 0;
		}
		else
		{
			if (!levFindDataSet((char *)pBuffer, &psLevel))
			{
				DBERROR(("scrValDefLoad: couldn't find level dataset %s", pBuffer));
			}
			*((STRING **)pData) = psLevel->pName;
		}
		break;
	case ST_RESEARCH:
		if ((size == 1) &&
			(*pBuffer == 0))
		{
			*pData = 0;
		}
		else
		{
			*pData = (UDWORD)getResearch((char *)pBuffer, TRUE);
			if (*pData == 0)
			{
				DBERROR(("scrValDefLoad: couldn't find research %s", pBuffer));
			}
		}
		break;
	case ST_GROUP:
		if (*pData == 0)
		{
			if (!grpCreate((DROID_GROUP **)pData))
			{
				DBERROR(("scrValDefLoad: out of memory"));
				break;
			}
			grpJoin(*((DROID_GROUP **)pData), NULL);
		}

		if (version == 1)
		{
			members = size / sizeof(UDWORD);
			pPos = pBuffer;
		}
		else if (version == 2)
		{
			members = (size - sizeof(SDWORD)*4) / sizeof(UDWORD);
			pPos = pBuffer;

			// load the retreat data
			psGroup = *((DROID_GROUP **)pData);
			psGroup->sRunData.sPos.x = *((SDWORD *)pPos);
			pPos += sizeof(SDWORD);
			psGroup->sRunData.sPos.y = *((SDWORD *)pPos);
			pPos += sizeof(SDWORD);
			psGroup->sRunData.forceLevel = (UBYTE)(*((SDWORD *)pPos));
			pPos += sizeof(SDWORD);
			psGroup->sRunData.leadership = (UBYTE)(*((SDWORD *)pPos));
			pPos += sizeof(SDWORD);
		}
		else 
		{
			members = (size - sizeof(SDWORD)*5) / sizeof(UDWORD);
			pPos = pBuffer;

			// load the retreat data
			psGroup = *((DROID_GROUP **)pData);
			psGroup->sRunData.sPos.x = *((SDWORD *)pPos);
			pPos += sizeof(SDWORD);
			psGroup->sRunData.sPos.y = *((SDWORD *)pPos);
			pPos += sizeof(SDWORD);
			psGroup->sRunData.forceLevel = (UBYTE)(*((SDWORD *)pPos));
			pPos += sizeof(SDWORD);
			psGroup->sRunData.leadership = (UBYTE)(*((SDWORD *)pPos));
			pPos += sizeof(SDWORD);
   			psGroup->sRunData.healthLevel = (UBYTE)(*((SDWORD *)pPos));
    		pPos += sizeof(SDWORD);
		}

		// load the droids
		while (members > 0)
		{
			if (!scrvGetBaseObj(*((UDWORD*)pPos), (BASE_OBJECT **)&psCDroid))
			{
				DBERROR(("scrValDefLoad: couldn't find object id %d", *((UDWORD*)pBuffer)));
			}
			else
			{
				grpJoin(*((DROID_GROUP **)pData), psCDroid);
			}

			pPos += sizeof(UDWORD);
			members -= 1;
		}
		break;
	case ST_SOUND:
		// find audio id
		index = audio_GetTrackIDFromHash( *((UDWORD *)pBuffer) );
		if (index == SAMPLE_NOT_FOUND)
		{
			// find empty id
			index = audio_GetAvailableID();
			if (index == SAMPLE_NOT_ALLOCATED)
			{
				DBERROR(("Sound ID not available %s not found", pBuffer));
				break;
			}
			// set track vals
			audio_SetTrackValsHashName( *((UDWORD *)pBuffer), FALSE, index, 100,
									1, 1800, 0 );
		}
		*pData = (UDWORD)index;
		break;
	case ST_STRUCTUREID:
	case ST_DROIDID:
	default:
		// just set the contents directly
		*pData = *((UDWORD *)pBuffer);
		break;
	}

	return TRUE;
}

