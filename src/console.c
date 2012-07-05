#include "Frame.h"
#include "GTime.h"
#include "Base.h"
#include "piedef.h"
#include "piestate.h"
#include "rendmode.h"
#include "intImage.h"
#include "Console.h"
#include "ScriptExtern.h"
#include "Audio_id.h"
#include "Audio.h"
#ifdef PSX
#include "Primatives.h"
#include "DCache.h"
#endif

/* Alex McLean, Pumpkin Studios, EIDOS Interactive */

/* Is the console history on or off */
static BOOL	bConsoleDropped = FALSE;

/* Stores the console dimensions and states */
static CONSOLE mainConsole;

/* Static storage for the maximum possible number of console messages */
static CONSOLE_MESSAGE	consoleStorage[MAX_CONSOLE_MESSAGES];

/* Maximum drop */
#define	MAX_DROP	32
static UDWORD	history[MAX_DROP];

/* Pointer to linked list of active messages - points to elements of the array above */
static CONSOLE_MESSAGE	*consoleMessages;

/* Where in the array are we - it's cyclic */
static UDWORD	messageIndex;

/* How many lines in the console history */
static UDWORD	consoleDrop = MAX_DROP;

static	UDWORD	maxDrop;

#define DROP_DROPPING	1
#define DROP_CLOSING	2
#define DROP_STATIC		3
#define DROP_CLOSED		4
#define DROP_STEP_INTERVAL	(15)

/* Console history state */
static UDWORD	dropState;

/* How many messages are presently active */
static UDWORD	numActiveMessages;

/* How long do messages last for? */
static UDWORD	messageDuration;

static UDWORD	lastDropChange = 0;

/* Is there a box under the console text? */
static BOOL		bTextBoxActive;

/* Is the console being displayed? */
static BOOL		bConsoleDisplayEnabled;

/* How many lines are displayed */
static UDWORD	consoleVisibleLines;

/* Whether new messages are allowed to be added */
static allowNewMessages;

/* What's the default justification */
static CONSOLE_TEXT_JUSTIFICATION	defJustification;

#ifdef WIN32
static UDWORD	messageId;	// unique ID
#endif

// Global string for new console messages.
char ConsoleString[MAX_CONSOLE_TMP_STRING_LENGTH];


/* MODULE CONSOLE PROTOTYPES */
void	consolePrintf				( SBYTE *layout, ... );
void	setConsoleSizePos			( UDWORD x, UDWORD y, UDWORD width );
BOOL	addConsoleMessage			( STRING *messageText, CONSOLE_TEXT_JUSTIFICATION jusType );
void	updateConsoleMessages		( void );
void	displayConsoleMessages		( void );
void	initConsoleMessages			( void );
void	setConsoleMessageDuration	( UDWORD time );
void	removeTopConsoleMessage		( void );
void	flushConsoleMessages		( void );
void	setConsoleBackdropStatus	( BOOL state );
void	enableConsoleDisplay		( BOOL state );
BOOL	getConsoleDisplayStatus		( void );
void	setDefaultConsoleJust		( CONSOLE_TEXT_JUSTIFICATION defJ );
void	setConsolePermanence		( BOOL state, BOOL bClearOld );
BOOL	mouseOverConsoleBox			( void );
void	setConsoleLineInfo			( UDWORD vis );
UDWORD	getConsoleLineInfo			( void );
void	permitNewConsoleMessages		( BOOL allow);
UDWORD	displayOldMessages			( void );
	
/* Sets the system up */
void	initConsoleMessages( void )
{
	messageIndex = 0;

	/* Console can extend to half screen height */
	maxDrop = ((DISP_HEIGHT/iV_GetTextLineSize())/2);

	if(maxDrop>32) maxDrop = 32;

	consoleDrop = maxDrop;//MAX_DROP;

	dropState = DROP_CLOSED;

	/* No active messages to begin with */
	numActiveMessages = 0;

	lastDropChange = 0;

	bConsoleDropped = FALSE;

	/* Linked list is empty */
	consoleMessages = NULL;

	/* Setup how long messages are displayed for... */
	setConsoleMessageDuration(DEFAULT_MESSAGE_DURATION);

	/* No box under the text */
	setConsoleBackdropStatus(TRUE);

	/* Turn on the console display */
	enableConsoleDisplay(TRUE);

	/* Set left justification as default */
	setDefaultConsoleJust(LEFT_JUSTIFY);

	/*	Set up the console size and postion 
		x,y,width */
	setConsoleSizePos(16,16,DISP_WIDTH-32);

	setConsoleLineInfo(MAX_CONSOLE_MESSAGES/4 + 4);

	/* We're not initially having permanent messages */
	setConsolePermanence(FALSE,TRUE);

	/* Allow new messages */
	permitNewConsoleMessages(TRUE);
}

void	toggleConsoleDrop( void )
{
	/* If it's closed ... */
	if(bConsoleDropped == FALSE)
	{
		dropState = DROP_DROPPING;
		consoleDrop = 0;
		bConsoleDropped = TRUE;

		audio_PlayTrack(ID_SOUND_WINDOWOPEN);
	}
	else
	{
		/* It's already open (or opening) */
		dropState = DROP_CLOSING;
		audio_PlayTrack(ID_SOUND_WINDOWCLOSE);
	}

	return;	
	if(!bConsoleDropped)
	{
		
		bConsoleDropped = !bConsoleDropped;
	}
	/* Are they opening it? */

	if(bConsoleDropped)
	{
		dropState = DROP_DROPPING;
	}
	else
	{
		dropState = DROP_CLOSING;
	}
}


/* Adds a string to the console. */
static BOOL _addConsoleMessage(STRING *messageText, CONSOLE_TEXT_JUSTIFICATION jusType)
{
UDWORD			textLength;
CONSOLE_MESSAGE	*psMessage;

	/* Just don't add it if there's too many already */
	if(numActiveMessages>=MAX_CONSOLE_MESSAGES-1)
	{
		return(FALSE);
	}

	/* Don't allow it to be added if we've disabled adding of new messages */
	if(!allowNewMessages)
	{
		return(FALSE);
	}

	/* Is the string too long? */
	textLength = strlen(messageText);

#ifdef WIN32
	ASSERT(( textLength<MAX_CONSOLE_STRING_LENGTH,
		"Attempt to add a message to the console that exceeds MAX_CONSOLE_STRING_LENGTH"));
#else
	if(textLength >= MAX_CONSOLE_STRING_LENGTH) {
		addConsoleMessage("bad length",jusType);
		return TRUE;
	}
#endif
 

	/* Are we using a defualt justification? */
	if(jusType == DEFAULT_JUSTIFY)
	{
		/* Then set it */
		jusType = defJustification;
	}
	/* Precalculate and store (quicker!) the indent for justified text */
	switch(jusType)
	{
		/* Allign to left edge of screen */
	case LEFT_JUSTIFY:
			consoleStorage[messageIndex].JustifyType = FTEXT_LEFTJUSTIFY;
		break;

		/* Allign to right edge of screen */
	case RIGHT_JUSTIFY:
			consoleStorage[messageIndex].JustifyType = FTEXT_RIGHTJUSTIFY;
		break;

		/* Allign to centre of the screen,NOT TO CENTRE OF CONSOLE!!!!!! */ 
	case CENTRE_JUSTIFY:
			consoleStorage[messageIndex].JustifyType = FTEXT_CENTRE;
		break;
		/* Gone tits up by the looks of it */
	default:
		DBERROR(("Weirdy type of text justification for console print"));
		break;
	}
	
	/* Copy over the text of the message */
	strcpy(consoleStorage[messageIndex].text,messageText);

	/* Set the time when it was added - this might not be needed */
	consoleStorage[messageIndex].timeAdded = gameTime2;

	/* This is the present newest message */
	consoleStorage[messageIndex].psNext = NULL;

#ifdef WIN32
	consoleStorage[messageIndex].id = 0;
#endif

	/* Are there no messages? */
	if(consoleMessages == NULL)
	{
		consoleMessages = &consoleStorage[messageIndex];
	}
	else
	{
		/* Get to the last element in our message list */
		for(psMessage = consoleMessages; psMessage->psNext; psMessage = psMessage->psNext)
		{
			/* NOP */
			;
		}
		/* Add it to the end */
		psMessage->psNext = &consoleStorage[messageIndex];
	}
 
	/* Move on in our array */
	if(messageIndex++ >= MAX_CONSOLE_MESSAGES-1)
	{
		/* Reset */
		messageIndex = 0;
	}

	/* There's one more active console message */
	numActiveMessages++;
	return(TRUE);
}


BOOL addConsoleMessage(STRING *messageText, CONSOLE_TEXT_JUSTIFICATION jusType)
{
#ifdef PSX
	// If the stacks in the dcache then..
	if(SpInDCache()) {
		static BOOL ret;
		// Set the stack pointer to point to the alternative stack which is'nt limited to 1k.
		SetSpAlt();
		ret = _addConsoleMessage(messageText,jusType);
		SetSpAltNormal();
		return ret;
	}
#endif
	return _addConsoleMessage(messageText,jusType);
}


UDWORD	getNumberConsoleMessages( void )
{
	return(numActiveMessages);
}

void	updateConsoleMessages( void )
{
	if(dropState == DROP_DROPPING)
	{
	 	if(gameTime - lastDropChange > DROP_STEP_INTERVAL)
		{
			lastDropChange = gameTime;
			if(++consoleDrop > maxDrop)//MAX_DROP)
			{
				consoleDrop = maxDrop;//MAX_DROP;
				dropState = DROP_STATIC;
			}
		}
	}
	else if (dropState == DROP_CLOSING)
	{
		if(gameTime - lastDropChange > DROP_STEP_INTERVAL)
		{
			lastDropChange = gameTime;
			if(consoleDrop)
			{
				consoleDrop--;
			}
			else
			{
				dropState = DROP_CLOSED;
				bConsoleDropped = FALSE;
			}
		}
	}

	/* Don't do anything for DROP_STATIC */

	/* If there are no messages or we're on permanent then exit */
 	if(consoleMessages == NULL OR mainConsole.permanent)
	{
		return;
	}
	
	/* Time to kill the top one ?*/
	if(gameTime2 - consoleMessages->timeAdded > messageDuration)
	{
#ifdef WIN32
		consoleMessages->id = messageId++;
#endif
		/* Is this the only message? */
		if(consoleMessages->psNext == NULL)
		{
 			/* Then list is now empty */
			consoleMessages = NULL;
		}
		else
		{
			/* Otherwise point it at the next one */
			consoleMessages = consoleMessages->psNext;
		}
		/* There's one less active console message */
 		numActiveMessages--;
	}
}

/* 
	Allows us to specify how long messages will stay on screen for. 
*/
void	setConsoleMessageDuration(UDWORD time)
{
	messageDuration = time;
}

/*	
	Allows us to remove the top message on screen.
	This and the function above should be sufficient to allow
	us to put up messages that stay there until we remove them
	ourselves - be sure and reset message duration afterwards 
*/
void	removeTopConsoleMessage( void )
{
	/* No point unless there is at least one */
	if(consoleMessages!=NULL)
	{
		/* Is this the only message? */
		if(consoleMessages->psNext == NULL)
		{
 			/* Then list is now empty */
			consoleMessages = NULL;
		}
		else
		{
			/* Otherwise point it at the next one */
			consoleMessages = consoleMessages->psNext;
		}
		/* There's one less active console message */
 		numActiveMessages--;
	}
}

/* Clears all console messages */
void	flushConsoleMessages( void )
{
	consoleMessages = NULL;
	numActiveMessages = 0;
#ifdef WIN32
	messageId = 0;
#endif
}

/* Displays all the console messages */
void	displayConsoleMessages( void )
{
CONSOLE_MESSAGE	*psMessage;
UDWORD	numProcessed;
UDWORD	linePitch;
UDWORD	boxDepth;
UDWORD	drop;
UDWORD	MesY;
UDWORD	clipDepth;
UDWORD	exceed;

	/* Are there any to display? */
	if(consoleMessages == NULL AND !bConsoleDropped)
	{
		/* No point - so get out */
 	return;
	}								

	/* Return if it's disabled */
	if(!bConsoleDisplayEnabled)
	{
		return;
	}

	/* Haven't done any yet */
	numProcessed = 0;

	/* Get the travel to the next line */
	linePitch = iV_GetTextLineSize();

#ifdef WIN32
	pie_SetDepthBufferStatus(DEPTH_CMP_ALWAYS_WRT_ON);
	pie_SetFogStatus(FALSE);
#endif
	iV_SetTextColour(-1);

#ifdef WIN32
	drop = 0;
	if(bConsoleDropped)
	{
		
		drop = displayOldMessages();
	}
	if(consoleMessages==NULL)
	{
		return;
	}
#endif

#ifdef WIN32
	/* Do we want a box under it? */
	if(bTextBoxActive)
	{
		for(psMessage = consoleMessages,exceed = 0; 
			psMessage AND (numProcessed<consoleVisibleLines) AND (exceed < 4); // ho ho ho!!! 
			psMessage = psMessage->psNext)
		{
			if((UDWORD)iV_GetTextWidth(psMessage->text) > mainConsole.width)
			{
				exceed++;
			}
		}
		
		/* How big a box is necessary? */
		boxDepth = (numActiveMessages> consoleVisibleLines ? consoleVisibleLines-1 : numActiveMessages-1);
		/* Add on the extra - hope it doesn't exceed two lines! */
		boxDepth+=exceed;
		/* GET RID OF THE MAGIC NUMBERS BELOW */
		clipDepth = (mainConsole.topY+(boxDepth*linePitch)+CON_BORDER_HEIGHT+drop);
		if(clipDepth > (DISP_HEIGHT-linePitch))
		{
			clipDepth = (DISP_HEIGHT - linePitch);
		}
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			if(bInTutorial) pie_SetSwirlyBoxes(TRUE);
			iV_UniTransBoxFill(mainConsole.topX - CON_BORDER_WIDTH,mainConsole.topY-mainConsole.textDepth-CON_BORDER_HEIGHT+drop+1,
				mainConsole.topX+mainConsole.width,clipDepth,						 // ho ho
//				(hack = (mainConsole.topY+(boxDepth*linePitch)+CON_BORDER_HEIGHT+drop)) < DISP_HEIGHT-linePitch ? hack : (DISP_HEIGHT-linePitch),
				(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE,FILLTRANS);
			if(bInTutorial) pie_SetSwirlyBoxes(FALSE);
		}
		else
		{
			iV_TransBoxFill(mainConsole.topX - CON_BORDER_WIDTH,mainConsole.topY-mainConsole.textDepth-CON_BORDER_HEIGHT+drop+1,
				mainConsole.topX+mainConsole.width ,clipDepth);
				//(hack = (mainConsole.topY+(boxDepth*linePitch)+CON_BORDER_HEIGHT+drop)) < DISP_HEIGHT-linePitch ? hack : (DISP_HEIGHT-linePitch)
		}
	}
 
   

	/* Stop when we've drawn enough or we're at the end */
	MesY = mainConsole.topY + drop;
	for(psMessage = consoleMessages,numProcessed = 0; 
		psMessage AND numProcessed<consoleVisibleLines AND MesY<(DISP_HEIGHT-linePitch); 
		psMessage = psMessage->psNext)
	{
 		/* Draw the text string */
		MesY = pie_DrawFormattedText(psMessage->text,
									mainConsole.topX,MesY,
									mainConsole.width,
									psMessage->JustifyType,FALSE);
		/* Move on */
		numProcessed++;
	}
#else // PSX version does it backwords.
	iV_SetOTIndex_PSX(OT2D_EXTREMEFORE);

	/* Stop when we've drawn enough or we're at the end */
	pie_StartTextExtents();
	MesY = mainConsole.topY;
	for(psMessage = consoleMessages; psMessage AND numProcessed<consoleVisibleLines; 
		psMessage = psMessage->psNext)
	{
 		/* Draw the text string */
		MesY = pie_DrawFormattedText(psMessage->text,
									mainConsole.topX,MesY,
									mainConsole.width,
									psMessage->JustifyType,TRUE);
		/* Move on */
		numProcessed++;
	}

	pie_FillTextExtents(0,16,16,128,TRUE);
	/* Do we want a box under it? */
//	if(bTextBoxActive)
//	{
//		/* How big a box is necessary? */
//		boxDepth = (numActiveMessages> consoleVisibleLines ? consoleVisibleLines-1 : numActiveMessages-1);
//		/* GET RID OF THE MAGIC NUMBERS BELOW */
//		iV_TransBoxFill(mainConsole.topX - CON_BORDER_WIDTH,
//						mainConsole.topY-mainConsole.textDepth+iV_GetTextBelowBase()-CON_BORDER_HEIGHT,
//						mainConsole.topX+mainConsole.width ,
//						mainConsole.topY+(boxDepth*linePitch)+iV_GetTextBelowBase()+CON_BORDER_HEIGHT);
//	}
#endif
}

#ifdef WIN32
/* Do up to the last 8 messages.... Returns how many it did... */
UDWORD	displayOldMessages( void )
{
UDWORD	thisIndex;
UDWORD	i;
UDWORD	count;
BOOL	bGotIt;
BOOL	bQuit;
UDWORD	marker;
UDWORD	linePitch;
UDWORD	MesY;
//UDWORD	buildWidth;
//STRING	buildData[255];

	/* Check there actually are any messages */
	thisIndex = messageId;
	count = 0;
	
	if(thisIndex)
	{
		bQuit = FALSE;
		while(!bQuit)
		{
			for(i=0,bGotIt = FALSE; i<MAX_CONSOLE_MESSAGES AND !bGotIt; i++)
			{
				if(consoleStorage[i].id == thisIndex-1)
				{
					bGotIt = TRUE;
					marker = i;
				}
			}
			/* We found an older one */
			if(bGotIt)
			{
				history[count++] = marker;
			}
			else
			{
				bQuit = TRUE;	// count holds how many we got
			}
			if(thisIndex) 
			{
			 	/* Look for an older one */
				thisIndex--;
			}
			else
			{
				bQuit = TRUE;	// We've reached the big bang - there is nothing older...
			}
			/* History can only hold so many */
			if(count>=consoleDrop)
			{
				bQuit = TRUE;
			}
		}
	}

	if(!count) 
	{
		/* there are messages - just no old ones yet */
		return(0);
	}	

	if(count)
	{
		/* Get the line pitch */ 
		linePitch = iV_GetTextLineSize();

		/* How big a box is necessary? */
		/* GET RID OF THE MAGIC NUMBERS BELOW */
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			if(bInTutorial) pie_SetSwirlyBoxes(TRUE);
			iV_UniTransBoxFill(mainConsole.topX - CON_BORDER_WIDTH,mainConsole.topY-mainConsole.textDepth-CON_BORDER_HEIGHT,
				mainConsole.topX+mainConsole.width,mainConsole.topY+((count)*linePitch)+CON_BORDER_HEIGHT-linePitch,(FILLRED<<16) | (FILLGREEN<<8) | FILLBLUE,FILLTRANS);
			if(bInTutorial) pie_SetSwirlyBoxes(FALSE);
		}
		else
		{
			iV_TransBoxFill(mainConsole.topX - CON_BORDER_WIDTH,mainConsole.topY-mainConsole.textDepth-CON_BORDER_HEIGHT,
				mainConsole.topX+mainConsole.width ,mainConsole.topY+((count)*linePitch)+CON_BORDER_HEIGHT-linePitch);
		}
	}
	/*
	if(count)
	{
		sprintf(buildData,"%s,%s",__TIME__,__DATE__);

		buildWidth = iV_GetTextWidth(buildData);

		pie_DrawText(buildData,((mainConsole.topX+mainConsole.width) - buildWidth - 16),
			mainConsole.topY);
	}
	*/
	MesY = mainConsole.topY;
	/* Render what we found */
	for(i=count-1; i>0; i--)
	{
		/* Draw the text string */
		MesY = pie_DrawFormattedText(consoleStorage[history[i]].text,
									mainConsole.topX,MesY,
									mainConsole.width,
									consoleStorage[history[i]].JustifyType,FALSE);
	}
	/* Draw the top one */
	(void) pie_DrawFormattedText(consoleStorage[history[0]].text,
									mainConsole.topX,MesY,
									mainConsole.width,
									consoleStorage[history[0]].JustifyType,FALSE);

	/* Return how much to drop the existing console by... Fix this for lines>screenWIDTH */
	if(count)
	{
		return((count)*linePitch);
	}
	else
	{
		return(0);
	}
	if(messageId)
	{
		for(i=0,bGotIt = FALSE; i<MAX_CONSOLE_MESSAGES AND !bGotIt; i++)
		{
			if(consoleStorage[i].id == messageId-1)
			{
				bGotIt = TRUE;
				thisIndex = i;
			}
		}
		if(bGotIt)
		{
			bQuit = FALSE;
			count = 0;
			while(!bQuit AND consoleStorage[thisIndex].id AND count<8)
			{
 				/* Draw the text string */
				MesY = pie_DrawFormattedText(consoleStorage[thisIndex].text,
											mainConsole.topX,MesY,
											mainConsole.width,
											consoleStorage[thisIndex].JustifyType,FALSE);
				count++;
				if(thisIndex) thisIndex--;
				else thisIndex = MAX_CONSOLE_MESSAGES-1;
			}
		}
	}
	return(count);
}
#endif


/* Allows toggling of the box under the console text */
void	setConsoleBackdropStatus(BOOL state)
{
	bTextBoxActive = state;
}

/* 
	Turns on and off display of console. It's worth
	noting that this is just the display so if you want
	to make sure that when it's turned back on again, there
	are no messages, the call flushConsoleMessages first.
*/
void	enableConsoleDisplay(BOOL state)
{
	bConsoleDisplayEnabled = state;
}

/* Sets the default justification for text */
void	setDefaultConsoleJust(CONSOLE_TEXT_JUSTIFICATION defJ)
{
	switch(defJ)
	{
	case LEFT_JUSTIFY:
	case RIGHT_JUSTIFY:
	case CENTRE_JUSTIFY:
		defJustification = defJ;
		break;
	default:
		DBERROR(("Weird default text justification for console"));
		break;
	}
}

/* Allows positioning of the console on screen */
void	setConsoleSizePos(UDWORD x, UDWORD y, UDWORD width)
{
#ifdef PSX
	y += 32;
#endif

	mainConsole.topX = x;
	mainConsole.topY = y;
	mainConsole.width = width;

	/* Should be done below */
#ifdef WIN32
	mainConsole.textDepth = 8;
#else
	mainConsole.textDepth = iV_GetTextLineSize();
#endif
	flushConsoleMessages();	
}

/*	Establishes whether the console messages stay there */
void	setConsolePermanence(BOOL state, BOOL bClearOld)
{
 	if(mainConsole.permanent == TRUE AND state == FALSE)
	{
		if(bClearOld)
		{
			flushConsoleMessages();
		}
		mainConsole.permanent = FALSE;
	}
	else
	{
		if(bClearOld)
		{
			flushConsoleMessages();
		}
		mainConsole.permanent = state;
	}
}

/* TRUE or FALSE as to whether the mouse is presently over the console window */
BOOL	mouseOverConsoleBox( void )
{
	if	( 
		((UDWORD)mouseX() > mainConsole.topX)	// condition 1
		AND ((UDWORD)mouseY() > mainConsole.topY)	// condition 2
		AND ((UDWORD)mouseX() < mainConsole.topX + mainConsole.width)	//condition 3
		AND ((UDWORD)mouseY() < (mainConsole.topY + iV_GetTextLineSize()*numActiveMessages))	//condition 4
	)
	{
		return(TRUE);	
	}
	else
	{
		return(FALSE);
	}
}

/* Sets up how many lines are allowed and how many are visible */
void	setConsoleLineInfo(UDWORD vis)
{
	ASSERT((vis<=MAX_CONSOLE_MESSAGES,"Request for more visible lines in the console than exist"));
	consoleVisibleLines = vis;
}

/* get how many lines are allowed and how many are visible */
UDWORD getConsoleLineInfo(VOID)
{
	return consoleVisibleLines;
}

void	consolePrintf(SBYTE *layout, ...)
{
STRING	consoleString[MAX_CONSOLE_STRING_LENGTH];
va_list	arguments;		// Formatting info

	/* Boot off the argument List */
	va_start(arguments,layout);

	/* 'print' it out into our buffer */
	(void)vsprintf(consoleString,layout,arguments);

	/* Add the message through the normal channels! */
	addConsoleMessage(consoleString,DEFAULT_JUSTIFY);

	/* Close arguments */
	va_end(arguments);
}

void	permitNewConsoleMessages(BOOL allow)
{
	allowNewMessages = allow;
}

BOOL	getConsoleDisplayStatus( void )
{
	return(bConsoleDisplayEnabled);
}

void	conShowReplayWav( void )
{
	

}