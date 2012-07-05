#include "Frame.h"
#include "Map.h"
#include "HCI.h"
#include "MapDisplay.h"
#include "Display3D.h"
#include "ivisdef.h" //ivis matrix code
#include "piedef.h" //pie render
#include "geo.h" //ivis matrix code
#include "MiscImd.h"
#include "Effects.h"
#include "Bridge.h"
/* 
Bridge.c
Alex McLean, Pumpkin Studios EIDOS Interactive, 1998.
Handles rendering and placement of bridging sections for
traversing water and ravines?! My guess is this won't make it into
the final game, but we'll see...
*/


/* 
Returns TRUE or FALSE as to whether a bridge is valid.
For it to be TRUE - all intervening points must be lower than the start
and end points. We can also check other stuff here like what it's going
over. Also, it has to be between a minimum and maximum length and 
one of the axes must share the same values.
*/ 
BOOL	bridgeValid(UDWORD startX,UDWORD startY, UDWORD endX, UDWORD endY)
{
BOOL	xBridge, yBridge;
UDWORD	bridgeLength;
UDWORD	startHeight,endHeight,sectionHeight;
UDWORD	i;

	/* Establish axes allignment */
	xBridge = ( (startX == endX) ? TRUE : FALSE );
	yBridge = ( (startY == endY) ? TRUE : FALSE );

	/* At least one axis must be constant */
	if (!xBridge AND !yBridge)
	{
		/*	Bridge isn't straight - this shouldn't have been passed
			in, but better safe than sorry! */
		return(FALSE);
	}

	/* Get the bridge length */
	bridgeLength = ( xBridge ? abs(startY-endY) : abs(startX-endX) );

	/* check it's not too long or short */
	if(bridgeLength<MINIMUM_BRIDGE_SPAN OR bridgeLength>MAXIMUM_BRIDGE_SPAN)
	{
		/* Cry out */
		return(FALSE);
	}

	/*	Check intervening tiles to see if they're lower 
	so first get the start and end heights */
	startHeight = mapTile(startX,startY)->height;
	endHeight = mapTile(endX,endY)->height;
	
	/*	
		Don't whinge about this piece of code please! It's nice and short
		and is called very infrequently. Could be made slightly faster. 
	*/
	for(i = ( xBridge ? ( min(startY,endY) ) : ( min(startX,endX)) ); 
		i < ( xBridge ? ( max(startY,endY) ) : ( max(startX,endX)) ); i++)
	{
		/* Get the height of a bridge section */
		sectionHeight = mapTile((xBridge ? startX : startY),i)->height;
		/* Is it higher than BOTH end points? */
		if( sectionHeight > max(startHeight,endHeight) )
		{
			/* Cry out */
			return(FALSE);
		}
	}
	/* Everything's just fine */
	return(TRUE);
}


/*	
	This function will actually draw a wall section 
	Slightly different from yer basic structure draw in that
	it's not alligned to the terrain as bridge sections sit
	at a height stored in their structure - as they're above the ground
	and wouldn't be much use if they weren't, bridge wise.
*/
BOOL	renderBridgeSection(STRUCTURE *psStructure)
{
	SDWORD			structX,structY,structZ;	
	SDWORD			rx,rz;
	//iIMDShape		*imd;
	iVector			dv;

			/* Bomb out if it's not visible and there's no active god mode */
			if(!psStructure->visible[selectedPlayer] AND !godMode)
			{
				return(FALSE);
			}

			/* Get it's x and y coordinates so we don't have to deref. struct later */
			structX = psStructure->x;
			structY = psStructure->y;
			structZ = psStructure->z;
	
			/* Establish where it is in the world */
			dv.x = (structX - player.p.x) - terrainMidX*TILE_UNITS;
			dv.z = terrainMidY*TILE_UNITS - (structY - player.p.z);
			dv.y = structZ;  

			SetBSPObjectPos(structX,dv.y,structY);	// world x,y,z coord of structure ... this is needed for the BSP code

			/* Push the indentity matrix */
			pie_MatBegin();

			/* Translate */
			pie_TRANSLATE(dv.x,dv.y,dv.z);

			/* Get the x,z translation components */
			rx = player.p.x & (TILE_UNITS-1);
			rz = player.p.z & (TILE_UNITS-1);

			/* Translate */
			pie_TRANSLATE(rx,0,-rz);

			pie_Draw3DShape(psStructure->sDisplay.imd, 0, 0, pie_DROID_BRIGHT_LEVEL, 0, 0, 0);
  	  
			pie_MatEnd();
			return(TRUE);
}
 
/*
	This will work out all the info you need about the bridge including
	length - height to set sections at in order to allign to terrain and
	what you need to alter start and end terrain heights by to establish to
	connection.
*/
void	getBridgeInfo(UDWORD startX,UDWORD startY,UDWORD endX, UDWORD endY, BRIDGE_INFO *info)
{
BOOL	xBridge,yBridge;
UDWORD	startHeight,endHeight;
BOOL	startHigher;
	
	/* Copy over the location coordinates */
	info->startX = startX;
	info->startY = startY;
	info->endX = endX;
	info->endY = endY;

	/* Get the heights of the start and end positions */
	startHeight = map_TileHeight(startX,startY);
	endHeight = map_TileHeight(endX,endY);

	/* Find out which is higher */
	startHigher = (startHeight>=endHeight ? TRUE : FALSE);
	
	/* If the start position is higher */
	if(startHigher)
	{
		/* Inform structure */
		info->startHighest = TRUE;

		/* And the end position needs raising */
		info->heightChange = startHeight - endHeight;
	}
	/* Otherwise, the end position is lower */
	else
	{
		/* Inform structure */
		info->startHighest = FALSE;
		/* So we need to raise the start position */
		info->heightChange = endHeight - startHeight;
	}
	
	/* Establish axes allignment */
	/* Only one of these can occur otherwise
	bridge is one square big */
	xBridge = ( (startX == endX) ? TRUE : FALSE );
	yBridge = ( (startY == endY) ? TRUE : FALSE );
	
	/* 
		Set the bridge's height.
		Note that when the bridge is built BOTH tile heights need
		to be set to the agreed value on their bridge trailing edge 
		(x,y) and (x,y+1) is constant X and (x,y) and (x+1,y) if constant
		Y
	*/
	if(startHigher)
	{
		info->bridgeHeight = map_TileHeight(startX,startY);
	}
	else
	{
		info->bridgeHeight = map_TileHeight(endX,endY);
	}
	
	/* We've got a bridge of constant X */
	if(xBridge)
	{
		info->bConstantX = TRUE;
		info->bridgeLength = abs(startY-endY);
	}
	/* We've got a bridge of constant Y */
	else if(yBridge)
	{
		info->bConstantX = FALSE;
		info->bridgeLength = abs(startX-endX);
	}
	else
	{
		DBERROR(("Weirdy Bridge requested - no axes allignment"));
	}
}
	
void	testBuildBridge(UDWORD startX,UDWORD startY,UDWORD endX,UDWORD endY)
{
BRIDGE_INFO	bridge;
UDWORD	i;
iVector	dv;

	if(bridgeValid(startX,startY,endX,endY))
	{
		getBridgeInfo(startX,startY,endX,endY,&bridge);
		if(bridge.bConstantX)
		{
	
			for(i=min(bridge.startY,bridge.endY); i<(max(bridge.startY,bridge.endY)+1); i++)
			{
		   		dv.x = ((bridge.startX*128)+64);
		   		dv.z = ((i*128)+64);
				dv.y = bridge.bridgeHeight;
				addEffect(&dv,EFFECT_SMOKE,SMOKE_TYPE_DRIFTING,FALSE,NULL,0);
//				addExplosion(&dv,TYPE_EXPLOSION_SMOKE_CLOUD,NULL);
			}
		}
		else
		{
			for(i=min(bridge.startX,bridge.endX); i<(max(bridge.startX,bridge.endX)+1); i++)
			{
		   		dv.x = ((i*128)+64);
		   		dv.z = ((bridge.startY*128)+64);
				dv.y = bridge.bridgeHeight;
				addEffect(&dv,EFFECT_SMOKE,SMOKE_TYPE_DRIFTING,FALSE,NULL,0);
//				addExplosion(&dv,TYPE_EXPLOSION_SMOKE_CLOUD,NULL);
			}
		}
			/* Flatten the start tile */
			setTileHeight(bridge.startX,bridge.startY,bridge.bridgeHeight);
			setTileHeight(bridge.startX,bridge.startY+1,bridge.bridgeHeight);
			setTileHeight(bridge.startX+1,bridge.startY,bridge.bridgeHeight);
			setTileHeight(bridge.startX+1,bridge.startY+1,bridge.bridgeHeight);

			/* Flatten the end tile */
			setTileHeight(bridge.endX,bridge.endY,bridge.bridgeHeight);
			setTileHeight(bridge.endX,bridge.endY+1,bridge.bridgeHeight);
			setTileHeight(bridge.endX+1,bridge.endY,bridge.bridgeHeight);
			setTileHeight(bridge.endX+1,bridge.endY+1,bridge.bridgeHeight);
	}
	else
	{
		getBridgeInfo(startX,startY,endX,endY,&bridge);
		if(bridge.bConstantX)
		{
			for(i=min(bridge.startY,bridge.endY); i<(max(bridge.startY,bridge.endY)+1); i++)
			{
		   		dv.x = ((bridge.startX*128)+64);
		   		dv.z = ((i*128)+64);
				dv.y = bridge.bridgeHeight;
				addEffect(&dv,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
//				addExplosion(&dv,TYPE_EXPLOSION_MED,NULL);
			}
		}
		else
		{
			for(i=min(bridge.startX,bridge.endX); i<(max(bridge.startX,bridge.endX)+1); i++)
			{
		   		dv.x = ((i*128)+64);
		   		dv.z = ((bridge.startY*128)+64);
				dv.y = bridge.bridgeHeight;
				addEffect(&dv,EFFECT_EXPLOSION,EXPLOSION_TYPE_SMALL,FALSE,NULL,0);
//				addExplosion(&dv,TYPE_EXPLOSION_MED,NULL);
			}
		}
	
	}
}


