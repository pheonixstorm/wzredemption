/*
	Spot FX code - will handle every miscellaneous imd render and update for temporary
	entities except projectiles.
	Handles stuff like
	- Smoke sprites on the card.
	- Explosions
	- Building body kit - flashing lights etc etc
	- Construction graphics
	- Gravitons
	- Dust
	- Blood
	************************************************************
	* STILL NEED TO REMOVE SOME MAGIC NUMBERS INTO #DEFINES!!! *
	************************************************************
*/

#include <stdio.h>
#include <assert.h>
#include "Frame.h"
#include "ivisdef.h" //ivis matrix code
#include "piedef.h" //ivis matrix code
#include "pieState.h"
#include "geo.h" //ivis matrix code
#include "GTime.h"
#include "Display3d.h"
#include "Map.h"
#include "Bucket3D.h"
#include "pieMode.h"
#include "Mission.h"

/*Remove this one!!! :-( */
#include "MiscImd.h"
#include "Effects.h"
#include "audio.h"
#include "audio_id.h"
#include "HCI.h"
#include "Lighting.h"
#include "Console.h"
#include "Loop.h"
#include "MultiPlay.h"

#include "Game.h"
//#define COUNTOFFSCREEN

#define DOLIGHTS

extern UWORD OffScreenEffects;

/* Our list of all game world effects */
EFFECT	asEffectsList[MAX_EFFECTS];

#define FIREWORK_EXPLODE_HEIGHT			400
#define STARBURST_RADIUS				150
#define STARBURST_DIAMETER				300

#define	EFFECT_SMOKE_ADDITIVE			1
#define	EFFECT_STEAM_ADDITIVE			8
#define	EFFECT_WAYPOINT_ADDITIVE		32
#define	EFFECT_EXPLOSION_ADDITIVE		164
#define EFFECT_PLASMA_ADDITIVE			224
#define	EFFECT_SMOKE_TRANSPARENCY		130
#define	EFFECT_STEAM_TRANSPARENCY		128
#define	EFFECT_WAYPOINT_TRANSPARENCY	128
#define	EFFECT_BLOOD_TRANSPARENCY		128
#define	EFFECT_EXPLOSION_TRANSPARENCY	164

#define	EFFECT_DROID_DIVISION			101
#define	EFFECT_STRUCTURE_DIVISION		103

#define	DROID_UPDATE_INTERVAL			500
#define	STRUCTURE_UPDATE_INTERVAL		1250
#define	BASE_FLAME_SIZE					80
#define	BASE_LASER_SIZE					10
#define BASE_PLASMA_SIZE				0
#define	DISCOVERY_SIZE					60
#define	FLARE_SIZE						100
#define SHOCKWAVE_SPEED	(GAME_TICKS_PER_SEC)
#define	MAX_SHOCKWAVE_SIZE				500

/* Tick counts for updates on a particular interval */
static	UDWORD	lastUpdateDroids[EFFECT_DROID_DIVISION];
static	UDWORD	lastUpdateStructures[EFFECT_STRUCTURE_DIVISION];

/* Current next slot to use - cyclic */
static	UDWORD freeEffect;

static	UDWORD	numEffects;
static	UDWORD	activeEffects;
static	UDWORD	missCount;
static	UDWORD	skipped,skippedEffects,letThrough;
static	UDWORD	auxVar; // dirty filthy hack - don't look for what this does.... //FIXME
static	UDWORD	auxVarSec; // dirty filthy hack - don't look for what this does.... //FIXME
static	UDWORD	aeCalls;
static	UDWORD	specifiedSize;
static  UDWORD	ellSpec;
static	POINT	powerHack[NUM_POWER_MODULES] = 		 // don't even ask
{
	{-90,90},
	{-90,-90},
	{90,-90},
	{90,90}
};
// ----------------------------------------------------------------------------------------
/* PROTOTYPES */
/* externals */

/* Don't even ask what this fellow does... */
void	effectResetUpdates		( void );
void	effectGiveAuxVar		( UDWORD var);
void	effectGiveAuxVarSec		( UDWORD var);
UDWORD	getFreeEffect			( void );

void	initEffectsSystem		( void );
void	drawEffects				( void );
void	processEffects			( void );			
void	addEffect				( iVector *pos, EFFECT_GROUP group, 
									EFFECT_TYPE type, BOOL specified, iIMDShape *imd, BOOL lit );
void	addMultiEffect(iVector *basePos, iVector *scatter,EFFECT_GROUP group, 
					   EFFECT_TYPE type,BOOL specified, iIMDShape *imd, UDWORD number, BOOL lit, UDWORD size);
UDWORD	getNumEffects			( void );
void	renderEffect			( EFFECT *psEffect );	// MASTER Fn
// ----------------------------------------------------------------------------------------
// ---- Update functions - every group type of effect has one of these */
void	updateEffect			( EFFECT *psEffect );		//MASTER Fn
void	updateWaypoint			( EFFECT *psEffect );
void	updateExplosion			( EFFECT *psEffect );
void	updatePolySmoke			( EFFECT *psEffect );
void	updateGraviton			( EFFECT *psEffect );
void	updateConstruction		( EFFECT *psEffect );
void	updateBlood				( EFFECT *psEffect );
void	updateDestruction		( EFFECT *psEffect );
void	updateFire				( EFFECT *psEffect );
void	updateSatLaser			( EFFECT *psEffect );
void	updateFirework			( EFFECT *psEffect );

// ----------------------------------------------------------------------------------------
// ---- The render functions - every group type of effect has a distinct one
static void	renderExplosionEffect	( EFFECT *psEffect );
static void	renderSmokeEffect		( EFFECT *psEffect );
static void	renderGravitonEffect	( EFFECT *psEffect );
static void	renderConstructionEffect( EFFECT *psEffect );
static void	renderWaypointEffect	( EFFECT *psEffect );
static void	renderBloodEffect		( EFFECT *psEffect );
static void	renderDestructionEffect	( EFFECT *psEffect );
static void renderFirework			( EFFECT *psEffect );
/* There is no render destruction effect! */

// ----------------------------------------------------------------------------------------
// ---- The set up functions - every type has one
static void	effectSetupSmoke		( EFFECT *psEffect );
static void	effectSetupGraviton		( EFFECT *psEffect );
static void	effectSetupExplosion	( EFFECT *psEffect );
static void	effectSetupConstruction ( EFFECT *psEffect );
//static void	effectSetupDust			( EFFECT *psEffect );
static void	effectSetupWayPoint		( EFFECT *psEffect );
static void	effectSetupBlood		( EFFECT *psEffect );
static void effectSetupDestruction  ( EFFECT *psEffect );
static void	effectSetupFire			( EFFECT *psEffect );
static void	effectSetUpSatLaser		( EFFECT *psEffect );
static void effectSetUpFirework		( EFFECT *psEffect );
BOOL	validatePie(EFFECT_GROUP group, EFFECT_TYPE type, iIMDShape *pie);
// ----------------------------------------------------------------------------------------
//void	initPerimeterSmoke			( EFFECT *psEffect );
void	initPerimeterSmoke			( iIMDShape *pImd, UDWORD x, UDWORD y, UDWORD z);
// ----------------------------------------------------------------------------------------
void	effectStructureUpdates	( void );
void	effectDroidUpdates		( void );

UDWORD EffectGetNumFrames(EFFECT *psEffect);
UDWORD IMDGetNumFrames(iIMDShape *Shape);

/* The fraction of a second that the last game frame took */
static	FRACT	fraction;

// ----------------------------------------------------------------------------------------
BOOL	essentialEffect(EFFECT_GROUP group, EFFECT_TYPE type)
{
	switch(group)
	{
	case	EFFECT_FIRE:
	case	EFFECT_WAYPOINT:
	case	EFFECT_DESTRUCTION:
	case	EFFECT_SAT_LASER:
	case	EFFECT_STRUCTURE:
		return(TRUE);
		break;
	case	EFFECT_EXPLOSION:
		if(type == EXPLOSION_TYPE_LAND_LIGHT)
		{
			return(TRUE);
		}
		else
		{
			return(FALSE);
		}
	default:
		return(FALSE);
		break;
	}
}
BOOL	utterlyReject(EFFECT_GROUP group, EFFECT_TYPE type)
{
	switch(group)
	{
	case EFFECT_BLOOD:
	case EFFECT_DUST_BALL:
	case EFFECT_CONSTRUCTION:
		return(TRUE);
	default:
		return(FALSE);
		break;
	}
}
// ----------------------------------------------------------------------------------------
/*	Simply sets the free pointer to the start - actually this isn't necessary
	as it will work just fine anyway. This WOULD be necessary were we to change
	the system so that it seeks FREE slots rather than the oldest one. This is because
	different effects last for different times and the oldest effect may have 
	just got going (if it was a long effect).
*/
void	initEffectsSystem( void )
{
UDWORD	i;
EFFECT	*psEffect;

  
	/* Set position to first */
	freeEffect = 0;

	/* None are active */
	numEffects = 0;

	activeEffects = 0;

	missCount=0;

	skipped = letThrough = 0;

	for(i=0; i<MAX_EFFECTS; i++)
	{
		/* Get a pointer - just cos our macro requires it, speeds not an issue here */
		psEffect = &asEffectsList[i];
		/* Clear all the control bits */
		psEffect->control = (UBYTE)0;
		/* All effects are initially inactive */
		asEffectsList[i].status = ES_INACTIVE;
	}
}

// ----------------------------------------------------------------------------------------
void	effectSetLandLightSpec(LAND_LIGHT_SPEC spec)
{
	ellSpec = spec;
}
// ----------------------------------------------------------------------------------------
void	effectSetSize(UDWORD size)
{
	specifiedSize = size;
}
// ----------------------------------------------------------------------------------------
void	addMultiEffect(iVector *basePos, iVector *scatter,EFFECT_GROUP group, 
					   EFFECT_TYPE type,BOOL specified, iIMDShape *imd, UDWORD number,BOOL lit,UDWORD size)
{
UDWORD	i;
iVector	scatPos;


	if(number==0)
	{
		return;
	}
	/* Set up the scaling for specified ones */
	specifiedSize = size;

	/* If there's only one, make sure it's in the centre */
	if(number == 1)
	{
		scatPos.x = basePos->x;
		scatPos.y = basePos->y;
		scatPos.z = basePos->z;
		addEffect(&scatPos,group,type,specified,imd,lit);
	}
	else
	{
		/* Fix for jim */
		scatter->x/=10;
		scatter->y/=10;
		scatter->z/=10;

		/* There are multiple effects - so scatter them around according to parameter */
		for(i=0; i<number; i++)
		{
			scatPos.x = basePos->x + (scatter->x ? ( scatter->x	- (rand()%(2*scatter->x)) ) : 0 );
			scatPos.y = basePos->y + (scatter->y ? ( scatter->y	- (rand()%(2*scatter->y)) ) : 0 );
			scatPos.z = basePos->z + (scatter->z ? ( scatter->z	- (rand()%(2*scatter->z)) ) : 0 );
			addEffect(&scatPos,group,type,specified,imd,lit);
		}
	}
}

// ----------------------------------------------------------------------------------------
UDWORD	getNumActiveEffects( void )
{
	return(activeEffects);
}
// ----------------------------------------------------------------------------------------
UDWORD	getMissCount( void )
{
	return(missCount);
}

UDWORD	getNumSkippedEffects(void)
{
	return(skippedEffects);
}

UDWORD	getNumEvenEffects(void)
{
	return(letThrough);
}
// ----------------------------------------------------------------------------------------

UDWORD Reject1;

void	addEffect(iVector *pos, EFFECT_GROUP group, EFFECT_TYPE type,BOOL specified, iIMDShape *imd, BOOL lit)
{
UDWORD	essentialCount;
UDWORD	i;
BOOL	bSmoke;

	aeCalls++;

	if(gamePaused())
	{
		return;
	}


	/* Quick optimsation to reject every second non-essential effect if it's off grid */
//	if(clipXY((UDWORD)MAKEINT(pos->x),(UDWORD)MAKEINT(pos->z)) == FALSE)
	if(clipXY((UDWORD)pos->x,(UDWORD)pos->z) == FALSE)
	{
		/* 	If effect is essentail - then let it through */
	  	if(!essentialEffect(group,type) )
		{
			/* Some we can get rid of right away */
			if(utterlyReject(group,type))
			{
				skipped++;
				return;
			}
			/* Smoke gets culled more than most off grid effects */
			if(group == EFFECT_SMOKE)
			{
				bSmoke = TRUE;
			}
			else
			{
				bSmoke = FALSE;
			}
			/* Others intermittently (50/50 for most and 25/100 for smoke */
			if(bSmoke ? (aeCalls & 0x03) : (aeCalls & 0x01) )
			{
				/* Do one */
				skipped++;
				return;			
			}
			letThrough++;
		}
	}


	for(i=freeEffect,essentialCount=0; (asEffectsList[i].control & EFFECT_ESSENTIAL) 
		AND essentialCount<MAX_EFFECTS; i++)
	{
		/* Check for wrap around */
		if(i>= (MAX_EFFECTS-1))
		{
			/* Go back to the first one */
			i = 0;
		}
		essentialCount++;
		missCount++;
	}

	/* Check the list isn't just full of essential effects */
	if(essentialCount>=MAX_EFFECTS)
	{
		/* All of the effects are essential!?!? */
		return;
	}
	else
	{
		freeEffect = i;
	}

	/* Store away it's position - into FRACTS */
	asEffectsList[freeEffect].position.x = MAKEFRACT(pos->x);
	asEffectsList[freeEffect].position.y = MAKEFRACT(pos->y);
	asEffectsList[freeEffect].position.z = MAKEFRACT(pos->z);



	/* Now, note group and type */
	asEffectsList[freeEffect].group =(UBYTE) group;
	asEffectsList[freeEffect].type = (UBYTE) type;

	/* Set when it entered the world */
	asEffectsList[freeEffect].birthTime = asEffectsList[freeEffect].lastFrame = gameTime;

	if(group == EFFECT_GRAVITON AND (type == GRAVITON_TYPE_GIBLET OR type == GRAVITON_TYPE_EMITTING_DR))
	{
		asEffectsList[freeEffect].frameNumber = lit;
	}

	else
	{
		/* Starts off on frame zero */
		asEffectsList[freeEffect].frameNumber = 0;
	}

	/*	
		See what kind of effect it is - the add fucnction is different for each,
		although some things are shared
	*/
	asEffectsList[freeEffect].imd = NULL;
	if(lit)
	{
		SET_LITABS(asEffectsList[freeEffect]);
	}
	
	if(specified)
	{
		/* We're specifying what the imd is - override */
		asEffectsList[freeEffect].imd = imd;
//		if(type == EXPLOSION_TYPE_SPECIFIED_FIXME)
//		{
//			asEffectsList[freeEffect].size = EXPLOSION_SIZE;
//		}
//		else
//		{
		asEffectsList[freeEffect].size =(UWORD) specifiedSize;
//		}
	}
	
	/* Do all the effect type specific stuff */
	switch(group)
	{
		case EFFECT_SMOKE:
			effectSetupSmoke(&asEffectsList[freeEffect]);
			break;
		case EFFECT_GRAVITON:
			effectSetupGraviton(&asEffectsList[freeEffect]);
			break;
		case EFFECT_EXPLOSION:
			effectSetupExplosion(&asEffectsList[freeEffect]);
			break;
		case EFFECT_CONSTRUCTION:
//			effectSetupDust(&asEffectsList[freeEffect]);
			effectSetupConstruction(&asEffectsList[freeEffect]);
			break;
		case EFFECT_WAYPOINT:
			effectSetupWayPoint(&asEffectsList[freeEffect]);
			break;
		case EFFECT_BLOOD:
			effectSetupBlood(&asEffectsList[freeEffect]);
			break;
		case EFFECT_DESTRUCTION:
			effectSetupDestruction(&asEffectsList[freeEffect]);
			break;
		case EFFECT_FIRE:
			effectSetupFire(&asEffectsList[freeEffect]);
			break;
		case EFFECT_SAT_LASER:
			effectSetUpSatLaser(&asEffectsList[freeEffect]);
			break;
		case EFFECT_FIREWORK:
			effectSetUpFirework(&asEffectsList[freeEffect]);
			break;
		default:
			ASSERT((FALSE,"Weirdy group type for an effect"));
			break;
	}

	/* Make the effect active */
	asEffectsList[freeEffect].status = ES_ACTIVE;

	/* As of yet, it hasn't bounced (or whatever)... */
	if(type!=EXPLOSION_TYPE_LAND_LIGHT)
	{
		asEffectsList[freeEffect].specific = 0;
	}

	/* Looks like we didn't establish an imd for the effect */
	/*
	ASSERT((asEffectsList[freeEffect].imd != NULL OR group == EFFECT_DESTRUCTION OR group == EFFECT_FIRE OR group == EFFECT_SAT_LASER,
		"null effect imd"));
	*/

#ifdef DEBUG
	if(validatePie(group,type,asEffectsList[freeEffect].imd) == FALSE)
	{
		ASSERT((FALSE,"No PIE found or specified for an effect"));
	}
#endif

	/* No more slots available? */
	if(freeEffect++ >= (MAX_EFFECTS-1))
	{
		/* Go back to the first one */
		freeEffect = 0;
	}
}

#ifdef DEBUG
// ----------------------------------------------------------------------------------------
BOOL	validatePie(EFFECT_GROUP group, EFFECT_TYPE type, iIMDShape *pie)
{
	
	/* If we haven't got a pie */
	if(pie == NULL)
	{
		if(group == EFFECT_DESTRUCTION OR group == EFFECT_FIRE OR group == EFFECT_SAT_LASER)
		{
			/* Ok in these cases */
			return(TRUE);
		}
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}
// ----------------------------------------------------------------------------------------
#endif
/* Calls all the update functions for each different currently active effect */
void	processEffects(void)
{
UDWORD	i;
UDWORD	num;
	
	/* Establish how long the last game frame took */
	fraction = MAKEFRACT(frameTime)/GAME_TICKS_PER_SEC;
	num=0;
	missCount = 0;

	for(i=0; i<MAX_EFFECTS; i++)
	{
		/* Is it active */
		switch(asEffectsList[i].status)
		{
		/* The effect is active */
		case ES_ACTIVE:
			/* So process it */
			updateEffect(&asEffectsList[i]);
			num++;
			break;
		case ES_DORMANT:
			/* Might be useful? */
			break;
		default:
			break;
		}
	}

	/* Add any droid effects */
	effectDroidUpdates();

	/* Add any structure effects */
	effectStructureUpdates();

	activeEffects = num;
	skippedEffects = skipped;
}

// ----------------------------------------------------------------------------------------
/*
drawEffects:-
This will either draw all the effects that are on the grid in a oner or
more likely add them to the bucket.
*/
void	drawEffects( void )
{
UDWORD	i;

	/* Reset counter */	
	numEffects = 0;

	/* Traverse the list */
	for(i=0; i<MAX_EFFECTS; i++)
	{
		/* Don't bother unless it's active */
		if(asEffectsList[i].status == ES_ACTIVE)
		{
			/* One more is active */
			numEffects++;
			/* Is it on the grid */
			if(clipXY((UDWORD)MAKEINT(asEffectsList[i].position.x),(UDWORD)MAKEINT(asEffectsList[i].position.z)))
			{
#ifndef BUCKET
				/* Draw it right now */
				renderEffect(&asEffectsList[i]);
#else
				/* Add it to the bucket */
				bucketAddTypeToList(RENDER_EFFECT,&asEffectsList[i]);
#endif
			}
		}
	}
}


// ----------------------------------------------------------------------------------------
/* The general update function for all effects - calls a specific one for each */
void	updateEffect(EFFECT *psEffect)
{
	/* What type of effect are we dealing with? */
	switch(psEffect->group)
	{
	case EFFECT_EXPLOSION:
		updateExplosion(psEffect);
		break;

	case EFFECT_WAYPOINT:
		if(!gamePaused()) updateWaypoint(psEffect);
		break;

	case EFFECT_CONSTRUCTION:
		if(!gamePaused()) updateConstruction(psEffect);
		break;

	case EFFECT_SMOKE:
		if(!gamePaused()) updatePolySmoke(psEffect);
		break;

	case EFFECT_STRUCTURE:
		break;

	case EFFECT_GRAVITON:
		if(!gamePaused()) updateGraviton(psEffect);
		break;

	case EFFECT_BLOOD:
		if(!gamePaused()) updateBlood(psEffect);
		break;

	case EFFECT_DESTRUCTION:
		if(!gamePaused()) updateDestruction(psEffect);
		break;

	case EFFECT_FIRE:
		if(!gamePaused()) updateFire(psEffect);
		break;

	case EFFECT_SAT_LASER:
		if(!gamePaused()) updateSatLaser(psEffect);
		break;
	case EFFECT_FIREWORK:
		if(!gamePaused()) updateFirework(psEffect);
		break;
	default:
		DBERROR(("Weirdy class of effect passed to updateEffect"));
		break;
	}
}

// ----------------------------------------------------------------------------------------
// ALL THE UPDATE FUNCTIONS 
// ----------------------------------------------------------------------------------------
/* Update the waypoint effects.*/
void	updateWaypoint(EFFECT *psEffect)
{
	if(!(keyDown(KEY_LCTRL) || keyDown(KEY_RCTRL) ||
	     keyDown(KEY_LSHIFT) || keyDown(KEY_RSHIFT)))
	{
		KILL_EFFECT(psEffect);
	}
}


// ----------------------------------------------------------------------------------------
void	updateFirework(EFFECT *psEffect)
{
#ifdef WIN32
UDWORD	height;
UDWORD	xDif,yDif,radius,val;
iVector	dv;
UDWORD	dif;
UDWORD	drop;

	
	/* Move it */
	psEffect->position.x += (psEffect->velocity.x * fraction);
	psEffect->position.y += (psEffect->velocity.y * fraction);
	psEffect->position.z += (psEffect->velocity.z * fraction);

	if(psEffect->type == FIREWORK_TYPE_LAUNCHER)
	{
		height = MAKEINT(psEffect->position.y);
		if(height > psEffect->size)
		{
			dv.x = MAKEINT(psEffect->position.x); 
   			dv.z = MAKEINT(psEffect->position.z);
   			dv.y = MAKEINT(psEffect->position.y) + (psEffect->radius/2); 
			addEffect(&dv,EFFECT_EXPLOSION,EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0);
			audio_PlayStaticTrack( MAKEINT(psEffect->position.x), MAKEINT(psEffect->position.z), ID_SOUND_EXPLOSION );

//			stepHeight = psEffect->radius/15;
//			stepAngle = psEffect->radius/15;
			for(dif =0; dif < (psEffect->radius*2); dif+=20)
			{
				if(dif<psEffect->radius)
				{
  					drop = psEffect->radius-dif;	
				}
				else
				{
					drop = dif - psEffect->radius;
				}
				radius = (UDWORD)(sqrt((psEffect->radius*psEffect->radius) - (drop*drop)));
				//val = getStaticTimeValueRange(720,360);	// grab an angle - 4 seconds cyclic
  				for(val = 0; val<=180; val+=20)
				{
   					xDif = radius * (SIN(DEG(val)));
   					yDif = radius * (COS(DEG(val)));
   					xDif = xDif/4096;	 // cos it's fixed point
   					yDif = yDif/4096;
   					dv.x = MAKEINT(psEffect->position.x)+xDif; 
   					dv.z = MAKEINT(psEffect->position.z)+yDif;
   					dv.y = MAKEINT(psEffect->position.y)+dif; 
					effectGiveAuxVar(100);
   					addEffect(&dv,EFFECT_FIREWORK, FIREWORK_TYPE_STARBURST,FALSE,NULL,0);
					dv.x = MAKEINT(psEffect->position.x)-xDif; 
   					dv.z = MAKEINT(psEffect->position.z)-yDif;
   					dv.y = MAKEINT(psEffect->position.y)+dif; 
					effectGiveAuxVar(100);
   					addEffect(&dv,EFFECT_FIREWORK, FIREWORK_TYPE_STARBURST,FALSE,NULL,0);
					dv.x = MAKEINT(psEffect->position.x)+xDif; 
   					dv.z = MAKEINT(psEffect->position.z)-yDif;
   					dv.y = MAKEINT(psEffect->position.y)+dif; 
					effectGiveAuxVar(100);
   					addEffect(&dv,EFFECT_FIREWORK, FIREWORK_TYPE_STARBURST,FALSE,NULL,0);
					dv.x = MAKEINT(psEffect->position.x)-xDif; 
   					dv.z = MAKEINT(psEffect->position.z)+yDif;
   					dv.y = MAKEINT(psEffect->position.y)+dif; 
					effectGiveAuxVar(100);
   					addEffect(&dv,EFFECT_FIREWORK, FIREWORK_TYPE_STARBURST,FALSE,NULL,0);

					//   			dv.x = dv.x - (2*xDif); 
	//   			dv.z = dv.z - (2*yDif);	// buildings are level!
	//			effectGiveAuxVar(100);
	//   			addEffect(&dv,EFFECT_FIREWORK, FIREWORK_TYPE_STARBURST,FALSE,NULL,0);
				}
			}
			KILL_EFFECT(psEffect);

		}
		else
		{
			/* Add an effect at the firework's position */
			dv.x = MAKEINT(psEffect->position.x);
			dv.y = MAKEINT(psEffect->position.y);
			dv.z = MAKEINT(psEffect->position.z);

			/* Add a trail graphic */
			addEffect(&dv,EFFECT_SMOKE,SMOKE_TYPE_TRAIL,FALSE,NULL,0);
		}
	}
	else	// must be a startburst
	{
			/* Time to update the frame number on the smoke sprite */
		if(gameTime - psEffect->lastFrame > psEffect->frameDelay)
		{
			/* Store away last frame change time */
			psEffect->lastFrame = gameTime;

			/* Are we on the last frame? */
			if(++psEffect->frameNumber >= EffectGetNumFrames(psEffect))
			{
				/* Does the anim wrap around? */
				if(TEST_CYCLIC(psEffect))
				{
					psEffect->frameNumber = 0;
				}
				else
				{
					/* Kill it off */
 					KILL_EFFECT(psEffect);
					return;
				}
			}
		}

		/* If it doesn't get killed by frame number, then by age */
		if(TEST_CYCLIC(psEffect))
		{
			/* Has it overstayed it's welcome? */
			if(gameTime - psEffect->birthTime > psEffect->lifeSpan)
			{
				/* Kill it */
				KILL_EFFECT(psEffect);
			}
		}


	}


#endif
}
// ----------------------------------------------------------------------------------------
void	updateSatLaser(EFFECT *psEffect)
{
iVector	dv;
UDWORD	val;
UDWORD	radius;
UDWORD	xDif,yDif;
UDWORD	i;
UDWORD	startHeight,endHeight;
iIMDShape	*pie;
UDWORD	xPos,yPos;
LIGHT	light;

	// Do these here cause there used by the lighting code below this if.
	xPos = MAKEINT(psEffect->position.x);
	startHeight = MAKEINT(psEffect->position.y);
	endHeight = startHeight+1064;
	yPos = MAKEINT(psEffect->position.z);
	
	if(psEffect->baseScale)
	{
		psEffect->baseScale = 0;

		pie = getImdFromIndex(MI_FLAME);

//printf("%d %d : %d %d : %p\n",xPos,yPos,startHeight,endHeight,pie);
		/* Add some big explosions....! */

		for(i=0; i<16; i++)
		{
			dv.x = xPos+(200-rand()%400);
			dv.z = yPos+(200-rand()%400);
			dv.y = startHeight + rand()%100;
			addEffect(&dv,EFFECT_EXPLOSION,EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0);
		}
		/* Add a sound effect */
  		audio_PlayStaticTrack( MAKEINT(psEffect->position.x), MAKEINT(psEffect->position.z), ID_SOUND_EXPLOSION );

		/* Add a shockwave */
		dv.x = xPos;
		dv.z = yPos;
		dv.y = startHeight+SHOCK_WAVE_HEIGHT;
		addEffect(&dv,EFFECT_EXPLOSION,EXPLOSION_TYPE_SHOCKWAVE,FALSE,NULL,0);


		/* Now, add the column of light */
#ifdef WIN32
		for(i=startHeight; i<endHeight; i+=56)
#else
		for(i=startHeight; i<endHeight; i+=56*4)
#endif
		{
#ifdef WIN32
			radius = 80;
			/* Add 36 around in a circle..! */
			for(val = 0; val<=180; val+=30)
#else
			radius = 40;
//			for(val = 0; val<=180; val+=180)
			val = 0;
#endif
			{
   				xDif = radius * (SIN(DEG(val)));
   				yDif = radius * (COS(DEG(val)));
   				xDif = xDif/4096;	 // cos it's fixed point
   				yDif = yDif/4096;
   				dv.x = xPos+xDif+i/64; 
   				dv.z = yPos+yDif;
   				dv.y = startHeight+i; 
				effectGiveAuxVar(100);
   				addEffect(&dv,EFFECT_EXPLOSION, EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0);
				dv.x = xPos-xDif+i/64; 
   				dv.z = yPos-yDif;
   				dv.y = startHeight+i; 
				effectGiveAuxVar(100);
   				addEffect(&dv,EFFECT_EXPLOSION, EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0);
				dv.x = xPos+xDif+i/64; 
   				dv.z = yPos-yDif;
   				dv.y = startHeight+i; 
				effectGiveAuxVar(100);
   				addEffect(&dv,EFFECT_EXPLOSION, EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0);
				dv.x = xPos-xDif+i/64; 
   				dv.z = yPos+yDif;
   				dv.y = startHeight+i; 
				effectGiveAuxVar(100);
   				addEffect(&dv,EFFECT_EXPLOSION, EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0);
			}
		}
	}
	
//printf("%d %d\n",gameTime,psEffect->birthTime);
	if(gameTime-psEffect->birthTime < 1000)
	{
  		light.position.x = xPos;
		light.position.y = startHeight;
		light.position.z = yPos;
		light.range = 800;
		light.colour = LIGHT_BLUE;
		processLight(&light);

	}
	else
	{
		KILL_EFFECT(psEffect);
	}
}
// ----------------------------------------------------------------------------------------
/* The update function for the explosions */
void	updateExplosion(EFFECT *psEffect)
{
LIGHT	light;
UDWORD	percent;
UDWORD	range;
FRACT	scaling;

if (pie_Hardware())//pc only on hardware
{
	if(TEST_LIT(psEffect))
	{
		if(psEffect->lifeSpan) 
		{
			percent = PERCENT(gameTime-psEffect->birthTime,psEffect->lifeSpan);
			if(percent > 100)  
			{
				percent = 100;
			}
			else
			{
				if(percent>50)
				{
					percent = 100-percent;
				}
			}
		} 
		else 
		{
			percent = 100;
		}
 
		range = percent;
//#ifdef DOLIGHTS
		light.position.x = MAKEINT(psEffect->position.x);
		light.position.y = MAKEINT(psEffect->position.y);
		light.position.z = MAKEINT(psEffect->position.z);
		light.range = (3*range)/2;
		light.colour = LIGHT_RED;
		processLight(&light);
//#endif
	}
}

#ifdef DOLIGHTS
/*
	if(psEffect->type == EXPLOSION_TYPE_LAND_LIGHT)
	{
		light.position.x = MAKEINT(psEffect->position.x);
		light.position.y = MAKEINT(psEffect->position.y);
		light.position.z = MAKEINT(psEffect->position.z);
		light.range = getTimeValueRange(1024,512);
		if(light.range>256) light.range = 512-light.range;
		light.colour = LIGHT_RED;
		processLight(&light);
	}
*/
#endif
	
	if(psEffect->type == EXPLOSION_TYPE_SHOCKWAVE)
	{
		psEffect->size += MAKEINT((fraction*SHOCKWAVE_SPEED));
		scaling = MAKEFRACT(psEffect->size)/MAX_SHOCKWAVE_SIZE;
		psEffect->frameNumber = MAKEINT(scaling*EffectGetNumFrames(psEffect));
#ifdef DOLIGHTS
		light.position.x = MAKEINT(psEffect->position.x);
		light.position.y = MAKEINT(psEffect->position.y);
		light.position.z = MAKEINT(psEffect->position.z);
		light.range = psEffect->size+200;
		light.colour = LIGHT_YELLOW;
		processLight(&light);
#endif
		if(psEffect->size>MAX_SHOCKWAVE_SIZE OR light.range>600)
		{
 			/* Kill it off */
			KILL_EFFECT(psEffect);
			return;

		}
	}
	
	/* Time to update the frame number on the explosion */
	else 
		if(gameTime - psEffect->lastFrame > psEffect->frameDelay)
		{
			psEffect->lastFrame = gameTime;
			/* Are we on the last frame? */

			if(++psEffect->frameNumber >= EffectGetNumFrames(psEffect))
			{
		 		if(psEffect->type!=EXPLOSION_TYPE_LAND_LIGHT)
				{
		 			/* Kill it off */
					KILL_EFFECT(psEffect);
					return;
				}
				else
				{
					psEffect->frameNumber = 0;
				}
			}
		}

	if(!gamePaused())
	{
		/* Tesla explosions are the only ones that rise, or indeed move */
		if(psEffect->type == EXPLOSION_TYPE_TESLA)
		{
			psEffect->position.y += (MAKEINT(psEffect->velocity.y) * fraction);
		}
	}
}
// ----------------------------------------------------------------------------------------
/* The update function for blood */
void	updateBlood(EFFECT *psEffect)
{
	/* Time to update the frame number on the blood */
	if(gameTime - psEffect->lastFrame > psEffect->frameDelay)
	{
		psEffect->lastFrame = gameTime;
		/* Are we on the last frame? */
		if(++psEffect->frameNumber >= EffectGetNumFrames(psEffect))
		{
			/* Kill it off */
			KILL_EFFECT(psEffect);
			return;
		}
	}
	/* Move it about in the world */
	psEffect->position.x += (psEffect->velocity.x * fraction);
	psEffect->position.y += (psEffect->velocity.y * fraction);
	psEffect->position.z += (psEffect->velocity.z * fraction);
}

// ----------------------------------------------------------------------------------------
/* Processes all the drifting smoke 
	Handles the smoke puffing out the factory as well */
void	updatePolySmoke(EFFECT *psEffect)
{

	/* Time to update the frame number on the smoke sprite */
	if(gameTime - psEffect->lastFrame > psEffect->frameDelay)
	{
		/* Store away last frame change time */
		psEffect->lastFrame = gameTime;

		/* Are we on the last frame? */
		if(++psEffect->frameNumber >= EffectGetNumFrames(psEffect))
		{
			/* Does the anim wrap around? */
			if(TEST_CYCLIC(psEffect))
			{
				/* Does it change drift direction? */
				if(psEffect->type == SMOKE_TYPE_DRIFTING)
				{
					/* Make it change direction */
					psEffect->velocity.x = MAKEFRACT((rand()%20));
					psEffect->velocity.z = MAKEFRACT((10-rand()%20));
					psEffect->velocity.y = MAKEFRACT((10+rand()%20));
				}
				/* Reset the frame */
				psEffect->frameNumber = 0;
			}
			else
			{
				/* Kill it off */
 				KILL_EFFECT(psEffect);
				return;
			}
		}
	}

	/* Update position */
	psEffect->position.x += (psEffect->velocity.x * fraction);
	psEffect->position.y += (psEffect->velocity.y * fraction);
	psEffect->position.z += (psEffect->velocity.z * fraction);

	/* If it doesn't get killed by frame number, then by age */
	if(TEST_CYCLIC(psEffect))
	{
		/* Has it overstayed it's welcome? */
		if(gameTime - psEffect->birthTime > psEffect->lifeSpan)
		{
			/* Kill it */
			KILL_EFFECT(psEffect);
		}
	}
}

// ----------------------------------------------------------------------------------------
/* 
	Gravitons just fly up for a bit and then drop down and are
	killed off when they hit the ground
*/
void	updateGraviton(EFFECT *psEffect)
{
FRACT	accel;
iVector	dv;
UDWORD	groundHeight;
MAPTILE	*psTile;

if (pie_Hardware())//pc only on hardware
{
LIGHT	light;
#ifdef DOLIGHTS
	if(psEffect->type!=GRAVITON_TYPE_GIBLET)
	{
		light.position.x = MAKEINT(psEffect->position.x);
		light.position.y = MAKEINT(psEffect->position.y);
		light.position.z = MAKEINT(psEffect->position.z);
		light.range = 128;
		light.colour = LIGHT_YELLOW;
		processLight(&light);
	}
#endif
}

	if(gamePaused())
	{
		/* Only update the lights if it's paused */
		return;
	}
	/* Move it about in the world */
		psEffect->position.x += (psEffect->velocity.x * fraction);
		psEffect->position.y += (psEffect->velocity.y * fraction);			
		psEffect->position.z += (psEffect->velocity.z * fraction);
	/* If it's bounced/drifted off the map then kill it */
	if(((UDWORD)MAKEINT(psEffect->position.x)/TILE_UNITS >= mapWidth) OR 
		(UDWORD)MAKEINT(psEffect->position.z)/TILE_UNITS >= mapHeight)
	{
		KILL_EFFECT(psEffect);
		return;
	}

	groundHeight = map_Height((UDWORD)MAKEINT(psEffect->position.x),(UDWORD)MAKEINT(psEffect->position.z));

	/* If it's going up and it's still under the landscape, then remove it... */
	if(psEffect->position.y<groundHeight AND MAKEINT(psEffect->velocity.y)>0)
	{
		KILL_EFFECT(psEffect);
		return;
	}

	/* Does it emit a trail? And is it high enough? */
	if( (psEffect->type == GRAVITON_TYPE_EMITTING_DR) OR (psEffect->type == GRAVITON_TYPE_EMITTING_ST)
		AND (psEffect->position.y>(groundHeight+10)))
	{
		/* Time to add another trail 'thing'? */
		if(gameTime>psEffect->lastFrame+psEffect->frameDelay)
		{
			/* Store away last update */
			psEffect->lastFrame = gameTime;

			/* Add an effect at the gravitons's position */
			dv.x = MAKEINT(psEffect->position.x);
			dv.y = MAKEINT(psEffect->position.y);
			dv.z = MAKEINT(psEffect->position.z);

			/* Add a trail graphic */
			addEffect(&dv,EFFECT_SMOKE,SMOKE_TYPE_TRAIL,FALSE,NULL,0);
		}	
	}

	else if(psEffect->type == GRAVITON_TYPE_GIBLET AND (psEffect->position.y>(groundHeight+5)))
	{
		/* Time to add another trail 'thing'? */
		if(gameTime>psEffect->lastFrame+psEffect->frameDelay)
		{
			/* Store away last update */
			psEffect->lastFrame = gameTime;

			/* Add an effect at the gravitons's position */
			dv.x = MAKEINT(psEffect->position.x);
			dv.y = MAKEINT(psEffect->position.y);
			dv.z = MAKEINT(psEffect->position.z);
			addEffect(&dv,EFFECT_BLOOD,BLOOD_TYPE_NORMAL,FALSE,NULL,0);
		}
	}

	/* Spin it round a bit */
	psEffect->rotation.x += MAKEINT(((FRACT)psEffect->spin.x) * fraction);
	psEffect->rotation.y += MAKEINT(((FRACT)psEffect->spin.y) * fraction);
	psEffect->rotation.z += MAKEINT(((FRACT)psEffect->spin.z) * fraction);

	/* Update velocity (and retarding of descent) according to present frame rate */
	accel = (GRAVITON_GRAVITY*fraction);
	psEffect->velocity.y += accel;

	/* If it's bounced/drifted off the map then kill it */
	if((MAKEINT(psEffect->position.x) <= TILE_UNITS) OR 
		MAKEINT(psEffect->position.z) <= TILE_UNITS)
	{
		KILL_EFFECT(psEffect);
		return;
	}

	/* Are we below it? - Hit the ground? */
	if( (MAKEINT(psEffect->position.y) < (SDWORD)groundHeight))
	{
		psTile = mapTile((MAKEINT(psEffect->position.x))>>TILE_SHIFT,(MAKEINT(psEffect->position.z))>>TILE_SHIFT);
	   	if(TERRAIN_TYPE(psTile) == TER_WATER)
		{
			KILL_EFFECT(psEffect);
			return;
		}
		else
		/* Are we falling - rather than rising? */
   		if(MAKEINT(psEffect->velocity.y)<0)
		{
			/* Has it sufficient energy to keep bouncing? */
			if(abs(MAKEINT(psEffect->velocity.y))>16 AND psEffect->specific <=2) 
			{
				psEffect->specific++;
				/* Half it's velocity */
//				psEffect->velocity.x/=(FRACT)(2);
				psEffect->velocity.y/=(FRACT)(-2); // only y gets flipped
//				psEffect->velocity.z/=(FRACT)(2);
				/* Set it at ground level - may have gone through */
				psEffect->position.y = MAKEFRACT(groundHeight);
			}
			else
			{
  				/* Giblets don't blow up when they hit the ground! */
				if(psEffect->type!=GRAVITON_TYPE_GIBLET)
				{
					/* Remove the graviton and add an explosion */
					dv.x = MAKEINT(psEffect->position.x);
					dv.y = MAKEINT(psEffect->position.y+10);
					dv.z = MAKEINT(psEffect->position.z);
					addEffect(&dv,EFFECT_EXPLOSION,EXPLOSION_TYPE_VERY_SMALL,FALSE,NULL,0);
				}
				KILL_EFFECT(psEffect);	
				return;
			}
		}
	}
}


// ----------------------------------------------------------------------------------------
/* updateDestruction
This isn't really an on-screen effect itself - it just spawns other ones....
  */
void	updateDestruction(EFFECT *psEffect)
{
iVector	pos;
UDWORD	effectType;
UDWORD	widthScatter,breadthScatter, heightScatter;
SDWORD	iX, iY;
LIGHT	light;
UDWORD	percent;
UDWORD	range;
FRACT	div;
UDWORD	height;

if (pie_Hardware())//pc only on hardware
{
	percent = PERCENT(gameTime-psEffect->birthTime,psEffect->lifeSpan);
	if(percent > 100)  
	{
		percent = 100;
	}
	range = 50 - abs(50-percent);
#ifdef DOLIGHTS
	light.position.x = MAKEINT(psEffect->position.x);
	light.position.y = MAKEINT(psEffect->position.y);
	light.position.z = MAKEINT(psEffect->position.z);
	if(psEffect->type == DESTRUCTION_TYPE_STRUCTURE)
	{
		light.range = range*10;
	}
	else
	{
		light.range = range*4;
	}
	if(psEffect->type == DESTRUCTION_TYPE_POWER_STATION)
	{
		light.range *=3;
		light.colour = LIGHT_WHITE;
	}
	else
	{
		light.colour = LIGHT_RED;
	}
	processLight(&light);
#endif
}


	if(gameTime > (psEffect->birthTime + psEffect->lifeSpan))
	{
		/* Kill it - it's too old */
		KILL_EFFECT(psEffect);
		return;
	}

	if(psEffect->type == DESTRUCTION_TYPE_SKYSCRAPER)
	{
		
		if((gameTime - psEffect->birthTime) > ((9*psEffect->lifeSpan)/10))
		{
			pos.x = MAKEINT(psEffect->position.x);
			pos.z = MAKEINT(psEffect->position.z); 
			pos.y = MAKEINT(psEffect->position.y);
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LARGE,FALSE,NULL,0); 
			KILL_EFFECT(psEffect);
			return;
		}

		div = MAKEFRACT(gameTime - psEffect->birthTime)/psEffect->lifeSpan;
		if(div>MAKEFRACT(1)) div=MAKEFRACT(1);
		div = MAKEFRACT(1)-div;
		height = MAKEINT(div*psEffect->imd->ymax);
	}
	else
	{
		height = 16;
	}


	/* Time to add another effect? */
	if((gameTime - psEffect->lastFrame) > psEffect->frameDelay)
	{
		psEffect->lastFrame = gameTime;
		switch(psEffect->type)
		{
		case DESTRUCTION_TYPE_SKYSCRAPER:
			widthScatter = TILE_UNITS;
			breadthScatter = TILE_UNITS;
			heightScatter = TILE_UNITS;
			break;

		case DESTRUCTION_TYPE_POWER_STATION:
		case DESTRUCTION_TYPE_STRUCTURE:		
			widthScatter = TILE_UNITS/2;
			breadthScatter = TILE_UNITS/2;
			heightScatter = TILE_UNITS/4;
			break;

		case DESTRUCTION_TYPE_DROID:
		case DESTRUCTION_TYPE_WALL_SECTION:
		case DESTRUCTION_TYPE_FEATURE:
			widthScatter = TILE_UNITS/6;
			breadthScatter = TILE_UNITS/6;
			heightScatter = TILE_UNITS/6;
			break;
		default:
			ASSERT((FALSE,"Weirdy destruction type effect"));
			break;
		}
		

		/* Find a position to dump it at */
		pos.x = MAKEINT(psEffect->position.x) + widthScatter - rand()%(2*widthScatter);
		pos.z = MAKEINT(psEffect->position.z) + breadthScatter - rand()%(2*breadthScatter);
		pos.y = MAKEINT(psEffect->position.y) + height + rand()%heightScatter;

		if(psEffect->type == DESTRUCTION_TYPE_SKYSCRAPER)
		{
	  		pos.y = MAKEINT(psEffect->position.y) + height;
  		}


		/* Choose an effect */
		effectType = rand()%15;
		switch(effectType)
		{
		case 0:
			addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_DRIFTING,FALSE,NULL,0);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if(psEffect->type == DESTRUCTION_TYPE_SKYSCRAPER)
			{
				addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_LARGE,FALSE,NULL,0); 
			}
			/* Only structures get the big explosions */
			else if(psEffect->type==DESTRUCTION_TYPE_STRUCTURE)
			{
				addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_MEDIUM,FALSE,NULL,0); 
			}
			else
			{
				addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
			}
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			if(psEffect->type == DESTRUCTION_TYPE_STRUCTURE)
			{
//				addEffect(&pos,EFFECT_GRAVITON,GRAVITON_TYPE_EMITTING_ST,TRUE,debrisImds[rand()%MAX_DEBRIS],0);
				addEffect(&pos,EFFECT_GRAVITON,GRAVITON_TYPE_EMITTING_ST,TRUE,getRandomDebrisImd(),0);
			}
			else
			{
				addEffect(&pos,EFFECT_GRAVITON,GRAVITON_TYPE_EMITTING_DR,TRUE,getRandomDebrisImd(),0);
			}
			break;
		case 11:
			addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_DRIFTING,FALSE,NULL,0);
			break;
		case 12:
		case 13:
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
			break;
		case 14:
			/* Add sound effect, but only if we're less than 3/4 of the way thru' destruction */
			if( gameTime < ((3*(psEffect->birthTime + psEffect->lifeSpan)/4)) )
			{
				iX = MAKEINT(psEffect->position.x);
				iY = MAKEINT(psEffect->position.z);
				audio_PlayStaticTrack( iX, iY, ID_SOUND_EXPLOSION );
			}
			break;

		}
	}
}
// ----------------------------------------------------------------------------------------
/* 
updateConstruction:-
Moves the construction graphic about - dust cloud or whatever....
*/
void	updateConstruction(EFFECT *psEffect)
{

	/* Time to update the frame number on the construction sprite */
	if(gameTime - psEffect->lastFrame > psEffect->frameDelay)
	{
		psEffect->lastFrame = gameTime;
		/* Are we on the last frame? */
		if(++psEffect->frameNumber >= EffectGetNumFrames(psEffect))
		{
			/* Is it a cyclic sprite? */
			if(TEST_CYCLIC(psEffect))
			{
				psEffect->frameNumber = 0;
			}
			else
			{
				KILL_EFFECT(psEffect);
				return;
			}
		}
	}

	/* Move it about in the world */
	psEffect->position.x += (psEffect->velocity.x * fraction);
	psEffect->position.y += (psEffect->velocity.y * fraction);
	psEffect->position.z += (psEffect->velocity.z * fraction);

	/* If it doesn't get killed by frame number, then by height */
	if(TEST_CYCLIC(psEffect))
	{
		/* Has it hit the ground */
		if((UDWORD)MAKEINT(psEffect->position.y) <= 
			map_Height((UDWORD)MAKEINT(psEffect->position.x),(UDWORD)MAKEINT(psEffect->position.z)))
		{
			KILL_EFFECT(psEffect);	
			return;
		}

		if(gameTime - psEffect->birthTime > psEffect->lifeSpan)
		{
			KILL_EFFECT(psEffect);	
			return;
		}
	}
}

// ----------------------------------------------------------------------------------------
/* Update fire sequences */
void	updateFire(EFFECT *psEffect)
{
iVector	pos;
LIGHT	light;
UDWORD	percent;

if (pie_Hardware())//pc only on hardware
{
	percent = PERCENT(gameTime-psEffect->birthTime,psEffect->lifeSpan);
	if(percent > 100)  
	{
		percent = 100;
	}
#ifdef DOLIGHTS
	light.position.x = MAKEINT(psEffect->position.x);
	light.position.y = MAKEINT(psEffect->position.y);
	light.position.z = MAKEINT(psEffect->position.z);
	light.range = (percent*psEffect->radius*3)/100;
	light.colour = LIGHT_RED;
	processLight(&light);
#endif
}
	

	/* Time to update the frame number on the construction sprite */
	if(gameTime - psEffect->lastFrame > psEffect->frameDelay)
	{
		psEffect->lastFrame = gameTime;
		pos.x = (MAKEINT(psEffect->position.x) + ((rand()%psEffect->radius) - (rand()%(2*psEffect->radius))));
		pos.z = (MAKEINT(psEffect->position.z) + ((rand()%psEffect->radius) - (rand()%(2*psEffect->radius))));
		pos.y = map_Height(pos.x,pos.z);
	 	
		if(psEffect->type == FIRE_TYPE_SMOKY_BLUE)
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_FLAMETHROWER,FALSE,NULL,0);
		}
		else
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}


//		pos.x = (MAKEINT(psEffect->position.x) + ((rand()%psEffect->radius) - (rand()%(2*psEffect->radius))));
//		pos.z = (MAKEINT(psEffect->position.z) + ((rand()%psEffect->radius) - (rand()%(2*psEffect->radius))));
//		pos.y = map_Height(pos.x,pos.z);
		if(psEffect->type == FIRE_TYPE_SMOKY OR psEffect->type == FIRE_TYPE_SMOKY_BLUE)
		{
			pos.x = (MAKEINT(psEffect->position.x) + ((rand()%psEffect->radius/2) - (rand()%(2*psEffect->radius/2))));
			pos.z = (MAKEINT(psEffect->position.z) + ((rand()%psEffect->radius/2) - (rand()%(2*psEffect->radius/2))));
			pos.y = map_Height(pos.x,pos.z);
			addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_DRIFTING_HIGH,FALSE,NULL,0);
		}
		else
		{
			pos.x = (MAKEINT(psEffect->position.x) + ((rand()%psEffect->radius) - (rand()%(2*psEffect->radius))));
			pos.z = (MAKEINT(psEffect->position.z) + ((rand()%psEffect->radius) - (rand()%(2*psEffect->radius))));
			pos.y = map_Height(pos.x,pos.z);
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}

		/*
		pos.x = MAKEINT(psEffect->position.x);
		pos.y = MAKEINT(psEffect->position.y);
		pos.z = MAKEINT(psEffect->position.z);

		scatter.x = psEffect->radius; scatter.y = 0; scatter.z = psEffect->radius;
		addMultiEffect(&pos,&scatter,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,2,0,0);
		*/

	}

	if(gameTime - psEffect->birthTime > psEffect->lifeSpan)
	{
		KILL_EFFECT(psEffect);	
		return;
	}
}

// ----------------------------------------------------------------------------------------
// ALL THE RENDER FUNCTIONS
// ----------------------------------------------------------------------------------------
/* 
renderEffect:-
Calls the appropriate render routine for each type of effect 
*/
void	renderEffect(EFFECT *psEffect)
{
		/* What type of effect are we dealing with? */
	switch(psEffect->group)
	{
	case EFFECT_WAYPOINT:
		renderWaypointEffect(psEffect);
		break;

	case EFFECT_EXPLOSION:
		renderExplosionEffect(psEffect);
		break;

	case EFFECT_CONSTRUCTION:
		renderConstructionEffect(psEffect);
		break;

	case EFFECT_SMOKE:
		renderSmokeEffect(psEffect);
		break;

	case EFFECT_GRAVITON:
		renderGravitonEffect(psEffect);
		break;

	case EFFECT_BLOOD:
		renderBloodEffect(psEffect);
		break;


	case EFFECT_STRUCTURE:
		break;

	case EFFECT_DESTRUCTION:
		/*	There is no display func for a destruction effect - 
			it merely spawn other effects over time */
		renderDestructionEffect(psEffect);
		break;
	case EFFECT_FIRE:
		/* Likewise */
		break;
	case EFFECT_SAT_LASER:
		/* Likewise */
		break;
	case EFFECT_FIREWORK:
		renderFirework(psEffect);
		break;
	default:
		DBERROR(("Weirdy class of effect passed to renderEffect"));
		break;
	}
}

// ----------------------------------------------------------------------------------------
/* drawing func for wapypoints . AJL. */
void	renderWaypointEffect(EFFECT *psEffect)
{
iVector		dv;
SDWORD		rx,rz;
UDWORD brightness, specular;
//SDWORD centreX, centreZ;

	dv.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	dv.y = (UDWORD)MAKEINT(psEffect->position.y);
	dv.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);
	iV_MatrixBegin();							/* Push the indentity matrix */
	iV_TRANSLATE(dv.x,dv.y,dv.z);
	rx = player.p.x & (TILE_UNITS-1);			/* Get the x,z translation components */
	rz = player.p.z & (TILE_UNITS-1);
	iV_TRANSLATE(rx,0,-rz);						/* Translate */

	// set up lighting
//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);

	pie_Draw3DShape(psEffect->imd, 0, 0, brightness, specular, 0, 0);
//	pie_Draw3DShape(psEffect->imd, 0, 0, pie_MAX_BRIGHT_LEVEL, 0, pie_NO_BILINEAR, 0);
	iV_MatrixEnd();
}

// ----------------------------------------------------------------------------------------
void	renderFirework(EFFECT *psEffect)
{
iVector		dv;
SDWORD		rx,rz;
UDWORD brightness, specular;
//SDWORD centreX, centreZ;

	/* these don't get rendered */
	if(psEffect->type == FIREWORK_TYPE_LAUNCHER)
	{
		return;
	}

	dv.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	dv.y = (UDWORD)MAKEINT(psEffect->position.y);
	dv.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);
	iV_MatrixBegin();							/* Push the indentity matrix */
	iV_TRANSLATE(dv.x,dv.y,dv.z);
	rx = player.p.x & (TILE_UNITS-1);			/* Get the x,z translation components */
	rz = player.p.z & (TILE_UNITS-1);
	iV_TRANSLATE(rx,0,-rz);						/* Translate */


	iV_MatrixRotateY(-player.r.y);
	iV_MatrixRotateX(-player.r.x);

   
  //	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
  //	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);

	/* Dither on software */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(TRUE);
	}


	scaleMatrix(psEffect->size);
 	pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, 0, pie_ADDITIVE, EFFECT_EXPLOSION_ADDITIVE);
 	iV_MatrixEnd();

	/* Dither on software OFF */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(FALSE);
	}

}
// ----------------------------------------------------------------------------------------
/* drawing func for blood. */
void	renderBloodEffect(EFFECT *psEffect)
{
iVector		dv;
SDWORD		rx,rz;
UDWORD brightness, specular;
//SDWORD centreX, centreZ;

	dv.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	dv.y = (UDWORD)MAKEINT(psEffect->position.y);
	dv.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);
	iV_MatrixBegin();							/* Push the indentity matrix */
	iV_TRANSLATE(dv.x,dv.y,dv.z);
	rx = player.p.x & (TILE_UNITS-1);			/* Get the x,z translation components */
	rz = player.p.z & (TILE_UNITS-1);
	iV_TRANSLATE(rx,0,-rz);						/* Translate */
	iV_MatrixRotateY(-player.r.y);
	iV_MatrixRotateX(-player.r.x);
	scaleMatrix(psEffect->size);

	// set up lighting
  //	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
  //	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);

	pie_Draw3DShape(getImdFromIndex(MI_BLOOD), psEffect->frameNumber, 0, brightness, specular, pie_TRANSLUCENT, EFFECT_BLOOD_TRANSPARENCY);
	iV_MatrixEnd();
}



// ----------------------------------------------------------------------------------------
void	renderDestructionEffect(EFFECT *psEffect)
{
iVector	dv;
SDWORD	rx,rz;
FRACT	div;
SDWORD	percent;
//SDWORD	centreX,centreZ;
UDWORD	brightness,specular;

	if(psEffect->type!=DESTRUCTION_TYPE_SKYSCRAPER)
	{
		return;
	}

   	dv.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	dv.y = (UDWORD)MAKEINT(psEffect->position.y);
	dv.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);
	iV_MatrixBegin();							/* Push the indentity matrix */
	iV_TRANSLATE(dv.x,dv.y,dv.z);
	rx = player.p.x & (TILE_UNITS-1);			/* Get the x,z translation components */
	rz = player.p.z & (TILE_UNITS-1);
	iV_TRANSLATE(rx,0,-rz);						/* Translate */

  
	div = MAKEFRACT(gameTime - psEffect->birthTime)/psEffect->lifeSpan;
	if(div>1.0)	div = 1.0;	//temporary!
	{
		div = 1.0 - div;
		percent = (SDWORD)(div*pie_RAISE_SCALE);
	}

	//get fog value
   //	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
  //	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,
		getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);
   
	if(!gamePaused())
	{
 		iV_MatrixRotateX(SKY_SHIMMY);
 		iV_MatrixRotateY(SKY_SHIMMY);
 		iV_MatrixRotateZ(SKY_SHIMMY);
	}
 	pie_Draw3DShape(psEffect->imd, 0, 0, brightness, 0,pie_RAISE, percent);
 

	iV_MatrixEnd();
}

// ----------------------------------------------------------------------------------------
BOOL	rejectLandLight(LAND_LIGHT_SPEC type)
{
UDWORD	timeSlice;

	timeSlice = gameTime%2000;
	if(timeSlice<400)
	{
		if(type == LL_MIDDLE) return(FALSE); else return(TRUE);	// reject all expect middle
	}
	else if(timeSlice<800)
	{
		if(type == LL_OUTER) return(TRUE); else return(FALSE);	// reject only outer
	}
	else if(timeSlice<1200)
	{
		return(FALSE);	//reject none
	}
	else if(timeSlice<1600)
	{
		if(type == LL_OUTER) return(TRUE); else return(FALSE);	// reject only outer
	}
	else
	{
		if(type == LL_MIDDLE) return(FALSE); else return(TRUE);	// reject all expect middle
	}
}
// ----------------------------------------------------------------------------------------
/* Renders the standard explosion effect */
void	renderExplosionEffect(EFFECT *psEffect)
{
#ifdef WIN32
	iVector		dv;
	SDWORD		rx,rz;
	SDWORD	percent;
	UDWORD brightness, specular;
//	SDWORD centreX, centreZ;
	UDWORD	timeSlice;

	if(psEffect->type == EXPLOSION_TYPE_LAND_LIGHT)
	{
		if(rejectLandLight(psEffect->specific))
		{
			return;
		}
	}
   
	dv.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	dv.y = (UDWORD)MAKEINT(psEffect->position.y);
	dv.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);
	iV_MatrixBegin();							/* Push the indentity matrix */
	iV_TRANSLATE(dv.x,dv.y,dv.z);
	rx = player.p.x & (TILE_UNITS-1);			/* Get the x,z translation components */
	rz = player.p.z & (TILE_UNITS-1);
	iV_TRANSLATE(rx,0,-rz);						/* Translate */

	/* Bit in comments - doesn't quite work yet? */
	if(TEST_FACING(psEffect))
	{
		/* Always face the viewer! */
/*		TEST_FLIPPED_Y(psEffect) ? iV_MatrixRotateY(-player.r.y+iV_DEG(180)) :*/ iV_MatrixRotateY(-player.r.y);
/*		TEST_FLIPPED_X(psEffect) ? iV_MatrixRotateX(-player.r.x+iV_DEG(180)) :*/ iV_MatrixRotateX(-player.r.x);
	}

	/* Tesla explosions diminish in size */
	if(psEffect->type == EXPLOSION_TYPE_TESLA)
	{
		percent = MAKEINT(PERCENT((gameTime - psEffect->birthTime),psEffect->lifeSpan));
		if(percent<0) percent = 0;
		if(percent>45) percent = 45;
		scaleMatrix(psEffect->size - percent);
	}
	else if(psEffect->type == EXPLOSION_TYPE_PLASMA)
	{
		percent = (MAKEINT(PERCENT((gameTime - psEffect->birthTime),psEffect->lifeSpan)))/3;
		scaleMatrix(BASE_PLASMA_SIZE + percent);
	}
	else
	{
		scaleMatrix(psEffect->size);
	}
	//get fog value
//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);

	/* Dither on software */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(TRUE);
	}


	if(psEffect->type == EXPLOSION_TYPE_PLASMA)
	{
		pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, 0, pie_ADDITIVE, EFFECT_PLASMA_ADDITIVE);
	}
	else if(psEffect->type == EXPLOSION_TYPE_KICKUP)
	{
		/* not transparent */
		pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, pie_TRANSLUCENT,128, 0, 0);
	}
	else
	{
		pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, 0, pie_ADDITIVE, EFFECT_EXPLOSION_ADDITIVE);
	}
	
	/* Dither on software OFF */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(FALSE);
	}


	iV_MatrixEnd();
#else
	PIE PieParams;
	int Size = psEffect->size;


	if(psEffect->type == EXPLOSION_TYPE_LAND_LIGHT)
	{
		if(rejectLandLight(psEffect->specific))
		{
			return;
		}
	}


	PieParams.Flags = PIE_COLOURED | PIE_TRANSPARENT;
	if(psEffect->type == EXPLOSION_TYPE_LASER) {
		PieParams.ColourRGB[0] = rand()&255;
		PieParams.ColourRGB[1] = rand()&255;
		PieParams.ColourRGB[2] = rand()&255;
	} else if(psEffect->type == EXPLOSION_TYPE_POWERMODULE) {
		PieParams.ColourRGB[0] = 32;
		PieParams.ColourRGB[1] = 32;
		PieParams.ColourRGB[2] = 128;
	} else if(psEffect->type == EXPLOSION_TYPE_RESEARCHMODULE) {
		PieParams.ColourRGB[0] = 32;
		PieParams.ColourRGB[1] = 128;
		PieParams.ColourRGB[2] = 32;
	} else if(psEffect->type == EXPLOSION_TYPE_LAND_LIGHT) {
		PieParams.ColourRGB[0] = 		//64; 
		PieParams.ColourRGB[1] = 		//0;
		PieParams.ColourRGB[2] = 64;	//0;
		Size /= 4;
	} else {
		PieParams.ColourRGB[0] = 
		PieParams.ColourRGB[1] = 
		PieParams.ColourRGB[2] = 64;
	}
	PieParams.TransMode = TRANSMODE_ADDITIVE;

	rendEffect(psEffect,Size,&PieParams);
#endif
}

// ----------------------------------------------------------------------------------------
void	renderGravitonEffect(EFFECT *psEffect)
{
#ifdef WIN32
iVector	vec;
SDWORD	rx,rz;
UDWORD  brightness, specular;
//SDWORD	centreX,centreZ;
  //	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
  //	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );

	/* Establish world position */
	vec.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	vec.y = (UDWORD)MAKEINT(psEffect->position.y);
	vec.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);

	/* Push matrix */
	iV_MatrixBegin();

	/* Move to position */
	iV_TRANSLATE(vec.x,vec.y,vec.z);

	/* Offset from camera */
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Move to camera reference */
	iV_TRANSLATE(rx,0,-rz);

	iV_MatrixRotateX(psEffect->rotation.x);
	iV_MatrixRotateY(psEffect->rotation.y);
	iV_MatrixRotateZ(psEffect->rotation.z);

	/* Buildings emitted by gravitons are chunkier */
	if(psEffect->type == GRAVITON_TYPE_EMITTING_ST)
	{
		/* Twice as big - 150 percent */
		scaleMatrix(psEffect->size);
	}
	else
	{
		scaleMatrix(100);
	}
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX()-MAKEINT(psEffect->position.x),
		getCentreZ()-MAKEINT(psEffect->position.z), &specular);

	pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, specular, 0, 0);

	/* Pop the matrix */
	iV_MatrixEnd();
#else
	PIE PieParams;

	PieParams.Flags=PIE_PALETTEID;
	PieParams.PaletteID = psEffect->frameNumber&3;

	rendEffect(psEffect,100,&PieParams);	//NULL);
#endif
}

// ----------------------------------------------------------------------------------------
/* 
renderConstructionEffect:-
Renders the standard construction effect */
void	renderConstructionEffect(EFFECT *psEffect)
{
iVector	vec,null;
SDWORD	rx,rz;
SDWORD	percent;
UDWORD	translucency;
UDWORD	size;
UDWORD brightness, specular;
//SDWORD centreX, centreZ;

	/* No rotation about arbitrary axis */
	null.x = null.y = null.z = 0;

	/* Establish world position */
	vec.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	vec.y = (UDWORD)MAKEINT(psEffect->position.y);
	vec.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);

	/* Push matrix */
	iV_MatrixBegin();

	/* Move to position */
	iV_TRANSLATE(vec.x,vec.y,vec.z);

	/* Offset from camera */
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Move to camera reference */
	iV_TRANSLATE(rx,0,-rz);

	/* Bit in comments doesn't quite work yet? */
	if(TEST_FACING(psEffect))
	{
/*		TEST_FLIPPED_Y(psEffect) ? iV_MatrixRotateY(-player.r.y+iV_DEG(180)) :*/ iV_MatrixRotateY(-player.r.y);
/*		TEST_FLIPPED_X(psEffect) ? iV_MatrixRotateX(-player.r.x+iV_DEG(180)) :*/ iV_MatrixRotateX(-player.r.x);
	}

	/* Scale size according to age */
	percent = MAKEINT(PERCENT((gameTime - psEffect->birthTime),psEffect->lifeSpan));
	if(percent<0) percent = 0;
	if(percent>100) percent = 100;

	/* Make imds be transparent on 3dfx */
	if(percent<50)
	{
		translucency = percent * 2;
	}
	else
	{
		translucency = (100 - percent) * 2;
	}
	translucency+=10;
	size = 2*translucency;
	if(size>90) size = 90;
	scaleMatrix(size);

	// set up lighting
//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);
  /* Dither on software */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(TRUE);
	}

	pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, specular, pie_TRANSLUCENT, (UBYTE)(translucency));
//	pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, pie_MAX_BRIGHT_LEVEL, 0, pie_TRANSLUCENT, (UBYTE)(40+percent));
//	pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, pie_MAX_BRIGHT_LEVEL, 0, pie_TRANSLUCENT, (UBYTE)(130-percent));

/* Dither on software */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(FALSE);
	}

	/* Pop the matrix */
	iV_MatrixEnd();
}

// ----------------------------------------------------------------------------------------
/*
renderSmokeEffect:-
Renders the standard smoke effect - it is now scaled in real-time as well 
*/
void	renderSmokeEffect(EFFECT *psEffect)
{
UDWORD	percent;
UDWORD	transparency;
iVector	vec;
SDWORD	rx,rz;
UDWORD brightness, specular;
//SDWORD centreX, centreZ;

	/* Establish world position */
	vec.x = ((UDWORD)MAKEINT(psEffect->position.x) - player.p.x) - terrainMidX * TILE_UNITS;
	vec.y = (UDWORD)MAKEINT(psEffect->position.y);
	vec.z = terrainMidY * TILE_UNITS - ((UDWORD)MAKEINT(psEffect->position.z) - player.p.z);

	/* Push matrix */
	iV_MatrixBegin();

	/* Move to position */
	iV_TRANSLATE(vec.x,vec.y,vec.z);

	/* Offset from camera */
	rx = player.p.x & (TILE_UNITS-1);
	rz = player.p.z & (TILE_UNITS-1);

	/* Move to camera reference */
	iV_TRANSLATE(rx,0,-rz);

	/* Bit in comments doesn't quite work yet? */
	if(TEST_FACING(psEffect))
	{
 		/* Always face the viewer! */
/*		TEST_FLIPPED_Y(psEffect) ? iV_MatrixRotateY(-player.r.y+iV_DEG(180)) : */iV_MatrixRotateY(-player.r.y);
/*		TEST_FLIPPED_X(psEffect) ? iV_MatrixRotateX(-player.r.x+iV_DEG(180)) : */iV_MatrixRotateX(-player.r.x);
	}



	/* Small smoke - used for the droids */
//		if(psEffect->type == SMOKE_TYPE_DRIFTING_SMALL OR psEffect->type == SMOKE_TYPE_TRAIL)

	if(TEST_SCALED(psEffect))
	{
		if (pie_Hardware())
		{
#ifdef HARDWARE_TEST//test additive
			percent = (MAKEINT(PERCENT((gameTime - psEffect->birthTime),psEffect->lifeSpan)));
			if(percent<10 AND psEffect->type == SMOKE_TYPE_TRAIL)
			{
				scaleMatrix((3 * percent/10 * psEffect->baseScale)/100);
				transparency = (EFFECT_SMOKE_ADDITIVE * (100 - 10))/100;
			}
			else
			{
				scaleMatrix((4 * percent * psEffect->baseScale)/100);
				transparency = (EFFECT_SMOKE_ADDITIVE * (100 - percent))/100;
			}
#else//Constant alpha
			percent = (MAKEINT(PERCENT((gameTime - psEffect->birthTime),psEffect->lifeSpan)));
			scaleMatrix(percent + psEffect->baseScale);
			transparency = (EFFECT_SMOKE_TRANSPARENCY * (100 - percent))/100;
#endif
		}
		else
		{//software
			percent = (MAKEINT(PERCENT((gameTime - psEffect->birthTime),psEffect->lifeSpan)))/2;
			scaleMatrix(percent + psEffect->baseScale);
			transparency = (EFFECT_SMOKE_TRANSPARENCY * (100 - percent))/100;
		}
	}

   	// set up lighting
//	centreX = ( player.p.x + ((visibleXTiles/2)<<TILE_SHIFT) );
//	centreZ = ( player.p.z + ((visibleYTiles/2)<<TILE_SHIFT) );
	brightness = lightDoFogAndIllumination(pie_MAX_BRIGHT_LEVEL,getCentreX() - MAKEINT(psEffect->position.x),getCentreZ() - MAKEINT(psEffect->position.z), &specular);

	/* Dither on software */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(TRUE);
	}
	else if(pie_GetRenderEngine() == ENGINE_D3D)//JPS smoke strength increased for d3d 12 may 99
	{
		transparency = (transparency*3)/2;
	}

	/* Make imds be transparent on 3dfx */
	if(psEffect->type==SMOKE_TYPE_STEAM)
	{
		pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, specular, pie_TRANSLUCENT, (UBYTE)(EFFECT_STEAM_TRANSPARENCY)/2);
	}
	else
	{
		if(psEffect->type == SMOKE_TYPE_TRAIL)
		{
			pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, specular, pie_TRANSLUCENT, (UBYTE)((2*transparency)/3));
		}
		else
		{
			pie_Draw3DShape(psEffect->imd, psEffect->frameNumber, 0, brightness, specular, pie_TRANSLUCENT, (UBYTE)(transparency)/2);
		}
	}
	/* Dither on software OFF */
	if(pie_GetRenderEngine() == ENGINE_4101)
	{
		pie_SetDitherStatus(FALSE);
	}

	/* Pop the matrix */
	iV_MatrixEnd();
}


// ----------------------------------------------------------------------------------------
// ALL THE SETUP FUNCTIONS
// ----------------------------------------------------------------------------------------
void	effectSetUpFirework(EFFECT *psEffect)
{
UDWORD	camExtra;
	if(psEffect->type == FIREWORK_TYPE_LAUNCHER)
	{
	 	psEffect->velocity.x = 200 - rand()%400;
		psEffect->velocity.z = 200 - rand()%400;	
		psEffect->velocity.y = 400 + rand()%200;	//height
		psEffect->lifeSpan = GAME_TICKS_PER_SEC * 3;
		psEffect->radius = 80 + rand()%150;
		camExtra = 0;
		if(getCampaignNumber()!=1)
		{
			camExtra+=rand()%200;
		}
		psEffect->size = 300+rand()%300;	//height it goes off
		psEffect->imd = getImdFromIndex(MI_FIREWORK); // not actually drawn
	}
	else
	{
		psEffect->velocity.x = 20 - rand()%40;
		psEffect->velocity.z = 20 - rand()%40;	
		psEffect->velocity.y = 0-(20+rand()%40);	//height
		psEffect->lifeSpan = GAME_TICKS_PER_SEC * 4;

		/* setup the imds */
		switch(rand()%3)
		{
		case 0:
			psEffect->imd = getImdFromIndex(MI_FIREWORK);
			psEffect->size = 45;	//size of graphic
			break;
		case 1:
			psEffect->imd = getImdFromIndex(MI_SNOW);
			SET_CYCLIC(psEffect);
			psEffect->size = 60;	//size of graphic

			break;
		default:
			psEffect->imd = getImdFromIndex(MI_FLAME);
			psEffect->size = 40;	//size of graphic

			
			break;
		}
	}

	psEffect->frameDelay = (EXPLOSION_FRAME_DELAY*2);
}
// ----------------------------------------------------------------------------------------
void	effectSetupSmoke(EFFECT *psEffect)
{

	/* everything except steam drifts about */
	if(psEffect->type==SMOKE_TYPE_STEAM)
	{
   		/* Only upwards */
		psEffect->velocity.x = MAKEFRACT(0);
   		psEffect->velocity.z = MAKEFRACT(0);
	}
	else if (psEffect->type == SMOKE_TYPE_BILLOW)
	{

		psEffect->velocity.x = MAKEFRACT((10-rand()%20));
   		psEffect->velocity.z = MAKEFRACT((10-rand()%20));
	}
	else
	{
   	  	psEffect->velocity.x = MAKEFRACT((rand()%20));
   		psEffect->velocity.z = MAKEFRACT((10-rand()%20));
	}

	/* Steam isn't cyclic  - it doesn't grow with time either */
	if(psEffect->type!=SMOKE_TYPE_STEAM)
	{
		SET_CYCLIC(psEffect);
		SET_SCALED(psEffect);
	}

	switch(psEffect->type)
  	{
  	case SMOKE_TYPE_DRIFTING:
  		psEffect->imd = getImdFromIndex(MI_SMALL_SMOKE);
		psEffect->lifeSpan = (UWORD)NORMAL_SMOKE_LIFESPAN;
   		psEffect->velocity.y = MAKEFRACT((35+rand()%30));
		psEffect->baseScale = 40;
  		break;
  	case SMOKE_TYPE_DRIFTING_HIGH:
  		psEffect->imd = getImdFromIndex(MI_SMALL_SMOKE);
		psEffect->lifeSpan = (UWORD)NORMAL_SMOKE_LIFESPAN;
   		psEffect->velocity.y = MAKEFRACT((40+rand()%45));
		psEffect->baseScale = 25;
  		break;
  	case SMOKE_TYPE_DRIFTING_SMALL:
  		psEffect->imd = getImdFromIndex(MI_SMALL_SMOKE);
		psEffect->lifeSpan = (UWORD)SMALL_SMOKE_LIFESPAN;
   		psEffect->velocity.y = MAKEFRACT((25+rand()%35));
		psEffect->baseScale = 17;
  		break;
  	case SMOKE_TYPE_BILLOW:
  		psEffect->imd = getImdFromIndex(MI_SMALL_SMOKE);
		psEffect->lifeSpan = (UWORD)SMALL_SMOKE_LIFESPAN;
   		psEffect->velocity.y = MAKEFRACT((10+rand()%20));
		psEffect->baseScale = 80;
  		break;
  	case SMOKE_TYPE_STEAM:
  		psEffect->imd = getImdFromIndex(MI_SMALL_STEAM);
   		psEffect->velocity.y = MAKEFRACT((rand()%5));
  		break;
	case SMOKE_TYPE_TRAIL:
		psEffect->imd = getImdFromIndex(MI_TRAIL);
		psEffect->lifeSpan = TRAIL_SMOKE_LIFESPAN;
   		psEffect->velocity.y = MAKEFRACT((5+rand()%10));
		psEffect->baseScale = 25;
		break;
	default:
		ASSERT((FALSE,"Weird smoke type"));
		break;
	}

	/* It always faces you */
	SET_FACING(psEffect);

	psEffect->frameDelay = (UWORD)SMOKE_FRAME_DELAY;
	/* Randomly flip gfx for variation */
	if(ONEINTWO)
	{
		SET_FLIPPED_X(psEffect);
	}
	if(ONEINTWO)
	{
		SET_FLIPPED_Y(psEffect);
	}
}

// ----------------------------------------------------------------------------------------
void effectSetUpSatLaser(EFFECT *psEffect)
{
	/* Does nothing at all..... Runs only for one frame! */
	psEffect->baseScale = 1;
	return;
}
// ----------------------------------------------------------------------------------------
void	effectSetupGraviton(EFFECT *psEffect)
{
	switch(psEffect->type)
	{
	case GRAVITON_TYPE_GIBLET:
	   	psEffect->velocity.x = GIBLET_INIT_VEL_X;
		psEffect->velocity.z = GIBLET_INIT_VEL_Z;
		psEffect->velocity.y = GIBLET_INIT_VEL_Y;
		break;
	case GRAVITON_TYPE_EMITTING_ST:
		psEffect->velocity.x = GRAVITON_INIT_VEL_X;
		psEffect->velocity.z = GRAVITON_INIT_VEL_Z;
		psEffect->velocity.y = (5*GRAVITON_INIT_VEL_Y)/4;
		psEffect->size =(UWORD)( 120 + rand()%30);
		break;
	case GRAVITON_TYPE_EMITTING_DR:
		psEffect->velocity.x = GRAVITON_INIT_VEL_X/2;
		psEffect->velocity.z = GRAVITON_INIT_VEL_Z/2;
		psEffect->velocity.y = GRAVITON_INIT_VEL_Y;
		break;
	default:
		ASSERT((FALSE,"Weirdy type of graviton"));
		break;

	}

	psEffect->rotation.x = DEG((rand()%360));
	psEffect->rotation.z = DEG((rand()%360));
	psEffect->rotation.y = DEG((rand()%360));

	psEffect->spin.x = DEG((rand()%100)+20);
	psEffect->spin.z = DEG((rand()%100)+20);
	psEffect->spin.y = DEG((rand()%100)+20);
 
	/* Gravitons are essential */
	SET_ESSENTIAL(psEffect);

	if(psEffect->type == GRAVITON_TYPE_GIBLET)
	{
		psEffect->frameDelay = (UWORD)GRAVITON_BLOOD_DELAY;
	}
	else
	{
		psEffect->frameDelay = (UWORD)GRAVITON_FRAME_DELAY;
	}
}

// ----------------------------------------------------------------------------------------
void effectSetupExplosion(EFFECT *psEffect)
{
	/* Get an imd if it's not established */
	if(psEffect->imd == NULL)
	{
		switch(psEffect->type)
		{
		case EXPLOSION_TYPE_SMALL:
  			psEffect->imd = getImdFromIndex(MI_EXPLOSION_SMALL);
  			psEffect->size = (UBYTE)((6*EXPLOSION_SIZE)/5);
  			break;
		case EXPLOSION_TYPE_VERY_SMALL:
  			psEffect->imd = getImdFromIndex(MI_EXPLOSION_SMALL);
  			psEffect->size = (UBYTE)(BASE_FLAME_SIZE + auxVar);
  			break;
		case EXPLOSION_TYPE_MEDIUM:
  			psEffect->imd = getImdFromIndex(MI_EXPLOSION_MEDIUM);
  			psEffect->size = (UBYTE)EXPLOSION_SIZE;
  			break;
		case EXPLOSION_TYPE_LARGE:
  			psEffect->imd = getImdFromIndex(MI_EXPLOSION_MEDIUM);
  			psEffect->size = (UBYTE)EXPLOSION_SIZE*2;
  			break;
		case EXPLOSION_TYPE_FLAMETHROWER:
			psEffect->imd = getImdFromIndex(MI_FLAME);
			psEffect->size = (UBYTE)(BASE_FLAME_SIZE + auxVar);
			break;
		case EXPLOSION_TYPE_LASER:
			psEffect->imd = getImdFromIndex(MI_FLAME);	// change this
			psEffect->size = (UBYTE)(BASE_LASER_SIZE + auxVar);
			break;
		case EXPLOSION_TYPE_DISCOVERY:
			psEffect->imd = getImdFromIndex(MI_TESLA);	// change this
			psEffect->size = DISCOVERY_SIZE;
			break;
		case EXPLOSION_TYPE_FLARE:
			psEffect->imd = getImdFromIndex(MI_MFLARE);
			psEffect->size = FLARE_SIZE;
			break;
		case EXPLOSION_TYPE_TESLA:
			psEffect->imd = getImdFromIndex(MI_TESLA);
			psEffect->size = TESLA_SIZE;
			psEffect->velocity.y = MAKEFRACT(TESLA_SPEED);
			break;
		case EXPLOSION_TYPE_KICKUP:
			psEffect->imd = getImdFromIndex(MI_KICK);
			psEffect->size = 100;
			break;
		case EXPLOSION_TYPE_PLASMA:
			psEffect->imd = getImdFromIndex(MI_PLASMA);
			psEffect->size = BASE_PLASMA_SIZE;
			psEffect->velocity.y = 0.0f;
			break;
		case EXPLOSION_TYPE_LAND_LIGHT:
			psEffect->imd = getImdFromIndex(MI_LANDING);
			psEffect->size = 120;
			psEffect->specific = ellSpec;
			psEffect->velocity.y = 0.0f;
			SET_ESSENTIAL(psEffect);		// Landing lights are permanent and cyclic
			break;
		case EXPLOSION_TYPE_SHOCKWAVE:
			psEffect->imd = getImdFromIndex(MI_SHOCK);//resGetData("IMD","blbhq.pie");
			psEffect->size = 50;
			psEffect->velocity.y = 0.0f;
			break;
		default:
  			break;
		}
	}

	if(psEffect->type == EXPLOSION_TYPE_FLAMETHROWER)
	{
		psEffect->frameDelay = 45;
	}
	/* Set how long it lasts */
	else if(psEffect->type == EXPLOSION_TYPE_LASER)
	{
		psEffect->frameDelay = (UWORD)(EXPLOSION_FRAME_DELAY/2);
	}
	else
	if(psEffect->type==EXPLOSION_TYPE_TESLA)
	{
		psEffect->frameDelay = EXPLOSION_TESLA_FRAME_DELAY;
	}
	else
	if(psEffect->type==EXPLOSION_TYPE_PLASMA)
	{
		psEffect->frameDelay = EXPLOSION_PLASMA_FRAME_DELAY;
	}
	else
		if(psEffect->type == EXPLOSION_TYPE_LAND_LIGHT)
		{
			psEffect->frameDelay = 120;
		}
	else
	{
		psEffect->frameDelay = (UWORD)EXPLOSION_FRAME_DELAY;
	}

	if(psEffect->type == EXPLOSION_TYPE_SHOCKWAVE)
	{
		psEffect->lifeSpan = GAME_TICKS_PER_SEC;
	}
	else
	{
		psEffect->lifeSpan = (psEffect->frameDelay *  psEffect->imd->numFrames);
	}

	if ( (psEffect->type!=EXPLOSION_TYPE_NOT_FACING) AND (psEffect->type!=EXPLOSION_TYPE_SHOCKWAVE))
	{
		SET_FACING(psEffect);
	}
	/* Randomly flip x and y for variation */
	if(ONEINTWO)
	{
		SET_FLIPPED_X(psEffect);
	}
	if(ONEINTWO)
	{
		SET_FLIPPED_Y(psEffect);
	}
}

// ----------------------------------------------------------------------------------------
void	effectSetupConstruction(EFFECT *psEffect)
{
	psEffect->velocity.x = MAKEFRACT(0);//(1-rand()%3);
	psEffect->velocity.z = MAKEFRACT(0);//(1-rand()%3);
	psEffect->velocity.y = MAKEFRACT((0-rand()%3));
	psEffect->frameDelay = (UWORD)CONSTRUCTION_FRAME_DELAY;
	psEffect->imd = getImdFromIndex(MI_CONSTRUCTION);
	psEffect->lifeSpan = CONSTRUCTION_LIFESPAN;

	/* These effects always face you */
	SET_FACING(psEffect);

	/* It's a cyclic anim - dies on age */
	SET_CYCLIC(psEffect);

	/* Randomly flip the construction graphics in x and y for variation */
	if(ONEINTWO)
	{
		SET_FLIPPED_X(psEffect);
	}
	if(ONEINTWO)
	{
		SET_FLIPPED_Y(psEffect);
	}
}

// ----------------------------------------------------------------------------------------
#if (0)
void	effectSetupDust(EFFECT *psEffect)
{
	psEffect->velocity.x = MAKEFRACT(0);//(1-rand()%3);
	psEffect->velocity.z = MAKEFRACT(0);//(1-rand()%3);
	psEffect->velocity.y = MAKEFRACT((0-rand()%3));
	psEffect->frameDelay = (UWORD)CONSTRUCTION_FRAME_DELAY;
	psEffect->imd = getImdFromIndex(MI_BLOOD);
	psEffect->lifeSpan = CONSTRUCTION_LIFESPAN;

	/* These effects always face you */
	SET_FACING(psEffect);

	/* It's a cyclic anim - dies on age */
	SET_CYCLIC(psEffect);

	/* Randomly flip the construction graphics in x and y for variation */
	if(ONEINTWO)
	{
		SET_FLIPPED_X(psEffect);
	}
	if(ONEINTWO)
	{
		SET_FLIPPED_Y(psEffect);
	}
}
#endif

void	effectSetupFire(EFFECT *psEffect)
{
	psEffect->frameDelay = 300;	   // needs to be investigated...
	psEffect->radius = auxVar;	// needs to be investigated
	psEffect->lifeSpan = (UWORD)auxVarSec;
	psEffect->birthTime = gameTime;
	SET_ESSENTIAL(psEffect);

}

// ----------------------------------------------------------------------------------------
void	effectSetupWayPoint(EFFECT *psEffect)
{
	psEffect->imd = pProximityMsgIMD;

	/* These effects musnt make way for others */
	SET_ESSENTIAL(psEffect);
}

// ----------------------------------------------------------------------------------------
void	effectSetupBlood(EFFECT *psEffect)
{
	psEffect->frameDelay = BLOOD_FRAME_DELAY;
	psEffect->velocity.y = MAKEFRACT(BLOOD_FALL_SPEED);
	psEffect->imd = getImdFromIndex(MI_BLOOD);
	psEffect->size = (UBYTE)BLOOD_SIZE;
}

// ----------------------------------------------------------------------------------------
void    effectSetupDestruction(EFFECT *psEffect)
{

	if(psEffect->type == DESTRUCTION_TYPE_SKYSCRAPER)
	{
		psEffect->lifeSpan = (3*GAME_TICKS_PER_SEC)/2 + (rand()%GAME_TICKS_PER_SEC);
		psEffect->frameDelay = DESTRUCTION_FRAME_DELAY/2;
	}
	else if(psEffect->type == DESTRUCTION_TYPE_DROID)
	{
		/* It's all over quickly for droids */
		psEffect->lifeSpan = DROID_DESTRUCTION_DURATION;
		psEffect->frameDelay = DESTRUCTION_FRAME_DELAY;
	}
	else if(psEffect->type == DESTRUCTION_TYPE_WALL_SECTION OR
			psEffect->type == DESTRUCTION_TYPE_FEATURE)
	{
		psEffect->lifeSpan = STRUCTURE_DESTRUCTION_DURATION/4;
		psEffect->frameDelay = DESTRUCTION_FRAME_DELAY/2;
	}
	else if(psEffect->type == DESTRUCTION_TYPE_POWER_STATION)
	{
		psEffect->lifeSpan = STRUCTURE_DESTRUCTION_DURATION/2;
		psEffect->frameDelay = DESTRUCTION_FRAME_DELAY/4;
	}
	else
	{
		/* building's destruction is longer */
		psEffect->lifeSpan = STRUCTURE_DESTRUCTION_DURATION;
		psEffect->frameDelay = DESTRUCTION_FRAME_DELAY/2;
	}
}
#define FX_PER_EDGE 6
#define	SMOKE_SHIFT	(16 - (rand()%32))
// ----------------------------------------------------------------------------------------
void	initPerimeterSmoke(iIMDShape *pImd, UDWORD x, UDWORD y, UDWORD z)
{
SDWORD	i;
SDWORD	inStart,inEnd;
SDWORD	varStart,varEnd,varStride;
SDWORD	shift;
iVector	base;
iVector	pos;

	base.x = x;
	base.y = y;
	base.z = z;

	varStart = pImd->xmin -16;
	varEnd = pImd->xmax + 16;
	varStride = 24;//(varEnd-varStart)/FX_PER_EDGE;

	inStart = pImd->zmin - 16;
	inEnd = pImd->zmax + 16;

	for(i=varStart; i<varEnd; i+=varStride)
	{
		shift = SMOKE_SHIFT;
		pos.x = base.x + i + shift;
		pos.y = base.y;
		pos.z = base.z + inStart + shift;
		if(rand()%6==1) 	
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}
		else
		{
			addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_BILLOW,FALSE,NULL,0);
		}
		
		pos.x = base.x + i + shift;
		pos.y = base.y;
		pos.z = base.z + inEnd + shift;
		if(rand()%6==1)  
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}
		else
		{
			addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_BILLOW,FALSE,NULL,0);
		}

	}

	varStart = pImd->zmin - 16;
	varEnd = pImd->zmax + 16;
	varStride = 24;//(varEnd-varStart)/FX_PER_EDGE;

	inStart = pImd->xmin - 16;
	inEnd = pImd->xmax + 16;

	for(i=varStart; i<varEnd; i+=varStride)
	{
		pos.x = base.x + inStart + shift;
		pos.y = base.y;
		pos.z = base.z + i + shift;
		if(rand()%6==1) 	
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}
		else
		{
	   		addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_BILLOW,FALSE,NULL,0);
		}
 
		
		pos.x = base.x + inEnd + shift;
		pos.y = base.y;
		pos.z = base.z + i + shift;
		if(rand()%6==1) 	
		{
			addEffect(&pos,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
		}
		else
		{
			addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_BILLOW,FALSE,NULL,0);
		}
	}
}
// ----------------------------------------------------------------------------------------
UDWORD	getNumEffects( void )
{
	return(numEffects);
}	



// ----------------------------------------------------------------------------------------
UDWORD EffectGetNumFrames(EFFECT *psEffect)
{
	return psEffect->imd->numFrames;
}

UDWORD IMDGetNumFrames(iIMDShape *Shape)
{
	return Shape->numFrames;
}

UDWORD IMDGetAnimInterval(iIMDShape *Shape)
{
	return Shape->animInterval;
}

void	effectGiveAuxVar( UDWORD var)
{
	auxVar = var;
}

void	effectGiveAuxVarSec( UDWORD var)
{
	auxVarSec = var;
}

// ----------------------------------------------------------------------------------------
/* Runs all the spot effect stuff for the droids - adding of dust and the like... */
void	effectDroidUpdates( void )
{
UDWORD	i;
DROID	*psDroid;
UDWORD	partition;
iVector	pos;
SDWORD	xBehind,yBehind;

	/* Go through all players */
	for(i=0; i<MAX_PLAYERS; i++)
	{
		/* Now go through all their droids */
		for(psDroid = apsDroidLists[i]; psDroid; psDroid = psDroid->psNext)
		{
			/* Gets it's group number */
			partition = psDroid->id % EFFECT_DROID_DIVISION;
			/* Right frame to process? */
			if(partition == frameGetFrameNumber() % EFFECT_DROID_DIVISION AND ONEINFOUR)		
			{
				/* Sufficent time since last update? - The EQUALS comparison is needed */
				if(gameTime >= (lastUpdateDroids[partition] + DROID_UPDATE_INTERVAL))
				{
					/* Store away when we last processed this group */
					lastUpdateDroids[partition] = gameTime;
					
					/*	Now add some dust at it's arse end if it's moving or skidding. 
						The check that it's not 0 is probably not sufficient.						
					*/
					if( (SDWORD)psDroid->sMove.speed != 0 )
					{
				   		/* Present direction is important */
						xBehind = ((50*iV_SIN(DEG(psDroid->direction))) >> FP12_SHIFT);
						yBehind = ((50*iV_COS(DEG(psDroid->direction))) >> FP12_SHIFT);
						pos.x = psDroid->x - xBehind;
						pos.z = psDroid->y - yBehind;
						pos.y = map_Height(pos.x,pos.z);
//						addEffect(&pos,EFFECT_SMOKE,SMOKE_TYPE_TRAIL,FALSE,NULL);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------------------
/* Runs all the structure effect stuff - steam puffing out etc */
void	effectStructureUpdates( void )
{
UDWORD		i;
UDWORD		partition;
STRUCTURE	*psStructure;
iVector		eventPos;
UDWORD		capacity;
POWER_GEN	*psPowerGen;
BOOL		active;

	
	/* Go thru' all players */
	for(i=0; i<MAX_PLAYERS; i++)
	{
		for(psStructure = apsStructLists[i]; psStructure; psStructure = psStructure->psNext)
		{
			/* Find it's group */
			partition = psStructure->id % EFFECT_STRUCTURE_DIVISION;
			/* Is it the right frame? */
			if(partition == frameGetFrameNumber() % EFFECT_STRUCTURE_DIVISION)		
			{
				/* Is it the right time? */
				if(gameTime > (lastUpdateStructures[partition] + STRUCTURE_UPDATE_INTERVAL))
				{
					/* Store away the last update time */
					lastUpdateStructures[partition] = gameTime;
					// -------------------------------------------------------------------------------
					/* Factories puff out smoke, power stations puff out tesla stuff */
				 	if( (psStructure->pStructureType->type == REF_FACTORY) OR
						(psStructure->pStructureType->type == REF_POWER_GEN) )
					if( (bMultiPlayer && isHumanPlayer(psStructure->player))
						|| (psStructure->player == 0) )
					if(psStructure->status==SS_BUILT)
					if(psStructure->visible[selectedPlayer])
					{
						/*	We're a factory, so better puff out a bit of steam 
							Complete hack with the magic numbers - just for IAN demo 
						*/
					if(psStructure->pStructureType->type == REF_FACTORY)
						{
							if (psStructure->sDisplay.imd->nconnectors == 1)
							{
								eventPos.x = psStructure->x+psStructure->sDisplay.imd->connectors->x;
								eventPos.z = psStructure->y-psStructure->sDisplay.imd->connectors->y;
								eventPos.y = psStructure->z+psStructure->sDisplay.imd->connectors->z;
								addEffect(&eventPos,EFFECT_SMOKE,SMOKE_TYPE_STEAM,FALSE,NULL,0);

								if(selectedPlayer == psStructure->player)
								{
									audio_PlayObjStaticTrack( (void *) psStructure, ID_SOUND_STEAM );
								}
							}
						}
						else if(psStructure->pStructureType->type == REF_POWER_GEN)
						{
							psPowerGen = (POWER_GEN*)psStructure->pFunctionality; 
							eventPos.x = psStructure->x;
							eventPos.z = psStructure->y;
							if (psStructure->sDisplay.imd->nconnectors > 0)
							{
								eventPos.y = psStructure->z+psStructure->sDisplay.imd->connectors->z;
							}
							else
							{
								eventPos.y = psStructure->z;
							}
							capacity = psPowerGen->capacity;
							/*if(capacity)
							{
								eventPos.y = psStructure->z + 48;
							}*/
							/* Add an effect over the central spire - if 
							connected to Res Extractor and it is active*/
							//look through the list to see if any connected Res Extr
							active = FALSE;
							for (i=0; i < NUM_POWER_MODULES; i++)
							{
								if (psPowerGen->apResExtractors[i] AND ((RES_EXTRACTOR *)
									psPowerGen->apResExtractors[i]->pFunctionality)->active)
								{
									active = TRUE;
									break;
								}
							}
							/*
							if (((POWER_GEN*)psStructure->pFunctionality)->
								apResExtractors[0] AND ((RES_EXTRACTOR *)((POWER_GEN*)
								psStructure->pFunctionality)->apResExtractors[0]->
								pFunctionality)->active)
							*/
							//if (active)
							{
								eventPos.y = psStructure->z + 48;
								addEffect(&eventPos,EFFECT_EXPLOSION,
										EXPLOSION_TYPE_TESLA,FALSE,NULL,0);
								if(selectedPlayer == psStructure->player)
								{
									audio_PlayObjStaticTrack( (void *) psStructure, ID_SOUND_POWER_SPARK );
								}
							}
							/*	Work out how many spires it has. This is a particularly unpleasant
								hack and I'm not proud of it, but it needs to done. Honest. AM
							*/
							//if(capacity)
						}
					}
				}
			}
		}
	}
}

UDWORD	getFreeEffect( void )
{
	return(freeEffect);
}


// ----------------------------------------------------------------------------------------
void	effectResetUpdates( void )
{
UDWORD	i;

	for(i=0; i<EFFECT_DROID_DIVISION; i++)
	{
		lastUpdateDroids[i] = 0;
	}
	for(i=0; i<EFFECT_STRUCTURE_DIVISION; i++)
	{
		lastUpdateStructures[i] = 0;
	}
}


// -----------------------------------------------------------------------------------
BOOL	fireOnLocation(UDWORD x, UDWORD y)
{
UDWORD	i;
UDWORD	posX,posY;
BOOL	bOnFire;

   	for(i=0, bOnFire = FALSE; i<MAX_EFFECTS AND !bOnFire; i++)
	{
	 	if( (asEffectsList[i].status == ES_ACTIVE) AND asEffectsList[i].group == EFFECT_FIRE)
		{
			posX = MAKEINT(asEffectsList[i].position.x);
			posY = MAKEINT(asEffectsList[i].position.z);
			if( (posX == x) AND (posY == y) )
			{
				bOnFire = TRUE;
			}
		}
	}
	return(bOnFire);
}
// -----------------------------------------------------------------------------------
/* This will save out the effects data */
BOOL	writeFXData( STRING *pFileName )
{
UBYTE			*pFileData;		// Pointer to the necessary allocated memory
EFFECT			*pFXData;
UDWORD			fileSize;		// How many bytes we need - depends on compression
FILE			*pFile;			// File pointer
FX_SAVEHEADER	*psHeader;		// Pointer to the header part of the file
UDWORD			fxEntries;		// Effectively, how many tiles are there?
UDWORD			i;
UDWORD			imdHashedNumber;
iIMDShape		*psOrig;

	/* How many FX do we write out data from? Only write active ones! */
	for(i=0,fxEntries = 0; i<MAX_EFFECTS; i++)
	{
		if(asEffectsList[i].status == ES_ACTIVE)
		{
			fxEntries++;
		}
	}

	/* Calculate memory required */
	fileSize = ( sizeof(struct _fx_save_header) + ( fxEntries*sizeof(struct _effect_def) ) );

	/* Try and allocate it - freed up in same function */
	pFileData = (UBYTE *)MALLOC(fileSize);

	/* Did we get it? */
	if(!pFileData)
	{
		/* Nope, so do one */	
		DBERROR(("Saving FX data : Cannot get the memory! (%d)",fileSize));
		return(FALSE);
	} 

	/* We got the memory, so put the file header on the file */
	psHeader = (FX_SAVEHEADER *)pFileData;
	psHeader->aFileType[0] = 'f';
	psHeader->aFileType[1] = 'x';
	psHeader->aFileType[2] = 'd';
	psHeader->aFileType[3] = 'a';
	psHeader->entries = fxEntries;

	/* Write out the version number - unlikely to change for FX data */
	psHeader->version = CURRENT_VERSION_NUM;

	/* Skip past the header to the raw data area */
	pFXData = (EFFECT*)(pFileData + sizeof(struct _fx_save_header));

	for(i=0; i<MAX_EFFECTS; i++)
	{
		if(asEffectsList[i].status == ES_ACTIVE)
		{
			/*
			restore = FALSE;
			// Can't save out the pointer, so hash it first 
			if(asEffectsList[i].imd)
			{
				restore = TRUE;
				psTemp = asEffectsList[i].imd;
				psOrig =asEffectsList[i].imd;
				asEffectsList[i].imd = (iIMDShape*)resGetHashfromData("IMD",psOrig,&imdHashedNumber);
			}
			memcpy(pFXData,&asEffectsList[i],sizeof(struct _effect_def));
			if(restore)
			{
				asEffectsList[i].imd = psTemp;
			}
			*/
			memcpy(pFXData,&asEffectsList[i],sizeof(struct _effect_def));
			/* Is there an imd? */
			if(asEffectsList[i].imd)
			{
				psOrig = asEffectsList[i].imd;
				resGetHashfromData("IMD",psOrig,&imdHashedNumber);
				pFXData->imd = (iIMDShape*)imdHashedNumber;
			}
			
			pFXData++;
		}
	}

	/* Have a bash at opening the file to write */
	pFile = fopen(pFileName, "wb");
	if (!pFile)
	{
		DBERROR(("Saving FX data : couldn't open file %s", pFileName));
		return(FALSE);
	}

	/* Now, try and write it out */
	if (fwrite(pFileData, 1, fileSize, pFile) != fileSize)
	{
		DBERROR(("Saving FX data : write failed for %s", pFileName));
		return(FALSE);
	}

	/* Finally, try and close it */
	if (fclose(pFile) != 0)
	{
		DBERROR(("Saving FX data : couldn't close %s", pFileName));
		return(FALSE);
	}

	/* And free up the memory we used */
	if (pFileData != NULL)
	{
		FREE(pFileData);
	}
	/* Everything is just fine! */
	return TRUE;
}

// -----------------------------------------------------------------------------------
/* This will read in the effects data */
BOOL	readFXData( UBYTE *pFileData, UDWORD fileSize )
{
UDWORD				expectedFileSize;
FX_SAVEHEADER		*psHeader;
UDWORD				i;
EFFECT				*pFXData;

	/* See if we've been given the right file type? */
	psHeader = (FX_SAVEHEADER *)pFileData;
	if (psHeader->aFileType[0] != 'f' || psHeader->aFileType[1] != 'x' ||
		psHeader->aFileType[2] != 'd' || psHeader->aFileType[3] != 'a')	{
		DBERROR(("Read FX data : Weird file type found? Has header letters \
				  - %s %s %s %s", psHeader->aFileType[0],psHeader->aFileType[1],
								  psHeader->aFileType[2],psHeader->aFileType[3]));
		return FALSE;
	}

	/* How much data are we expecting? */
	expectedFileSize = (sizeof(struct _fx_save_header) + (psHeader->entries*sizeof(struct _effect_def)) );

	/* Is that what we've been given? */
	if(fileSize!=expectedFileSize)
	{
		/* No, so bomb out */
		DBERROR(("Read FX data : Weird file size!"));
		return(FALSE);
	}
	
	/* Skip past the header gubbins - can check version number here too */	
	pFXData = (EFFECT*)(pFileData + sizeof(struct _fx_save_header));

	/* Clear out anything that's there already! */
	initEffectsSystem();	

	/* For every FX... */
	for(i=0; i<psHeader->entries; i++)
	{
		memcpy(&asEffectsList[i],pFXData++,sizeof(struct _effect_def));
		if(asEffectsList[i].imd)
		{
			/* Restore the pointer from the hashed ID */
			asEffectsList[i].imd = (iIMDShape*)resGetDataFromHash("IMD",(UDWORD)asEffectsList[i].imd);
		}
	}

	/* Ensure free effects kept up to date */
	freeEffect = i;

	/* Hopefully everything's just fine by now */
	return(TRUE);
}
// -----------------------------------------------------------------------------------
void	addFireworksEffect( void )
{
	
}
