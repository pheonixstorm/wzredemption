/*
 * Multibot.c
 *
 * Alex Lee , 97/98 Pumpkin Studios, Bath
 * Multiplay stuff relevant to droids only.
 */
#include "frame.h"

#include "droid.h"						// for droid sending and ordering.
#include "droiddef.h"
#include "base.h"						// for sending WHOLE droids.
#include "stats.h"
#include "move.h"						// for ordering droids
#include "objmem.h"
#include "power.h"						// for powercalculated
#include "order.h"
#include "geometry.h"					// for formations.
#include "map.h"
#include "group.h"
#include "formation.h"
#include "netplay.h"					// the netplay library.
#include "multiplay.h"					// warzone net stuff.
#include "multijoin.h"
#include "cmddroid.h"					// command droids
#include "action.h"
#include "console.h"
#include "mapgrid.h"

#define ANYPLAYER	99
#define UNKNOWN		99

DROIDSTORE *tempDroidList = NULL;

// ////////////////////////////////////////////////////////////////////////////
// External Stuff.
extern DROID_ORDER chooseOrderLoc(DROID *psDroid, UDWORD x,UDWORD y);
extern DROID_ORDER chooseOrderObj(DROID *psDroid, BASE_OBJECT *psObj);

// ////////////////////////////////////////////////////////////////////////////
// Local Prototypes

BOOL		sendVtolRearm(DROID *psDroid,STRUCTURE *psStruct, UBYTE chosen);
BOOL		recvVtolRearm(NETMSG *pMsg);

BOOL		sendDroidSecondary	(DROID *psDroid, SECONDARY_ORDER sec, SECONDARY_STATE state);
BOOL		recvDroidSecondary	(NETMSG *pMsg);

//BOOL		SendSingleDroidWaypoint(DROID *psDroid, UDWORD x,UDWORD y);
//BOOL		SendDroidWaypoint	(UBYTE player,UDWORD	x, UDWORD y);
//BOOL		recvDroidWaypoint	(NETMSG *pMsg);

BOOL		SendDroidMove		(DROID  *pDroid, UDWORD x, UDWORD y,BOOL bFormation);
BOOL		recvDroidMove		(NETMSG *pMsg);
BOOL		SendDroid			(DROID_TEMPLATE *pTemplate, UDWORD x, UDWORD y, UBYTE player, UDWORD id);
BOOL		recvDroid			(NETMSG *pMsg);
BOOL		SendDroidInfo		(DROID  *pDroid, DROID_ORDER order, UDWORD x, UDWORD y, BASE_OBJECT *psObj);
BOOL		recvDroidInfo		(NETMSG *pMsg);
BOOL		SendGroupOrderSelected	(UBYTE player, UDWORD x, UDWORD y, BASE_OBJECT *psObj);
BOOL		SendGroupOrderGroup(DROID_GROUP *psGroup, DROID_ORDER order,UDWORD x,UDWORD y,BASE_OBJECT *psObj);
BOOL		SendCmdGroup		(DROID_GROUP *group, UWORD x, UWORD y, BASE_OBJECT *psObj);
BOOL		recvGroupOrder		(NETMSG *pMsg);
static VOID ProcessDroidOrder	(DROID *psDroid, DROID_ORDER order, UDWORD x, UDWORD y, OBJECT_TYPE desttype,UDWORD destid);
BOOL		SendDestroyDroid	(DROID  *pD);
BOOL		recvDestroyDroid	(NETMSG *pMsg);
BOOL		sendWholeDroid		(DROID  *pD, DPID dest);
BOOL		receiveWholeDroid	(NETMSG *pMsg);
BOOL		sendRequestDroid	(UDWORD droidId);
BOOL		recvRequestDroid	(NETMSG *pMsg);


// ////////////////////////////////////////////////////////////////////////////
// Command Droids.

// sod em.


// ////////////////////////////////////////////////////////////////////////////
// vtol bits.
// happy vtol = vtol ready to go back to attack.
BOOL sendHappyVtol(DROID *psDroid)
{
	NETMSG m;

	if(!myResponsibility(psDroid->player))
	{
		return FALSE;
	}

	NetAdd(m,0,psDroid->player);
	NetAdd(m,1,psDroid->id);
	m.size =5;
	m.type =NET_VTOL;

	return NETbcast(&m,FALSE);
}

BOOL recvHappyVtol(NETMSG *pMsg)
{
	DROID	*pD;
	UBYTE	player;
	UDWORD	id;

	NetGet(pMsg,0,player);
	NetGet(pMsg,1,id);

	if(!IdToDroid(id,player,&pD))			//find droid.
	{
		return FALSE;
	}
		
	pD->sMove.iAttackRuns =0;						// finish it for next time round.
	pD->body = pD->originalBody;
	pD->asWeaps[0].ammo = asWeaponStats[pD->asWeaps[0].nStat].numRounds;
	pD->asWeaps[0].lastFired = 0;

	return TRUE;
}

// vtol now needs to return to rearm.
BOOL sendVtolRearm(DROID *psDroid,STRUCTURE *psStruct, UBYTE chosen)
{
	NETMSG msg;
	UDWORD	blank=0;

	if(!myResponsibility(psDroid->player))
	{
		return FALSE;
	}

	NetAdd(msg,0,(UBYTE)(psDroid->player));
	NetAdd(msg,1,psDroid->id);
	NetAdd(msg,5,chosen);
	if(psStruct)
	{
		NetAdd(msg,6,psStruct->id);
	}
	else
	{
		NetAdd(msg,6,blank);
	}

	NetAdd(msg,10,((UBYTE)(psDroid->sMove.iAttackRuns)) );
	NetAdd(msg,11,((UBYTE)(psDroid->asWeaps[0].ammo)) );

	msg.size = 12;
	msg.type = NET_VTOLREARM;
	return TRUE;
}

// this is a complete rip from recvvtolrearm
BOOL recvVtolRearm(NETMSG *pMsg)
{	
	DROID	*psDroid;
	UBYTE	player,chosen,aruns,amm;
	UDWORD	id,ids;
	STRUCTURE *psStruct;

	NetGet(pMsg,0,player);
	NetGet(pMsg,1,id);
	NetGet(pMsg,5,chosen);
	NetGet(pMsg,6,ids);

	NetGet(pMsg,10,aruns);
	NetGet(pMsg,11,amm);
	
	if(!IdToDroid(id,player,&psDroid))			//find droid.
	{
		return FALSE;
	}
	
	if(ids)										// find rearm pad.
	{
		psStruct = IdToStruct(id,psDroid->player);
		if(!psStruct)
		{
			return FALSE;
		}
		else
		{	
			psDroid->psBaseStruct = psStruct;
		}
	}	

	psDroid->sMove.iAttackRuns = aruns;
	psDroid->asWeaps[0].ammo = amm;

	turnOffMultiMsg(TRUE);
	switch(chosen)
	{
	case 1:
		psDroid->order = DORDER_NONE;
		orderDroidObj(psDroid, DORDER_REARM, (BASE_OBJECT *)psStruct);
		break;
	case 2:
		actionDroidObj(psDroid,DACTION_MOVETOREARM, (BASE_OBJECT *)psStruct);
		break;
	case 3:		
		orderDroid( psDroid, DORDER_RTB );
		break;
	default:
		break;
	}
	turnOffMultiMsg(FALSE);

	return TRUE;
}


// ////////////////////////////////////////////////////////////////////////////
// Secondary Orders.

// send
BOOL sendDroidSecondary(DROID *psDroid, SECONDARY_ORDER sec, SECONDARY_STATE state)
{
	NETMSG	m;

	NetAdd(m,0,psDroid->id);
	NetAdd(m,4,sec);
	NetAdd(m,8,state);
	m.body[12] = (char) psDroid->player;

	m.size = 13;
	m.type = NET_SECONDARY;
	return NETbcast(&m,FALSE);
}

// recv
BOOL recvDroidSecondary(NETMSG *pMsg)
{
	DROID			*psDroid;
	SECONDARY_ORDER sec;
	SECONDARY_STATE	state;
	UDWORD			id,player;

	NetGet(pMsg,0,id);
	NetGet(pMsg,4,sec);
	NetGet(pMsg,8,state);
	player = pMsg->body[12];

	if(!IdToDroid(id,player,&psDroid))		//find droid.
	{
		return FALSE;
	}
	turnOffMultiMsg(TRUE);
	secondarySetState(psDroid,sec,state);	// set order
	turnOffMultiMsg(FALSE);

	return TRUE;
}


BOOL sendDroidSecondaryAll(DROID *psDroid)
{
	NETMSG	m;

	NetAdd(m,0,psDroid->id);
	NetAdd(m,4,psDroid->secondaryOrder);
	m.body[8] = (char) psDroid->player;

	m.size = 9;
	m.type = NET_SECONDARY_ALL;
	return NETbcast(&m,FALSE);

}

BOOL recvDroidSecondaryAll(NETMSG *pMsg)
{
    DROID			*psDroid;
	UDWORD			id,player,sorder;

	NetGet(pMsg,0,id);
	NetGet(pMsg,4,sorder);
	player = pMsg->body[8];

	if(!IdToDroid(id,player,&psDroid))		//find droid.
	{
		return FALSE;
	}
        
    if(psDroid)
    {
        psDroid->secondaryOrder = sorder;
    }

    return TRUE;
}

BOOL sendDroidEmbark(DROID *psDroid)
{
	NETMSG	m;

	NetAdd(m,0,psDroid->id);
	m.body[4] = (char) psDroid->player;

	m.size = 5;
	m.type = NET_DROIDEMBARK;

	return NETbcast(&m,FALSE);

}

BOOL recvDroidEmbark(NETMSG *pMsg)
{
    DROID			*psDroid;
	UDWORD			id,player;

	NetGet(pMsg,0,id);
	player = pMsg->body[4];

	if(!IdToDroid(id,player,&psDroid))		//find droid.
	{
		return FALSE;
	}
        
    if(psDroid)
    {
        //take it out of the world without destroying it
        droidRemove(psDroid, apsDroidLists);
        //init the order for when disembark
        psDroid->order = DORDER_NONE;
    	psDroid->psTarget = NULL;
        psDroid->psTarStats = NULL;
    }

    return TRUE;
}

BOOL sendDroidDisEmbark(DROID *psDroid)
{
	NETMSG	m;

	NetAdd(m,0,psDroid->id);
    NetAdd(m,4,psDroid->x);
    NetAdd(m,6,psDroid->y);
	m.body[8] = (char) psDroid->player;

	m.size = 9;
	m.type = NET_DROIDDISEMBARK;

	return NETbcast(&m,FALSE);

}

BOOL recvDroidDisEmbark(NETMSG *pMsg)
{
    DROID			*psDroid;
	UDWORD			id,player;
    UWORD           x, y;

	NetGet(pMsg,0,id);
    NetGet(pMsg,4,x);
    NetGet(pMsg,6,y);
	player = pMsg->body[8];

	if(!IdToDroid(id,player,&psDroid))		//find droid.
	{
		return FALSE;
	}
        
    if(psDroid)
    {
        //add it back into the world at the x/y
        psDroid->x = x;
        psDroid->y = y;
	    updateDroidOrientation(psDroid);
		//initialise the movement data
    	initDroidMovement(psDroid);
	    //reset droid orders
		orderDroid(psDroid, DORDER_STOP);
		gridAddObject((BASE_OBJECT *)psDroid);
		psDroid->cluster = 0;
        addDroid(psDroid, apsDroidLists);
    }

    return TRUE;
}



// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// Waypoints

// send

/*
// occasionally need to send non seleceted waypoints.
BOOL SendSingleDroidWaypoint(DROID *psDroid, UDWORD x,UDWORD y)
{
	NETMSG m;
	UDWORD	count =1;
	NetAdd(m,0,(psDroid->player));			// player
	NetAdd(m,1,x);							// waypoint x
	NetAdd(m,5,y);							// waypoint y
	NetAdd(m,9,count);
	NetAdd(m,13,psDroid->id);
	m.type = NET_WAYPOINT;
	m.size =17;
	return NETbcast(m,FALSE);
}

BOOL SendDroidWaypoint	(UBYTE player,UDWORD	x, UDWORD y)
{
	NETMSG	m;
	UDWORD	count=0;
	DROID	*psCurr;
	
	NetAdd(m,0,player);				// player
	NetAdd(m,1,x);					// waypoint x
	NetAdd(m,5,y);					// waypoint y
	m.size = 13;
	m.type = NET_WAYPOINT;

	// find each droid under instruction
	for(psCurr = apsDroidLists[player]; psCurr; psCurr=psCurr->psNext)
	{
		if (psCurr->selected)
		{
			NetAdd(m,m.size,psCurr->id);
			count++;
			m.size += 4;
		}
	}
	NetAdd(m,9,count);

	if(count>0)
	{
		return NETbcast(m,FALSE);
	}
	else
	{
		return TRUE;
	}
}

// recv
BOOL recvDroidWaypoint(NETMSG *pMsg)
{
	UBYTE	play;
	UDWORD	x,y,count,i,id;
	DROID	*psDroid;
	FORMATION	*psFormation=NULL;
	DROID		*psPrev=NULL;
		
	NetGet(pMsg,0,play);
	NetGet(pMsg,1,x);
	NetGet(pMsg,5,y);
	NetGet(pMsg,9,count);

	for(i=0;i<count;i++)
	{
		NetGet(pMsg,13+(i*4),id);			// get droid id.
		if(!IdToDroid(id,play,&psDroid))	//find droid.
		{
			sendRequestDroid(id);			// request it.
			return FALSE;
		}
		
		turnOffMultiMsg(TRUE);
		orderAddWayPoint(psDroid,x,y);		// add waypoint to that droid.
		turnOffMultiMsg(FALSE);
	}
	return TRUE;
}

*/

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// Droids

// posibly Send an updated droid movement order.
BOOL SendDroidMove(DROID *pDroid, UDWORD x, UDWORD y,BOOL bFormation)
{		
	NETMSG m;			
	BOOL sendit = TRUE;								// wether or not to send
	BOOL allow = TRUE;								// wether or not to allow the move to proceed.
	
	switch (pDroid->action)							// look at the action. Some actions shouldn't be sent. 
	{	
		case DACTION_NONE:
			sendit = FALSE;							// don't send if no action is taking place
			break;
		default:
			sendit = TRUE;
			break;
	}

	switch (pDroid->order)							// next look at the order. 
	{
		case DORDER_MOVE:
			sendit = FALSE;							// dont bother sending if the order is just a move
			break;
		default:
			break;
	}

	if (!myResponsibility(pDroid->player))			// we may not be responsible, find out.
	{
		allow = FALSE;
		sendit = FALSE;
	}

	if(sendit)										// send the mesg if required.
	{
		NetAdd(m,0,pDroid->id);						//droid to move
		NetAdd(m,4,x);								//x pos
		NetAdd(m,8,y);								//y pos
		NetAdd(m,12,(char)pDroid->player);			//owner of droid(for speed!)
		NetAdd(m,13,(char)bFormation);				//use a formation?
		m.size = 14;
		m.type = NET_DROIDMOVE;
		NETbcast(&m,FALSE);
	}
	return allow;

}

// recv and updated droid position
BOOL recvDroidMove(NETMSG *m)
{
	UDWORD player,id,x,y;
	DROID *psDroid;
	UBYTE	bFormation;

	NetGet(m,0,id);
	NetGet(m,4,x);
	NetGet(m,8,y);
	player = m->body[12];
	NetGet(m,13,bFormation);

	if(!(IdToDroid(id,player,&psDroid)))			// find the droid
	{
		sendRequestDroid(id);						// if we can't find it, ask for it!
		return TRUE;
	}

	turnOffMultiMsg(TRUE);
	if(bFormation)
	{
		moveDroidTo(psDroid, x, y);						// do the move
	}
	else
	{
		moveDroidToNoFormation(psDroid,x,y);			// move, no form...
	}
	turnOffMultiMsg(FALSE);

	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
// Send a new Droid to the other players
BOOL SendDroid(DROID_TEMPLATE *pTemplate, UDWORD x, UDWORD y, UBYTE player,UDWORD id)
{
	NETMSG m;

//	if( ingame.localJoiningInProgress &&  (game.type != DMATCH) )
	if( ingame.localJoiningInProgress )
	{
		return TRUE;								// dont send other droids during campaign setup.
	}

	if( !myResponsibility(player) )					// only send the droid if we are responsible
	{
		return FALSE;								// dont' build if we are not responsible.
	}


	NetAdd(m,0,player);						//ok since <255  players!
	NetAdd(m,1,x);									//x pos of new droid
	NetAdd(m,5,y);									//y pos of new droid
	NetAdd(m,9,id);									//id of droid to create

	if(powerCalculated)
	{
		m.body[13] = 1;
	}
	else
	{
		m.body[13] = 0;
	}
	
	// new version
	NetAdd(m,14,pTemplate->multiPlayerID);
	m.size = 14+sizeof(pTemplate->multiPlayerID);

	m.type=NET_DROID;
	NETbcast(&m,FALSE);						// send it.
	return TRUE;

}

// ////////////////////////////////////////////////////////////////////////////
// receive droid creation information from other players
BOOL recvDroid(NETMSG * m)
{
	DROID_TEMPLATE *pT;
	UDWORD			x,y,id;
	UDWORD			player;
	UDWORD			targetRef;
	DROID			*d;

	player=m->body[0];
	NetGet(m,1,x);									// new droids x position
	NetGet(m,5,y);									// new droids y position
	NetGet(m,9,id);									// droid to build's id.
	
	NetGet(m,14,targetRef);

	pT = IdToTemplate(targetRef,player);

	if(!pT)
	{
		NETlogEntry("Couldn't find template to build recvd droid. val = player",0,player );
		DBPRINTF(("Couldn't find template to build recvd droid" ));
		sendRequestDroid(id);						// request the droid instead.
		return FALSE;
	}
	

	if(m->body[13] != 0)
	{
		if (!usePower(player,pT->powerPoints))// take the power.
		{
//			DBCONPRINTF(ConsoleString,(ConsoleString,"MULTIPLAYER: not enough power to build remote droid."));
			NETlogEntry("not enough power to build recvd droid, val=player",0,player);
// build anyway..
//			return FALSE;
		}
	}

	turnOffMultiMsg(TRUE);
	d = buildDroid(pT,x,y,player, FALSE);			// create that droid on this machine.
	turnOffMultiMsg(FALSE);

	if(d)
	{	
		d->id = id;
		addDroid(d, apsDroidLists);					// put droid in world
	}
	else
	{
		DBCONPRINTF(ConsoleString,(ConsoleString,"MULTIPLAYER: Couldn't build a remote droid, relying on checking to resync"));
		return FALSE;
	}

	return (TRUE);
}

// ////////////////////////////////////////////////////////////////////////////
// Droid Group/selection orders. 
// minimises comms by sending orders for whole groups, rather than each droid

BOOL SendCmdGroup(DROID_GROUP *psGroup, UWORD x, UWORD y, BASE_OBJECT *psObj)
{
	//NETMSG	m;
	//DROID	*pDroid;
	//USHORT	droidcount=0;

    UNUSEDPARAMETER(psObj);
    UNUSEDPARAMETER(x);
    UNUSEDPARAMETER(y);
    UNUSEDPARAMETER(psGroup);

    return FALSE;	//doesnt fukin work. return FALSE and use other msgs to cope with this (about 2 packet overhead)
/*
	if (psObj == NULL)							//it's a position order
	{	
		NetAdd(m,0,x);
		NetAdd(m,4,y);
		m.body[8] = 0;							// subtype flag
	}
	else										// it's a object order
	{
		NetAdd(m,0, psObj->id);
		NetAdd(m,4, psObj->type);
		m.body[8]=1;							// subtype flag
	}
	m.body[10]=1;		//  a cmd order.
	m.size=12;
	
	for (pDroid = psGroup->psList; pDroid; pDroid=pDroid->psGrpNext)
	{

		if (!orderState(pDroid, DORDER_RTR))	// CHANGE THIS BIT WHEN REQD!!!
		{
			NetAdd(m,m.size,pDroid->id);
			m.size += sizeof(UDWORD);
			droidcount ++;
		}
	}
	
	if( droidcount > 0	) 							// return TRUE if it's worth using. 
	{
		NetAdd(m,9,droidcount);						// note how many in this message.
		m.type = NET_GROUPORDER;					// send it
		NETbcast(&m,FALSE);
		return TRUE;
	}
	else
	{	
		return FALSE;								// didn't bother using it, so return false, to allow individiual orders.
	}
*/
}


BOOL SendGroupOrderSelected(UBYTE player, UDWORD x, UDWORD y, BASE_OBJECT *psObj)
{
	NETMSG	m;
	DROID	*pDroid;
	USHORT	droidcount=0;

	if (psObj == NULL)							//it's a position order
	{	
		NetAdd(m,0,x);
		NetAdd(m,4,y);
		m.body[8] = 0;							// subtype flag
	}
	else										// it's a object order
	{
		NetAdd(m,0, psObj->id);
		NetAdd(m,4, psObj->type);
		m.body[8]=1;							// subtype flag
	}
	m.body[10]=0;		// not a cmd order.
	
	m.body[12] = UNKNOWN;	// set the order.

	m.size=13;
	
	//now add the list of droid id's to order

	for(pDroid = apsDroidLists[player]; pDroid; pDroid=pDroid->psNext)
	{
		if (pDroid->selected)
		{
			NetAdd(m,m.size,pDroid->id);
			m.size += sizeof(UDWORD);
			droidcount ++;
		}
	}
	
	if( droidcount >1	) 							// return TRUE if it's worth using. 
	{
		NetAdd(m,9,droidcount);						// note how many in this message.
		m.type = NET_GROUPORDER;					// send it
		NETbcast(&m,FALSE);
		return TRUE;
	}
	else
	{	// didn't bother using it, so return false, to allow individiual orders.
		return FALSE;
	}
}

BOOL SendGroupOrderGroup(DROID_GROUP *psGroup, DROID_ORDER order,UDWORD x,UDWORD y,BASE_OBJECT *psObj) 
{
	NETMSG	m;
	DROID	*pDroid;
	USHORT	droidcount=0;

	if (psObj == NULL)							//it's a position order
	{	
		NetAdd(m,0,x);
		NetAdd(m,4,y);
		m.body[8] = 0;							// subtype flag
	}
	else										// it's a object order
	{
		NetAdd(m,0, psObj->id);
		NetAdd(m,4, psObj->type);
		m.body[8]=1;							// subtype flag
	}


	// check this!!!!!
	m.body[10]=0;		// not a cmd order.

	m.body[12] = order;	// set the order.

	m.size=13;
	
	//now add the list of droid id's to order

	for(pDroid = psGroup->psList; pDroid; pDroid=pDroid->psGrpNext)
	{
		NetAdd(m,m.size,pDroid->id);
		m.size += sizeof(UDWORD);
		droidcount ++;
	}
	
	NetAdd(m,9,droidcount);						// note how many in this message.
	m.type = NET_GROUPORDER;					// send it
	NETbcast(&m,FALSE);
	return TRUE;

}

// ////////////////////////////////////////////////////////////////////////////
// receive a group order.
BOOL recvGroupOrder(NETMSG *pMsg)
{
	UDWORD		x,y,id,destid;
	DROID		*psDroid=NULL;
	OBJECT_TYPE	desttype;
	USHORT		droidcount;

//	DROID		*psPrev = NULL;	// fomation vars.
//	FORMATION	*psFormation = NULL;
	BOOL		bCmdOr = FALSE;
	DROID_ORDER	order;

	bCmdOr = pMsg->body[10];
	
	order = pMsg->body[12];

	NetGet(pMsg,9,droidcount);
	if(pMsg->body[8] == 1)											// it's target is an object
	{
		NetGet(pMsg,0,destid);
		NetGet(pMsg,4,desttype);
		x=0;
		y=0;
	}
	else															// it's target is a position
	{
		NetGet(pMsg,0,x);											// x coord
		NetGet(pMsg,4,y);											// y coord
		destid=0;
		desttype=0;
	}

	// for each droid
	for(droidcount;droidcount>0;droidcount--)
	{
		NetGet(pMsg,13+((droidcount-1) * sizeof(UDWORD)) ,id);
		IdToDroid(id, ANYPLAYER, &psDroid);							// find the droid
		if (psDroid==NULL)
		{
			sendRequestDroid(id);									//droid not found, request it.
			return (FALSE);	
		}

		if(!bCmdOr)
		{
			if(	(psDroid->droidType != DROID_COMMAND) &&
				(psDroid->psGroup != NULL) &&
				(psDroid->psGroup->type == GT_COMMAND)
			  )
			{			
				grpLeave(psDroid->psGroup, psDroid);
			}	
		}

		ProcessDroidOrder(psDroid,order,x,y,desttype,destid);		// process the order.

		// no need to do formation stuff anymore.

	}
	return TRUE;
}


// ////////////////////////////////////////////////////////////////////////////
// Droid update information
BOOL SendDroidInfo(DROID *psDroid, DROID_ORDER order, UDWORD x, UDWORD y, BASE_OBJECT *psObj)
{
	NETMSG m;
	
	if(!myResponsibility(psDroid->player))
	{
		return TRUE;
	}

	NetAdd(m,0,psDroid->id);
	NetAdd(m,4,order);

	if((x==0) &&(y==0) && (psObj==NULL))
	{
		NetAdd(m,8,x);	//note: x,y not needed
		NetAdd(m,12,y);
		m.body[16] = 2;	
	}
	else if (psObj == NULL)											//it's a position order
	{	
		NetAdd(m,8,x);
		NetAdd(m,12,y);
		m.body[16] = 0;												// subtype flag
	}
	else															// it's a object order
	{
		NetAdd(m,8, psObj->id);
		NetAdd(m,12,psObj->type);
		m.body[16]=1;												// subtype flag
	}

	m.size=( 4*(sizeof(UDWORD)) )+1;
	m.type = NET_DROIDINFO;											// set the type
	return( NETbcast(&m,FALSE));										// send it.
}




// ////////////////////////////////////////////////////////////////////////////
// receive droid information form other players. 
BOOL recvDroidInfo(NETMSG *pMsg)
{
	UDWORD		x,y,id,destid;
	DROID_ORDER order;
	DROID		*psDroid=NULL;
	OBJECT_TYPE	desttype;

	NetGet(pMsg,0,id);												//droid's id
	NetGet(pMsg,4,order);											//droid's order
	
	if(!IdToDroid(id, ANYPLAYER, &psDroid))
	{
		sendRequestDroid(id);										//droid not found, request it.
		return (FALSE);	
	}

	// now process the actual order..
	if(pMsg->body[16] == 2)
	{
		turnOffMultiMsg(TRUE);
		orderDroid(psDroid, order);
		turnOffMultiMsg(FALSE);
	}
	else if(pMsg->body[16] == 1)											// it's an object order
	{
		NetGet(pMsg,8,destid);
		NetGet(pMsg,12,desttype);
		ProcessDroidOrder(psDroid,order,0,0,desttype,destid);
	}
	else															// it's a position order
	{
		NetGet(pMsg,8,x);											// x coord
		NetGet(pMsg,12,y);											// y coord
		ProcessDroidOrder(psDroid,order,x,y,0,0);
	}
	return TRUE;
}


// ////////////////////////////////////////////////////////////////////////////
// process droid order
static VOID ProcessDroidOrder(DROID *psDroid, DROID_ORDER order,UDWORD x,		 UDWORD y, 
														 OBJECT_TYPE desttype,UDWORD destid)
{
	UDWORD		i;
	DROID		*pD;
	STRUCTURE	*pS;
	FEATURE		*pF;
	BASE_OBJECT *psObj= NULL;

	if(destid==0 && desttype==0)							// target is a location
	{
		if(    abs(psDroid->x - x )< (TILE_UNITS/2)  
			&& abs(psDroid->y - y )< (TILE_UNITS/2) )		// don't bother if close.
		{
			return;
		}

		if(order == UNKNOWN)	// get the order to do if we didn't specify it.
		{
			order = chooseOrderLoc(psDroid, x,y);
		}
	
		turnOffMultiMsg(TRUE);
		orderDroidLoc(psDroid, order, x,y);
		turnOffMultiMsg(FALSE);
	}
	else													//  target is object 
	{
		switch(desttype)
		{
		case OBJ_DROID:
			psObj = NULL;
			for (i=0; i<MAX_PLAYERS AND !psObj; i++)
			{
				for(pD=apsDroidLists[i];(pD) && (pD->id != destid);pD=pD->psNext);
				if(pD)
				{
					psObj = (BASE_OBJECT*)pD;
				}
			}
			break;	

		case OBJ_STRUCTURE:
			psObj = NULL;
			pS = IdToStruct(destid,ANYPLAYER);
			if(pS)
			{
				psObj = (BASE_OBJECT*)pS;
			}
			break;

		case OBJ_FEATURE:
			psObj = NULL;
			pF = IdToFeature(destid,ANYPLAYER);
			if(pF)
			{
				psObj = (BASE_OBJECT*)pF;
			}
			break;

		case OBJ_BULLET:												// shouldn't be getting this! 
			DBERROR(("multibot: order specified destination as a bullet. what am i to do??"));
			break;

		default:
			DBERROR(("unknown object type"));
		}

		if(!psObj)													// failed to find it;
		{	
			return;
		}

		// if we didn't sepcify an order, then make one..
		if(order == UNKNOWN)	// get the order to do if we didn't specify it.
		{
			order = chooseOrderObj(psDroid,psObj);
		}

		turnOffMultiMsg(TRUE);
		orderDroidObj(psDroid, order, psObj);
		turnOffMultiMsg(FALSE);
	}

}


// ////////////////////////////////////////////////////////////////////////////
// Inform other players that a droid has been destroyed

BOOL SendDestroyDroid(DROID *pD)
{
	NETMSG m;
	
	NetAdd(m,0,pD->id);								// id of the droid to be destroyed
	m.size	= sizeof(UDWORD);
	m.type	= NET_DROIDDEST;	
	
	return( NETbcast(&m,TRUE));						//guaranteed msg?????
}
// ////////////////////////////////////////////////////////////////////////////
// Accept a droid which was destroyed on another machine
BOOL recvDestroyDroid(NETMSG *pMsg)
{
	DROID *pD;
	UDWORD r;

	NetGet(pMsg,0,r);								// get the id of the droid.
	if( IdToDroid(r, ANYPLAYER, &pD) ) 
	{
		if(!pD->died)
		{
			turnOffMultiMsg(TRUE);
			destroyDroid(pD);						// remove the droid recvd from the remote players machine.
			turnOffMultiMsg(FALSE);
		}
		return (TRUE);
	}
	else
	{
		return FALSE;
	}
}

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// stuff for sending the WHOLE of a droid!
BOOL sendWholeDroid(DROID *pD, DPID dest)
{
	NETMSG	m;
	UDWORD	sizecount=0;
	UDWORD	noTarget =0;
	SDWORD	asParts[DROID_MAXCOMP];
	UDWORD	asWeaps[DROID_MAXWEAPS];	
	
    if (pD->asWeaps[0].nStat > 0)									// build some bits for the template.
	{
           asWeaps[0] = pD->asWeaps[0].nStat;
	}
	else
	{
		asWeaps[0] = 0;
	}

	asParts[COMP_BODY]		=	pD->asBits[COMP_BODY].nStat;		//allocate the components
	asParts[COMP_BRAIN]		=	pD->asBits[COMP_BRAIN].nStat;
	asParts[COMP_PROPULSION]=	pD->asBits[COMP_PROPULSION].nStat;
	asParts[COMP_SENSOR]	=	pD->asBits[COMP_SENSOR].nStat;
	asParts[COMP_ECM]		=	pD->asBits[COMP_ECM].nStat;
	asParts[COMP_REPAIRUNIT]=	pD->asBits[COMP_REPAIRUNIT].nStat;
	asParts[COMP_CONSTRUCT]	=	pD->asBits[COMP_CONSTRUCT].nStat;
	asParts[COMP_WEAPON]	=	pD->asBits[COMP_WEAPON].nStat;
	NetAdd(m,sizecount,asParts);					sizecount+=sizeof(asParts);	  
	NetAdd(m,sizecount,asWeaps);					sizecount+=sizeof(asWeaps);			// to build a template.

	NetAdd(m,sizecount,pD->x);						sizecount+=sizeof(pD->x);
	NetAdd(m,sizecount,pD->y);						sizecount+=sizeof(pD->y);
	NetAdd(m,sizecount,pD->z);						sizecount+=sizeof(pD->z);
	NetAdd(m,sizecount,pD->player);					sizecount+=sizeof(pD->player);	

	NetAddSt(m,sizecount,pD->aName);				sizecount+=strlen(pD->aName)+1;	
	
	// that's enough to build a template, now the specific stuff!
	NetAdd(m,sizecount,pD->id);						sizecount+=sizeof(pD->id);

	NetAdd(m,sizecount,pD->NameVersion);			sizecount+=sizeof(pD->NameVersion);
	NetAdd(m,sizecount,pD->droidType);				sizecount+=sizeof(pD->droidType);

	NetAdd(m,sizecount,pD->direction);				sizecount+=sizeof(pD->direction);
	NetAdd(m,sizecount,pD->pitch);					sizecount+=sizeof(pD->pitch);
	NetAdd(m,sizecount,pD->roll);					sizecount+=sizeof(pD->roll);
	NetAdd(m,sizecount,pD->visible);				sizecount+=sizeof(pD->visible);
	NetAdd(m,sizecount,pD->inFire);					sizecount+=sizeof(pD->inFire);
	NetAdd(m,sizecount,pD->burnDamage);				sizecount+=sizeof(pD->burnDamage);
	
	NetAdd(m,sizecount,pD->body);					sizecount+=sizeof(pD->body);
	NetAdd(m,sizecount,pD->secondaryOrder);			sizecount+=sizeof(pD->secondaryOrder);
	NetAdd(m,sizecount,pD->order);					sizecount+=sizeof(pD->order);
	NetAdd(m,sizecount,pD->orderX);					sizecount+=sizeof(pD->orderX);
	NetAdd(m,sizecount,pD->orderY);					sizecount+=sizeof(pD->orderY);
	NetAdd(m,sizecount,pD->orderX2);				sizecount+=sizeof(pD->orderX2);
	NetAdd(m,sizecount,pD->orderY2);				sizecount+=sizeof(pD->orderY2);
	
	if (pD->psTarget)
	{
		NetAdd(m,sizecount,pD->psTarget->id);		sizecount+=sizeof(pD->psTarget->id);
	}
	else
	{
		NetAdd(m,sizecount,noTarget);				sizecount+=sizeof(noTarget);
	}

	if (pD->psTarStats)
	{
		NetAdd(m,sizecount,pD->psTarStats->ref);	sizecount+=sizeof(pD->psTarStats->ref);
	}
	else
	{
		NetAdd(m,sizecount,noTarget);				sizecount+=sizeof(noTarget);
	}


	m.type = NET_WHOLEDROID;
	m.size = (UWORD)sizecount;
	return NETsend(&m,dest,FALSE);
}
// ////////////////////////////////////////////////////////////////////////////
// receive a whole droid!!!!
BOOL receiveWholeDroid(NETMSG *m)
{
	UDWORD			sizecount=0;
	DROID_TEMPLATE	dt;
	DROID			*pD,*existingdroid;
//	BOOL			temp= FALSE;

	DROIDSTORE		*tempDroid;

	//UDWORD x,y,z,id;
	UWORD x,y,z;
	UDWORD id;
	UBYTE player;

	// get the stuff
	NetGet(m,sizecount,dt.asParts);				sizecount+=sizeof(dt.asParts);		// build a template
//	NetGet(m,sizecount,dt.powerPoints);			sizecount+=sizeof(dt.powerPoints);	
	NetGet(m,sizecount,dt.asWeaps);				sizecount+=sizeof(dt.asWeaps);
	NetGet(m,sizecount,x);						sizecount+=sizeof(x);				// edit it.
	NetGet(m,sizecount,y);						sizecount+=sizeof(y);
	NetGet(m,sizecount,z);						sizecount+=sizeof(z);
	NetGet(m,sizecount,player);					sizecount+=sizeof(player);

	dt.pName = (CHAR*)&dt.aName;
	strncpy(dt.aName, &(m->body[sizecount]), DROID_MAXNAME-1);
	dt.aName[DROID_MAXNAME-1]=0;		// just in case.
	sizecount+=strlen(dt.pName)+1;		// name is pointed at directly into the buffer.
	
	if(dt.asWeaps[0] == 0)
	{
		dt.numWeaps =0;
	}
	else
	{
		dt.numWeaps = 1;
	}
	dt.powerPoints = calcTemplatePower(&dt);

	NetGet(m,sizecount,id);						sizecount+=sizeof(id);

	if(IdToDroid(id,ANYPLAYER,&existingdroid))// if a droid of id already exists then go no further.
	{
		return FALSE;
	}
	
	// could do usepower , but we usually do this in an emergency, so leave it!
	turnOffMultiMsg(TRUE);
	pD = buildDroid(&dt,x,y,player, FALSE);			// make a droid
	turnOffMultiMsg(FALSE);

	if(!pD)										// failed to build it, give up.
	{	
		return FALSE;
	}
	
	// now the instance specific stuff.
	pD->id = id;
	pD->x = x;									//correct builddroid to use exact pos, not tile center
	pD->y = y;
	pD->z = z;

	NetGet(m,sizecount,pD->NameVersion);		sizecount+=sizeof(pD->NameVersion);
	NetGet(m,sizecount,pD->droidType);			sizecount+=sizeof(pD->droidType);

	NetGet(m,sizecount,pD->direction);			sizecount+=sizeof(pD->direction);
	NetGet(m,sizecount,pD->pitch);				sizecount+=sizeof(pD->pitch);
	NetGet(m,sizecount,pD->roll);				sizecount+=sizeof(pD->roll);

	NetGet(m,sizecount,pD->visible);			sizecount+=sizeof(pD->visible);
	NetGet(m,sizecount,pD->inFire);				sizecount+=sizeof(pD->inFire);
	NetGet(m,sizecount,pD->burnDamage);			sizecount+=sizeof(pD->burnDamage);

	NetGet(m,sizecount,pD->body);				sizecount+=sizeof(pD->body);
	
	NetGet(m,sizecount,pD->secondaryOrder);		sizecount+=sizeof(pD->secondaryOrder);

	if(ingame.localJoiningInProgress)
	{
		NetGet(m,sizecount,pD->order);				sizecount+=sizeof(pD->order);		// processed laater
		NetGet(m,sizecount,pD->orderX);				sizecount+=sizeof(pD->orderX);		//later!
		NetGet(m,sizecount,pD->orderY);				sizecount+=sizeof(pD->orderY);		//later!
		NetGet(m,sizecount,pD->orderX2);			sizecount+=sizeof(pD->orderX2);
		NetGet(m,sizecount,pD->orderY2);			sizecount+=sizeof(pD->orderY2);
			
		NetGet(m,sizecount,pD->psTarget);			sizecount+=sizeof(pD->psTarget);	//later!
		NetGet(m,sizecount,pD->psTarStats);			sizecount+=sizeof(pD->psTarStats);	//later!

		//store the droid for later.
		tempDroid = MALLOC(sizeof(DROIDSTORE));
		tempDroid->psDroid  = pD;
		tempDroid->psNext	= tempDroidList;	
		tempDroidList		= tempDroid;
	}
	else							//don't bother setting the orders. they'll update sooner or later anywho. 
	{
		pD->order     = 0;
		pD->orderX    = 0;
		pD->orderY    = 0;
	
		pD->psTarget  = 0;
		pD->psTarStats= 0;
		
		addDroid(pD, apsDroidLists);	
	}
		
	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
// Functions for cases where a machine receives a netmessage about a certain 
// droid. The droid is unknown, so the machine uses tese functions in order to
// find out about it.
BOOL sendRequestDroid(UDWORD droidId)
{
	NETMSG msg;

	if(ingame.localJoiningInProgress)		// dont worry if still joining. 
	{
		return FALSE;
	}
	
	NetAdd(msg,0,droidId);
	NetAdd(msg,4,player2dpid[selectedPlayer] );

	DBPRINTF(("multibot: unknown droid %d, requesting info\n"));

	msg.type = NET_REQUESTDROID;
	msg.size = sizeof(DPID)+sizeof(UDWORD);
	
	NETbcast(&msg,FALSE);
	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
BOOL recvRequestDroid(NETMSG *pMsg)
{
	DROID	*pDroid;
	UDWORD	droidid,dpid;

	NetGet(pMsg,0,droidid);									// get the droid's id
	NetGet(pMsg,4,dpid);									// get the player who needs it.
	
	
	if(!(IdToDroid(droidid , ANYPLAYER,&pDroid) )  )		// find the droid
	{
		return TRUE;										// can't find it, so ignore.
	}
	
	if(myResponsibility(pDroid->player))					// if resposible
	{												
		sendWholeDroid(pDroid,dpid);						// send the whole of the droid
	}

	return TRUE;
}

