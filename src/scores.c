/* 
	Scores.c Deals with all the mission results gubbins.
	Alex W. McLean
*/

// --------------------------------------------------------------------
#include "Frame.h"
#include "GTime.h"
#include "Console.h"
#include "Scores.h"
#include "pieDef.h"
#include "pieFunc.h"
#include "pieMode.h"
#include "pieState.h"
#include "RendMode.h"
#include "Objects.h"
#include "DroidDef.h"
#include "Base.h"
#include "StatsDef.h"
#include "HCI.h"
#include "Text.h"
#include "MiscImd.h"
#include "Geo.h"
#include "Display3d.h"
#include "Mission.h"
#include "Game.h"
#include "Audio.h"
#include "Audio_id.h"
#include "IntImage.h"

#define	BAR_CRAWL_TIME	(GAME_TICKS_PER_SEC*3)

#define	MT_X_POS	(MISSIONRES_TITLE_X  + D_W + 140)
#define MT_Y_POS	(MISSIONRES_TITLE_Y  + D_H + 80)

#define DROID_LEVELS	9
#define MAX_BAR_LENGTH	100
#define LC_UPPER	100


#define LC_X	32
#define RC_X	320+32
#define	RANK_BAR_WIDTH	100
#define STAT_BAR_WIDTH	100
STAT_BAR	infoBars[]=
{
	{LC_X,100,STAT_BAR_WIDTH,16,10,STR_MR_UNITS_LOST,0,FALSE,TRUE,0,165},	// left column		STAT_UNIT_LOST       
	{LC_X,120,STAT_BAR_WIDTH,16,20,STR_MR_UNITS_KILLED,0,FALSE,TRUE,0,81},			 //	STAT_UNIT_KILLED         
	{LC_X,160,STAT_BAR_WIDTH,16,30,STR_MR_STR_LOST,0,FALSE,TRUE,0,165},				 //	STAT_STR_LOST            
	{LC_X,180,STAT_BAR_WIDTH,16,40,STR_MR_STR_BLOWN_UP,0,FALSE,TRUE,0,81}, 			 //	STAT_STR_BLOWN_UP        
	{LC_X,220,STAT_BAR_WIDTH,16,50,STR_MR_UNITS_BUILT,0,FALSE,TRUE,0,185}, 			 //	STAT_UNITS_BUILT         
	{LC_X,240,STAT_BAR_WIDTH,16,60,STR_MR_UNITS_NOW,0,FALSE,TRUE,0,185}, 				 //	STAT_UNITS_NOW           
	{LC_X,260,STAT_BAR_WIDTH,16,70,STR_MR_STR_BUILT,0,FALSE,TRUE,0,185}, 				 //	STAT_STR_BUILT           
	{LC_X,280,STAT_BAR_WIDTH,16,80,STR_MR_STR_NOW,0,FALSE,FALSE,0,185}, 				 //	STAT_STR_NOW             
																		                             
	{RC_X,100,RANK_BAR_WIDTH,16,10,STR_MR_LEVEL_ROOKIE,0,FALSE,TRUE,0,117},	// right column	//	STAT_ROOKIE      
	{RC_X,120,RANK_BAR_WIDTH,16,20,STR_MR_LEVEL_GREEN,0,FALSE,TRUE,0,117}, 			 		//	STAT_GREEN       
	{RC_X,140,RANK_BAR_WIDTH,16,30,STR_MR_LEVEL_TRAINED,0,FALSE,TRUE,0,117}, 		 			//	STAT_TRAINED 
	{RC_X,160,RANK_BAR_WIDTH,16,40,STR_MR_LEVEL_REGULAR,0,FALSE,TRUE,0,117}, 		 			//	STAT_REGULAR 
	{RC_X,180,RANK_BAR_WIDTH,16,50,STR_MR_LEVEL_VETERAN,0,FALSE,TRUE,0,117}, 		 			//	STAT_VETERAN 
	{RC_X,200,RANK_BAR_WIDTH,16,60,STR_MR_LEVEL_CRACK,0,FALSE,TRUE,0,117}, 			 		//	STAT_CRACK       
	{RC_X,220,RANK_BAR_WIDTH,16,70,STR_MR_LEVEL_ELITE,0,FALSE,TRUE,0,117}, 			 		//	STAT_ELITE       
	{RC_X,240,RANK_BAR_WIDTH,16,80,STR_MR_LEVEL_SPECIAL,0,FALSE,TRUE,0,117}, 		 			//	STAT_SPECIAL 
	{RC_X,260,RANK_BAR_WIDTH,16,90,STR_MR_LEVEL_ACE,0,FALSE,TRUE,0,117}, 			 			//	STAT_ACE     

	{0,0,0,0,0,0,0}
};

// --------------------------------------------------------------------
void	constructTime(STRING *psText, UDWORD hours, UDWORD minutes, UDWORD seconds);
void	drawDroidBars( void );
void	drawUnitBars( void );
void	drawStatBars( void );
void	fillUpStats( void );
void	dispAdditionalInfo( void );
// --------------------------------------------------------------------

/* The present mission data */
static	MISSION_DATA	missionData;
static	UDWORD	numUnits;
static	UDWORD	numStrs;
static	UDWORD	dispST;
static	BOOL	bDispStarted = FALSE;
static	char	text[255];
static	char	text2[255];


// --------------------------------------------------------------------
/* Initialise the mission data info - done before each mission */
BOOL	scoreInitSystem( void )
{
	missionData.unitsBuilt		= 0;
	missionData.unitsKilled		= 0;
	missionData.unitsLost		= 0;

	missionData.strBuilt		= 0;
	missionData.strKilled		= 0;
	missionData.strLost			= 0;

	missionData.artefactsFound	= 0;
	missionData.missionStarted	= gameTime; // total game time is just gameTime
	missionData.shotsOnTarget	= 0;
	missionData.shotsOffTarget	= 0;
	missionData.babasMowedDown	= 0;
	bDispStarted = FALSE;
	return(TRUE);
}

// --------------------------------------------------------------------
// Updates a game statistic - more can be added if we need 'em
void	scoreUpdateVar( DATA_INDEX var )
{
	switch(var)
	{
	case	WD_UNITS_BUILT:
		missionData.unitsBuilt++;	// We've built another unit
		break;
	case	WD_UNITS_KILLED:
		missionData.unitsKilled++;	// We've destroyed an enemy unit
		break;
	case	WD_UNITS_LOST:
		missionData.unitsLost++;	// We've lost a unit
		break;
	case	WD_STR_BUILT:
		missionData.strBuilt++;		// Built a structure
		break;
	case	WD_STR_KILLED:
		missionData.strKilled++;	// Destroyed an enemy structure
		break;
	case	WD_STR_LOST:
		missionData.strLost++;		// Lost a structure
		break;
	case	WD_ARTEFACTS_FOUND:
		missionData.artefactsFound++;	// Got an artefact
		break;
	case	WD_MISSION_STARTED:
		missionData.missionStarted = gameTime;	// Init the mission start time
		break;									// Should be called once per mission
	case	WD_SHOTS_ON_TARGET:
		missionData.shotsOnTarget++;	// We hit something
		break;
	case	WD_SHOTS_OFF_TARGET:
		missionData.shotsOffTarget++;	// Missed something
		break;
	case	WD_BARBARIANS_MOWED_DOWN:
		missionData.babasMowedDown++;	// Ran over a barbarian
		break;
	default:
		DBERROR(("Weirdy variable request from scoreUpdateVar"));
		break;
	}
}


// --------------------------------------------------------------------
void	scoreDataToScreen(void)
{
	drawStatBars();
}

// --------------------------------------------------------------------
/* Builds an ascii string for the passed in components 04:02:23 for example */
void	constructTime(STRING *psText, UDWORD hours, UDWORD minutes, UDWORD seconds)
{
UDWORD	index;
UDWORD	div;

	index = 0;
	// Hours do not have trailing zeros
	if(hours!=0)
	{
   		if(hours<10)
		{
			// Less than 10 hours
			psText[index++] = (UBYTE)('0'+ hours%10);
		}
		else if(hours<100)
		{
			// Over ten hours
			psText[index++] = (UBYTE)('0'+ hours/10);
			psText[index++] = (UBYTE)('0'+ hours%10);
		}
		else
		{
			// Over 100 hours - go outside people!!!!
			// build hours
			psText[index++] = (UBYTE)('0' + (hours/100));		// hmmmmmm....
			div = hours/100;
			psText[index++] = (UBYTE)('0' + (hours-(div*100))/10);	// nice
			psText[index++] = (UBYTE)('0' + hours%10);

		}
	   	// Put in the hrs/mins separator - only for non-zero hours
		psText[index++] = (UBYTE)(':');
	}

   
	// put in the minutes
	psText[index++] = (UBYTE)('0' + minutes/10);
	psText[index++] = (UBYTE)('0' + minutes%10);

	// mins/secs separator
	psText[index++] = (UBYTE)(':');


	// Put in the seconds
	psText[index++] = (UBYTE)('0' + seconds/10);
	psText[index++] = (UBYTE)('0' + seconds%10);

	// terminate the string
	psText[index] = '\0';
}
// --------------------------------------------------------------------
/* Builds an ascii string for the passed in time */
void	getAsciiTime( STRING *psText, UDWORD time )
{
UDWORD	hours,minutes,seconds;

	getTimeComponents(time,&hours,&minutes,&seconds);
	constructTime(psText,hours,minutes,seconds);
}


// -----------------------------------------------------------------------------------
void	drawStatBars( void )
{
UDWORD	index;
BOOL	bMoreBars;
UDWORD	x,y;
UDWORD	width,height;
FRACT	length;
FRACT	mul;
UDWORD	div;

	if(!bDispStarted)
	{
		bDispStarted = TRUE;
		dispST = gameTime2;
		audio_PlayTrack(ID_SOUND_BUTTON_CLICK_5);
	}

	fillUpStats();

	pie_SetSwirlyBoxes(TRUE);
	pie_UniTransBoxFill(16+D_W,MT_Y_POS-16,DISP_WIDTH-D_W-16,MT_Y_POS+256,0x00000088,128);
	iV_Box(16+D_W,MT_Y_POS-16,DISP_WIDTH-D_W-16,MT_Y_POS+256,1);
	pie_SetSwirlyBoxes(FALSE);
				
	iV_DrawText(strresGetString(psStringRes,STR_MR_UNIT_LOSSES),LC_X+D_W,80+16+D_H);
	iV_DrawText(strresGetString(psStringRes,STR_MR_STRUCTURE_LOSSES),LC_X+D_W,140+16+D_H);
	iV_DrawText(strresGetString(psStringRes,STR_MR_FORCE_INFO),LC_X+D_W,200+16+D_H);


	index = 0;
	bMoreBars = TRUE;
	while(bMoreBars)
	{	
		/* Is it time to display this bar? */
		if( infoBars[index].bActive)
		{
			/* Has it been queued before? */
			if(infoBars[index].bQueued == FALSE)
			{
				/* Don't do this next time...! */
				infoBars[index].bQueued = TRUE;

				/* Play a sound */
//				audio_PlayTrack(ID_SOUND_BUTTON_CLICK_5);
			}
			x = infoBars[index].topX+D_W;
			y = infoBars[index].topY+D_H;
			width = infoBars[index].width;
			height = infoBars[index].height;

			iV_Box(x,y,x+width,y+height,0);

			/* Draw the background border box */
		 //	pie_UniTransBoxFill(x-1,y-1,x+width+1,y+height+1,0x00010101,255);
			iV_BoxFill(x-1,y-1,x+width+1,y+height+1,1);

			/* Draw the interior grey */
		  //	pie_UniTransBoxFill(x,y,x+width,y+height,0x00888888,96);
			iV_BoxFill(x,y,x+width,y+height,222);

			if( ((gameTime2 - dispST) > infoBars[index].queTime) )
			{
			   					/* Now draw amount filled */
				length = MAKEFRACT(infoBars[index].percent)/MAKEFRACT(100);
				length = length*MAKEFRACT(infoBars[index].width);
				div = PERCENT(gameTime2-dispST,BAR_CRAWL_TIME);
				if(div>100) div = 100;
				mul = MAKEFRACT(div)/100;
				length = length * mul;
		   	   	if(MAKEINT(length)>4)
				{

					/* Black shadow */
	//				pie_UniTransBoxFill(x+1,y+3,x+MAKEINT(length)-1,y+height-1,0x00010101,255);
					iV_BoxFill(x+1,y+3,x+MAKEINT(length)-1,y+height-1,1);
					/* Solid coloured bit */
	//				pie_UniTransBoxFill(x+1,y+2,x+MAKEINT(length)-3,y+height-3,0x00ffff00,255);
					iV_BoxFill(x+1,y+2,x+MAKEINT(length)-4,y+height-4,(UBYTE)infoBars[index].colour);
				}
			}
			/* Now render the text by the bar */
			sprintf(text,strresGetString(psStringRes,infoBars[index].stringID),
					infoBars[index].number);
			iV_DrawText(text, x + width + 16, y + 12);

			/* If we're beyond STAT_ROOKIE, then we're on rankings */
			if(index>=STAT_GREEN AND index <= STAT_ACE)
			{
				iV_DrawTransImage(IntImages,(UWORD)(IMAGE_LEV_0 + (index - STAT_GREEN)),x-8,y+2);
			}


		}
		/* Move onto the next bar */
		index++;
		if(infoBars[index].topX == 0 AND infoBars[index].topY == 0)
		{
			bMoreBars = FALSE;
		}
	}
	dispAdditionalInfo();
}
// -----------------------------------------------------------------------------------
void	dispAdditionalInfo( void )
{

	/* We now need to dsiplay the mission time, game time, 
		average unit experience level an number of artefacts found */

	/* Firstly, top of the screen, number of artefacts found */
	sprintf(text,strresGetString(psStringRes,STR_MR_ARTEFACTS_FOUND),missionData.artefactsFound);
	iV_DrawText(text,(DISP_WIDTH - iV_GetTextWidth(text))/2,300+D_H);

	/* Get the mission result time in a string - and write it out */
	getAsciiTime((char*)&text2,gameTime-missionData.missionStarted);
	sprintf(text,strresGetString(psStringRes,STR_MR_MISSION_TIME),text2);
	iV_DrawText(text,(DISP_WIDTH - iV_GetTextWidth(text))/2,320+D_H);	
  
	/* Write out total game time so far */
	getAsciiTime((char*)&text2,gameTime);
	sprintf(text,strresGetString(psStringRes,STR_MR_GAME_TIME),text2);
	iV_DrawText(text,(DISP_WIDTH - iV_GetTextWidth(text))/2,340+D_H);
}
// -----------------------------------------------------------------------------------
void	fillUpStats( void )
{
UDWORD	i;
UDWORD	maxi,num;
FRACT	scaleFactor;
UDWORD	length;
UDWORD	numUnits;
DROID	*psDroid;

	/* Do rankings first cos they're easier */
	for(i=0,maxi=0; i<DROID_LEVELS; i++)
	{
		num = getNumDroidsForLevel(i);
		if(num>maxi)
		{
			maxi = num;
		}
	}

	/* Make sure we got something */
	if(maxi == 0)
	{
		scaleFactor = MAKEFRACT(0);
	}
	else
	{
		scaleFactor = (MAKEFRACT(RANK_BAR_WIDTH)/maxi);
	}	

	/* Scale for percent */
	for(i=0; i<DROID_LEVELS; i++)
	{
		length = MAKEINT((scaleFactor*getNumDroidsForLevel(i)));
		infoBars[STAT_ROOKIE+i].percent = PERCENT(length,RANK_BAR_WIDTH);
		infoBars[STAT_ROOKIE+i].number = getNumDroidsForLevel(i);
	}

	/* Now do the other stuff... */
	/* Units killed and lost... */
	maxi = max(missionData.unitsLost,missionData.unitsKilled);
	if (maxi == 0)
	{
		scaleFactor = 0;
	}
	else
	{
		scaleFactor = (MAKEFRACT(STAT_BAR_WIDTH)/maxi);
	}

	length = MAKEINT(scaleFactor*missionData.unitsLost);
	infoBars[STAT_UNIT_LOST].percent = PERCENT(length,STAT_BAR_WIDTH);
	length = MAKEINT(scaleFactor*missionData.unitsKilled);
	infoBars[STAT_UNIT_KILLED].percent = PERCENT(length,STAT_BAR_WIDTH);

	/* Now do the structure losses */
	maxi = max(missionData.strLost,missionData.strKilled);
	if (maxi == 0)
	{
		scaleFactor = 0;
	}
	else
	{
		scaleFactor = (MAKEFRACT(STAT_BAR_WIDTH)/maxi);
	}

	length = MAKEINT(scaleFactor*missionData.strLost);
	infoBars[STAT_STR_LOST].percent = PERCENT(length,STAT_BAR_WIDTH);
	length = MAKEINT(scaleFactor*missionData.strKilled);
	infoBars[STAT_STR_BLOWN_UP].percent = PERCENT(length,STAT_BAR_WIDTH);

	/* Finally the force information - need amount of droids as well*/
	for(psDroid = apsDroidLists[selectedPlayer],numUnits = 0;
		psDroid; psDroid = psDroid->psNext,numUnits++);

	for(psDroid = mission.apsDroidLists[selectedPlayer];
		psDroid; psDroid = psDroid->psNext,numUnits++);


	maxi = max(missionData.unitsBuilt,missionData.strBuilt);
	maxi = max(maxi,numUnits);

	if (maxi == 0)
	{
		scaleFactor = 0;
	}
	else
	{
		scaleFactor = (MAKEFRACT(STAT_BAR_WIDTH)/maxi);
	}

	length = MAKEINT(scaleFactor*missionData.unitsBuilt);
	infoBars[STAT_UNITS_BUILT].percent = PERCENT(length,STAT_BAR_WIDTH);
	length = MAKEINT(scaleFactor*numUnits);
	infoBars[STAT_UNITS_NOW].percent = PERCENT(length,STAT_BAR_WIDTH);
	length = MAKEINT(scaleFactor*missionData.strBuilt);
	infoBars[STAT_STR_BUILT].percent = PERCENT(length,STAT_BAR_WIDTH);

	/* Finally the numbers themselves */
	infoBars[STAT_UNIT_LOST].number = missionData.unitsLost;
	infoBars[STAT_UNIT_KILLED].number = missionData.unitsKilled;
	infoBars[STAT_STR_LOST].number = missionData.strLost;
	infoBars[STAT_STR_BLOWN_UP].number = missionData.strKilled;
	infoBars[STAT_UNITS_BUILT].number =	missionData.unitsBuilt;
	infoBars[STAT_UNITS_NOW].number = numUnits;
	infoBars[STAT_STR_BUILT].number = missionData.strBuilt;
}
// -----------------------------------------------------------------------------------
/* This will save out the score data */
BOOL	writeScoreData( STRING *pFileName )
{
UBYTE			*pFileData;		// Pointer to the necessary allocated memory
MISSION_DATA	*pScoreData;
UDWORD			fileSize;		// How many bytes we need - depends on compression
FILE			*pFile;			// File pointer
SCORE_SAVEHEADER	*psHeader;		// Pointer to the header part of the file

	/* Calculate memory required */
	fileSize = ( sizeof(struct _score_save_header) + sizeof(struct mission_data) );

	/* Try and allocate it - freed up in same function */
	pFileData = (UBYTE *)MALLOC(fileSize);

	/* Did we get it? */
	if(!pFileData)
	{
		/* Nope, so do one */	
		DBERROR(("Saving Score data : Cannot get the memory! (%d)",fileSize));
		return(FALSE);
	} 

	/* We got the memory, so put the file header on the file */
	psHeader = (SCORE_SAVEHEADER *)pFileData;
	psHeader->aFileType[0] = 's';
	psHeader->aFileType[1] = 'c';
	psHeader->aFileType[2] = 'd';
	psHeader->aFileType[3] = 'a';
	psHeader->entries = 1;	// always for score save?

	/* Write out the version number - unlikely to change for FX data */
	psHeader->version = CURRENT_VERSION_NUM;

	/* Skip past the header to the raw data area */
	pScoreData = (MISSION_DATA*)(pFileData + sizeof(struct _score_save_header));

	/* copy over the score data */
	memcpy(pScoreData,&missionData,sizeof(struct mission_data));

	/* Have a bash at opening the file to write */
	pFile = fopen(pFileName, "wb");
	if (!pFile)
	{
		DBERROR(("Saving Score data : couldn't open file %s", pFileName));
		return(FALSE);
	}

	/* Now, try and write it out */
	if (fwrite(pFileData, 1, fileSize, pFile) != fileSize)
	{
		DBERROR(("Saving Score data : write failed for %s", pFileName));
		return(FALSE);
	}

	/* Finally, try and close it */
	if (fclose(pFile) != 0)
	{
		DBERROR(("Saving Score data : couldn't close %s", pFileName));
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
/* This will read in the score data */
BOOL	readScoreData( UBYTE *pFileData, UDWORD fileSize )
{
UDWORD				expectedFileSize;
SCORE_SAVEHEADER	*psHeader;
MISSION_DATA		*pScoreData;

	/* See if we've been given the right file type? */
	psHeader = (SCORE_SAVEHEADER *)pFileData;
	if (psHeader->aFileType[0] != 's' || psHeader->aFileType[1] != 'c' ||
		psHeader->aFileType[2] != 'd' || psHeader->aFileType[3] != 'a')	{
		DBERROR(("Read Score data : Weird file type found? Has header letters \
				  - %s %s %s %s", psHeader->aFileType[0],psHeader->aFileType[1],
								  psHeader->aFileType[2],psHeader->aFileType[3]));
		return FALSE;
	}

	/* How much data are we expecting? */
	expectedFileSize = (sizeof(struct _score_save_header) + (psHeader->entries*sizeof(struct mission_data)) );

	/* Is that what we've been given? */
	if(fileSize!=expectedFileSize)
	{
		/* No, so bomb out */
		DBERROR(("Read Score data : Weird file size!"));
		return(FALSE);
	}
	
	/* Skip past the header gubbins - can check version number here too */	
	pScoreData = (MISSION_DATA*)(pFileData + sizeof(struct _score_save_header));

	memcpy(&missionData,pScoreData,sizeof(struct mission_data));

	/* Hopefully everything's just fine by now */
	return(TRUE);
}
// -----------------------------------------------------------------------------------



