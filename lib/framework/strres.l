%{
/*
 * StrRes.l
 *
 * Lex file for parsing string resource files
 */

#include <stdio.h>

/* Allow frame header files to be singly included */
#define FRAME_LIB_INCLUDE

#include <string.h>
#include "types.h"
#include "debug.h"
#include "mem.h"
#include "heap.h"
#include "treap.h"
#include "StrRes.h"
#include "StrResLY.h"

/* Get the Yacc definitions */
#include "StrRes_y.h"

/* Turn off a couple of warnings that the lex generated code gives */
#pragma warning ( disable : 4102 4305 )

/* Maximum length for any TEXT value */
#define YYLMAX	255

/* Store for any string values */
extern STRING aText[TEXT_BUFFERS][YYLMAX];
static UDWORD currText=0;

// Note if in a comment
static BOOL inComment;

/* Pointer to the input buffer */
static UBYTE *pInputBuffer = NULL;
static UBYTE *pEndBuffer = NULL;

#undef strres_getc
#define strres_getc() (pInputBuffer != pEndBuffer ? *(pInputBuffer++) : EOF)

%}

%x COMMENT
%x QUOTE
%x SLCOMMENT

%%

	/* Match text values */
[a-zA-Z][-0-9_a-zA-Z]*		{
								strcpy(aText[currText], strres_text);
								strres_lval.sval = aText[currText];
								currText = (currText + 1) % TEXT_BUFFERS;
								return TEXT;
							}

	/* Match quoted text */
\"							{ BEGIN QUOTE; }
<QUOTE>\"					{ BEGIN 0; }
<QUOTE>\n					{ strres_error("Unexpected end of line in string"); }
<QUOTE>[^\"\n]*				{
								strcpy(aText[currText], strres_text);
								strres_lval.sval = aText[currText];
								currText = (currText + 1) % TEXT_BUFFERS;
								return QTEXT;
							}

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
.							return strres_text[0];

%%

/* Set the current input buffer for the lexer */
void strresSetInputBuffer(UBYTE *pBuffer, UDWORD size)
{
	pInputBuffer = pBuffer;
	pEndBuffer = pBuffer + size;

	/* Reset the lexer incase it's been used before */
	strres__reset();
}

void strresGetErrorData(int *pLine, char **ppText)
{
	*pLine = strres_lineno;
	*ppText = strres_text;
}

int strres_wrap(void)
{
	if (inComment)
	{
		DBERROR(("Warning: reched end of file in a comment"));
	}

	return 1;
}

