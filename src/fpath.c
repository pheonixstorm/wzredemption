/*
 * fpath.c
 *
 * Interface to the routing functions
 *
 */

// route success printf's
//#define DEBUG_GROUP0
// way point printf's
//#define DEBUG_GROUP1
// gateway route printf's
//#define DEBUG_GROUP2

#include "frame.h"

BOOL	fpathDoMessage;
#undef DBP2
#define DBP2( x ) \
	if (fpathDoMessage) \
		DBPRINTF( x )

#include "objects.h"
#include "map.h"
#include "raycast.h"
#include "geometry.h"
#include "hci.h"
#include "order.h"

#ifdef TEST_BED
#include "main.h"
#endif
#include "astar.h"
#include "losroute.h"
#include "gateway.h"
#include "gatewayroute.h"
#include "action.h"
#include "formation.h"

#define DEFINE_MAPINLINE
#include "fpath.h"

/* minimum height difference for VTOL blocking tile */
#define	LIFT_BLOCK_HEIGHT_LIGHTBODY		  30
#define	LIFT_BLOCK_HEIGHT_MEDIUMBODY	 350
#define	LIFT_BLOCK_HEIGHT_HEAVYBODY		 350

#define NUM_DIR		8
// Convert a direction into an offset
// dir 0 => x = 0, y = -1
static POINT aDirOffset[NUM_DIR] =
{
	 0, 1,
	-1, 1,
	-1, 0,
	-1,-1,
	 0,-1,
	 1,-1,
	 1, 0,
	 1, 1,
};

/* global pointer for object being routed - GJ hack -
 * currently only used in fpathLiftBlockingTile */
static	BASE_OBJECT	*g_psObjRoute = NULL;

// function pointer for the blocking tile check
BOOL (*fpathBlockingTile)(SDWORD x, SDWORD y);

// if a route is spread over a number of frames this stores the object
// the route is being done for
static BASE_OBJECT *psPartialRouteObj = NULL;

// coords of the partial route
static SDWORD partialSX,partialSY, partialTX,partialTY;

// the last frame on which the partial route was calculatated
static SDWORD	lastPartialFrame;


// the maximum amount of routing to do per frame
SDWORD	astarMaxRoute = FPATH_MAX_ROUTE_INIT;

BOOL fpathFindRoute(DROID *psDroid, SDWORD sX,SDWORD sY, SDWORD tX,SDWORD tY);

// initialise the findpath module
BOOL fpathInitialise(void)
{
	fpathBlockingTile = fpathGroundBlockingTile;
	psPartialRouteObj = NULL;

	return TRUE;
}

// update routine for the findpath system
void fpathUpdate(void)
{
	if ((psPartialRouteObj != NULL) &&
		((psPartialRouteObj->died) ||
		 (((DROID*)psPartialRouteObj)->sMove.Status != MOVEWAITROUTE) ||
		 ((lastPartialFrame + 5) < (SDWORD)frameGetFrameNumber()) ) )
	{
		psPartialRouteObj = NULL;
	}
}


// access functions for the loop limit
void fpathSetMaxRoute(SDWORD max)
{
	astarMaxRoute = max;
}
SDWORD fpathGetMaxRoute(void)
{
	return astarMaxRoute;
}

#define	VTOL_MAP_EDGE_TILES		1

// Check if the map tile at a location blocks a droid
BOOL fpathGroundBlockingTile(SDWORD x, SDWORD y)
{
	MAPTILE	*psTile;
//	FEATURE	*psFeat;

    //doesn't look like we need this - pickATile wasn't working with it! - AB 8/2/99
	/* check VTOL limits if not routing */
	/*if ( g_psObjRoute == NULL )
	{
		if ( x < VTOL_MAP_EDGE_TILES || y < VTOL_MAP_EDGE_TILES ||
			 x >= (SDWORD)mapWidth-VTOL_MAP_EDGE_TILES || y >= (SDWORD)mapHeight-VTOL_MAP_EDGE_TILES)
		{
			// coords off map - auto blocking tile
			return TRUE;
		}
	}
	else*/
	{
		if (x < scrollMinX+1 || y < scrollMinY+1 ||
			x >= scrollMaxX-1 || y >= scrollMaxY-1)
		{
			// coords off map - auto blocking tile
			return TRUE;
		}
	}

	ASSERT(( !(x <1 || y < 1 ||	x >= (SDWORD)mapWidth-1 || y >= (SDWORD)mapHeight-1),
		"fpathBlockingTile: off map" ));

	psTile = mapTile((UDWORD)x, (UDWORD)y);
/*
	// THIS CAN'T BE HERE - TESTING ONLY FIXME
	if( (TILE_HAS_STRUCTURE(psTile)) AND
		(getTileStructure(x,y)->pStructureType->type == REF_BLASTDOOR) AND	  // slow bit
		(getTileStructure(x,y)->player==selectedPlayer) )
	{
		return(FALSE);
	}
*/

/*  This god awful hack RIP - John 15.2.99
	if(TILE_HAS_FEATURE(psTile))
	{
		psFeat = getTileFeature(x,y);
		if ((psFeat != NULL) &&
			(psFeat->psStats->subType == FEAT_GEN_ARTE OR psFeat->psStats->subType == FEAT_OIL_DRUM))
		{
			return(FALSE);
		}
	}*/

#ifndef TEST_BED
	if ((psTile->tileInfoBits & BITS_FPATHBLOCK) ||
		(TILE_OCCUPIED(psTile) && !TILE_IS_NOTBLOCKING(psTile)) ||
		(TERRAIN_TYPE(psTile) == TER_CLIFFFACE) ||
		(TERRAIN_TYPE(psTile) == TER_WATER))
#else
	if (psTile->tileInfoBits & BLOCKED)
#endif
	{
		return TRUE;
	}
	return FALSE;
}

#ifndef TEST_BED
// Check if the map tile at a location blocks a droid
BOOL fpathHoverBlockingTile(SDWORD x, SDWORD y)
{
	MAPTILE	*psTile;

	if (x < scrollMinX+1 || y < scrollMinY+1 ||
		x >= scrollMaxX-1 || y >= scrollMaxY-1)
	{
		// coords off map - auto blocking tile
		return TRUE;
	}

	ASSERT(( !(x <1 || y < 1 ||	x >= (SDWORD)mapWidth-1 || y >= (SDWORD)mapHeight-1),
		"fpathBlockingTile: off map" ));

	psTile = mapTile((UDWORD)x, (UDWORD)y);

	if ((psTile->tileInfoBits & BITS_FPATHBLOCK) ||
		(TILE_OCCUPIED(psTile) && !TILE_IS_NOTBLOCKING(psTile)) ||
		(TERRAIN_TYPE(psTile) == TER_CLIFFFACE))
	{
		return TRUE;
	}
	return FALSE;
}

// Check if the map tile at a location blocks a vtol
BOOL fpathLiftBlockingTile(SDWORD x, SDWORD y)
{
	MAPTILE		*psTile;
	SDWORD		iLiftHeight, iBlockingHeight;
	DROID		*psDroid = (DROID *) g_psObjRoute;

	ASSERT( (PTRVALID(g_psObjRoute, sizeof(BASE_OBJECT)),
		"fpathLiftBlockingTile: invalid object pointer") );
	ASSERT( (PTRVALID(psDroid, sizeof(DROID)),
		"fpathLiftBlockingTile: invalid droid pointer") );

	if (psDroid->droidType == DROID_TRANSPORTER )
	{
		if ( x<1 || y<1 || x>=(SDWORD)mapWidth-1 || y>=(SDWORD)mapHeight-1 )
		{
			return TRUE;
		}

		psTile = mapTile((UDWORD)x, (UDWORD)y);

		if ( TILE_HAS_TALLSTRUCTURE(psTile) )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	if ( x < VTOL_MAP_EDGE_TILES ||
		 y < VTOL_MAP_EDGE_TILES ||
		 x >= (SDWORD)mapWidth-VTOL_MAP_EDGE_TILES ||
		 y >= (SDWORD)mapHeight-VTOL_MAP_EDGE_TILES   )
	{
		// coords off map - auto blocking tile
		return TRUE;
	}

	ASSERT(( !(x <1 || y < 1 ||	x >= (SDWORD)mapWidth-1 || y >= (SDWORD)mapHeight-1),
			"fpathLiftBlockingTile: off map" ));

	/* no tiles are blocking if returning to rearm */
	if( psDroid->action == DACTION_MOVETOREARM )
	{
		return FALSE;
	}

	psTile = mapTile((UDWORD)x, (UDWORD)y);

	/* consider cliff faces */
	if ( TERRAIN_TYPE(psTile) == TER_CLIFFFACE )
	{
		switch ( (asBodyStats + psDroid->asBits[COMP_BODY].nStat)->size )
		{
			case SIZE_LIGHT:
				iBlockingHeight = LIFT_BLOCK_HEIGHT_LIGHTBODY;
				break;
			case SIZE_MEDIUM:
				iBlockingHeight = LIFT_BLOCK_HEIGHT_MEDIUMBODY;
				break;
			case SIZE_HEAVY:
				iBlockingHeight = LIFT_BLOCK_HEIGHT_HEAVYBODY;
				break;
			default:
				iBlockingHeight = LIFT_BLOCK_HEIGHT_LIGHTBODY;
		}

		/* approaching cliff face; block if below it */
		iLiftHeight = (SDWORD) map_Height( x<<TILE_SHIFT, y<<TILE_SHIFT ) -
					  (SDWORD) map_Height( g_psObjRoute->x, g_psObjRoute->y );
		if ( iLiftHeight > iBlockingHeight )
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if ( TILE_HAS_TALLSTRUCTURE(psTile) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

// Check if an edge map tile blocks a vtol (for sliding at map edge)
BOOL fpathLiftSlideBlockingTile(SDWORD x, SDWORD y)
{
	if ( x < 1 || y < 1 ||
		 x >= (SDWORD)GetWidthOfMap()-1  ||
		 y >= (SDWORD)GetHeightOfMap()-1    )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#endif

// Calculate the distance to a tile from a point
SDWORD fpathDistToTile(SDWORD tileX,SDWORD tileY, SDWORD pointX, SDWORD pointY)
{
	SDWORD	xdiff,ydiff, dist;
	SDWORD	tx,ty;

	// get the difference in tile coords
	xdiff = tileX - (pointX >> TILE_SHIFT);
	ydiff = tileY - (pointY >> TILE_SHIFT);

	ASSERT(((xdiff >= -1 && xdiff <= 1 && ydiff >= -1 && ydiff <= 1),
		"fpathDistToTile: points are more than one tile apart"));
	ASSERT((xdiff != 0 || ydiff != 0,
		"fpathDistToTile: points are on same tile"));

	// not the most elegant solution but it works
	switch (xdiff + ydiff * 10)
	{
	case 10:	// xdiff == 0, ydiff == 1
		dist = TILE_UNITS - (pointY & TILE_MASK);
		break;
	case 9:		// xdiff == -1, ydiff == 1
		tx = pointX & TILE_MASK;
		ty = TILE_UNITS - (pointY & TILE_MASK);
		dist = tx > ty ? tx + ty/2 : tx/2 + ty;
		break;
	case -1:	// xdiff == -1, ydiff == 0
		dist = pointX & TILE_MASK;
		break;
	case -11:	// xdiff == -1, ydiff == -1
		tx = pointX & TILE_MASK;
		ty = pointY & TILE_MASK;
		dist = tx > ty ? tx + ty/2 : tx/2 + ty;
		break;
	case -10:	// xdiff == 0, ydiff == -1
		dist = pointY & TILE_MASK;
		break;
	case -9:	// xdiff == 1, ydiff == -1
		tx = TILE_UNITS - (pointX & TILE_MASK);
		ty = pointY & TILE_MASK;
		dist = tx > ty ? tx + ty/2 : tx/2 + ty;
		break;
	case 1:		// xdiff == 1, ydiff == 0
		dist = TILE_UNITS - (pointX & TILE_MASK);
		break;
	case 11:	// xdiff == 1, ydiff == 1
		tx = TILE_UNITS - (pointX & TILE_MASK);
		ty = TILE_UNITS - (pointY & TILE_MASK);
		dist = tx > ty ? tx + ty/2 : tx/2 + ty;
		break;
	default:
		ASSERT((FALSE, "fpathDistToTile: unexpected point relationship"));
		dist = TILE_UNITS;
		break;
	}

	return dist;
}

// Variables for the callback
static SDWORD	finalX,finalY, vectorX,vectorY;
static SDWORD	clearX,clearY;
static BOOL		obstruction;

// callback to find the first clear tile before an obstructed target
BOOL fpathEndPointCallback(SDWORD x, SDWORD y, SDWORD dist)
{
	SDWORD	vx,vy;

	dist = dist;

	// See if this point is past the final point (dot product)
	vx = x - finalX;
	vy = y - finalY;
	if (vx*vectorX + vy*vectorY <=0)
	{
		return FALSE;
	}

	// note the last clear tile
	if (!fpathBlockingTile(x>>TILE_SHIFT,y>>TILE_SHIFT))
	{
		clearX = (x & ~TILE_MASK) + TILE_UNITS/2;
		clearY = (y & ~TILE_MASK) + TILE_UNITS/2;
	}
	else
	{
		obstruction = TRUE;
	}

	return TRUE;
}

void fpathSetDirectRoute( BASE_OBJECT *psObj, SDWORD targetX, SDWORD targetY )
{
	MOVE_CONTROL *psMoveCntl;

	ASSERT( (PTRVALID(psObj, sizeof(BASE_OBJECT)),
			"fpathSetDirectRoute: invalid object pointer\n") );

	if ( psObj->type == OBJ_DROID )
	{
		psMoveCntl = &((DROID *) psObj)->sMove;

		/* set global pointer for object being routed - GJ hack */
		fpathSetCurrentObject( psObj );

		psMoveCntl->DestinationX = targetX;
		psMoveCntl->DestinationY = targetY;
//		psMoveCntl->MovementList[0].XCoordinate = targetX;
//		psMoveCntl->MovementList[0].YCoordinate = targetY;
//		psMoveCntl->MovementList[1].XCoordinate = -1;
//		psMoveCntl->MovementList[1].YCoordinate = -1;
		psMoveCntl->numPoints = 1;
		psMoveCntl->asPath[0].x = (UBYTE)(targetX >> TILE_SHIFT);
		psMoveCntl->asPath[0].y = (UBYTE)(targetY >> TILE_SHIFT);
	}
}


// append an astar route onto a move-control route
void fpathAppendRoute( MOVE_CONTROL *psMoveCntl, ASTAR_ROUTE *psAStarRoute )
{
	SDWORD		mi, ai;

	// find the end of the last route
//	for(mi=0; psMoveCntl->MovementList[mi].XCoordinate != -1; mi += 1)
//		;

	mi = psMoveCntl->numPoints;
	ai = 0;
	while ((mi < TRAVELSIZE) && (ai < psAStarRoute->numPoints))
	{
//		psMoveCntl->MovementList[mi].XCoordinate =
		psMoveCntl->asPath[mi].x = (UBYTE)(psAStarRoute->asPos[ai].x);
//			(UBYTE)(psAStarRoute->asPos[ai].x >> TILE_SHIFT);
//		psMoveCntl->MovementList[mi].YCoordinate =
		psMoveCntl->asPath[mi].y = (UBYTE)(psAStarRoute->asPos[ai].y);
//			(UBYTE)(psAStarRoute->asPos[ai].y >> TILE_SHIFT);

		ai += 1;
		mi += 1;
	}

//	psMoveCntl->MovementList[mi].XCoordinate = -1;
//	psMoveCntl->MovementList[mi].YCoordinate = -1;
	psMoveCntl->numPoints = (UBYTE)(psMoveCntl->numPoints + ai);
	psMoveCntl->DestinationX = (psAStarRoute->finalX << TILE_SHIFT) + TILE_UNITS/2;
	psMoveCntl->DestinationY = (psAStarRoute->finalY << TILE_SHIFT) + TILE_UNITS/2;
}


// set the routing block flags for the gateways
/*void fpathSetGatewayBlock(void)
{
	GATEWAY		*psCurr;
	SDWORD		pos;
	MAPTILE		*psTile;

	for(psCurr=psGateways; psCurr; psCurr=psCurr->psNext)
	{
		if (!(psCurr->flags & GWR_INROUTE))
		{
			if (psCurr->x1 == psCurr->x2)
			{
				for(pos = psCurr->y1; pos <= psCurr->y2; pos += 1)
				{
					psTile = mapTile(psCurr->x1, pos);
					psTile->tileInfoBits |= BITS_FPATHBLOCK;
				}
			}
			else
			{
				for(pos = psCurr->x1; pos <= psCurr->x2; pos += 1)
				{
					psTile = mapTile(pos, psCurr->y1);
					psTile->tileInfoBits |= BITS_FPATHBLOCK;
				}
			}
		}
	}
}*/


// clear the routing block flags for the gateways
/*void fpathClearGatewayBlock(void)
{
	GATEWAY		*psCurr;
	SDWORD		pos;
	MAPTILE		*psTile;

	for(psCurr=psGateways; psCurr; psCurr=psCurr->psNext)
	{
//		if (!(psCurr->flags & GWR_INROUTE))
		{
			if (psCurr->x1 == psCurr->x2)
			{
				for(pos = psCurr->y1; pos <= psCurr->y2; pos += 1)
				{
					psTile = mapTile(psCurr->x1, pos);
					psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
				}
			}
			else
			{
				for(pos = psCurr->x1; pos <= psCurr->x2; pos += 1)
				{
					psTile = mapTile(pos, psCurr->y1);
					psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
				}
			}
		}
	}
}*/


// check whether a WORLD coordinate point is within a gateway's tiles
BOOL fpathPointInGateway(SDWORD x, SDWORD y, GATEWAY *psGate)
{
	x = x >> TILE_SHIFT;
	y = y >> TILE_SHIFT;

	if ((x >= psGate->x1) && (x <= psGate->x2) &&
		(y >= psGate->y1) && (y <= psGate->y2))
	{
		return TRUE;
	}

	return FALSE;
}


// set blocking flags for all gateways around a zone
void fpathSetGatewayBlock(SDWORD zone, GATEWAY *psLast, GATEWAY *psNext)
{
	GATEWAY		*psCurr;
	SDWORD		pos, tx,ty, blockZone;
	MAPTILE		*psTile;

	for(psCurr=psGateways; psCurr; psCurr=psCurr->psNext)
	{
		if ((psCurr != psLast) &&
			(psCurr != psNext) &&
			!(psCurr->flags & GWR_WATERLINK) &&
			((psCurr->zone1 == zone) || (psCurr->zone2 == zone)) )
		{
			if (psCurr->x1 == psCurr->x2)
			{
				for(pos = psCurr->y1; pos <= psCurr->y2; pos += 1)
				{
					psTile = mapTile(psCurr->x1, pos);
					psTile->tileInfoBits |= BITS_FPATHBLOCK;
				}
			}
			else
			{
				for(pos = psCurr->x1; pos <= psCurr->x2; pos += 1)
				{
					psTile = mapTile(pos, psCurr->y1);
					psTile->tileInfoBits |= BITS_FPATHBLOCK;
				}
			}
		}
	}

	// now set the blocking flags next to the two gateways that the route
	// is going through
	DBP2(("Blocking gateways for zones :"));
	if (psLast != NULL)
	{
		blockZone = (psLast->flags & GWR_ZONE1) ? psLast->zone1 : psLast->zone2;
		DBP2((" %d", blockZone));
		for(tx = psLast->x1 - 1; tx <= psLast->x2 + 1; tx ++)
		{
			for(ty = psLast->y1 - 1; ty <= psLast->y2 + 1; ty ++)
			{
				if (!fpathPointInGateway(tx << TILE_SHIFT, ty << TILE_SHIFT, psLast) &&
					tileOnMap(tx,ty) && gwGetZone(tx,ty) == blockZone)
				{
					psTile = mapTile(tx, ty);
					psTile->tileInfoBits |= BITS_FPATHBLOCK;
				}
			}
		}
	}
	if (psNext != NULL)
	{
		blockZone = (psNext->flags & GWR_ZONE1) ? psNext->zone2 : psNext->zone1;
		DBP2((" %d", blockZone));
		for(tx = psNext->x1 - 1; tx <= psNext->x2 + 1; tx ++)
		{
			for(ty = psNext->y1 - 1; ty <= psNext->y2 + 1; ty ++)
			{
				if (!fpathPointInGateway(tx << TILE_SHIFT, ty << TILE_SHIFT, psNext) &&
					tileOnMap(tx,ty) && gwGetZone(tx,ty) == blockZone)
				{
					psTile = mapTile(tx, ty);
					psTile->tileInfoBits |= BITS_FPATHBLOCK;
				}
			}
		}
	}
	DBP2(("\n"));
}


// clear blocking flags for all gateways around a zone
void fpathClearGatewayBlock(SDWORD zone, GATEWAY *psLast, GATEWAY *psNext)
{
	GATEWAY		*psCurr;
	SDWORD		pos, tx,ty, blockZone;
	MAPTILE		*psTile;

	for(psCurr=psGateways; psCurr; psCurr=psCurr->psNext)
	{
		if (!(psCurr->flags & GWR_WATERLINK) &&
			((psCurr->zone1 == zone) || (psCurr->zone2 == zone)) )
		{
			if (psCurr->x1 == psCurr->x2)
			{
				for(pos = psCurr->y1; pos <= psCurr->y2; pos += 1)
				{
					psTile = mapTile(psCurr->x1, pos);
					psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
				}
			}
			else
			{
				for(pos = psCurr->x1; pos <= psCurr->x2; pos += 1)
				{
					psTile = mapTile(pos, psCurr->y1);
					psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
				}
			}
		}
	}
	// clear the flags around the route gateways
	if (psLast != NULL)
	{
		blockZone = (psLast->flags & GWR_ZONE1) ? psLast->zone1 : psLast->zone2;
		for(tx = psLast->x1 - 1; tx <= psLast->x2 + 1; tx ++)
		{
			for(ty = psLast->y1 - 1; ty <= psLast->y2 + 1; ty ++)
			{
				if (!fpathPointInGateway(tx << TILE_SHIFT, ty << TILE_SHIFT, psLast) &&
					tileOnMap(tx,ty) && gwGetZone(tx,ty) == blockZone)
				{
					psTile = mapTile(tx, ty);
					psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
				}
			}
		}
	}
	if (psNext != NULL)
	{
		blockZone = (psNext->flags & GWR_ZONE1) ? psNext->zone2 : psNext->zone1;
		for(tx = psNext->x1 - 1; tx <= psNext->x2 + 1; tx ++)
		{
			for(ty = psNext->y1 - 1; ty <= psNext->y2 + 1; ty ++)
			{
				if (!fpathPointInGateway(tx << TILE_SHIFT, ty << TILE_SHIFT, psNext) &&
					tileOnMap(tx,ty) && gwGetZone(tx,ty) == blockZone)
				{
					psTile = mapTile(tx, ty);
					psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
				}
			}
		}
	}
}


// clear the routing ignore flags for the gateways
void fpathClearIgnore(void)
{
	GATEWAY		*psCurr;
	SDWORD		link, numLinks;

	for(psCurr=psGateways; psCurr; psCurr=psCurr->psNext)
	{
		psCurr->flags &= ~GWR_IGNORE;
		numLinks = psCurr->zone1Links + psCurr->zone2Links;
		for (link = 0; link < numLinks; link += 1)
		{
			psCurr->psLinks[link].flags &= ~ GWRL_BLOCKED;
		}
	}
}

// find a clear tile on a gateway to route to
void fpathGatewayCoords(GATEWAY *psGate, SDWORD *px, SDWORD *py)
{
	SDWORD	x,y, dist, mx,my, pos;

	// find the clear tile nearest to the middle
	mx = (psGate->x1 + psGate->x2)/2;
	my = (psGate->y1 + psGate->y2)/2;
	dist = SDWORD_MAX;
	if (psGate->x1 == psGate->x2)
	{
		for(pos=psGate->y1;pos <= psGate->y2; pos+=1)
		{
			if (!fpathBlockingTile(psGate->x1,pos) &&
				(abs(pos - my) < dist))
			{
				x = psGate->x1;
				y = pos;
				dist = abs(pos - my);
			}
		}
	}
	else
	{
		for(pos=psGate->x1;pos <= psGate->x2; pos+=1)
		{
			if (!fpathBlockingTile(pos, psGate->y1) &&
				(abs(pos - mx) < dist))
			{
				x = pos;
				y = psGate->y1;
				dist = abs(pos - mx);
			}
		}
	}

	// if no clear tile is found just return the middle
	if (dist == SDWORD_MAX)
	{
		x = mx;
		y = my;
	}

	*px = (x * TILE_UNITS) + TILE_UNITS/2;
	*py = (y * TILE_UNITS) + TILE_UNITS/2;
}

// create a final route from a gateway route
#if 0
SDWORD fpathGatewayRouteOld(BASE_OBJECT *psObj, SDWORD routeMode, SDWORD GWTerrain,
						 SDWORD sx, SDWORD sy, SDWORD fx, SDWORD fy,
						 MOVE_CONTROL *psMoveCntl)
{
	static SDWORD	linkx,linky, gwx,gwy, asret, routex,routey;
	ASTAR_ROUTE		sAStarRoute;
	SDWORD			retval = FPR_OK, gwRet;
	static GATEWAY	*psCurrRoute, *psGWRoute, *psLastGW;
	BOOL			bRouting = TRUE;
	BOOL			firstRoute = TRUE;

	// keep trying gateway routes until out of options
	while (bRouting)
	{

		if (routeMode == ASR_NEWROUTE)
		{
			firstRoute = FALSE;

			DBP2(("Gateway route - droid %d\n", psObj->id));
			gwRet = gwrAStarRoute(psObj->player, GWTerrain,
								  sx,sy, fx,fy, &psGWRoute);
			switch (gwRet)
			{
			case GWR_OK:
				// initialise the move control structure
				psMoveCntl->numPoints = 0;
				break;
			case GWR_NEAREST:
				if (firstRoute)
				{
					// first time a route has been generated
					// initialise the move control structure
					psMoveCntl->numPoints = 0;
				}
				else
				{
					DBP2(("   GW route returned GWR_NEAREST for second route\n"));
					// can't find a better route than the last one
					if (psMoveCntl->numPoints > 0)
					{
						// return the last route as it got as near as you can
						retval = FPR_OK;
					}
					else
					{
						DBP2(("     no points - route failed\n"));
						retval = FPR_FAILED;
					}
					goto exit;
				}
				break;
			case GWR_NOZONE:
			case GWR_SAMEZONE:
				// no zone information - try a normal route
				psGWRoute = NULL;
				break;
			case GWR_FAILED:
				DBP2(("   Gateway route failed\n"));
				retval = FPR_FAILED;
				goto exit;
				break;
			}
		}


		// stop routing through any gateways which are not in the route
		fpathSetGatewayBlock();

		if (routeMode == ASR_NEWROUTE)
		{
			// if the start of the route is on the first gateway, skip it
			if ((psGWRoute != NULL) && fpathPointInGateway(sx,sy, psGWRoute))
			{
				psGWRoute = psGWRoute->psRoute;
			}

			linkx = sx;
			linky = sy;
			psCurrRoute = psGWRoute;
			psLastGW = NULL;
		}

		// now generate the route
		bRouting = FALSE;
		while (psCurrRoute != NULL)
		{
			// if the end of the route is on the last gateway, skip it
			if ((psCurrRoute->psRoute == NULL) && fpathPointInGateway(fx,fy, psCurrRoute))
			{
				break;
			}

	/*		gwx = (psCurrRoute->x1 + psCurrRoute->x2)/2;
			gwy = (psCurrRoute->y1 + psCurrRoute->y2)/2;

			gwx = gwx * TILE_UNITS + TILE_UNITS/2;
			gwy = gwy * TILE_UNITS + TILE_UNITS/2;*/
			fpathGatewayCoords(psCurrRoute, &gwx, &gwy);

			DBP2(("   astar route : (%d,%d) -> (%d,%d)\n",
				linkx>>TILE_SHIFT, linky>>TILE_SHIFT,
				gwx>>TILE_SHIFT, gwy>>TILE_SHIFT));
			asret = fpathAStarRoute(routeMode, &sAStarRoute, linkx,linky, gwx,gwy);
			routeMode = ASR_NEWROUTE;
			if ((asret == ASR_NEAREST) &&
				actionRouteBlockingPos((DROID *)psObj, sAStarRoute.finalX,sAStarRoute.finalY))
			{
				// found a blocking wall - route to that
				fpathAppendRoute(psMoveCntl, &sAStarRoute);
				retval = FPR_OK;
				goto exit;
			}
			else if ((asret == ASR_FAILED) ||
					 (asret == ASR_NEAREST))
			{
				// no route found - try ditching this gateway
				// and trying a new gateway route
				DBP2(("   Route failed - trying new gatway route\n"));
				psCurrRoute->flags |= GWR_IGNORE;
				bRouting = TRUE;
				fpathClearGatewayBlock();
				break;
			}
			else if (asret == ASR_PARTIAL)
			{
				// routing hasn't finished yet
				DBP2(("   Reschedule\n"));
				retval = FPR_WAIT;
				goto exit;
			}

			fpathAppendRoute(psMoveCntl, &sAStarRoute);

			linkx = gwx;
			linky = gwy;

			psLastGW = psCurrRoute;
			psCurrRoute = psCurrRoute->psRoute;
		}

		// only finish off if no new gateway route is going to be generated
		if (!bRouting)
		{
			asret = fpathAStarRoute(routeMode,&sAStarRoute, linkx,linky, fx,fy);
			routeMode = ASR_NEWROUTE;
			if ((asret == ASR_NEAREST) &&
				actionRouteBlockingPos((DROID *)psObj, sAStarRoute.finalX,sAStarRoute.finalY))
			{
				// found a blocking wall - route to that
				fpathAppendRoute(psMoveCntl, &sAStarRoute);
				retval = FPR_OK;
				goto exit;
			}
			else if ((psLastGW != NULL) &&
				((asret == ASR_FAILED) ||
				 (asret == ASR_NEAREST)))
			{
				// no route found - try ditching the last gateway
				// and trying a new gateway route
				DBP2(("   Route failed - trying new gatway route\n"));
				psLastGW->flags |= GWR_IGNORE;
				bRouting = TRUE;
				fpathClearGatewayBlock();
			}
			else if (asret == ASR_FAILED)
			{
				DBP2(("   Final route failed\n"));
				retval = FPR_FAILED;
				goto exit;
			}
			else if (asret == ASR_PARTIAL)
			{
				// routing hasn't finished yet
				DBP2(("   Reschedule\n"));
				retval = FPR_WAIT;
				goto exit;
			}

			fpathAppendRoute(psMoveCntl, &sAStarRoute);
		}
	}

exit:
	// reset the routing block flags
	fpathClearGatewayBlock();
	if (retval != FPR_WAIT)
	{
		fpathClearIgnore();
	}

	return retval;
}
#endif


// check if the route to a gateway has already been generated
BOOL fpathCheckRouteMatch(MOVE_CONTROL *psMoveCntl, SDWORD *pPos, SDWORD gwx,SDWORD gwy)
{
	SDWORD	pos = *pPos;

	while (pos < psMoveCntl->numPoints)
	{
		if ((psMoveCntl->asPath[pos].x == (gwx >> TILE_SHIFT)) &&
			(psMoveCntl->asPath[pos].y == (gwy >> TILE_SHIFT)))
		{
			*pPos = pos + 1;
			return TRUE;
		}
		pos += 1;
	}

	return FALSE;
}

void fpathBlockGatewayLink(GATEWAY *psLast, GATEWAY *psCurr)
{
	SDWORD	link, numLinks;

	if ((psLast == NULL) && (psCurr != NULL))
	{
		DBP2(("   Blocking first gateway\n"));
		psCurr->flags |= GWR_IGNORE;
	}
	else if ((psCurr == NULL) && (psLast != NULL))
	{
		DBP2(("   Blocking last gateway\n"));
		psLast->flags |= GWR_IGNORE;
	}
	else if ((psLast != NULL) && (psCurr != NULL))
	{
		DBP2(("   Blocking link between gateways"));
		numLinks = psLast->zone1Links + psLast->zone2Links;
		for(link = 0; link < numLinks; link += 1)
		{
//			if (psLast->psLinks[link].psGateway == psCurr)
			if (psLast->psLinks[link].flags & GWRL_CHILD)
			{
				DBP2((" last link %d", link));
				psLast->psLinks[link].flags |= GWRL_BLOCKED;
			}
		}
		numLinks = psCurr->zone1Links + psCurr->zone2Links;
		for(link = 0; link < numLinks; link += 1)
		{
//			if (psCurr->psLinks[link].psGateway == psLast)
			if (psCurr->psLinks[link].flags & GWRL_PARENT)
			{
				DBP2((" curr link %d", link));
				psCurr->psLinks[link].flags |= GWRL_BLOCKED;
			}
		}
		DBP2(("\n"));
	}
}


// check if a new route is closer to the target than the one stored in
// the droid
BOOL fpathRouteCloser(MOVE_CONTROL *psMoveCntl, ASTAR_ROUTE *psAStarRoute, SDWORD tx,SDWORD ty)
{
	SDWORD	xdiff,ydiff, prevDist, nextDist;

	if (psAStarRoute->numPoints == 0)
	{
		// no route to copy do nothing
		return FALSE;
	}

	if (psMoveCntl->numPoints == 0)
	{
		// no previous route - this has to be better
		return TRUE;
	}

	// see which route is closest to the final destination
	xdiff = (psMoveCntl->asPath[psMoveCntl->numPoints - 1].x << TILE_SHIFT) + TILE_UNITS/2 - tx;
	ydiff = (psMoveCntl->asPath[psMoveCntl->numPoints - 1].y << TILE_SHIFT) + TILE_UNITS/2 - ty;
	prevDist = xdiff*xdiff + ydiff*ydiff;

	xdiff = (psAStarRoute->finalX << TILE_SHIFT) + TILE_UNITS/2 - tx;
	ydiff = (psAStarRoute->finalY << TILE_SHIFT) + TILE_UNITS/2 - ty;
	nextDist = xdiff*xdiff + ydiff*ydiff;

	if (nextDist < prevDist)
	{
		return TRUE;
	}

	return FALSE;
}

// create a final route from a gateway route
SDWORD fpathGatewayRoute(BASE_OBJECT *psObj, SDWORD routeMode, SDWORD GWTerrain,
						 SDWORD sx, SDWORD sy, SDWORD fx, SDWORD fy,
						 MOVE_CONTROL *psMoveCntl)
{
	static SDWORD	linkx,linky, gwx,gwy, asret, routex,routey, matchPoints;
	static ASTAR_ROUTE		sAStarRoute;
	SDWORD			retval = FPR_OK, gwRet, zone;
	static GATEWAY	*psCurrRoute, *psGWRoute, *psLastGW;
	BOOL			bRouting, bFinished;
	static BOOL		bFirstRoute;

	if (routeMode == ASR_NEWROUTE)
	{
		fpathClearIgnore();
		// initialise the move control structures
		psMoveCntl->numPoints = 0;
		sAStarRoute.numPoints = 0;
		bFirstRoute = TRUE;
	}

	// keep trying gateway routes until out of options
	bRouting = TRUE;
	while (bRouting)
	{
		if (routeMode == ASR_NEWROUTE)
		{
			DBP2(("Gateway route - droid %d\n", psObj->id));
			gwRet = gwrAStarRoute(psObj->player, GWTerrain,
								  sx,sy, fx,fy, &psGWRoute);
			switch (gwRet)
			{
			case GWR_OK:
				break;
			case GWR_NEAREST:
				// need to deal with this case for retried routing - only accept this if no previous route?
				if (!bFirstRoute)
				{
					if (psMoveCntl->numPoints > 0)
					{
						DBP2(("   Gateway route nearest - Use previous route\n"));
						retval = FPR_OK;
						goto exit;
					}
					else
					{
						DBP2(("   Gateway route nearest - No points - failed\n"));
						retval = FPR_FAILED;
						goto exit;
					}
				}
				break;
			case GWR_NOZONE:
			case GWR_SAMEZONE:
				// no zone information - try a normal route
				psGWRoute = NULL;
				break;
			case GWR_FAILED:
				DBP2(("   Gateway route failed\n"));
				if ((psObj->type == OBJ_DROID) && vtolDroid((DROID *)psObj))
				{
					// just fail for VTOLs - they can set a direct route
					retval = FPR_FAILED;
					goto exit;
				}
				else
				{
					psGWRoute = NULL;
				}
				break;
			}

			// reset matchPoints so that routing between gateways generated
			// by the previous gateway route can be reused
			matchPoints = 0;
			sAStarRoute.numPoints = 0;
		}
		bFirstRoute = FALSE;

		if (routeMode == ASR_NEWROUTE)
		{
			// if the start of the route is on the first gateway, skip it
			if ((psGWRoute != NULL) && fpathPointInGateway(sx,sy, psGWRoute))
			{
				psGWRoute = psGWRoute->psRoute;
			}

			linkx = sx;
			linky = sy;
			psCurrRoute = psGWRoute;
			psLastGW = NULL;
		}

		// now generate the route
		bRouting = FALSE;
		bFinished = FALSE;
		while (!bFinished)
		{
			if ((psCurrRoute == NULL) ||
				((psCurrRoute->psRoute == NULL) && fpathPointInGateway(fx,fy, psCurrRoute)))
			{
				// last stretch on the route is not to a gatway but to
				// the final route coordinates
				gwx = fx;
				gwy = fy;
				zone = gwGetZone(fx >> TILE_SHIFT, fy >> TILE_SHIFT);
			}
			else
			{
				fpathGatewayCoords(psCurrRoute, &gwx, &gwy);
				zone = psCurrRoute->flags & GWR_ZONE1 ? psCurrRoute->zone1 : psCurrRoute->zone2;
			}

			// only route between the gateways if it wasn't done on a previous route
//			if (!fpathCheckRouteMatch(psMoveCntl, &matchPoints, gwx,gwy))
//			if (1)
			{
//				psMoveCntl->numPoints = (UBYTE)matchPoints;

				DBP2(("   astar route : (%d,%d) -> (%d,%d) zone %d\n",
					linkx>>TILE_SHIFT, linky>>TILE_SHIFT,
					gwx>>TILE_SHIFT, gwy>>TILE_SHIFT, zone));
				fpathSetGatewayBlock(zone, psLastGW, psCurrRoute);
				asret = fpathAStarRoute(routeMode, &sAStarRoute, linkx,linky, gwx,gwy);
				fpathClearGatewayBlock(zone, psLastGW, psCurrRoute);
				if (asret == ASR_PARTIAL)
				{
					// routing hasn't finished yet
					DBP2(("   Reschedule\n"));
					retval = FPR_WAIT;
					goto exit;
				}
/*				else if (asret != ASR_FAILED)
				{
					fpathAppendRoute(psMoveCntl, &sAStarRoute);
					matchPoints = psMoveCntl->numPoints;
				}*/
				routeMode = ASR_NEWROUTE;

				if ((asret == ASR_NEAREST) &&
					actionRouteBlockingPos((DROID *)psObj, sAStarRoute.finalX,sAStarRoute.finalY))
				{
					// found a blocking wall - route to that
					DBP2(("   Got blocking wall\n"));
					retval = FPR_OK;
					goto exit;
				}
				else if ((asret == ASR_NEAREST) && (psGWRoute == NULL))
				{
					// all routing was in one zone - this is as good as it's going to be
					DBP2(("   Nearest route in same zone\n"));
					if (fpathRouteCloser(psMoveCntl, &sAStarRoute, fx,fy))
					{
						psMoveCntl->numPoints = 0;
						fpathAppendRoute(psMoveCntl, &sAStarRoute);
					}
					retval = FPR_OK;
					goto exit;
				}
				else if ((asret == ASR_FAILED) && (psGWRoute == NULL))
				{
					// all routing was in one zone - can't retry
					DBP2(("   Failed route in same zone\n"));
					retval = FPR_FAILED;
					goto exit;
				}
				else if ((asret == ASR_FAILED) ||
						 (asret == ASR_NEAREST))
				{
					// no route found - try ditching this gateway
					// and trying a new gateway route
					DBP2(("   Route failed - ignore gateway/link and reroute\n"));
					if (fpathRouteCloser(psMoveCntl, &sAStarRoute, fx,fy))
					{
						psMoveCntl->numPoints = 0;
						fpathAppendRoute(psMoveCntl, &sAStarRoute);
					}
					fpathBlockGatewayLink(psLastGW, psCurrRoute);
					bRouting = TRUE;
					break;
				}
			}
#ifdef DEBUG_GROUP2
/*			else
			{
				DBP2(("   matched previous route : (%d,%d) -> (%d,%d)\n",
					linkx>>TILE_SHIFT, linky>>TILE_SHIFT,
					gwx>>TILE_SHIFT, gwy>>TILE_SHIFT));
			}*/
#endif

			linkx = gwx;
			linky = gwy;

			psLastGW = psCurrRoute;
			if (psCurrRoute != NULL)
			{
				psCurrRoute = psCurrRoute->psRoute;
			}
			else
			{
				bFinished = TRUE;
			}
		}
	}

	if (fpathRouteCloser(psMoveCntl, &sAStarRoute, fx,fy))
	{
		psMoveCntl->numPoints = 0;
		fpathAppendRoute(psMoveCntl, &sAStarRoute);
	}

exit:
	// reset the routing block flags
	if (retval != FPR_WAIT)
	{
		fpathClearIgnore();
	}

	return retval;
}

/* set pointer for current fpath object - GJ hack */
void fpathSetCurrentObject( BASE_OBJECT *psObj )
{
	g_psObjRoute = psObj;
}

// set the correct blocking tile function
void fpathSetBlockingTile( UBYTE ubPropulsionType )
{
#ifndef TEST_BED
	switch ( ubPropulsionType )
	{
	case HOVER:
		fpathBlockingTile = fpathHoverBlockingTile;
		break;
	case LIFT:
		fpathBlockingTile = fpathLiftBlockingTile;
		break;
	default:
		fpathBlockingTile = fpathGroundBlockingTile;
	}
#else
	fpathBlockingTile = fpathGroundBlockingTile;
#endif
}


// Find a route for an object to a location
FPATH_RETVAL fpathRoute(BASE_OBJECT *psObj, MOVE_CONTROL *psMoveCntl,
						SDWORD tX, SDWORD tY)
{
	SDWORD				startX,startY, targetX,targetY;
	SDWORD				x,y;
	SDWORD				dir, nearestDir, minDist, tileDist;
	FPATH_RETVAL		retVal = FPR_OK;
//	DROID				*psCurr;
	DROID				*psDroid;
	PROPULSION_STATS	*psPropStats;
	UDWORD				GWTerrain;


	/* set global pointer for object being routed - GJ hack */
	fpathSetCurrentObject( psObj );

	if ((psPartialRouteObj == NULL) ||
		(psPartialRouteObj != psObj))
	{
		targetX = tX;
		targetY = tY;
		startX = (SDWORD)psObj->x;
		startY = (SDWORD)psObj->y;
	}
	else if (psObj->type == OBJ_DROID &&
			 ((DROID *)psObj)->sMove.Status == MOVEWAITROUTE &&
			 (((DROID *)psObj)->sMove.DestinationX != tX ||
			  ((DROID *)psObj)->sMove.DestinationX != tX))
	{
		psPartialRouteObj = NULL;
		targetX = tX;
		targetY = tY;
		startX = (SDWORD)psObj->x;
		startY = (SDWORD)psObj->y;
	}
	else
	{
		// continuing routing for the object
		startX = partialSX;
		startY = partialSY;
		targetX = partialTX;
		targetY = partialTY;
	}

	// don't have to do anything if already there
	if (startX == targetX && startY == targetY)
	{
		// return failed to stop them moving anywhere
		DBP0(("Unit %d: route failed (same pos)\n", psDroid->id));
		return FPR_FAILED;
	}

	// set the correct blocking tile function and gateway terrain flag
	if (psObj->type == OBJ_DROID)
	{
		psDroid = (DROID *)psObj;
		psPropStats = asPropulsionStats + psDroid->asBits[COMP_PROPULSION].nStat;
		ASSERT( (PTRVALID(psPropStats, sizeof(PROPULSION_STATS)),
			"fpathRoute: invalid propulsion stats pointer") );

		fpathSetBlockingTile( psPropStats->propulsionType );

		/* set gateway terrain flag */
		switch ( psPropStats->propulsionType )
		{
			case HOVER:
				GWTerrain = GWR_TER_ALL;
				break;

			case LIFT:
				GWTerrain = GWR_TER_ALL;
				break;

			default:
				GWTerrain = GWR_TER_LAND;
				break;
		}
	}
	else
	{
		GWTerrain = GWR_TER_LAND;
	}

#ifndef TEST_BED
	// set all the flags for stationary droids
/*	for(psCurr = apsDroidLists[psObj->player]; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr != (DROID *)psObj &&
			!psCurr->selected &&
			psCurr->sMove.Status == MOVEINACTIVE)
		{
			psTile = mapTile(psCurr->x >> TILE_SHIFT, psCurr->y >> TILE_SHIFT);
			psTile->tileInfoBits |= BITS_FPATHBLOCK;
		}
	}*/
#endif

	if ((psPartialRouteObj == NULL) ||
		(psPartialRouteObj != psObj))
	{
		// check whether the start point of the route
		// is a blocking tile and find an alternative if it is
		if (fpathBlockingTile(startX >> TILE_SHIFT, startY >> TILE_SHIFT))
		{
			// find the nearest non blocking tile to the object
			minDist = SDWORD_MAX;
			nearestDir = NUM_DIR;
			for(dir=0; dir<NUM_DIR; dir++)
			{
				x = (startX>>TILE_SHIFT) + aDirOffset[dir].x;
				y = (startY>>TILE_SHIFT) + aDirOffset[dir].y;
				if (!fpathBlockingTile(x,y))
				{
					tileDist = fpathDistToTile(x,y, startX,startY);
					if (tileDist < minDist)
					{
						minDist = tileDist;
						nearestDir = dir;
					}
				}
			}

			if (nearestDir == NUM_DIR)
			{
				// surrounded by blocking tiles, give up
				retVal = FPR_FAILED;
				DBP0(("Unit %d: route failed (surrouned by blocking)\n", psDroid->id));
				goto exit;
			}
			else
			{
				startX = (((startX>>TILE_SHIFT) + aDirOffset[nearestDir].x) << TILE_SHIFT)
							+ TILE_SHIFT/2;
				startY = (((startY>>TILE_SHIFT) + aDirOffset[nearestDir].y) << TILE_SHIFT)
							+ TILE_SHIFT/2;
			}
		}

		// initialise the raycast - if there is los to the target, no routing necessary
		finalX = targetX & ~TILE_MASK;
		finalX += TILE_UNITS/2;
		finalY = targetY & ~TILE_MASK;
		finalY += TILE_UNITS/2;

		clearX = finalX; clearY = finalY;
		vectorX = startX - finalX;
		vectorY = startY - finalY;
		obstruction = FALSE;

		// cast the ray to find the last clear tile before the obstruction
		rayCast(startX,startY, rayPointsToAngle(startX,startY, finalX, finalY),
				RAY_MAXLEN, fpathEndPointCallback);

		if (!obstruction)
		{
			// no obstructions - trivial route
			fpathSetDirectRoute( psObj, targetX, targetY );
			retVal = FPR_OK;
			DBP0(("Unit %d: trivial route\n", psDroid->id));
			if (psPartialRouteObj != NULL)
			{
				DBP0(("Unit %d: trivial route during multi-frame route\n"));
			}
			goto exit;
		}

		// check whether the end point of the route
		// is a blocking tile and find an alternative if it is
		if (fpathBlockingTile(targetX >> TILE_SHIFT, targetY >> TILE_SHIFT))
		{
			// route to the last clear tile found by the raycast
			targetX = clearX;
			targetY = clearY;
		}

		// see if there is another unit with a usable route
		if (fpathFindRoute((DROID *)psDroid, startX,startY, targetX,targetY))
		{
			DBP0(("Unit %d: found route\n", psDroid->id));
			if (psPartialRouteObj != NULL)
			{
				DBP0(("Unit %d: found route during multi-frame route\n"));
			}
			goto exit;
		}
	}

	ASSERT((startX >= 0 && startX < (SDWORD)mapWidth*TILE_UNITS &&
			startY >= 0 && startY < (SDWORD)mapHeight*TILE_UNITS,
			"fpathRoute: start coords off map"));
	ASSERT((targetX >= 0 && targetX < (SDWORD)mapWidth*TILE_UNITS &&
			targetY >= 0 && targetY < (SDWORD)mapHeight*TILE_UNITS,
			"fpathRoute: target coords off map"));
	ASSERT((fpathBlockingTile == fpathGroundBlockingTile ||
			fpathBlockingTile == fpathHoverBlockingTile ||
			fpathBlockingTile == fpathLiftBlockingTile,
			"fpathRoute: invalid blocking function"));

	if (astarInner > FPATH_LOOP_LIMIT)
	{
		if (psPartialRouteObj == psObj)
		{
			retVal = FPR_WAIT;
			goto exit;
		}
		else
		{
			DBP0(("Unit %d: reschedule\n"));
			retVal = FPR_RESCHEDULE;
			goto exit;
		}
	}
	else if ( ((psPartialRouteObj != NULL) &&
			   (psPartialRouteObj != psObj)) ||
			  ((psPartialRouteObj != psObj) &&
			   (psNextRouteDroid != NULL) &&
			   (psNextRouteDroid != (DROID *)psObj)) )
	{
		retVal = FPR_RESCHEDULE;
		goto exit;
	}
	
	DBP0(("Unit %d: ", psObj->id));
	if (psPartialRouteObj == NULL)
	{
		retVal = fpathGatewayRoute(psObj, ASR_NEWROUTE, GWTerrain,
						startX,startY, targetX,targetY, psMoveCntl);
	}
	else
	{
//		DBPRINTF(("Partial Route: %d\n", psDroid->id));
		psPartialRouteObj = NULL;
		retVal = fpathGatewayRoute(psObj, ASR_CONTINUE, GWTerrain,
						startX,startY, targetX,targetY, psMoveCntl);
	}
	if (retVal == FPR_WAIT)
	{
		psPartialRouteObj = psObj;
		lastPartialFrame = frameGetFrameNumber();
		partialSX = startX;
		partialSY = startY;
		partialTX = targetX;
		partialTY = targetY;
	}
	else if ((retVal == FPR_FAILED) &&
			 (psObj->type == OBJ_DROID) && vtolDroid((DROID *)psObj))
	{
		fpathSetDirectRoute( psObj, targetX, targetY );
		retVal = FPR_OK;
	}

exit:

#ifndef TEST_BED
	// reset all the droid flags
/*	for(psCurr = apsDroidLists[psObj->player]; psCurr; psCurr = psCurr->psNext)
	{
		if (psCurr->sMove.Status == MOVEINACTIVE)
		{
			psTile = mapTile(psCurr->x >> TILE_SHIFT, psCurr->y >> TILE_SHIFT);
			psTile->tileInfoBits &= ~BITS_FPATHBLOCK;
		}
	}*/
#endif

	// reset the blocking tile function
	fpathBlockingTile = fpathGroundBlockingTile;

	/* reset global pointer for object being routed */
	fpathSetCurrentObject( NULL );

#if defined(JOHN) && defined(DEBUG)
	{
		MAPTILE				*psTile;

		psTile = psMapTiles;
		for(x=0; x<(SDWORD)(mapWidth*mapHeight); x+= 1)
		{
			if (psTile->tileInfoBits & BITS_FPATHBLOCK)
			{
				ASSERT((FALSE,"fpathRoute: blocking flags still in the map"));
			}
			psTile += 1;
		}
	}
#endif

#ifdef DEBUG_GROUP1
	{
		SDWORD	pos;

		DBP1(("Waypoints:"));

		for(pos = 0; pos < psMoveCntl->numPoints; pos += 1)
		{
			DBP1(("  (%d,%d)",
				psMoveCntl->asPath[pos].x,
				psMoveCntl->asPath[pos].y));
		}
		DBP1(("\n"));
	}
#endif

	return retVal;
}


// find the first point on the route which has both droids on the same side of it
BOOL fpathFindFirstRoutePoint(MOVE_CONTROL *psMove, SDWORD *pIndex, SDWORD x1,SDWORD y1, SDWORD x2,SDWORD y2)
{
	SDWORD	vx1,vy1, vx2,vy2;

	for(*pIndex = 0; *pIndex < psMove->numPoints; (*pIndex) ++)
	{
		vx1 = x1 - psMove->asPath[ *pIndex ].x;
		vy1 = y1 - psMove->asPath[ *pIndex ].y;
		vx2 = x2 - psMove->asPath[ *pIndex ].x;
		vy2 = y2 - psMove->asPath[ *pIndex ].y;

		// found it if the dot products have the same sign
		if ( (vx1 * vx2 + vy1 * vy2) < 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

// See if there is another unit on your side that has a route this unit can use
BOOL fpathFindRoute(DROID *psDroid, SDWORD sX,SDWORD sY, SDWORD tX,SDWORD tY)
{
	FORMATION	*psFormation;
	DROID		*psCurr;
	SDWORD		i, startX,startY, index;

	if (!formationFind(&psFormation, tX,tY))
	{
		return FALSE;
	}

	// now look for a unit in this formation with a route that can be used
	for(psCurr = apsDroidLists[psDroid->player]; psCurr; psCurr = psCurr->psNext)
	{
		if ((psCurr != psDroid) &&
			(psCurr != (DROID *)psPartialRouteObj) &&
			(psCurr->sMove.psFormation == psFormation) &&
			(psCurr->sMove.numPoints > 0))
		{
			// find the first route point
			if (!fpathFindFirstRoutePoint(&psCurr->sMove, &index, sX,sY, (SDWORD)psCurr->x, (SDWORD)psCurr->y))
			{
				continue;
			}

			// initialise the raycast - if there is los to the start of the route
			startX = (sX & ~TILE_MASK) + TILE_UNITS/2;
			startY = (sY & ~TILE_MASK) + TILE_UNITS/2;
			finalX = (psCurr->sMove.asPath[index].x * TILE_UNITS) + TILE_UNITS/2;
			finalY = (psCurr->sMove.asPath[index].y * TILE_UNITS) + TILE_UNITS/2;

			clearX = finalX; clearY = finalY;
			vectorX = startX - finalX;
			vectorY = startY - finalY;
			obstruction = FALSE;

			// cast the ray to find the last clear tile before the obstruction
			rayCast(startX,startY, rayPointsToAngle(startX,startY, finalX, finalY),
					RAY_MAXLEN, fpathEndPointCallback);

			if (!obstruction)
			{
				// This route is OK, copy it over
				for(i=index; i<psCurr->sMove.numPoints; i++)
				{
					psDroid->sMove.asPath[i] = psCurr->sMove.asPath[i];
				}
				psDroid->sMove.numPoints = psCurr->sMove.numPoints;

				// now see if the route 

				return TRUE;
			}
		}
	}


	return FALSE;
}
