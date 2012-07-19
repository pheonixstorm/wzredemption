#ifndef _effects_h
#define _effects_h
/*	
	All singing, all dancing new effects code. Does all the stuff
	that explosion.c and most of particle.c used to do as well as
	all the construction dust clouds, body parts and all other
	temporary world 'effects 
	Alex McLean, Pumpkin Studios, EIDOS Interactive, 1998.
*/
#include "piedef.h"
#include "pieTypes.h"

/* All the effect groups */
typedef enum 
{
EFFECT_EXPLOSION,
EFFECT_CONSTRUCTION,
EFFECT_SMOKE,
EFFECT_STRUCTURE,
EFFECT_GRAVITON,
EFFECT_WAYPOINT,
EFFECT_BLOOD,
EFFECT_DESTRUCTION,
EFFECT_SAT_LASER,
EFFECT_DUST_BALL,
EFFECT_FIRE,
EFFECT_FIREWORK
}EFFECT_GROUP;

/* Might not even need this */
typedef enum
{
EXPLOSION_TYPE_SMALL,
EXPLOSION_TYPE_VERY_SMALL,
EXPLOSION_TYPE_MEDIUM,
EXPLOSION_TYPE_LARGE,
EXPLOSION_TYPE_SPECIFIED,	// superfluous?
EXPLOSION_TYPE_NOT_FACING,
EXPLOSION_TYPE_SPECIFIED_SOLID,
EXPLOSION_TYPE_SPECIFIED_FIXME,
EXPLOSION_TYPE_FLAMETHROWER,
EXPLOSION_TYPE_LASER,
EXPLOSION_TYPE_TESLA,
EXPLOSION_TYPE_DISCOVERY,
EXPLOSION_TYPE_FLARE,
EXPLOSION_TYPE_PLASMA,
EXPLOSION_TYPE_KICKUP,
EXPLOSION_TYPE_LAND_LIGHT,
EXPLOSION_TYPE_SHOCKWAVE,
//EXPLOSION_TYPE_SKYSCRAPER,

GRAVITON_TYPE_STANDARD,
GRAVITON_TYPE_EMITTING_DR,
GRAVITON_TYPE_EMITTING_ST,
GRAVITON_TYPE_GIBLET,

SMOKE_TYPE_DRIFTING,
SMOKE_TYPE_DRIFTING_HIGH,
SMOKE_TYPE_DRIFTING_SMALL,
SMOKE_TYPE_BILLOW,
SMOKE_TYPE_STEAM,
SMOKE_TYPE_TRAIL,
FIRE_TYPE_LOCALISED,
FIRE_TYPE_SMOKY,
FIRE_TYPE_SMOKY_BLUE,
CONSTRUCTION_TYPE_DRIFTING,

BLOOD_TYPE_NORMAL,

DUST_TYPE_NORMAL,
DESTRUCTION_TYPE_DROID,
DESTRUCTION_TYPE_STRUCTURE,
DESTRUCTION_TYPE_POWER_STATION,
DESTRUCTION_TYPE_WALL_SECTION,
DESTRUCTION_TYPE_FEATURE,
DESTRUCTION_TYPE_SKYSCRAPER,

SAT_LASER_STANDARD,

WAYPOINT_TYPE,

FIREWORK_TYPE_STARBURST,
FIREWORK_TYPE_LAUNCHER,

}EFFECT_TYPE;

/* Is the slot currently being used and is it active? */
typedef enum
{
    ES_INACTIVE,
    ES_ACTIVE,
    ES_DORMANT
}EFFECT_STATUS;

typedef enum
{
	LL_MIDDLE,
	LL_INNER,
	LL_OUTER
}LAND_LIGHT_SPEC;

#define MAX_EFFECTS	2500
#define	GRAVITON_GRAVITY	((FRACT)-800)
#define	EFFECT_X_FLIP		0x1
#define	EFFECT_Y_FLIP		0x2
#define	EFFECT_CYCLIC		0x4
#define EFFECT_ESSENTIAL	0x8
#define EFFECT_FACING		0x10
#define	EFFECT_SCALED		0x20
#define	EFFECT_LIT			0x40

#define	TEST_FLIPPED_X(x)		(x->control & EFFECT_X_FLIP)
#define	TEST_FLIPPED_Y(x)		(x->control & EFFECT_Y_FLIP)
#define TEST_ESSENTIAL(x)		(x->control & EFFECT_ESSENTIAL)
#define TEST_FACING(x)			(x->control & EFFECT_FACING)
#define TEST_CYCLIC(x)			(x->control & EFFECT_CYCLIC)
#define TEST_SCALED(x)			(x->control & EFFECT_SCALED)
#define TEST_LIT(x)				(x->control & EFFECT_LIT)

#define	SET_FLIPPED_X(x)		((x->control) = (UBYTE)(x->control | EFFECT_X_FLIP))
#define	SET_FLIPPED_Y(x)		((x->control) = (UBYTE)(x->control | EFFECT_Y_FLIP))
#define SET_ESSENTIAL(x)		((x->control) = (UBYTE)(x->control | EFFECT_ESSENTIAL))
#define SET_FACING(x)			((x->control) = (UBYTE)(x->control | EFFECT_FACING))
#define SET_CYCLIC(x)			((x->control) = (UBYTE)(x->control | EFFECT_CYCLIC))
#define SET_SCALED(x)			((x->control) = (UBYTE)(x->control | EFFECT_SCALED))
#define SET_LIT(x)				((x->control) = (UBYTE)(x->control | EFFECT_LIT))
#define SET_LITABS(x)			((x.control) = (UBYTE)(x.control | EFFECT_LIT))

#define MINIMUM_IMPACT_VELOCITY		(16)
#define	NORMAL_SMOKE_LIFESPAN		(6000 + rand()%3000)
#define SMALL_SMOKE_LIFESPAN		(3000 + rand()%3000)
#define	TRAIL_SMOKE_LIFESPAN		(1200)
#define	CONSTRUCTION_LIFESPAN		(5000)

#define	SMOKE_FRAME_DELAY			(40 + rand()%30)
#define	EXPLOSION_FRAME_DELAY		(25 + rand()%40)
#define	EXPLOSION_TESLA_FRAME_DELAY	(65)
#define	EXPLOSION_PLASMA_FRAME_DELAY	(45)
#define	BLOOD_FRAME_DELAY			(150)
#define DESTRUCTION_FRAME_DELAY		(200)

#define SHOCK_WAVE_HEIGHT	(64)

#define	TESLA_SPEED					(170)// + (30 - rand()%60))
#define	TESLA_SIZE					(100)// + (20 - rand()%40))

#define GRAVITON_FRAME_DELAY		(100 + rand()%50)
#define GRAVITON_BLOOD_DELAY		(200 + rand()%100)

#define CONSTRUCTION_FRAME_DELAY	(40 + rand()%30)

#define	EXPLOSION_SIZE				(110+(30-rand()%60))
#define	LIMITED_EXPLOSION_SIZE		(100+(5-rand()%10))
#define	BLOOD_SIZE					(100+(30-rand()%60))
#define	BLOOD_FALL_SPEED			(-(20+rand()%20))

#define GRAVITON_INIT_VEL_X			MAKEFRACT(200-rand()%300)
#define GRAVITON_INIT_VEL_Z			MAKEFRACT(200-rand()%300)
#define GRAVITON_INIT_VEL_Y			MAKEFRACT(300+rand()%100)

#define GIBLET_INIT_VEL_X			MAKEFRACT(50-rand()%100)
#define GIBLET_INIT_VEL_Z			MAKEFRACT(50-rand()%100)
#define GIBLET_INIT_VEL_Y			MAKEFRACT(12)

#define	DROID_DESTRUCTION_DURATION		(3*GAME_TICKS_PER_SEC/2) // 1.5 seconds
#define	STRUCTURE_DESTRUCTION_DURATION	((7*GAME_TICKS_PER_SEC)/2)	 // 3.5 seconds

/* 
	Definition of an 'effect'. This'll need to come down in size
	for the PlayStation? 
*/

typedef struct	_effect_def
{
    UBYTE			status;			// what status is the present effect - active/inactive/dormant
    UBYTE			control;		// Controls the bits above - essential,flips etc
    UBYTE			group;			// what	group is it - explosion, building effect etc....
    UBYTE			type;			// what type is it within the group?
    UBYTE			frameNumber;	// what frame number is the imd on?
    UWORD			size;			// Size in terms of percent of original imd.
    UBYTE			baseScale;		// if scaled, what's bottom line?
    UBYTE			specific;		// how many times has it bounced?
    PIEVECTORF		position;		// world coordinates of the effect - floats on the PC.
    PIEVECTORF		velocity;		// movement values per update
    iVector			rotation;		// current rotation - only for gravitons
    iVector			spin;			// rotation info for spinning things.
    UDWORD			birthTime;		// what time was it introduced into the world?
    UDWORD			lastFrame;		// when did we last update the frame?
    UWORD			frameDelay;		// how many game ticks between each frame?
    UWORD			lifeSpan;		// what is it's life expectancy?
    UWORD			radius;			// Used for area effects
    struct iIMDShape		*imd;	// pointer to the imd the effect uses.
} EFFECT;

#define KILL_EFFECT(x)	(x)->status = ES_INACTIVE; \
						(x)->control = (UBYTE) 0

/* Maximum number of effects in the world - need to investigate what this should be */
/* EXTERNAL REFERENCES */
extern void	effectGiveAuxVar		( UDWORD var); // naughty
extern void	effectGiveAuxVarSec		( UDWORD var); // and so's this

extern void	initEffectsSystem		( void );
extern void	processEffects			( void );		
extern void	addEffect				( iVector *pos, EFFECT_GROUP group, 
										EFFECT_TYPE type, BOOL specified, struct iIMDShape *imd, BOOL lit );
extern void	addMultiEffect			( iVector *basePos, iVector *scatter,EFFECT_GROUP group, 
									EFFECT_TYPE type,BOOL specified, struct iIMDShape *imd, UDWORD number, BOOL lit, UDWORD size );

extern void drawEffects				( void );
extern void	renderEffect			( EFFECT *psEffect );
extern UDWORD getNumEffects			( void );
extern UDWORD	getFreeEffect		( void );
extern void	effectResetUpdates		( void );
extern UDWORD	getNumActiveEffects		( void );
extern UDWORD	getMissCount( void );
extern	UDWORD	getNumSkippedEffects(void);
extern	UDWORD	getNumEvenEffects(void);

extern	BOOL	fireOnLocation(UDWORD x, UDWORD y);

extern UDWORD EffectGetNumFrames(EFFECT *psEffect);
extern UDWORD IMDGetNumFrames(iIMDShape *Shape);
extern UDWORD IMDGetAnimInterval(iIMDShape *Shape);
extern void	initPerimeterSmoke			( iIMDShape *pImd, UDWORD x, UDWORD y, UDWORD z);

#define SKY_MULT	1
#define SKY_SHIMMY_BASE	((DEG(1)*SKY_MULT)/2)
#define SKY_SHIMMY (SKY_SHIMMY_BASE - (rand()%(2*SKY_SHIMMY_BASE)))
extern BOOL	readFXData( UBYTE *pFileData, UDWORD fileSize );
extern BOOL	writeFXData( STRING *pFileName );
extern	void	effectSetSize(UDWORD size);
extern void	effectSetLandLightSpec(LAND_LIGHT_SPEC spec);

#endif
