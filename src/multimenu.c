/*
 *  MultiMenu.c
 *  Handles the In Game MultiPlayer Screen, alliances etc...
 *  Also the selection of disk files..
 */

#include "frame.h"
#include "widget.h"

#include "Display3d.h"
#include "intDisplay.h"
#include "text.h"
#include "vid.h"
#include "piedef.h"
#include "gtime.h"
#include "geo.h"
#include "levels.h"
#include "objmem.h"		 	//for droid lists.
#include "component.h"		// for disaplycomponentobj.
#include "HCI.h"			// for wFont def.& intmode.
//#include "intfac.h"		// for images.
#include "power.h"
#include "loadsave.h"		// for drawbluebox
#include "console.h"
#include "ai.h"
#include "csnap.h"
#include "frend.h"
#include "netplay.h"
#include "multiplay.h"
#include "multistat.h"
#include "multimenu.h"
#include "multiint.h"
#include "multigifts.h"
#include "multijoin.h"

// ////////////////////////////////////////////////////////////////////////////
// defines

W_SCREEN  *psRScreen;			// requester stuff.

extern CURSORSNAP InterfaceSnap;
//extern W_SCREEN *psWScreen;
extern IMAGEFILE *FrontImages;
extern void	displayMultiBut(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours);

BOOL	MultiMenuUp			= FALSE;
BOOL	ClosingMultiMenu	= FALSE;
static UDWORD	context;	

#define MULTIMENU_FORM_X		10 + D_W
#define MULTIMENU_FORM_Y		23 + D_H
#define MULTIMENU_FORM_W		620
#define MULTIMENU_FORM_H		295

#define MULTIMENU_PLAYER_H		32
#define MULTIMENU_FONT_OSET		20

#define MULTIMENU_C1			30
#define MULTIMENU_C2			(MULTIMENU_C1+30)

#define MULTIMENU_C0			(MULTIMENU_C2+95)
#define MULTIMENU_C3			(MULTIMENU_C0+36)

#define MULTIMENU_C4			(MULTIMENU_C3+36)
#define MULTIMENU_C5			(MULTIMENU_C4+32)
#define MULTIMENU_C6			(MULTIMENU_C5+32)
#define MULTIMENU_C7			(MULTIMENU_C6+32)



#define MULTIMENU_C8			(MULTIMENU_C7+45)
#define MULTIMENU_C9			(MULTIMENU_C8+95)
#define MULTIMENU_C10			(MULTIMENU_C9+50)
#define MULTIMENU_C11			(MULTIMENU_C10+45)


#define MULTIMENU_CLOSE			(MULTIMENU+1)
#define MULTIMENU_PLAYER		(MULTIMENU+2)

#define	MULTIMENU_ALLIANCE_BASE (MULTIMENU_PLAYER        +MAX_PLAYERS)
#define	MULTIMENU_GIFT_RAD		(MULTIMENU_ALLIANCE_BASE +MAX_PLAYERS)
#define	MULTIMENU_GIFT_RES		(MULTIMENU_GIFT_RAD		 +MAX_PLAYERS)
#define	MULTIMENU_GIFT_DRO		(MULTIMENU_GIFT_RES		 +MAX_PLAYERS)
#define	MULTIMENU_GIFT_POW		(MULTIMENU_GIFT_DRO		 +MAX_PLAYERS)
#define MULTIMENU_CHANNEL		(MULTIMENU_GIFT_POW		 +MAX_PLAYERS)
	
#define MULTIMENU_STOPS			50
#define MULTIMENU_MIDPOS		(MULTIMENU_STOPS/2)
#define MULTIMENU_MULTIPLIER	((100/MULTIMENU_STOPS)*2)

/// requester stuff.
#define M_REQUEST_CLOSE (MULTIMENU+49)
#define M_REQUEST		(MULTIMENU+50)
#define M_REQUEST_TAB	(MULTIMENU+51)
#define M_REQUEST_BUT	(MULTIMENU+55)		// allow loads of buttons.
#define M_REQUEST_BUTM	(MULTIMENU+150)

#define M_REQUEST_C1	(MULTIMENU+151)
#define M_REQUEST_C2	(MULTIMENU+152)
#define M_REQUEST_C3	(MULTIMENU+153)

#define M_REQUEST_X		MULTIOP_PLAYERSX
#define M_REQUEST_Y		MULTIOP_PLAYERSY
#define M_REQUEST_W		MULTIOP_PLAYERSW
#define M_REQUEST_H		MULTIOP_PLAYERSH


#define	R_BUT_W			105//112
#define R_BUT_H			40


BOOL			multiRequestUp = FALSE;				//multimenu is up.
static BOOL		giftsUp[MAX_PLAYERS] = {TRUE};		//gift buttons for player are up.


// ////////////////////////////////////////////////////////////////////////////
// Map / force / name load save stuff.

// ////////////////////////////////////////////////////////////////////////////
// enumerates maps in the gamedesc file.
// returns only maps that are valid the right 'type'

BOOL enumerateMultiMaps(STRING *found, UDWORD *players,BOOL first, UBYTE camToUse)
{	
	static LEVEL_DATASET *lev;
	UBYTE cam;

	if(first)
	{
		lev = psLevels;
	}
	while(lev)
	{
//		if(game.type == DMATCH)
//		{
//			if(lev->type == DMATCH)
//			{
//				strcpy(found,lev->pName);
//				*players = lev->players;
//				lev = lev->psNext;
//				return TRUE;
//			}
//		}
//		else
		if(game.type == SKIRMISH)
		{
			if(lev->type == MULTI_SKIRMISH2)
			{	
				cam = 2;
			} 
			else if(lev->type == MULTI_SKIRMISH3)
			{	
				cam = 3;
			}
//			else if(lev->type == MULTI_SKIRMISHA)
//			{	
//				cam = 0;
//			}
			else
			{
				cam = 1;
			}

			if((lev->type == SKIRMISH || lev->type == MULTI_SKIRMISH2 || lev->type == MULTI_SKIRMISH3)
//				|| lev->type == MULTI_SKIRMISHA)
				&& cam == camToUse)
			{
				strcpy(found,lev->pName);
				*players = lev->players;
				lev = lev->psNext;
				return TRUE;
			}
		}
		else	//  campaign, teamplay
		{
// 'service pack 1' 
			if(lev->type == MULTI_CAMPAIGN2)
			{	
				cam = 2;
			} 
			else if(lev->type == MULTI_CAMPAIGN3)
			{	
				cam = 3;
			}
//			else if(lev->type == MULTI_CAMPAIGNA)
//			{	
//				cam = 0;
//			}
			else
			{
				cam = 1;
			}
//	end of service pack
		
			if((lev->type == CAMPAIGN || lev->type == MULTI_CAMPAIGN2 || lev->type == MULTI_CAMPAIGN3 )
//				||lev->type == MULTI_CAMPAIGNA)
				&& cam == camToUse )
			{
				strcpy(found,lev->pName);
				*players = lev->players;
				lev = lev->psNext;
				return TRUE;
			}


		}
		lev = lev->psNext;
	}
	return FALSE;

}

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////
void displayRequestOption(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	
	UDWORD	x = xOffset+psWidget->x;
	UDWORD	y = yOffset+psWidget->y;
//	UWORD	im = (UWORD)UNPACKDWORD_TRI_B((UDWORD)psWidget->pUserData);
//	UWORD	im2= (UWORD)(UNPACKDWORD_TRI_C((UDWORD)psWidget->pUserData));
	UDWORD	count;
	STRING  butString[255];
	UNUSEDPARAMETER(pColours);
	
	strcpy(butString,((W_BUTTON *)psWidget)->pTip);

	drawBlueBox(x,y,psWidget->width,psWidget->height);	//draw box
		
	iV_SetFont(WFont);									// font
	iV_SetTextColour(-1);								//colour

	while(iV_GetTextWidth(butString) > psWidget->width -10 )
	{
		butString[strlen(butString)-1]='\0';
	}

	//draw text								
	iV_DrawText( (UCHAR*)butString,
				 x+8,
				 y+24);


	// if map, then draw no. of players.
	for(count=0;count<(UDWORD)psWidget->pUserData;count++)
	{
		iV_DrawTransImage(FrontImages,IMAGE_WEE_GUY,(x+(6*count)+8),y+28);
	}

	AddCursorSnap(&InterfaceSnap, (SWORD)(x+5),(SWORD)(y+5),psWidget->formID,psWidget->id,NULL);

}

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////

void displayCamTypeBut(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	UDWORD	x = xOffset+psWidget->x;
	UDWORD	y = yOffset+psWidget->y;
//	UDWORD	count;
	UNUSEDPARAMETER(pColours);

	drawBlueBox(x,y,psWidget->width,psWidget->height);	//draw box
	pie_DrawText( &(psWidget->UserData), x+2, y+12);

}

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////

VOID addMultiRequest(STRING *ToFindb,UDWORD mode, UBYTE mapCam)
{
	W_FORMINIT		sFormInit;
	W_BUTINIT		sButInit;
	UDWORD			players,numButtons,butPerForm,i;
	WIN32_FIND_DATA	found;	
	HANDLE			dir;
	STRING			ToFind[255],sTemp[64];
	static STRING	tips[40][MAX_STR_SIZE];
	UDWORD			count;

	numButtons = 0;
	
	context = mode;

	if(GetCurrentDirectory(255,(char*)&ToFind) == 0)
	{
		return;
	}
	strcat(ToFind,ToFindb);

	// count buttons.
	dir =FindFirstFile(ToFind,&found);
	if(dir != INVALID_HANDLE_VALUE)
	{
		while( TRUE ) 
		{
			numButtons++;
			if(! FindNextFile(dir,&found ) )
			{
				break;
			}
		}
	}
	FindClose(dir);

	
	if(mode == MULTIOP_MAP)									// if its a map, also look in the predone stuff.
	{
		if(enumerateMultiMaps(tips[0],&players, TRUE,mapCam))
		{
			numButtons++;
			while(enumerateMultiMaps(tips[0],&players, FALSE,mapCam))
			{
				numButtons++;
			}
		}
	}

	widgCreateScreen(&psRScreen);			/// move this to intinit or somewhere like that.. (close too.)
	widgSetTipFont(psRScreen,WFont);

	/* Calculate how many buttons will go on a single form */
	butPerForm = ((M_REQUEST_W - 0 - 4) / 
						(R_BUT_W +4)) *
				 ((M_REQUEST_H - 0- 4) / 
						(R_BUT_H+ 4));

	/* add a form to place the tabbed form on */
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = 0;
	sFormInit.id = M_REQUEST;
	sFormInit.style = WFORM_PLAIN;
	sFormInit.x = (SWORD)(M_REQUEST_X+D_W);
	sFormInit.y = (SWORD)(M_REQUEST_Y+D_H);
	sFormInit.width = M_REQUEST_W;
	sFormInit.height = M_REQUEST_H;
	sFormInit.disableChildren = TRUE;
	sFormInit.pDisplay = intOpenPlainForm;
	widgAddForm(psRScreen, &sFormInit);

	/* Add the tabs */
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID = M_REQUEST;	
	sFormInit.id = M_REQUEST_TAB;
	sFormInit.style = WFORM_TABBED;
	sFormInit.x = 2;
	sFormInit.y = 2;
	sFormInit.width = M_REQUEST_W;	
	sFormInit.height = M_REQUEST_H-4;	
	if(numForms(numButtons, butPerForm) >4)
	{
		sFormInit.numMajor =4;
	}
	else
	{
		sFormInit.numMajor = numForms(numButtons, butPerForm);
	}
	sFormInit.majorPos = WFORM_TABTOP;
	sFormInit.minorPos = WFORM_TABNONE;
	sFormInit.majorSize = OBJ_TABWIDTH+2;
	sFormInit.majorOffset = OBJ_TABOFFSET;
	sFormInit.tabVertOffset = (OBJ_TABHEIGHT/2);
	sFormInit.tabMajorThickness = OBJ_TABHEIGHT;
	sFormInit.pFormDisplay = intDisplayObjectForm;	
	sFormInit.pUserData = (void*)&StandardTab;
	sFormInit.pTabDisplay = intDisplayTab;
	for (i=0; i< sFormInit.numMajor; i++)
	{
		sFormInit.aNumMinors[i] = 1;
	}
	widgAddForm(psRScreen, &sFormInit);

	// Add the close button.
	memset(&sButInit, 0, sizeof(W_BUTINIT));
	sButInit.formID = M_REQUEST;
	sButInit.id = M_REQUEST_CLOSE;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = M_REQUEST_W - CLOSE_WIDTH;
	sButInit.y = 0;
	sButInit.width = CLOSE_WIDTH;
	sButInit.height = CLOSE_HEIGHT;
	sButInit.pTip = strresGetString(psStringRes, STR_MISC_CLOSE);
	sButInit.FontID = WFont;
	sButInit.pDisplay = intDisplayImageHilight;
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_CLOSEHILIGHT , IMAGE_CLOSE);
	widgAddButton(psRScreen, &sButInit);
	
	/* Put the buttons on it *//* Set up the button struct */
	memset(&sButInit, 0, sizeof(W_BUTINIT));
	sButInit.formID = M_REQUEST_TAB;
	sButInit.id		= M_REQUEST_BUT;
	sButInit.style	= WBUT_PLAIN;
	sButInit.x		= 17;
	sButInit.y		= 4;
	sButInit.width	= R_BUT_W;
	sButInit.height = R_BUT_H;
	sButInit.pUserData	= (void*)0;			
	sButInit.pDisplay	= displayRequestOption; 
	sButInit.FontID		= WFont;

	dir =FindFirstFile(ToFind,&found);
	if(dir != INVALID_HANDLE_VALUE)
	{
		count=0;
		while( count < (butPerForm*4)  ) 
		{
			/* Set the tip and add the button */		
// use MALLOC, and do it dynamically
			found.cFileName[strlen(found.cFileName) -4 ] = '\0';			// chop extension
			strcpy(tips[sButInit.id-M_REQUEST_BUT],found.cFileName);		//need to store one!
				
			sButInit.pTip		= tips[sButInit.id-M_REQUEST_BUT];
			sButInit.pText		= tips[sButInit.id-M_REQUEST_BUT];
	
			if(mode == MULTIOP_MAP)											// if its a map, set player flag.
			{
				sButInit.pUserData	= (void*)( found.cFileName[0]-'0'  );

//				if(game.type == DMATCH)
//				{				
//					if( found.cFileName[1] != 'd')
//					{
//						goto nextone;
//					}
//				}
//				else
//				{
					if( found.cFileName[1] != 'c')
					{
						goto nextone;
					}
//				}
	
				strcpy(sTemp,   strrchr(found.cFileName,'-')+1  );		//chop off description

				// add number of players to string, choping of description
			//	sprintf(tips[sButInit.id-M_REQUEST_BUT], "%d)%s",
			//											sButInit.pUserData, 
			//											sTemp  );
				sprintf(tips[sButInit.id-M_REQUEST_BUT], "%s", sTemp  );
			}
			
			count++;
			widgAddButton(psRScreen, &sButInit);
			
			/* Update the init struct for the next button */
			sButInit.id += 1;
			sButInit.x = (SWORD)(sButInit.x + (R_BUT_W+ 4));
			if (sButInit.x + R_BUT_W+ 2 > M_REQUEST_W)
			{
				sButInit.x = 17;
				sButInit.y = (SWORD)(sButInit.y +R_BUT_H + 4);
			}
			if (sButInit.y +R_BUT_H + 4 > M_REQUEST_H)
			{
				sButInit.y = 4;
				sButInit.majorID += 1;
			}
			
 nextone:

			if(!FindNextFile(dir,&found ) )	/* find next one*/	
			{
				break;
			}
		}
	}
	FindClose(dir);

	if(mode == MULTIOP_MAP)		
	{
		if(enumerateMultiMaps( sTemp,&players,TRUE,mapCam))
		{
			do{
				
				// add number of players to string.
//				sprintf(tips[sButInit.id-M_REQUEST_BUT], "%d)%s",players,sTemp );
				sprintf(tips[sButInit.id-M_REQUEST_BUT],"%s",sTemp );
				//found.cFileName[strlen(found.cFileName) -4 ] = '\0';			// chop extension
				//strcpy(tips[sButInit.id-M_REQUEST_BUT],found.cFileName);		//need to store one!		
				
				sButInit.pTip = tips[sButInit.id-M_REQUEST_BUT];
				sButInit.pText = tips[sButInit.id-M_REQUEST_BUT];
				sButInit.pUserData	= (void*)players; 

				widgAddButton(psRScreen, &sButInit);
				
				sButInit.id += 1;
				sButInit.x = (SWORD)(sButInit.x + (R_BUT_W+ 4));
				if (sButInit.x + R_BUT_W+ 2 > M_REQUEST_W)
				{
					sButInit.x = 17;
					sButInit.y = (SWORD)(sButInit.y +R_BUT_H + 4);
				}	
				if (sButInit.y +R_BUT_H + 4 > M_REQUEST_H)
				{
					sButInit.y = 4;
					sButInit.majorID += 1;
				}
			}while(enumerateMultiMaps(sTemp,&players,FALSE,mapCam));
		}
	}
	multiRequestUp = TRUE;


	// if it's map select then add the cam style buttons.
	if(mode == MULTIOP_MAP)								
	{
		memset(&sButInit, 0, sizeof(W_BUTINIT));
		sButInit.formID = M_REQUEST;
		sButInit.id		= M_REQUEST_C1;
		sButInit.style	= WBUT_PLAIN;
		sButInit.x		= 4;
		sButInit.y		= 17;
		sButInit.width	= 10;
		sButInit.height = 17;
		sButInit.UserData	= '1';			
		sButInit.FontID		= WFont;
		sButInit.pTip		= "Tech:1";
		sButInit.pDisplay	= displayCamTypeBut;
		
		widgAddButton(psRScreen, &sButInit);

		sButInit.id		=	M_REQUEST_C2;
		sButInit.y		+=	22;
		sButInit.UserData	= '2';			
		sButInit.pTip		= "Tech:2";
		widgAddButton(psRScreen, &sButInit);

		sButInit.id		= M_REQUEST_C3;
		sButInit.y		+=	22;
		sButInit.UserData	= '3';			
		sButInit.pTip		= "Tech:3";
		widgAddButton(psRScreen, &sButInit);

//		sButInit.id		= M_REQUEST_CA;
//		sButInit.y		+=	22;
//		sButInit.UserData	= '*';			
//		sButInit.pTip		= "Tech:all";
		
//		widgAddButton(psRScreen, &sButInit);

	}

}

void closeMultiRequester()
{
	widgDelete(psRScreen,M_REQUEST);
	multiRequestUp = FALSE;

	widgReleaseScreen(psRScreen);		// move this to the frontend shutdown...
	return;
}

BOOL runMultiRequester(UDWORD id,UDWORD *mode, STRING *chosen,UDWORD *chosenValue)
{
	if( id==M_REQUEST_CLOSE)							// close
	{
		closeMultiRequester();
		return TRUE;
	}

	if( id>=M_REQUEST_BUT && id<=M_REQUEST_BUTM)		// chose a file.
	{
		strcpy(chosen,((W_BUTTON *)widgGetFromID(psRScreen,id))->pText );

//		if(context == MULTIOP_MAP)						// chop off the number of players.
//		{
//			strcpy(chosen, strrchr(chosen,')')+1  );	
//		}

		*chosenValue = (UDWORD) ((W_BUTTON *)widgGetFromID(psRScreen,id))->pUserData ; 
		closeMultiRequester();
		*mode = context;
		return TRUE;
	}

	if( id == M_REQUEST_C1)
	{
		closeMultiRequester();
		addMultiRequest("\\multiplay\\customMaps\\*.wrf",MULTIOP_MAP,1);
	}
	if( id == M_REQUEST_C2)
	{
		closeMultiRequester();
		addMultiRequest("\\multiplay\\customMaps\\*.wrf",MULTIOP_MAP,2);
	}
	if( id == M_REQUEST_C3)
	{
		closeMultiRequester();
		addMultiRequest("\\multiplay\\customMaps\\*.wrf",MULTIOP_MAP,3);
	}
//	if( id == M_REQUEST_CA)
//	{
//		closeMultiRequester();
//		addMultiRequest("\\multiplay\\customMaps\\*.wrf",MULTIOP_MAP,0);
//	}

	return FALSE;
}


// ////////////////////////////////////////////////////////////////////////////
// Multiplayer in game menu stuff

// ////////////////////////////////////////////////////////////////////////////
// Display Functions 


void displayExtraGubbins(UDWORD height)
{
	char	str[128];

	// draw timer
	getAsciiTime(str,gameTime);
	iV_DrawText(str, MULTIMENU_FORM_X+MULTIMENU_C2 ,MULTIMENU_FORM_Y+MULTIMENU_FONT_OSET) ;

	//draw grid
	iV_Line(MULTIMENU_FORM_X+MULTIMENU_C0 -6 , MULTIMENU_FORM_Y,
			MULTIMENU_FORM_X+MULTIMENU_C0 -6 , MULTIMENU_FORM_Y+height, iV_COL_BLACK);

	iV_Line(MULTIMENU_FORM_X+MULTIMENU_C8 -6 , MULTIMENU_FORM_Y,
			MULTIMENU_FORM_X+MULTIMENU_C8 -6 , MULTIMENU_FORM_Y+height, iV_COL_BLACK);
	
	iV_Line(MULTIMENU_FORM_X+MULTIMENU_C9 -6 , MULTIMENU_FORM_Y,
			MULTIMENU_FORM_X+MULTIMENU_C9 -6 , MULTIMENU_FORM_Y+height, iV_COL_BLACK);

	iV_Line(MULTIMENU_FORM_X+MULTIMENU_C10 -6 , MULTIMENU_FORM_Y,
			MULTIMENU_FORM_X+MULTIMENU_C10 -6 , MULTIMENU_FORM_Y+height, iV_COL_BLACK);

	iV_Line(MULTIMENU_FORM_X+MULTIMENU_C11 -6 , MULTIMENU_FORM_Y,
			MULTIMENU_FORM_X+MULTIMENU_C11 -6 , MULTIMENU_FORM_Y+height, iV_COL_BLACK);

	iV_Line(MULTIMENU_FORM_X				, MULTIMENU_FORM_Y+MULTIMENU_PLAYER_H,
			MULTIMENU_FORM_X+MULTIMENU_FORM_W, MULTIMENU_FORM_Y+MULTIMENU_PLAYER_H, iV_COL_BLACK);
				  
	//draw titles.
	iV_SetFont(WFont);											// font
	iV_SetTextColour(-1);										//colour

	iV_DrawText((UCHAR*)strresGetString(psStringRes, STR_MUL_ALLIANCES),MULTIMENU_FORM_X+MULTIMENU_C0,MULTIMENU_FORM_Y+MULTIMENU_FONT_OSET);
	iV_DrawText((UCHAR*)strresGetString(psStringRes, STR_MUL_SCORE)	   ,MULTIMENU_FORM_X+MULTIMENU_C8,MULTIMENU_FORM_Y+MULTIMENU_FONT_OSET);
	iV_DrawText((UCHAR*)strresGetString(psStringRes, STR_MUL_KILLS)    ,MULTIMENU_FORM_X+MULTIMENU_C9,MULTIMENU_FORM_Y+MULTIMENU_FONT_OSET);
	iV_DrawText((UCHAR*)strresGetString(psStringRes, STR_MUL_PING)     ,MULTIMENU_FORM_X+MULTIMENU_C10,MULTIMENU_FORM_Y+MULTIMENU_FONT_OSET);	
	iV_DrawText((UCHAR*)strresGetString(psStringRes,STR_MUL_PLAY)     ,									MULTIMENU_FORM_X+MULTIMENU_C11,MULTIMENU_FORM_Y+MULTIMENU_FONT_OSET);

#ifdef DEBUG
	sprintf(str,"Traf:%d/%d",NETgetBytesSent(),NETgetBytesRecvd());
	iV_DrawText((UCHAR*)str,MULTIMENU_FORM_X,MULTIMENU_FORM_Y+MULTIMENU_FORM_H);

	sprintf(str,"Pack:%d/%d",NETgetPacketsSent(), NETgetPacketsRecvd());
	iV_DrawText((UCHAR*)str,MULTIMENU_FORM_X+80,MULTIMENU_FORM_Y+MULTIMENU_FORM_H);
#endif

	return;
}

void displayMultiPlayer(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	char			str[128];
	UDWORD			x					= xOffset+psWidget->x;
	UDWORD			y					= yOffset+psWidget->y;
	UDWORD			player;//,pl2;
	iVector			Rotation,Position;

	UNUSEDPARAMETER(pColours);

	player = (int)psWidget->pUserData;	//get the in game player number.


	if( responsibleFor(player,0) )
	{
		displayExtraGubbins(widgGetFromID(psWScreen,MULTIMENU_FORM)->height);
	}

					
	iV_SetFont(WFont);											// font
	iV_SetTextColour(-1);										//colour

	if(isHumanPlayer(player) || (game.type == SKIRMISH && player<game.maxPlayers) )
	{	
		//c2:name, 
		
		sprintf(str,"%d:",player,getPlayerName(player) );
		
		strcat(str, getPlayerName(player) );
		while(iV_GetTextWidth(str) >= (MULTIMENU_C0-MULTIMENU_C2-10) )
		{
			str[strlen(str)-1]='\0';
		}
		iV_DrawText((UCHAR*)str, x+MULTIMENU_C2, y+MULTIMENU_FONT_OSET);

		//c3-7 alliance
		//manage buttons by showing or hiding them. gifts only in campaign,
//		if(game.type != DMATCH)
		{
			if(game.alliance != NO_ALLIANCES)
			{
				if(alliances[selectedPlayer][player] == ALLIANCE_FORMED)
				{
					if(player != selectedPlayer &&  !giftsUp[player] )
					{
						widgReveal(psWScreen,MULTIMENU_GIFT_RAD+ player);
						widgReveal(psWScreen,MULTIMENU_GIFT_RES+ player);
						widgReveal(psWScreen,MULTIMENU_GIFT_DRO+ player);
						widgReveal(psWScreen,MULTIMENU_GIFT_POW+ player);
						giftsUp[player] = TRUE;
					}
				}
				else
				{
					if(player != selectedPlayer && giftsUp[player])
					{
						widgHide(psWScreen,MULTIMENU_GIFT_RAD+ player);
						widgHide(psWScreen,MULTIMENU_GIFT_RES+ player);
						widgHide(psWScreen,MULTIMENU_GIFT_DRO+ player);
						widgHide(psWScreen,MULTIMENU_GIFT_POW+ player);
						giftsUp[player] = FALSE;
					}
				}
			}		
		}
	}	
	if(isHumanPlayer(player))
	{
		//c8:score,
		sprintf(str,"%d",getMultiStats(player,TRUE).recentScore);
		iV_DrawText((UCHAR*)str,x+MULTIMENU_C8,y+MULTIMENU_FONT_OSET);
		
		//c9:kills,
//		if(game.type == DMATCH)
//		{
//			sprintf(str,"%d",getMultiStats(player,TRUE).recentKills);
//		}
//		else
//		{
			sprintf(str,"%d",getMultiStats(player,TRUE).recentKills);
//		}
		iV_DrawText((UCHAR*)str,x+MULTIMENU_C9,y+MULTIMENU_FONT_OSET);

		//c10:ping
		if(player != selectedPlayer)
		{
			if(ingame.PingTimes[player] >2000)
			{
				sprintf(str,"***");
			}
			else
			{
				sprintf(str,"%d",ingame.PingTimes[player]);				
			}
			iV_DrawText((UCHAR*)str,x+MULTIMENU_C10,y+MULTIMENU_FONT_OSET);
		}

		//c11:played
		sprintf(str,"%d",getMultiStats(player,TRUE).played);		
		iV_DrawText((UCHAR*)str,x+MULTIMENU_C11,y+MULTIMENU_FONT_OSET);
	}
	else
	{
		// estimate of score.
		sprintf(str,"%d",ingame.skScores[player][0]);
		iV_DrawText((UCHAR*)str,x+MULTIMENU_C8,y+MULTIMENU_FONT_OSET);
		// estimated kills
		sprintf(str,"%d",ingame.skScores[player][1]);
		iV_DrawText((UCHAR*)str,x+MULTIMENU_C9,y+MULTIMENU_FONT_OSET);

	}

	// a droid of theirs.
	if(apsDroidLists[player])
	{
		pie_SetGeometricOffset( MULTIMENU_FORM_X+MULTIMENU_C1 ,y+MULTIMENU_PLAYER_H);
		Rotation.x = -15;
		Rotation.y = 45;
		Rotation.z = 0;
		Position.x = 0;
		Position.y = 0;
		Position.z = 2000;		//scale them!

		displayComponentButtonObject(apsDroidLists[player],&Rotation,&Position,FALSE, 100);
	}

	// clean up widgets if player leaves while menu is up.
	if(!isHumanPlayer(player) && !(game.type == SKIRMISH && player<game.maxPlayers))
	{	
		if(widgGetFromID(psWScreen,MULTIMENU_CHANNEL+player))
		{
			widgDelete(psWScreen,MULTIMENU_CHANNEL+ player);
		}

		if(widgGetFromID(psWScreen,MULTIMENU_ALLIANCE_BASE+player) )
		{
			widgDelete(psWScreen,MULTIMENU_ALLIANCE_BASE+ player);
			widgDelete(psWScreen,MULTIMENU_GIFT_RAD+ player);
			widgDelete(psWScreen,MULTIMENU_GIFT_RES+ player);
			widgDelete(psWScreen,MULTIMENU_GIFT_DRO+ player);
			widgDelete(psWScreen,MULTIMENU_GIFT_POW+ player);
			giftsUp[player] = FALSE;
		}
	}
}


// ////////////////////////////////////////////////////////////////////////////
// alliance display funcs

void displayAllianceState(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	UDWORD a,b,c,player;

	player = (UDWORD) psWidget->pUserData;
	switch(alliances[selectedPlayer][player])
	{
	case ALLIANCE_BROKEN:
		a = 0;
		b = IMAGE_MULTI_NOAL_HI;
		c = IMAGE_MULTI_NOAL;					// replace with real gfx
		break;
	case ALLIANCE_FORMED:
		a = 0;
		b = IMAGE_MULTI_AL_HI;
		c = IMAGE_MULTI_AL;						// replace with real gfx
		break;
	case ALLIANCE_REQUESTED:
	case ALLIANCE_INVITATION:
		a = 0;
		b = IMAGE_MULTI_OFFAL_HI;
		c = IMAGE_MULTI_OFFAL;						// replace with real gfx
		break;
	default:	
		a = 0;
		b = IMAGE_MULTI_NOAL_HI;
		c = IMAGE_MULTI_NOAL;	
		break;
	}

	psWidget->pUserData = (VOID *)PACKDWORD_TRI(a,b,c);
	intDisplayImageHilight(psWidget,  xOffset,  yOffset, pColours);
	psWidget->pUserData = (VOID *)player;

}


void displayChannelState(struct _widget *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	UDWORD a,b,c,player;

	player = (UDWORD) psWidget->pUserData;
	switch(openchannels[player])
	{
	case 1:
		a = 0;
		b = IMAGE_MULTI_CHAN;
		c = IMAGE_MULTI_CHAN;
		break;
	case 0:
	default:
		a = 0;
		b = IMAGE_MULTI_NOCHAN;
		c = IMAGE_MULTI_NOCHAN;						
		break;
	}

	psWidget->pUserData = (VOID *)PACKDWORD_TRI(a,b,c);
	intDisplayImageHilight(psWidget,  xOffset,  yOffset, pColours);
	psWidget->pUserData = (VOID *)player;
}


// ////////////////////////////////////////////////////////////////////////////

void addMultiPlayer(UDWORD player,UDWORD pos)
{
	UDWORD			y,id;
	W_BUTINIT		sButInit;
	W_FORMINIT		sFormInit;
	y	= MULTIMENU_PLAYER_H*(pos+1);					// this is the top of the pos.
	id	= MULTIMENU_PLAYER+player;

	// add the whole thing.
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID		  = MULTIMENU_FORM;
	sFormInit.id			  = id;
	sFormInit.style			  = WFORM_PLAIN;
	sFormInit.x				  = 2;
	sFormInit.y				  = (short)y;
	sFormInit.width			  = MULTIMENU_FORM_W -4;
	sFormInit.height		  = MULTIMENU_PLAYER_H;
	sFormInit.pDisplay		  = displayMultiPlayer;
	sFormInit.pUserData		  = (VOID *)player;
	widgAddForm(psWScreen, &sFormInit);

	//name, 
	//score,
	//kills,
	//ping
	//ALL DONE IN THE DISPLAY FUNC.

	// add channel opener.
	if(player != selectedPlayer)
	{
		memset(&sButInit, 0, sizeof(W_BUTINIT));
		sButInit.formID = id; 
		sButInit.style	= WBUT_PLAIN;
		sButInit.x		= MULTIMENU_C0;
		sButInit.y		= 5;
		sButInit.width	= 35;
		sButInit.height = 24;
		sButInit.FontID = WFont;
		sButInit.id		= MULTIMENU_CHANNEL + player;
		sButInit.pTip	= "channel";
		sButInit.pDisplay = displayChannelState; 
		sButInit.pUserData = (VOID*)player;
		widgAddButton(psWScreen, &sButInit);
	}

	if(game.alliance!=NO_ALLIANCES && player!=selectedPlayer)
	{		
		//alliance
		sButInit.x		= MULTIMENU_C3;
		sButInit.y		= 5;
		sButInit.width	= 35;
		sButInit.height = 24;
		sButInit.FontID = WFont;
		sButInit.id		= MULTIMENU_ALLIANCE_BASE + player;
		sButInit.pTip	= strresGetString(psStringRes,STR_ALLI_STATE);
		sButInit.pDisplay = displayAllianceState; 
		sButInit.pUserData = (VOID*)player;

		widgAddButton(psWScreen, &sButInit);

		sButInit.pDisplay = intDisplayImageHilight;

//		if(game.type != DMATCH)
//		{
			// add the gift buttons.
			sButInit.y		+= 1;	// move down a wee bit.

			sButInit.id		= MULTIMENU_GIFT_RAD+ player;
			sButInit.x		= MULTIMENU_C4;
			sButInit.pTip	= strresGetString(psStringRes,STR_ALLI_VIS);
			sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_MULTI_VIS_HI, IMAGE_MULTI_VIS);
			widgAddButton(psWScreen, &sButInit);

			sButInit.id		= MULTIMENU_GIFT_RES + player;
			sButInit.x		= MULTIMENU_C5;
			sButInit.pTip	= strresGetString(psStringRes,STR_ALLI_TEC);
			sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_MULTI_TEK_HI , IMAGE_MULTI_TEK);
			widgAddButton(psWScreen, &sButInit);

			sButInit.id		= MULTIMENU_GIFT_DRO + player;
			sButInit.x		= MULTIMENU_C6;
			sButInit.pTip	= strresGetString(psStringRes,STR_ALLI_DRO);
			sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_MULTI_DRO_HI , IMAGE_MULTI_DRO);
			widgAddButton(psWScreen, &sButInit);

			sButInit.id		= MULTIMENU_GIFT_POW + player;
			sButInit.x		= MULTIMENU_C7;
			sButInit.pTip	= strresGetString(psStringRes,STR_ALLI_POW);
			sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_MULTI_POW_HI , IMAGE_MULTI_POW);
			widgAddButton(psWScreen, &sButInit);
	
			giftsUp[player] = TRUE;				// note buttons are up!
//		}
		
	}		
}



BOOL intAddMultiMenu(VOID)
{
	W_FORMINIT		sFormInit;
	W_BUTINIT		sButInit;
	UDWORD			i,pos = 0,formHeight=0;

	//check for already open.
	if(widgGetFromID(psWScreen,MULTIMENU_FORM))
	{
		intCloseMultiMenu();
		return TRUE;
	}

	intResetScreen(FALSE);


	// calculate required height.
	formHeight = MULTIMENU_PLAYER_H+7;
	for(i=0;i<MAX_PLAYERS;i++)
	{
//		if(isHumanPlayer(i) || (game.type == SKIRMISH && i<game.maxPlayers && game.skirmishPlayers[i] ))
		if(isHumanPlayer(i) || (game.type == SKIRMISH && i<game.maxPlayers && game.skDiff[i] ))
		{
			formHeight += MULTIMENU_PLAYER_H;
		}
	}

	// add form
	memset(&sFormInit, 0, sizeof(W_FORMINIT));
	sFormInit.formID		  = 0;
	sFormInit.id			  = MULTIMENU_FORM;
	sFormInit.style			  = WFORM_PLAIN;
	sFormInit.x				  =(SWORD)(MULTIMENU_FORM_X);
	sFormInit.y				  =(SWORD)(MULTIMENU_FORM_Y);
	sFormInit.width			  = MULTIMENU_FORM_W;
	sFormInit.height		  = (UWORD)formHeight;			//MULTIMENU_FORM_H;
	sFormInit.pDisplay		  = intOpenPlainForm;
	sFormInit.disableChildren = TRUE;

	if (!widgAddForm(psWScreen, &sFormInit))
	{
		return FALSE;
	}

	// add any players
	for(i=0;i<MAX_PLAYERS;i++)
	{
//		if(isHumanPlayer(i) || (game.type == SKIRMISH && i<game.maxPlayers && game.skirmishPlayers[i] ) )
		if(isHumanPlayer(i) || (game.type == SKIRMISH && i<game.maxPlayers && game.skDiff[i] ) )
		{
			addMultiPlayer(i,pos);
			pos++;
		}
	}
	
	// Add the close button.
	memset(&sButInit, 0, sizeof(W_BUTINIT));
	sButInit.formID = MULTIMENU_FORM;
	sButInit.id = MULTIMENU_CLOSE;
	sButInit.style = WBUT_PLAIN;
	sButInit.x = MULTIMENU_FORM_W - CLOSE_WIDTH;
	sButInit.y = 0;
	sButInit.width = CLOSE_WIDTH;
	sButInit.height = CLOSE_HEIGHT;
	sButInit.pTip = strresGetString(psStringRes, STR_MISC_CLOSE);
	sButInit.FontID = WFont;
	sButInit.pDisplay = intDisplayImageHilight;
	sButInit.pUserData = (void*)PACKDWORD_TRI(0,IMAGE_CLOSEHILIGHT , IMAGE_CLOSE);
	if (!widgAddButton(psWScreen, &sButInit))
	{
		return FALSE;
	}

	intShowPowerBar();						// add power bar

	intMode		= INT_MULTIMENU;			// change interface mode.
	MultiMenuUp = TRUE;
	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
VOID intCloseMultiMenuNoAnim(VOID)
{
	widgDelete(psWScreen, MULTIMENU_CLOSE);
	widgDelete(psWScreen, MULTIMENU_FORM);
	MultiMenuUp = FALSE;
	intMode		= INT_NORMAL;
}


// ////////////////////////////////////////////////////////////////////////////
BOOL intCloseMultiMenu(VOID)
{
	W_TABFORM *Form;

	widgDelete(psWScreen, MULTIMENU_CLOSE);

	// Start the window close animation.
	Form = (W_TABFORM*)widgGetFromID(psWScreen,MULTIMENU_FORM);
	if(Form) {
		Form->display = intClosePlainForm;
		Form->pUserData = (void*)0;	// Used to signal when the close anim has finished.
		Form->disableChildren = TRUE;
		ClosingMultiMenu = TRUE;
		MultiMenuUp  = FALSE;
	}
//intCloseMultiMenuNoAnim();

	intMode		= INT_NORMAL;
	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
// In Game Options house keeping stuff.
BOOL intRunMultiMenu(VOID)
{	
	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////

BOOL intCheckAllianceValid( UBYTE player1, UBYTE player2 )
{
	UBYTE	i, iAlliances, iHumanPlayers;

	/* only interested in teamplay */
	if ( bMultiPlayer && game.type != TEAMPLAY )
	{
		return TRUE;
	}

	/* throw out currently allied or human/computer alliances */
	if ( (player1 == player2) ||
		 aiCheckAlliances( player1, player2 ) ||
		 !(isHumanPlayer(player1) && isHumanPlayer(player2)) )
	{
		return FALSE;
	}

	/* get num human players */
	iHumanPlayers = 0;
	for( i=0;i<MAX_PLAYERS;i++ )
	{
		if ( isHumanPlayer(i) )
		{
			iHumanPlayers++;
		}
	}

	/* count number of current alliances */
	iAlliances = 0;
	for(i=0;i<MAX_PLAYERS;i++)
	{
		if ( isHumanPlayer(i) )
		{
			if ( (i != player1) && aiCheckAlliances( i, player1 ) ||
				 (i != player2) && aiCheckAlliances( i, player2 )    )
			{
				iAlliances++;
			}
		}
	}

	/* return FALSE if total alliances excedds max */
	if ( iAlliances >= iHumanPlayers-2 )
	{
		return FALSE;
	}

	return TRUE;
}

// ////////////////////////////////////////////////////////////////////////////
// process clicks made by user.
VOID intProcessMultiMenu(UDWORD id)
{	
	UBYTE	i;

	//close
	if (id == MULTIMENU_CLOSE)
	{
		intCloseMultiMenu();
	}

	//alliance button
	if(id >=MULTIMENU_ALLIANCE_BASE  &&  id<MULTIMENU_ALLIANCE_BASE+MAX_PLAYERS)
	{
		i =(UBYTE)( id - MULTIMENU_ALLIANCE_BASE);

		switch(alliances[selectedPlayer][i])
		{
		case ALLIANCE_BROKEN:
			if ( intCheckAllianceValid( (UBYTE)selectedPlayer, i ) )
			{
				requestAlliance((UBYTE)selectedPlayer,i,TRUE,TRUE);			// request an alliance
			}
			break;
		case ALLIANCE_INVITATION:
			if ( intCheckAllianceValid( (UBYTE)selectedPlayer, i ) )
			{
				formAlliance((UBYTE)selectedPlayer,i,TRUE,TRUE);			// form an alliance
			}
			break;
		case ALLIANCE_REQUESTED:	
			breakAlliance((UBYTE)selectedPlayer,i,TRUE,TRUE);		// break an alliance
			break;	

		case ALLIANCE_FORMED:
			if(game.type != TEAMPLAY)									// cant break state in teamplay..
			{
				breakAlliance((UBYTE)selectedPlayer,i,TRUE,TRUE);		// break an alliance
			}
			break;	
		default:
			break;
		}	
	}


	//channel opens.
	if(id >=MULTIMENU_CHANNEL &&  id<MULTIMENU_CHANNEL+MAX_PLAYERS)
	{
		i =(UBYTE)( id - MULTIMENU_CHANNEL);
		if(openchannels[i])
		{
			openchannels[i] = FALSE;// close channel
		}
		else
		{	
			openchannels[i] = TRUE;// open channel
		}
	}

	//radar gifts
	if(id >=  MULTIMENU_GIFT_RAD && id< MULTIMENU_GIFT_RAD +MAX_PLAYERS)
	{
		sendGift(RADAR_GIFT, id - MULTIMENU_GIFT_RAD);
	}
	
	// research gift
	if(id >= MULTIMENU_GIFT_RES && id<MULTIMENU_GIFT_RES  +MAX_PLAYERS)
	{
		sendGift(RESEARCH_GIFT, id - MULTIMENU_GIFT_RES);
	}
	
	//droid gift
	if(id >=  MULTIMENU_GIFT_DRO && id<  MULTIMENU_GIFT_DRO +MAX_PLAYERS)
	{
		sendGift(DROID_GIFT, id - MULTIMENU_GIFT_DRO);	
	}

	//power gift
	if(id >=  MULTIMENU_GIFT_POW && id<  MULTIMENU_GIFT_POW +MAX_PLAYERS)
	{
		sendGift(POWER_GIFT, id - MULTIMENU_GIFT_POW);	
	}


}


//////////////////////////////////////////////////////////////////////////////
/*
void intDisplayMiniMultiMenu(void)
{
	SDWORD sc[MAX_PLAYERS];
	SDWORD scp[MAX_PLAYERS];

	UDWORD j,i,temp;

	UDWORD x = RADTLX;
	UDWORD y = RADTLY - 60;
	UDWORD w = RADWIDTH;
	UDWORD h = 50;
	UDWORD players = 0;
	STRING	sTmp[64];

	if( ingame.localJoiningInProgress)
	{
		RenderWindowFrame(&FrameNormal,x, y ,w, h);			// draw a wee blu box.

		// display how far done..
		sprintf(sTmp,"%d%%", PERCENT(arenaPlayersReceived,MAX_PLAYERS) );
		iV_DrawText((UCHAR*)sTmp ,x+(w/2)-10,y+(h/2)+3 );
	}
	else
	{
		for(i=0;i<MAX_PLAYERS;i++)							// clear out...
		{
			sc[i] = -1;
			scp[i] = -1;
		}

		for(j=0;j<MAX_PLAYERS;j++)							// find biggest score.
		{
			if(isHumanPlayer(j) )
			{
				players++;
				sc[j]  = getMultiStats(j,TRUE).recentScore;
				scp[j] = j;
			}
		}

		if(players <3)										// shrink box to fit.
		{
			y = RADTLY- (players*13) -18 ;
			h = players*13 + 8 ;
		}

		RenderWindowFrame(&FrameNormal,x, y ,w, h);			// draw a wee blu box.

		for(i=0;i<MAX_PLAYERS;++i)							// bubble sort.
		{
			for(j=MAX_PLAYERS-1;j>i;--j)
			{
				if (sc[j-1] > sc[j])
				{
					temp	= sc[j];
					sc[j]	= sc[j-1];
					sc[j-1] = temp;

					temp	= scp[j];
					scp[j]	= scp[j-1];
					scp[j-1] = temp;						// swap players too.
				}
			}
		}

		for(j=0;(scp[j] != (SDWORD)selectedPlayer) && (j<MAX_PLAYERS);j++);		// rate ourselves.

		iV_DrawText("1",x+5,y+13);							// display stuff
		strcpy(sTmp,getPlayerName(scp[7]));
		while(iV_GetTextWidth(sTmp) >= RADWIDTH-20 )
		{
			sTmp[strlen(sTmp)-1]='\0';
		}
		iV_DrawText((UCHAR*)sTmp,x+16,y+13);

		if(players >1)
		{
			iV_DrawText("2",x+5,y+26);
			strcpy(sTmp,getPlayerName(scp[6]));
			while(iV_GetTextWidth(sTmp) >= RADWIDTH-20 )
			{
				sTmp[strlen(sTmp)-1]='\0';
			}
			iV_DrawText((UCHAR*)sTmp ,x+16,y+26);
		}

		if(players >2)
		{
			if(j!=7 && j!=6)
			{
				sprintf(sTmp,"%d",8-j);
				iV_DrawText((UCHAR*)sTmp,x+5,y+39);

				strcpy(sTmp,getPlayerName(scp[selectedPlayer]));
				while(iV_GetTextWidth(sTmp) >= RADWIDTH-20 )
				{
					sTmp[strlen(sTmp)-1]='\0';
				}
				iV_DrawText((UCHAR*) sTmp,x+16,y+39);
			}
			else
			{
				iV_DrawText("3",x+5,y+39);
				strcpy(sTmp,getPlayerName(scp[5]));
				while(iV_GetTextWidth(sTmp) >= RADWIDTH-20 )
				{
					sTmp[strlen(sTmp)-1]='\0';
				}
				iV_DrawText((UCHAR*)sTmp ,x+16,y+39);
			}
		}
	}
	return;
}
*/