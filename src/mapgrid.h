/*
 * MapGrid.h
 *
 * Definitions for storing objects in a grid over the map.
 * The objects are stored in every grid over which they might
 * have some influence.
 *
 */
#ifndef _mapgrid_h
#define _mapgrid_h


// Number of Objects in each chunk of the grid array
#define MAX_GRID_ARRAY_CHUNK 32

// Objects are stored in an extensible array for each grid
typedef struct _grid_array
{
	BASE_OBJECT *apsObjects[MAX_GRID_ARRAY_CHUNK];

	struct _grid_array *psNext;
} GRID_ARRAY;


// The number of tiles per grid
#ifdef PSX
#define GRID_SIZE	16
#else
#define GRID_SIZE	8
#endif

#define GRID_MAXAREA (MAP_MAXAREA/(GRID_SIZE*GRID_SIZE))

// The size of the grid
//#define GRID_WIDTH	(MAP_MAXWIDTH/GRID_SIZE)
//#define GRID_HEIGHT	(MAP_MAXHEIGHT/GRID_SIZE)

// The map grid 
//extern GRID_ARRAY	*apsMapGrid[GRID_WIDTH][GRID_HEIGHT];


// initialise the grid system
extern BOOL gridInitialise(void);

// shutdown the grid system
extern void gridShutDown(void);

//clear the grid of everything on it
extern void gridClear(void);

// reset the grid system
extern void gridReset(void);

// add an object to the grid system
extern void gridAddObject(BASE_OBJECT *psObj);

// move an object within the grid
// oldX,oldY are the old position of the object in world coords
extern void gridMoveObject(BASE_OBJECT *psObj, SDWORD oldX, SDWORD oldY);

// remove an object from the grid system
extern void gridRemoveObject(BASE_OBJECT *psObj);

// compact some of the grid arrays
extern void gridGarbageCollect(void);

// Display all the grid's an object is a member of
extern void gridDisplayCoverage(BASE_OBJECT *psObj);

// initialise the grid system to start iterating through units that
// could affect a location (x,y in world coords)
extern void gridStartIterate(SDWORD x, SDWORD y);

// get the next object that could affect a location,
// should only be called after gridStartIterate
extern BASE_OBJECT *gridIterate(void);

#endif

