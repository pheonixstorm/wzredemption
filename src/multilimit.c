/*
 * multilimit.c
 *
 * interface for setting limits to the game, bots, structlimits etc...
 */ 

#include "frame.h"
#include "csnap.h"
#include "widget.h"
#include "hci.h"
#include "text.h"
#include "intimage.h"
#include "intdisplay.h"
#include "init.h"		// for gameheap
#include "frend.h"
#include "stats.h"
#include "frontend.h"
#include "component.h"
#include "loadsave.h"
#include "wrappers.h"	// for loading screen
#include "gtime.h"
#include "console.h"
#include "bitimage.h"	// GFX incs
#include "textdraw.h"
#include "piematrix.h"
#include "piestate.h"
#include "netplay.h"
#include "multiplay.h"
#include "multirecv.h"
#include "multiint.h"
#include "multilimit.h"
#include "pieMode.h"

// ////////////////////////////////////////////////////////////////////////////
// externs
extern IMAGEFILE	*FrontImages;
extern CURSORSNAP	InterfaceSnap;
extern void			intDisplayPlainForm	(struct _widget *psWidget, UDWORD xOffset, 
										 UDWORD yOffset, UDWORD *pColours);

// ////////////////////////////////////////////////////////////////////////////
// defines
#define	IDLIMITS				22000
#define IDLIMITS_RETURN			(IDLIMITS+1)
#define IDLIMITS_OK				(IDLIMITS+2)
#define IDLIMITS_TABS			(IDLIMITS+3)
#define IDLIMITS_ENTRIES_START	(IDLIMITS+4)
#define IDLIMITS_ENTRIES_END	(IDLIMITS+99)

#define LIMITSX					25
#define LIMITSY					30
#define LIMITSW					580
#define LIMITSH					430

#define LIMITS_OKX				(LIMITSW-90)
#define LIMITS_OKY				(LIMITSH-42)

#define BARWIDTH				480
#define BARHEIGHT				40
#define BUTPERFORM				8
// ////////////////////////////////////////////////////////////////////////////
// protos.
BOOL startLimitScreen	(VOID);
VOID runLimitScreen		(VOID);
VOID applyLimitSet		(VOID);
VOID createLimitSet		(VOID);

VOID displayStructureBar(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);

// ////////////////////////////////////////////////////////////////////////////

BOOL useStruct(UDWORD count,UDWORD i)
{
//	STRUCTURE_STATS	*pStat = asStructureStats+i;

	if(count >= (4*BUTPERFORM))
	{
		return FALSE;
	}

	// now see if we loaded that stat..
	if(asStructLimits[0][i].globalLimit ==LOTS_OF)
	{
		return FALSE;
	}

	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
BOOL startLimitScreen(void)
{
	W_FORMINIT		sButInit;
	W_FORMINIT		sFormInit;			
	UDWORD			numButtons = 0;
	UDWORD			i;

	addBackdrop();											//background

	// load stats...
	if(!bForceEditorLoaded)
	{
		initLoadingScreen( TRUE, TRUE);//changed by jeremy mar8
/*		if (!resLoad("wrf\\forcedit.wrf", 500,
					 DisplayBuffer, displayBufferSize,
					 psGameHeap))				//need the object heaps to have been set up before loading 
		{
			return FALSE;
		}
*/
		if (!resLoad("wrf\\piestats.wrf", 501,
					 DisplayBuffer, displayBufferSize,
					 psGameHeap))				//need the object heaps to have been set up before loading 
		{
			return FALSE;
		}
		
		if (!resLoad("wrf\\forcedit2.wrf", 502,
					 DisplayBuffer, displayBufferSize,
					 psGameHeap))				//need the object heaps to have been set up before loading 
		{
			return FALSE;
		}

		pie_GlobalRenderBegin(TRUE);//force to black
		bForceEditorLoaded = TRUE;
		closeLoadingScreen();
	}

	addSideText(FRONTEND_SIDETEXT1,LIMITSX-2,LIMITSY,"LIMITS");	// draw sidetext...

	memset(&sFormInit, 0, sizeof(W_FORMINIT));				// draw blue form...
	sFormInit.formID	= FRONTEND_BACKDROP;
	sFormInit.id		= IDLIMITS;
	sFormInit.style		= WFORM_PLAIN;
	sFormInit.x			= LIMITSX;
	sFormInit.y			= LIMITSY;
	sFormInit.width		= LIMITSW;
	sFormInit.height	= LIMITSH;
	sFormInit.pDisplay	= intDisplayPlainForm;
	widgAddForm(psWScreen, &sFormInit);	

	// return button.
//	addMultiBut(psWScreen,IDLIMITS,IDLIMITS_RETURN,
//					8,5,
//					iV_GetImageWidth(FrontImages,IMAGE_RETURN),
//					iV_GetImageHeight(FrontImages,IMAGE_RETURN),
//					STR_MUL_CANCEL,IMAGE_RETURN,IMAGE_RETURN_HI,TRUE);


	// ok button
//	addMultiBut(psWScreen,IDLIMITS,IDLIMITS_OK,
//					LIMITS_OKX,LIMITS_OKY,
//					iV_GetImageWidth(FrontImages,IMAGE_BIGOK),
//					iV_GetImageHeight(FrontImages,IMAGE_BIGOK),
//					STR_MUL_OK,IMAGE_BIGOK,IMAGE_BIGOK,TRUE);

	addMultiBut(psWScreen,IDLIMITS,IDLIMITS_RETURN,
					LIMITS_OKX-40,LIMITS_OKY,
					iV_GetImageWidth(FrontImages,IMAGE_RETURN),
					iV_GetImageHeight(FrontImages,IMAGE_RETURN),
					STR_MUL_CANCEL,IMAGE_NO,IMAGE_NO,TRUE);


	// ok button
	addMultiBut(psWScreen,IDLIMITS,IDLIMITS_OK,
					LIMITS_OKX,LIMITS_OKY,
					iV_GetImageWidth(FrontImages,IMAGE_BIGOK),
					iV_GetImageHeight(FrontImages,IMAGE_BIGOK),
					STR_MUL_OK,IMAGE_OK,IMAGE_OK,TRUE);


	
	// Count the number of minor tabs needed	
	numButtons = 0;

	for(i=0;i<numStructureStats;i++ )
	{
		if(useStruct(numButtons,i))
		{
			numButtons++;
		}
	}

	if(numButtons >(4*BUTPERFORM)) numButtons =(4*BUTPERFORM);

	// add tab form..
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = IDLIMITS;	
	sFormInit.id = IDLIMITS_TABS;
	sFormInit.style = WFORM_TABBED;
	sFormInit.x = 50;
	sFormInit.y = 10;
	sFormInit.width = LIMITSW - 100;	
	sFormInit.height =LIMITSH - 4;	
	sFormInit.numMajor = numForms(numButtons, BUTPERFORM);
	sFormInit.majorPos = WFORM_TABTOP;
	sFormInit.minorPos = WFORM_TABNONE;
	sFormInit.majorSize = OBJ_TABWIDTH+3; //!!
	sFormInit.majorOffset = OBJ_TABOFFSET;
	sFormInit.tabVertOffset = (OBJ_TABHEIGHT/2);			//(DES_TAB_HEIGHT/2)+2;
	sFormInit.tabMajorThickness = OBJ_TABHEIGHT;
	sFormInit.pFormDisplay = intDisplayObjectForm;	
	sFormInit.pUserData = (void*)&StandardTab;
	sFormInit.pTabDisplay = intDisplayTab;
	for (i=0; i< sFormInit.numMajor; i++)
	{
		sFormInit.aNumMinors[i] = 1;
	}
	widgAddForm(psWScreen, &sFormInit);

	//Put the buttons on it 
	memset(&sButInit, 0, sizeof(W_BUTINIT));		
	sButInit.formID   = IDLIMITS_TABS;//IDLIMITS;
	sButInit.style	  = WFORM_PLAIN;
	sButInit.width    = BARWIDTH;
	sButInit.height	  = BARHEIGHT;
	sButInit.pDisplay = displayStructureBar;
	sButInit.x		  = 2;
	sButInit.y		  = 5;
	sButInit.id	 	  = IDLIMITS_ENTRIES_START;

	numButtons =0;
	for(i=0; i<numStructureStats ;i++)
	{
		if(useStruct(numButtons,i))
		{
			numButtons++;
			sButInit.pUserData= (VOID*) i;
		
			widgAddForm(psWScreen, &sButInit);
			sButInit.id	++;
			
			addFESlider(sButInit.id,sButInit.id-1, 290,11,
						asStructLimits[0][i].globalLimit,
						asStructLimits[0][i].limit, 0);
			sButInit.id	++;


			if (sButInit.y + BARHEIGHT + 2 > (BUTPERFORM*(BARHEIGHT+2) - 4) )
			{
				sButInit.y = 5;
				sButInit.majorID += 1;
			}
			else
			{
				sButInit.y +=  BARHEIGHT +5;
			}
		}
	}

	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////

VOID runLimitScreen(void)
{
	UDWORD id,statid;	
	
	processFrontendSnap(FALSE);
	frontendMultiMessages();							// network stuff.

	id = widgRunScreen(psWScreen);						// Run the current set of widgets 

	// sliders 
	if((id > IDLIMITS_ENTRIES_START)  && (id< IDLIMITS_ENTRIES_END))
	{
		statid = (UDWORD) widgGetFromID(psWScreen,id-1)->pUserData ;
		if(statid)
		{
			asStructLimits[0][statid].limit = (UBYTE) ((W_SLIDER*)(widgGetFromID(psWScreen,id)))->pos;
		}
	}
	else
	{
		// icons that are always about.
		switch(id)
		{
		case IDLIMITS_RETURN:
			// reset the sliders..
			eventReset();
//			resReleaseBlockData(500);
			resReleaseBlockData(501);
			resReleaseBlockData(502);
//			eventReset();
			bForceEditorLoaded = FALSE;
			changeTitleMode(MULTIOPTION);
			

			// make some noize.
			if(!ingame.localOptionsReceived)
			{				
				addConsoleMessage("Limits Reset To Default Values",DEFAULT_JUSTIFY);
			}
			else
			{
				sendTextMessage("Limits Reset To Default Values",TRUE);	
			}


			break;
		case IDLIMITS_OK:
			createLimitSet();
			changeTitleMode(MULTIOPTION);
			break;
		default:
			break;
		}
	}

	StartCursorSnap(&InterfaceSnap);

	DrawBegin();
	widgDisplayScreen(psWScreen);						// show the widgets currently running
	DrawEnd();
}

// ////////////////////////////////////////////////////////////////////////////
VOID createLimitSet(VOID)
{
	UDWORD	i,numchanges;
	VOID	*pChanges;
	UBYTE	*pEntry;

	if(ingame.numStructureLimits)									// free the old set if required.
	{
		ingame.numStructureLimits = 0;
		FREE(ingame.pStructureLimits);
	}

	numchanges =0;													// count number of changes
	for(i=0;i<numStructureStats;i++)
	{
		if(asStructLimits[0][i].limit != LOTS_OF)
		{
			numchanges++;	
		}
	}

	//close your eyes now
	pChanges = MALLOC(numchanges*(sizeof(UDWORD)+sizeof(UBYTE)));	// allocate some mem for this.
	pEntry = pChanges;

	for(i=0;i<numStructureStats;i++)								// prepare chunk.
	{
		if(asStructLimits[0][i].limit != LOTS_OF)
		{
			memcpy(pEntry, &i,sizeof(UDWORD));
			pEntry += sizeof(UDWORD);
			memcpy(pEntry, &asStructLimits[0][i].limit,sizeof(UBYTE));
			pEntry += sizeof(UBYTE);
		}
	}
	// you can open them again.

	ingame.numStructureLimits	= numchanges;
	ingame.pStructureLimits		= pChanges;

	sendOptions(0,0);

	return;
}

// ////////////////////////////////////////////////////////////////////////////
VOID applyLimitSet(VOID)
{
	UBYTE *pEntry;
	UDWORD i;
	UBYTE val;
	UDWORD id;
	
	if(ingame.numStructureLimits == 0)
	{
		return;
	}

	// get the limits
	// decode and apply
	pEntry = ingame.pStructureLimits;
	for(i=0;i<ingame.numStructureLimits;i++)								// prepare chunk.
	{
		memcpy(&id,pEntry,sizeof(UDWORD));
		pEntry += sizeof(UDWORD);
		memcpy(&val,pEntry,sizeof(UBYTE));
		pEntry += sizeof(UBYTE);
	
		if(id <numStructureStats)
		{
			asStructLimits[0][id].limit=val;
			asStructLimits[1][id].limit=val;
			asStructLimits[2][id].limit=val;
			asStructLimits[3][id].limit=val;
			asStructLimits[4][id].limit=val;
			asStructLimits[5][id].limit=val;
			asStructLimits[6][id].limit=val;
			asStructLimits[7][id].limit=val;
		}
	}

	// free.
	if(	ingame.numStructureLimits )	
	{
		FREE(ingame.pStructureLimits);
		ingame.numStructureLimits = 0;
	}
}

// ////////////////////////////////////////////////////////////////////////////

VOID displayStructureBar(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	UDWORD	x = xOffset+psWidget->x;
	UDWORD	y = yOffset+psWidget->y;
	UDWORD	w = psWidget->width;
	UDWORD	h = psWidget->height; 
	STRUCTURE_STATS	*stat = asStructureStats+(UDWORD)psWidget->pUserData;
	iVector	Rotation,Position;
	CHAR	str[3];
	
	UDWORD scale,Radius;

	UNUSEDPARAMETER(pColours);
	
	drawBlueBox(x,y,w,h);

	// draw image
	pie_SetGeometricOffset( x+35 ,y+(psWidget->height/2)+9);
	Rotation.x = -15;
	Rotation.y = ((gameTime2/45)%360) ; //45
	Rotation.z = 0;
	Position.x = 0;
	Position.y = 0;
	Position.z = BUTTON_DEPTH*2;//getStructureStatSize(stat)  * 38 * OBJECT_RADIUS;

	Radius = getStructureStatSize(stat);
	if(Radius <= 128) {
		scale = SMALL_STRUCT_SCALE;
	} else if(Radius <= 256) {
		scale = MED_STRUCT_SCALE;
	} else {
		scale = LARGE_STRUCT_SCALE;
	}

	pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);
	displayStructureStatButton(stat ,0,	 &Rotation,&Position,TRUE, scale);
	pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);				

	// draw name
	iV_SetFont(WFont);											// font
	iV_SetTextColour(-1);										//colour
	iV_DrawText(getName(stat->pName), x+80, y+(psWidget->height/2)+3);

	// draw limit
	sprintf(str,"%d",((W_SLIDER*)(widgGetFromID(psWScreen,psWidget->id+1)))->pos);
	iV_DrawText(str, x+270, y+(psWidget->height/2)+3);

	// add snap
	AddCursorSnap(&InterfaceSnap,(SWORD)(x+10) , (SWORD)(y+10) , psWidget->formID,psWidget->id,NULL);

	return;
}
