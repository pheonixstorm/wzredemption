/*
 * Map.h
 *
 * Definitions for the map structure
 *
 */
#ifndef _map_h
#define _map_h

#include <stdio.h>
#include "Frame.h"
#include "Objects.h"


#define	TIB_VIS0		=	0x1,	// Visibility bits - can also be accessed as a byte (as a whole).
#define TIB_VIS1		=	0x2,
#define TIB_VIS2		=	0x4,
#define	TIB_VIS3		=	0x8,
#define	TIB_VIS4		=	0x10,
#define	TIB_VIS5		=	0x20,
#define	TIB_VIS6		=	0x40,
#define	TIB_VIS7		=	0x80,

/* The different types of terrain as far as the game is concerned */
typedef enum _terrain_type
{
	TER_SAND,
	TER_SANDYBRUSH,
	TER_BAKEDEARTH,
	TER_GREENMUD,
	TER_REDBRUSH,
	TER_PINKROCK,
	TER_ROAD,
	TER_WATER,
	TER_CLIFFFACE,
	TER_RUBBLE,
	TER_SHEETICE,
	TER_SLUSH,

	TER_MAX,
} TYPE_OF_TERRAIN;

/* change these if you change above - maybe wrap up in enumerate? */
#define	TERRAIN_TYPES	TER_MAX

typedef enum _tts
{
TILE_TYPE,		
SPEED,	   
MARKER,		
} TYPE_SPEEDS;
extern UDWORD	relativeSpeeds[TERRAIN_TYPES][MARKER];

#define TALLOBJECT_YMAX		(200)
#define TALLOBJECT_ADJUST	(200)

/* Flags for whether texture tiles are flipped in X and Y or rotated */
#define TILE_XFLIP		0x8000
#define TILE_YFLIP		0x4000
#define TILE_ROTMASK	0x3000
#define TILE_ROTSHIFT	12
#define TILE_TRIFLIP	0x0800	// This bit describes the direction the tile is split into 2 triangles (same as triangleFlip)
#define TILE_HILIGHT	0x0400	// set when the tile has the structure cursor over it

// NASTY - this should be in tileInfoBits but there isn't any room left
#define TILE_NOTBLOCKING	0x0200	// units can drive on this even if there is a structure or feature on it

#define TILE_NUMMASK	0x01ff
#define TILE_BITMASK	0xfe00

#define BITS_STRUCTURE	0x1
#define BITS_FEATURE	0x2
#define BITS_NODRAWTILE	0x4
//#define	BITS_TILE_HIGHLIGHT 0x8
#define BITS_SMALLSTRUCTURE	0x8			// show small structures - tank traps / bunkers
#define BITS_FPATHBLOCK	0x10		// bit set temporarily by find path to mark a blocking tile
#define BITS_WALL		0x20
#define BITS_GATEWAY	0x40		// bit set to show a gateway on the tile
#define BITS_TALLSTRUCTURE 0x80		// bit set to show a tall structure which camera needs to avoid.

/*#ifdef WIN32	// Extra tile info bits.... WIN32 only
#define	EXTRA_BITS_SENSOR	0x1
#define	EXTRA_BITS_2		0x2
#define	EXTRA_BITS_3		0x4
#define	EXTRA_BITS_4		0x8
#define	EXTRA_BITS_5		0x10
#define	EXTRA_BITS_6		0x20
#define	EXTRA_BITS_7		0x40
#define	EXTRA_BITS_8		0x80
#endif*/

#define BITS_STRUCTURE_MASK	0xfe
#define BITS_FEATURE_MASK	0xfd 
#define BITS_OCCUPIED_MASK	0xfc

#define TILE_IS_NOTBLOCKING(x)	(x->texture & TILE_NOTBLOCKING)

#define TILE_IMPOSSIBLE(x)		((TILE_HAS_STRUCTURE(x)) && (TILE_HAS_FEATURE(x)))
#define BITS_OCCUPIED			(BITS_STRUCTURE | BITS_FEATURE | BITS_WALL)
#define TILE_OCCUPIED(x)		(x->tileInfoBits & BITS_OCCUPIED)
#define TILE_HAS_STRUCTURE(x)	(x->tileInfoBits & BITS_STRUCTURE)
#define TILE_HAS_FEATURE(x)		(x->tileInfoBits & BITS_FEATURE)
#define TILE_DRAW(x)			(!((x)->tileInfoBits & BITS_NODRAWTILE))
//#define TILE_HIGHLIGHT(x)		(x->tileInfoBits & BITS_TILE_HIGHLIGHT)
#define TILE_HIGHLIGHT(x)		(x->texture & TILE_HILIGHT)
#define TILE_HAS_TALLSTRUCTURE(x)	(x->tileInfoBits & BITS_TALLSTRUCTURE)
#define TILE_HAS_SMALLSTRUCTURE(x)	(x->tileInfoBits & BITS_SMALLSTRUCTURE)

/*
#ifdef WIN32		// I've even set them up for you...:-)
#define TILE_IN_SENSORRANGE(x)	(x->tileExtraBits & EXTRA_BITS_SENSOR)
#define TILE_EXTRA_BIT2_SET(x)	(x->tileExtraBits & EXTRA_BITS_2)
#define TILE_EXTRA_BIT3_SET(x)	(x->tileExtraBits & EXTRA_BITS_3)
#define TILE_EXTRA_BIT4_SET(x)	(x->tileExtraBits & EXTRA_BITS_4)
#define TILE_EXTRA_BIT5_SET(x)	(x->tileExtraBits & EXTRA_BITS_5)
#define TILE_EXTRA_BIT6_SET(x)	(x->tileExtraBits & EXTRA_BITS_6)
#define TILE_EXTRA_BIT7_SET(x)	(x->tileExtraBits & EXTRA_BITS_7)
#define TILE_EXTRA_BIT8_SET(x)	(x->tileExtraBits & EXTRA_BITS_8)
#endif
*/
#define SET_TILE_NOTBLOCKING(x)	(x->texture |= TILE_NOTBLOCKING)
#define CLEAR_TILE_NOTBLOCKING(x)	(x->texture &= ~TILE_NOTBLOCKING)

#define SET_TILE_STRUCTURE(x)	(x->tileInfoBits = (UBYTE)(((x->tileInfoBits & (~BITS_OCCUPIED))) | BITS_STRUCTURE))
#define SET_TILE_FEATURE(x)		(x->tileInfoBits = (UBYTE)(((x->tileInfoBits & (~BITS_OCCUPIED))) | BITS_FEATURE))
#define SET_TILE_EMPTY(x)		(x->tileInfoBits = (UBYTE) (x->tileInfoBits & (~BITS_OCCUPIED)) )
#define SET_TILE_NODRAW(x)		(x->tileInfoBits = (UBYTE)((x)->tileInfoBits | BITS_NODRAWTILE))
#define CLEAR_TILE_NODRAW(x)	(x->tileInfoBits = (UBYTE)((x)->tileInfoBits & (~BITS_NODRAWTILE)))
#define SET_TILE_HIGHLIGHT(x)	(x->texture = (UWORD)((x)->texture | TILE_HILIGHT))
#define CLEAR_TILE_HIGHLIGHT(x)	(x->texture = (UWORD)((x)->texture & (~TILE_HILIGHT)))
#define SET_TILE_TALLSTRUCTURE(x)	(x->tileInfoBits = (UBYTE)((x)->tileInfoBits | BITS_TALLSTRUCTURE))
#define CLEAR_TILE_TALLSTRUCTURE(x)	(x->tileInfoBits = (UBYTE)((x)->tileInfoBits & (~BITS_TALLSTRUCTURE)))
#define SET_TILE_SMALLSTRUCTURE(x)	(x->tileInfoBits = (UBYTE)((x)->tileInfoBits | BITS_SMALLSTRUCTURE))
#define CLEAR_TILE_SMALLSTRUCTURE(x)	(x->tileInfoBits = (UBYTE)((x)->tileInfoBits & (~BITS_SMALLSTRUCTURE)))

/*
#ifdef WIN32	// again, done for you again!
#define SET_TILE_SENSOR(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_SENSOR))
#define CLEAR_TILE_SENSOR(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_SENSOR)))
#define SET_TILE_BIT2(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_2))
#define CLEAR_TILE_BIT2(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_2)))
#define SET_TILE_BIT3(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_3))
#define CLEAR_TILE_BIT3(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_3)))
#define SET_TILE_BIT4(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_4))
#define CLEAR_TILE_BIT4(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_4)))
#define SET_TILE_BIT5(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_5))
#define CLEAR_TILE_BIT5(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_5)))
#define SET_TILE_BIT6(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_6))
#define CLEAR_TILE_BIT6(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_6)))
#define SET_TILE_BIT7(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_7))
#define CLEAR_TILE_BIT7(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_7)))
#define SET_TILE_BIT8(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits | EXTRA_BITS_8))
#define CLEAR_TILE_BIT8(x)	(x->tileExtraBits = (UBYTE)((x)->tileExtraBits & (~EXTRA_BITS_8)))
#endif
*/
// Multiplier for the tile height
#define	ELEVATION_SCALE	2

/* Allows us to do if(TRI_FLIPPED(psTile)) */
#define TRI_FLIPPED(x)		(x->texture & TILE_TRIFLIP)
/* Flips the triangle partition on a tile pointer */
#define TOGGLE_TRIFLIP(x)	(x->texture = (UWORD)(x->texture ^ TILE_TRIFLIP))

/* Can player number p see tile t? */
#define TEST_TILE_VISIBLE(p,t)	( (t->tileVisBits) & (1<<p) )


/* Set a tile to be visible for a player */
#define SET_TILE_VISIBLE(p,t) t->tileVisBits = (UBYTE)(t->tileVisBits | (1<<p))

/*#ifdef WIN32
#define SET_TILE_DOOR(p,t) t->tileDoorBits = (UBYTE) (t->tileDoorBits | (1<<p))
#define CLEAR_TILE_DOOR(p,t) t->tileDoorBits = (UBYTE) (t->tileDoorBits & (~(1<<p))) // check logic
// Is there a door here for the player?
#define TEST_TILE_DOOR(p,t) ( (t->tileDoorBits) & (1<<p) )
#endif*/
/* Arbitrary maximum number of terrain textures - used in look up table for terrain type */

#ifdef WIN32
#define MAX_TILE_TEXTURES	255
#else
#define MAX_TILE_TEXTURES	81
#endif

extern UBYTE terrainTypes[MAX_TILE_TEXTURES];

#define TERRAIN_TYPE(x) terrainTypes[x->texture & TILE_NUMMASK]

/* Information stored with each tile */
// The name is now changed to MAPTILE to allow correct compilation on the PlayStation

typedef struct _maptile
{

	UBYTE			tileInfoBits;
/*#ifdef WIN32
	UBYTE			tileExtraBits;	// We've got more than you... We've got more than you..;-)
#endif*/
	UBYTE			tileVisBits;	// COMPRESSED - bit per player
/*#ifdef WIN32
	UBYTE			tileDoorBits;   // same thing - bit per player
#endif*/
	UBYTE			height;			// The height at the top left of the tile
	UBYTE			illumination;	// How bright is this tile?
	UWORD			texture;		// Which graphics texture is on this tile
#ifdef WIN32
	UBYTE			bMaxed;
	UBYTE			level;

	UBYTE			inRange;		// sensor range display.
#endif

									// This is also used to store the tile flip flags
//  What's been removed - 46 bytes per tile so far
//	BASE_OBJECT		*psObject;		// Any object sitting on the location (e.g. building)
//	UBYTE			onFire;			// Is tile on fire?
//	UBYTE			rippleIndex;	// Current value in ripple table?
//	BOOL			tileVisible[MAX_PLAYERS]; // Which players can see the tile?
//	BOOL			triangleFlip;	// Is the triangle flipped?
//	TYPE_OF_TERRAIN	type;			// The terrain type for the tile
} MAPTILE;



/* The maximum map size */
#ifdef WIN32
#define MAP_MAXWIDTH	256
#define MAP_MAXHEIGHT	256
#define MAP_MAXAREA		(256*256)
#else
#define MAP_MAXWIDTH	256			// Maximum value for map width.
#define MAP_MAXHEIGHT	256			// Maximum value for map height.
#define MAP_MAXAREA		(192*128)	// Maximum value for map width * map height
#endif
#define TILE_MAX_HEIGHT		(255 * ELEVATION_SCALE) 
#define TILE_MIN_HEIGHT		  0

/* The size and contents of the map */
extern UDWORD	mapWidth, mapHeight;
extern MAPTILE *psMapTiles;

/* The shift on the y coord when calculating into the map */
extern UDWORD	mapShift;

#ifdef NECROMANCER
/* The number of units accross a tile */
#define TILE_UNITS	64

/* The shift on a coordinate to get the tile coordinate */
#define TILE_SHIFT	6

/* The mask to get internal tile coords from a full coordinate */
#define TILE_MASK	0x3f
#else
/* The number of units accross a tile */
#define TILE_UNITS	128

/* The shift on a coordinate to get the tile coordinate */
#define TILE_SHIFT	7

/* The mask to get internal tile coords from a full coordinate */
#define TILE_MASK	0x7f
#endif

/* Shutdown the map module */
extern BOOL mapShutdown(void);

/* Create a new map of a specified size */
extern BOOL mapNew(UDWORD width, UDWORD height);

/* Load the map data */
extern BOOL mapLoad(UBYTE *pFileData, UDWORD fileSize);

/* Save the map data */
extern BOOL mapSave(UBYTE **ppFileData, UDWORD *pFileSize);

/* Load map texture info */
extern void mapLoadTexture(void);

/* Save the current map texture info */
extern void mapSaveTexture(void);

/* A post process for the water tiles in map to ensure height integrity */
extern void	mapWaterProcess( void );


#ifdef WIN32
#define FUNCINLINE _inline
#else

#undef FUNCINLINE
#ifdef DEFINE_MAPINLINE
#define FUNCINLINE 
#else
#define FUNCINLINE __inline extern
#endif


#endif

/* Return a pointer to the tile structure at x,y */
FUNCINLINE MAPTILE *mapTile(UDWORD x, UDWORD y)
{
#ifdef WIN32
	ASSERT((x < mapWidth,
		"mapTile: x coordinate bigger than map width"));
	ASSERT((y < mapHeight,
		"mapTile: y coordinate bigger than map height"));
#else
#ifdef DEBUG
	assert(psMapTiles);		// make sure it's not zero
	if((x>=mapWidth) || (y>=mapHeight)) 
	{
		printf("mapTile: invalid XY (%d,%d)\n",x,y);
		return psMapTiles;
	}

#endif
#endif
	//return psMapTiles + x + (y << mapShift); //width no longer a power of 2
	return psMapTiles + x + (y * mapWidth); 
}

/* Return height of tile at x,y */
//FUNCINLINE SDWORD map_TileHeight(UDWORD x, UDWORD y)
FUNCINLINE SWORD map_TileHeight(UDWORD x, UDWORD y)
{
    x = x >= (mapWidth) ? (mapWidth-1) : x;
	y = y >= (mapHeight) ? (mapHeight-1) : y;
#ifdef WIN32
	ASSERT((x < mapWidth,
		"mapTile: x coordinate bigger than map width"));
	ASSERT((y < mapHeight,
		"mapTile: y coordinate bigger than map height"));
#else
	if((x>=mapWidth) || (y>=mapHeight)) {
		printf("mapTileHeight: invalid XY (%d,%d)\n",x,y);
		return 0;
	}
#endif

//	return ((psMapTiles[x + (y << mapShift)].height) * ELEVATION_SCALE);//width no longer a power of 2
	return (SWORD)((psMapTiles[x + (y * mapWidth)].height) * ELEVATION_SCALE);
}

/*sets the tile height */
FUNCINLINE void setTileHeight(UDWORD x, UDWORD y, UDWORD height)
{
#ifdef WIN32
	ASSERT((x < mapWidth,
		"mapTile: x coordinate bigger than map width"));
	ASSERT((y < mapHeight,
		"mapTile: y coordinate bigger than map height"));
#else
	if((x>=mapWidth) || (y>=mapHeight)) {
		printf("setTileHeight: invalid XY (%d,%d)\n",x,y);
		return;
	}
#endif
	//psMapTiles[x + (y << mapShift)].height = height;//width no longer a power of 2
	psMapTiles[x + (y * mapWidth)].height = (UBYTE) (height / ELEVATION_SCALE);
}

/*increases the tile height by one */
/*FUNCINLINE void incTileHeight(UDWORD x, UDWORD y)
{
	psMapTiles[x + (y << mapShift)].height++;
}*/

/*decreases the tile height by one */
/*FUNCINLINE void decTileHeight(UDWORD x, UDWORD y)
{
	psMapTiles[x + (y << mapShift)].height--;
}*/

/* Return whether a tile coordinate is on the map */
FUNCINLINE BOOL tileOnMap(SDWORD x, SDWORD y)
{
	return (x >= 0) && (x < (SDWORD)mapWidth) && (y >= 0) && (y < (SDWORD)mapHeight);
}

/* Return whether a world coordinate is on the map */
FUNCINLINE BOOL worldOnMap(SDWORD x, SDWORD y)
{
	return (x >= 0) && (x < ((SDWORD)mapWidth << TILE_SHIFT)) &&
		   (y >= 0) && (y < ((SDWORD)mapHeight << TILE_SHIFT));
}

/* Store a map coordinate and it's associated tile, used by mapCalcLine */
typedef struct _tile_coord
{
	UDWORD	x,y;
	MAPTILE	*psTile;
} TILE_COORD;

/* The map tiles generated by map calc line */
extern TILE_COORD	*aMapLinePoints;

/* work along a line on the map storing the points in aMapLinePoints.
 * pNumPoints is set to the number of points generated.
 * The start and end points are in TILE coordinates.
 */
extern void mapCalcLine(UDWORD startX, UDWORD startY,
						UDWORD endX, UDWORD endY,
						UDWORD *pNumPoints);

/* Same as mapCalcLine, but does a wider line in the map */
extern void mapCalcAALine(SDWORD X1, SDWORD Y1,
				   SDWORD X2, SDWORD Y2,
				   UDWORD *pNumPoints);

/* Return height of x,y */
//extern SDWORD map_Height(UDWORD x, UDWORD y);
extern SWORD map_Height(UDWORD x, UDWORD y);

/* returns TRUE if object is above ground */
extern BOOL mapObjIsAboveGround( BASE_OBJECT *psObj );

/* returns the max and min height of a tile by looking at the four corners 
   in tile coords */
extern void getTileMaxMin(UDWORD x, UDWORD y, UDWORD *pMax, UDWORD *pMin);

MAPTILE *GetCurrentMap(void);	// returns a pointer to the current loaded map data
UDWORD GetHeightOfMap(void);
UDWORD GetWidthOfMap(void);
extern BOOL	readVisibilityData( UBYTE *pFileData, UDWORD fileSize );
extern BOOL	writeVisibilityData( STRING *pFileName );
extern void	mapFreeTilesAndStrips( void );

//scroll min and max values
extern SDWORD		scrollMinX, scrollMaxX, scrollMinY, scrollMaxY;
extern BOOL	bDoneWater;

#endif

