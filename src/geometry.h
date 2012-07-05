/* Geometry.h */

#ifndef _geometry_h
#define _geometry_h

#define SIZE_SINE_TABLE		100
#define AMPLITUDE_HEIGHT	100
#define	pi 3.141592657
#define deg pi/SIZE_SINE_TABLE
#define RESTRICT_iV_ANGLE(x)		((x) & (iV_RMULTP - 1))

typedef struct _t_tri
{
POINT	coords[3];
} TRI;

typedef struct _t_quad
{
POINT	coords[4];
} QUAD;

extern void	processImpact(UDWORD worldX, UDWORD worldY, UBYTE severity,UDWORD tilesAcross);
extern UDWORD	getTileOwner(UDWORD	x, UDWORD y);
extern BASE_OBJECT	*getTileOccupier(UDWORD x, UDWORD y);
extern STRUCTURE	*getTileStructure(UDWORD x, UDWORD y);
extern FEATURE		*getTileFeature(UDWORD x, UDWORD y);


//extern	BOOL	bScreenShakeActive;
//extern	UDWORD	screenShakeStarted;
//extern	UDWORD	screenShakeLength;
//extern	void	attemptScreenShake(void);

extern	void			baseObjScreenCoords(BASE_OBJECT *baseObj, iPoint *pt);
extern	void			WorldPointToScreen( iPoint *worldPt, iPoint *screenPt );
extern	UDWORD			adjustDirection	( SDWORD present, SDWORD difference );
extern	SDWORD			calcDirection	( UDWORD x0, UDWORD y0, UDWORD x1, UDWORD y1 );
extern	void			initBulletTable	( void );
extern	int				inQuad			( POINT *pt, QUAD *quad );
extern DROID			*getNearestDroid ( UDWORD x, UDWORD y, BOOL bSelected );

extern	UBYTE			sineHeightTable[SIZE_SINE_TABLE];
extern BOOL	droidOnScreen ( DROID *psDroid, SDWORD tolerance );

#define BOTLEFT		11
#define BOTRIGHT	21
#define TOPLEFT		31
#define TOPRIGHT	41
#define MAX_TILE_DAMAGE	255
#define CRATER_SMALL	3
#define CRATER_MEDIUM	5
#define CRATER_LARGE	7



/* Return a signed difference in direction : a - b
 * result is 180 .. -180
 */
extern SDWORD directionDiff	( SDWORD a, SDWORD b );
extern UDWORD	dirtySqrt	( SDWORD x1,SDWORD y1, SDWORD x2,SDWORD y2 );

#endif
