/*
 * RayCast.c
 *
 * raycasting routine that gives intersection points with map tiles
 *
 */


#include <math.h>
#include <stdio.h>

#include "Frame.h"

#include "Objects.h"
#include "Map.h"

#include "RayCast.h"
#include "Display3d.h"
//#ifdef ALEXM
#include "Effects.h"
//#endif
#ifdef PSX
#include "DCache.h"
#endif

// accuracy for the raycast lookup tables
#define RAY_ACC		12

#define RAY_ACCMUL	(1<<RAY_ACC)

// Get the tile from tile coords
#define RAY_TILE(x,y) mapTile((x),(y))

// control the type of clip method
// 0 - clip on ray length (faster but it doesn't always work :-)
// 1 - clip on coordinates (accurate but possibly a bit slower)
#define RAY_CLIP	1

// ray point
typedef struct _ray_point
{
	SDWORD	x,y;
} RAY_POINT;

/* x and y increments for each ray angle */
static SDWORD	rayDX[NUM_RAYS], rayDY[NUM_RAYS];
static SDWORD	rayHDist[NUM_RAYS], rayVDist[NUM_RAYS];
//static FRACT	rayTan[NUM_RAYS], rayCos[NUM_RAYS], raySin[NUM_RAYS];
static SDWORD	rayFPTan[NUM_RAYS], rayFPInvTan[NUM_RAYS];
static SDWORD	rayFPInvCos[NUM_RAYS], rayFPInvSin[NUM_RAYS];

#define MAX_FRACT (0x7fffffff)
#define angle_PSX2WORLD(ang) ((((ang)%4096)*360)/4096)

/* Initialise the ray tables */
#ifdef WIN32
BOOL rayInitialise(void)
{
	SDWORD	i;
	FRACT	angle = MAKEFRACT(0);
	FRACT	val;

	for(i=0; i<NUM_RAYS; i++)
	{
		// Set up the fixed offset tables for calculating the intersection points
		val = (float)tan(angle);

		rayDX[i] = (SDWORD)(TILE_UNITS * RAY_ACCMUL * val);

		if (i <= NUM_RAYS/4 ||
			(i >= 3*NUM_RAYS/4))
		{
			rayDX[i] = -rayDX[i];
		}

		if(val == 0) {
			val = (FRACT)1;	// Horrible hack to avoid divide by zero.
		}

		rayDY[i] = (SDWORD)(TILE_UNITS * RAY_ACCMUL / val);
		if (i >= NUM_RAYS/2)
		{
			rayDY[i] = -rayDY[i];
		}

		// These are used to calculate the initial intersection
		rayFPTan[i] = MAKEINT(FRACTmul(val, MAKEFRACT(RAY_ACCMUL)));
		rayFPInvTan[i] = MAKEINT(FRACTdiv(MAKEFRACT(RAY_ACCMUL), val));

		// Set up the trig tables for calculating the offset distances
		val = (float)sin(angle);
		if(val == 0) {
			val = (FRACT)1;
		}
		rayFPInvSin[i] = MAKEINT(FRACTdiv(MAKEFRACT(RAY_ACCMUL), val));
		if (i >= NUM_RAYS/2)
		{
			rayVDist[i] = MAKEINT(FRACTdiv(MAKEFRACT(-TILE_UNITS), val));
		}
		else
		{
			rayVDist[i] = MAKEINT(FRACTdiv(MAKEFRACT(TILE_UNITS), val));
		}

		val = (float)cos(angle);
		if(val == 0) {
			val = (FRACT)1;
		}
		rayFPInvCos[i] = MAKEINT(FRACTdiv(MAKEFRACT(RAY_ACCMUL), val));
		if (i < NUM_RAYS/4 || i > 3*NUM_RAYS/4)
		{
			rayHDist[i] = MAKEINT(FRACTdiv(MAKEFRACT(TILE_UNITS), val));
		}
		else
		{
			rayHDist[i] = MAKEINT(FRACTdiv(MAKEFRACT(-TILE_UNITS), val));
		}

		angle += RAY_ANGLE;
	}

	return TRUE;
}

#else	// Start of PSX version.

BOOL rayInitialise(void)
{
	SDWORD	i;
	FRACT	angle = MAKEFRACT(0);
	FRACT	val;

	for(i=0; i<NUM_RAYS; i++)
	{
		// Set up the fixed offset tables for calculating the intersection points

		angle = (i*4096) / NUM_RAYS;

		val = rcos(angle);
		if(val != 0) {
			val = (rsin(angle)*4096) / val;
		} else {
			val = MAX_FRACT;
		}



		rayDX[i] = (SDWORD)(TILE_UNITS * (RAY_ACCMUL / 4096) * val);

//		rayDX[i] = (SDWORD)(TILE_UNITS * RAY_ACCMUL * val);
		if (i <= NUM_RAYS/4 ||
			(i >= 3*NUM_RAYS/4))
		{
			rayDX[i] = -rayDX[i];
		}
		if(val == 0) {
			val = (FRACT)1;	// Horrible hack to avoid divide by zero.
		}


#define ACC_LOST (2)	// we this fact from the calc to make all the calcs fit in 32 signed bits
		{
			SDWORD top;
			SDWORD bot;

			top = (TILE_UNITS * RAY_ACCMUL * (4096 / ACC_LOST) );
			bot=val/ACC_LOST;

			if (bot==0)	// divide by zero check
			{
				if (top>=0)
				{
					rayDY[i] = (SDWORD)(SDWORD_MAX);
				}
				else
				{
					rayDY[i] = (SDWORD)(SDWORD_MIN);
					
				}
			}
			else
			{
				rayDY[i] = (SDWORD)(top / bot);
			}

			
		}	


//		rayDY[i] = (SDWORD)(TILE_UNITS * RAY_ACCMUL / val);
		if (i >= NUM_RAYS/2)
		{
			rayDY[i] = -rayDY[i];
		}

		// These are used to calculate the initial intersection
		rayFPTan[i] = val;
		rayFPInvTan[i] = RAY_ACCMUL*4096 / val;

		// Set up the trig tables for calculating the offset distances
		val = rsin(angle);
		if(val == 0) {
			val = (FRACT)1;
		}

		rayFPInvSin[i] = RAY_ACCMUL*4096 / val;
		if (i >= NUM_RAYS/2)
		{
			rayVDist[i] = (-TILE_UNITS*4096) / val;
		}
		else
		{
			rayVDist[i] = TILE_UNITS*4096 / val;
		}

		val = rcos(angle);

		if(val == 0) {
			val = (FRACT)1;
		}

		rayFPInvCos[i] = RAY_ACCMUL*4096 /  val;
		if (i < NUM_RAYS/4 || i > 3*NUM_RAYS/4)
		{
			rayHDist[i] = TILE_UNITS*4096 / val;
		}
		else
		{
			rayHDist[i] = (-TILE_UNITS*4096) / val;
		}

//		DBPRINTF(("%d Tan %d InvTan %d ",i,rayFPTan[i],rayFPInvTan[i]);
//		DBPRINTF(("InvSin %d VDist %d ",rayFPInvSin[i],rayVDist[i]);
//		DBPRINTF(("InvCos %d HDist %d ",rayFPInvCos[i],rayHDist[i]);
//		DBPRINTF(("rayDX %d rayDY %d\n",rayDX[i],rayDY[i]);
	}

	return TRUE;
}

#endif

//void rayC(UDWORD x, UDWORD y, UDWORD ray, UDWORD length, RAY_CALLBACK callback);
//
////#ifdef WIN32
//
//void rayCast(UDWORD x, UDWORD y, UDWORD ray, UDWORD length, RAY_CALLBACK callback)
//{
//	rayC(x, y, ray, length, callback);
//}

//#else
//
//void rayCast(UDWORD x, UDWORD y, UDWORD ray, UDWORD length, RAY_CALLBACK callback)
//{
//	static UDWORD Tx;
//	static UDWORD Ty;
//	static UDWORD Tray;
//	static UDWORD Tlength;
//	static RAY_CALLBACK Tcallback;
//
//	Tx = x;
//	Ty = y;
//	Tray = ray;
//	Tlength = length;
//	Tcallback = callback;
//	// Stack in the DCache.
//	SetSpDCache();
//	rayC(Tx, Ty, Tray, Tlength, Tcallback);
//	SetSpNormal();
//}
//
//#endif

/* cast a ray from x,y (world coords) at angle ray (0-360)
 * The ray angle starts at zero along the positive y axis and
 * increases towards -ve X.
 *
 * Sorry about the wacky angle set up but that was what I thought
 * warzone used, but turned out not to be after I wrote it.
 */
void rayCast(UDWORD x, UDWORD y, UDWORD ray, UDWORD length, RAY_CALLBACK callback)
{
	SDWORD		hdInc=0, vdInc=0;		// increases in x and y distance per intersection
	SDWORD		hDist, vDist;		// distance to current horizontal and vertical intersections
	RAY_POINT	sVert, sHoriz;
	SDWORD		vdx=0, hdy=0;			// vertical x increment, horiz y inc
#if RAY_CLIP == 0
	SDWORD		newLen, clipLen;	// ray length after clipping
#endif

	// Clipping is done with the position offset by TILE_UNITS/4 to account 
	// for the rounding errors when the intersection length is calculated.
	// Bit of a hack but I'm pretty sure it doesn't let through anything
	// that should be clippped.

#if RAY_CLIP == 0
	// Initial clip length is just the length of the ray
	clipLen = (SDWORD)length;
#endif

	// initialise the horizontal intersection calculations
	// and clip to the top and bottom of the map
	// (no horizontal intersection for a horizontal ray)
	if (ray != NUM_RAYS/4 && ray != 3*NUM_RAYS/4)
	{
		if (ray < NUM_RAYS/4 || ray > 3*NUM_RAYS/4)
		{
			// intersection
			sHoriz.y = (y & ~TILE_MASK) + TILE_UNITS;
			hdy = TILE_UNITS;

#if RAY_CLIP == 0
			// clipping
			newLen = (((mapHeight << TILE_SHIFT) - ((SDWORD)y + TILE_UNITS/4))
						* rayFPInvCos[ray])	>> RAY_ACC;
			if (newLen < clipLen)
			{
				clipLen = newLen;
			}
#endif
		}
		else
		{
			// intersection
			sHoriz.y = (y & ~TILE_MASK) - 1;
			hdy = -TILE_UNITS;

#if RAY_CLIP == 0
			// clipping
			newLen = ((TILE_UNITS/4 - (SDWORD)y) * rayFPInvCos[ray]) >> RAY_ACC;
			if (newLen < clipLen)
			{
				clipLen = newLen;
			}
#endif
		}

		// Horizontal x is kept in fixed point form until passed to the callback
		// to avoid rounding errors
		// Horizontal y is in integer form all the time
		sHoriz.x = (x << RAY_ACC) + (((SDWORD)y-sHoriz.y) * rayFPTan[ray]);

		// Set up the distance calculations
		hDist = ((sHoriz.y - (SDWORD)y) * rayFPInvCos[ray]) >> RAY_ACC;
		hdInc = rayHDist[ray];
	}
	else
	{
		// ensure no horizontal intersections are calculated
		hDist = length;
	}

	// initialise the vertical intersection calculations
	// and clip to the left and right of the map
	// (no vertical intersection for a vertical ray)
	if (ray != 0 && ray != NUM_RAYS/2)
	{
		if (ray >= NUM_RAYS/2)
		{
			// intersection
			sVert.x = (x & ~TILE_MASK) + TILE_UNITS;
			vdx = TILE_UNITS;

#if RAY_CLIP == 0
			// clipping
			newLen = ((((SDWORD)x + TILE_UNITS/4) - (mapWidth << TILE_SHIFT))
						* rayFPInvSin[ray])	>> RAY_ACC;
			if (newLen < clipLen)
			{
				clipLen = newLen;
			}
#endif
		}
		else
		{
			// intersection
			sVert.x = (x & ~TILE_MASK) - 1;
			vdx = -TILE_UNITS;

#if RAY_CLIP == 0
			// clipping
			newLen = (((SDWORD)x - TILE_UNITS/4) * rayFPInvSin[ray]) >> RAY_ACC;
			if (newLen < clipLen)
			{
				clipLen = newLen;
			}
#endif
		}

		// Vertical y is kept in fixed point form until passed to the callback
		// to avoid rounding errors
		// Vertical x is in integer form all the time
		sVert.y = (y << RAY_ACC) + ((SDWORD)x-sVert.x) * rayFPInvTan[ray];

		// Set up the distance calculations
		vDist = (((SDWORD)x - sVert.x) * rayFPInvSin[ray]) >> RAY_ACC;
		vdInc = rayVDist[ray];
	}
	else
	{
		// ensure no vertical intersections are calculated
		vDist = length;
	}

	ASSERT((hDist != 0 && vDist != 0,
		"rayCast: zero distance"));
	ASSERT(((hDist == (SDWORD)length || hdInc > 0) &&
			(vDist == (SDWORD)length || vdInc > 0),
		"rayCast: negative (or 0) distance increment"));

#if RAY_CLIP == 0
	while(hDist < clipLen ||
		  vDist < clipLen)
	{
		// choose the next closest intersection
		if (hDist < vDist)
		{
			// pass through the current intersection, converting x from fixed point
			if (!callback( sHoriz.x >> RAY_ACC,sHoriz.y, hDist))
			{
				// callback doesn't want any more points so return
				return;
			}

			// update for the next intersection
			sHoriz.x += rayDX[ray];
			sHoriz.y += hdy;
			hDist += hdInc;
		}
		else
		{
			// pass through the current intersection, converting y from fixed point
			if (!callback( sVert.x,sVert.y >> RAY_ACC, vDist))
			{
				// callback doesn't want any more points so return
				return;
			}

			// update for the next intersection
			sVert.x += vdx;
			sVert.y += rayDY[ray];
			vDist += vdInc;
		}
		ASSERT((hDist != 0 && vDist != 0,
			"rayCast: zero distance"));
	}
#elif RAY_CLIP == 1
	while(hDist < (SDWORD)length ||
		  vDist < (SDWORD)length)
	{
		// choose the next closest intersection
		if (hDist < vDist)
		{
			// clip to the edge of the map
			if (sHoriz.x < 0 || (sHoriz.x >> RAY_ACC) >= (SDWORD)(mapWidth << TILE_SHIFT) ||
				sHoriz.y < 0 || sHoriz.y >= (SDWORD)(mapHeight << TILE_SHIFT))
			{
				return;
			}

			// pass through the current intersection, converting x from fixed point
			if (!callback( sHoriz.x >> RAY_ACC,sHoriz.y, hDist))
			{
				// callback doesn't want any more points so return
				return;
			}

			// update for the next intersection
			sHoriz.x += rayDX[ray];
			sHoriz.y += hdy;
			hDist += hdInc;
		}
		else
		{
			// clip to the edge of the map
			if (sVert.x < 0 || sVert.x >= (SDWORD)(mapWidth << TILE_SHIFT) ||
				sVert.y < 0 || (sVert.y >> RAY_ACC) >= (SDWORD)(mapHeight << TILE_SHIFT))
			{
				return;
			}

			// pass through the current intersection, converting y from fixed point
			if (!callback( sVert.x,sVert.y >> RAY_ACC, vDist))
			{
				// callback doesn't want any more points so return
				return;
			}

			// update for the next intersection
			sVert.x += vdx;
			sVert.y += rayDY[ray];
			vDist += vdInc;
		}
		ASSERT((hDist != 0 && vDist != 0,
			"rayCast: zero distance"));
	}
#endif
}


// Calculate the angle to cast a ray between two points
UDWORD rayPointsToAngle(SDWORD x1,SDWORD y1, SDWORD x2,SDWORD y2)
{
	SDWORD		xdiff, ydiff;
	SDWORD		angle;
	
	xdiff = x2 - x1;
	ydiff = y1 - y2;

#ifdef WIN32
	angle = (SDWORD)((NUM_RAYS/2) * atan2(xdiff, ydiff) / PI);
#else
	angle = (SDWORD)ratan2(xdiff, ydiff);
	angle = angle_PSX2WORLD(angle);
#endif

	angle += NUM_RAYS/2;
	angle = angle % NUM_RAYS;

	ASSERT((angle >= 0 && angle < NUM_RAYS,
		"rayPointsToAngle: angle out of range"));

	return (UDWORD)angle;
}


/* Distance of a point from a line.
 * NOTE: This is not 100% accurate - it approximates to get the square root
 *
 * This is based on Graphics Gems II setion 1.3
 */
SDWORD rayPointDist(SDWORD x1,SDWORD y1, SDWORD x2,SDWORD y2,
					   SDWORD px,SDWORD py)
{
	SDWORD	a, lxd,lyd, dist;

	lxd = x2 - x1;
	lyd = y2 - y1;

	a = (py - y1)*lxd - (px - x1)*lyd;
	if (a < 0)
	{
		a = -a;
	}
	if (lxd < 0)
	{
		lxd = -lxd;
	}
	if (lyd < 0)
	{
		lyd = -lyd;
	}

	if (lxd < lyd)
	{
		dist = a / (lxd + lyd - lxd/2);
	}
	else
	{
		dist = a / (lxd + lyd - lyd/2);
	}

	return dist;
}


//-----------------------------------------------------------------------------------
/*	Gets the maximum terrain height along a certain direction to the edge of the grid
	from wherever you specify, as well as the distance away 
*/
// typedef BOOL (*RAY_CALLBACK)(SDWORD x, SDWORD y, SDWORD dist);
//void rayCast(UDWORD x, UDWORD y, UDWORD ray, UDWORD length, RAY_CALLBACK callback)

//#define TEST_RAY

/* Nasty global vars - put into a structure? */
//-----------------------------------------------------------------------------------
SDWORD	gHeight;
FRACT	gPitch;
UDWORD	gStartTileX;
UDWORD	gStartTileY;

SDWORD	gHighestHeight,gHOrigHeight;
SDWORD	gHMinDist;
FRACT	gHPitch;


//-----------------------------------------------------------------------------------
UDWORD getTileTallObj(UDWORD x, UDWORD y)
{
	UDWORD	i, j;
	UDWORD	TallObj = 0;

	x = x >> TILE_SHIFT;
	y = y >> TILE_SHIFT;

	for (j=y; j < y+2; j++)
	{
		for (i=x; i < x+2; i++)
		{
			TallObj |= TILE_HAS_TALLSTRUCTURE(mapTile(i,j));
		}
	}

	return TallObj;
}

//-----------------------------------------------------------------------------------
static BOOL	getTileHighestCallback(SDWORD x, SDWORD y, SDWORD dist)
{
SDWORD	heightDif;
UDWORD	height;
//iVector	pos;
	if(clipXY(x,y))
	{
		height = map_Height(x,y);
		if( (height > gHighestHeight) AND (dist >= gHMinDist) )
		{
			heightDif = height - gHOrigHeight;
			gHPitch = RAD_TO_DEG(atan2(MAKEFRACT(heightDif),
				MAKEFRACT(6*TILE_UNITS)));//MAKEFRACT(dist-(TILE_UNITS*3))));
			gHighestHeight = height;
  		}
//		pos.x = x;
//		pos.y = height;
//		pos.z = y;
//		addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
	}
	else
	{
		return(FALSE);
	}

	return(TRUE);

}
//-----------------------------------------------------------------------------------
/* Will return false when we've hit the edge of the grid */
static BOOL	getTileHeightCallback(SDWORD x, SDWORD y, SDWORD dist)
{
SDWORD	height,heightDif;
FRACT	newPitch;
BOOL HasTallStructure = FALSE;
#ifdef TEST_RAY
iVector	pos;
#endif

	/* Are we still on the grid? */
   	if(clipXY(x,y))
	{
		HasTallStructure = TILE_HAS_TALLSTRUCTURE(mapTile(x>>TILE_SHIFT,y>>TILE_SHIFT));

		if( (dist>TILE_UNITS) || HasTallStructure)
		{
		// Only do it the current tile is > TILE_UNITS away from the starting tile. Or..
		// there is a tall structure  on the current tile and the current tile is not the starting tile.
//		if( (dist>TILE_UNITS) ||
//			( (HasTallStructure = TILE_HAS_TALLSTRUCTURE(mapTile(x>>TILE_SHIFT,y>>TILE_SHIFT))) &&
//			((x >> TILE_SHIFT != gStartTileX) || (y >> TILE_SHIFT != gStartTileY)) ) ) {
			/* Get height at this intersection point */
			height = map_Height(x,y);

			if(HasTallStructure) {
				height += 300;	//TALLOBJECT_ADJUST;
			}

			if(height<=gHeight)
			{
				heightDif = 0;
			}
			else
			{
				heightDif = height-gHeight;
			}

			/* Work out the angle to this point from start point */
#ifdef WIN32
			newPitch = RAD_TO_DEG(atan2(MAKEFRACT(heightDif),MAKEFRACT(dist)));
#else
			newPitch = MAKEFRACT((SDWORD)angle_PSX2World( ratan2(heightDif, dist) ));
#endif

			/* Is this the steepest we've found? */
			if(newPitch>gPitch)
			{
				/* Yes, then keep a record of it */
				gPitch = newPitch;
			}
			//---

#ifdef TEST_RAY
			pos.x = x;
			pos.y = height;
			pos.z = y;
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
#endif
	//		if(height > gMaxRayHeight)
	//		{
	//			gMaxRayHeight = height;
	//			gRayDist = dist;
	//			return(TRUE);
	//		}
		}
	}
	else
	{
		/* We've hit edge of grid - so exit!! */
		return(FALSE);
	}

	/* Not at edge yet - so exit */
	return(TRUE);
}

void	getBestPitchToEdgeOfGrid(UDWORD x, UDWORD y, UDWORD direction, SDWORD *pitch)
{
	/* Set global var to clear */
	gPitch = MAKEFRACT(0);
	gHeight = map_Height(x,y);
	gStartTileX = x >> TILE_SHIFT;
	gStartTileY = y >> TILE_SHIFT;
//#ifdef TEST_RAY
//DBPRINTF(("%d\n",direction);
//#endif
	rayCast(x,y, direction%360,5430,getTileHeightCallback);
	*pitch = MAKEINT(gPitch);
}

//-----------------------------------------------------------------------------------
void	getPitchToHighestPoint( UDWORD x, UDWORD y, UDWORD direction, 
							   UDWORD thresholdDistance, SDWORD *pitch)
{
	gHPitch = MAKEFRACT(0);
	gHOrigHeight = map_Height(x,y);
	gHighestHeight = map_Height(x,y);
	gHMinDist = thresholdDistance;
	rayCast(x,y,direction%360,3000,getTileHighestCallback);
	*pitch = MAKEINT(gHPitch);
}
//-----------------------------------------------------------------------------------
