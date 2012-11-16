/*
 * oPrint.c
 *
 * Object information printing routines
 *
 */

#include "frame.h"
#include "objects.h"
#include "projectile.h"
#include "console.h"
#include "oprint.h"

#ifndef FINALBUILD
#define OPRINTF	CONPRINTF
//#define OPRINTF DBPRINTF

// print out information about a base object
void printBaseObjInfo(BASE_OBJECT *psObj)
{
	STRING	*pType;
	switch (psObj->type)
	{
	case OBJ_DROID:
		pType = "UNIT";
		break;
	case OBJ_STRUCTURE:
		pType = "STRUCT";
		break;
	case OBJ_FEATURE:
		pType = "FEAT";
		break;
	default:
		pType = "UNKNOWN TYPE";
		break;
	}

	OPRINTF(ConsoleString,(ConsoleString,"%s id %d at (%d,%d,%d) dpr (%d,%d,%d)\n",
		pType, psObj->id, psObj->x,psObj->y,psObj->z,
			   psObj->direction,psObj->pitch,psObj->roll));
}

// print out information about a general component
void printComponentInfo(COMP_BASE_STATS *psStats)
{
	psStats = psStats;

	OPRINTF(ConsoleString,(ConsoleString,"%s ref %d tl %d\n"
			  "   bPwr %d bPnts %d wt %d hp %d sp %d bdy %d imd %p\n",
			  getStatName(psStats), psStats->ref, psStats->techLevel,
			  psStats->buildPower, psStats->buildPoints, psStats->weight,
			  psStats->hitPoints, psStats->systemPoints, psStats->body,
			  psStats->pIMD));
}


// print out weapon information
void printWeaponInfo(WEAPON_STATS *psStats)
{
	STRING	*pWC, *pWSC, *pMM;

	switch (psStats->weaponClass)
	{
	case WC_KINETIC:		//bullets etc
		pWC = "WC_KINETIC";
		break;
	//case WC_EXPLOSIVE:	//rockets etc
	//	pWC = "WC_EXPLOSIVE";
	//	break;
	case WC_HEAT:		//laser etc
		pWC = "WC_HEAT";
		break;
	//case WC_MISC:		//others we haven't thought of!
	//	pWC = "WC_MISC";
	//	break;
	default:
		pWC = "UNKNOWN CLASS";
		break;
	}
	switch (psStats->weaponSubClass)
	{
	case WSC_MGUN:
		pWSC = "WSC_MGUN";
		break;
	case WSC_CANNON:
		pWSC = "WSC_CANNON";
		break;
	/*case WSC_ARTILLARY:
		pWSC = "WSC_ARTILLARY";
		break;*/
	case WSC_MORTARS:
		pWSC = "WSC_MORTARS";
		break;
	case WSC_MISSILE:
		pWSC = "WSC_MISSILE";
		break;
	case WSC_ROCKET:
		pWSC = "WSC_ROCKET";
		break;
	case WSC_ENERGY:
		pWSC = "WSC_ENERGY";
		break;
	case WSC_GAUSS:
		pWSC = "WSC_GAUSS";
		break;
	case WSC_FLAME:
		pWSC = "WSC_FLAME";
		break;
	/*case WSC_CLOSECOMBAT:
		pWSC = "WSC_CLOSECOMBAT";
		break;*/
	case WSC_HOWITZERS:
		pWSC = "WSC_HOWITZERS";
		break;
	case WSC_ELECTRONIC:
		pWSC = "WSC_ELECTRONIC";
		break;
	case WSC_AAGUN:
		pWSC = "WSC_AAGUN";
		break;
	case WSC_SLOWMISSILE:
		pWSC = "WSC_SLOWMISSILE";
		break;
	case WSC_SLOWROCKET:
		pWSC = "WSC_SLOWROCKET";
		break;
	case WSC_LAS_SAT:
		pWSC = "WSC_LAS_SAT";
		break;
	case WSC_BOMB:
		pWSC = "WSC_BOMB";
		break;
	case WSC_COMMAND:
		pWSC = "WSC_COMMAND";
		break;
	case WSC_EMP:
		pWSC = "WSC_EMP";
		break;
	default:
		pWSC = "UNKNOWN SUB CLASS";
		break;
	}
	switch (psStats->movementModel)
	{
	case MM_DIRECT:
		pMM = "MM_DIRECT";
		break;
	case MM_INDIRECT:
		pMM = "MM_INDIRECT";
		break;
	case MM_HOMINGDIRECT:
		pMM = "MM_HOMINGDIRECT";
		break;
	case MM_HOMINGINDIRECT:
		pMM = "MM_HOMINGINDIRECT";
		break;
	case MM_ERRATICDIRECT:
		pMM = "MM_ERRATICDIRECT";
		break;
	case MM_SWEEP:
		pMM = "MM_SWEEP";
		break;
	default:
		pMM = "UNKNOWN MOVE MODEL";
		break;
	}

	
	OPRINTF(ConsoleString,(ConsoleString,"Weapon: "));
	printComponentInfo((COMP_BASE_STATS *)psStats);
	OPRINTF(ConsoleString,(ConsoleString,"   sRng %d lRng %d mRng %d %s\n"
			  "   sHt %d lHt %d pause %d dam %d\n",
				psStats->shortRange, proj_GetLongRange(psStats,0), psStats->minRange,
				proj_Direct(psStats) ? "direct" : "indirect",
				//psStats->shortHit, psStats->longHit, psStats->firePause, psStats->damage));
				weaponShortHit(psStats,(UBYTE)selectedPlayer), weaponLongHit(psStats,
				(UBYTE)selectedPlayer), weaponFirePause(psStats,(UBYTE)selectedPlayer), 
				weaponDamage(psStats, (UBYTE)selectedPlayer)));
	OPRINTF(ConsoleString,(ConsoleString,"   rad %d radHt %d radDam %d\n"
			  "   inTime %d inDam %d inRad %d\n",
				psStats->radius, psStats->radiusHit, psStats->radiusDamage,
				psStats->incenTime, psStats->incenDamage, psStats->incenRadius));
	OPRINTF(ConsoleString,(ConsoleString,"   flSpd %d inHt %d %s\n",
				psStats->flightSpeed, psStats->indirectHeight,
				psStats->fireOnMove ? "fireOnMove" : "not fireOnMove"));
	OPRINTF(ConsoleString,(ConsoleString,"   %s %s %s\n", pWC, pWSC, pMM));
	/*OPRINTF(ConsoleString,(ConsoleString,"   %shoming %srotate recoil %d\n"
			  "   dLife %d radLife %d\n",
			  psStats->homingRound ? "" : "not ", psStats->rotate ? "" : "not ",
			  psStats->recoilValue, psStats->directLife, psStats->radiusLife));*/
	OPRINTF(ConsoleString,(ConsoleString,"   %srotate recoil %d\n"
			  "   dLife %d radLife %d\n",
			  psStats->rotate ? "" : "not ",
			  psStats->recoilValue, psStats->directLife, psStats->radiusLife));
}


// print out information about a droid and it's components
void printDroidInfo(DROID *psDroid)
{
	SDWORD	i;
	BODY_STATS			*psBdyStats;
	PROPULSION_STATS	*psPropStats;
	ECM_STATS			*psECMStats;
	SENSOR_STATS		*psSensStats;
	CONSTRUCT_STATS		*psConstStats;
	REPAIR_STATS		*psRepairStats;

	printBaseObjInfo((BASE_OBJECT *)psDroid);

	OPRINTF(ConsoleString,(ConsoleString,"   wt %d bSpeed %d sRng %d sPwr %d ECM %d bdy %d\n",
		psDroid->weight, psDroid->baseSpeed, psDroid->sensorRange, 
		psDroid->sensorPower,psDroid->ECMMod, psDroid->body));

	/*for(i=0; i<(SDWORD)psDroid->numWeaps; i++)
	{
		printWeaponInfo(asWeaponStats + psDroid->asWeaps[i].nStat);
	}*/
    if (psDroid->asWeaps[0].nStat > 0)
    {
        printWeaponInfo(asWeaponStats + psDroid->asWeaps[0].nStat);
    }

	for(i=0; i<DROID_MAXCOMP; i++)
	{
		switch (i)
		{
		case COMP_UNKNOWN:
			break;
		case COMP_BODY:
			if (psDroid->asBits[i].nStat > 0)
			{
				OPRINTF(ConsoleString,(ConsoleString,"Body: "));
				psBdyStats = asBodyStats + psDroid->asBits[i].nStat;
				printComponentInfo((COMP_BASE_STATS *)psBdyStats);
			}
			else
			{
				OPRINTF(ConsoleString,(ConsoleString,"ZNULL BODY\n"));
			}
			break;
		case COMP_BRAIN:
			break;
		case COMP_PROPULSION:
			if (psDroid->asBits[i].nStat > 0)
			{
				OPRINTF(ConsoleString,(ConsoleString,"Prop: "));
				psPropStats = asPropulsionStats + psDroid->asBits[i].nStat;
				printComponentInfo((COMP_BASE_STATS *)psPropStats);
			}
			else
			{
				OPRINTF(ConsoleString,(ConsoleString,"ZNULL PROPULSION\n"));
			}
			break;
		case COMP_ECM:
			if (psDroid->asBits[i].nStat > 0)
			{
				OPRINTF(ConsoleString,(ConsoleString,"ECM: "));
				psECMStats = asECMStats + psDroid->asBits[i].nStat;
				printComponentInfo((COMP_BASE_STATS *)psECMStats);
				OPRINTF(ConsoleString,(ConsoleString,"   pwr %d loc %d imd %p\n",
					//psECMStats->power, psECMStats->location, psECMStats->pMountGraphic));
					ecmPower(psECMStats, psDroid->player), psECMStats->location, 
					psECMStats->pMountGraphic));
			}
			else
			{
				OPRINTF(ConsoleString,(ConsoleString,"ZNULL ECM\n"));
			}
			break;
		case COMP_SENSOR:
			if (psDroid->asBits[i].nStat > 0)
			{
				OPRINTF(ConsoleString,(ConsoleString,"Sensor: "));
				psSensStats = asSensorStats + psDroid->asBits[i].nStat;
				printComponentInfo((COMP_BASE_STATS *)psSensStats);
				OPRINTF(ConsoleString,(ConsoleString,"   rng %d pwr %d loc %d imd %p\n",
					//psSensStats->range, psSensStats->power,
					sensorRange(psSensStats,psDroid->player), 
					sensorPower(psSensStats,psDroid->player),
					psSensStats->location, psSensStats->pMountGraphic));
			}
			else
			{
				OPRINTF(ConsoleString,(ConsoleString,"ZNULL SENSOR\n"));
			}
			break;
		case COMP_CONSTRUCT:
			if (psDroid->asBits[i].nStat > 0)
			{
				OPRINTF(ConsoleString,(ConsoleString,"Construct: "));
				psConstStats = asConstructStats + psDroid->asBits[i].nStat;
				printComponentInfo((COMP_BASE_STATS *)psConstStats);
				OPRINTF(ConsoleString,(ConsoleString,"   cPnts %d imd %p\n",
					//psConstStats->constructPoints, psConstStats->pMountGraphic));
					constructorPoints(psConstStats, psDroid->player), 
					psConstStats->pMountGraphic));
			}
			break;
		case COMP_REPAIRUNIT:
			if (psDroid->asBits[i].nStat > 0)
			{
				OPRINTF(ConsoleString,(ConsoleString,"Repair: "));
				psRepairStats = asRepairStats + psDroid->asBits[i].nStat;
				printComponentInfo((COMP_BASE_STATS *)psRepairStats);
				OPRINTF(ConsoleString,(ConsoleString,"   repPnts %d loc %d imd %p\n",
					//psRepairStats->repairPoints, psRepairStats->location, 
					repairPoints(psRepairStats, psDroid->player), 
					psRepairStats->location, 
					psRepairStats->pMountGraphic));
			}
			break;
		default:
			break;
		}
	}
}




#endif