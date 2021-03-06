%{
/*
 * Level.l
 *
 * lexer for loading level description files
 *
 */

#ifdef WIN32
#include <stdio.h>

#include "Frame.h"

#include "Levels.h"
#include "LevelInt.h"

/* Turn off a couple of warnings that the lex generated code gives */
#pragma warning ( disable : 4102 4305 )

/* Maximum length for any TEXT value */
#define YYLMAX	255

/* Store for any string values */
static STRING aText[YYLMAX];

// Note if we are in a comment
static BOOL inComment = FALSE;

/* Pointer to the input buffer */
static UBYTE *pInputBuffer = NULL;
static UBYTE *pEndBuffer = NULL;

#undef lev_getc
#define lev_getc() (pInputBuffer != pEndBuffer ? *(pInputBuffer++) : EOF)

%}

%x COMMENT
%x SLCOMMENT
%x QUOTE

%%

	/* Keywords */
level						return LTK_LEVEL;
players						return LTK_PLAYERS;
type						return LTK_TYPE;
data						return LTK_DATA;
game						return LTK_GAME;
campaign					return LTK_CAMPAIGN;
camstart					return LTK_CAMSTART;
camchange					return LTK_CAMCHANGE;
dataset						return LTK_DATASET;
expand						return LTK_EXPAND;
expand_limbo				return LTK_EXPAND_LIMBO;
between						return LTK_BETWEEN;
miss_keep					return LTK_MKEEP;
miss_keep_limbo				return LTK_MKEEP_LIMBO;
miss_clear					return LTK_MCLEAR;

	/* Match text values */
[a-zA-Z][-0-9_a-zA-Z]*		{
								strcpy(aText, lev_text);
								pLevToken = aText;
								return LTK_IDENT;
							}

	/* Match quoted text */
\"							{ BEGIN QUOTE; }
<QUOTE>\"					{ BEGIN 0; }
<QUOTE>\n					{ levError("Unexpected end of line in string"); }
<QUOTE>[^\"]*				{
								strcpy(aText, lev_text);
								pLevToken = aText;
								return LTK_STRING;
							}

	/* Match integer numbers */
-?[0-9]+					{ levVal = atol(lev_text); return LTK_INTEGER; }

	/* Skip white space */
[ \t\n\x0d\x0a]						;

	/* Strip comments */
"/*"						{ inComment=TRUE; BEGIN COMMENT; }
<COMMENT>"*/" |
<COMMENT>"*/"\n				{ inComment=FALSE; BEGIN 0; }
<COMMENT>. |
<COMMENT>\n					;

	/* Strip single line comments */
"//"						{ BEGIN SLCOMMENT; }
<SLCOMMENT>\n				{ BEGIN 0; }
<SLCOMMENT>[^\n]*			;

	/* Match anything that's been missed and pass it as a char */
.							return lev_text[0];

%%

/* Set the current input buffer for the lexer */
void levSetInputBuffer(UBYTE *pBuffer, UDWORD size)
{
	pInputBuffer = pBuffer;
	pEndBuffer = pBuffer + size;

	/* Reset the lexer incase it's been used before */
	lev__reset();
}

void levGetErrorData(int *pLine, char **ppText)
{
	*pLine = lev_lineno;
	*ppText = lev_text;
}

int lev_wrap(void)
{
	if (inComment)
	{
		DBERROR(("Warning: reched end of file in a comment"));
	}
	return 1;
}

