/*
 * ScriptFuncs.c
 *
 * All the C functions callable from the script code
 *
 */

#include <time.h>

#include "Frame.h"
#include "Widget.h"

#include "Script.h"
#include "ScriptTabs.h"
#include "GTime.h"
#include "Objects.h"
#include "HCI.h"
#include "Message.h"
#include "IntelMap.h"
#include "map.h"
#include "player.h"
#include "structure.h"
#include "display3d.h"
#include "research.h"
#include "audio.h"
#include "text.h"
#include "audio_id.h"
#include "power.h"
#include "console.h"
#include "ScriptFuncs.h"
#include "Geometry.h"
#include "Visibility.h"
#include "Gateway.h"
#include "drive.h"
#include "display.h"
#include "component.h"
#include "ScriptExtern.h"
#include "seqDisp.h"

#include "fpath.h"
#ifdef WIN32
#include "warzoneConfig.h"
#include "lighting.h"
#include "atmos.h"
#include "cdaudio.h"
#include "cdspan.h"
#include "netplay.h"
#include "multiplay.h"
#include "multigifts.h"
#include "multilimit.h"
#include "advvis.h"
#endif
#include "pieState.h"
#include "wrappers.h"
#include "Order.h"
#include "orderDef.h"
#include "Mission.h"
#include "Loop.h"
#include "FrontEnd.h"
#include "Group.h"
#include "Transporter.h"
#include "Radar.h"
#include "levels.h"
#include "mission.h"
#include "projectile.h"
#include "Cluster.h"

#ifdef PSX
#include "dcache.h"
#endif

//used in the set nogoArea and LandingZone functions - use the ones defined in Map.h
//#define MAX_MAP_WIDTH		192
//#define MAX_MAP_HEIGHT		128

// If this is defined then check max number of units not reached before adding more.
#define SCRIPT_CHECK_MAX_UNITS

// -----------------------------------------------------------------------------------------
BOOL	structHasModule(STRUCTURE *psStruct);

/******************************************************************************************/
/*                 Check for objects in areas                                             */

// check for a base object being in range of a point
BOOL objectInRange(BASE_OBJECT *psList, SDWORD x, SDWORD y, SDWORD range)
{
	BASE_OBJECT		*psCurr; 
	SDWORD			xdiff, ydiff, rangeSq;

	// See if there is a droid in range
	rangeSq = range * range;
	for(psCurr = psList; psCurr; psCurr = psCurr->psNext)
	{
		// skip partially build structures
		if ( (psCurr->type == OBJ_STRUCTURE) &&
			 (((STRUCTURE *)psCurr)->status != SS_BUILT) )
		{
			continue;
		}

		// skip flying vtols
		if ( (psCurr->type == OBJ_DROID) &&
			 vtolDroid((DROID *)psCurr) &&
			 ((DROID *)psCurr)->sMove.Status != MOVEINACTIVE )
		{
			continue;
		}

		xdiff = (SDWORD)psCurr->x - x;
		ydiff = (SDWORD)psCurr->y - y;
		if (xdiff*xdiff + ydiff*ydiff < rangeSq)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// -----------------------------------------------------------------------------------------
// Check for any player object being within a certain range of a position
BOOL scrObjectInRange(void)
{
	SDWORD		range, player, x,y;
	BOOL		found;

	if (!stackPopParams(4, VAL_INT, &player, VAL_INT, &x, VAL_INT, &y, VAL_INT, &range))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrObjectInRange: invalid player number"));
		return FALSE;
	}

	found = objectInRange((BASE_OBJECT *)apsDroidLists[player], x,y, range) ||
			objectInRange((BASE_OBJECT *)apsStructLists[player], x,y, range);

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Check for a droid being within a certain range of a position
BOOL scrDroidInRange(void)
{
	SDWORD		range, player, x,y;
	BOOL		found;

	if (!stackPopParams(4, VAL_INT, &player, VAL_INT, &x, VAL_INT, &y, VAL_INT, &range))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrUnitInRange: invalid player number"));
		return FALSE;
	}

	found = objectInRange((BASE_OBJECT *)apsDroidLists[player], x,y, range);

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Check for a struct being within a certain range of a position
BOOL scrStructInRange(void)
{
	SDWORD		range, player, x,y;
	BOOL		found;

	if (!stackPopParams(4, VAL_INT, &player, VAL_INT, &x, VAL_INT, &y, VAL_INT, &range))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrStructInRange: invalid player number"));
		return FALSE;
	}

	found = objectInRange((BASE_OBJECT *)apsStructLists[player], x,y, range);

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrPlayerPower(void)
{
	SDWORD player;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}
	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrPlayerPower: invalid player number"));
		return FALSE;
	}
	if (!stackPushResult(VAL_INT, asPower[player]->currentPower))
	{
		return FALSE;
	}
	return TRUE;
}


// -----------------------------------------------------------------------------------------
// check for a base object being in an area
static BOOL objectInArea(BASE_OBJECT *psList, SDWORD x1, SDWORD y1, SDWORD x2, SDWORD y2)
{
	BASE_OBJECT		*psCurr;
	SDWORD			ox,oy;

	// See if there is a droid in Area
	for(psCurr = psList; psCurr; psCurr = psCurr->psNext)
	{
		// skip partially build structures
		if ( (psCurr->type == OBJ_STRUCTURE) &&
			 (((STRUCTURE *)psCurr)->status != SS_BUILT) )
		{
			continue;
		}

		ox = (SDWORD)psCurr->x;
		oy = (SDWORD)psCurr->y;
		if (ox >= x1 && ox <= x2 &&
			oy >= y1 && oy <= y2)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// -----------------------------------------------------------------------------------------
// Check for any player object being within a certain area
BOOL scrObjectInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	BOOL		found;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrObjectInArea: invalid player number"));
		return FALSE;
	}

	found = objectInArea((BASE_OBJECT *)apsDroidLists[player], x1,y1, x2,y2) ||
			objectInArea((BASE_OBJECT *)apsStructLists[player], x1,y1, x2,y2);

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Check for a droid being within a certain area
BOOL scrDroidInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	BOOL		found;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrUnitInArea: invalid player number"));
		return FALSE;
	}

	found = objectInArea((BASE_OBJECT *)apsDroidLists[player], x1,y1, x2,y2);

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Check for a struct being within a certain Area of a position
BOOL scrStructInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	BOOL		found;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrStructInArea: invalid player number"));
		return FALSE;
	}

	found = objectInArea((BASE_OBJECT *)apsStructLists[player], x1,y1, x2,y2);

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
BOOL scrSeenStructInArea(void)
{
	BOOL		walls=FALSE,found = FALSE;
	SDWORD		player,enemy,x1,y1, x2,y2;
	STRUCTURE	*psCurr;
	SDWORD		ox,oy;

	// player, enemyplayer, walls, x1,r1,x2,y2
	if (!stackPopParams(7, VAL_INT, &player, VAL_INT, &enemy, VAL_BOOL,&walls,VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}


	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSeenStructInArea: invalid player number"));
		return FALSE;
	}

	for(psCurr = apsStructLists[enemy]; psCurr; psCurr = psCurr->psNext)
	{
		// skip partially build structures
		if ( (psCurr->type == OBJ_STRUCTURE) && (((STRUCTURE *)psCurr)->status != SS_BUILT) )
		{
			continue;
		}

		// possible skip walls
		if(walls && (psCurr->pStructureType->type != REF_WALL  || psCurr->pStructureType->type !=REF_WALLCORNER))
		{
			continue;
		}

		ox = (SDWORD)psCurr->x;
		oy = (SDWORD)psCurr->y;
		if (ox >= x1 && ox <= x2 &&	oy >= y1 && oy <= y2)
		{
			// structure is in area.
			if(psCurr->visible[player])
			{
				found = TRUE;
			}
		}
	}

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Check for a players structures but no walls being within a certain area
BOOL scrStructButNoWallsInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	SDWORD		ox,oy;
	STRUCTURE	*psStruct;
	SDWORD		found = FALSE;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrStructButNoWallsInArea: invalid player number"));
		return FALSE;
	}

	for(psStruct = apsStructLists[player]; psStruct; psStruct = psStruct->psNext)
	{
		if ((psStruct->pStructureType->type != REF_WALL) &&
			(psStruct->pStructureType->type != REF_WALLCORNER) &&
			(psStruct->status == SS_BUILT) )
		{
			ox = (SDWORD)psStruct->x;
			oy = (SDWORD)psStruct->y;
			if ((ox >= x1) && (ox <= x2) &&
				(oy >= y1) && (oy <= y2))
			{
				found = TRUE;
				break;
			}
		}
	}

	if (!stackPushResult(VAL_BOOL, found))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// check for the number of base objects in an area
static SDWORD numObjectsInArea(BASE_OBJECT *psList, SDWORD x1, SDWORD y1, SDWORD x2, SDWORD y2)
{
	BASE_OBJECT		*psCurr;
	SDWORD			ox,oy;
	SDWORD			count;

	// See if there is a droid in Area
	count = 0;
	for(psCurr = psList; psCurr; psCurr = psCurr->psNext)
	{
		// skip partially build structures
		if ( (psCurr->type == OBJ_STRUCTURE) &&
			 (((STRUCTURE *)psCurr)->status != SS_BUILT) )
		{
			continue;
		}

		ox = (SDWORD)psCurr->x;
		oy = (SDWORD)psCurr->y;
		if (ox >= x1 && ox <= x2 &&
			oy >= y1 && oy <= y2)
		{
			count += 1;
		}
	}

	return count;
}

// -----------------------------------------------------------------------------------------
// Count the number of player objects within a certain area
BOOL scrNumObjectsInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	SDWORD		count;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrNumObjectsInArea: invalid player number"));
		return FALSE;
	}

	count = numObjectsInArea((BASE_OBJECT *)apsDroidLists[player], x1,y1, x2,y2) +
			numObjectsInArea((BASE_OBJECT *)apsStructLists[player], x1,y1, x2,y2);

	if (!stackPushResult(VAL_INT, count))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Count the number of player droids within a certain area
BOOL scrNumDroidsInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	SDWORD		count;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrNumUnitInArea: invalid player number"));
		return FALSE;
	}

	count = numObjectsInArea((BASE_OBJECT *)apsDroidLists[player], x1,y1, x2,y2);

	if (!stackPushResult(VAL_INT, count))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Count the number of player structures within a certain area
BOOL scrNumStructsInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	SDWORD		count;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrNumStructsInArea: invalid player number"));
		return FALSE;
	}

	count = numObjectsInArea((BASE_OBJECT *)apsStructLists[player], x1,y1, x2,y2);

	if (!stackPushResult(VAL_INT, count))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Count the number of player structures but not walls within a certain area
BOOL scrNumStructsButNotWallsInArea(void)
{
	SDWORD		player, x1,y1, x2,y2;
	SDWORD		count, ox,oy;
	STRUCTURE	*psStruct;

	if (!stackPopParams(5, VAL_INT, &player, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrNumStructsButNotWallsInArea: invalid player number"));
		return FALSE;
	}

	count = 0;
	for(psStruct = apsStructLists[player]; psStruct; psStruct = psStruct->psNext)
	{
		if ((psStruct->pStructureType->type != REF_WALL) &&
			(psStruct->pStructureType->type != REF_WALLCORNER) &&
			(psStruct->status == SS_BUILT))
		{
			ox = (SDWORD)psStruct->x;
			oy = (SDWORD)psStruct->y;
			if ((ox >= x1) && (ox <= x2) &&
				(oy >= y1) && (oy <= y2))
			{
				count += 1;
			}
		}
	}

	if (!stackPushResult(VAL_INT, count))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Count the number of structures in an area of a certain type
BOOL scrNumStructsByTypeInArea(void)
{
	SDWORD		player, type, x1,y1, x2,y2;
	SDWORD		count, ox,oy;
	STRUCTURE	*psStruct;

	if (!stackPopParams(6, VAL_INT, &player, VAL_INT, &type,
					VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrNumStructsByTypeInArea: invalid player number"));
		return FALSE;
	}

	count = 0;
	for(psStruct = apsStructLists[player]; psStruct; psStruct = psStruct->psNext)
	{
		if ((psStruct->pStructureType->type == (UDWORD)type) &&
			(psStruct->status == SS_BUILT))
		{
			ox = (SDWORD)psStruct->x;
			oy = (SDWORD)psStruct->y;
			if ((ox >= x1) && (ox <= x2) &&
				(oy >= y1) && (oy <= y2))
			{
				count += 1;
			}
		}
	}

	if (!stackPushResult(VAL_INT, count))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Check for a droid having seen a certain object
BOOL scrDroidHasSeen(void)
{
	SDWORD		player;
	BASE_OBJECT	*psObj;
	BOOL		seen;

	if (!stackPopParams(2, ST_BASEOBJECT, &psObj, VAL_INT, &player))
	{
		return FALSE;
	}

	if (psObj == NULL)
	{
		ASSERT((FALSE, "scrUnitHasSeen: NULL object"));
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrUnitHasSeen:player number is too high"));
		return FALSE;
	}

	// See if any droid has seen this object
	seen = FALSE;
	if (psObj->visible[player])
	{
		seen = TRUE;
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)seen))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Check for a droid being within range of a position
BOOL scrDroidInRangeOfPosition(void)
{
	SDWORD		range, player;
	DROID		*psCurr;
	SDWORD		rangeSquared;
	SDWORD		dx, dy, dz, iX, iY, iZ;
	BOOL		found;

	if (!stackPopParams(5, VAL_INT, &range, VAL_INT, &player,
						VAL_INT, &iX, VAL_INT, &iY, VAL_INT, &iZ ))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrUnitInRangeOfPosition: invalid player number"));
		return FALSE;
	}

	// See if there is a droid in range
	rangeSquared = range * range;
	found = FALSE;
	for(psCurr = apsDroidLists[player]; psCurr; psCurr = psCurr->psNext)
	{
		dx = (SDWORD)psCurr->x - iX;
		dy = (SDWORD)psCurr->y - iY;
		dz = (SDWORD)psCurr->z - iZ;

		if ( (dx*dx + dy*dy + dz*dz) < rangeSquared)
		{
			found = TRUE;
			break;
		}
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)found))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Enable a component to be researched
BOOL scrEnableComponent(void)
{
	SDWORD		player;
	INTERP_VAL	sVal;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}
	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrEnableComponent:player number is too high"));
		return FALSE;
	}

	// enable the appropriate component
	switch (sVal.type)
	{
	case ST_BODY:
		apCompLists[player][COMP_BODY][sVal.v.ival] = FOUND;
		break;
	case ST_PROPULSION:
		apCompLists[player][COMP_PROPULSION][sVal.v.ival] = FOUND;
		break;
	case ST_ECM:
		apCompLists[player][COMP_ECM][sVal.v.ival] = FOUND;
		break;
	case ST_SENSOR:
		apCompLists[player][COMP_SENSOR][sVal.v.ival] = FOUND;
		break;
	case ST_CONSTRUCT:
		apCompLists[player][COMP_CONSTRUCT][sVal.v.ival] = FOUND;
		break;
	case ST_WEAPON:
		apCompLists[player][COMP_WEAPON][sVal.v.ival] = FOUND;
		break;
	case ST_REPAIR:
		apCompLists[player][COMP_REPAIRUNIT][sVal.v.ival] = FOUND;
		break;
	case ST_BRAIN:
		apCompLists[player][COMP_BRAIN][sVal.v.ival] = FOUND;
		break;
	default:
		ASSERT((FALSE, "scrEnableComponent: unknown type"));
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Make a component available
BOOL scrMakeComponentAvailable(void)
{
	SDWORD		player;
	INTERP_VAL	sVal;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}
	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrMakeComponentAvailable:player number is too high"));
		return FALSE;
	}

	// make the appropriate component available
	switch (sVal.type)
	{
	case ST_BODY:
		apCompLists[player][COMP_BODY][sVal.v.ival] = AVAILABLE;
		break;
	case ST_PROPULSION:
		apCompLists[player][COMP_PROPULSION][sVal.v.ival] = AVAILABLE;
		break;
	case ST_ECM:
		apCompLists[player][COMP_ECM][sVal.v.ival] = AVAILABLE;
		break;
	case ST_SENSOR:
		apCompLists[player][COMP_SENSOR][sVal.v.ival] = AVAILABLE;
		break;
	case ST_CONSTRUCT:
		apCompLists[player][COMP_CONSTRUCT][sVal.v.ival] = AVAILABLE;
		break;
	case ST_WEAPON:
		apCompLists[player][COMP_WEAPON][sVal.v.ival] = AVAILABLE;
		break;
	case ST_REPAIR:
		apCompLists[player][COMP_REPAIRUNIT][sVal.v.ival] = AVAILABLE;
		break;
	case ST_BRAIN:
		apCompLists[player][COMP_BRAIN][sVal.v.ival] = AVAILABLE;
		break;
	default:
		ASSERT((FALSE, "scrEnableComponent: unknown type"));
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Add a droid
BOOL scrAddDroidToMissionList(void)
{
	SDWORD			player;
	DROID_TEMPLATE	*psTemplate;
	DROID			*psDroid;

	if (!stackPopParams(2, ST_TEMPLATE, &psTemplate, VAL_INT, &player))
	{
		return FALSE;
	}

/*	if ((UBYTE)player == selectedPlayer )
	{
		ASSERT( (FALSE, "scrAddDroidToMissionList: can't add own player to list") );
		return FALSE;
	}*/

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddUnitToMissionList:player number is too high"));
		return FALSE;
	}

	ASSERT((PTRVALID(psTemplate, sizeof(DROID_TEMPLATE)),
		"scrAddUnitToMissionList: Invalid template pointer"));

#ifdef SCRIPT_CHECK_MAX_UNITS
	// Don't build a new droid if player limit reached, unless it's a transporter.
	if( IsPlayerDroidLimitReached(player) && (psTemplate->droidType != DROID_TRANSPORTER) ) {
		DBPRINTF(("scrAddUnit : Max units reached ,player %d\n",player));
		psDroid = NULL;
	} else
#endif
	{
		psDroid = buildMissionDroid( psTemplate, 128, 128, player );
	}

	if (!stackPushResult(ST_DROID, (SDWORD)psDroid))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Add a droid
BOOL scrAddDroid(void)
{
	SDWORD			x, y, player;
//	INTERP_VAL		sVal;
	DROID_TEMPLATE	*psTemplate;
	DROID			*psDroid;

	if (!stackPopParams(4, ST_TEMPLATE, &psTemplate, VAL_INT, &x, VAL_INT, &y, VAL_INT, &player))
	{
		return FALSE;
	}
/*	if (!stackPop(&sVal))
	{
		return FALSE;
	}
	if (sVal.type != ST_TEMPLATE)
	{
		ASSERT((FALSE, "scrAddDroid: type mismatch for object"));
		return FALSE;
	}
	psTemplate = (DROID_TEMPLATE *)sVal.v.ival;
*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddUnit:player number is too high"));
		return FALSE;
	}

	ASSERT((PTRVALID(psTemplate, sizeof(DROID_TEMPLATE)),
		"scrAddUnit: Invalid template pointer"));

#ifdef SCRIPT_CHECK_MAX_UNITS
	// Don't build a new droid if player limit reached, unless it's a transporter.
	if( IsPlayerDroidLimitReached(player) && (psTemplate->droidType != DROID_TRANSPORTER) ) {
		DBPRINTF(("scrAddUnit : Max units reached ,player %d\n",player));
		psDroid = NULL;
	} else
#endif
	{
		psDroid = buildDroid(psTemplate, x, y, player, FALSE);
		if (psDroid)
		{
			addDroid(psDroid, apsDroidLists);
			if (vtolDroid(psDroid))
			{
				// vtols start in the air
				moveMakeVtolHover(psDroid);
			}
		}
	}

	if (!stackPushResult(ST_DROID, (SDWORD)psDroid))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Add droid to transporter

BOOL scrAddDroidToTransporter(void)
{
	DROID	*psTransporter, *psDroid;

	if (!stackPopParams(2, ST_DROID, &psTransporter, ST_DROID, &psDroid))
	{
		return FALSE;
	}
	
    if (psTransporter == NULL OR psDroid == NULL)
    {
        //ignore!
        ASSERT((FALSE, "scrAddUnitToTransporter: null unit passed"));
        return TRUE;
    }

	ASSERT( (PTRVALID(psTransporter, sizeof(DROID)),
			"scrAddUnitToTransporter: invalid transporter pointer") );
	ASSERT( (PTRVALID(psDroid, sizeof(DROID)),
			"scrAddUnitToTransporter: invalid unit pointer") );
	ASSERT( (psTransporter->droidType == DROID_TRANSPORTER,
			"scrAddUnitToTransporter: invalid transporter type") );

	/* check for space */
	if (checkTransporterSpace(psTransporter, psDroid))
	{
		if (droidRemove(psDroid, mission.apsDroidLists))
        {
		    grpJoin(psTransporter->psGroup, psDroid);
        }
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//check for a building to have been destroyed
BOOL scrBuildingDestroyed(void)
{
	SDWORD		player;
	UDWORD		structureID;
//	INTERP_VAL	sVal;
	BOOL		destroyed;
	STRUCTURE	*psCurr;

	if (!stackPopParams(2, ST_STRUCTUREID, &structureID, VAL_INT, &player))
	{
		return FALSE;
	}
/*	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (sVal.type != ST_STRUCTUREID)
	{
		ASSERT((FALSE, "scrBuildingDestroyed: type mismatch for object"));
		return FALSE;
	}
	structureID = (UDWORD)sVal.v.ival;
*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrBuildingDestroyed:player number is too high"));
		return FALSE;
	}

	destroyed = TRUE;
	//look thru the players list to see if the structure still exists
	for (psCurr = apsStructLists[player]; psCurr != NULL; psCurr = psCurr->psNext)
	{
		if (psCurr->id == structureID)
		{
			destroyed = FALSE;
		}
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)destroyed))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Enable a structure to be built
BOOL scrEnableStructure(void)
{
	SDWORD		player, index;
//	INTERP_VAL	sVal;

	if (!stackPopParams(2, ST_STRUCTURESTAT, &index, VAL_INT, &player))
	{
		return FALSE;
	}
/*	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (sVal.type != ST_STRUCTURESTAT)
	{
		ASSERT((FALSE, "scrEnableStructure: type mismatch for object"));
		return FALSE;
	}*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrEnableStructure:player number is too high"));
		return FALSE;
	}

	if (index < (SDWORD)0 OR index > (SDWORD)numStructureStats)
	{
		ASSERT((FALSE, "scrEnableStructure:invalid structure stat"));
		return FALSE;
	}

	// enable the appropriate structure
	apStructTypeLists[player][index] = AVAILABLE;

	return TRUE;
}


#ifdef WIN32
// -----------------------------------------------------------------------------------------
// Check if a structure can be built.
// currently PC skirmish only.
BOOL scrIsStructureAvailable(void)
{
	SDWORD		player, index;
	BOOL		result;

	if (!stackPopParams(2, ST_STRUCTURESTAT, &index, VAL_INT, &player))
	{
		return FALSE;
	}
	if(	apStructTypeLists[player][index] == AVAILABLE)
	{
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}

	if (!stackPushResult(VAL_BOOL, result))
	{
		return FALSE;
	}
	return TRUE;
}
#endif

// -----------------------------------------------------------------------------------------
//make the droid with the matching id the currently selected droid
BOOL scrSelectDroidByID(void)
{
	SDWORD			player, droidID;
//	INTERP_VAL		sVal;
	BOOL			selected;

	if (!stackPopParams(2, ST_DROIDID, &droidID, VAL_INT, &player))
	{
		return FALSE;
	}
/*	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (sVal.type != ST_DROIDID)
	{
		ASSERT((FALSE, "scrSelectDroidByID: type mismatch for object"));
		return FALSE;
	}
*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSelectUnitByID:player number is too high"));
		return FALSE;
	}

	selected = FALSE;
	if (selectDroidByID(droidID, player))
	{
		selected = TRUE;
	}

	//store the reult cos might need to check the droid exists before doing anything else
	if (!stackPushResult(VAL_BOOL, (UDWORD)selected))
	{
		return FALSE;
	}
	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Pop up a message box with a number value in it
BOOL scrNumMB(void)
{
	SDWORD	val;

	if (!stackPopParams(1, VAL_INT, &val))
	{
		return FALSE;
	}

/*	gameTimeStop();
	DBERROR(("scrNumMB: called by script with value: %d", val));
	gameTimeStart();*/
	DBPRINTF(("scrNumMB: called by script with value: %d\n", val));

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Do an approximation to a square root
BOOL scrApproxRoot(void)
{
	SDWORD	val1, val2;

	if (!stackPopParams(2, VAL_INT, &val1, VAL_INT, &val2))
	{
		return FALSE;
	}

	if (val1 < val2)
	{
		val1 = val2 + (val1 >> 1);
	}
	else
	{
		val1 = val1 + (val2 >> 1);
	}

	if (!stackPushResult(VAL_INT, val1))
	{
		return FALSE;
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------
extern void intShowReticuleButton(UDWORD id,BOOL Show);
// Add a reticule button to the interface
BOOL scrAddReticuleButton(void)
{
	SDWORD	val;


	if (!stackPopParams(1, VAL_INT, &val))
	{
		return FALSE;
	}

#ifdef NON_INTERACT
	return(TRUE);
#endif

#ifdef WIN32
	//set the appropriate flag to 'draw' the button
	switch (val)
	{
	case IDRET_OPTIONS:
		// bit of a hack here to keep compatibility with old scripts
		widgReveal(psWScreen, IDRET_COMMAND);
		break;
	case IDRET_COMMAND:
		widgReveal(psWScreen, IDRET_COMMAND);
		break;
	case IDRET_BUILD:
		widgReveal(psWScreen, IDRET_BUILD);
		break;
	case IDRET_MANUFACTURE:
		widgReveal(psWScreen, IDRET_MANUFACTURE);
		break;
	case IDRET_RESEARCH:
		widgReveal(psWScreen, IDRET_RESEARCH);
		break;
	case IDRET_INTEL_MAP:
		widgReveal(psWScreen, IDRET_INTEL_MAP);
		break;
	case IDRET_DESIGN:
		widgReveal(psWScreen, IDRET_DESIGN);
		break;
	case IDRET_CANCEL:
		widgReveal(psWScreen, IDRET_CANCEL);
		break;
	default:
		ASSERT((FALSE, "scrAddReticuleButton: Invalid reticule Button ID"));
		return FALSE;
	}
#else
	intShowReticuleButton(val,TRUE);
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//Remove a reticule button from the interface
BOOL scrRemoveReticuleButton(void)
{
	SDWORD	val;
#ifdef WIN32
	BOOL	bReset;
#endif

#ifdef WIN32
	if (!stackPopParams(2, VAL_INT, &val,VAL_BOOL, &bReset))
	{
		return FALSE;
	}
#else
	// SCARY HUH?!
	if (!stackPopParams(1, VAL_INT, &val))
	{
		return FALSE;
	}
#endif
#ifdef WIN32
	if(bInTutorial)
	{
		if(bReset)	// not always desirable
		{
			intResetScreen(TRUE);
		}
	}
	switch (val)
	{
	case IDRET_OPTIONS:
		// bit of a hack here to keep compatibility with old scripts
		widgHide(psWScreen, IDRET_COMMAND);
		break;
	case IDRET_COMMAND:
		widgHide(psWScreen, IDRET_COMMAND);
		break;
	case IDRET_BUILD:
		widgHide(psWScreen, IDRET_BUILD);
		break;
	case IDRET_MANUFACTURE:
		widgHide(psWScreen, IDRET_MANUFACTURE);
		break;
	case IDRET_RESEARCH:
		widgHide(psWScreen, IDRET_RESEARCH);
		break;
	case IDRET_INTEL_MAP:
		widgHide(psWScreen, IDRET_INTEL_MAP);
		break;
	case IDRET_DESIGN:
		widgHide(psWScreen, IDRET_DESIGN);
		break;
	case IDRET_CANCEL:
		widgHide(psWScreen, IDRET_CANCEL);
		break;
	default:
		ASSERT((FALSE, "scrAddReticuleButton: Invalid reticule Button ID"));
		return FALSE;
	}
#else
		intShowReticuleButton(val,FALSE);

//		widgHide(psWScreen, val);
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// add a message to the Intelligence Display
BOOL scrAddMessage(void)
{
	MESSAGE			*psMessage;
	SDWORD			msgType, player;
	BOOL			playImmediate;
//	INTERP_VAL		sVal;
	VIEWDATA		*psViewData;
	UDWORD			height;


	if (!stackPopParams(4, ST_INTMESSAGE, &psViewData , VAL_INT, &msgType,
				VAL_INT, &player, VAL_BOOL, &playImmediate))
	{
		return FALSE;
	}
/*
	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (sVal.type != ST_INTMESSAGE)
	{
		ASSERT((FALSE, "scrAddMessage: type mismatch for object"));
		return FALSE;
	}
*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddMessage:player number is too high"));
		return FALSE;
	}

	//create the message
	psMessage = addMessage(msgType, FALSE, player);
	if (psMessage)
	{
		//set the data
		psMessage->pViewData = (MSG_VIEWDATA *)psViewData;
		if (msgType == MSG_PROXIMITY)
		{
			//check the z value is at least the height of the terrain
			height = map_Height(((VIEW_PROXIMITY *)psViewData->pData)->x, 
				((VIEW_PROXIMITY *)psViewData->pData)->y);
			if (((VIEW_PROXIMITY *)psViewData->pData)->z < height)
			{
				((VIEW_PROXIMITY *)psViewData->pData)->z = height;
			}
		}

		if (playImmediate)
		{
	//		psCurrentMsg = psMessage;
			//initTextDisplay(psCurrentMsg, WFont, 255);
			//addIntelScreen(TRUE);
	//		addIntelScreen();
			displayImmediateMessage(psMessage);
			stopReticuleButtonFlash(IDRET_INTEL_MAP);
		}
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// remove a message from the Intelligence Display
BOOL scrRemoveMessage(void)
{
	MESSAGE			*psMessage;
	SDWORD			msgType, player;
	VIEWDATA		*psViewData;


	if (!stackPopParams(3, ST_INTMESSAGE, &psViewData , VAL_INT, &msgType,
				VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddMessage:player number is too high"));
		return FALSE;
	}

	//find the message
	psMessage = findMessage((MSG_VIEWDATA *)psViewData, msgType, player);
	if (psMessage)
	{
		//delete it
		removeMessage(psMessage, player);
	}
	else
	{
		ASSERT((FALSE, "scrRemoveMessage:cannot find message - %s", 
			psViewData->pName));
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// add a tutorial message to the Intelligence Display
/*BOOL scrAddTutorialMessage(void)
{
	SDWORD			player;
	VIEWDATA		*psViewData;


	if (!stackPopParams(2, ST_INTMESSAGE, &psViewData , VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddTutorialMessage:player number is too high"));
		return FALSE;
	}

	//set the data
	tutorialMessage.pViewData = psViewData;
	tutorialMessage.player = player;

	//play the tutorial message immediately
	psCurrentMsg = &tutorialMessage;
	initTextDisplay(psCurrentMsg, WFont, 255);
	addIntelScreen(TRUE);

	return TRUE;
}*/

// -----------------------------------------------------------------------------------------
/*builds a droid in the specified factory*/
BOOL scrBuildDroid(void)
{
	SDWORD			player, productionRun;
//	INTERP_VAL		sVal, sVal2;
	STRUCTURE		*psFactory;
	DROID_TEMPLATE	*psTemplate;

	if (!stackPopParams(4, ST_TEMPLATE, &psTemplate, ST_STRUCTURE, &psFactory,
					VAL_INT, &player, VAL_INT, &productionRun))
	{
		return FALSE;
	}

	if (psFactory == NULL)
	{
		ASSERT((FALSE, "scrBuildUnit: NULL factory object"));
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrBuildUnit:player number is too high"));
		return FALSE;
	}

	if (productionRun > UBYTE_MAX)
	{
		ASSERT((FALSE, "scrBuildUnit: production run too high"));
		return FALSE;
	}

	ASSERT((PTRVALID(psFactory, sizeof(STRUCTURE)),
		"scrBuildUnit: Invalid structure pointer"));
	ASSERT(((psFactory->pStructureType->type == REF_FACTORY OR
		psFactory->pStructureType->type == REF_CYBORG_FACTORY OR
		psFactory->pStructureType->type == REF_VTOL_FACTORY), 
		"scrBuildUnit: structure is not a factory"));
	ASSERT((PTRVALID(psTemplate, sizeof(DROID_TEMPLATE)),
		"scrBuildUnit: Invalid template pointer"));

	//check building the right sort of droid for the factory
	if (!validTemplateForFactory(psTemplate, psFactory))
	{
#ifdef WIN32
		ASSERT((FALSE, "scrBuildUnit: invalid template - %s for factory - %s",
			&psTemplate->aName, psFactory->pStructureType->pName));
#else
		ASSERT((FALSE, "scrBuildUnit: invalid template - for factory"));
#endif
		return FALSE;
	}
		
	structSetManufacture(psFactory, psTemplate, (UBYTE)productionRun);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// for a specified structure, set the assembly point droids go to when built
BOOL	scrSetAssemblyPoint(void)
{
	SDWORD		x, y;
	STRUCTURE	*psBuilding;

	if (!stackPopParams(3, ST_STRUCTURE, &psBuilding, VAL_INT, &x, VAL_INT, &y))
	{
		return FALSE;
	}

	if (psBuilding == NULL)
	{
		ASSERT((FALSE, "scrSetAssemblyPoint: NULL structure"));
		return FALSE;
	}

	if (psBuilding->pStructureType->type != REF_FACTORY AND
		psBuilding->pStructureType->type != REF_CYBORG_FACTORY AND
		psBuilding->pStructureType->type != REF_VTOL_FACTORY)
	{
		ASSERT((FALSE, "scrSetAssemblyPoint: structure is not a factory"));
		return FALSE;
	}

	setAssemblyPoint(((FACTORY *)psBuilding->pFunctionality)->psAssemblyPoint,x,y,
        psBuilding->player, TRUE);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// test for structure is idle or not
BOOL	scrStructureIdle(void)
{
//	INTERP_VAL	sVal;
	STRUCTURE	*psBuilding;
	BOOL		idle;

	if (!stackPopParams(1, ST_STRUCTURE, &psBuilding))
	{
		return FALSE;
	}
//	DBPRINTF(("scrStructureIdle called\n"));

	if (psBuilding == NULL)
	{
		ASSERT((FALSE, "scrStructureIdle: NULL structure"));
		return FALSE;
	}

	//test for idle
	idle = FALSE;
	if (structureIdle(psBuilding))
	{
		idle = TRUE;
	}


//	DBPRINTF(("structure %p is %d\n",psBuilding,idle));

	if (!stackPushResult(VAL_BOOL, (UDWORD)idle))
	{
		return FALSE;
	}
	
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// sends a players droids to a location to attack
BOOL	scrAttackLocation(void)
{
	SDWORD		player, x, y;

	if (!stackPopParams(3, VAL_INT, &x, VAL_INT, &y, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAttackLocation:player number is too high"));
		return FALSE;
	}

	attackLocation(x, y, player);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//Destroy a feature
BOOL scrDestroyFeature(void)
{
	FEATURE		*psFeature;
//	INTERP_VAL	sVal;

	if (!stackPopParams(1, ST_FEATURE, &psFeature))
	{
		return FALSE;
	}

	if (psFeature == NULL)
	{
		ASSERT((PTRVALID(psFeature, sizeof(FEATURE)),
			"scrDestroyFeature: Invalid feature pointer"));
	}

	removeFeature(psFeature);

	return TRUE;
}




// -----------------------------------------------------------------------------------------
// static vars to enum features.
static	FEATURE_STATS	*psFeatureStatToFind[MAX_PLAYERS];
static	SDWORD			playerToEnum[MAX_PLAYERS];
static  SDWORD			getFeatureCount[MAX_PLAYERS]={0};
//static	FEATURE			*psCurrEnumFeature[MAX_PLAYERS];

// -----------------------------------------------------------------------------------------
// init enum visible features.
BOOL scrInitGetFeature(void)
{
	SDWORD			player,iFeat,bucket;
	
	if ( !stackPopParams(3, ST_FEATURESTAT, &iFeat,  VAL_INT, &player,VAL_INT,&bucket) )
	{
		return FALSE;
	}

	psFeatureStatToFind[bucket]		= (FEATURE_STATS *)(asFeatureStats + iFeat);				// find this stat
	playerToEnum[bucket]			= player;				// that this player can see
//	psCurrEnumFeature[bucket]		= apsFeatureLists[0];			
	getFeatureCount[bucket]			= 0;					// start at the beginning of list.
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// get next visible feature of required type
// notes:	can't rely on just using the feature list, since it may change 
//			between calls, Use an index into list instead.
//			Doesn't return Features sharing a tile with a structure.
//			Skirmish Only, dunno if kev uses this?
BOOL scrGetFeature(void)
{
	SDWORD	bucket,count;
	FEATURE	*psFeat;

	if ( !stackPopParams(1,VAL_INT,&bucket) )
	{
		return FALSE;
	}

	count =0;
	// go to the correct start point in the feature list.
	for(psFeat=apsFeatureLists[0];psFeat && count<getFeatureCount[bucket] ;count++)
	{
		psFeat = psFeat->psNext;
	}
	
	if(psFeat == NULL)		// no more to find.
	{
		if (!stackPushResult(ST_FEATURE, (SDWORD)NULL))
		{
			return FALSE;
		}
		return TRUE;
	}

	// check to see if badly called 
	if(psFeatureStatToFind[bucket] == NULL)
	{
		DBPRINTF(("invalid feature to find. possibly due to save game\n"));
		if(!stackPushResult(ST_FEATURE,(SDWORD)NULL))
		{
			return FALSE;
		}
		return TRUE;
	}

	// begin searching the feature list for the required stat.
	while(psFeat)
	{
		if(	( psFeat->psStats->subType == psFeatureStatToFind[bucket]->subType)
			&&( psFeat->visible[playerToEnum[bucket]]	!= 0)
			&&!TILE_HAS_STRUCTURE(mapTile(psFeat->x>>TILE_SHIFT,psFeat->y>>TILE_SHIFT) )
			&&!fireOnLocation(psFeat->x,psFeat->y)		// not burning.
			)
		{
			if (!stackPushResult(ST_FEATURE,(SDWORD)psFeat))	//	push result
			{
				return FALSE;
			}

			getFeatureCount[bucket]++;
			return TRUE;
		}
		getFeatureCount[bucket]++;
		psFeat=psFeat->psNext;
	}
	
	// none found
	if (!stackPushResult(ST_FEATURE, (UDWORD)NULL))
	{
		return FALSE;
	}
}


/*
// -----------------------------------------------------------------------------------------
// enum next visible feature of required type.
// note: wont return features covered by structures (ie oil resources)
// YUK NASTY BUG. CANT RELY ON THE FEATURE LIST BETWEEN CALLS.
BOOL scrGetFeature(void)
{
	SDWORD	bucket;
		
	if ( !stackPopParams(1,VAL_INT,&bucket) )
	{
		return FALSE;
	}

	while(psCurrEnumFeature[bucket])
	{
		if(	( psCurrEnumFeature[bucket]->psStats->subType == psFeatureStatToFind[bucket]->subType)
			&&
			( psCurrEnumFeature[bucket]->visible[playerToEnum[bucket]]	!= 0)
			&& 
			!TILE_HAS_STRUCTURE(mapTile(psCurrEnumFeature[bucket]->x>>TILE_SHIFT,psCurrEnumFeature[bucket]->y>>TILE_SHIFT) )
		   )
		{
			if (!stackPushResult(ST_FEATURE,(UDWORD) psCurrEnumFeature[bucket]))			//	push result
			{
				return FALSE;
			}
			psCurrEnumFeature[bucket] = psCurrEnumFeature[bucket]->psNext;				
			return TRUE;
		}

		psCurrEnumFeature[bucket] = psCurrEnumFeature[bucket]->psNext;
	}
	// push NULL, none found;
	if (!stackPushResult(ST_FEATURE, (UDWORD)NULL))
	{
		return FALSE;
	}
	return TRUE;
}
*/

// -----------------------------------------------------------------------------------------
//Add a feature
BOOL scrAddFeature(void)
{
	FEATURE_STATS	*psStat;
	FEATURE			*psFeat = NULL;
	SDWORD			iX, iY, iMapX, iMapY, iTestX, iTestY, iFeat;

	if ( !stackPopParams(3, ST_FEATURESTAT, &iFeat,
		 VAL_INT, &iX, VAL_INT, &iY ) )
	{
		return FALSE;
	}

	psStat = (FEATURE_STATS *)(asFeatureStats + iFeat);

	ASSERT( (PTRVALID(psStat, sizeof(FEATURE_STATS)),
			"scrAddFeature: Invalid feature pointer") );

	if ( psStat != NULL )
	{
		iMapX = iX >> TILE_SHIFT;
		iMapY = iY >> TILE_SHIFT;

		/* check for wrecked feature already on-tile and remove */
		for(psFeat = apsFeatureLists[0]; psFeat; psFeat = psFeat->psNext)
		{
			iTestX = psFeat->x >> TILE_SHIFT;
			iTestY = psFeat->y >> TILE_SHIFT;

			if ( (iTestX == iMapX) && (iTestY == iMapY) )
			{
				if ( psFeat->psStats->subType == FEAT_BUILD_WRECK )
				{
					/* remove feature */
					removeFeature( psFeat );
					break;
				}
				else
				{
					ASSERT( (FALSE,
					"scrAddFeature: building feature on tile already occupied\n") );
				}
			}
		}
		
		psFeat = buildFeature( psStat, iX, iY, FALSE );
	}

	if (!stackPushResult(ST_FEATURE, (UDWORD)psFeat))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//Add a structure
BOOL scrAddStructure(void)
{
	STRUCTURE_STATS		*psStat;
	STRUCTURE			*psStruct = NULL;
	SDWORD				iX, iY, iMapX, iMapY;//, iWidth, iBreadth;
	SDWORD				iStruct, iPlayer;//, iW, iB;

	if ( !stackPopParams( 4, ST_STRUCTURESTAT, &iStruct, VAL_INT, &iPlayer,
							 VAL_INT, &iX, VAL_INT, &iY ) )
	{
		return FALSE;
	}

	psStat = (STRUCTURE_STATS *)(asStructureStats + iStruct);

	ASSERT( (PTRVALID(psStat, sizeof(STRUCTURE_STATS)),
			"scrAddStructure: Invalid feature pointer") );

	if ( psStat != NULL )
	{
		/* offset coords so building centre at (iX, iY) */
/*		no longer necessary - buildStruct no longer uses top left
		iX -= psStat->baseWidth*TILE_UNITS/2;
		iY -= psStat->baseBreadth*TILE_UNITS/2;*/

		iMapX = iX >> TILE_SHIFT;
		iMapY = iY >> TILE_SHIFT;

		/* check for structure already on-tile */
		if(TILE_HAS_STRUCTURE(mapTile(iMapX,iMapY)))
		{
			ASSERT( (FALSE,
			"scrAddStructure: tile already occupied by structure\n") );
		}
		
		psStruct = buildStructure( psStat, iX, iY, iPlayer, FALSE );
		if ( psStruct != NULL )
		{
			psStruct->status = SS_BUILT;
			buildingComplete(psStruct);

            /*
            Apart from this being wrong (iWidth = 0 when psStat->baseWidth = 1 
            and you end up in an infinite loop) we don't need to do this here 
            since the map is flattened as part of buildStructure 

			iWidth   = psStat->baseWidth/2;
			iBreadth = psStat->baseBreadth/2;

			// flatten tiles across building base
			for ( iW=iMapX; iW<=iMapX+(SDWORD)psStat->baseWidth; iW+=iWidth )
			{
				for ( iB=iMapY; iB<=iMapY+(SDWORD)psStat->baseBreadth; iB+=iBreadth )
				{
					setTileHeight(iW, iB, psStruct->z);
				}
			}*/
		}
	}

	if (!stackPushResult(ST_STRUCTURE, (UDWORD)psStruct))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//Destroy a structure
BOOL scrDestroyStructure(void)
{
	STRUCTURE	*psStruct;

	if (!stackPopParams(1, ST_STRUCTURE, &psStruct))
	{
		return FALSE;
	}

	if (psStruct == NULL)
	{
		ASSERT((PTRVALID(psStruct, sizeof(STRUCTURE)),
			"scrDestroyStructure: Invalid structure pointer"));
	}

	removeStruct( psStruct, TRUE );

	return TRUE;
}



// -----------------------------------------------------------------------------------------
//NEXT 2 FUNCS ONLY USED IN MULTIPLAYER AS FAR AS I KNOW (25 AUG98) alexl.
// static vars to enum structs;
static	STRUCTURE_STATS	*psStructStatToFind;
static	UDWORD			playerToEnumStruct;
static	UDWORD			enumStructCount;
static	BOOL			structfindany;

// init enum visible structures.
BOOL scrInitEnumStruct(void)
{
	SDWORD			player,iStat,targetplayer,any;
	
	if ( !stackPopParams(4,VAL_BOOL,&any, ST_STRUCTURESTAT, &iStat,  VAL_INT, &player, VAL_INT, &targetplayer) )
	{
		return FALSE;
	}

	if(any == 1)
	{
		structfindany = TRUE;
	}
	else
	{
		structfindany = FALSE;
	}
	psStructStatToFind	= (STRUCTURE_STATS *)(asStructureStats + iStat);
	playerToEnumStruct	= (UDWORD)player;
	enumStructCount		= 0;
	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrEnumStruct(void)
{
	UDWORD		count;
	STRUCTURE	*psStruct;

	// go to the correct start point in the structure list.
	count = 0;
	for(psStruct=apsStructLists[playerToEnumStruct];psStruct && count<enumStructCount;count++)
	{
		psStruct = psStruct->psNext;
	}
	
	if(psStruct == NULL)		// no more to find.
	{
		if (!stackPushResult(ST_STRUCTURE, (SDWORD)NULL))
		{
			return FALSE;
		}
		return TRUE;
	}

	while(psStruct)	// find a visible structure of required type.
	{
//		if(	(structfindany || (psStruct->pStructureType->type == psStructStatToFind->type))
		if(	(structfindany || (psStruct->pStructureType->ref == psStructStatToFind->ref))
			&&
			(psStruct->visible[playerToEnumStruct])
		   )
		{
			if (!stackPushResult(ST_STRUCTURE,(UDWORD) psStruct))			//	push result
			{
				return FALSE;
			}
			enumStructCount++;
			return TRUE;
		}
		enumStructCount++;
		psStruct = psStruct->psNext;
	}
	// push NULL, none found;
	if (!stackPushResult(ST_STRUCTURE, (UDWORD)NULL))
	{
		return FALSE;
	}
	return TRUE;
}



// -----------------------------------------------------------------------------------------
/*looks to see if a structure (specified by type) exists and is being built*/
BOOL scrStructureBeingBuilt(void)
{
//	INTERP_VAL			sVal;
	UDWORD				structInc;
	STRUCTURE_STATS		*psStats;
	SDWORD				player;
	BOOL				beingBuilt;

	if (!stackPopParams(2, ST_STRUCTURESTAT, &structInc, VAL_INT, &player))
	{
		return FALSE;
	}

/*	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (sVal.type != ST_STRUCTURESTAT)
	{
		ASSERT((FALSE, "scrStructureBeingBuilt: type mismatch for object"));
		return FALSE;
	}
	psStats = (STRUCTURE_STATS *)(asStructureStats + sVal.v.ival);
*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrStructureBeingBuilt:player number is too high"));
		return FALSE;
	}

	psStats = (STRUCTURE_STATS *)(asStructureStats + structInc);
	beingBuilt = FALSE;
	if (checkStructureStatus(psStats, player, SS_BEING_BUILT))
	{
		beingBuilt = TRUE;
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)beingBuilt))
	{
		return FALSE;
	}
	
	return TRUE;
}


#ifdef WIN32
// -----------------------------------------------------------------------------------------
// multiplayer skirmish only for now.
// returns TRUE if a specific struct is complete. I know it's like the previous func, 
BOOL scrStructureComplete(void)
{
	STRUCTURE	*psStruct;
	BOOL		result;
	if (!stackPopParams(1, ST_STRUCTURE, &psStruct))
	{
		return FALSE;
	}
	if(psStruct->status == SS_BUILT)
	{
		result = TRUE;
	}
	else
	{
		result = FALSE;
	}
	if (!stackPushResult(VAL_BOOL, result))
	{
		return FALSE;
	}
	
	return TRUE;
}

#endif

// -----------------------------------------------------------------------------------------
/*looks to see if a structure (specified by type) exists and built*/
BOOL scrStructureBuilt(void)
{
//	INTERP_VAL			sVal;
	UDWORD				structInc;
	STRUCTURE_STATS		*psStats;
	SDWORD				player;
	BOOL				built;

	if (!stackPopParams(2, ST_STRUCTURESTAT, &structInc, VAL_INT, &player))
	{
		return FALSE;
	}

/*	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	if (sVal.type != ST_STRUCTURESTAT)
	{
		ASSERT((FALSE, "scrStructureBuilt: type mismatch for object"));
		return FALSE;
	}
	psStats = (STRUCTURE_STATS *)(asStructureStats + sVal.v.ival);
*/
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrStructureBuilt:player number is too high"));
		return FALSE;
	}

	psStats = (STRUCTURE_STATS *)(asStructureStats + structInc);

	built = FALSE;
	if (checkStructureStatus(psStats, player, SS_BUILT))
	{
		built = TRUE;
	}
	if (!stackPushResult(VAL_BOOL, (UDWORD)built))
	{
		return FALSE;
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------
/*centre the view on an object - can be droid/structure or feature */
BOOL scrCentreView(void)
{
	BASE_OBJECT	*psObj;
//	INTERP_VAL	sVal;

	if (!stackPopParams(1, ST_BASEOBJECT, &psObj))
	{
		return FALSE;
	}

	if (psObj == NULL)
	{
		ASSERT((FALSE, "scrCentreView: NULL object"));
		return FALSE;
	}

	//centre the view on the objects x/y
	setViewPos(psObj->x >> TILE_SHIFT, psObj->y >> TILE_SHIFT,FALSE);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
/*centre the view on a position */
BOOL scrCentreViewPos(void)
{
	SDWORD		x,y;

	if (!stackPopParams(2, VAL_INT, &x, VAL_INT, &y))
	{
		return FALSE;
	}

	if ( (x < 0) || (x >= (SDWORD)mapWidth*TILE_UNITS) ||
		 (y < 0) || (y >= (SDWORD)mapHeight*TILE_UNITS))
	{
		ASSERT((FALSE, "scrCenterViewPos: coords off map"));
		return FALSE;
	}

	//centre the view on the objects x/y
	setViewPos(x >> TILE_SHIFT, y >> TILE_SHIFT,FALSE);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Get a pointer to a structure based on a stat - returns NULL if cannot find one
BOOL scrGetStructure(void)
{
	SDWORD				player, index;
	STRUCTURE			*psStruct;
	UDWORD				structType;
	BOOL				found;

	if (!stackPopParams(2, ST_STRUCTURESTAT, &index, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrGetStructure:player number is too high"));
		return FALSE;
	}

	structType = asStructureStats[index].ref;

	//search the players' list of built structures to see if one exists
	found = FALSE;
	for (psStruct = apsStructLists[player]; psStruct != NULL; psStruct = 
		psStruct->psNext)
	{
		if (psStruct->pStructureType->ref == structType)
		{
			found = TRUE;
			break;
		}
	}

	//make sure pass NULL back if not got one
	if (!found)
	{
		psStruct = NULL;
	}
	
	if (!stackPushResult(ST_STRUCTURE, (UDWORD)psStruct))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Get a pointer to a template based on a component stat - returns NULL if cannot find one
BOOL scrGetTemplate(void)
{
	SDWORD				player;
	DROID_TEMPLATE		*psTemplate;
	BOOL				found;
	INTERP_VAL			sVal;
	UDWORD				i;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrGetTemplate:player number is too high"));
		return FALSE;
	}

	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	//search the players' list of templates to see if one exists
	found = FALSE;
	for (psTemplate = apsDroidTemplates[player]; psTemplate != NULL; psTemplate = 
		psTemplate->psNext)
	{
		switch( sVal.type)
		{
		case ST_BODY:
			if (psTemplate->asParts[COMP_BODY] == sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_PROPULSION:
			if (psTemplate->asParts[COMP_PROPULSION] == sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_ECM:
			if (psTemplate->asParts[COMP_ECM] == sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_SENSOR:
			if (psTemplate->asParts[COMP_SENSOR] == sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_CONSTRUCT:
			if (psTemplate->asParts[COMP_CONSTRUCT] == sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_REPAIR:
			if (psTemplate->asParts[COMP_REPAIRUNIT] == sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_WEAPON:
			for (i=0; i < DROID_MAXWEAPS; i++)
			{
				if (psTemplate->asWeaps[i] == (UDWORD)sVal.v.ival)
				{
					found = TRUE;
					break;
				}
			}
			break;
		default:
			ASSERT((FALSE, "scrGetTemplate: unknown type"));
			return FALSE;
		}

		if (found)
		{
			break;
		}
	}

	//make sure pass NULL back if not got one
	if (!found)
	{
		psTemplate = NULL;
	}
	
	if (!stackPushResult(ST_TEMPLATE, (UDWORD)psTemplate))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Get a pointer to a droid based on a component stat - returns NULL if cannot find one
BOOL scrGetDroid(void)
{
	SDWORD				player;
	DROID				*psDroid;
	BOOL				found;
	INTERP_VAL			sVal;
	UDWORD				i;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrGetUnit:player number is too high"));
		return FALSE;
	}

	if (!stackPop(&sVal))
	{
		return FALSE;
	}

	//search the players' list of droid to see if one exists
	found = FALSE;
	for (psDroid = apsDroidLists[player]; psDroid != NULL; psDroid = 
		psDroid->psNext)
	{
		switch( sVal.type)
		{
		case ST_BODY:
			if (psDroid->asBits[COMP_BODY].nStat == (UDWORD)sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_PROPULSION:
			if (psDroid->asBits[COMP_PROPULSION].nStat == (UDWORD)sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_ECM:
			if (psDroid->asBits[COMP_ECM].nStat == (UDWORD)sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_SENSOR:
			if (psDroid->asBits[COMP_SENSOR].nStat == (UDWORD)sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_CONSTRUCT:
			if (psDroid->asBits[COMP_CONSTRUCT].nStat == (UDWORD)sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_REPAIR:
			if (psDroid->asBits[COMP_REPAIRUNIT].nStat == (UDWORD)sVal.v.ival)
			{
				found = TRUE;
			}
			break;
		case ST_WEAPON:
			for (i=0; i < DROID_MAXWEAPS; i++)
			{
				if (psDroid->asWeaps[i].nStat == (UDWORD)sVal.v.ival)
				{
					found = TRUE;
					break;
				}
			}
			break;
		default:
			ASSERT((FALSE, "scrGetUnit: unknown type"));
			return FALSE;
		}

		if (found)
		{
			break;
		}
	}

	//make sure pass NULL back if not got one
	if (!found)
	{
		psDroid = NULL;
	}
	
	if (!stackPushResult(ST_DROID, (UDWORD)psDroid))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets all the scroll params for the map
BOOL scrSetScrollParams(void)
{
	SDWORD				minX, minY, maxX, maxY;
    SDWORD              prevMinX, prevMinY, prevMaxX, prevMaxY;

	if (!stackPopParams(4, VAL_INT, &minX, VAL_INT, &minY, VAL_INT, &maxX, VAL_INT, &maxY))
	{
		return FALSE;
	}

	//check the values entered are valid
	if (minX < 0)
	{
		ASSERT((FALSE, "Minimum scroll x value is less than zero - ", minX));
		return FALSE;
	}
	if (minY < 0)
	{
		ASSERT((FALSE, "Minimum scroll y value is less than zero - ", minY));
	}
	if (maxX > (SDWORD)mapWidth)
	{
		ASSERT((FALSE, "Maximum scroll x value is greater than mapWidth - ", maxX));
	}
	if (maxX < (SDWORD)(visibleXTiles+1))
	{
		ASSERT((FALSE, "Maximum scroll x has to be bigger than Visible Width(22) - ", maxX));
	}
	if (maxY > (SDWORD)mapHeight)
	{
		ASSERT((FALSE, "Maximum scroll y value is greater than mapWidth - ", maxY));
	}
	if (maxY < (SDWORD)(visibleYTiles+1))
	{
		ASSERT((FALSE, "Maximum scroll y has to be bigger than Visible Height(22) - ", maxY));
	}

    prevMinX = scrollMinX;
    prevMinY = scrollMinY;
    prevMaxX = scrollMaxX;
    prevMaxY = scrollMaxY;

	scrollMinX = minX;
	scrollMaxX = maxX;
	scrollMinY = minY;
	scrollMaxY = maxY;

    //when the scroll limits change midgame - need to redo the lighting
    //initLighting(scrollMinX, scrollMinY, scrollMaxX, scrollMaxY);
    initLighting(prevMinX < scrollMinX ? prevMinX : scrollMinX, 
        prevMinY < scrollMinY ? prevMinY : scrollMinY, 
        prevMaxX < scrollMaxX ? prevMaxX : scrollMaxX,
        prevMaxY < scrollMaxY ? prevMaxY : scrollMaxY);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets the scroll minX separately for the map
BOOL scrSetScrollMinX(void)
{
	SDWORD				minX, prevMinX;

	if (!stackPopParams(1, VAL_INT, &minX))
	{
		return FALSE;
	}

	//check the value entered are valid
	if (minX < 0)
	{
		ASSERT((FALSE, "Minimum scroll x value is less than zero - ", minX));
		return FALSE;
	}

    prevMinX = scrollMinX;

    scrollMinX = minX;

    //when the scroll limits change midgame - need to redo the lighting
    //initLighting(scrollMinX, scrollMinY, scrollMaxX, scrollMaxY);
    initLighting(prevMinX < scrollMinX ? prevMinX : scrollMinX, 
        scrollMinY, scrollMaxX, scrollMaxY);

    return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets the scroll minY separately for the map
BOOL scrSetScrollMinY(void)
{
	SDWORD				minY, prevMinY;

	if (!stackPopParams(1, VAL_INT, &minY))
	{
		return FALSE;
	}

	//check the value entered are valid
	if (minY < 0)
	{
		ASSERT((FALSE, "Minimum scroll y value is less than zero - ", minY));
		return FALSE;
	}

    prevMinY = scrollMinY;

	scrollMinY = minY;

    //when the scroll limits change midgame - need to redo the lighting
    //initLighting(scrollMinX, scrollMinY, scrollMaxX, scrollMaxY);
    initLighting(scrollMinX, 
        prevMinY < scrollMinY ? prevMinY : scrollMinY, 
        scrollMaxX, scrollMaxY);

    return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets the scroll maxX separately for the map
BOOL scrSetScrollMaxX(void)
{
	SDWORD				maxX, prevMaxX;

	if (!stackPopParams(1, VAL_INT, &maxX))
	{
		return FALSE;
	}

	//check the value entered are valid
	if (maxX > (SDWORD)mapWidth)
	{
		ASSERT((FALSE, "Maximum scroll x value is greater than mapWidth - ", maxX));
		return FALSE;
	}

    prevMaxX = scrollMaxX;

	scrollMaxX = maxX;

    //when the scroll limits change midgame - need to redo the lighting
    //initLighting(scrollMinX, scrollMinY, scrollMaxX, scrollMaxY);
    initLighting(scrollMinX,  scrollMinY, 
        prevMaxX < scrollMaxX ? prevMaxX : scrollMaxX,
        scrollMaxY);

    return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets the scroll maxY separately for the map
BOOL scrSetScrollMaxY(void)
{
	SDWORD				maxY, prevMaxY;

	if (!stackPopParams(1, VAL_INT, &maxY))
	{
		return FALSE;
	}

	//check the value entered are valid
	if (maxY > (SDWORD)mapHeight)
	{
		ASSERT((FALSE, "Maximum scroll y value is greater than mapWidth - ", maxY));
		return FALSE;
	}

    prevMaxY = scrollMaxY;

	scrollMaxY = maxY;

    //when the scroll limits change midgame - need to redo the lighting
    //initLighting(scrollMinX, scrollMinY, scrollMaxX, scrollMaxY);
    initLighting(scrollMinX, scrollMinY, scrollMaxX,
        prevMaxY < scrollMaxY ? prevMaxY : scrollMaxY);

    return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets which sensor will be used as the default for a player
BOOL scrSetDefaultSensor(void)
{
	SDWORD				player;
	UDWORD				sensorInc;

	if (!stackPopParams(2, ST_SENSOR, &sensorInc, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetDefaultSensor:player number is too high"));
		return FALSE;
	}

	//check is a valid sensor Inc
	if (sensorInc > numSensorStats)
	{
		ASSERT((FALSE, "scrSetDefaultSensor: Sensor Inc is too high - %d", sensorInc));
		return FALSE;
	}

	//check that this sensor is a default sensor
	if (asSensorStats[sensorInc].location != LOC_DEFAULT)
	{

		ASSERT((FALSE, "scrSetDefaultSensor: This sensor is not a default one - %s", 
			getStatName(&asSensorStats[sensorInc]) ));
		return FALSE;
	}

	//assign since OK!
	aDefaultSensor[player] = sensorInc;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets which ECM will be used as the default for a player
BOOL scrSetDefaultECM(void)
{
	SDWORD				player;
	UDWORD				ecmInc;

	if (!stackPopParams(2, ST_ECM, &ecmInc, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetDefaultECM:player number is too high"));
		return FALSE;
	}

	//check is a valid ecmInc
	if (ecmInc > numECMStats)
	{
		ASSERT((FALSE, "scrSetDefaultECM: ECM Inc is too high - %d", ecmInc));
		return FALSE;
	}

	//check that this ecm is a default ecm
	if (asECMStats[ecmInc].location != LOC_DEFAULT)
	{
		ASSERT((FALSE, "scrSetDefaultECM: This ecm is not a default one - %s", 
			getStatName(&asECMStats[ecmInc])));
		return FALSE;
	}

	//assign since OK!
	aDefaultECM[player] = ecmInc;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets which RepairUnit will be used as the default for a player
BOOL scrSetDefaultRepair(void)
{
	SDWORD				player;
	UDWORD				repairInc;

	if (!stackPopParams(2, ST_REPAIR, &repairInc, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetDefaultRepair:player number is too high"));
		return FALSE;
	}

	//check is a valid repairInc
	if (repairInc > numRepairStats)
	{
		ASSERT((FALSE, "scrSetDefaultRepair: Repair Inc is too high - %d", repairInc));
		return FALSE;
	}

	//check that this repair is a default repair
	if (asRepairStats[repairInc].location != LOC_DEFAULT)
	{
		ASSERT((FALSE, "scrSetDefaultRepair: This repair is not a default one - %s", 
			getStatName(&asRepairStats[repairInc])));
		return FALSE;
	}

	//assign since OK!
	aDefaultRepair[player] = repairInc;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets the structure limits for a player
BOOL scrSetStructureLimits(void)
{
	SDWORD				player, limit;
	UDWORD				structInc;
	STRUCTURE_LIMITS	*psStructLimits;

	if (!stackPopParams(3, ST_STRUCTURESTAT, &structInc, VAL_INT, &limit, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetStructureLimits:player number is too high"));
		return FALSE;
	}

	if (structInc > numStructureStats)
	{
		ASSERT((FALSE, "scrSetStructureLimits: Structure stat is too high - %d", structInc));
		return FALSE;
	}

	if (limit < 0)
	{
		ASSERT((FALSE, "scrSetStructureLimits: limit is less than zero - %d", limit));
		return FALSE;
	}

	if (limit > LOTS_OF)
	{
		ASSERT((FALSE, "scrSetStructureLimits: limit is too high - %d - must be less than %d",
			limit, LOTS_OF));
		return FALSE;
	}

	psStructLimits = asStructLimits[player];
	psStructLimits[structInc].limit = (UBYTE)limit;
#ifdef WIN32
	psStructLimits[structInc].globalLimit = (UBYTE)limit;
#endif
	return TRUE;
}


#ifdef WIN32
// -----------------------------------------------------------------------------------------
// multiplayer limit handler.
BOOL scrApplyLimitSet()
{
	applyLimitSet();
	return TRUE;
}
#endif


// -----------------------------------------------------------------------------------------
// plays a sound for the specified player - only plays the sound if the 
// specified player = selectedPlayer
BOOL scrPlaySound(void)
{
	SDWORD	player, soundID;

	if (!stackPopParams(2, ST_SOUND, &soundID, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrPlaySound:player number is too high"));
		return FALSE;
	}

	if (player == (SDWORD)selectedPlayer)
	{
		audio_QueueTrack(soundID);
		if(bInTutorial)
		{
			audio_QueueTrack(ID_SOUND_OF_SILENCE);
		}
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// plays a sound for the specified player - only plays the sound if the 
// specified player = selectedPlayer - saves position
BOOL scrPlaySoundPos(void)
{
	SDWORD	player, soundID, iX, iY, iZ;

	if (!stackPopParams(5, ST_SOUND, &soundID, VAL_INT, &player,
							VAL_INT, &iX, VAL_INT, &iY, VAL_INT, &iZ))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrPlaySoundPos:player number is too high"));
		return FALSE;
	}

	if (player == (SDWORD)selectedPlayer)
	{
		audio_QueueTrackPos(soundID, iX, iY, iZ);
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------

/* add a text message to the top of the screen for the selected player*/
BOOL scrShowConsoleText(void)
{
	STRING				*pText;
	SDWORD				player;

	if (!stackPopParams(2, ST_TEXTSTRING, &pText, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddConsoleText:player number is too high"));
		return FALSE;
	}
	
	if (player == (SDWORD)selectedPlayer)
	{
		permitNewConsoleMessages(TRUE);
		addConsoleMessage(pText, CENTRE_JUSTIFY);
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
/* add a text message to the top of the screen for the selected player*/
BOOL scrAddConsoleText(void)
{
	STRING				*pText;
	SDWORD				player;

	if (!stackPopParams(2, ST_TEXTSTRING, &pText, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddConsoleText:player number is too high"));
		return FALSE;
	}
	
	if (player == (SDWORD)selectedPlayer)
	{
		permitNewConsoleMessages(TRUE);
		setConsolePermanence(TRUE,TRUE);
		addConsoleMessage(pText, CENTRE_JUSTIFY);
		permitNewConsoleMessages(FALSE);
	}

	return TRUE;
}


#ifdef WIN32
// -----------------------------------------------------------------------------------------
/* add a text message to the top of the screen for the selected player - without clearing whats there*/
BOOL scrTagConsoleText(void)
{
	STRING				*pText;
	SDWORD				player;

	if (!stackPopParams(2, ST_TEXTSTRING, &pText, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddConsoleText:player number is too high"));
		return FALSE;
	}
	
	if (player == (SDWORD)selectedPlayer)
	{
		permitNewConsoleMessages(TRUE);
		setConsolePermanence(TRUE,FALSE);
		addConsoleMessage(pText, CENTRE_JUSTIFY);
		permitNewConsoleMessages(FALSE);
	}

	return TRUE;
}
#endif

// -----------------------------------------------------------------------------------------
#ifdef WIN32
BOOL	scrClearConsole(void)
{
	flushConsoleMessages();
	return(TRUE);
}
#endif
// -----------------------------------------------------------------------------------------
//demo functions for turning the power on
BOOL scrTurnPowerOff(void)
{
	//powerCalculated = FALSE;
	powerCalc(FALSE);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//demo functions for turning the power off
BOOL scrTurnPowerOn(void)
{

	//powerCalculated = TRUE;
	powerCalc(TRUE);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//flags when the tutorial is over so that console messages can be turned on again
BOOL scrTutorialEnd(void)
{
	initConsoleMessages();
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//function to play a full-screen video in the middle of the game for the selected player
BOOL scrPlayVideo(void)
{
	STRING				*pVideo, *pText;

	if (!stackPopParams(2, ST_TEXTSTRING, &pVideo, ST_TEXTSTRING, &pText))
	{
		return FALSE;
	}

		seq_ClearSeqList();
		seq_AddSeqToList(pVideo, NULL, pText, FALSE,0);		// Arpzzzzzzzzzzzzzzzlksht!
		seq_StartNextFullScreenVideo();

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//checks to see if there are any droids for the specified player
BOOL scrAnyDroidsLeft(void)
{
	SDWORD		player;
	BOOL		droidsLeft;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAnyUnitsLeft:player number is too high"));
		return FALSE;
	}

	//check the players list for any droid
	droidsLeft = TRUE;
	if (apsDroidLists[player] == NULL)
	{
		droidsLeft = FALSE;
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)droidsLeft))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//function to call when the game is over, plays a message then does game over stuff.
//
BOOL scrGameOverMessage(void)
{
	BOOL			gameOver;
	MESSAGE			*psMessage;
	SDWORD			msgType, player;
	VIEWDATA		*psViewData;
	//UDWORD			height;


	if (!stackPopParams(4, ST_INTMESSAGE, &psViewData , VAL_INT, &msgType,
				VAL_INT, &player, VAL_BOOL, &gameOver))
	{
		return FALSE;
	}


	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrGameOverMessage:player number is too high"));
		return FALSE;
	}

	//create the message
	psMessage = addMessage(msgType, FALSE, player);

	ASSERT((msgType != MSG_PROXIMITY, "scrGameOverMessage: Bad message type (MSG_PROXIMITY)"));

	if (psMessage)
	{
		//set the data
		psMessage->pViewData = (MSG_VIEWDATA *)psViewData;
		displayImmediateMessage(psMessage);
		stopReticuleButtonFlash(IDRET_INTEL_MAP);

        //we need to set this here so the VIDEO_QUIT callback is not called
        setScriptWinLoseVideo((UBYTE)(gameOver ? PLAY_WIN : PLAY_LOSE));

        // Can't do this cos won't process windows stuff 
        // Wait for the video to finish.
		/*while (loop_GetVideoStatus())
		{
			videoLoop();
		}*/
	}

    //this now called when the video Quit is processed
	//displayGameOver(gameOver);

	return TRUE;
}


#ifdef PSX
UBYTE OutroMovie[] = "misc\\outro.str";
UBYTE OutroText[] = "misc\\outro.txa";
#endif

// -----------------------------------------------------------------------------------------
//function to call when the game is over
BOOL scrGameOver(void)
{
	BOOL	gameOver;

	if (!stackPopParams(1, VAL_BOOL, &gameOver))
	{
		return FALSE;
	}

    /*this function will only be called with gameOver = TRUE when at the end of 
    the game so we'll just hard-code what happens!*/
#ifdef WIN32
    //don't want this in multiplayer...
    if (!bMultiPlayer)
#endif
    {
        if (gameOver == TRUE AND !bInTutorial)
        {
            //we need to set this here so the VIDEO_QUIT callback is not called
		    setScriptWinLoseVideo(PLAY_WIN);

    	    seq_ClearSeqList();
#ifdef WIN32
	        seq_AddSeqToList("outro.rpl",NULL,"outro.txa", FALSE,0);
	        seq_StartNextFullScreenVideo();
#else
			// Set the stack pointer to point to the alternative stack which is'nt limited to 1k.
//	        seq_AddSeqToList("misc\\outro.str",NULL,"misc\\outro.txa", FALSE,0);
		
			SetSpAlt();
	        seq_AddSeqToList(OutroMovie,NULL,OutroText, FALSE,0);
	        seq_StartNextFullScreenVideo();
			SetSpAltNormal();
#endif
        }
    }

	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrAnyFactoriesLeft(void)
{
	SDWORD		player;
	BOOL		result;
	STRUCTURE	*psCurr;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAnyFactorysLeft:player number is too high"));
		return FALSE;
	}

	//check the players list for any structures
	result = FALSE;
	if(apsStructLists[player])
	{
		for (psCurr = apsStructLists[player]; psCurr != NULL; psCurr = psCurr->psNext)
		{
//			if (psCurr->pStructureType->type	== REF_FACTORY OR
//				psCurr->pStructureType->type == REF_CYBORG_FACTORY OR
//				psCurr->pStructureType->type == REF_VTOL_FACTORY )
			if(StructIsFactory(psCurr))
			{
				result = TRUE;
				break;
			}
		}
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)result))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
//checks to see if there are any structures (except walls) for the specified player
BOOL scrAnyStructButWallsLeft(void)
{
	SDWORD		player;
	BOOL		structuresLeft;
	STRUCTURE	*psCurr;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAnyStructuresButWallsLeft:player number is too high"));
		return FALSE;
	}

	//check the players list for any structures
	structuresLeft = TRUE;
	if (apsStructLists[player] == NULL)
	{
		structuresLeft = FALSE;
	}
	else
	{
		structuresLeft = FALSE;
		for (psCurr = apsStructLists[player]; psCurr != NULL; psCurr = psCurr->psNext)
		{
			if (psCurr->pStructureType->type != REF_WALL AND psCurr->pStructureType->
				type != REF_WALLCORNER)
			{
				structuresLeft = TRUE;
				break;
			}
		}
	}

	if (!stackPushResult(VAL_BOOL, (UDWORD)structuresLeft))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//defines the background audio to play
BOOL scrPlayBackgroundAudio(void)
{
	STRING	*pText;
	SDWORD	iVol;

	if (!stackPopParams(2, ST_TEXTSTRING, &pText, VAL_INT, &iVol))
	{
		return FALSE;
	}

#ifdef WIN32
	cdspan_PlayInGameAudio(pText, iVol);
#endif

	return TRUE;
	
}

// -----------------------------------------------------------------------------------------
//defines the CD audio to play
BOOL scrPlayCDAudio(void)
{
	SDWORD	iTrack;

	if (!stackPopParams(1, VAL_INT, &iTrack))
	{
		return FALSE;
	}

#ifdef WIN32

#if defined(WIN32) && !defined(I_LIKE_LISTENING_TO_CDS)
	cdAudio_PlayTrack( iTrack );	
#endif
//#ifdef PSX
//	cdAudio_PlayTrack( iTrack );	
//
//#endif
#endif	// Playstation CD audio no hardcoded.
	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrStopCDAudio(void)
{
#ifdef WIN32
#if defined(WIN32) && !defined(I_LIKE_LISTENING_TO_CDS)
	cdAudio_Stop();
#endif
#endif	// Playstation CD audio no hardcoded.
	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrPauseCDAudio(void)
{
#if defined(WIN32) && !defined(I_LIKE_LISTENING_TO_CDS)
	cdAudio_Pause();
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrResumeCDAudio(void)
{
#if defined(WIN32) && !defined(I_LIKE_LISTENING_TO_CDS)
	cdAudio_Resume();
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// set the retreat point for a player
BOOL scrSetRetreatPoint(void)
{
	SDWORD	player, x,y;

	if (!stackPopParams(3, VAL_INT, &player, VAL_INT, &x, VAL_INT, &y))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetRetreatPoint: player out of range"));
		return FALSE;
	}
	if (x < 0 || x >= (SDWORD)mapWidth*TILE_UNITS ||
		y < 0 || y >= (SDWORD)mapHeight*TILE_UNITS)
	{
		ASSERT((FALSE, "scrSetRetreatPoint: coords off map"));
		return FALSE;
	}

	asRunData[player].sPos.x = x;
	asRunData[player].sPos.y = y;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// set the retreat force level
BOOL scrSetRetreatForce(void)
{
	SDWORD	player, level, numDroids;
	DROID	*psCurr;

	if (!stackPopParams(2, VAL_INT, &player, VAL_INT, &level))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetRetreatForce: player out of range"));
		return FALSE;
	}

	if (level > 100 || level < 0)
	{
		ASSERT((FALSE, "scrSetRetreatForce: level out of range"));
		return FALSE;
	}

	// count up the current number of droids
	numDroids = 0;
	for(psCurr = apsDroidLists[player]; psCurr; psCurr=psCurr->psNext)
	{
		numDroids += 1;
	}

	asRunData[player].forceLevel = (UBYTE)(level * numDroids / 100);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// set the retreat leadership
BOOL scrSetRetreatLeadership(void)
{
	SDWORD	player, level;

	if (!stackPopParams(2, VAL_INT, &player, VAL_INT, &level))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetRetreatLeadership: player out of range"));
		return FALSE;
	}

	if (level > 100 || level < 0)
	{
		ASSERT((FALSE, "scrSetRetreatLeadership: level out of range"));
		return FALSE;
	}

	asRunData[player].leadership = (UBYTE)level;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// set the retreat point for a group
BOOL scrSetGroupRetreatPoint(void)
{
	SDWORD		x,y;
	DROID_GROUP	*psGroup;

	if (!stackPopParams(3, ST_GROUP, &psGroup, VAL_INT, &x, VAL_INT, &y))
	{
		return FALSE;
	}

	if (x < 0 || x >= (SDWORD)mapWidth*TILE_UNITS ||
		y < 0 || y >= (SDWORD)mapHeight*TILE_UNITS)
	{
		ASSERT((FALSE, "scrSetRetreatPoint: coords off map"));
		return FALSE;
	}

	psGroup->sRunData.sPos.x = x;
	psGroup->sRunData.sPos.y = y;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrSetGroupRetreatForce(void)
{
	SDWORD		level, numDroids;
	DROID_GROUP	*psGroup;
	DROID		*psCurr;

	if (!stackPopParams(2, ST_GROUP, &psGroup, VAL_INT, &level))
	{
		return FALSE;
	}

	if (level > 100 || level < 0)
	{
		ASSERT((FALSE, "scrSetRetreatForce: level out of range"));
		return FALSE;
	}

	// count up the current number of droids
	numDroids = 0;
	for(psCurr = psGroup->psList; psCurr; psCurr=psCurr->psGrpNext)
	{
		numDroids += 1;
	}

	psGroup->sRunData.forceLevel = (UBYTE)(level * numDroids / 100);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// set the retreat health level
BOOL scrSetRetreatHealth(void)
{
	SDWORD	player, health;

	if (!stackPopParams(2, VAL_INT, &player, VAL_INT, &health))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetHealthForce: player out of range"));
		return FALSE;
	}

	if (health > 100 || health < 0)
	{
		ASSERT((FALSE, "scrSetHealthForce: health out of range"));
		return FALSE;
	}

	asRunData[player].healthLevel = (UBYTE)health;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrSetGroupRetreatHealth(void)
{
	SDWORD		health;
	DROID_GROUP	*psGroup;

	if (!stackPopParams(2, ST_GROUP, &psGroup, VAL_INT, &health))
	{
		return FALSE;
	}

	if (health > 100 || health < 0)
	{
		ASSERT((FALSE, "scrSetGroupRetreatHealth: health out of range"));
		return FALSE;
	}

	psGroup->sRunData.healthLevel = (UBYTE)health;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// set the retreat leadership
BOOL scrSetGroupRetreatLeadership(void)
{
	SDWORD		level;
	DROID_GROUP	*psGroup;

	if (!stackPopParams(2, ST_GROUP, &psGroup, VAL_INT, &level))
	{
		return FALSE;
	}

	if (level > 100 || level < 0)
	{
		ASSERT((FALSE, "scrSetRetreatLeadership: level out of range"));
		return FALSE;
	}

	psGroup->sRunData.leadership = (UBYTE)level;

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//start a Mission - the missionType is ignored now - gets it from the level data ***********
BOOL scrStartMission(void)
{
	STRING				*pGame;
	SDWORD				missionType;
	LEVEL_DATASET		*psNewLevel;

	if (!stackPopParams(2, VAL_INT, &missionType, ST_LEVEL, &pGame))
	{
		return FALSE;
	}

	//if (missionType > MISSION_NONE)
	if (missionType > LDS_NONE)
	{
		ASSERT((FALSE, "Invalid Mission Type"));
		return FALSE;
	}

	// check the last mission got finished
	/*if (mission.type != MISSION_NONE)
	{
		DBMB(("scrStartMission: old mission incomplete\n   ending mission with success"));
		endMission(TRUE);
	}*/

	// tell the loop that a new level has to be loaded up - not yet!
	//loopNewLevel = TRUE;
	strcpy(pLevelName, pGame);

	// find the level dataset
	if (!levFindDataSet(pGame, &psNewLevel))
	{
		DBERROR(("scrStartMission: couldn't find level data"));
		return FALSE;
	}

	//set the mission rolling...
	//nextMissionType = missionType;
	nextMissionType = psNewLevel->type;
//	loopMissionState = LMS_SETUPMISSION;
	loopMissionState = LMS_CLEAROBJECTS;

/*	if (!setUpMission(missionType))
	{
		ASSERT((FALSE, "Unable to start mission - %s", pGame));
		return FALSE;
	}*/

	return TRUE;
}

//end a mission - NO LONGER CALLED FROM SCRIPT
/*BOOL scrEndMission(void)
{
	BOOL	status;

	if (!stackPopParams(1, VAL_BOOL, &status))
	{
		return FALSE;
	}

	endMission(status);
	return TRUE;
}*/
// -----------------------------------------------------------------------------------------
//set Snow (enable disable snow)
BOOL scrSetSnow(void)
{
	BOOL bState;

	if (!stackPopParams(1, VAL_BOOL, &bState))
	{
		return FALSE;
	}

#ifdef WIN32
	if(bState)
	{
		atmosSetWeatherType(WT_SNOWING);
	}
	else
	{
		atmosSetWeatherType(WT_NONE);
	}
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set Rain (enable disable Rain)
BOOL scrSetRain(void)
{
	BOOL bState;

	if (!stackPopParams(1, VAL_BOOL, &bState))
	{
		return FALSE;
	}

#ifdef WIN32
	if(bState)
	{
		atmosSetWeatherType(WT_RAINING);
	}
	else
	{
		atmosSetWeatherType(WT_NONE);
	}
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set Background Fog (replace fade out with fog)
BOOL scrSetBackgroundFog(void)
{
	BOOL bState;

	if (!stackPopParams(1, VAL_BOOL, &bState))
	{
		return FALSE;
	}
#ifdef WIN32
	//jps 17 feb 99 just set the status let other code worry about fogEnable/reveal
	if (bState)//true, so go to false
	{
		//restart fog if it was off
		if ((fogStatus == 0) && war_GetFog() && !(bMultiPlayer && game.fog))
		{
			pie_EnableFog(TRUE);
		}
		fogStatus |= FOG_BACKGROUND;//set lowest bit of 3
	}
	else
	{
		fogStatus &= FOG_FLAGS-FOG_BACKGROUND;//clear middle bit of 3
		//disable fog if it longer used
		if (fogStatus == 0)
		{
			pie_SetFogStatus(FALSE);
			pie_EnableFog(FALSE);
		}
	}

/* jps 17 feb 99
	if(getRevealStatus())		// fog'o war enabled
	{
		pie_SetFogStatus(FALSE);
		pie_EnableFog(FALSE);
//		fogStatus = 0;
		return TRUE;
	}

	if (bState)//true, so go to false
	{
		if (war_GetFog())
		{
			//restart fog if it was off
			if (fogStatus == 0)
			{
				pie_EnableFog(TRUE);
			}
			fogStatus |= FOG_BACKGROUND;//set lowest bit of 3
		}
	}
	else
	{
		if (war_GetFog())
		{
			fogStatus &= FOG_FLAGS-FOG_BACKGROUND;//clear middle bit of 3
			//disable fog if it longer used
			if (fogStatus == 0)
			{
				pie_SetFogStatus(FALSE);
				pie_EnableFog(FALSE);
			}
		}
	}
*/
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set Depth Fog (gradual fog from mid range to edge of world)
BOOL scrSetDepthFog(void)
{
	BOOL bState;

	if (!stackPopParams(1, VAL_BOOL, &bState))
	{
		return FALSE;
	}
#ifdef WIN32		// ffs am
//jps 17 feb 99 just set the status let other code worry about fogEnable/reveal
	if (bState)//true, so go to false
	{
		//restart fog if it was off
		if ((fogStatus == 0) && war_GetFog() )
		{
			pie_EnableFog(TRUE);
		}
		fogStatus |= FOG_DISTANCE;//set lowest bit of 3
	}
	else
	{
		fogStatus &= FOG_FLAGS-FOG_DISTANCE;//clear middle bit of 3
		//disable fog if it longer used
		if (fogStatus == 0)
		{
			pie_SetFogStatus(FALSE);
			pie_EnableFog(FALSE);
		}
	}

/* jps 17 feb 99	if(getRevealStatus())		// fog'o war enabled
	{	
		pie_SetFogStatus(FALSE);
		pie_EnableFog(FALSE);
//		fogStatus = 0;
		return TRUE;
	}

	if (bState)//true, so go to false
	{
		if (war_GetFog())
		{
			//restart fog if it was off
			if (fogStatus == 0)
			{
				pie_EnableFog(TRUE);
			}
			fogStatus |= FOG_DISTANCE;//set lowest bit of 3
		}
	}
	else
	{
		if (war_GetFog())
		{
			fogStatus &= FOG_FLAGS-FOG_DISTANCE;//clear middle bit of 3
			//disable fog if it longer used
			if (fogStatus == 0)
			{
				pie_SetFogStatus(FALSE);
				pie_EnableFog(FALSE);
			}
		}
	}
*/
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set Mission Fog colour, may be modified by weather effects
BOOL scrSetFogColour(void)
{
	SDWORD	red,green,blue;
	SDWORD	scrFogColour;

	if (!stackPopParams(3, VAL_INT, &red, VAL_INT, &green, VAL_INT, &blue))
	{
		return FALSE;
	}

#ifdef WIN32
//	if (pie_GetRenderEngine() == ENGINE_GLIDE)
//	{
		red &= 0xff;	
		green &= 0xff;	
		blue &= 0xff;	
		scrFogColour = ((red << 16) + (green << 8) + blue);
		pie_SetFogColour(scrFogColour);
//	}
#endif
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// test function to test variable references
BOOL scrRefTest(void)
{
	SDWORD		Num;


	if (!stackPopParams(1,VAL_INT, Num));
	{
		return FALSE;
	}

	DBPRINTF(("scrRefTest: num: %d \n", Num));

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// is player a human or computer player? (multiplayer only)
#ifdef WIN32
BOOL scrIsHumanPlayer(void)
{
	SDWORD	player;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (!stackPushResult(VAL_BOOL,isHumanPlayer(player) ))
	{
		return FALSE;
	}

	return TRUE;
}
#endif

// -----------------------------------------------------------------------------------------
// Set an alliance between two players
BOOL scrCreateAlliance(void)
{
	SDWORD	player1,player2;

	if (!stackPopParams(2, VAL_INT, &player1, VAL_INT, &player2))
	{
		return FALSE;
	}

	if (player1 < 0 || player1 >= MAX_PLAYERS ||
		player2 < 0 || player2 >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrCreateAlliance: player out of range"));
		return FALSE;
	}

	if(bMultiPlayer) 
	{
		if(game.alliance==NO_ALLIANCES || player1 >= game.maxPlayers || player2>=game.maxPlayers)
		{
			return TRUE;
		}
	}

	formAlliance((UBYTE)player1, (UBYTE)player2,TRUE,FALSE);

/*
#ifdef WIN32	
	if(bMultiPlayer) 
	{

		if(game.alliance==NO_ALLIANCES || player1 >= game.maxPlayers || player2>=game.maxPlayers)
		{
			return TRUE;
		}

		if(alliances[player1][player2] != ALLIANCE_FORMED)
		{
#ifdef DEBUG
			CONPRINTF(ConsoleString,(ConsoleString,"%d and %d form an alliance.",player1,player2));
#endif
			sendAlliance((UBYTE)player1,(UBYTE)player2,ALLIANCE_FORMED,0);
		}
	}
#endif

	alliances[player1][player2] = ALLIANCE_FORMED;
	alliances[player2][player1] = ALLIANCE_FORMED;
*/
	return TRUE;
}


#ifdef WIN32
// -----------------------------------------------------------------------------------------
// offer an alliance
BOOL scrOfferAlliance(void)
{
	SDWORD	player1,player2;
	if (!stackPopParams(2, VAL_INT, &player1, VAL_INT, &player2))
	{
		return FALSE;
	}
	if (game.alliance==NO_ALLIANCES ||
		player1 < 0 || player1 >= MAX_PLAYERS ||
		player2 < 0 || player2 >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrCreateAlliance: player out of range"));
		return FALSE;
	}


	requestAlliance((UBYTE)player1,(UBYTE)player2,TRUE,TRUE);
	return TRUE;
}
#endif

// -----------------------------------------------------------------------------------------
// Break an alliance between two players
BOOL scrBreakAlliance(void)
{
	SDWORD	player1,player2;

	if (!stackPopParams(2, VAL_INT, &player1, VAL_INT, &player2))
	{
		return FALSE;
	}

	if (
		player1 < 0 || player1 >= MAX_PLAYERS ||
		player2 < 0 || player2 >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrCreateAlliance: player out of range"));
		return FALSE;
	}
/*
if(bMultiPlayer)
	{
	

		if(alliances[player1][player2] != ALLIANCE_BROKEN)
		{
			CONPRINTF(ConsoleString,(ConsoleString,"%d and %d break alliance.",player1,player2));
			sendAlliance((UBYTE)player1,(UBYTE)player2,ALLIANCE_BROKEN,0);
		}
}
*/


	if(bMultiPlayer)
	{
		if(game.alliance==NO_ALLIANCES || player1 >= game.maxPlayers || player2>=game.maxPlayers)
		{
			return TRUE;
		}
		breakAlliance(player1,player2,TRUE,TRUE);
	}
	else
	{
		breakAlliance(player1,player2,FALSE,TRUE);
	}
/*
	alliances[player1][player2] = ALLIANCE_BROKEN;
	alliances[player2][player1] = ALLIANCE_BROKEN;
*/
	return TRUE;
}


// -----------------------------------------------------------------------------------------
// Multiplayer relevant scriptfuncs
// returns true if 2 or more players are in alliance.
BOOL scrAllianceExists(void)
{
#ifdef WIN32
	UDWORD i,j;
	for(i=0;i<MAX_PLAYERS;i++)
	{
		for(j=0;j<MAX_PLAYERS;j++)
		{
			if(alliances[i][j] == ALLIANCE_FORMED)
			{
				if (!stackPushResult(VAL_BOOL, TRUE))
				{
					return FALSE;
				}
				return TRUE;
			}
		}
	}

#endif

	if (!stackPushResult(VAL_BOOL, FALSE))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL scrAllianceExistsBetween(void)
{
	UDWORD i,j;

	
	if (!stackPopParams(2, VAL_INT, &i,VAL_INT, &j))
	{
		return FALSE;
	}
	if(alliances[i][j] == ALLIANCE_FORMED)
	{
		if (!stackPushResult(VAL_BOOL, TRUE))
		{
			return FALSE;
		}
		return TRUE;
	}
	
	if (!stackPushResult(VAL_BOOL, FALSE))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
BOOL scrPlayerInAlliance(void)
{
	UDWORD player,j;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	for(j=0;j<MAX_PLAYERS;j++)
	{
		if(alliances[player][j] == ALLIANCE_FORMED)
		{
			if (!stackPushResult(VAL_BOOL, TRUE))
			{
				return FALSE;
			}
			return TRUE;
		}
	}
	if (!stackPushResult(VAL_BOOL, FALSE))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// returns true if a single alliance is dominant.
BOOL scrDominatingAlliance(void)
{
	UDWORD i,j;
#ifdef WIN32
	for(i=0;i<MAX_PLAYERS;i++)
	{
		for(j=0;j<MAX_PLAYERS;j++)
		{
			if(   isHumanPlayer(j)
			   && isHumanPlayer(i) 
			   && i != j  
			   && alliances[i][j] != ALLIANCE_FORMED)
			{
				if (!stackPushResult(VAL_BOOL, FALSE))
				{
					return FALSE;
				}
				return TRUE;
			}
		}
// -----------------------------------------------------------------------------------------
	}
#endif

	if (!stackPushResult(VAL_BOOL, TRUE))
	{
		return FALSE;
	}

	return TRUE;
}


BOOL scrMyResponsibility(void)
{
	SDWORD player;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}
#ifdef WIN32
	if(	myResponsibility(player) )
	{
		if (!stackPushResult(VAL_BOOL, TRUE))
		{
			return FALSE;
		}
	}
	else
	{
		if (!stackPushResult(VAL_BOOL, FALSE))
		{
			return FALSE;
		}
	}
#else
	if (!stackPushResult(VAL_BOOL, TRUE))
	{
		return FALSE;
	}
#endif

	return TRUE;
}	

// -----------------------------------------------------------------------------------------
/*checks to see if a structure of the type specified exists within the 
specified range of an XY location */
BOOL scrStructureBuiltInRange(void)
{
	SDWORD		player, index, x, y, range;
	SDWORD		rangeSquared;
	STRUCTURE	*psCurr;
	BOOL		found;
	SDWORD		xdiff, ydiff;
	STRUCTURE_STATS *psTarget;

	if (!stackPopParams(5, ST_STRUCTURESTAT, &index, VAL_INT, &x, VAL_INT, &y, 
		VAL_INT, &range, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrStructureBuiltInRange:player number is too high"));
		return FALSE;
	}

	if (x < (SDWORD)0 OR (x >> TILE_SHIFT) > (SDWORD)mapWidth)
	{
		ASSERT((FALSE, "scrStructureBuiltInRange : invalid X coord"));
		return FALSE;
	}
	if (y < (SDWORD)0 OR (y >> TILE_SHIFT) > (SDWORD)mapHeight)
	{
		ASSERT((FALSE,"scrStructureBuiltInRange : invalid Y coord"));
		return FALSE;
	}
	if (index < (SDWORD)0 OR index > (SDWORD)numStructureStats)
	{
		ASSERT((FALSE, "scrStructureBuiltInRange : Invalid structure stat"));
		return FALSE;
	}
	if (range < (SDWORD)0)
	{
		ASSERT((FALSE, "scrStructureBuiltInRange : Rnage is less than zero"));
		return FALSE;
	}

	//now look through the players list of structures to see if this type 
	//exists within range
	psTarget = &asStructureStats[index];
	rangeSquared = range * range;
	found = FALSE;
	for(psCurr = apsStructLists[player]; psCurr; psCurr = psCurr->psNext)
	{
		xdiff = (SDWORD)psCurr->x - x;
		ydiff = (SDWORD)psCurr->y - y;
		if (xdiff*xdiff + ydiff*ydiff <= rangeSquared)
		{	

#ifdef HASH_NAMES
			if( psCurr->pStructureType->NameHash == psTarget->NameHash ) 
#else
			if( strcmp(psCurr->pStructureType->pName,psTarget->pName) == 0 ) 
#endif	
			{
				if (psCurr->status == SS_BUILT)
				{
					found = TRUE;
					break;
				}
			}
		}
	}
	//make sure pass NULL back if not got one
	if (!found)
	{
		psCurr = NULL;
	}
	
	if (!stackPushResult(ST_STRUCTURE, (UDWORD)psCurr))
	{
		return FALSE;
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// generate a random number
BOOL scrRandom(void)
{
	SDWORD		range, result;

	if (!stackPopParams(1, VAL_INT, &range))
	{
		return FALSE;
	}

	if (range == 0)
	{
		result = 0;
	}
	else if (range > 0)
	{
		result = rand() % range;
	}
	else
	{
		result = rand() % (-range);
	}

	if (!stackPushResult(VAL_INT, result))
	{
		return FALSE;
	}

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// randomise the random number seed
BOOL scrRandomiseSeed(void)
{
	srand((UDWORD)clock());

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//explicitly enables a research topic
BOOL scrEnableResearch(void)
{
	SDWORD		player;
	RESEARCH	*psResearch;

	if (!stackPopParams(2, ST_RESEARCH, &psResearch, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrEnableResearch:player number is too high"));
		return FALSE;
	}

	if (!enableResearch(psResearch, player))
	{
		return FALSE;
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//acts as if the research topic was completed - used to jump into the tree
BOOL scrCompleteResearch(void)
{
	SDWORD		player;
	RESEARCH	*psResearch;
	UDWORD		researchIndex;

	if (!stackPopParams(2, ST_RESEARCH, &psResearch, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrCompleteResearch:player number is too high"));
		return FALSE;
	}


	if(psResearch == NULL)
	{	
		ASSERT((FALSE, "scrCompleteResearch: no such research topic"));
		return FALSE;
	}

	researchIndex = psResearch - asResearch;
	if (researchIndex > numResearch)
	{
		ASSERT((FALSE, "scrCompleteResearch: invalid research index"));
		return FALSE;
	}

	researchResult(researchIndex, (UBYTE)player, FALSE);

#ifdef WIN32
	if(bMultiPlayer && (gameTime > 2 ))
	{
		SendResearch((UBYTE)player,researchIndex );
	}
#endif

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// This routine used to start just a reticule button flashing
//   .. now it starts any button flashing (awaiting implmentation from widget library)
BOOL scrFlashOn(void)
{
	SDWORD		button;

	if (!stackPopParams(1, VAL_INT, &button))
	{
		return FALSE;
	}
#ifdef WIN32
	// For the time being ... we will perform the old code for the reticule ...
	if (button >= IDRET_OPTIONS && button <= IDRET_CANCEL)
	{
		flashReticuleButton((UDWORD)button);
		return TRUE;
	}
#endif

	if(widgGetFromID(psWScreen,button) != NULL)
	{
		widgSetButtonFlash(psWScreen,button);
	}
	return TRUE;
}


// -----------------------------------------------------------------------------------------
// stop a generic button flashing
BOOL scrFlashOff(void)
{
	SDWORD		button;

	if (!stackPopParams(1, VAL_INT, &button))
	{
		return FALSE;
	}
#ifdef WIN32
	if (button >= IDRET_OPTIONS && button <= IDRET_CANCEL)
	{
		stopReticuleButtonFlash((UDWORD)button);
		return TRUE;
	}
#endif

	if(widgGetFromID(psWScreen,button) != NULL)
	{
		widgClearButtonFlash(psWScreen,button);
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set the initial power level settings for a player
BOOL scrSetPowerLevel(void)
{
	SDWORD		player, power;

	if (!stackPopParams(2, VAL_INT, &power, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetPowerLevel:player number is too high"));
		return FALSE;
	}
	
	setPlayerPower(power, player);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//add some power for a player
BOOL scrAddPower(void)
{
	SDWORD		player, power;

	if (!stackPopParams(2, VAL_INT, &power, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddPower:player number is too high"));
		return FALSE;
	}
	
	addPower(player, power);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
/*set the landing Zone position for the map - this is for player 0. Can be 
scrapped and replaced by setNoGoAreas, left in for compatibility*/
BOOL scrSetLandingZone(void)
{
	SDWORD		x1, x2, y1, y2;

	if (!stackPopParams(4, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	//check the values - check against max possible since can set in one mission for the next
	//if (x1 > (SDWORD)mapWidth)
	if (x1 > (SDWORD)MAP_MAXWIDTH)
	{
		ASSERT((FALSE, "scrSetLandingZone: x1 is greater than max mapWidth"));
		return FALSE;
	}
	//if (x2 > (SDWORD)mapWidth)
	if (x2 > (SDWORD)MAP_MAXWIDTH)
	{
		ASSERT((FALSE, "scrSetLandingZone: x2 is greater than max mapWidth"));
		return FALSE;
	}
	//if (y1 > (SDWORD)mapHeight)
	if (y1 > (SDWORD)MAP_MAXHEIGHT)
	{
		ASSERT((FALSE, "scrSetLandingZone: y1 is greater than max mapHeight"));
		return FALSE;
	}
	//if (y2 > (SDWORD)mapHeight)
	if (y2 > (SDWORD)MAP_MAXHEIGHT)
	{
		ASSERT((FALSE, "scrSetLandingZone: y2 is greater than max mapHeight"));
		return FALSE;
	}
	//check won't overflow!
	if (x1 > UBYTE_MAX OR y1 > UBYTE_MAX OR x2 > UBYTE_MAX OR y2 > UBYTE_MAX)
	{
		ASSERT((FALSE, "scrSetLandingZone: one coord is greater than %s", UBYTE_MAX));
		return FALSE;
	}

	setLandingZone((UBYTE)x1, (UBYTE)y1, (UBYTE)x2, (UBYTE)y2);

	return TRUE;
}

/*set the landing Zone position for the Limbo droids and adds the Limbo droids 
to the world at the location*/
BOOL scrSetLimboLanding(void)
{
	SDWORD		x1, x2, y1, y2;

	if (!stackPopParams(4, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	//check the values - check against max possible since can set in one mission for the next
	//if (x1 > (SDWORD)mapWidth)
	if (x1 > (SDWORD)MAP_MAXWIDTH)
	{
		ASSERT((FALSE, "scrSetLimboLanding: x1 is greater than max mapWidth"));
		return FALSE;
	}
	//if (x2 > (SDWORD)mapWidth)
	if (x2 > (SDWORD)MAP_MAXWIDTH)
	{
		ASSERT((FALSE, "scrSetLimboLanding: x2 is greater than max mapWidth"));
		return FALSE;
	}
	//if (y1 > (SDWORD)mapHeight)
	if (y1 > (SDWORD)MAP_MAXHEIGHT)
	{
		ASSERT((FALSE, "scrSetLimboLanding: y1 is greater than max mapHeight"));
		return FALSE;
	}
	//if (y2 > (SDWORD)mapHeight)
	if (y2 > (SDWORD)MAP_MAXHEIGHT)
	{
		ASSERT((FALSE, "scrSetLimboLanding: y2 is greater than max mapHeight"));
		return FALSE;
	}
	//check won't overflow!
	if (x1 > UBYTE_MAX OR y1 > UBYTE_MAX OR x2 > UBYTE_MAX OR y2 > UBYTE_MAX)
	{
		ASSERT((FALSE, "scrSetLimboLanding: one coord is greater than %s", UBYTE_MAX));
		return FALSE;
	}

	setNoGoArea((UBYTE)x1, (UBYTE)y1, (UBYTE)x2, (UBYTE)y2, LIMBO_LANDING);

    //this calls the Droids from the Limbo list onto the map
    placeLimboDroids();

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//initialises all the no go areas
BOOL scrInitAllNoGoAreas(void)
{
	initNoGoAreas();

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set a no go area for the map - landing zones for the enemy, or player 0
BOOL scrSetNoGoArea(void)
{
	SDWORD		x1, x2, y1, y2, area;

	if (!stackPopParams(5, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2, 
		VAL_INT, &area))
	{
		return FALSE;
	}

    if (area == LIMBO_LANDING)
    {
        ASSERT((FALSE, "scrSetNoGoArea: Cannot set the Limbo Landing area with this function"));
        return FALSE;
    }

	//check the values - check against max possible since can set in one mission for the next
	//if (x1 > (SDWORD)mapWidth)
	if (x1 > (SDWORD)MAP_MAXWIDTH)
	{
		ASSERT((FALSE, "scrSetNoGoArea: x1 is greater than max mapWidth"));
		return FALSE;
	}
	//if (x2 > (SDWORD)mapWidth)
	if (x2 > (SDWORD)MAP_MAXWIDTH)
	{
		ASSERT((FALSE, "scrSetNoGoArea: x2 is greater than max mapWidth"));
		return FALSE;
	}
	//if (y1 > (SDWORD)mapHeight)
	if (y1 > (SDWORD)MAP_MAXHEIGHT)
	{
		ASSERT((FALSE, "scrSetNoGoArea: y1 is greater than max mapHeight"));
		return FALSE;
	}
	//if (y2 > (SDWORD)mapHeight)
	if (y2 > (SDWORD)MAP_MAXHEIGHT)
	{
		ASSERT((FALSE, "scrSetNoGoArea: y2 is greater than max mapHeight"));
		return FALSE;
	}
	//check won't overflow!
	if (x1 > UBYTE_MAX OR y1 > UBYTE_MAX OR x2 > UBYTE_MAX OR y2 > UBYTE_MAX)
	{
		ASSERT((FALSE, "scrSetNoGoArea: one coord is greater than %s", UBYTE_MAX));
		return FALSE;
	}

	if (area >= MAX_NOGO_AREAS)
	{
		ASSERT((FALSE, "scrSetNoGoArea: max num of areas is %d", MAX_NOGO_AREAS));
		return FALSE;
	}

	setNoGoArea((UBYTE)x1, (UBYTE)y1, (UBYTE)x2, (UBYTE)y2, (UBYTE)area);

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// set the zoom level for the radar
BOOL scrSetRadarZoom(void)
{
	SDWORD	level;

	if (!stackPopParams(1, VAL_INT, &level))
	{
		return TRUE;
	}

	// MAX_RADARZOOM is different on PC and PSX
	if (level < 0 || level > 2)
	{
		ASSERT((FALSE, "scrSetRadarZoom: zoom level out of range"));
		return FALSE;
	}

#ifdef PSX
	if (level == 2)
	{
		level = 1;
	}
#endif

	SetRadarZoom((UWORD)level);

	return TRUE;
}

// -----------------------------------------------------------------------------------------
//set how long an offworld mission can last -1 = no limit
BOOL scrSetMissionTime(void)
{
	SDWORD		time;

	if (!stackPopParams(1, VAL_INT, &time))
	{
		return FALSE;
	}

	time *= 100;

	//check not more than one hour - the mission timers cannot cope at present! - (visually)
	//if (time > 60*60*GAME_TICKS_PER_SEC)
	//check not more than 99 mins - the mission timers cannot cope at present! - (visually)
    //we're allowing up to 5 hours now!
    if (time > 5*60*60*GAME_TICKS_PER_SEC)
	{
		ASSERT((FALSE,"The mission timer cannot be set to more than 99!"));
		time = -1;
	}
	//store the value
	mission.time = time;
#ifdef WIN32		// ffs ab    ... but shouldn't this be on the psx ?
    setMissionCountDown();
#endif

	//add the timer to the interface
	if (mission.time >= 0)
	{
		mission.startTime = gameTime;
		addMissionTimerInterface();
	}
    else
    {
        //make sure its not up if setting to -1
        intRemoveMissionTimer();
        //make sure the cheat time is not set
        mission.cheatTime = 0;
    }

	return TRUE;
}

// this returns how long is left for the current mission time is 1/100th sec - same units as passed in
BOOL scrMissionTimeRemaining(void)
{
    SDWORD      timeRemaining;

	timeRemaining = mission.time - (gameTime - mission.startTime);

    if (timeRemaining < 0)
    {
        timeRemaining = 0;
    }
    else
    {
        timeRemaining /= 100;
    }

	if(!stackPushResult(VAL_INT, timeRemaining))
	{
		return(FALSE);
	}
	return(TRUE);
}

// -----------------------------------------------------------------------------------------
//set the time delay for reinforcements for an offworld mission
BOOL scrSetReinforcementTime(void)
{
	SDWORD		time;
    DROID       *psDroid;

	if (!stackPopParams(1, VAL_INT, &time))
	{
		return FALSE;
	}

    time *= 100;

	//check not more than one hour - the mission timers cannot cope at present!
	if (time != LZ_COMPROMISED_TIME AND time > 60*60*GAME_TICKS_PER_SEC)
	{
		ASSERT((FALSE,"The transport timer cannot be set to more than 1 hour!"));
		time = -1;
	}

    //not interseted in this check any more -  AB 28/01/99
    //quick check of the value - don't check if time has not been set
	/*if (mission.time > 0 AND time != LZ_COMPROMISED_TIME AND time > mission.time)
	{
		DBMB(("scrSetReinforcementTime: reinforcement time greater than mission time!"));
	}*/
	//store the value
	mission.ETA = time;

	//if offworld or campaign change mission, then add the timer
	//if (mission.type == LDS_MKEEP OR mission.type == LDS_MCLEAR OR 
    //    mission.type == LDS_CAMCHANGE)
    if (missionCanReEnforce())
	{
		addTransporterTimerInterface();
	}

    //make sure the timer is not there if the reinforcement time has been set to < 0
    if (time < 0)
    {
#ifdef WIN32
        intRemoveTransporterTimer();
#endif
        /*only remove the launch if haven't got a transporter droid since the 
        scripts set the time to -1 at the between stage if there are not going 
        to be reinforcements on the submap  */
        for (psDroid = apsDroidLists[selectedPlayer]; psDroid != NULL; psDroid = 
            psDroid->psNext)
        {
            if (psDroid->droidType == DROID_TRANSPORTER)
            {
                break;
            }
        }
        //if not found a transporter, can remove the launch button
        if (psDroid ==  NULL)
        {
            intRemoveTransporterLaunch();
        }
    }

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets all structure limits for a player to a specified value
BOOL scrSetAllStructureLimits(void)
{
	SDWORD				player, limit;
	STRUCTURE_LIMITS	*psStructLimits;
	UDWORD				i;

	if (!stackPopParams(2, VAL_INT, &limit, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetStructureLimits:player number is too high"));
		return FALSE;
	}

	if (limit < 0)
	{
		ASSERT((FALSE, "scrSetStructureLimits: limit is less than zero - %d", limit));
		return FALSE;
	}

	if (limit > LOTS_OF)
	{
		ASSERT((FALSE, "scrSetStructureLimits: limit is too high - %d - must be less than %d",
			limit, LOTS_OF));
		return FALSE;
	}

	//set all the limits to the value specified
	psStructLimits = asStructLimits[player];
	for (i = 0; i < numStructureStats; i++)
	{
		psStructLimits[i].limit = (UBYTE)limit;
#ifdef WIN32
		psStructLimits[i].globalLimit = (UBYTE)limit;
#endif
	}

	return TRUE;
}


// -----------------------------------------------------------------------------------------
// clear all the console messages
BOOL scrFlushConsoleMessages(void)
{
	flushConsoleMessages();

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Establishes the distance between two points - uses an approximation
BOOL scrDistanceTwoPts( void )
{
SDWORD	x1,y1,x2,y2;
SDWORD	retVal;


	if(!stackPopParams(4,VAL_INT,&x1,VAL_INT,&y1,VAL_INT,&x2,VAL_INT,&y2))
	{
		ASSERT((FALSE,"SCRIPT : Distance between two points - cannot get parameters"));
		return(FALSE);
	}

	/* Approximate the distance */
	retVal = dirtySqrt(x1,y1,x2,y2);

	if(!stackPushResult(VAL_INT,retVal))
	{
		ASSERT((FALSE,"SCRIPT : Distance between two points - cannot return result"));
		return(FALSE);
	}
	return(TRUE);
}

// -----------------------------------------------------------------------------------------
// Returns whether two objects can see each other
BOOL	scrLOSTwoBaseObjects( void )
{
BASE_OBJECT	*psSource,*psDest;
BOOL		bWallsBlock;
BOOL		retVal;

	if(!stackPopParams(3,ST_BASEOBJECT,&psSource,ST_BASEOBJECT,&psDest,VAL_BOOL,&bWallsBlock))
	{
		ASSERT((FALSE,"SCRIPT : scrLOSTwoBaseObjects - cannot get parameters"));
		return(FALSE);
	}

	if(bWallsBlock)
	{
		retVal = visibleObjWallBlock(psSource,psDest);
	}
	else
	{
		retVal = visibleObject(psSource,psDest);
	}

	if(!stackPushResult(VAL_BOOL,retVal))
	{
		ASSERT((FALSE,"SCRIPT : scrLOSTwoBaseObjects - cannot return result"));
		return(FALSE);
	}
	return(TRUE);
}

// -----------------------------------------------------------------------------------------
// Destroys all structures within a certain bounding area.
BOOL	scrDestroyStructuresInArea( void )
{
SDWORD		x1,y1,x2,y2;
UDWORD		typeRef;
UDWORD		player;
STRUCTURE	*psStructure,*psNextS;
FEATURE		*psFeature,*psNextF;
BOOL		bVisible,bTakeFeatures;
SDWORD		sX,sY;

	if(!stackPopParams(8, VAL_INT, &player, VAL_INT, &typeRef, VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, 
						VAL_INT, &y2, VAL_BOOL, &bVisible, VAL_BOOL, &bTakeFeatures))
	{
		ASSERT((FALSE,"SCRIPT : scrDestroyStructuresInArea - Cannot get parameters"));
		return(FALSE);
	}

	if(player>=MAX_PLAYERS)
	{
		ASSERT((FALSE,"Player number too high in scrDestroyStructuresInArea"));
	}
  
	for(psStructure = apsStructLists[player]; psStructure; psStructure = psNextS)
	{
		/* Keep a copy */
		psNextS = psStructure->psNext;

		sX = psStructure->x;
		sY = psStructure->y;

		if(psStructure->pStructureType->type == typeRef)
		{
			if(sX >= x1 AND sX <=x2 AND sY >= y1 AND sY <= y2)
			{
				if(bVisible)
				{
					destroyStruct(psStructure);
				}
				else
				{
					removeStruct(psStructure, TRUE);
				}	
			}
		}
	}

	if(bTakeFeatures)
	{
		for(psFeature = apsFeatureLists[0]; psFeature; psFeature = psNextF)
		{
			/* Keep a copy */
			psNextF = psFeature->psNext;

			sX = psFeature->x;
			sY = psFeature->y;

		  	if( psFeature->psStats->subType == FEAT_BUILDING) 
		  //		(psFeature->psStats->subType != FEAT_OIL_DRUM) AND
		  //		(psFeature->psStats->subType != FEAT_OIL_RESOURCE) )

			{
				if(sX >= x1 AND sX <=x2 AND sY >= y1 AND sY <= y2)
				{
					if(bVisible)
					{
						destroyFeature(psFeature);
					}
					else
					{
						removeFeature(psFeature);
					}	
				}
			}
		}
	}
	return(TRUE);
}
// -----------------------------------------------------------------------------------------
// Returns a value representing the threat from droids in a given area
BOOL	scrThreatInArea( void )
{
SDWORD	x1,y1,x2,y2;
SDWORD	ldThreat,mdThreat,hdThreat;
UDWORD	playerLooking,playerTarget;
SDWORD	totalThreat;
DROID	*psDroid;
SDWORD	dX,dY;
BOOL	bVisible;
	
	if(!stackPopParams(10,VAL_INT,&playerLooking,VAL_INT,&playerTarget,VAL_INT,&x1,VAL_INT,&y1,VAL_INT,&x2,VAL_INT,&y2,
		VAL_INT,&ldThreat,VAL_INT,&mdThreat,VAL_INT,&hdThreat, VAL_BOOL, &bVisible))
	{
		ASSERT((FALSE,"SCRIPT : scrThreatInArea - Cannot get parameters"));
		return(FALSE);
	}

	if(playerLooking>=MAX_PLAYERS OR playerTarget >= MAX_PLAYERS)
	{
		ASSERT((FALSE,"Player number too high in scrThreatInArea"));
		return(FALSE);
	}

	totalThreat = 0;

	for(psDroid = apsDroidLists[playerTarget]; psDroid; psDroid = psDroid->psNext)
	{
		if (psDroid->droidType != DROID_WEAPON AND
			psDroid->droidType != DROID_PERSON AND
			psDroid->droidType != DROID_CYBORG AND
			psDroid->droidType != DROID_CYBORG_SUPER)
		{
			continue;
		}

		dX = psDroid->x;
		dY = psDroid->y;
		/* Do we care if the droid is visible or not */
		if(bVisible ? psDroid->visible[playerLooking] : TRUE)
		{
			/* Have we found a droid in this area */
			if(dX >= x1 AND dX <=x2 AND dY >= y1 AND dY <= y2)
			{
				switch ((asBodyStats + psDroid->asBits[COMP_BODY].nStat)->size)
				{	
				case SIZE_LIGHT:
					totalThreat += ldThreat;
					break;
				case SIZE_MEDIUM:
					totalThreat += mdThreat;
					break;
				case SIZE_HEAVY:
				case SIZE_SUPER_HEAVY:
					totalThreat += hdThreat;
					break;
				default:
					ASSERT((FALSE, "Weird droid size in threat assessment"));
					break;
				}
			}
		}
	}
//	DBPRINTF(("scrThreatInArea: returning %d\n", totalThreat));
	if(!stackPushResult(VAL_INT,totalThreat))
	{
		ASSERT((FALSE,"SCRIPT : Cannot push result in scrThreatInArea"));
		return(FALSE);
	}
	return(TRUE);
}
// -----------------------------------------------------------------------------------------
// returns the nearest gateway bottleneck to a specified point
BOOL scrGetNearestGateway( void )
{
SDWORD	x,y;
SDWORD	gX,gY;
UDWORD	nearestSoFar;
UDWORD	dist;
GATEWAY	*psGateway;
SDWORD	retX,retY;
SDWORD	*rX,*rY;
BOOL	success;

	if(!stackPopParams(4, VAL_INT, &x, VAL_INT, &y, VAL_REF|VAL_INT, &rX, VAL_REF|VAL_INT, &rY))
	{
		ASSERT((FALSE,"SCRIPT : Cannot get parameters for scrGetNearestGateway"));
		return(FALSE);
	}

	if(x<0 OR x>(SDWORD)mapWidth OR y<0 OR y>(SDWORD)mapHeight)
	{
		ASSERT((FALSE,"SCRIPT : Invalid coordinates in getNearestGateway"));
		return(FALSE);
	}

	if(psGateways == NULL)
	{
		ASSERT((FALSE,"SCRIPT : No gateways found in getNearestGatway"));
		return(FALSE);
	}

	nearestSoFar = UDWORD_MAX;
	retX = retY = 0;
	success = FALSE;
	for(psGateway = psGateways; psGateway; psGateway = psGateway->psNext)
	{
		/* Get gateway midpoint */
		gX = (psGateway->x1 + psGateway->x2)/2;
		gY = (psGateway->y1 + psGateway->y2)/2;

		/* Estimate the distance to it */
		dist = dirtySqrt(x,y,gX,gY);

		/* Is it best we've found? */
		if(dist<nearestSoFar)
		{
			success = TRUE;
			/* Yes, then keep a record of it */
			nearestSoFar = dist;
			retX = gX;
			retY = gY;
		}
	}

	*rX = retX;
	*rY = retY;
	
	if(!stackPushResult(VAL_BOOL,success))
	{
		ASSERT((FALSE,"SCRIPT : Cannot return result for stackPushResult"));
		return(FALSE);
	}


	return(TRUE);
}
// -----------------------------------------------------------------------------------------
BOOL	scrSetWaterTile(void)
{
UDWORD	tileNum;

	if(!stackPopParams(1,VAL_INT, &tileNum))
	{
		ASSERT((FALSE,"SCRIPT : Cannot get parameter for scrSetWaterTile"));
		return(FALSE);
	}

#ifdef WIN32
	if(tileNum > 96)
	{
		ASSERT((FALSE,"SCRIPT : Water tile number too high in scrSetWaterTile"));
		return(FALSE);
	}

	setUnderwaterTile(tileNum);
#endif
	return(TRUE);
}
// -----------------------------------------------------------------------------------------
BOOL	scrSetRubbleTile(void)
{
UDWORD	tileNum;

	if(!stackPopParams(1,VAL_INT, &tileNum))
	{
		ASSERT((FALSE,"SCRIPT : Cannot get parameter for scrSetRubbleTile"));
		return(FALSE);
	}

#ifdef WIN32
	if(tileNum > 96)
	{
		ASSERT((FALSE,"SCRIPT : Rubble tile number too high in scrSetWaterTile"));
		return(FALSE);
	}

	setRubbleTile(tileNum);
#endif
	return(TRUE);
}
// -----------------------------------------------------------------------------------------
BOOL	scrSetCampaignNumber(void)
{
UDWORD	campaignNumber;

	if(!stackPopParams(1,VAL_INT, &campaignNumber))
	{
		ASSERT((FALSE,"SCRIPT : Cannot get parameter for scrSetCampaignNumber"));
		return(FALSE);
	}

#ifdef WIN32
	setCampaignNumber(campaignNumber);
#endif
	return(TRUE);
}
// -----------------------------------------------------------------------------------------
#ifdef WIN32
BOOL	scrGetUnitCount( void )
{
	return TRUE;
}

#endif
// -----------------------------------------------------------------------------------------
// Tests whether a structure has a certain module for a player. Tests whether any structure
// has this module if structure is null
BOOL	scrTestStructureModule(void)
{
SDWORD	player,refId;
STRUCTURE	*psStructure,*psStruct;
BOOL	bFound;

	if(!stackPopParams(3,VAL_INT,&player,ST_STRUCTURE,&psStructure,VAL_INT,&refId))
	{
		ASSERT((FALSE,"SCRIPT : Cannot get parameters in scrTestStructureModule"));
		return(FALSE);
	}

	if(player>=MAX_PLAYERS)
	{
		ASSERT((FALSE,"SCRIPT : Player number too high in scrTestStructureModule"));
		return(FALSE);

	}

	/* Nothing yet */
	bFound = FALSE;

	/* Check the specified case first */
	if(psStructure)
	{
		if(structHasModule(psStructure))
		{
			bFound = TRUE;
		}
	}
	/* psStructure was NULL - so test the general case */
	else
	{
		// Search them all, but exit if we get one!!
		for(psStruct = apsStructLists[player],bFound = FALSE; 
			psStruct AND !bFound; psStruct = psStruct->psNext)
		{
			if(structHasModule(psStruct))
			{
				bFound = TRUE;
			}
		}
	}

	/* Send back the result */
	if(!stackPushResult(VAL_BOOL,bFound))
	{
		ASSERT((FALSE,"SCRIPT : Cannot push result for scrTestStructureModule"));
		return(FALSE);
	}

	return(TRUE);
}


// -----------------------------------------------------------------------------------------
BOOL	scrForceDamage( void )
{
DROID		*psDroid;
STRUCTURE	*psStructure;
FEATURE		*psFeature;
BASE_OBJECT	*psObj;
UDWORD		damagePercent;
FRACT		divisor;
UDWORD		newVal;

	/* OK - let's get the vars */
	if(!stackPopParams(2,ST_BASEOBJECT,&psObj,VAL_INT,&damagePercent))
	{
		ASSERT((FALSE,"Cannot pop params for scrForceDamage"));
		return(FALSE);
	}

	/* Got to be a percent, so must be less than or equal to 100 */
	if(damagePercent > 100)
	{
		ASSERT((FALSE,"scrForceDamage : You're supposed to be passing in a PERCENTAGE VALUE, \
			instead I got given %d, which is clearly no good, now is it!?"));
		return(FALSE);
	}

	/* Get percentage in range [0.1] */
	divisor =  MAKEFRACT(damagePercent) / 100;

	/* See what we're dealing with */
	switch(psObj->type)
	{
	case OBJ_DROID:
		psDroid = (DROID *) psObj;
		newVal = MAKEINT((divisor*psDroid->originalBody));
		psDroid->body = newVal;
		break;
	case OBJ_STRUCTURE:
		psStructure = (STRUCTURE *) psObj;
		newVal = MAKEINT((divisor*structureBody(psStructure)));
		psStructure->body = (UWORD)newVal;
		break;
	case OBJ_FEATURE:
		psFeature = (FEATURE *) psObj;
		/* Some features cannot be damaged */
		if(psFeature->psStats->damageable)
		{
			newVal = MAKEINT((divisor*psFeature->psStats->body));
			psFeature->body = newVal;
		}
		break;
	default:
		ASSERT((FALSE,"Unsupported base object type in scrForceDamage"));
		return(FALSE);
		break;
	}

	return(TRUE);

}
// Kills of a droid without spawning any explosion effects.
// -----------------------------------------------------------------------------------------
BOOL	scrDestroyUnitsInArea( void )
{
DROID	*psDroid,*psNext;
SDWORD	x1,y1,x2,y2;
UDWORD	player;
UDWORD	count=0;

	if(!stackPopParams(5,VAL_INT,&x1,VAL_INT,&y1,VAL_INT,&x2,VAL_INT,&y2,VAL_INT, &player))
	{
		ASSERT((FALSE,"Cannot get params for scrDestroyUnitsInArea"));
		return(FALSE);
	}

	if(player>=MAX_PLAYERS)
	{
		ASSERT((FALSE,"Invalid player number in scrKillDroidsInArea"));
	}

	for(psDroid = apsDroidLists[player]; psDroid; psDroid = psNext)
	{
		psNext = psDroid->psNext;	// get a copy cos pointer will be lost
		if( (psDroid->x > x1) AND (psDroid->x < x2) AND
			(psDroid->y > y1) AND (psDroid->y < y2) )
		{
			/* then it's inside the area */
			destroyDroid(psDroid);
			count++;
		}
	}

	if(!stackPushResult(VAL_INT,count))
	{
		return(FALSE);
	}

	return(TRUE);
}
// -----------------------------------------------------------------------------------------
BOOL	scrRemoveDroid( void )
{
DROID	*psDroid;
	
	if(!stackPopParams(1,ST_DROID,&psDroid))
	{		ASSERT((FALSE,"Cannot get vars for scrRemoveDroid!"));
		return(FALSE);
	}

	if(psDroid)
	{
		vanishDroid(psDroid);
	}

	return(TRUE);
}
// -----------------------------------------------------------------------------------------
BOOL	structHasModule(STRUCTURE *psStruct)
{
STRUCTURE_STATS	*psStats;
BOOL			bFound;

	/* Fail if the structure isn't built yet */   
	if(psStruct->status != SS_BUILT)
	{
		return(FALSE);
	}

	/* Not found yet */
	bFound = FALSE;

	
	if(psStruct==NULL)
	{
		ASSERT((psStruct!=NULL,"structHasModule - Testing for a module from a NULL struct - huh!?"));
		return(FALSE);
	}

	if(psStruct)
	{
		/* Grab a stats pointer */
		psStats = psStruct->pStructureType;
		if(StructIsFactory(psStruct) 
			OR psStats->type == REF_POWER_GEN OR psStats->type == REF_RESEARCH)
		{
			switch(psStats->type)
			{
				case REF_POWER_GEN:
					if (((POWER_GEN *)psStruct->pFunctionality)->capacity)
					{
						bFound = TRUE;
					}
					break;
				case REF_FACTORY:
				case REF_VTOL_FACTORY:
					if (((FACTORY *)psStruct->pFunctionality)->capacity)
					{
						bFound = TRUE;
					}
					break;
				case REF_RESEARCH:
					if (((RESEARCH_FACILITY *)psStruct->pFunctionality)->capacity)
					   
					{
						bFound = TRUE;
					}
					break;
				default:
					//no other structures can have modules attached
					break;
			}
		}
		else
		{
			/* Wrong type of building - cannot have a module */
			bFound = FALSE;
		}

	}
	return(bFound);
}

// -----------------------------------------------------------------------------------------
// give player a template belonging to another.
BOOL scrAddTemplate(void)
{
	DROID_TEMPLATE *psTemplate;
	UDWORD			player;
	
	if (!stackPopParams(2, ST_TEMPLATE, &psTemplate, VAL_INT, &player))
	{
		return FALSE;
	}
#ifdef PSX
	ASSERT((FALSE,"ScrAddTemplate: Not on PSX"));
	stackPushResult(VAL_BOOL,FALSE);
#else
	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrAddTemplate:player number is too high"));
		return FALSE;
	}

	ASSERT((PTRVALID(psTemplate, sizeof(DROID_TEMPLATE)),"scrAddTemplate: Invalid template pointer"));

	if(	addTemplate(player,psTemplate)) 
	{
		if (!stackPushResult(VAL_BOOL,TRUE))
		{
			return FALSE;
		}
	}
	else
	{
		if (!stackPushResult(VAL_BOOL,FALSE))
		{
			return FALSE;
		}
	}
#endif
	return TRUE;
}



// -----------------------------------------------------------------------------------------

// additional structure check
BOOL structDoubleCheck(BASE_STATS *psStat,UDWORD xx,UDWORD yy)
{
	UDWORD x,y,xTL,yTL,xBR,yBR;
	UBYTE count =0;
	
	STRUCTURE_STATS *psBuilding = (STRUCTURE_STATS *)psStat;

	xTL = xx-1;
	yTL = yy-1;
	xBR = (xx + psBuilding->baseWidth );
	yBR = (yy + psBuilding->baseBreadth );
	// can you get past it?

	y = yTL;	// top
	for(x = xTL;x!=xBR+1;x++)
	{
		if(fpathGroundBlockingTile(x,y))
		{
			count++;
			break;
		}}

	y = yBR;	// bottom
	for(x = xTL;x!=xBR+1;x++)
	{
		if(fpathGroundBlockingTile(x,y))
		{
			count++;
			break;
		}}
	
	x = xTL;	// left
	for(y = yTL+1; y!=yBR; y++)
	{
		if(fpathGroundBlockingTile(x,y))
		{
			count++;
			break;
		}}

	x = xBR;	// right
	for(y = yTL+1; y!=yBR; y++)
	{
		if(fpathGroundBlockingTile(x,y))
		{
			count++;
			break;
		}}

	if(count <2)//no more than one blocking side.
	{
		return TRUE;
	}
	return FALSE;

}

// pick a structure location(only used in skirmish game at 27Aug) ajl.
BOOL scrPickStructLocation(void)
{
	SDWORD			*pX,*pY;
	SDWORD			index;
	STRUCTURE_STATS	*psStat;
	UDWORD			numIterations = 30;
	BOOL			found = FALSE;
	UDWORD			startX, startY, incX, incY;
	SDWORD			x=0, y=0;
	UDWORD			player;

	if (!stackPopParams(4, ST_STRUCTURESTAT, &index, VAL_REF|VAL_INT, &pX ,
        VAL_REF|VAL_INT, &pY, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrPickStructLocation:player number is too high"));
		return FALSE;
	}

    // check for wacky coords.
	if(		*pX < 0 
		||	*pX > (SDWORD)(mapWidth<<TILE_SHIFT)
		||	*pY < 0
		||	*pY > (SDWORD)(mapHeight<<TILE_SHIFT)
	  )
	{
		goto failedstructloc;
	}

	psStat = &asStructureStats[index];			// get stat.
	startX = *pX >> TILE_SHIFT;					// change to tile coords.
	startY = *pY >> TILE_SHIFT;

	for (incX = 1, incY = 1; incX < numIterations; incX++, incY++)
	{
		if (!found){			//top
			y = startY - incY;
			for(x = startX - incX; x < (SDWORD)(startX + incX); x++){
				if ( validLocation((BASE_STATS*)psStat, x, y, player, FALSE)
					 && structDoubleCheck((BASE_STATS*)psStat,x,y)
					){
					found = TRUE;
					break;
				}}}

		if (!found)	{			//right
			x = startX + incX;
			for(y = startY - incY; y < (SDWORD)(startY + incY); y++){
				if(validLocation((BASE_STATS*)psStat, x, y, player, FALSE)
					 && structDoubleCheck((BASE_STATS*)psStat,x,y)
					){
					found = TRUE;
					break;
				}}}

		if (!found){			//bot
			y = startY + incY;
			for(x = startX + incX; x > (SDWORD)(startX - incX); x--){
				if(validLocation((BASE_STATS*)psStat, x, y, player, FALSE)
					 && structDoubleCheck((BASE_STATS*)psStat,x,y)
					 ){
					found = TRUE;
					break;
				}}}

		if (!found){			//left
			x = startX - incX;
			for(y = startY + incY; y > (SDWORD)(startY - incY); y--){
				if(validLocation((BASE_STATS*)psStat, x, y, player, FALSE)
					 && structDoubleCheck((BASE_STATS*)psStat,x,y)
					 ){
					found = TRUE;
					break;
				}}}

		if (found)
		{
			break;
		}
	}

	if(found)	// did It!
	{
		// back to world coords.
		*pX = (x << TILE_SHIFT) + (psStat->baseWidth * (TILE_UNITS/2));
		*pY = (y << TILE_SHIFT) + (psStat->baseBreadth * (TILE_UNITS/2));

		if (!stackPushResult(VAL_BOOL, TRUE))		// success!
		{
			return FALSE;
		}

		return TRUE;
	}
	else
	{
failedstructloc:
		if (!stackPushResult(VAL_BOOL,FALSE))		// failed!
		{
			return FALSE;
		}
	}
	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Sets the transporter entry and exit points for the map
BOOL scrSetTransporterExit(void)
{
	SDWORD	iPlayer, iExitTileX, iExitTileY;

	if (!stackPopParams(3, VAL_INT, &iPlayer, VAL_INT, &iExitTileX, VAL_INT, &iExitTileY))
	{
		return FALSE;
	}

	missionSetTransporterExit( iPlayer, iExitTileX, iExitTileY );

	return TRUE;
}

// -----------------------------------------------------------------------------------------
// Fly transporters in at start of map
BOOL scrFlyTransporterIn(void)
{
	SDWORD	iPlayer, iEntryTileX, iEntryTileY;
	BOOL	bTrackTransporter;

	if (!stackPopParams(4, VAL_INT, &iPlayer, VAL_INT, &iEntryTileX, VAL_INT, &iEntryTileY,
							VAL_BOOL, &bTrackTransporter))
	{
		return FALSE;
	}

	missionSetTransporterEntry( iPlayer, iEntryTileX, iEntryTileY );
	missionFlyTransportersIn( iPlayer, bTrackTransporter );

	return TRUE;
}

// -----------------------------------------------------------------------------------------



/*
 ** scrGetGameStatus
 *
 *  FILENAME: C:\Deliverance\SrcPSX\ScriptFuncs.c
 *
 *  PARAMETERS: The parameter passed must be one of the STATUS_ variable 
 *
 *  DESCRIPTION: Returns various BOOL options in the game	e.g. If the reticule is open 
 *      - You should use the externed variable intMode for other game mode options 
 *        e.g. in the intelligence screen or desgin screen)
 *
 *  RETURNS:
 *
 */
BOOL scrGetGameStatus(void)
{
	SDWORD GameChoice;
	BOOL result;

	if (!stackPopParams(1, VAL_INT, &GameChoice))
	{
		return FALSE;
	}

//	DBPRINTF(("getgamestatus choice=%d\n",GameChoice));

	result=FALSE;		// the default result is false

	switch (GameChoice)
	{

		case STATUS_ReticuleIsOpen:
			if(widgGetFromID(psWScreen,IDRET_FORM) != NULL) result=TRUE;
			break;

		case STATUS_BattleMapViewEnabled:
//			if (driveTacticalActive()==TRUE) result=TRUE;
#ifdef PSX
			if (driveWasDriving()==TRUE) result=TRUE;
#endif 

			if (result==TRUE)
			{
				DBPRINTF(("battle map active"));
			}
			else
			{
				DBPRINTF(("battle map notactive"));
			}


			break;
		case STATUS_DeliveryReposInProgress:
			if (DeliveryReposValid()==TRUE) result=TRUE;
			break;

		default:
		ASSERT((FALSE,"ScrGetGameStatus. Invalid STATUS_ variable"));
		break;
	}

	if (!stackPushResult(VAL_BOOL, result))
	{
		return FALSE;
	}
	return TRUE;
}

//get the colour number used by a player
BOOL scrGetPlayerColour(void)
{
	SDWORD		player, colour;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrGetPlayerColour:player number is too high"));
		return FALSE;
	}
	
    colour = (SDWORD)getPlayerColour(player);

	if (!stackPushResult(VAL_INT, colour))
	{
		return FALSE;
	}

	return TRUE;
}

//set the colour number to use for a player
BOOL scrSetPlayerColour(void)
{
	SDWORD		player, colour;

	if (!stackPopParams(2, VAL_INT, &colour, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetPlayerColour:player number is too high"));
		return FALSE;
	}
	
	if (colour >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetPlayerColour:colour number is too high"));
		return FALSE;
	}

    //not the end of the world if this doesn't work so don't check the return code
    (void)setPlayerColour(player, colour);

	return TRUE;
}

//set all droids in an area to belong to a different player - returns the number of droids changed
BOOL scrTakeOverDroidsInArea(void)
{
	SDWORD		fromPlayer, toPlayer, x1, x2, y1, y2, numChanged;
    DROID       *psDroid, *psNext;

	if (!stackPopParams(6, VAL_INT, &fromPlayer, VAL_INT, &toPlayer, 
        VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (fromPlayer >= MAX_PLAYERS OR toPlayer >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea:player number is too high"));
		return FALSE;
	}
	
	if (x1 > (SDWORD)(MAP_MAXWIDTH << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: x1 is greater than max mapWidth"));
		return FALSE;
	}

    if (x2 > (SDWORD)(MAP_MAXWIDTH << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: x2 is greater than max mapWidth"));
		return FALSE;
	}

    if (y1 > (SDWORD)(MAP_MAXHEIGHT << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: y1 is greater than max mapHeight"));
		return FALSE;
	}

    if (y2 > (SDWORD)(MAP_MAXHEIGHT << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: y2 is greater than max mapHeight"));
		return FALSE;
	}

    numChanged = 0;
    for (psDroid = apsDroidLists[fromPlayer]; psDroid != NULL; psDroid = psNext)
    {
        psNext = psDroid->psNext;
        //check if within area specified
        if (psDroid->x >= x1 AND psDroid->x <= x2 AND 
            psDroid->y >= y1 AND psDroid->y <= y2)
        {
            //give the droid away
            if (giftSingleDroid(psDroid, toPlayer))
            {
                numChanged++;
            }
        }
    }

	if (!stackPushResult(VAL_INT, numChanged))
	{
		return FALSE;
	}

    return TRUE;
}

/*this takes over a single droid and passes a pointer back to the new one*/
BOOL scrTakeOverSingleDroid(void)
{
	SDWORD			playerToGain;
    DROID           *psDroidToTake, *psNewDroid;

    if (!stackPopParams(2, ST_DROID, &psDroidToTake, VAL_INT, &playerToGain))
    {
		return FALSE;
    }

	if (playerToGain >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrTakeOverSingleUnit:player number is too high"));
		return FALSE;
	}
    
    if (psDroidToTake == NULL)
    {
        ASSERT((FALSE, "scrTakeOverSingleUnit: Null unit"));
        return FALSE;
    }

	ASSERT((PTRVALID(psDroidToTake, sizeof(DROID)),
		"scrTakeOverSingleUnit: Invalid unit pointer"));

    psNewDroid = giftSingleDroid(psDroidToTake, playerToGain);

	if (!stackPushResult(ST_DROID, (SDWORD)psNewDroid))
	{
		return FALSE;
    }
	return TRUE;
}

// set all droids in an area of a certain experience level or less to belong to
// a different player - returns the number of droids changed
BOOL scrTakeOverDroidsInAreaExp(void)
{
	SDWORD		fromPlayer, toPlayer, x1, x2, y1, y2, numChanged, level, maxUnits;
    DROID       *psDroid, *psNext;

	if (!stackPopParams(8, VAL_INT, &fromPlayer, VAL_INT, &toPlayer, 
        VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2, VAL_INT, &level, VAL_INT, &maxUnits))
	{
		return FALSE;
	}

	if (fromPlayer >= MAX_PLAYERS OR toPlayer >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea:player number is too high"));
		return FALSE;
	}
	
	if (x1 > (SDWORD)(MAP_MAXWIDTH << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: x1 is greater than max mapWidth"));
		return FALSE;
	}

    if (x2 > (SDWORD)(MAP_MAXWIDTH << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: x2 is greater than max mapWidth"));
		return FALSE;
	}

    if (y1 > (SDWORD)(MAP_MAXHEIGHT << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: y1 is greater than max mapHeight"));
		return FALSE;
	}

    if (y2 > (SDWORD)(MAP_MAXHEIGHT << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverUnitsInArea: y2 is greater than max mapHeight"));
		return FALSE;
	}

    numChanged = 0;
    for (psDroid = apsDroidLists[fromPlayer]; psDroid != NULL; psDroid = psNext)
    {
        psNext = psDroid->psNext;
        //check if within area specified
        if ((psDroid->droidType != DROID_CONSTRUCT) &&
			(psDroid->droidType != DROID_REPAIR) &&
            (psDroid->droidType != DROID_CYBORG_CONSTRUCT) &&
            (psDroid->droidType != DROID_CYBORG_REPAIR) &&
//			((SDWORD)getDroidLevel(psDroid) <= level) AND
			((SDWORD)psDroid->numKills <= level) AND
			psDroid->x >= x1 AND psDroid->x <= x2 AND 
            psDroid->y >= y1 AND psDroid->y <= y2)
        {
            //give the droid away
            if (giftSingleDroid(psDroid, toPlayer))
            {
                numChanged++;
            }
        }

		if (numChanged >= maxUnits)
		{
			break;
		}
    }

	if (!stackPushResult(VAL_INT, numChanged))
	{
		return FALSE;
	}

    return TRUE;
}

/*this takes over a single structure and passes a pointer back to the new one*/
BOOL scrTakeOverSingleStructure(void)
{
	SDWORD			playerToGain;
    STRUCTURE       *psStructToTake, *psNewStruct;
    UDWORD          structureInc;

    if (!stackPopParams(2, ST_STRUCTURE, &psStructToTake, VAL_INT, &playerToGain))
    {
		return FALSE;
    }

	if (playerToGain >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrTakeOverSingleStructure:player number is too high"));
		return FALSE;
	}
    
    if (psStructToTake == NULL)
    {
        ASSERT((FALSE, "scrTakeOverSingleStructure: Null structure"));
        return FALSE;
    }

	ASSERT((PTRVALID(psStructToTake, sizeof(STRUCTURE)),
		"scrTakeOverSingleStructure: Invalid structure pointer"));

    structureInc = psStructToTake->pStructureType->ref - REF_STRUCTURE_START;
    if (playerToGain == (SDWORD)selectedPlayer AND StructIsFactory(psStructToTake) AND
        asStructLimits[playerToGain][structureInc].currentQuantity >= MAX_FACTORY)
    {
        DBPRINTF(("scrTakeOverSingleStructure - factory ignored for selectedPlayer\n"));
        psNewStruct = NULL;
    }
    else
    {
        psNewStruct = giftSingleStructure(psStructToTake, (UBYTE)playerToGain, TRUE);
        if (psNewStruct)
        {
            //check the structure limits aren't compromised
            if (asStructLimits[playerToGain][structureInc].currentQuantity > 
                asStructLimits[playerToGain][structureInc].limit)
            {
                asStructLimits[playerToGain][structureInc].limit = asStructLimits[
                    playerToGain][structureInc].currentQuantity;
            }
            //for each structure taken - add graphical effect if the selectedPlayer
            if (playerToGain == (SDWORD)selectedPlayer)
            {
                assignSensorTarget((BASE_OBJECT *)psNewStruct);
            }
        }
    }

	if (!stackPushResult(ST_STRUCTURE, (SDWORD)psNewStruct))
	{
		return FALSE;
    }
	return TRUE;
}

//set all structures in an area to belong to a different player - returns the number of droids changed
//will not work on factories for the selectedPlayer
BOOL scrTakeOverStructsInArea(void)
{
	SDWORD		fromPlayer, toPlayer, x1, x2, y1, y2, numChanged;
    STRUCTURE   *psStruct, *psNext, *psNewStruct;
    UDWORD      structureInc;

	if (!stackPopParams(6, VAL_INT, &fromPlayer, VAL_INT, &toPlayer, 
        VAL_INT, &x1, VAL_INT, &y1, VAL_INT, &x2, VAL_INT, &y2))
	{
		return FALSE;
	}

	if (fromPlayer >= MAX_PLAYERS OR toPlayer >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrTakeOverStructsInArea:player number is too high"));
		return FALSE;
	}
	
	if (x1 > (SDWORD)(MAP_MAXWIDTH << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverStructsInArea: x1 is greater than max mapWidth"));
		return FALSE;
	}

    if (x2 > (SDWORD)(MAP_MAXWIDTH << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverStructsInArea: x2 is greater than max mapWidth"));
		return FALSE;
	}

    if (y1 > (SDWORD)(MAP_MAXHEIGHT << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverStructsInArea: y1 is greater than max mapHeight"));
		return FALSE;
	}

    if (y2 > (SDWORD)(MAP_MAXHEIGHT << TILE_SHIFT))
	{
		ASSERT((FALSE, "scrTakeOverStructsInArea: y2 is greater than max mapHeight"));
		return FALSE;
	}

    numChanged = 0;
    for (psStruct = apsStructLists[fromPlayer]; psStruct != NULL; psStruct = psNext)
    {
        psNext = psStruct->psNext;
        //check if within area specified
        if (psStruct->x >= x1 AND psStruct->x <= x2 AND 
            psStruct->y >= y1 AND psStruct->y <= y2)
        {
            //changed this so allows takeOver is have less than 5 factories
            //don't work on factories for the selectedPlayer
            structureInc = psStruct->pStructureType->ref - REF_STRUCTURE_START;
            if (toPlayer == (SDWORD)selectedPlayer AND StructIsFactory(psStruct) AND
                asStructLimits[toPlayer][structureInc].currentQuantity >= MAX_FACTORY)
            {
                DBPRINTF(("scrTakeOverStructsInArea - factory ignored for selectedPlayer\n"));
            }
            else
            {
                //give the structure away
                psNewStruct = giftSingleStructure(psStruct, (UBYTE)toPlayer, TRUE);
                if (psNewStruct)
                {
                    numChanged++;
                    //check the structure limits aren't compromised
                    //structureInc = psNewStruct->pStructureType->ref - REF_STRUCTURE_START;
                    if (asStructLimits[toPlayer][structureInc].currentQuantity > 
                        asStructLimits[toPlayer][structureInc].limit)
                    {
                        asStructLimits[toPlayer][structureInc].limit = asStructLimits[
                            toPlayer][structureInc].currentQuantity;
                    }
                    //for each structure taken - add graphical effect if the selectedPlayer
                    if (toPlayer == (SDWORD)selectedPlayer)
                    {
                        assignSensorTarget((BASE_OBJECT *)psNewStruct);
                    }
                }
            }
        }
    }

	if (!stackPushResult(VAL_INT, numChanged))
	{
		return FALSE;
	}

    return TRUE;
}

//set Flag for defining what happens to the droids in a Transporter
BOOL scrSetDroidsToSafetyFlag(void)
{
	BOOL bState;

	if (!stackPopParams(1, VAL_BOOL, &bState))
	{
		return FALSE;
	}

    setDroidsToSafetyFlag(bState);

	return TRUE;
}

//set Flag for defining whether the coded countDown is called
BOOL scrSetPlayCountDown(void)
{
	BOOL bState;

	if (!stackPopParams(1, VAL_BOOL, &bState))
	{
		return FALSE;
	}

#ifdef WIN32
    setPlayCountDown((UBYTE)bState);
#endif

	return TRUE;
}

//get the number of droids currently onthe map for a player
BOOL scrGetDroidCount(void)
{
	SDWORD		player;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrGetUnitCount:player number is too high"));
		return FALSE;
	}
	
	if (!stackPushResult(VAL_INT, getNumDroids(player)))
	{
		return FALSE;
	}

	return TRUE;
}


// fire a weapon stat at an object
BOOL scrFireWeaponAtObj(void)
{
	SDWORD			wIndex;
	BASE_OBJECT		*psTarget;
	WEAPON			sWeapon;

	if (!stackPopParams(2, ST_WEAPON, &wIndex, ST_BASEOBJECT, &psTarget))
	{
		return FALSE;
	}

	if (psTarget == NULL)
	{
		ASSERT((FALSE,"scrFireWeaponAtObj: Null target pointer"));
		return FALSE;
	}

	memset(&sWeapon, 0, sizeof(WEAPON));
	sWeapon.nStat = wIndex;

	// send the projectile using the selectedPlayer so that it can always be seen
	proj_SendProjectile(&sWeapon, NULL, selectedPlayer, psTarget->x,psTarget->y,psTarget->z, psTarget, TRUE);

	return TRUE;
}

// fire a weapon stat at a location
BOOL scrFireWeaponAtLoc(void)
{
	SDWORD			wIndex, x,y;
	WEAPON			sWeapon;

	if (!stackPopParams(3, ST_WEAPON, &wIndex, VAL_INT, &x, VAL_INT, &y))
	{
		return FALSE;
	}

	memset(&sWeapon, 0, sizeof(WEAPON));
	sWeapon.nStat = wIndex;

	// send the projectile using the selectedPlayer so that it can always be seen
	proj_SendProjectile(&sWeapon, NULL, selectedPlayer, x,y,map_Height(x,y), NULL, TRUE);

	return TRUE;
}

// set the number of kills for a droid
BOOL scrSetDroidKills(void)
{
	DROID	*psDroid;
	SDWORD	kills;

	if (!stackPopParams(2, ST_DROID, &psDroid, VAL_INT, &kills))
	{
		return TRUE;
	}

	if ((psDroid == NULL) ||
		(psDroid->type != OBJ_DROID))
	{
		ASSERT((FALSE, "scrSetUnitKills: NULL/invalid unit pointer"));
		return FALSE;
	}

	psDroid->numKills = (UWORD)kills;

	return TRUE;
}

// reset the visibility for a player
BOOL scrResetPlayerVisibility(void)
{
	SDWORD			player, i;
	BASE_OBJECT		*psObj;

	if (!stackPopParams(1, VAL_INT, &player))
	{
		return FALSE;
	}

	if (player < 0 || player > MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrResetPlayerVisibility: invalid player"));
		return FALSE;
	}

	for(i=0; i< MAX_PLAYERS; i++)
	{
		if (i == player)
		{
			continue;
		}

		for(psObj = (BASE_OBJECT *)apsDroidLists[i]; psObj; psObj = psObj->psNext)
		{
			psObj->visible[player] = 0;
		}

		for(psObj = (BASE_OBJECT *)apsStructLists[i]; psObj; psObj = psObj->psNext)
		{
			psObj->visible[player] = 0;
		}
	}

	for(psObj = (BASE_OBJECT *)apsFeatureLists[0]; psObj; psObj = psObj->psNext)
	{
		psObj->visible[player] = 0;
	}

	clustResetVisibility(player);

	return TRUE;
}


// set the vtol return pos for a player
BOOL scrSetVTOLReturnPos(void)
{
	SDWORD		player, tx,ty;

	if (!stackPopParams(3, VAL_INT, &player, VAL_INT, &tx, VAL_INT, &ty))
	{
		return FALSE;
	}

	if (player < 0 || player >= MAX_PLAYERS)
	{
		ASSERT((FALSE, "scrSetVTOLReturnPos: invalid player"));
			return FALSE;
	}

	asVTOLReturnPos[player].x = (tx * TILE_UNITS) + TILE_UNITS/2;
	asVTOLReturnPos[player].y = (ty * TILE_UNITS) + TILE_UNITS/2;

	return TRUE;
}

//called via the script in a Limbo Expand level to set the level to plain ol' expand
BOOL scrResetLimboMission(void)
{
    //check currently on a Limbo expand mission
    if (!missionLimboExpand())
    {
        ASSERT((FALSE, "scrResetLimboMission: current mission type invalid"));
        return FALSE;
    }

    //turn it into an expand mission
    resetLimboMission();

    return TRUE;
}


#ifdef WIN32
// skirmish only.
BOOL scrIsVtol(void)
{
	DROID *psDroid;
	BOOL	result;

	if (!stackPopParams(1, ST_DROID, &psDroid))
	{
		return TRUE;
	}

	if(psDroid == NULL)
	{
		ASSERT((FALSE,"scrIsVtol: null droid passed in.")); 
	}

	result = vtolDroid(psDroid) ;
	if (!stackPushResult(VAL_BOOL,result))
	{
		return FALSE;
	}
	return TRUE;
}

#endif

#ifdef WIN32
// do the setting up of the template list for the tutorial.
BOOL scrTutorialTemplates(void)
{
	DROID_TEMPLATE	*psCurr, *psPrev;
	STRING			pName[MAX_NAME_SIZE];

	// find ViperLtMGWheels
	strcpy(pName,"ViperLtMGWheels");
	if (!getResourceName(pName))
	{
		DBERROR(("tutorial template setup failed"));
		return FALSE;
	}

	getDroidResourceName(pName);

	
	for (psCurr = apsDroidTemplates[selectedPlayer],psPrev = NULL; 
			psCurr != NULL; 
			psCurr = psCurr->psNext)
	{
		if (strcmp(pName,psCurr->aName)==0)
		{
			if (psPrev)
			{
				psPrev->psNext = psCurr->psNext;
			}
			else
			{
				apsDroidTemplates[selectedPlayer] = psCurr->psNext;
			}
			//quit looking cos found
			break;
		}
		psPrev = psCurr;
	}
	
	// Delete the template.
	if(psCurr)
	{
		HEAP_FREE(psTemplateHeap, psCurr);
	}
	else
	{
		DBERROR(("tutorial template setup failed"));
		return FALSE;
	}
	return TRUE;
}
#endif

static	UDWORD			playerToEnumDroid;
static	UDWORD			playerVisibleDroid;
static	UDWORD			enumDroidCount;

/* Prepare the droid iteration */
BOOL scrInitEnumDroids(void)
{
	SDWORD	targetplayer,playerVisible;
	
	if ( !stackPopParams(2,  VAL_INT, &targetplayer, VAL_INT, &playerVisible) )
	{
		//DbgMsg("scrInitEnumDroids() - failed to pop params");
		return FALSE;
	}

	playerToEnumDroid	= (UDWORD)targetplayer;
	playerVisibleDroid	= (UDWORD)playerVisible;
	enumDroidCount = 0;		//returned 0 droids so far
	return TRUE;
}

/* Get next droid */
BOOL scrEnumDroid(void)
{
	UDWORD			count;
	DROID		 *psDroid;
	BOOL			found;

	count = 0;
	for(psDroid=apsDroidLists[playerToEnumDroid];psDroid && count<enumDroidCount;count++)
	{
		psDroid = psDroid->psNext;
	}

	
	//search the players' list of droid to see if one exists and is visible
	found = FALSE;
	while(psDroid)
	{
		if(psDroid->visible[playerVisibleDroid])
		{
			if (!stackPushResult(ST_DROID,(UDWORD) psDroid))			//	push result
			{
				return FALSE;
			}

			enumDroidCount++;
			return TRUE;
		}

		enumDroidCount++;
		psDroid = psDroid->psNext;
	}

	// push NULLDROID, since didn't find any
	if (!stackPushResult(ST_DROID, (UDWORD)NULL))
	{
		return FALSE;
	}

	return TRUE;
}