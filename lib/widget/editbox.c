/*
 * EditBox.c
 *
 * Functions for the edit box widget.
 */

#include "frame.h"
#include "widget.h"
#include "widgint.h"
#include "editbox.h"
#include "form.h"
#include "vid.h"

/* Pixel gap between edge of edit box and text */
#define WEDB_XGAP	4
#define WEDB_YGAP	2

/* Size of the overwrite cursor */
#define WEDB_CURSORSIZE		8

/* Whether the cursor blinks or not */
#define CURSOR_BLINK		0

/* The time the cursor blinks for */
#define WEDB_BLINKRATE		800

/* Number of characters to jump the edit box text when moving the cursor */
#define WEDB_CHARJUMP		6

/* The widget heap */
OBJ_HEAP	*psEdbHeap;

									// the other states
/* Calculate how much of the start of a string can fit into the edit box */
static void fitStringStart(STRING *pBuffer, UDWORD boxWidth, UWORD *pCount, UWORD *pCharWidth);

/* Create an edit box widget data structure */
BOOL editBoxCreate(W_EDITBOX **ppsWidget, W_EDBINIT *psInit)
{
	if (psInit->style & ~(WEDB_PLAIN | WIDG_HIDDEN | WEDB_DISABLED))
	{
		ASSERT((FALSE, "Unknown edit box style"));
		return FALSE;
	}

//	ASSERT((PTRVALID(psInit->psFont, sizeof(PROP_FONT)),
//		"editBoxCreate: Invalid font pointer"));

	/* Allocate the required memory */
#if W_USE_MALLOC
	*ppsWidget = (W_EDITBOX *)MALLOC(sizeof(W_EDITBOX));
	if (*ppsWidget == NULL)
#else
	if (!HEAP_ALLOC(psEdbHeap, ppsWidget))
#endif
	{
		ASSERT((FALSE, "Out of memory"));
		return FALSE;
	}

	/* Initialise the structure */
	(*ppsWidget)->type = WIDG_EDITBOX;
	(*ppsWidget)->id = psInit->id;
	(*ppsWidget)->formID = psInit->formID;
	(*ppsWidget)->style = psInit->style;
	(*ppsWidget)->x = psInit->x;
	(*ppsWidget)->y = psInit->y;
	(*ppsWidget)->width = psInit->width;
	(*ppsWidget)->height = psInit->height;
//	(*ppsWidget)->psFont = psInit->psFont;
	(*ppsWidget)->FontID = psInit->FontID;
	if (psInit->pDisplay)
	{
		(*ppsWidget)->display = psInit->pDisplay;
	}
	else
	{
		(*ppsWidget)->display = editBoxDisplay;
	}
	(*ppsWidget)->callback = psInit->pCallback;
	(*ppsWidget)->pUserData = psInit->pUserData;
	(*ppsWidget)->UserData = psInit->UserData;
	(*ppsWidget)->pBoxDisplay = psInit->pBoxDisplay;
	(*ppsWidget)->pFontDisplay = psInit->pFontDisplay;
	(*ppsWidget)->AudioCallback = WidgGetAudioCallback();
	(*ppsWidget)->HilightAudioID = WidgGetHilightAudioID();
	(*ppsWidget)->ClickedAudioID = WidgGetClickedAudioID();

	if (psInit->pText)
	{
		widgCopyString((*ppsWidget)->aText, psInit->pText);
	}
	else
	{
		(*ppsWidget)->aText[0] = 0;
	}

	editBoxInitialise(*ppsWidget);

	return TRUE;
}


/* Free the memory used by an edit box */
void editBoxFree(W_EDITBOX *psWidget)
{
#if W_USE_MALLOC
	FREE(psWidget);
#else
	HEAP_FREE(psEdbHeap, psWidget);
#endif
}


/* Initialise an edit box widget */
void editBoxInitialise(W_EDITBOX *psWidget)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_EDITBOX)),
		"editBoxInitialise: Invalid edit box pointer"));

	psWidget->state = WEDBS_FIXED;
	psWidget->printStart = 0;
	iV_SetFont(psWidget->FontID);
	fitStringStart(psWidget->aText, psWidget->width,
		&psWidget->printChars, &psWidget->printWidth);
}


/* Insert a character into a text buffer */
static void insertChar(STRING *pBuffer, UDWORD *pPos, STRING ch)
{
	STRING	*pSrc, *pDest;
	UDWORD	len, count;

	ASSERT((*pPos <= strlen(pBuffer),
		"insertChar: Invalid insertion point"));

	len = strlen(pBuffer);

	if (len == WIDG_MAXSTR - 1)
	{
		/* Buffer is full */
		return;
	}

	/* Move the end of the string up by one (including terminating \0) */
	count = len - *pPos + 1;
	pSrc = pBuffer + len;
	pDest = pSrc + 1;
	while (count--)
	{
		*pDest-- = *pSrc--;
	}

	/* Insert the character */
	*pDest = ch;

	/* Update the insertion point */
	*pPos += 1;
}


/* Put a character into a text buffer overwriting any text under the cursor */
static void overwriteChar(STRING *pBuffer, UDWORD *pPos, STRING ch)
{
	STRING	*pDest;
	UDWORD	len;

	ASSERT((*pPos <= strlen(pBuffer),
		"insertChar: Invalid insertion point"));

	len = strlen(pBuffer);

	if (len == WIDG_MAXSTR - 1)
	{
		/* Buffer is full */
		return;
	}

	/* Store the character */
	pDest = pBuffer + *pPos;
	*pDest = ch;

	if (*pPos == len)
	{
		/* At the end of the string, move the \0 up one */
		*(pDest + 1) = '\0';
	}

	/* Update the insertion point */
	*pPos += 1;
}


/* Delete a character to the left of the position */
static void delCharLeft(STRING *pBuffer, UDWORD *pPos)
{
	STRING	*pSrc, *pDest;
	UDWORD	len, count;

	ASSERT((*pPos <= strlen(pBuffer),
		"delCharLeft: Invalid insertion point"));

	/* Can't delete if we are at the start of the string */
	if (*pPos == 0)
	{
		return;
	}

	len = strlen(pBuffer);

	/* Move the end of the string down by one */
	count = len - *pPos + 1;
	pSrc = pBuffer + *pPos;
	pDest = pSrc - 1;
	while (count--)
	{
		*pDest++ = *pSrc ++;
	}

	/* Update the insertion point */
	*pPos -= 1;
}


/* Delete a character to the right of the position */
static void delCharRight(STRING *pBuffer, UDWORD *pPos)
{
	STRING	*pSrc, *pDest;
	UDWORD	len, count;

	ASSERT((*pPos <= strlen(pBuffer),
		"delCharLeft: Invalid insertion point"));

	len = strlen(pBuffer);

	/* Can't delete if we are at the end of the string */
	if (*pPos == len)
	{
		return;
	}

	/* Move the end of the string down by one */
	count = len - *pPos;
	pDest = pBuffer + *pPos;
	pSrc = pDest + 1;
	while (count--)
	{
		*pDest++ = *pSrc ++;
	}
}


/* Calculate how much of the start of a string can fit into the edit box */
static void fitStringStart(STRING *pBuffer, UDWORD boxWidth, UWORD *pCount, UWORD *pCharWidth)
{
	UDWORD		len;
	UWORD		printWidth, printChars, width;
	STRING		*pCurr;
//	PROP_FONT	*psCurrFont;

	len = strlen(pBuffer);
	printWidth = 0;
	printChars = 0;
	pCurr = pBuffer;
//	psCurrFont = fontGet();

	/* Find the number of characters that will fit in boxWidth */
	while (printChars < len)
	{
		width = (UWORD)(printWidth + iV_GetCharWidth(*pCurr));
		if (width > boxWidth - WEDB_XGAP*2)
		{
			/* We've got as many characters as will fit in the box */
			break;
		}
		printWidth = width;
		printChars += 1;
		pCurr += 1;
	}

	/* Return the number of characters and their width */
	*pCount = printChars;
	*pCharWidth = printWidth;
}


/* Calculate how much of the end of a string can fit into the edit box */
static void fitStringEnd(STRING *pBuffer, UDWORD boxWidth,
						 UWORD *pStart, UWORD *pCount, UWORD *pCharWidth)
{
	UDWORD		len;
	UWORD		printWidth, printChars, width;
	STRING		*pCurr;
//	PROP_FONT	*psCurrFont;

	len = strlen(pBuffer);

//	psCurrFont = fontGet();
	pCurr = pBuffer + len - 1;
	printChars = 0;
	printWidth = 0;

	/* Find the number of characters that will fit in boxWidth */
	while (printChars < len)
	{
		width = (UWORD)(printWidth + iV_GetCharWidth(*pCurr));
		if (width > boxWidth - (WEDB_XGAP*2 + WEDB_CURSORSIZE))
		{
			/* Got as many characters as will fit into the box */
			break;
		}
		printWidth = width;
		printChars += 1;
		pCurr -= 1;
	}

	/* Return the number of characters and their width */
	*pStart = (UWORD)(len - printChars);
	*pCount = printChars;
	*pCharWidth = printWidth;
}


/* Run an edit box widget */
void editBoxRun(W_EDITBOX *psWidget, W_CONTEXT *psContext)
{
	UDWORD	key, len, editState;
	UDWORD	pos;
	STRING	*pBuffer;
	BOOL	done;
	UWORD	printStart, printWidth, printChars;
	SDWORD	mx,my;

	/* Note the edit state */
	editState = psWidget->state & WEDBS_MASK;

	/* Only have anything to do if the widget is being edited */
	if ((editState & WEDBS_MASK) == WEDBS_FIXED)
	{
		return;
	}

	/* If there is a mouse click outside of the edit box - stop editing */
	mx = psContext->mx;
	my = psContext->my;
	if (mousePressed(MOUSE_LMB) &&
		(mx < psWidget->x ||
		 (mx > psWidget->x + psWidget->width) ||
		 my < psWidget->y ||
		 (my > psWidget->y + psWidget->height)))
	{
		screenClearFocus(psContext->psScreen);
		return;
	}

	/* note the widget state */
	pos = psWidget->insPos;
	pBuffer = psWidget->aText;
	printStart = psWidget->printStart;
	printWidth = psWidget->printWidth;
	printChars = psWidget->printChars;
	iV_SetFont(psWidget->FontID);

	/* Loop through the characters in the input buffer */
	done = FALSE;
	for(key = inputGetKey(); key != 0 && !done; key = inputGetKey())
	{
		/* Deal with all the control keys, assume anything else is a printable character */
		switch (key)
		{
		case INPBUF_LEFT :
			/* Move the cursor left */
			if (pos > 0)
			{
				pos -= 1;
			}

			/* If the cursor has gone off the left of the edit box,
			 * need to update the printable text.
			 */
			if (pos < printStart)
			{
				if (printStart <= WEDB_CHARJUMP)
				{
					/* Got to the start of the string */
					printStart = 0;
					fitStringStart(pBuffer, psWidget->width, &printChars, &printWidth);
				}
				else
				{
					printStart -= WEDB_CHARJUMP;
					fitStringStart(pBuffer + printStart, psWidget->width,
						&printChars, &printWidth);
				}
			}
			break;
		case INPBUF_RIGHT :
			/* Move the cursor right */
			len = strlen(pBuffer);
			if (pos < len)
			{
				pos += 1;
			}

			/* If the cursor has gone off the right of the edit box,
			 * need to update the printable text.
			 */
			if (pos > (UDWORD)(printStart + printChars))
			{
				printStart += WEDB_CHARJUMP;
				if (printStart >= len)
				{
					printStart = (UWORD)(len - 1);
				}
				fitStringStart(pBuffer + printStart, psWidget->width,
					&printChars, &printWidth);
			}
			break;
		case INPBUF_UP :
			break;
		case INPBUF_DOWN :
			break;
		case INPBUF_HOME :
			/* Move the cursor to the start of the buffer */
			pos = 0;
			printStart = 0;
			fitStringStart(pBuffer, psWidget->width, &printChars, &printWidth);
			break;
		case INPBUF_END :
			/* Move the cursor to the end of the buffer */
			pos = strlen(pBuffer);
			if (pos != (UWORD)(printStart + printChars))
			{
				fitStringEnd(pBuffer, psWidget->width, &printStart, &printChars, &printWidth);
			}
			break;
		case INPBUF_INS :
			if (editState == WEDBS_INSERT)
			{
				editState = WEDBS_OVER;
			}
			else
			{
				editState = WEDBS_INSERT;
			}
			break;
		case INPBUF_DEL :
			delCharRight(pBuffer, &pos);

			/* Update the printable text */
			fitStringStart(pBuffer + printStart, psWidget->width,
				&printChars, &printWidth);
			break;
		case INPBUF_PGUP :
			break;
		case INPBUF_PGDN :
			break;
		case INPBUF_BKSPACE :
			/* Delete the character to the left of the cursor */
			delCharLeft(pBuffer, &pos);

			/* Update the printable text */
			if (pos <= printStart)
			{
				if (printStart <= WEDB_CHARJUMP)
				{
					/* Got to the start of the string */
					printStart = 0;
					fitStringStart(pBuffer, psWidget->width, &printChars, &printWidth);
				}
				else
				{
					printStart -= WEDB_CHARJUMP;
					fitStringStart(pBuffer + printStart, psWidget->width,
						&printChars, &printWidth);
				}
			}
			else
			{
				fitStringStart(pBuffer + printStart, psWidget->width,
					&printChars, &printWidth);
			}
			break;
		case INPBUF_TAB :
			break;
		case INPBUF_CR :
			/* Finish editing */
			editBoxFocusLost(psWidget);
			screenClearFocus(psContext->psScreen);
			return;
			break;
		case INPBUF_ESC :
			break;

		default:
			/* Dealt with everything else this must be a printable character */
			if (editState == WEDBS_INSERT)
			{
				insertChar(pBuffer, &pos, (STRING)key);
			}
			else
			{
				overwriteChar(pBuffer, &pos, (STRING)key);
			}

			/* Update the printable chars */
			if (pos == strlen(pBuffer))
			{
				fitStringEnd(pBuffer, psWidget->width, &printStart, &printChars, &printWidth);
			}
			else
			{
				fitStringStart(pBuffer + printStart, psWidget->width, &printChars, &printWidth);
				if (pos > (UDWORD)(printStart + printChars))
				{
					printStart += WEDB_CHARJUMP;
					if (printStart >= len)
					{
						printStart = (UWORD)(len - 1);
						fitStringStart(pBuffer + printStart, psWidget->width,
							&printChars, &printWidth);
					}
				}
			}
			break;
		}
	}

	/* Store the current widget state */
	psWidget->insPos = (UWORD)pos;
	psWidget->state = (psWidget->state & ~WEDBS_MASK) | editState;
	psWidget->printStart = printStart;
	psWidget->printWidth = printWidth;
	psWidget->printChars = printChars;
}


/* Set the current string for the edit box */
void editBoxSetString(W_EDITBOX *psWidget, STRING *pText)
{
	ASSERT((PTRVALID(psWidget, sizeof(W_EDITBOX)),
		"editBoxSetString: Invalid edit box pointer"));

	widgCopyString(psWidget->aText, pText);
	psWidget->state = WEDBS_FIXED;
	psWidget->printStart = 0;
	iV_SetFont(psWidget->FontID);
	fitStringStart(psWidget->aText, psWidget->width,
		&psWidget->printChars, &psWidget->printWidth);
}


/* Respond to a mouse click */
void editBoxClicked(W_EDITBOX *psWidget, W_CONTEXT *psContext)
{
	UDWORD		len;

	if(psWidget->state & WEDBS_DISABLE)	// disabled button.
	{
		return;							
	}

	if ((psWidget->state & WEDBS_MASK) == WEDBS_FIXED)
	{
		if(!(psWidget->style & WEDB_DISABLED)) {
			if(psWidget->AudioCallback) {
				psWidget->AudioCallback(psWidget->ClickedAudioID);
			}

			/* Set up the widget state */
			psWidget->state = (psWidget->state & ~WEDBS_MASK) | WEDBS_INSERT;
			len = strlen(psWidget->aText);
			psWidget->insPos = (UWORD)len;

			/* Calculate how much of the string can appear in the box */
			iV_SetFont(psWidget->FontID);
			fitStringEnd(psWidget->aText, psWidget->width,
				&psWidget->printStart, &psWidget->printChars, &psWidget->printWidth);

            // FIXME
			/* Clear the input buffer */
			inputClearBuffer();

			/* Tell the form that the edit box has focus */
			screenSetFocus(psContext->psScreen, (WIDGET *)psWidget);

		}
	}
}


/* Respond to loss of focus */
void editBoxFocusLost(W_EDITBOX *psWidget)
{
	ASSERT(( !(psWidget->state & WEDBS_DISABLE),
		"editBoxFocusLost: disabled edit box"));

	/* Stop editing the widget */
	psWidget->state = WEDBS_FIXED;
	psWidget->printStart = 0;
	fitStringStart(psWidget->aText,psWidget->width,
				   &psWidget->printChars, &psWidget->printWidth);
	widgSetReturn((WIDGET *)psWidget);
}


/* Respond to a mouse button up */
void editBoxReleased(W_EDITBOX *psWidget)
{
	(void)psWidget;
}


/* Respond to a mouse moving over an edit box */
void editBoxHiLite(W_EDITBOX *psWidget)
{
	if(psWidget->state & WEDBS_DISABLE)
	{
		return;
	}

	if(psWidget->AudioCallback) {
		psWidget->AudioCallback(psWidget->HilightAudioID);
	}

	psWidget->state |= WEDBS_HILITE;
}


/* Respond to the mouse moving off an edit box */
void editBoxHiLiteLost(W_EDITBOX *psWidget)
{
	if(psWidget->state & WEDBS_DISABLE)
	{
		return;
	}

	psWidget->state = psWidget->state & WEDBS_MASK;
}


/* The edit box display function */
void editBoxDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, UDWORD *pColours)
{
	W_EDITBOX	*psEdBox;
	SDWORD		x0,y0,x1,y1, fx,fy, cx,cy;
//	PROP_FONT	*psCurrFont;
	int CurrFontID;
	STRING		ch, *pInsPoint, *pPrint;
#if CURSOR_BLINK
	BOOL		blink;
#endif

	psEdBox = (W_EDITBOX *)psWidget;
//	psCurrFont = psEdBox->psFont;
	CurrFontID = psEdBox->FontID;

	x0=psEdBox->x + xOffset;
	y0=psEdBox->y + yOffset;
	x1=x0 + psEdBox->width;
	y1=y0 + psEdBox->height;

	if(psEdBox->pBoxDisplay) {
		psEdBox->pBoxDisplay((WIDGET *)psEdBox, xOffset, yOffset, pColours);
	} else {
		pie_BoxFillIndex(x0,y0,x1,y1,WCOL_BKGRND);
		iV_Line(x0,y0, x1,y0,*(pColours + WCOL_DARK));
		iV_Line(x0,y0, x0,y1,*(pColours + WCOL_DARK));
		iV_Line(x0,y1, x1,y1,*(pColours + WCOL_LIGHT));
		iV_Line(x1,y1, x1,y0,*(pColours + WCOL_LIGHT));
	}

	fx = x0 + WEDB_XGAP;// + (psEdBox->width - fw) / 2;
//	fy = y0 + (psEdBox->height - psCurrFont->height + psCurrFont->baseLine) / 2;

	iV_SetFont(CurrFontID);
//	fontSet(psCurrFont);
	iV_SetTextColour((UBYTE)*(pColours + WCOL_TEXT));

  	fy = y0 + (psEdBox->height - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();


	/* If there is more text than will fit into the box,
	   display the bit with the cursor in it */
	pPrint = psEdBox->aText + psEdBox->printStart;
	pInsPoint = pPrint + psEdBox->printChars;
	ch = *pInsPoint;

	*pInsPoint = '\0';
//	if(psEdBox->pFontDisplay) {
//		psEdBox->pFontDisplay(fx,fy, pPrint);
//	} else {

		iV_DrawText(pPrint,fx,fy);
//	}
	*pInsPoint = ch;

	/* Display the cursor if editing */
#if CURSOR_BLINK
	blink = (GetTickCount()/WEDB_BLINKRATE) % 2;
	if ((psEdBox->state & WEDBS_MASK) == WEDBS_INSERT && blink)
#else
	if ((psEdBox->state & WEDBS_MASK) == WEDBS_INSERT)
#endif
	{
		pInsPoint = psEdBox->aText + psEdBox->insPos;
		ch = *pInsPoint;
		*pInsPoint = '\0';
		cx = x0 + WEDB_XGAP + iV_GetTextWidth(psEdBox->aText + psEdBox->printStart);
		*pInsPoint = ch;
		cy = fy;
		iV_Line(cx,cy+iV_GetTextAboveBase(), cx,cy+iV_GetTextBelowBase(),*(pColours + WCOL_CURSOR));
	}
#if CURSOR_BLINK
	else if ((psEdBox->state & WEDBS_MASK) == WEDBS_OVER && blink)
#else
	else if ((psEdBox->state & WEDBS_MASK) == WEDBS_OVER)
#endif
	{
		pInsPoint = psEdBox->aText + psEdBox->insPos;
		ch = *pInsPoint;
		*pInsPoint = '\0';
		cx = x0 + WEDB_XGAP + iV_GetTextWidth(psEdBox->aText + psEdBox->printStart);
		*pInsPoint = ch;
	  	cy = fy;
//		cy = fy + psCurrFont->height - (psCurrFont->baseLine >> 1);
		iV_Line(cx,cy, cx + WEDB_CURSORSIZE,cy,*(pColours + WCOL_CURSOR));
	}

	if(psEdBox->pBoxDisplay == NULL) {
		if (psEdBox->state & WEDBS_HILITE)
		{
			/* Display the button hilite */
			iV_Line(x0-2,y0-2, x1+2,y0-2,*(pColours + WCOL_HILITE));
			iV_Line(x0-2,y0-2, x0-2,y1+2,*(pColours + WCOL_HILITE));
			iV_Line(x0-2,y1+2, x1+2,y1+2,*(pColours + WCOL_HILITE));
			iV_Line(x1+2,y1+2, x1+2,y0-2,*(pColours + WCOL_HILITE));
		}
	}
}



/* Set an edit box'sstate */
void editBoxSetState(W_EDITBOX *psEditBox, UDWORD state)
{
	if (state & WEDBS_DISABLE)
	{
		psEditBox->state |= WEDBS_DISABLE;
	}
	else
	{
		psEditBox->state &= ~WEDBS_DISABLE;
	}

}