/*
 * Cluster.c
 *
 * Form droids and structures into clusters
 *
 */

// cluster empty printf's
//#define DEBUG_GROUP0
#include "frame.h"
#include "objects.h"
#include "map.h"
#include "cluster.h"
#include "console.h"
#include "hci.h"
#include "gtime.h"
#include "script.h"
#include "scripttabs.h"
#include "scriptcb.h"

// distance between units for them to be in the same cluster
#define CLUSTER_DIST	(TILE_UNITS*8)


// cluster information flags
#define CLUSTER_PLAYER_MASK		0x07
#define CLUSTER_DROID			0x08
#define CLUSTER_STRUCTURE		0x10

// Indirect the cluster ID to an actual cluster number
UBYTE	aClusterMap[CLUSTER_MAX];

// flag to note when a cluster needs the cluster empty callback
UBYTE	aClusterEmpty[CLUSTER_MAX];

// number of droids in a cluster
UWORD	aClusterUsage[CLUSTER_MAX];

// whether a cluster can be seen by a player
UBYTE	aClusterVisibility[CLUSTER_MAX];

// when a cluster was last attacked
UDWORD	aClusterAttacked[CLUSTER_MAX];

// information about the cluster
UBYTE	aClusterInfo[CLUSTER_MAX];

// initialise the cluster system
void clustInitialise(void)
{
	DROID		*psDroid;
	STRUCTURE	*psStruct;
	SDWORD		player;

	ASSERT((CLUSTER_MAX <= UBYTE_MAX,
		"clustInitialse: invalid CLUSTER_MAX, this is a BUILD error"));

	memset(aClusterMap, 0, sizeof(UBYTE) * CLUSTER_MAX);
	memset(aClusterEmpty, 0, sizeof(UBYTE) * CLUSTER_MAX);
	memset(aClusterUsage, 0, sizeof(UWORD) * CLUSTER_MAX);
	memset(aClusterVisibility, 0, sizeof(UBYTE) * CLUSTER_MAX);
	memset(aClusterAttacked, 0, sizeof(UDWORD) * CLUSTER_MAX);
	memset(aClusterInfo, 0, sizeof(UBYTE) * CLUSTER_MAX);

	for(player=0; player<MAX_PLAYERS; player++)
	{
		for(psDroid=apsDroidLists[player]; psDroid; psDroid=psDroid->psNext)
		{
			psDroid->cluster = 0;
		}

		for(psStruct=apsStructLists[player]; psStruct; psStruct=psStruct->psNext)
		{
			psStruct->cluster = 0;
		}

		for(psStruct=apsStructLists[player]; psStruct; psStruct=psStruct->psNext)
		{
			if (psStruct->cluster == 0)
			{
				clustUpdateObject((BASE_OBJECT *)psStruct);
			}
		}
	}
}


// check the cluster usage
void clustValidateUsage()
{
	SDWORD		cluster, player, droidUsage, structUsage;
	STRUCTURE	*psStruct;
	DROID		*psDroid;
	SDWORD		found;

	for(cluster=1; cluster<CLUSTER_MAX; cluster++)
	{
		found=MAX_PLAYERS;
		for(player=0; player<MAX_PLAYERS; player++)
		{
			droidUsage=0;
			structUsage=0;
			for(psDroid=apsDroidLists[player]; psDroid; psDroid=psDroid->psNext)
			{
				if (psDroid->cluster == cluster)
				{
					ASSERT(( (found == MAX_PLAYERS) || (droidUsage != 0),
						"clustValidateUsage: cluster has mixed players"));

					found = player;
					droidUsage += 1;
				}
			}
			for(psStruct=apsStructLists[player]; psStruct; psStruct=psStruct->psNext)
			{
				if (psStruct->cluster == cluster)
				{
					ASSERT(( (found == MAX_PLAYERS) || (structUsage != 0),
						"clustValidateUsage: cluster has mixed players"));

					found = player;
					structUsage += 1;
				}
			}

			if (found == player)
			{
				ASSERT(( (droidUsage == 0) || (structUsage == 0),
					"clustValidateUsage: cluster contains both droids and structs"));

				ASSERT(( aClusterUsage[cluster] == droidUsage + structUsage,
					"clustValidateUsage: invalid cluster usage"));
			}
		}
	}
}

// update routine for the cluster system
void clusterUpdate(void)
{
	SDWORD	i;

	for(i=1; i< CLUSTER_MAX; i++)
	{
		if (aClusterEmpty[i])
		{
			scrCBEmptyClusterID = i;
			eventFireCallbackTrigger(CALL_CLUSTER_EMPTY);
			aClusterEmpty[i] = FALSE;
		}
	}
}


// update all objects from a list belonging to a specific cluster
void clustUpdateCluster(BASE_OBJECT *psList, SDWORD cluster)
{
	BASE_OBJECT		*psCurr;

	if (cluster == 0)
	{
		return;
	}

	for(psCurr = psList; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->cluster == cluster)
		{
			clustUpdateObject(psCurr);
		}
	}
}

// remove an object from the cluster system
void clustRemoveObject(BASE_OBJECT *psObj)
{
	SDWORD i;

	ASSERT((psObj->cluster < CLUSTER_MAX,
		"clustRemoveObject: invalid cluster number"));

	// update the usage counter
	if (psObj->cluster != 0)
	{
		ASSERT((aClusterUsage[psObj->cluster] > 0,
			"clustRemoveObject: usage array out of sync"));
		aClusterUsage[psObj->cluster] -= 1;

		if (aClusterUsage[psObj->cluster] == 0)
		{
			// cluster is empty - make sure the cluster map gets emptied
			for (i=0; i<CLUSTER_MAX; i++)
			{
				if (aClusterMap[i] == psObj->cluster)
				{
					aClusterMap[i] = 0;
					if (i != 0)
					{
						DBP0(("Cluster %d empty: ", i));
						DBP0(("%s ", (psObj->type == OBJ_DROID) ? "Unit" : ((psObj->type == OBJ_STRUCTURE) ? "Struct" : "Feat") ));
						DBP0(("id %d player %d\n", psObj->id, psObj->player));
						aClusterEmpty[i] = TRUE;
					}
				}
			}

			// reset the cluster visibility and attacked
			aClusterVisibility[psObj->cluster] = 0;
			aClusterAttacked[psObj->cluster] = 0;
			aClusterInfo[psObj->cluster] = 0;
		}
	}

	psObj->cluster = 0;
}


// tell a droid to join a cluster
void _clustAddDroid(DROID *psDroid, SDWORD cluster)
{
	DROID	*psCurr;
	SDWORD	xdiff, ydiff;
	SDWORD	player;

	clustRemoveObject((BASE_OBJECT *)psDroid);

	aClusterUsage[cluster] += 1;
	psDroid->cluster = (UBYTE)cluster;
	for(player=0; player<MAX_PLAYERS; player++)
	{
		if (psDroid->visible[player])
		{
			aClusterVisibility[cluster] |= (1 << player);
		}
	}

	for(psCurr = apsDroidLists[psDroid->player]; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->cluster == (UBYTE)cluster)
		{
			continue;
		}

		xdiff = (SDWORD)psDroid->x - (SDWORD)psCurr->x;
		ydiff = (SDWORD)psDroid->y - (SDWORD)psCurr->y;
		if (xdiff*xdiff + ydiff*ydiff < CLUSTER_DIST*CLUSTER_DIST)
		{
			_clustAddDroid(psCurr, cluster);
		}
	}
}


void clustAddDroid(DROID *psDroid, SDWORD cluster)
{
	_clustAddDroid(psDroid,cluster);
}



// tell the cluster system about a new droid
void clustNewDroid(DROID *psDroid)
{
	DROID	*psCurr;
	SDWORD	xdiff, ydiff;

	psDroid->cluster = 0;
	for(psCurr = apsDroidLists[psDroid->player]; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->cluster != 0)
		{
			xdiff = (SDWORD)psDroid->x - (SDWORD)psCurr->x;
			ydiff = (SDWORD)psDroid->y - (SDWORD)psCurr->y;
			if (xdiff*xdiff + ydiff*ydiff < CLUSTER_DIST*CLUSTER_DIST)
			{
				clustAddDroid(psDroid, psCurr->cluster);
				return;
			}
		}
	}
}


// tell a structure to join a cluster
void _clustAddStruct(STRUCTURE *psStruct, SDWORD cluster)
{
	STRUCTURE	*psCurr;
	SDWORD		xdiff, ydiff;
	SDWORD		player;

	clustRemoveObject((BASE_OBJECT *)psStruct);

	aClusterUsage[cluster] += 1;
	psStruct->cluster = (UBYTE)cluster;
	for(player=0; player<MAX_PLAYERS; player++)
	{
		if (psStruct->visible[player])
		{
			aClusterVisibility[cluster] |= (1 << player);
		}
	}

	for(psCurr = apsStructLists[psStruct->player]; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->cluster == (UBYTE)cluster)
		{
			continue;
		}

		xdiff = (SDWORD)psStruct->x - (SDWORD)psCurr->x;
		ydiff = (SDWORD)psStruct->y - (SDWORD)psCurr->y;
		if (xdiff*xdiff + ydiff*ydiff < CLUSTER_DIST*CLUSTER_DIST)
		{
			_clustAddStruct(psCurr, cluster);
		}
	}
}

void clustAddStruct(STRUCTURE *psStruct, SDWORD cluster)
{
	_clustAddStruct(psStruct,cluster);
}

// tell the cluster system about a new structure
void clustNewStruct(STRUCTURE *psStruct)
{
	STRUCTURE	*psCurr;
	SDWORD		xdiff, ydiff;

	psStruct->cluster = 0;
	for(psCurr = apsStructLists[psStruct->player]; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->cluster != 0)
		{
			xdiff = (SDWORD)psStruct->x - (SDWORD)psCurr->x;
			ydiff = (SDWORD)psStruct->y - (SDWORD)psCurr->y;
			if (xdiff*xdiff + ydiff*ydiff < CLUSTER_DIST*CLUSTER_DIST)
			{
				psStruct->cluster = psCurr->cluster;
				aClusterUsage[psCurr->cluster] += 1;
				break;
			}
		}
	}

	clustUpdateObject((BASE_OBJECT *)psStruct);
}


// find an unused cluster number for a droid
SDWORD clustFindUnused(void)
{
	SDWORD	cluster;

	for(cluster = 1; cluster < CLUSTER_MAX; cluster ++)
	{
		if (aClusterUsage[cluster] == 0)
		{
			return cluster;
		}
	}

	// no unused cluster return the default
	return 0;
}


// display the current clusters
/*void clustDisplay(void)
{
	SDWORD	player, cluster;
	DROID	*psCurr;
	BOOL	shownCluster;

	DBPRINTF(("Current clusters:\n"));
	for (player=0; player<MAX_PLAYERS; player++)
	{
		DBPRINTF(("Player %d:\n", player));
		for (cluster=0; cluster < UBYTE_MAX; cluster++)
		{
			shownCluster = FALSE;
			for (psCurr=apsDroidLists[player]; psCurr; psCurr=psCurr->psNext)
			{
				if (psCurr->cluster == cluster)
				{
					if (!shownCluster)
					{
						DBPRINTF(("   Cluster %d:  ", cluster));
						shownCluster = TRUE;
					}
					DBPRINTF(("%d  ", psCurr->id));
				}
			}
			if (shownCluster)
			{
				DBPRINTF(("\n"));
			}
		}
	}
}*/

// display the current clusters
void clustDisplay(void)
{
	SDWORD	cluster, map, player;
	DROID		*psDroid;
	STRUCTURE	*psStruct;
	STRING	aBuff[255];
	BOOL	found;
	SDWORD	numUsed;

	numUsed = 0;
	for(map=0; map < CLUSTER_MAX; map++)
	{
		if (aClusterMap[map] != 0)
		{
			numUsed += 1;
		}
	}

	CONPRINTF(ConsoleString, (ConsoleString, "Current clusters (%d):\n", numUsed));
	for(map=0; map < CLUSTER_MAX; map++)
	{
		cluster = aClusterMap[map];
		if (cluster != 0)
		{
			found = FALSE;
			for(player = 0; player < MAX_PLAYERS; player++)
			{
//				if (player == (SDWORD)selectedPlayer)
//				{
//					continue;
//				}

				for(psDroid=apsDroidLists[player]; psDroid; psDroid=psDroid->psNext)
				{
					if ((psDroid->cluster == cluster) &&
//						(psDroid->visible[selectedPlayer]))
						(psDroid->player == selectedPlayer))
					{
						if (!found)
						{
							// found a cluster print it out
							sprintf(aBuff, "Unit cluster %d (%d), ", map, cluster);
							sprintf(aBuff + strlen(aBuff), "player %d:", psDroid->player);
							found = TRUE;
						}

						if (strlen(aBuff) < 250)
						{
							sprintf(aBuff + strlen(aBuff), " %d", psDroid->id);
						}
					}
				}
				for(psStruct=apsStructLists[player]; psStruct; psStruct=psStruct->psNext)
				{
					if ((psStruct->cluster == cluster) &&
//						(psStruct->visible[selectedPlayer]))
						(psStruct->player == selectedPlayer))

					{
						if (!found)
						{
							// found a cluster print it out
							sprintf(aBuff, "struct cluster %d (%d), ", map, cluster);
							sprintf(aBuff + strlen(aBuff), "player %d:", psStruct->player);
							found = TRUE;
						}

						if (strlen(aBuff) < 250)
						{
							sprintf(aBuff + strlen(aBuff), " %d", psStruct->id);
						}
					}
				}
			}

			if (found)
			{
				CONPRINTF(ConsoleString, (ConsoleString, "%s", aBuff));
			}
		}
	}
}

// update the cluster information for an object
void clustUpdateObject(BASE_OBJECT * psObj)
{
	SDWORD	newCluster, oldCluster, i;
	BOOL	found;
	SDWORD	player;

	newCluster = clustFindUnused();
	oldCluster = psObj->cluster;

	// update the cluster map
	found = FALSE;
	if (oldCluster != 0)
	{
		for(i=0; i<CLUSTER_MAX; i++)
		{
			ASSERT(( (aClusterMap[i] == 0) ||
					 (aClusterUsage[ aClusterMap[i] ] != 0),
				"clustUpdateObject: cluster map out of sync" ));

			if (aClusterMap[i] == oldCluster)
			{
				// found the old cluster - change it to the new one
				aClusterMap[i] = (UBYTE)newCluster;
				aClusterAttacked[newCluster] = aClusterAttacked[oldCluster];
//				aClusterVisibility[newCluster] = aClusterVisibility[oldCluster];
				aClusterVisibility[newCluster] = 0;
				for(player = 0; player < MAX_PLAYERS; player++)
				{
					if (psObj->visible[player])
					{
						aClusterVisibility[newCluster] |= 1 << player;
					}
				}
				found = TRUE;
				break;
			}
		}
	}

	if (!found)
	{
		// there is no current cluster map - create a new one
		for(i=1; i<CLUSTER_MAX; i++)
		{
			if (aClusterMap[i] == 0)
			{
				// found a free cluster
				aClusterMap[i] = (UBYTE)newCluster;
				break;
			}
		}
	}

	// store the information about this cluster
	aClusterInfo[newCluster] = (UBYTE)(psObj->player & CLUSTER_PLAYER_MASK);
	switch (psObj->type)
	{
	case OBJ_DROID:
		aClusterInfo[newCluster] |= CLUSTER_DROID;
		break;
	case OBJ_STRUCTURE:
		aClusterInfo[newCluster] |= CLUSTER_STRUCTURE;
		break;
	default:
		ASSERT((FALSE,"clustUpdateObject: invalid object type"));
		break;
	}

	switch (psObj->type)
	{
	case OBJ_DROID:
		clustAddDroid((DROID *)psObj, newCluster);
		break;
	case OBJ_STRUCTURE:
		clustAddStruct((STRUCTURE *)psObj, newCluster);
		break;
	default:
		ASSERT((FALSE, "clustUpdateObject: invalid object type"));
		break;
	}
}


// get the cluster ID for a droid
SDWORD clustGetClusterID(BASE_OBJECT *psObj)
{
	SDWORD	cluster;

	if (psObj->cluster == 0)
	{
		return 0;
	}

	for (cluster=0; cluster < CLUSTER_MAX; cluster ++)
	{
		if (aClusterMap[cluster] == psObj->cluster)
		{
			return cluster;
		}
	}

	return 0;
}

// get the actual cluster number from a cluster ID
SDWORD clustGetClusterFromID(SDWORD clusterID)
{
	ASSERT(((clusterID >= 0) && (clusterID < CLUSTER_MAX),
		"clustGetClusterFromID: invalid cluster ID"));

	return aClusterMap[clusterID];
}

// variables for the cluster iteration
static SDWORD		iterateClusterID;
static BASE_OBJECT	*psIterateList, *psIterateObj;

// initialise iterating a cluster
void clustInitIterate(SDWORD clusterID)
{
	SDWORD		player, cluster;

	iterateClusterID = clusterID;
	cluster = aClusterMap[clusterID];
	player = aClusterInfo[cluster] & CLUSTER_PLAYER_MASK;

	if (aClusterInfo[cluster] & CLUSTER_DROID)
	{
		psIterateList = (BASE_OBJECT *)apsDroidLists[player];
	}
	else // if (aClusterInfo[cluster] & CLUSTER_STRUCTURE)
	{
		psIterateList = (BASE_OBJECT *)apsStructLists[player];
	}

	psIterateObj = NULL;
}

// iterate a cluster
BASE_OBJECT *clustIterate(void)
{
	BASE_OBJECT	*psStart;
	SDWORD		cluster;

	cluster = aClusterMap[iterateClusterID];

	if (psIterateObj == NULL)
	{
		psStart = psIterateList;
	}
	else
	{
		psStart = psIterateObj->psNext;
	}

	for(psIterateObj=psStart;
		psIterateObj && psIterateObj->cluster != cluster;
		psIterateObj = psIterateObj->psNext)
		;

	if (psIterateObj == NULL)
	{
		psIterateList = NULL;
	}

	return psIterateObj;
}

// find the center of a cluster
void clustGetCenter(BASE_OBJECT *psObj, SDWORD *px, SDWORD *py)
{
	BASE_OBJECT		*psList;
	BASE_OBJECT		*psCurr;
	SDWORD			averagex, averagey, num;

	switch (psObj->type)
	{
	case OBJ_DROID:
		psList = (BASE_OBJECT *)apsDroidLists[psObj->player];
		break;
	case OBJ_STRUCTURE:
		psList = (BASE_OBJECT *)apsStructLists[psObj->player];
		break;
	default:
		ASSERT((FALSE,"clustGetCenter: invalid object type"));
		psList = NULL;
		break;
	}

	averagex = 0;
	averagey = 0;
	num = 0;
	for (psCurr = psList; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->cluster == psObj->cluster)
		{
			averagex += (SDWORD)psCurr->x;
			averagey += (SDWORD)psCurr->y;
			num += 1;
		}
	}

	if (num > 0)
	{
		*px = averagex / num;
		*py = averagey / num;
	}
	else
	{
		*px = (SDWORD)psObj->x;
		*py = (SDWORD)psObj->y;
	}
}


// tell the cluster system that an objects visibility has changed
void clustObjectSeen(BASE_OBJECT *psObj, BASE_OBJECT *psViewer)
{
	SDWORD	player;

	for(player=0; player<MAX_PLAYERS; player++)
	{
		if ( (player != (SDWORD)psObj->player) &&
			 psObj->visible[player] &&
			!(aClusterVisibility[psObj->cluster] & (1 << player)))
		{
//			DBPRINTF(("cluster %d (player %d) seen by player %d\n",
//				clustGetClusterID(psObj), psObj->player, player));
			aClusterVisibility[psObj->cluster] |= 1 << player;

			psScrCBObjSeen = psObj;
			psScrCBObjViewer = psViewer;
			eventFireCallbackTrigger(CALL_OBJ_SEEN);

			switch (psObj->type)
			{
			case OBJ_DROID:
				eventFireCallbackTrigger(CALL_DROID_SEEN);
				break;
			case OBJ_STRUCTURE:
				eventFireCallbackTrigger(CALL_STRUCT_SEEN);
				break;
			case OBJ_FEATURE:
				eventFireCallbackTrigger(CALL_FEATURE_SEEN);
				break;
			default:
				ASSERT((FALSE, "clustObjectSeen: invalid object type"));
				break;
			}

			psScrCBObjSeen = NULL;
			psScrCBObjViewer = NULL;
		}
	}
}


// tell the cluster system that an object has been attacked
void clustObjectAttacked(BASE_OBJECT *psObj)
{
	if ((aClusterAttacked[psObj->cluster] + ATTACK_CB_PAUSE) < gameTime)
	{
//		DBPRINTF(("CALL_ATTACKED player %d, cluster %d\n",
//			psObj->player, psObj->cluster));
		psScrCBTarget = psObj;
		eventFireCallbackTrigger(CALL_ATTACKED);

		switch (psObj->type)
		{
		case OBJ_DROID:
			psLastDroidHit = (DROID *)psObj;
			eventFireCallbackTrigger(CALL_DROID_ATTACKED);
			psLastDroidHit = NULL;
			break;
		case OBJ_STRUCTURE:
			psLastStructHit = (STRUCTURE *)psObj;
			eventFireCallbackTrigger(CALL_STRUCT_ATTACKED);
			psLastStructHit = NULL;
			break;
		default:
			ASSERT((FALSE, "clustObjectAttacked: invalid object type"));
			break;
		}

		aClusterAttacked[psObj->cluster] = gameTime;
	}
}


// reset the visibility for all clusters for a particular player
void clustResetVisibility(SDWORD player)
{
	SDWORD	i;

	for(i=0; i<CLUSTER_MAX; i++)
	{
		aClusterVisibility[i] &= ~(1 << player);
	}
}

