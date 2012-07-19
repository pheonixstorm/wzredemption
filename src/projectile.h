/***************************************************************************/
/*
 * Projectile types and function headers
 *
 * Gareth Jones 11/7/97
 */
/***************************************************************************/

#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

/***************************************************************************/

#include "types.h"
#include "base.h"
#include "statsdef.h"
#include "movedef.h"
#include "anim.h"

/***************************************************************************/

// the last unit that did damage - used by script functions
extern	BASE_OBJECT		*g_pProjLastAttacker;
extern UDWORD	selectedPlayer;


// whether an object is in a fire
#define IN_FIRE		0x01
// whether an object has just left the fire, but is still burning
#define BURNING		0x02

// how long an object burns for after leaving a fire
#define BURN_TIME	10000
// how much damaga a second an object takes when it is burning
#define BURN_DAMAGE	15

BOOL	proj_InitSystem( void );
void	proj_UpdateAll( void );
BOOL	proj_Shutdown( void );

PROJ_OBJECT *	proj_GetFirst( void );
PROJ_OBJECT *	proj_GetNext( void );

void	proj_FreeAllProjectiles( void );
BOOL	proj_SendProjectile( WEAPON *psWeap, BASE_OBJECT *psAttacker, SDWORD player,
					 UDWORD tarX, UDWORD tarY, UDWORD tarZ, BASE_OBJECT *psTarget, BOOL bVisible );

// return whether a weapon is direct or indirect
BOOL	proj_Direct(WEAPON_STATS *psStats);

// return the maximum range for a weapon
SDWORD	proj_GetLongRange(WEAPON_STATS *psStats, SDWORD dz);


/*
// The fattest macro around - change this little bastard at your peril
#define GFX_VISIBLE(psObj)	(							\
	(													\
		(psObj->player == selectedPlayer)				\
	)													\
	OR													\
	(													\
		(psObj->psSource != NULL)						\
		AND												\
		(												\
			(psObj->psSource->type == OBJ_STRUCTURE && psObj->psSource->player == selectedPlayer) OR\
			(psObj->psSource->type == OBJ_STRUCTURE && psObj->psSource->visible[selectedPlayer]) OR\
			(psObj->psSource->type == OBJ_DROID     && psObj->psSource->visible[selectedPlayer]) OR\
			(psObj->psSource->type == OBJ_DROID     && psObj->psSource->player == selectedPlayer) \
		)												\
	)													\
	OR													\
	(													\
		(psObj->psDest != NULL)							\
		AND												\
		(												\
			(psObj->psDest->type == OBJ_STRUCTURE && psObj->psDest->player == selectedPlayer) OR\
			(psObj->psDest->type == OBJ_STRUCTURE && psObj->psSource->visible[selectedPlayer]) OR\
			(psObj->psDest->type == OBJ_DROID     && psObj->psDest->visible[selectedPlayer]) OR\
			(psObj->psDest->type == OBJ_DROID     && psObj->psSource->player == selectedPlayer) \
		)												\
	)													\
	OR													\
	(													\
		godMode											\
	)													\
)														
*/
extern UDWORD calcDamage(UDWORD baseDamage, WEAPON_EFFECT weaponEffect, 
						 BASE_OBJECT *psTarget);
extern BOOL gfxVisible(PROJ_OBJECT *psObj);	

/***************************************************************************/
extern BOOL	justBeenHitByEW		( BASE_OBJECT *psObj );
extern void	objectShimmy	( BASE_OBJECT *psObj );


#endif	/* _PROJECTILE_H_ */

/***************************************************************************/
