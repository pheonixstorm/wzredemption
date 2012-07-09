/*
 * Widget.c
 *
 * The main interface functions to the widget library
 */


#include "Frame.h"
#include "FrameInt.h"

#include "Widget.h"
#include "WidgInt.h"

#include "Form.h"
#include "Label.h"
#include "Button.h"
#include "EditBox.h"
#include "Bar.h"
#include "Slider.h"

#include "Tip.h"

#include <assert.h>

/* The initial and extension number of strings to allocate in the string heap */
#define WIDG_STRINIT		100
#define WIDG_STREXT			10

/* the widget to be returned by widgRunScreen */
static WIDGET	*psRetWidget;

static	BOOL	bWidgetsActive = TRUE;

/* The widget the mouse is over this update */
static WIDGET	*psMouseOverWidget;

static UDWORD	pressed, released;

static WIDGET_AUDIOCALLBACK AudioCallback = NULL;
static SWORD HilightAudioID = -1;
static SWORD ClickedAudioID = -1;


/* Function prototypes */
void widgHiLite(WIDGET *psWidget, W_CONTEXT *psContext);
void widgHiLiteLost(WIDGET *psWidget, W_CONTEXT *psContext);
static void widgClicked(WIDGET *psWidget, UDWORD key, W_CONTEXT *psContext);
static void widgReleased(WIDGET *psWidget, UDWORD key, W_CONTEXT *psContext);
static void widgRun(WIDGET *psWidget, W_CONTEXT *psContext);
static void widgDisplayForm(W_FORM *psForm, UDWORD xOffset, UDWORD yOffset);

/* The heap for widget strings */
static OBJ_HEAP		*psStrHeap;

/* Buffer to return strings in */
static STRING aStringRetBuffer[WIDG_MAXSTR];

/* Initialise the widget module */
BOOL widgInitialise(W_HEAPINIT *psInit)
{
#if W_USE_STRHEAP
	// Create the string heap
	if (!HEAP_CREATE(&psStrHeap, WIDG_MAXSTR, WIDG_STRINIT, WIDG_STREXT))
	{
		return FALSE;
	}
#endif
	
	tipInitialise();

#if W_USE_MALLOC
	psInit = psInit;
#else
	// Create the widget heaps
	if (!HEAP_CREATE(&psBarHeap, sizeof(W_BARGRAPH), psInit->barInit, psInit->barExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psButHeap, sizeof(W_BUTTON), psInit->butInit, psInit->butExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psEdbHeap, sizeof(W_EDITBOX), psInit->edbInit, psInit->edbExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psFormHeap, sizeof(W_FORM), psInit->formInit, psInit->formExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psCFormHeap, sizeof(W_CLICKFORM), psInit->cFormInit, psInit->cFormExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psTFormHeap, sizeof(W_TABFORM), psInit->tFormInit, psInit->tFormExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psLabHeap, sizeof(W_LABEL), psInit->labInit, psInit->labExt))
	{
		return FALSE;
	}
	if (!HEAP_CREATE(&psSldHeap, sizeof(W_SLIDER), psInit->sldInit, psInit->sldExt))
	{
		return FALSE;
	}
#endif

	return TRUE;
}


// Reset the widgets.
//
void widgReset(void)
{
	tipInitialise();
}


/* Shut down the widget module */
void widgShutDown(void)
{
#if W_USE_STRHEAP
	HEAP_DESTROY(psStrHeap);
#endif

#if !W_USE_MALLOC
	HEAP_DESTROY(psBarHeap);
	HEAP_DESTROY(psButHeap);
	HEAP_DESTROY(psEdbHeap);
	HEAP_DESTROY(psFormHeap);
	HEAP_DESTROY(psCFormHeap);
	HEAP_DESTROY(psTFormHeap);
	HEAP_DESTROY(psLabHeap);
	HEAP_DESTROY(psSldHeap);
#endif
}


/* Get a string from the string heap */
BOOL widgAllocString(STRING **ppStr)
{
	if (!HEAP_ALLOC(psStrHeap, ppStr))
	{
		return FALSE;
	}

	return TRUE;
}

/* Copy one string to another
 * The string to copy will be truncated if it is longer than WIDG_MAXSTR.
 */
void widgCopyString(STRING *pDest, STRING *pSrc)
{
	/* See if we need to clip the string, then copy */
	if (strlen(pSrc) >= WIDG_MAXSTR)
	{
		memcpy(pDest, pSrc, WIDG_MAXSTR - 1);
		*(pDest + WIDG_MAXSTR-1) = 0;
	}
	else
	{
		strcpy(pDest, pSrc);
	}
}

/* Get a string from the heap and copy in some data.
 * The string to copy will be truncated if it is too long.
 */
BOOL widgAllocCopyString(STRING **ppDest, STRING *pSrc)
{
	if (!HEAP_ALLOC(psStrHeap, ppDest))
	{
		*ppDest = NULL;
		return FALSE;
	}

	widgCopyString(*ppDest, pSrc);

	return TRUE;
}


/* Return a string to the string heap */
void widgFreeString(STRING *pStr)
{
	HEAP_FREE(psStrHeap, pStr);
}

/* Create an empty widget screen */
BOOL widgCreateScreen(W_SCREEN **ppsScreen)
{
	W_FORM		*psForm;
	W_FORMINIT	sInit;

	*ppsScreen = (W_SCREEN *)MALLOC(sizeof(W_SCREEN));
	if (*ppsScreen == NULL)
	{
		ASSERT((FALSE, "Out of memory"));
		return FALSE;
	}

	memset(&sInit, 0, sizeof(W_FORMINIT));
	sInit.id = 0;
	sInit.style = WFORM_PLAIN | WFORM_INVISIBLE;
	sInit.x = 0;
	sInit.y = 0;
	sInit.width = (UWORD)(screenWidth - 1);
	sInit.height = (UWORD)(screenHeight - 1);
	if (!formCreate(&psForm, &sInit))
	{
		return FALSE;
	}

	(*ppsScreen)->psForm = (WIDGET *)psForm;
	(*ppsScreen)->psFocus = NULL;
	(*ppsScreen)->TipFontID = 0;

	return TRUE;
}


/* Release a list of widgets */
void widgReleaseWidgetList(WIDGET *psWidgets)
{
	WIDGET	*psCurr, *psNext;

	for(psCurr = psWidgets; psCurr; psCurr = psNext)
	{
		psNext = psCurr->psNext;
		switch(psCurr->type)
		{
		case WIDG_FORM:
			formFree((W_FORM *)psCurr);
			break;
		case WIDG_LABEL:
			labelFree((W_LABEL *)psCurr);
			break;
		case WIDG_BUTTON:
			buttonFree((W_BUTTON *)psCurr);
			break;
		case WIDG_EDITBOX:
			editBoxFree((W_EDITBOX *)psCurr);
			break;
		case WIDG_BARGRAPH:
			barGraphFree((W_BARGRAPH *)psCurr);
			break;
		case WIDG_SLIDER:
			sliderFree((W_SLIDER *)psCurr);
			break;
		default:
			ASSERT((FALSE,"widgReleaseWidgetList: Unknown widget type"));
			break;
		}
	}

}

/* Release a screen and all its associated data */
void widgReleaseScreen(W_SCREEN *psScreen)
{
	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgReleaseScreen: Invalid screen pointer"));

	formFree((W_FORM *)psScreen->psForm);

	FREE(psScreen);
}


/* Release a widget */
void widgRelease(WIDGET *psWidget)
{
	switch(psWidget->type)
	{
	case WIDG_FORM:
		formFree((W_FORM *)psWidget);
		break;
	case WIDG_LABEL:
		labelFree((W_LABEL *)psWidget);
		break;
	case WIDG_BUTTON:
		buttonFree((W_BUTTON *)psWidget);
		break;
	case WIDG_EDITBOX:
		editBoxFree((W_EDITBOX *)psWidget);
		break;
	case WIDG_BARGRAPH:
		barGraphFree((W_BARGRAPH *)psWidget);
		break;
	case WIDG_SLIDER:
		sliderFree((W_SLIDER *)psWidget);
		break;
	default:
		ASSERT((FALSE,"widgRelease: Unknown widget type"));
		break;
	}
}

/* Check whether an ID has been used on a form */
static BOOL widgCheckIDForm(W_FORM *psForm, UDWORD id)
{
	WIDGET			*psCurr;
	W_FORMGETALL	sGetAll;

	/* Check the widgets on the form */
	formInitGetAllWidgets(psForm, &sGetAll);
	psCurr = formGetAllWidgets(&sGetAll);
	while (psCurr != NULL)
	{
		if (psCurr->id == id)
		{
			return TRUE;
		}

		if (psCurr->type == WIDG_FORM)
		{
			/* Another form so recurse */
			if (widgCheckIDForm((W_FORM *)psCurr, id))
			{
				return TRUE;
			}
		}

		psCurr = psCurr->psNext;
		if (!psCurr)
		{
			/* Got to the end of this list see if there is another */
			psCurr = formGetAllWidgets(&sGetAll);
		}
	}

	return FALSE;
}

#if 0
/* Check whether an ID number has been used on a screen */
static BOOL widgCheckID(W_SCREEN *psScreen, UDWORD id)
{
	return widgCheckIDForm((W_FORM *)psScreen->psForm, id);
}
#endif


///* Set the tool tip font for a screen */
//void widgSetTipFont(W_SCREEN *psScreen, PROP_FONT *psFont)
//{
//	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
//		"widgSetTipFont: Invalid screen pointer"));
//	ASSERT((psFont == NULL || PTRVALID(psFont, sizeof(PROP_FONT)),
//		"widgSetTipFont: Invalid font pointer"));
//
//	psScreen->psTipFont = psFont;
//}


/* Set the tool tip font for a screen */
void widgSetTipFont(W_SCREEN *psScreen, int FontID)
{
	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgSetTipFont: Invalid screen pointer"));
//	ASSERT((psFont == NULL || PTRVALID(psFont, sizeof(PROP_FONT)),
//		"widgSetTipFont: Invalid font pointer"));

	psScreen->TipFontID = FontID;
}



/* Add a form to the widget screen */
BOOL widgAddForm(W_SCREEN *psScreen, W_FORMINIT *psInit)
{
	W_FORM	*psParent, *psForm;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgAddForm: Invalid screen pointer"));

	if (widgCheckIDForm((W_FORM *)psScreen->psForm,psInit->id))
	{
		ASSERT((FALSE, "widgAddForm: ID number has already been used"));
		return FALSE;
	}

	/* Find the form to add the widget to */
	if (psInit->formID == 0)
	{
		/* Add to the base form */
		psParent = (W_FORM *)psScreen->psForm;
	}
	else
	{
		psParent = (W_FORM *)widgGetFromID(psScreen, psInit->formID);
		if (!psParent || psParent->type != WIDG_FORM)
		{
			ASSERT((FALSE,
				"widgAddForm: Could not find parent form from formID"));
			return FALSE;
		}
	}

	/* Create the form structure */
	if (!formCreate(&psForm, psInit))
	{
		return FALSE;
	}

	/* Add it to the screen */
	if (!formAddWidget(psParent, (WIDGET *)psForm, (W_INIT *)psInit))
	{
		return FALSE;
	}

	return TRUE;
}


/* Add a label to the widget screen */
BOOL widgAddLabel(W_SCREEN *psScreen, W_LABINIT *psInit)
{
	W_LABEL		*psLabel;
	W_FORM		*psForm;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgAddLabel: Invalid screen pointer"));

	if (widgCheckIDForm((W_FORM *)psScreen->psForm,psInit->id))
	{
		ASSERT((FALSE, "widgAddLabel: ID number has already been used"));
		return FALSE;
	}

	/* Find the form to put the button on */
	if (psInit->formID == 0)
	{
		psForm = (W_FORM *)psScreen->psForm;
	}
	else
	{
		psForm = (W_FORM *)widgGetFromID(psScreen, psInit->formID);
		if (psForm == NULL || psForm->type != WIDG_FORM)
		{
			ASSERT((FALSE,
				"widgAddLabel: Could not find parent form from formID"));
			return FALSE;
		}
	}

	/* Create the button structure */
	if (!labelCreate(&psLabel, psInit))
	{
		return FALSE;
	}

	/* Add it to the form */
	if (!formAddWidget(psForm, (WIDGET *)psLabel, (W_INIT *)psInit))
	{
		return FALSE;
	}

	return TRUE;
}


/* Add a button to the widget screen */
BOOL widgAddButton(W_SCREEN *psScreen, W_BUTINIT *psInit)
{
	W_BUTTON	*psButton;
	W_FORM		*psForm;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgAddButton: Invalid screen pointer"));

	if (widgCheckIDForm((W_FORM *)psScreen->psForm,psInit->id))
	{
		ASSERT((FALSE, "widgAddButton: ID number has already been used"));
		return FALSE;
	}

	/* Find the form to put the button on */
	if (psInit->formID == 0)
	{
		psForm = (W_FORM *)psScreen->psForm;
	}
	else
	{
		psForm = (W_FORM *)widgGetFromID(psScreen, psInit->formID);
		if (psForm == NULL || psForm->type != WIDG_FORM)
		{
			ASSERT((FALSE,
				"widgAddButton: Could not find parent form from formID"));
			return FALSE;
		}
	}

	/* Create the button structure */
	if (!buttonCreate(&psButton, psInit))
	{
		return FALSE;
	}

	/* Add it to the form */
	if (!formAddWidget(psForm, (WIDGET *)psButton, (W_INIT *)psInit))
	{
		return FALSE;
	}

	return TRUE;
}


/* Add an edit box to the widget screen */
BOOL widgAddEditBox(W_SCREEN *psScreen, W_EDBINIT *psInit)
{
	W_EDITBOX	*psEdBox;
	W_FORM		*psForm;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgAddEditBox: Invalid screen pointer"));

	if (widgCheckIDForm((W_FORM *)psScreen->psForm,psInit->id))
	{
		ASSERT((FALSE, "widgAddEditBox: ID number has already been used"));
		return FALSE;
	}

	/* Find the form to put the edit box on */
	if (psInit->formID == 0)
	{
		psForm = (W_FORM *)psScreen->psForm;
	}
	else
	{
		psForm = (W_FORM *)widgGetFromID(psScreen, psInit->formID);
		if (!psForm || psForm->type != WIDG_FORM)
		{
			ASSERT((FALSE,
				"widgAddEditBox: Could not find parent form from formID"));
			return FALSE;
		}
	}

	/* Create the edit box structure */
	if (!editBoxCreate(&psEdBox, psInit))
	{
		return FALSE;
	}

	/* Add it to the form */
	if (!formAddWidget(psForm, (WIDGET *)psEdBox, (W_INIT *)psInit))
	{
		return FALSE;
	}

	return TRUE;
}


/* Add a bar graph to the widget screen */
BOOL widgAddBarGraph(W_SCREEN *psScreen, W_BARINIT *psInit)
{
	W_BARGRAPH	*psBarGraph;
	W_FORM		*psForm;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgAddEditBox: Invalid screen pointer"));

	if (widgCheckIDForm((W_FORM *)psScreen->psForm,psInit->id))
	{
		ASSERT((FALSE, "widgAddBarGraph: ID number has already been used"));
		return FALSE;
	}

	/* Find the form to put the bar graph on */
	if (psInit->formID == 0)
	{
		psForm = (W_FORM *)psScreen->psForm;
	}
	else
	{
		psForm = (W_FORM *)widgGetFromID(psScreen, psInit->formID);
		if (!psForm || psForm->type != WIDG_FORM)
		{
			ASSERT((FALSE,
				"widgAddBarGraph: Could not find parent form from formID"));
			return FALSE;
		}
	}

	/* Create the bar graph structure */
	if (!barGraphCreate(&psBarGraph, psInit))
	{
		return FALSE;
	}

	/* Add it to the form */
	if (!formAddWidget(psForm, (WIDGET *)psBarGraph, (W_INIT *)psInit))
	{
		return FALSE;
	}

	return TRUE;
}


/* Add a slider to a form */
BOOL widgAddSlider(W_SCREEN *psScreen, W_SLDINIT *psInit)
{
	W_SLIDER	*psSlider;
	W_FORM		*psForm;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgAddEditBox: Invalid screen pointer"));

	if (widgCheckIDForm((W_FORM *)psScreen->psForm, psInit->id))
	{
		ASSERT((FALSE, "widgSlider: ID number has already been used"));
		return FALSE;
	}

	/* Find the form to put the slider on */
	if (psInit->formID == 0)
	{
		psForm = (W_FORM *)psScreen->psForm;
	}
	else
	{
		psForm = (W_FORM *)widgGetFromID(psScreen, psInit->formID);
		if (!psForm || psForm->type != WIDG_FORM)
		{
			ASSERT((FALSE,
				"widgAddSlider: Could not find parent form from formID"));
			return FALSE;
		}
	}

	/* Create the slider structure */
	if (!sliderCreate(&psSlider, psInit))
	{
		return FALSE;
	}

	/* Add it to the form */
	if (!formAddWidget(psForm, (WIDGET *)psSlider, (W_INIT *)psInit))
	{
		return FALSE;
	}

	return TRUE;
}


/* Delete a widget from a form */
BOOL widgDeleteFromForm(W_FORM *psForm, UDWORD id, W_CONTEXT *psContext)
{
	WIDGET		*psPrev, *psCurr, *psNext;
	W_TABFORM	*psTabForm;
	UDWORD		minor,major;
	W_MAJORTAB	*psMajor;
	W_MINORTAB	*psMinor;
	W_CONTEXT	sNewContext;

	/* Clear the last hilite if necessary */
	if ((psForm->psLastHiLite != NULL) && (psForm->psLastHiLite->id == id))
	{
		widgHiLiteLost(psForm->psLastHiLite, psContext);
		psForm->psLastHiLite = NULL;
	}

	if (psForm->style & WFORM_TABBED)
	{
		psTabForm = (W_TABFORM *)psForm;
		ASSERT((PTRVALID(psTabForm, sizeof(W_TABFORM)),
			"widgDeleteFromForm: Invalid form pointer"));

		/* loop through all the tabs */
		psMajor = psTabForm->asMajor;
		for(major=0; major < psTabForm->numMajor; major++)
		{
			psMinor = psMajor->asMinor;
			for(minor=0; minor < psMajor->numMinor; minor++)
			{
				if (psMinor->psWidgets && psMinor->psWidgets->id == id)
				{
					/* The widget is the first on this tab */
					psNext = psMinor->psWidgets->psNext;
					widgRelease(psMinor->psWidgets);
					psMinor->psWidgets = psNext;

					return TRUE;
				}
				else
				{
					for(psCurr = psMinor->psWidgets; psCurr; psCurr = psCurr->psNext)
					{
						if (psCurr->id == id)
						{
							psPrev->psNext = psCurr->psNext;
							widgRelease(psCurr);

							return TRUE;
						}
						if (psCurr->type == WIDG_FORM)
						{
							/* Recurse down to other form */
							sNewContext.psScreen = psContext->psScreen;
							sNewContext.psForm = (W_FORM *)psCurr;
							sNewContext.xOffset = psContext->xOffset - psCurr->x;
							sNewContext.yOffset = psContext->yOffset - psCurr->y;
							sNewContext.mx = psContext->mx - psCurr->x;
							sNewContext.my = psContext->my - psCurr->y;
							if (widgDeleteFromForm((W_FORM *)psCurr, id, &sNewContext))
							{
								return TRUE;
							}
						}
						psPrev = psCurr;
					}
				}
				psMinor++;
			}
			psMajor++;
		}
	}
	else
	{
		ASSERT((PTRVALID(psForm, sizeof(W_FORM)),
			"widgDeleteFromForm: Invalid form pointer"));

		/* Delete from a normal form */
		if (psForm->psWidgets && psForm->psWidgets->id == id)
		{
			/* The widget is the first in the list */
			psNext = psForm->psWidgets->psNext;
			widgRelease(psForm->psWidgets);
			psForm->psWidgets = psNext;

			return TRUE;
		}
		else
		{
			/* Search the rest of the list */
			for(psCurr = psForm->psWidgets; psCurr; psCurr = psCurr->psNext)
			{
				if (psCurr->id == id)
				{
					psPrev->psNext = psCurr->psNext;
					widgRelease(psCurr);

					return TRUE;
				}
				if (psCurr->type == WIDG_FORM)
				{
					/* Recurse down to other form */
					sNewContext.psScreen = psContext->psScreen;
					sNewContext.psForm = (W_FORM *)psCurr;
					sNewContext.xOffset = psContext->xOffset - psCurr->x;
					sNewContext.yOffset = psContext->yOffset - psCurr->y;
					sNewContext.mx = psContext->mx - psCurr->x;
					sNewContext.my = psContext->my - psCurr->y;
					if (widgDeleteFromForm((W_FORM *)psCurr, id, &sNewContext))
					{
						return TRUE;
					}
				}
				psPrev = psCurr;
			}
		}
	}

	return FALSE;
}


/* Delete a widget from the screen */
void widgDelete(W_SCREEN *psScreen, UDWORD id)
{
	W_CONTEXT	sContext;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
			"widgDelete: Invalid screen pointer"));

	/* Clear the keyboard focus if necessary */
	if ((psScreen->psFocus != NULL) && (psScreen->psFocus->id == id))
	{
		screenClearFocus(psScreen);
	}

	/* Set up the initial context */
	sContext.psScreen = psScreen;
	sContext.psForm = (W_FORM *)psScreen->psForm;
	sContext.xOffset = 0;
	sContext.yOffset = 0;
	sContext.mx = mouseX();
	sContext.my = mouseY();
	(void)widgDeleteFromForm((W_FORM *)psScreen->psForm, id, &sContext);
}


/* Initialise a form and all it's widgets */
static void widgStartForm(W_FORM *psForm)
{
	WIDGET			*psCurr;
	W_FORMGETALL	sGetAll;

	/* Initialise this form */
	formInitialise(psForm);

	/*Initialise the widgets on the form */
	formInitGetAllWidgets(psForm, &sGetAll);
	psCurr = formGetAllWidgets(&sGetAll);
	while (psCurr != NULL)
	{
		switch (psCurr->type)
		{
		case WIDG_FORM:
			widgStartForm((W_FORM *)psCurr);
			break;
		case WIDG_LABEL:
			break;
		case WIDG_BUTTON:
			buttonInitialise((W_BUTTON *)psCurr);
			break;
		case WIDG_EDITBOX:
			editBoxInitialise((W_EDITBOX *)psCurr);
			break;
		case WIDG_BARGRAPH:
			break;
		case WIDG_SLIDER:
			sliderInitialise((W_SLIDER *)psCurr);
			break;
		default:
			ASSERT((FALSE,"widgStartScreen: Unknown widget type"));
			break;
		}

		psCurr = psCurr->psNext;
		if (!psCurr)
		{
			/* Got to the end of this list see if there is another */
			psCurr = formGetAllWidgets(&sGetAll);
		}
	}
}

/* Initialise the set of widgets that make up a screen */
void widgStartScreen(W_SCREEN *psScreen)
{
	psScreen->psFocus = NULL;
	widgStartForm((W_FORM *)psScreen->psForm);
}

/* Clean up after a screen has been run */
void widgEndScreen(W_SCREEN *psScreen)
{
	(void)psScreen;
}

/* Find a widget on a form from it's id number */
static WIDGET *widgFormGetFromID(W_FORM *psForm, UDWORD id)
{
	WIDGET			*psCurr, *psFound;
	W_FORMGETALL	sGetAll;

	/* See if the form matches the ID */
	if (psForm->id == id)
	{
		return (WIDGET *)psForm;
	}

	/* Now search the widgets on the form */
	psFound = NULL;
	formInitGetAllWidgets(psForm,&sGetAll);
	psCurr = formGetAllWidgets(&sGetAll);
	while (psCurr && !psFound)
	{
		if (psCurr->id == id)
		{
			psFound = psCurr;
		}
		else if (psCurr->type == WIDG_FORM)
		{
			psFound = widgFormGetFromID((W_FORM *)psCurr, id);
		}

		psCurr = psCurr->psNext;
		if (!psCurr)
		{
			/* Got to the end of this list see if there is another */
			psCurr = formGetAllWidgets(&sGetAll);
		}
	}

	return psFound;
}

/* Find a widget in a screen from its ID number */
WIDGET *widgGetFromID(W_SCREEN *psScreen, UDWORD id)
{
	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgGetFromID: Invalid screen pointer"));

	return widgFormGetFromID((W_FORM *)psScreen->psForm, id);
}


/* Hide a widget */
void widgHide(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	ASSERT((PTRVALID(psWidget, sizeof(WIDGET)),
		"widgHide: couldn't find widget from id"));
	if (psWidget)
	{
		psWidget->style |= WIDG_HIDDEN;
	}
}


/* Reveal a widget */
void widgReveal(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	ASSERT((PTRVALID(psWidget, sizeof(WIDGET)),
		"widgReveal: couldn't find widget from id"));
	if (psWidget)
	{
		psWidget->style &= ~WIDG_HIDDEN;
	}
}


/* Get the current position of a widget */
void widgGetPos(W_SCREEN *psScreen, UDWORD id, SWORD *pX, SWORD *pY)
{
	WIDGET	*psWidget;

	/* Find the widget */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget != NULL)
	{
		*pX = psWidget->x;
		*pY = psWidget->y;
	}
	else
	{
		ASSERT((FALSE, "widgGetPos: Couldn't find widget from ID"));
		*pX = 0;
		*pY = 0;
	}
}

/* Return the ID of the widget the mouse was over this frame */
UDWORD widgGetMouseOver(W_SCREEN *psScreen)
{
	/* Don't actually need the screen parameter at the moment - but it might be
	   handy if psMouseOverWidget needs to stop being a static and moves into
	   the screen structure */
	(void)psScreen;

	if (psMouseOverWidget == NULL)
	{
		return 0;
	}

	return psMouseOverWidget->id;
}


/* Return the user data for a widget */
void *widgGetUserData(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	if (psWidget)
	{
		return psWidget->pUserData;
	}

	return NULL;
}


/* Return the user data for a widget */
UDWORD widgGetUserData2(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	if (psWidget)
	{
		return psWidget->UserData;
	}

	return 0;
}


/* Set user data for a widget */
void widgSetUserData(W_SCREEN *psScreen, UDWORD id,void *UserData)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	if (psWidget)
	{
		psWidget->pUserData = UserData;
	}
}

/* Set user data for a widget */
void widgSetUserData2(W_SCREEN *psScreen, UDWORD id,UDWORD UserData)
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	if (psWidget)
	{
		psWidget->UserData = UserData;
	}
}



/* Return the user data for the returned widget */
void *widgGetLastUserData(W_SCREEN *psScreen)
{
	/* Don't actually need the screen parameter at the moment - but it might be
	   handy if psRetWidget needs to stop being a static and moves into
	   the screen structure */
	(void)psScreen;

	if (psRetWidget)
	{
		return psRetWidget->pUserData;
	}

	return NULL;
}

/* Set tip string for a widget */
void widgSetTip( W_SCREEN *psScreen, UDWORD id, STRING *pTip )
{
	WIDGET	*psWidget;

	psWidget = widgGetFromID(psScreen, id);
	if ( psWidget )
	{
		switch (psWidget->type)
		{
			case WIDG_FORM:
			if (psWidget->style & WFORM_CLICKABLE)
			{
				((W_CLICKFORM *) psWidget)->pTip = pTip;
			}
			else if (psWidget->style & WFORM_TABBED)
			{
				ASSERT((FALSE, "widgSetTip: tabbed forms do not have a tip"));
			}
			else
			{
				ASSERT((FALSE, "widgSetTip: plain forms do not have a tip"));
			}
			break;

			case WIDG_LABEL:
			((W_LABEL *) psWidget)->pTip = pTip;
			break;

			case WIDG_BUTTON:
			((W_BUTTON *) psWidget)->pTip = pTip;
			break;

			case WIDG_BARGRAPH:
			((W_BARGRAPH *) psWidget)->pTip = pTip;
			break;

			case WIDG_SLIDER:
			((W_SLIDER *) psWidget)->pTip = pTip;
			break;

			case WIDG_EDITBOX:
			ASSERT((FALSE, "widgSetTip: edit boxes do not have a tip"));
			break;

			default:
			ASSERT((FALSE,"widgSetTip: Unknown widget type"));
			break;
		}
	}
}

/* Return which key was used to press the last returned widget */
UDWORD widgGetButtonKey(W_SCREEN *psScreen)
{
	/* Don't actually need the screen parameter at the moment - but it might be
	   handy if released needs to stop being a static and moves into
	   the screen structure */
	(void)psScreen;

	return released;
}


/* Get a button or clickable form's state */
UDWORD widgGetButtonState(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	/* Get the button */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget == NULL)
	{
		ASSERT((FALSE, "widgGetButtonState: Couldn't find button/click form from ID"));
	}
	else if (psWidget->type == WIDG_BUTTON)
	{
		return buttonGetState((W_BUTTON *)psWidget);
	}
	else if ((psWidget->type == WIDG_FORM) && (psWidget->style & WFORM_CLICKABLE))
	{
		return formGetClickState((W_CLICKFORM *)psWidget);
	}
	else
	{
		ASSERT((FALSE, "widgGetButtonState: Couldn't find button/click form from ID"));
	}
}


void widgSetButtonFlash(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	/* Get the button */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget == NULL)
	{
		ASSERT((FALSE, "widgSetButtonFlash: Couldn't find button/click form from ID"));
	}
	else if (psWidget->type == WIDG_BUTTON)
	{
		buttonSetFlash((W_BUTTON *)psWidget);
	}
	else if ((psWidget->type == WIDG_FORM) && (psWidget->style & WFORM_CLICKABLE))
	{
		formSetFlash((W_FORM *)psWidget);
	}
	else if ((psWidget->type == WIDG_EDITBOX))
	{
//		editBoxSetState((W_EDITBOX *)psWidget, state);
	}
	else
	{
		ASSERT((FALSE, "widgSetButtonFlash: Couldn't find button/click form from ID"));
	}
}


void widgClearButtonFlash(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	/* Get the button */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget == NULL)
	{
		ASSERT((FALSE, "widgSetButtonFlash: Couldn't find button/click form from ID"));
	}
	else if (psWidget->type == WIDG_BUTTON)
	{
		buttonClearFlash((W_BUTTON *)psWidget);
	}
	else if ((psWidget->type == WIDG_FORM) && (psWidget->style & WFORM_CLICKABLE))
	{
		formClearFlash((W_FORM *)psWidget);
	}
	else if ((psWidget->type == WIDG_EDITBOX))
	{
	}
	else
	{
		ASSERT((FALSE, "widgClearButtonFlash: Couldn't find button/click form from ID"));
	}
}


/* Set a button or clickable form's state */
void widgSetButtonState(W_SCREEN *psScreen, UDWORD id, UDWORD state)
{
	WIDGET	*psWidget;

	/* Get the button */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget == NULL)
	{
		ASSERT((FALSE, "widgSetButtonState: Couldn't find button/click form from ID"));
	}
	else if (psWidget->type == WIDG_BUTTON)
	{
		buttonSetState((W_BUTTON *)psWidget, state);
	}
	else if ((psWidget->type == WIDG_FORM) && (psWidget->style & WFORM_CLICKABLE))
	{
		formSetClickState((W_CLICKFORM *)psWidget, state);
	}
	else if ((psWidget->type == WIDG_EDITBOX))
	{
		editBoxSetState((W_EDITBOX *)psWidget, state);
	}
	else
	{
		ASSERT((FALSE, "widgSetButtonState: Couldn't find button/click form from ID"));
	}
}


/* Return a pointer to a buffer containing the current string of a widget.
 * NOTE: The string must be copied out of the buffer
 */
STRING *widgGetString(W_SCREEN *psScreen, UDWORD id)
{
	WIDGET	*psWidget;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgGetString: Invalid screen pointer"));

	/* Get the widget */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget != NULL)
	{
		switch (psWidget->type)
		{
		case WIDG_FORM:
			ASSERT((FALSE, "widgGetString: Forms do not have a string"));
			aStringRetBuffer[0]=0;
			break;
		case WIDG_LABEL:
			strcpy(aStringRetBuffer, ((W_LABEL *)psWidget)->aText);
			break;
		case WIDG_BUTTON:
			if (((W_BUTTON *)psWidget)->pText)
			{
				strcpy(aStringRetBuffer, ((W_BUTTON *)psWidget)->pText);
			}
			else
			{
				aStringRetBuffer[0]=0;
			}
			break;
		case WIDG_EDITBOX:
			strcpy(aStringRetBuffer, ((W_EDITBOX *)psWidget)->aText);
			break;
		case WIDG_BARGRAPH:
			ASSERT((FALSE, "widgGetString: Bar Graphs do not have a string"));
			aStringRetBuffer[0]=0;
			break;
		case WIDG_SLIDER:
			ASSERT((FALSE, "widgGetString: Sliders do not have a string"));
			aStringRetBuffer[0]=0;
			break;
		default:
			ASSERT((FALSE,"widgGetString: Unknown widget type"));
			aStringRetBuffer[0]=0;
			break;
		}
	}
	else
	{
		ASSERT((FALSE, "widgGetString: couldn't get widget from id"));
		aStringRetBuffer[0]=0;
	}

	return aStringRetBuffer;
}


/* Set the text in a widget */
void widgSetString(W_SCREEN *psScreen, UDWORD id, STRING *pText)
{
	WIDGET	*psWidget;

	ASSERT((PTRVALID(psScreen, sizeof(W_SCREEN)),
		"widgSetString: Invalid screen pointer"));

	/* Get the widget */
	psWidget = widgGetFromID(psScreen, id);
	if (psWidget != NULL)
	{
		switch (psWidget->type)
		{
		case WIDG_FORM:
			ASSERT((FALSE, "widgSetString: forms do not have a string"));
			break;
		case WIDG_LABEL:
			widgCopyString(((W_LABEL *)psWidget)->aText, pText);
			break;
		case WIDG_BUTTON:
#if W_USE_STRHEAP
			if (((W_BUTTON *)psWidget)->pText)
			{
				widgCopyString(((W_BUTTON *)psWidget)->pText, pText);
			}
			else
			{
				widgAllocCopyString(&((W_BUTTON *)psWidget)->pText, pText);
			}
#else
			((W_BUTTON *)psWidget)->pText = pText;
#endif
			break;
		case WIDG_EDITBOX:
			if (psScreen->psFocus == psWidget)
			{
				screenClearFocus(psScreen);
			}
			editBoxSetString((W_EDITBOX *)psWidget, pText);
			break;
		case WIDG_BARGRAPH:
			ASSERT((FALSE, "widgGetString: Bar graphs do not have a string"));
			break;
		case WIDG_SLIDER:
			ASSERT((FALSE, "widgGetString: Sliders do not have a string"));
			break;
		default:
			ASSERT((FALSE,"widgSetString: Unknown widget type"));
			break;
		}
	}
	else
	{
		ASSERT((FALSE, "widgSetString: couldn't get widget from id"));
	}
}


/* Call any callbacks for the widgets on a form */
static void widgProcessCallbacks(W_CONTEXT *psContext)
{
	WIDGET			*psCurr;
	W_CONTEXT		sFormContext, sWidgContext;
	SDWORD			xOrigin, yOrigin;
	W_FORMGETALL	sFormCtl;

	/* Initialise the form context */
	sFormContext.psScreen = psContext->psScreen;

	/* Initialise widget context */
	formGetOrigin(psContext->psForm, &xOrigin, &yOrigin);
	sWidgContext.psScreen = psContext->psScreen;
	sWidgContext.psForm = psContext->psForm;
	sWidgContext.mx = psContext->mx - xOrigin;
	sWidgContext.my = psContext->my - yOrigin;
	sWidgContext.xOffset = psContext->xOffset + xOrigin;
	sWidgContext.yOffset = psContext->yOffset + yOrigin;

	/* Go through all the widgets on the form */
	formInitGetAllWidgets(psContext->psForm, &sFormCtl);
	psCurr = formGetAllWidgets(&sFormCtl);
	while (psCurr)
	{
		for(;psCurr; psCurr = psCurr->psNext)
		{

			/* Skip any hidden widgets */
/*  Not sure if we want to skip hidden widgets or not ....
			if (psCurr->style & WIDG_HIDDEN)
			{
				continue;
			}*/

			/* Call the callback */
			if (psCurr->callback)
			{
				psCurr->callback(psCurr, &sWidgContext);
			}

			/* and then recurse */
			if (psCurr->type == WIDG_FORM)
			{
				sFormContext.psForm = (W_FORM *)psCurr;
				sFormContext.mx = sWidgContext.mx - psCurr->x;
				sFormContext.my = sWidgContext.my - psCurr->y;
				sFormContext.xOffset = sWidgContext.xOffset + psCurr->x;
				sFormContext.yOffset = sWidgContext.yOffset + psCurr->y;
				widgProcessCallbacks(&sFormContext);
			}
		}

		/* See if the form has any more widgets on it */
		psCurr = formGetAllWidgets(&sFormCtl);
	}
}


/* Process all the widgets on a form.
 * mx and my are the coords of the mouse relative to the form origin.
 */
static void widgProcessForm(W_CONTEXT *psContext)
{
	WIDGET		*psCurr, *psOver;
	SDWORD		mx,my, omx,omy, xOffset,yOffset, xOrigin,yOrigin;
	W_FORM		*psForm;
	W_CONTEXT	sFormContext, sWidgContext;

	/* Note current form */
	psForm = psContext->psForm;

//	if(psForm->disableChildren == TRUE) {
//		return;
//	}

	/* Note the current mouse position */
	mx = psContext->mx;
	my = psContext->my;

	/* Note the current offset */
	xOffset = psContext->xOffset;
	yOffset = psContext->yOffset;

	/* Initialise the form context */
	sFormContext.psScreen = psContext->psScreen;

	/* Initialise widget context */
	formGetOrigin(psForm, &xOrigin, &yOrigin);
	sWidgContext.psScreen = psContext->psScreen;
	sWidgContext.psForm = psForm;
	sWidgContext.mx = mx - xOrigin;
	sWidgContext.my = my - yOrigin;
	sWidgContext.xOffset = xOffset + xOrigin;
	sWidgContext.yOffset = yOffset + yOrigin;

	/* Process the form's widgets */
	psOver = NULL;
	for(psCurr = formGetWidgets(psForm); psCurr; psCurr = psCurr->psNext)
	{
		/* Skip any hidden widgets */
		if (psCurr->style & WIDG_HIDDEN)
		{
			continue;
		}

		if (psCurr->type == WIDG_FORM)
		{
			/* Found a sub form, so set up the context */
			sFormContext.psForm = (W_FORM *)psCurr;
			sFormContext.mx = mx - psCurr->x - xOrigin;
			sFormContext.my = my - psCurr->y - yOrigin;
			sFormContext.xOffset = xOffset + psCurr->x + xOrigin;
			sFormContext.yOffset = yOffset + psCurr->y + yOrigin;

			/* Process it */
			widgProcessForm(&sFormContext);
		}
		else
		{
			/* Run the widget */
			widgRun(psCurr, &sWidgContext);
		}
	}

	/* Now check for mouse clicks */
	omx = mx - xOrigin;
	omy = my - yOrigin;
	if (mx >= 0 && mx <= psForm->width &&
		my >= 0 && my <= psForm->height)
	{
   		/* Update for the origin */

   		/* Mouse is over the form - is it over any of the widgets */
   		for(psCurr = formGetWidgets(psForm); psCurr; psCurr = psCurr->psNext)
   		{
   			/* Skip any hidden widgets */
   			if (psCurr->style & WIDG_HIDDEN)
   			{
   				continue;
   			}

   			if (omx >= psCurr->x &&
   				omy >= psCurr->y &&
   				omx <= psCurr->x + psCurr->width &&
   				omy <= psCurr->y + psCurr->height)
   			{
   				/* Note the widget the mouse is over */
   				if (!psMouseOverWidget)
   				{
   					psMouseOverWidget = (WIDGET *)psCurr;
   				}
   				psOver = psCurr;

   				/* Don't check the widgets if it is a clickable form */
   				if (!(psForm->style & WFORM_CLICKABLE))
   				{
   					if (pressed != WKEY_NONE && psCurr->type != WIDG_FORM)
   					{
   						/* Tell the widget it has been clicked */
   						widgClicked(psCurr, pressed, &sWidgContext);
   					}
   					if (released != WKEY_NONE && psCurr->type != WIDG_FORM)
   					{
   						/* Tell the widget the mouse button has gone up */
   						widgReleased(psCurr, released, &sWidgContext);
   					}
   				}
   			}
   		}
   		/* Note that the mouse is over this form */
   		if (!psMouseOverWidget)
   		{
   			psMouseOverWidget = (WIDGET *)psForm;
   		}

		/* Only send the Clicked or Released messages if a widget didn't get the message */
		if (pressed != WKEY_NONE &&
			(psOver == NULL || (psForm->style & WFORM_CLICKABLE)))
		{
			/* Tell the form it has been clicked */
			widgClicked((WIDGET *)psForm, pressed, psContext);
		}
		if (released != WKEY_NONE &&
			(psOver == NULL || (psForm->style & WFORM_CLICKABLE)))
		{
			/* Tell the form the mouse button has gone up */
			widgReleased((WIDGET *)psForm, released, psContext);
		}
	}

	/* See if the mouse has moved onto or off a widget */
	if (psForm->psLastHiLite != psOver)
	{
		if (psOver != NULL)
		{
			widgHiLite(psOver, &sWidgContext);
		}
		if (psForm->psLastHiLite != NULL)
		{
			widgHiLiteLost(psForm->psLastHiLite, &sWidgContext);
		}
		psForm->psLastHiLite = psOver;
	}

	/* Run this form */
	widgRun((WIDGET *)psForm, psContext);
}



/* Execute a set of widgets for one cycle.
 * Return the id of the widget that was activated, or 0 for none.
 */
UDWORD widgRunScreen(W_SCREEN *psScreen)
{
	W_CONTEXT	sContext;
	UDWORD		returnID;

	psRetWidget = NULL;

	// Note which keys have been pressed
	pressed = WKEY_NONE;
	if(getWidgetsStatus())
	{

		if (mousePressed(MOUSE_LMB))
		{
			pressed = WKEY_PRIMARY;
		}
		else if (mousePressed(MOUSE_RMB))
		{
			pressed = WKEY_SECONDARY;
		}
		released = WKEY_NONE;
		if (mouseReleased(MOUSE_LMB))
		{
			released = WKEY_PRIMARY;
		}
		else if (mouseReleased(MOUSE_RMB))
		{
			released = WKEY_SECONDARY;
		}
	}
	/* Initialise the context */
	sContext.psScreen = psScreen;
	sContext.psForm = (W_FORM *)psScreen->psForm;
	sContext.xOffset = 0;
	sContext.yOffset = 0;
	sContext.mx = mouseX();
	sContext.my = mouseY();
	psMouseOverWidget = NULL;

	/* Process the screen's widgets */
	widgProcessForm(&sContext);

	/* Process any user callback functions */
	widgProcessCallbacks(&sContext);

	/* Return the ID of a pressed button or finished edit box if any */
	if (psRetWidget)
	{
		returnID = psRetWidget->id;
	}
	else
	{
		returnID = 0;
	}
	return returnID;
}


/* Set the id number for widgRunScreen to return */
void widgSetReturn(WIDGET *psWidget)
{
	psRetWidget = psWidget;
}


/* Display the widgets on a form */
static void _widgDisplayForm(W_FORM *psForm, UDWORD xOffset, UDWORD yOffset)
{
	WIDGET	*psCurr;
	SDWORD	xOrigin,yOrigin;

	/* Display the form */
	psForm->display((WIDGET *)psForm, xOffset, yOffset, psForm->aColours);
	if(psForm->disableChildren == TRUE) {
		return;
	}

	/* Update the offset from the current form's position */
	formGetOrigin(psForm, &xOrigin, &yOrigin);
	xOffset += psForm->x + xOrigin;
	yOffset += psForm->y + yOrigin;

	/* If this is a clickable form, the widgets on it have to move when it's down */
	if(!(psForm->style & WFORM_NOCLICKMOVE)) {
		if ((psForm->style & WFORM_CLICKABLE) &&
			(((W_CLICKFORM *)psForm)->state &
					(WCLICK_DOWN | WCLICK_LOCKED | WCLICK_CLICKLOCK)))
		{
			xOffset += 1;
			yOffset += 1;
		}
	}

	/* Display the widgets on the form */
	for(psCurr = formGetWidgets(psForm); psCurr; psCurr = psCurr->psNext)
	{
		/* Skip any hidden widgets */
		if (psCurr->style & WIDG_HIDDEN)
		{
			continue;
		}

		if (psCurr->type == WIDG_FORM)
		{
			widgDisplayForm((W_FORM *)psCurr, xOffset, yOffset);
		}
		else
		{
			psCurr->display(psCurr, xOffset, yOffset, psForm->aColours);
		}
	}
}



static void widgDisplayForm(W_FORM *psForm, UDWORD xOffset, UDWORD yOffset)
{
	_widgDisplayForm(psForm,xOffset,yOffset);
}



/* Display the screen's widgets in their current state
 * (Call after calling widgRunScreen, this allows the input
 *  processing to be seperated from the display of the widgets).
 */
void widgDisplayScreen(W_SCREEN *psScreen)
{
	/* Display the widgets */
	widgDisplayForm((W_FORM *)psScreen->psForm, 0,0);

	/* Display the tool tip if there is one */
	tipDisplay();
}


/* Set the keyboard focus for the screen */
void screenSetFocus(W_SCREEN *psScreen, WIDGET *psWidget)
{
	if (psScreen->psFocus != NULL)
	{
		widgFocusLost(psScreen->psFocus);
	}
	psScreen->psFocus = psWidget;
}


/* Clear the keyboard focus */
void screenClearFocus(W_SCREEN *psScreen)
{
	if (psScreen->psFocus != NULL)
	{
		widgFocusLost(psScreen->psFocus);
		psScreen->psFocus = NULL;
	}
}

/* Call the correct function for loss of focus */
void widgFocusLost(WIDGET *psWidget)
{
	switch (psWidget->type)
	{
	case WIDG_FORM:
		break;
	case WIDG_LABEL:
		break;
	case WIDG_BUTTON:
		break;
	case WIDG_EDITBOX:
		editBoxFocusLost((W_EDITBOX *)psWidget);
		break;
	case WIDG_BARGRAPH:
		break;
	case WIDG_SLIDER:
		break;
	default:
		ASSERT((FALSE,"widgFocusLost: Unknown widget type"));
		break;
	}
}

/* Call the correct function for mouse over */
void widgHiLite(WIDGET *psWidget, W_CONTEXT *psContext)
{
	(void)psContext;
	switch (psWidget->type)
	{
	case WIDG_FORM:
		formHiLite((W_FORM *)psWidget, psContext);
		break;
	case WIDG_LABEL:
		labelHiLite((W_LABEL *)psWidget, psContext);
		break;
	case WIDG_BUTTON:
		buttonHiLite((W_BUTTON *)psWidget, psContext);
		break;
	case WIDG_EDITBOX:
		editBoxHiLite((W_EDITBOX *)psWidget);
		break;
	case WIDG_BARGRAPH:
		barGraphHiLite((W_BARGRAPH *)psWidget, psContext);
		break;
	case WIDG_SLIDER:
		sliderHiLite((W_SLIDER *)psWidget);
		break;
	default:
		ASSERT((FALSE,"widgHiLite: Unknown widget type"));
		break;
	}
}


/* Call the correct function for mouse moving off */
void widgHiLiteLost(WIDGET *psWidget, W_CONTEXT *psContext)
{
	(void)psContext;
	switch (psWidget->type)
	{
	case WIDG_FORM:
		formHiLiteLost((W_FORM *)psWidget, psContext);
		break;
	case WIDG_LABEL:
		labelHiLiteLost((W_LABEL *)psWidget);
		break;
	case WIDG_BUTTON:
		buttonHiLiteLost((W_BUTTON *)psWidget);
		break;
	case WIDG_EDITBOX:
		editBoxHiLiteLost((W_EDITBOX *)psWidget);
		break;
	case WIDG_BARGRAPH:
		barGraphHiLiteLost((W_BARGRAPH *)psWidget);
		break;
	case WIDG_SLIDER:
		sliderHiLiteLost((W_SLIDER *)psWidget);
		break;
	default:
		ASSERT((FALSE,"widgHiLiteLost: Unknown widget type"));
		break;
	}
}

/* Call the correct function for mouse pressed */
static void widgClicked(WIDGET *psWidget, UDWORD key, W_CONTEXT *psContext)
{
	switch (psWidget->type)
	{
	case WIDG_FORM:
		formClicked((W_FORM *)psWidget, key);
		break;
	case WIDG_LABEL:
		break;
	case WIDG_BUTTON:
		buttonClicked((W_BUTTON *)psWidget, key);
		break;
	case WIDG_EDITBOX:
		editBoxClicked((W_EDITBOX *)psWidget, psContext);
		break;
	case WIDG_BARGRAPH:
		break;
	case WIDG_SLIDER:
		sliderClicked((W_SLIDER *)psWidget, psContext);
		break;
	default:
		ASSERT((FALSE,"widgClicked: Unknown widget type"));
		break;
	}
}


/* Call the correct function for mouse released */
static void widgReleased(WIDGET *psWidget, UDWORD key, W_CONTEXT *psContext)
{
	switch (psWidget->type)
	{
	case WIDG_FORM:
		formReleased((W_FORM *)psWidget, key, psContext);
		break;
	case WIDG_LABEL:
		break;
	case WIDG_BUTTON:
		buttonReleased((W_BUTTON *)psWidget, key);
		break;
	case WIDG_EDITBOX:
		editBoxReleased((W_EDITBOX *)psWidget);
		break;
	case WIDG_BARGRAPH:
		break;
	case WIDG_SLIDER:
		sliderReleased((W_SLIDER *)psWidget);
		break;
	default:
		ASSERT((FALSE,"widgReleased: Unknown widget type"));
		break;
	}
}


/* Call the correct function to run a widget */
static void widgRun(WIDGET *psWidget, W_CONTEXT *psContext)
{
	switch (psWidget->type)
	{
	case WIDG_FORM:
		formRun((W_FORM *)psWidget, psContext);
		break;
	case WIDG_LABEL:
		break;
	case WIDG_BUTTON:
		buttonRun((W_BUTTON *)psWidget);
		break;
	case WIDG_EDITBOX:
		editBoxRun((W_EDITBOX *)psWidget, psContext);
		break;
	case WIDG_BARGRAPH:
		break;
	case WIDG_SLIDER:
		sliderRun((W_SLIDER *)psWidget, psContext);
		break;
	default:
		ASSERT((FALSE,"widgRun: Unknown widget type"));
		break;
	}
}

void WidgSetAudio(WIDGET_AUDIOCALLBACK Callback,SWORD HilightID,SWORD ClickedID)
{
	AudioCallback = Callback;
	HilightAudioID = HilightID;
	ClickedAudioID = ClickedID;
}


WIDGET_AUDIOCALLBACK WidgGetAudioCallback(void)
{
	return AudioCallback;
}


SWORD WidgGetHilightAudioID(void)
{
	return HilightAudioID;
}


SWORD WidgGetClickedAudioID(void)
{
	return ClickedAudioID;
}


void	setWidgetsStatus(BOOL var)
{
	bWidgetsActive = var;
}

BOOL	getWidgetsStatus( void )
{
	return(bWidgetsActive);
}