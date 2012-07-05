%{
/*
 * script.y
 *
 * The yacc grammar for the scipt files.
 */

#ifdef PSX
/* A few definitions so the yacc generated code will compile on the PSX.
 * These shouldn't actually be used by any code that is run on the PSX, it
 * just keeps the compiler happy.
 */
//static int printf(char* c, ...)
//{
//	return 0;
//}
#endif

#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "Frame.h"
#include "Interp.h"
#include "Parse.h"
#include "Script.h"

/* Error return codes for code generation functions */
typedef enum _code_error
{
	CE_OK,				// No error
	CE_MEMORY,			// Out of memory
	CE_PARSE			// A parse error occured
} CODE_ERROR;

/* Turn off a couple of warnings that the yacc generated code gives */
#pragma warning ( disable : 4305 4102)

/* Pointer to the compiled code */
static SCRIPT_CODE	*psFinalProg=NULL;

/* Pointer to current block of compiled code */
static CODE_BLOCK	*psCurrBlock=NULL;

/* Pointer to current block of conditional code */
static COND_BLOCK	*psCondBlock=NULL;

/* Pointer to current block of object variable code */
static OBJVAR_BLOCK	*psObjVarBlock=NULL;

/* Pointer to current block of compiled parameter code */
static PARAM_BLOCK	*psCurrPBlock=NULL;

/* Pointer into the current code block */
static UDWORD		*ip;

/* Pointer to current parameter declaration block */
static PARAM_DECL	*psCurrParamDecl=NULL;

/* Pointer to current trigger subdeclaration */
static TRIGGER_DECL	*psCurrTDecl=NULL;

/* Pointer to current variable subdeclaration */
static VAR_DECL		*psCurrVDecl=NULL;

/* Pointer to the current variable identifier declaration */
static VAR_IDENT_DECL	*psCurrVIdentDecl=NULL;

/* Pointer to the current array access block */
static ARRAY_BLOCK		*psCurrArrayBlock=NULL;

/* Return code from code generation functions */
static CODE_ERROR	codeRet;

/* The list of global variables */
static VAR_SYMBOL	*psGlobalVars=NULL;

/* The list of global arrays */
static VAR_SYMBOL	*psGlobalArrays=NULL;

/* The list of current local variables */
static VAR_SYMBOL	*psLocalVars=NULL;

/* The list of function definitions */
static FUNC_SYMBOL	*psFunctions=NULL;

/* The current object variable context */
static INTERP_TYPE	objVarContext = 0;

/* Control whether debug info is generated */
static BOOL			genDebugInfo = TRUE;

/* Currently defined triggers */
static TRIGGER_SYMBOL	*psTriggers;
static UDWORD			numTriggers;

/* Currently defined events */
static EVENT_SYMBOL		*psEvents;
static UDWORD			numEvents;

/* This is true when local variables are being defined.
 * (So local variables can have the same name as global ones)
 */
static BOOL localVariableDef=FALSE;

/* The identifier for the current script function being defined */
static STRING *pCurrFuncIdent=NULL;

/* A temporary store for a line number - used when
 * generating debugging info for functions, conditionals and loops.
 */
static UDWORD		debugLine;

/* The table of user types */
TYPE_SYMBOL		*asScrTypeTab;

/* The table of instinct function type definitions */
FUNC_SYMBOL		*asScrInstinctTab;

/* The table of external variables and their access functions */
VAR_SYMBOL		*asScrExternalTab;

/* The table of object variables and their access functions. */
VAR_SYMBOL		*asScrObjectVarTab;

/* The table of constant variables */
CONST_SYMBOL	*asScrConstantTab;

/* The table of callback triggers */
CALLBACK_SYMBOL	*asScrCallbackTab;

/****************************************************************************************
 *
 * Code Block Macros
 *
 * These macros are used to allocate and free the different types of code
 * block used within the compiler
 */


/* What the macro should do if it has an allocation error.
 * This is different depending on whether the macro is used
 * in a function, or in a rule body.
 *
 * This definition is used within the code generation functions
 * and is then changed for use within a rule body.
 */
#define ALLOC_ERROR_ACTION return CE_MEMORY

/* Macro to allocate a program structure */
#define ALLOC_PROG(psProg, codeSize, pAICode, numGlobs, numArys, numTrigs, numEvnts) \
	(psProg) = (SCRIPT_CODE *)MALLOC(sizeof(SCRIPT_CODE)); \
	if ((psProg) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psProg)->pCode = (UDWORD *)MALLOC(codeSize); \
	if ((psProg)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	if (numGlobs > 0) \
	{ \
		(psProg)->pGlobals = (INTERP_TYPE *)MALLOC(sizeof(INTERP_TYPE) * (numGlobs)); \
		if ((psProg)->pGlobals == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
	} \
	else \
	{ \
		(psProg)->pGlobals = NULL; \
	} \
	if (numArys > 0) \
	{ \
		(psProg)->psArrayInfo = (ARRAY_DATA *)MALLOC(sizeof(ARRAY_DATA) * (numArys)); \
		if ((psProg)->psArrayInfo == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
	} \
	else \
	{ \
		(psProg)->psArrayInfo = NULL; \
	} \
	(psProg)->numArrays = (UWORD)(numArys); \
	if ((numTrigs) > 0) \
	{ \
		(psProg)->pTriggerTab = MALLOC(sizeof(UWORD) * ((numTrigs) + 1)); \
		if ((psProg)->pTriggerTab == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
		(psProg)->psTriggerData = MALLOC(sizeof(TRIGGER_DATA) * (numTrigs)); \
		if ((psProg)->psTriggerData == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
	} \
	else \
	{ \
		(psProg)->pTriggerTab = NULL; \
		(psProg)->psTriggerData = NULL; \
	} \
	(psProg)->pEventTab = MALLOC(sizeof(UWORD) * ((numEvnts) + 1)); \
	if ((psProg)->pEventTab == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psProg)->pEventLinks = MALLOC(sizeof(SWORD) * (numEvnts)); \
	if ((psProg)->pEventLinks == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psProg)->numGlobals = (UWORD)(numGlobs); \
	(psProg)->numTriggers = (UWORD)(numTriggers); \
	(psProg)->numEvents = (UWORD)(numEvnts); \
	(psProg)->size = (codeSize)

/* Macro to allocate a code block */
#define ALLOC_BLOCK(psBlock, blockSize) \
	(psBlock) = (CODE_BLOCK *)MALLOC(sizeof(CODE_BLOCK)); \
	if ((psBlock) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->pCode = (UDWORD *)MALLOC(blockSize); \
	if ((psBlock)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		FREE((psBlock)); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->size = blockSize

/* Macro to free a code block */
#define FREE_BLOCK(psBlock) \
	FREE((psBlock)->pCode); \
	FREE((psBlock))

/* Macro to allocate a parameter block */
#define ALLOC_PBLOCK(psBlock, codeSize, paramSize) \
	(psBlock) = (PARAM_BLOCK *)MALLOC(sizeof(PARAM_BLOCK)); \
	if ((psBlock) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->pCode = (UDWORD *)MALLOC(codeSize); \
	if ((psBlock)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		FREE((psBlock)); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->aParams = (INTERP_TYPE *)MALLOC(sizeof(INTERP_TYPE) * (paramSize)); \
	if ((psBlock)->aParams == NULL) \
	{ \
		scr_error("Out of memory"); \
		FREE((psBlock)->pCode); \
		FREE((psBlock)); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->size = (codeSize); \
	(psBlock)->numParams = (paramSize)

/* Macro to free a parameter block */
#define FREE_PBLOCK(psBlock) \
	FREE((psBlock)->pCode); \
	FREE((psBlock)->aParams); \
	FREE((psBlock))

/* Macro to allocate a parameter declaration block */
#define ALLOC_PARAMDECL(psPDecl, num) \
	(psPDecl) = (PARAM_DECL *)MALLOC(sizeof(PARAM_DECL)); \
	if ((psPDecl) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psPDecl)->aParams = (INTERP_TYPE *)MALLOC(sizeof(INTERP_TYPE) * (num)); \
	if ((psPDecl)->aParams == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psPDecl)->numParams = (num)

/* Macro to free a parameter declaration block */
#define FREE_PARAMDECL(psPDecl) \
	FREE((psPDecl)->aParams); \
	FREE((psPDecl))

/* Macro to allocate a conditional block */
#define ALLOC_CONDBLOCK(psCB, num, blockSize) \
	(psCB) = (COND_BLOCK *)MALLOC(sizeof(COND_BLOCK)); \
	if ((psCB) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psCB)->aOffsets = (UDWORD *)MALLOC(sizeof(SDWORD) * (num)); \
	if ((psCB)->aOffsets == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psCB)->pCode = (UDWORD *)MALLOC(blockSize); \
	if ((psCB)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psCB)->size = (blockSize); \
	(psCB)->numOffsets = (num)

/* Macro to free a conditional block */
#define FREE_CONDBLOCK(psCB) \
	FREE((psCB)->aOffsets); \
	FREE((psCB)->pCode); \
	FREE(psCB)

/* Macro to allocate a code block */
#define ALLOC_USERBLOCK(psBlock, blockSize) \
	(psBlock) = (USER_BLOCK *)MALLOC(sizeof(USER_BLOCK)); \
	if ((psBlock) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->pCode = (UDWORD *)MALLOC(blockSize); \
	if ((psBlock)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		FREE((psBlock)); \
		ALLOC_ERROR_ACTION; \
	} \
	(psBlock)->size = blockSize

/* Macro to free a code block */
#define FREE_USERBLOCK(psBlock) \
	FREE((psBlock)->pCode); \
	FREE((psBlock))

/* Macro to allocate an object variable block */
#define ALLOC_OBJVARBLOCK(psOV, blockSize, psVar) \
	(psOV) = (OBJVAR_BLOCK *)MALLOC(sizeof(OBJVAR_BLOCK)); \
	if ((psOV) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psOV)->pCode = (UDWORD *)MALLOC(blockSize); \
	if ((psOV)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psOV)->size = (blockSize); \
	(psOV)->psObjVar = (psVar)

/* Macro to free an object variable block */
#define FREE_OBJVARBLOCK(psOV) \
	FREE((psOV)->pCode); \
	FREE(psOV)

/* Macro to allocate an array variable block */
#define ALLOC_ARRAYBLOCK(psAV, blockSize, psVar) \
	(psAV) = (ARRAY_BLOCK *)MALLOC(sizeof(ARRAY_BLOCK)); \
	if ((psAV) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psAV)->pCode = (UDWORD *)MALLOC(blockSize); \
	if ((psAV)->pCode == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psAV)->size = (blockSize); \
	(psAV)->dimensions = 1; \
	(psAV)->psArrayVar = (psVar)

/* Macro to free an object variable block */
#define FREE_ARRAYBLOCK(psAV) \
	FREE((psAV)->pCode); \
	FREE(psAV)

/* Allocate a trigger subdecl */
#define ALLOC_TSUBDECL(psTSub, blockType, blockSize, blockTime) \
	(psTSub) = MALLOC(sizeof(TRIGGER_DECL)); \
	if ((psTSub) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	(psTSub)->type = (blockType); \
	(psTSub)->time = (blockTime); \
	if ((blockSize) > 0) \
	{ \
		(psTSub)->pCode = MALLOC(blockSize); \
		if ((psTSub)->pCode == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
		(psTSub)->size = (blockSize); \
	} \
	else \
	{ \
		(psTSub)->pCode = NULL; \
		(psTSub)->size = 0; \
	}

/* Free a trigger subdecl */
#define FREE_TSUBDECL(psTSub) \
	if ((psTSub)->pCode) \
	{ \
		FREE((psTSub)->pCode); \
	} \
	FREE(psTSub)

/* Allocate a variable declaration block */
#define ALLOC_VARDECL(psDcl) \
	(psDcl)=MALLOC(sizeof(VAR_DECL)); \
	if ((psDcl) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	}

/* Free a variable declaration block */
#define FREE_VARDECL(psDcl) \
	FREE(psDcl)

/* Allocate a variable declaration block */
#define ALLOC_VARIDENTDECL(psDcl, ident, dim) \
	(psDcl)=MALLOC(sizeof(VAR_IDENT_DECL)); \
	if ((psDcl) == NULL) \
	{ \
		scr_error("Out of memory"); \
		ALLOC_ERROR_ACTION; \
	} \
	if ((ident) != NULL) \
	{ \
		(psDcl)->pIdent=MALLOC(strlen(ident)+1); \
		if ((psDcl)->pIdent == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
		strcpy((psDcl)->pIdent, (ident)); \
	} \
	else \
	{ \
		(psDcl)->pIdent = NULL; \
	} \
	(psDcl)->dimensions = (dim)

/* Free a variable declaration block */
#define FREE_VARIDENTDECL(psDcl) \
	FREE(psDcl)

/****************************************************************************************
 *
 * Code block manipulation macros.
 *
 * These are used to copy chunks of code from one block to another
 * or to insert opcodes or other values into a code block.
 * All the macros use the ip parameter.  This is a pointer into the code
 * block that is incremented by the macro. This ensures that it always points
 * to the next free space in the code block.
 */


/* Macro to store an opcode in a code block */
#define PUT_OPCODE(ip, opcode) \
	*ip = (opcode) << OPCODE_SHIFT; \
	ip += 1

/* Macro to put a packed opcode in a code block */
#define PUT_PKOPCODE(ip, opcode, data) \
	*ip = ((UDWORD)(data)) & OPCODE_DATAMASK; \
	*ip = (((UDWORD)(opcode)) << OPCODE_SHIFT) | (*ip); \
	ip += 1

/* Macro to store the data part of an INTERP_VAL in a code block */
#define PUT_DATA(ip, data) \
	*ip = (UDWORD)(data); \
	ip += 1

/* Macros to store a value in a code block */
#define PUT_BOOL(ip, value) \
	((INTERP_VAL *)(ip))->type = VAL_BOOL; \
	((INTERP_VAL *)(ip))->v.bval = (value); \
	((INTERP_VAL *)(ip)) += 1
#define PUT_INT(ip, value) \
	((INTERP_VAL *)(ip))->type = VAL_INT; \
	((INTERP_VAL *)(ip))->v.ival = (value); \
	((INTERP_VAL *)(ip)) += 1
/*#define PUT_FLOAT(ip, value) \
	((INTERP_VAL *)(ip))->type = VAL_FLOAT; \
	((INTERP_VAL *)(ip))->v.fval = (value); \
	((INTERP_VAL *)(ip)) += 1*/
#define PUT_STRING(ip, value) \
	((INTERP_VAL *)(ip))->type = VAL_STRING; \
	((INTERP_VAL *)(ip))->v.sval = (value); \
	((INTERP_VAL *)(ip)) += 1
/*#define PUT_OBJECT(ip, value) \
	((INTERP_VAL *)(ip))->type = VAL_OBJECT; \
	((INTERP_VAL *)(ip))->v.oval = (value); \
	((INTERP_VAL *)(ip)) += 1*/

/* Macro to store a function pointer in a code block */
#define PUT_FUNC(ip, func) \
	*ip = (UDWORD)func; \
	ip += 1

/* Macro to store a function pointer in a code block */
#define PUT_VARFUNC(ip, func) \
	*ip = (UDWORD)func; \
	ip += 1

/* Macro to store a reference to a script function. */
/* This will be converted to a program location at the link stage */
#define PUT_SCRIPTFUNC(ip, func) \
	*ip = (UDWORD)func; \
	ip += 1

/* Macro to store a variable index number in a code block */
#define PUT_INDEX(ip, index) \
	*ip = (index); \
	ip++

/* Macro to store a jump offset in a code block */
#define PUT_OFFSET(ip, offset) \
	*((SDWORD *)ip) = (offset); \
	ip++

/* Macro to copy a code block into another code block */
#define PUT_BLOCK(ip, psBlock) \
	memcpy(ip, (psBlock)->pCode, (psBlock)->size); \
	ip = (UDWORD *)(((UBYTE *)ip) + (psBlock)->size)

/***********************************************************************************
 *
 * Debugging information macros
 *
 * These macros are only used to generate debugging information for scripts.
 */

/* Macro to allocate debugging info for a CODE_BLOCK or a COND_BLOCK */
#define ALLOC_DEBUG(psBlock, num) \
	if (genDebugInfo) \
	{ \
		(psBlock)->psDebug = (SCRIPT_DEBUG *)MALLOC(sizeof(SCRIPT_DEBUG) * (num)); \
		if ((psBlock)->psDebug == NULL) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
		memset((psBlock)->psDebug, 0, sizeof(SCRIPT_DEBUG) * (num));\
		(psBlock)->debugEntries = (UWORD)(num); \
	} \
	else \
	{ \
		(psBlock)->psDebug = NULL; \
		(psBlock)->debugEntries = 0; \
	}

/* Macro to free debugging info */
#define FREE_DEBUG(psBlock) \
	if (genDebugInfo) \
		FREE((psBlock)->psDebug)


/* Macro to copy the debugging information from one block to another */
#define PUT_DEBUG(psFinal, psBlock) \
	if (genDebugInfo) \
	{ \
		memcpy((psFinal)->psDebug, (psBlock)->psDebug, \
					sizeof(SCRIPT_DEBUG) * (psBlock)->debugEntries); \
		(psFinal)->debugEntries = (psBlock)->debugEntries; \
	}

/* Macro to combine the debugging information in two blocks into a third block */
static UDWORD		_dbEntry;
static SCRIPT_DEBUG	*_psCurr;
static UDWORD		_baseOffset;
#define COMBINE_DEBUG(psFinal, psBlock1, psBlock2) \
	if (genDebugInfo) \
	{ \
		memcpy((psFinal)->psDebug, (psBlock1)->psDebug, \
				 sizeof(SCRIPT_DEBUG) * (psBlock1)->debugEntries); \
		_baseOffset = (psBlock1)->size / sizeof(UDWORD); \
		for(_dbEntry = 0; _dbEntry < (psBlock2)->debugEntries; _dbEntry++) \
		{ \
			_psCurr = (psFinal)->psDebug + (psBlock1)->debugEntries + _dbEntry; \
			_psCurr->line = (psBlock2)->psDebug[_dbEntry].line; \
			_psCurr->offset = (psBlock2)->psDebug[_dbEntry].offset + _baseOffset; \
		} \
		(psFinal)->debugEntries = (psBlock1)->debugEntries + (psBlock2)->debugEntries; \
	}

/* Macro to append some debugging information onto a block, given the instruction
   offset of the debugging information already in the destination block */
#define APPEND_DEBUG(psFinal, baseOffset, psBlock) \
	if (genDebugInfo) \
	{ \
		for(_dbEntry = 0; _dbEntry < (psBlock)->debugEntries; _dbEntry++) \
		{ \
			_psCurr = (psFinal)->psDebug + (psFinal)->debugEntries + _dbEntry; \
			_psCurr->line = (psBlock)->psDebug[_dbEntry].line; \
			_psCurr->offset = (psBlock)->psDebug[_dbEntry].offset + (baseOffset); \
		} \
		(psFinal)->debugEntries = (UWORD)((psFinal)->debugEntries + (psBlock)->debugEntries); \
	}
	

/* Macro to store a label in the debug info */
#define DEBUG_LABEL(psBlock, offset, pString) \
	if (genDebugInfo) \
	{ \
		(psBlock)->psDebug[offset].pLabel = MALLOC(strlen(pString)+1); \
		if (!(psBlock)->psDebug[offset].pLabel) \
		{ \
			scr_error("Out of memory"); \
			ALLOC_ERROR_ACTION; \
		} \
		strcpy((psBlock)->psDebug[offset].pLabel, pString); \
	}


/***************************************************************************************
 *
 * Code generation functions
 *
 * These functions are used within rule bodies to generate code.
 */

/* Macro to deal with the errors returned by code generation functions.
 * Used within the rule body.
 */
#define CHECK_CODE_ERROR(error) \
	if ((error) == CE_MEMORY) \
	{ \
		YYABORT; \
	} \
	else if ((error) == CE_PARSE) \
	{ \
		YYERROR; \
	} 


   
/* Generate the code for a function call, checking the parameter
 * types match.
 */
CODE_ERROR scriptCodeFunction(FUNC_SYMBOL		*psFSymbol,		// The function being called
							PARAM_BLOCK		*psPBlock,		// The functions parameters
							BOOL			expContext,		// Whether the function is being
															// called in an expression context
							CODE_BLOCK		**ppsCBlock)	// The generated code block
{
	UDWORD		size, i, *ip;
	BOOL		typeError = FALSE;
	STRING		aErrorString[255];

	ASSERT((psFSymbol != NULL, "ais_CodeFunction: Invalid function symbol pointer"));
	ASSERT((PTRVALID(psPBlock, sizeof(PARAM_BLOCK)),
		"scriptCodeFunction: Invalid param block pointer"));
	ASSERT(((psPBlock->size == 0) || PTRVALID(psPBlock->pCode, psPBlock->size),
		"scriptCodeFunction: Invalid parameter code pointer"));
	ASSERT((ppsCBlock != NULL,
		 "scriptCodeFunction: Invalid generated code block pointer"));

	/* Check the parameter types match what the function needs */
	for(i=0; (i<psFSymbol->numParams) && (i<psPBlock->numParams); i++)
	{
/*		if (psFSymbol->aParams[i] != VAL_VOID &&
			psFSymbol->aParams[i] != psPBlock->aParams[i])*/
		if (!interpCheckEquiv(psFSymbol->aParams[i], psPBlock->aParams[i]))
		{
			sprintf(aErrorString, "Type mismatch for paramter %d", i);
			scr_error(aErrorString);
			typeError = TRUE;
		}
	}

	/* Check the number of parameters matches that expected */
	if (psFSymbol->numParams != psPBlock->numParams)
	{
		sprintf(aErrorString, "Expected %d parameters", psFSymbol->numParams);
		scr_error(aErrorString);
		*ppsCBlock = NULL;
		return CE_PARSE;
	}
	if (typeError)
	{
		/* Report the error here so all the */
		/* type mismatches are reported */
		*ppsCBlock = NULL;
		return CE_PARSE;
	}

	size = psPBlock->size + sizeof(OPCODE) + sizeof(SCRIPT_FUNC);
	if (!expContext && (psFSymbol->type != VAL_VOID))
	{
		size += sizeof(OPCODE);
	}
	
	ALLOC_BLOCK(*ppsCBlock, size);
	ip = (*ppsCBlock)->pCode;
	(*ppsCBlock)->type = psFSymbol->type;

	/* Copy in the code for the parameters */
	PUT_BLOCK(ip, psPBlock);
	FREE_PBLOCK(psPBlock);

	/* Make the function call */
	if (psFSymbol->script)
	{
		/* function defined in this script */
//		PUT_OPCODE(ip, OP_FUNC);
//		PUT_SCRIPTFUNC(ip, psFSymbol);
	}
	else
	{
		/* call an instinct function */
		PUT_OPCODE(ip, OP_CALL);
		PUT_FUNC(ip, psFSymbol->pFunc);
	}

	if (!expContext && (psFSymbol->type != VAL_VOID))
	{
		/* Clear the return value from the stack */
		PUT_OPCODE(ip, OP_POP);
	}

	return CE_OK;
}


/* Generate the code for a parameter callback, checking the parameter
 * types match.
 */
CODE_ERROR scriptCodeCallbackParams(
							CALLBACK_SYMBOL	*psCBSymbol,	// The callback being called
							PARAM_BLOCK		*psPBlock,		// The callbacks parameters
							TRIGGER_DECL	**ppsTDecl)		// The generated code block
{
	UDWORD		size, i, *ip;
	BOOL		typeError = FALSE;
	STRING		aErrorString[255];

	ASSERT((PTRVALID(psPBlock, sizeof(PARAM_BLOCK)),
		"scriptCodeCallbackParams: Invalid param block pointer"));
	ASSERT(((psPBlock->size == 0) || PTRVALID(psPBlock->pCode, psPBlock->size),
		"scriptCodeCallbackParams: Invalid parameter code pointer"));
	ASSERT((ppsTDecl != NULL,
		 "scriptCodeCallbackParams: Invalid generated code block pointer"));
	ASSERT((psCBSymbol->pFunc != NULL,
		 "scriptCodeCallbackParams: Expected function pointer for callback symbol"));

	/* Check the parameter types match what the function needs */
	for(i=0; (i<psCBSymbol->numParams) && (i<psPBlock->numParams); i++)
	{
		if (!interpCheckEquiv(psCBSymbol->aParams[i], psPBlock->aParams[i]))
		{
			sprintf(aErrorString, "Type mismatch for paramter %d", i);
			scr_error(aErrorString);
			typeError = TRUE;
		}
	}

	/* Check the number of parameters matches that expected */
	if (psPBlock->numParams == 0)
	{
		scr_error("Expected parameters to callback");
		*ppsTDecl = NULL;
		return CE_PARSE;
	}
	else if (psCBSymbol->numParams != psPBlock->numParams)
	{
		sprintf(aErrorString, "Expected %d parameters", psCBSymbol->numParams);
		scr_error(aErrorString);
		*ppsTDecl = NULL;
		return CE_PARSE;
	}
	if (typeError)
	{
		/* Return the error here so all the */
		/* type mismatches are reported */
		*ppsTDecl = NULL;
		return CE_PARSE;
	}

	size = psPBlock->size + sizeof(OPCODE) + sizeof(SCRIPT_FUNC);
	
	ALLOC_TSUBDECL(*ppsTDecl, psCBSymbol->type, size, 0);
	ip = (*ppsTDecl)->pCode;

	/* Copy in the code for the parameters */
	PUT_BLOCK(ip, psPBlock);
	FREE_PBLOCK(psPBlock);

	/* call the instinct function */
	PUT_OPCODE(ip, OP_CALL);
	PUT_FUNC(ip, psCBSymbol->pFunc);

	return CE_OK;
}


/* Generate code for assigning a value to a variable */
CODE_ERROR scriptCodeAssignment(VAR_SYMBOL	*psVariable,	// The variable to assign to
							  CODE_BLOCK	*psValue,		// The code for the value to
															// assign
							  CODE_BLOCK	**ppsBlock)		// Generated code
{
	SDWORD		size;

	ASSERT((psVariable != NULL,
		"scriptCodeAssignment: Invalid variable symbol pointer"));
	ASSERT((PTRVALID(psValue, sizeof(CODE_BLOCK)),
		"scriptCodeAssignment: Invalid value code block pointer"));
	ASSERT((PTRVALID(psValue->pCode, psValue->size),
		"scriptCodeAssignment: Invalid value code pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeAssignment: Invalid generated code block pointer"));

	size = psValue->size + sizeof(OPCODE);
	if (psVariable->storage == ST_EXTERN)
	{
		// Check there is a set function
		if (psVariable->set == NULL)
		{
			scr_error("No set function for external variable");
			return CE_PARSE;
		}
		size += sizeof(SCRIPT_VARFUNC);
	}
	ALLOC_BLOCK(*ppsBlock, size);
	ip = (*ppsBlock)->pCode;

	/* Copy in the code for the expression */
	PUT_BLOCK(ip, psValue);
	FREE_BLOCK(psValue);

	/* Code to get the value from the stack into the variable */
	switch (psVariable->storage)
	{
	case ST_PUBLIC:
	case ST_PRIVATE:
		PUT_PKOPCODE(ip, OP_POPGLOBAL, psVariable->index);
		break;
	case ST_EXTERN:
		PUT_PKOPCODE(ip, OP_VARCALL, psVariable->index);
		PUT_VARFUNC(ip, psVariable->set);
		break;
	case ST_OBJECT:
		scr_error("Cannot use member variables in this context");
		return CE_PARSE;
		break;
	default:
		scr_error("Unknown storage type");
		return CE_PARSE;
		break;
	}

	return CE_OK;
}


/* Generate code for assigning a value to an object variable */
CODE_ERROR scriptCodeObjAssignment(OBJVAR_BLOCK	*psVariable,// The variable to assign to
								 CODE_BLOCK		*psValue,	// The code for the value to
															// assign
								 CODE_BLOCK		**ppsBlock)	// Generated code
{
	ASSERT((PTRVALID(psVariable, sizeof(OBJVAR_BLOCK)),
		"scriptCodeObjAssignment: Invalid object variable block pointer"));
	ASSERT((PTRVALID(psVariable->pCode, psVariable->size),
		"scriptCodeObjAssignment: Invalid object variable code pointer"));
	ASSERT((psVariable->psObjVar != NULL,
		"scriptCodeObjAssignment: Invalid object variable symbol pointer"));
	ASSERT((PTRVALID(psValue, sizeof(CODE_BLOCK)),
		"scriptCodeObjAssignment: Invalid value code block pointer"));
	ASSERT((PTRVALID(psValue->pCode, psValue->size),
		"scriptCodeObjAssignment: Invalid value code pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeObjAssignment: Invalid generated code block pointer"));

	// Check there is an access function for the variable
	if (psVariable->psObjVar->set == NULL)
	{
		scr_error("No set function for object variable");
		return CE_PARSE;
	}

	ALLOC_BLOCK(*ppsBlock, psVariable->size + psValue->size + 
					sizeof(OPCODE) + sizeof(SCRIPT_VARFUNC));
	ip = (*ppsBlock)->pCode;

	/* Copy in the code for the value */
	PUT_BLOCK(ip, psValue);
	FREE_BLOCK(psValue);

	/* Copy in the code for the object */
	PUT_BLOCK(ip, psVariable);

	/* Code to get the value from the stack into the variable */
	PUT_PKOPCODE(ip, OP_VARCALL, psVariable->psObjVar->index);
	PUT_VARFUNC(ip, (psVariable->psObjVar->set));

	/* Free the variable block */
	FREE_OBJVARBLOCK(psVariable);

	return CE_OK;
}


/* Generate code for getting a value from an object variable */
CODE_ERROR scriptCodeObjGet(OBJVAR_BLOCK	*psVariable,// The variable to get from
						  CODE_BLOCK	**ppsBlock)	// Generated code
{
	ASSERT((PTRVALID(psVariable, sizeof(OBJVAR_BLOCK)),
		"scriptCodeObjAssignment: Invalid object variable block pointer"));
	ASSERT((PTRVALID(psVariable->pCode, psVariable->size),
		"scriptCodeObjAssignment: Invalid object variable code pointer"));
	ASSERT((psVariable->psObjVar != NULL,
		"scriptCodeObjAssignment: Invalid object variable symbol pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeObjAssignment: Invalid generated code block pointer"));

	// Check there is an access function for the variable
	if (psVariable->psObjVar->get == NULL)
	{
		scr_error("No get function for object variable");
		return CE_PARSE;
	}

	ALLOC_BLOCK(*ppsBlock, psVariable->size + 
					sizeof(OPCODE) + sizeof(SCRIPT_VARFUNC));
	ip = (*ppsBlock)->pCode;

	/* Copy in the code for the object */
	PUT_BLOCK(ip, psVariable);
	(*ppsBlock)->type = psVariable->psObjVar->type;

	/* Code to get the value from the object onto the stack */
	PUT_PKOPCODE(ip, OP_VARCALL, psVariable->psObjVar->index);
	PUT_VARFUNC(ip, psVariable->psObjVar->get);

	/* Free the variable block */
	FREE_OBJVARBLOCK(psVariable);

	return CE_OK;
}


/* Generate code for assigning a value to an array variable */
CODE_ERROR scriptCodeArrayAssignment(ARRAY_BLOCK	*psVariable,// The variable to assign to
								 CODE_BLOCK		*psValue,	// The code for the value to
															// assign
								 CODE_BLOCK		**ppsBlock)	// Generated code
{
//	SDWORD		elementDWords, i;
//	UBYTE		*pElement;

	ASSERT((PTRVALID(psVariable, sizeof(ARRAY_BLOCK)),
		"scriptCodeObjAssignment: Invalid object variable block pointer"));
	ASSERT((PTRVALID(psVariable->pCode, psVariable->size),
		"scriptCodeObjAssignment: Invalid object variable code pointer"));
	ASSERT((psVariable->psArrayVar != NULL,
		"scriptCodeObjAssignment: Invalid object variable symbol pointer"));
	ASSERT((PTRVALID(psValue, sizeof(CODE_BLOCK)),
		"scriptCodeObjAssignment: Invalid value code block pointer"));
	ASSERT((PTRVALID(psValue->pCode, psValue->size),
		"scriptCodeObjAssignment: Invalid value code pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeObjAssignment: Invalid generated code block pointer"));

	// Check this is an array
	if (psVariable->psArrayVar->dimensions == 0)
	{
		scr_error("Not an array variable");
		return CE_PARSE;
	}

	// calculate the number of DWORDs needed to store the number of elements for each dimension of the array
//	elementDWords = (psVariable->psArrayVar->dimensions - 1)/4 + 1;

//	ALLOC_BLOCK(*ppsBlock, psVariable->size + psValue->size + sizeof(OPCODE) + elementDWords*4);
	ALLOC_BLOCK(*ppsBlock, psVariable->size + psValue->size + sizeof(OPCODE));
	ip = (*ppsBlock)->pCode;

	/* Copy in the code for the value */
	PUT_BLOCK(ip, psValue);
	FREE_BLOCK(psValue);

	/* Copy in the code for the array index */
	PUT_BLOCK(ip, psVariable);

	/* Code to get the value from the stack into the variable */
	PUT_PKOPCODE(ip, OP_POPARRAYGLOBAL,
		((psVariable->psArrayVar->dimensions << ARRAY_DIMENSION_SHIFT) & ARRAY_DIMENSION_MASK) |
		(psVariable->psArrayVar->index & ARRAY_BASE_MASK) );

	// store the size of each dimension
/*	pElement = (UBYTE *)ip;
	for(i=0; i<psVariable->psArrayVar->dimensions; i++)
	{
		*pElement = (UBYTE)psVariable->psArrayVar->elements[i];
		pElement += 1;
	}*/

	/* Free the variable block */
	FREE_ARRAYBLOCK(psVariable);

	return CE_OK;
}


/* Generate code for getting a value from an array variable */
CODE_ERROR scriptCodeArrayGet(ARRAY_BLOCK	*psVariable,// The variable to get from
						  CODE_BLOCK	**ppsBlock)	// Generated code
{
//	SDWORD		elementDWords, i;
//	UBYTE		*pElement;

	ASSERT((PTRVALID(psVariable, sizeof(ARRAY_BLOCK)),
		"scriptCodeObjAssignment: Invalid object variable block pointer"));
	ASSERT((PTRVALID(psVariable->pCode, psVariable->size),
		"scriptCodeObjAssignment: Invalid object variable code pointer"));
	ASSERT((psVariable->psArrayVar != NULL,
		"scriptCodeObjAssignment: Invalid object variable symbol pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeObjAssignment: Invalid generated code block pointer"));

	// Check this is an array
	if (psVariable->psArrayVar->dimensions == 0)
	{
		scr_error("Not an array variable");
		return CE_PARSE;
	}

	// calculate the number of DWORDs needed to store the number of elements for each dimension of the array
//	elementDWords = (psVariable->psArrayVar->dimensions - 1)/4 + 1;

//	ALLOC_BLOCK(*ppsBlock, psVariable->size + sizeof(OPCODE) + elementDWords*4);
	ALLOC_BLOCK(*ppsBlock, psVariable->size + sizeof(OPCODE));
	ip = (*ppsBlock)->pCode;

	/* Copy in the code for the array index */
	PUT_BLOCK(ip, psVariable);
	(*ppsBlock)->type = psVariable->psArrayVar->type;

	/* Code to get the value from the array onto the stack */
	PUT_PKOPCODE(ip, OP_PUSHARRAYGLOBAL,
		((psVariable->psArrayVar->dimensions << ARRAY_DIMENSION_SHIFT) & ARRAY_DIMENSION_MASK) |
		(psVariable->psArrayVar->index & ARRAY_BASE_MASK) );

	// store the size of each dimension
/*	pElement = (UBYTE *)ip;
	for(i=0; i<psVariable->psArrayVar->dimensions; i++)
	{
		*pElement = (UBYTE)psVariable->psArrayVar->elements[i];
		pElement += 1;
	}*/

	/* Free the variable block */
	FREE_ARRAYBLOCK(psVariable);

	return CE_OK;
}


/* Generate the final code block for conditional statements */
CODE_ERROR scriptCodeConditional(
					COND_BLOCK *psCondBlock,	// The intermediate conditional code
					CODE_BLOCK **ppsBlock)		// The final conditional code
{
	UDWORD		i;

	ASSERT((PTRVALID(psCondBlock, sizeof(CODE_BLOCK)),
		"scriptCodeConditional: Invalid conditional code block pointer"));
	ASSERT((PTRVALID(psCondBlock->pCode, psCondBlock->size),
		"scriptCodeConditional: Invalid conditional code pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeConditional: Invalid generated code block pointer"));

	/* Allocate the final block */
	ALLOC_BLOCK(*ppsBlock, psCondBlock->size);
	ALLOC_DEBUG(*ppsBlock, psCondBlock->debugEntries);
	ip = (*ppsBlock)->pCode;

	/* Copy the code over */
	PUT_BLOCK(ip, psCondBlock);

	/* Copy the debugging information */
	PUT_DEBUG(*ppsBlock, psCondBlock);

	/* Now set the offsets of jumps in the conditional to the correct value */
	for(i = 0; i < psCondBlock->numOffsets; i++)
	{
		ip = (*ppsBlock)->pCode + psCondBlock->aOffsets[i];
		*ip = ((*ppsBlock)->size / sizeof(UDWORD)) - (ip - (*ppsBlock)->pCode);
		*ip = (OP_JUMP << OPCODE_SHIFT) | ( (*ip) & OPCODE_DATAMASK );
	}

	/* Free the original code */
	FREE_DEBUG(psCondBlock);
	FREE_CONDBLOCK(psCondBlock);

	return CE_OK;
}

/* Generate code for function parameters */
CODE_ERROR scriptCodeParameter(CODE_BLOCK		*psParam,		// Code for the parameter
							 INTERP_TYPE		type,			// Parameter type
							 PARAM_BLOCK	**ppsBlock)		// Generated code
{
	ASSERT((PTRVALID(psParam, sizeof(CODE_BLOCK)),
		"scriptCodeParameter: Invalid parameter code block pointer"));
	ASSERT((PTRVALID(psParam->pCode, psParam->size),
		"scriptCodeParameter: Invalid parameter code pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeParameter: Invalid generated code block pointer"));

	ALLOC_PBLOCK(*ppsBlock, psParam->size, 1);
	ip = (*ppsBlock)->pCode;

	/* Copy in the code for the parameter */
	PUT_BLOCK(ip, psParam);
	FREE_BLOCK(psParam);
	
	(*ppsBlock)->aParams[0] = type;

	return CE_OK;
}


/* Generate code for binary operators (e.g. 2 + 2) */
CODE_ERROR scriptCodeBinaryOperator(CODE_BLOCK	*psFirst,	// Code for first parameter
								  CODE_BLOCK	*psSecond,	// Code for second parameter
								  OPCODE		opcode,		// Operator function
								  CODE_BLOCK	**ppsBlock) // Generated code
{
	ALLOC_BLOCK(*ppsBlock, psFirst->size + psSecond->size + sizeof(UDWORD));
	ip = (*ppsBlock)->pCode;

	/* Copy the already generated bits of code into the code block */
	PUT_BLOCK(ip, psFirst);
	PUT_BLOCK(ip, psSecond);

	/* Now put an add operator into the code */
	PUT_PKOPCODE(ip, OP_BINARYOP, opcode);

	/* Free the two code blocks that have been copied */
	FREE_BLOCK(psFirst);
	FREE_BLOCK(psSecond);

	return CE_OK;
}


/* Generate code for accessing an object variable.  The variable symbol is
 * stored with the code for the object value so that this block can be used for
 * both setting and retrieving an object value.
 */
CODE_ERROR scriptCodeObjectVariable(CODE_BLOCK	*psObjCode,	// Code for the object value
								  VAR_SYMBOL	*psVar,		// The object variable symbol
								  OBJVAR_BLOCK	**ppsBlock) // Generated code
{
	ASSERT((PTRVALID(psObjCode, sizeof(CODE_BLOCK)),
		"scriptCodeObjectVariable: Invalid object code block pointer"));
	ASSERT((PTRVALID(psObjCode->pCode, psObjCode->size),
		"scriptCodeObjectVariable: Invalid object code pointer"));
	ASSERT((psVar != NULL,
		"scriptCodeObjectVariable: Invalid variable symbol pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeObjectVariable: Invalid generated code block pointer"));

	ALLOC_OBJVARBLOCK(*ppsBlock, psObjCode->size, psVar);
	ip = (*ppsBlock)->pCode;

	/* Copy the already generated bit of code into the code block */
	PUT_BLOCK(ip, psObjCode);
	FREE_BLOCK(psObjCode);

	/* Check the variable is the correct type */
	if (psVar->storage != ST_OBJECT)
	{
		scr_error("Only object variables are valid in this context");
		return CE_PARSE;
	}

	return CE_OK;
}


/* Generate code for accessing an array variable.  The variable symbol is
 * stored with the code for the object value so that this block can be used for
 * both setting and retrieving an array value.
 */
CODE_ERROR scriptCodeArrayVariable(ARRAY_BLOCK	*psArrayCode,	// Code for the array index
								  VAR_SYMBOL	*psVar,			// The array variable symbol
								  ARRAY_BLOCK	**ppsBlock)		// Generated code
{
	ASSERT((PTRVALID(psArrayCode, sizeof(CODE_BLOCK)),
		"scriptCodeObjectVariable: Invalid object code block pointer"));
	ASSERT((PTRVALID(psArrayCode->pCode, psArrayCode->size),
		"scriptCodeObjectVariable: Invalid object code pointer"));
	ASSERT((psVar != NULL,
		"scriptCodeObjectVariable: Invalid variable symbol pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeObjectVariable: Invalid generated code block pointer"));

/*	ALLOC_ARRAYBLOCK(*ppsBlock, psArrayCode->size, psVar);
	ip = (*ppsBlock)->pCode;

	// Copy the already generated bit of code into the code block
	PUT_BLOCK(ip, psArrayCode);
	FREE_BLOCK(psArrayCode);*/

	// Check the variable is the correct type
	if (psVar->dimensions != psArrayCode->dimensions)
	{
		scr_error("Invalid number of array dimensions for this variable");
		return CE_PARSE;
	}

	psArrayCode->psArrayVar = psVar;
	*ppsBlock = psArrayCode;

	return CE_OK;
}


/* Generate code for a constant */
CODE_ERROR scriptCodeConstant(CONST_SYMBOL	*psConst,	// The object variable symbol
							CODE_BLOCK		**ppsBlock)	// Generated code
{
	ASSERT((psConst != NULL,
		"scriptCodeConstant: Invalid constant symbol pointer"));
	ASSERT((ppsBlock != NULL,
		"scriptCodeConstant: Invalid generated code block pointer"));

	ALLOC_BLOCK(*ppsBlock, sizeof(OPCODE) + sizeof(UDWORD));
	ip = (*ppsBlock)->pCode;
	(*ppsBlock)->type = psConst->type;

	/* Put the value onto the stack */
	switch (psConst->type)
	{
	case VAL_BOOL:
		PUT_PKOPCODE(ip, OP_PUSH, VAL_BOOL);
		PUT_DATA(ip, psConst->bval);
		break;
	case VAL_INT:
		PUT_PKOPCODE(ip, OP_PUSH, VAL_INT);
		PUT_DATA(ip, psConst->ival);
		break;
/*	case VAL_FLOAT:
		PUT_PKOPCODE(ip, OP_PUSH, VAL_FLOAT);
		PUT_DATA(ip, psConst->fval);
		break;*/
	default:
		PUT_PKOPCODE(ip, OP_PUSH, psConst->type);
		PUT_DATA(ip, psConst->oval);
		break;
	}

	return CE_OK;
}


/* Generate code for getting a variables value */
CODE_ERROR scriptCodeVarGet(VAR_SYMBOL		*psVariable,	// The object variable symbol
							CODE_BLOCK		**ppsBlock)		// Generated code
{
	SDWORD	size;

	size = sizeof(OPCODE);
	if (psVariable->storage == ST_EXTERN)
	{
		// Check there is a set function
		if (psVariable->get == NULL)
		{
			scr_error("No get function for external variable");
			return CE_PARSE;
		}
		size += sizeof(SCRIPT_VARFUNC);
	}
	ALLOC_BLOCK(*ppsBlock, size);
	ip = (*ppsBlock)->pCode;
	(*ppsBlock)->type = psVariable->type;

	/* Code to get the value onto the stack */
	switch (psVariable->storage)
	{
	case ST_PUBLIC:
	case ST_PRIVATE:
		PUT_PKOPCODE(ip, OP_PUSHGLOBAL, psVariable->index);
		break;
	case ST_EXTERN:
		PUT_PKOPCODE(ip, OP_VARCALL, psVariable->index);
		PUT_VARFUNC(ip, psVariable->get);
		break;
	case ST_OBJECT:
		scr_error("Cannot use member variables in this context");
		return CE_PARSE;
		break;
	default:
		scr_error("Unknown storage type");
		return CE_PARSE;
		break;
	}

	return CE_OK;
}


/* Generate code for getting a variables value */
CODE_ERROR scriptCodeVarRef(VAR_SYMBOL		*psVariable,	// The object variable symbol
							PARAM_BLOCK		**ppsBlock)		// Generated code
{
	SDWORD	size;

	size = sizeof(OPCODE) + sizeof(SDWORD);
	ALLOC_PBLOCK(*ppsBlock, size, 1);
	ip = (*ppsBlock)->pCode;
	(*ppsBlock)->aParams[0] = psVariable->type | VAL_REF;

	/* Code to get the value onto the stack */
	switch (psVariable->storage)
	{
	case ST_PUBLIC:
	case ST_PRIVATE:
		PUT_PKOPCODE(ip, OP_PUSHREF, (*ppsBlock)->aParams[0]);
		PUT_DATA(ip, psVariable->index);
		break;
	case ST_EXTERN:
		scr_error("Cannot use external variables in this context");
		return CE_PARSE;
		break;
	case ST_OBJECT:
		scr_error("Cannot use member variables in this context");
		return CE_PARSE;
		break;
	default:
		scr_error("Unknown storage type");
		return CE_PARSE;
		break;
	}

	return CE_OK;
}


/* Generate the code for a trigger and store it in the trigger list */
CODE_ERROR scriptCodeTrigger(STRING *pIdent, CODE_BLOCK *psCode)
{
	CODE_BLOCK		*psNewBlock;
	UDWORD			line;
	STRING			*pDummy;

	pIdent = pIdent;

	// Have to add the exit code to the end of the event
	ALLOC_BLOCK(psNewBlock, psCode->size + sizeof(OPCODE));
	ip = psNewBlock->pCode;
	PUT_BLOCK(ip, psCode);
	PUT_OPCODE(ip, OP_EXIT);

	// Add the debug info
	ALLOC_DEBUG(psNewBlock, psCode->debugEntries + 1);
	PUT_DEBUG(psNewBlock, psCode);
	if (genDebugInfo)
	{
		/* Add debugging info for the EXIT instruction */
		scriptGetErrorData((SDWORD *)&line, &pDummy);
		psNewBlock->psDebug[psNewBlock->debugEntries].line = line;
		psNewBlock->psDebug[psNewBlock->debugEntries].offset = 
				ip - psNewBlock->pCode;
		psNewBlock->debugEntries ++;
	}
	FREE_BLOCK(psCode);

	// Create the trigger
/*	if (!scriptAddTrigger(pIdent, psNewBlock))
	{
		return CE_MEMORY;
	}*/

	return CE_OK;
}


/* Generate the code for an event and store it in the event list */
CODE_ERROR scriptCodeEvent(EVENT_SYMBOL *psEvent, TRIGGER_SYMBOL *psTrig, CODE_BLOCK *psCode)
{
	CODE_BLOCK		*psNewBlock;
	UDWORD			line;
	STRING			*pDummy;

	// Have to add the exit code to the end of the event
	ALLOC_BLOCK(psNewBlock, psCode->size + sizeof(OPCODE));
	ip = psNewBlock->pCode;
	PUT_BLOCK(ip, psCode);
	PUT_OPCODE(ip, OP_EXIT);

	// Add the debug info
	ALLOC_DEBUG(psNewBlock, psCode->debugEntries + 1);
	PUT_DEBUG(psNewBlock, psCode);
	if (genDebugInfo)
	{
		/* Add debugging info for the EXIT instruction */
		scriptGetErrorData((SDWORD *)&line, &pDummy);
		psNewBlock->psDebug[psNewBlock->debugEntries].line = line;
		psNewBlock->psDebug[psNewBlock->debugEntries].offset = 
				ip - psNewBlock->pCode;
		psNewBlock->debugEntries ++;
	}
	FREE_BLOCK(psCode);

	// Create the event
	if (!scriptDefineEvent(psEvent, psNewBlock, psTrig->index))
	{
		return CE_MEMORY;
	}

	return CE_OK;
}


/* Store the types of a list of variables into a code block.
 * The order of the list is reversed so that the type of the
 * first variable defined is stored first.
 */
static void scriptStoreVarTypes(VAR_SYMBOL *psVar)
{
	if (psVar != NULL)
	{
		/* Recurse down the list to get to the end of it */
		scriptStoreVarTypes(psVar->psNext);

		/* Now store the current variable */
		PUT_INDEX(ip, psVar->type);
	}
}


/* Change the error action for the ALLOC macro's to what it
 * should be inside a rule body.
 *
 * NOTE: DO NOT USE THE ALLOC MACRO'S INSIDE ANY FUNCTIONS
 *       ONCE ALLOC_ERROR_ACTION IS SET TO THIS VALUE.
 *       ALL FUNCTIONS THAT USE THESE MACROS MUST BE PLACED
 *       BEFORE THIS #define.
 */
#undef ALLOC_ERROR_ACTION
#define ALLOC_ERROR_ACTION YYABORT

%}

%union {
	/* Types returned by the lexer */
	BOOL			bval;
	/*	float			fval; */
	SDWORD			ival;
	STRING			*sval;
	INTERP_TYPE		tval;
	STORAGE_TYPE	stype;
	VAR_SYMBOL		*vSymbol;
	CONST_SYMBOL	*cSymbol;
	FUNC_SYMBOL		*fSymbol;
	TRIGGER_SYMBOL	*tSymbol;
	EVENT_SYMBOL	*eSymbol;
	CALLBACK_SYMBOL	*cbSymbol;

	/* Types only returned by rules */
	CODE_BLOCK		*cblock;
	COND_BLOCK		*condBlock;
	OBJVAR_BLOCK	*objVarBlock;
	ARRAY_BLOCK		*arrayBlock;
	PARAM_BLOCK		*pblock;
	PARAM_DECL		*pdecl;
	TRIGGER_DECL	*tdecl;
	VAR_DECL		*vdecl;
	VAR_IDENT_DECL	*videcl;
}
	/* key words */
%token FUNCTION
%token TRIGGER
%token EVENT
%token WAIT
%token EVERY
%token INACTIVE
%token INITIALISE
%token LINK
%token REF

	/* %token COND */
%token WHILE
%token IF
%token ELSE
%token EXIT
%token PAUSE

	/* boolean operators */
%token BOOLEQUAL
%token NOTEQUAL
%token GREATEQUAL
%token LESSEQUAL
%token GREATER
%token LESS
%token _AND
%token _OR
%token _NOT

%left _AND _OR
%left BOOLEQUAL NOTEQUAL GREATEQUAL LESSEQUAL GREATER LESS
%nonassoc _NOT
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

	/* value tokens */
%token <bval> BOOLEAN
	/*%token <fval> FLOAT*/
%token <ival> INTEGER
%token <sval> QTEXT			/* Text with double quotes surrounding it */
%token <tval> TYPE			/* A type specifier */
%token <stype> STORAGE		/* Variable storage type */

%token <sval> IDENT			/* An undefined identifier */

%token <vSymbol> VAR		/* A defined variable */
%token <vSymbol> BOOL_VAR	/* A defined boolean variable */
%token <vSymbol> NUM_VAR	/* A defined numeric (int/float) variable */
%token <vSymbol> OBJ_VAR	/* A defined object pointer variable */

%token <vSymbol> VAR_ARRAY		/* A defined variable array */
%token <vSymbol> BOOL_ARRAY		/* A defined boolean variable array */
%token <vSymbol> NUM_ARRAY		/* A defined numeric (int/float) variable array */
%token <vSymbol> OBJ_ARRAY		/* A defined object pointer variable array */

%token <vSymbol> BOOL_OBJVAR	/* A defined boolean object member variable */
%token <vSymbol> NUM_OBJVAR		/* A defined numeric object member variable */
%token <vSymbol> USER_OBJVAR	/* A defined user type object member variable */
%token <vSymbol> OBJ_OBJVAR		/* A defined object pointer object member variable */

%token <cSymbol> BOOL_CONSTANT	/* A defined boolean constant */
%token <cSymbol> NUM_CONSTANT	/* A defined numeric constant */
%token <cSymbol> USER_CONSTANT	/* A defined object pointer constant */
%token <cSymbol> OBJ_CONSTANT	/* A defined object pointer constant */

%token <fSymbol> FUNC		/* A defined function */
%token <fSymbol> BOOL_FUNC	/* A defined boolean function */
%token <fSymbol> NUM_FUNC	/* A defined numeric (int/float) function */
%token <fSymbol> USER_FUNC	/* A defined user defined type function */
%token <fSymbol> OBJ_FUNC	/* A defined object pointer function */

%token <tSymbol> TRIG_SYM	/* A defined trigger */
%token <eSymbol> EVENT_SYM	/* A defined event */
%token <cbSymbol> CALLBACK_SYM	/* A callback trigger */

	/* rule types */
%type <cblock> expression
%type <cblock> boolexp
%type <cblock> objexp
%type <cblock> objexp_dot
%type <cblock> userexp
%type <objVarBlock> num_objvar
%type <objVarBlock> bool_objvar
%type <objVarBlock> user_objvar
%type <objVarBlock> obj_objvar

%type <arrayBlock> array_index
%type <arrayBlock> array_index_list
%type <arrayBlock> num_array_var
%type <arrayBlock> bool_array_var
%type <arrayBlock> user_array_var
%type <arrayBlock> obj_array_var

%type <cblock> statement_list
%type <cblock> statement

%type <cblock> assignment

%type <cblock> func_call
%type <pblock> param_list
%type <pblock> parameter
%type <pblock> var_ref

%type <cblock> conditional
%type <condBlock> cond_clause
%type <condBlock> terminal_cond
%type <condBlock> cond_clause_list

%type <cblock> loop

%type <videcl>    array_sub_decl
%type <videcl>    array_sub_decl_list
%type <videcl>    variable_ident
%type <vdecl>     variable_decl_head
%type <vdecl>     variable_decl
%type <tdecl>     trigger_subdecl
%type <eSymbol>   event_subdecl

%%

script:			header var_list 
				{
					/* stop parsing here if we are just getting */
					/* the variables in the script */
					/* YYACCEPT */
				}
						 trigger_list event_list
				{
					SDWORD			size, debug, i, dimension, arraySize, totalArraySize;
					SDWORD			numArrays;
					UDWORD			base;
					VAR_SYMBOL		*psCurr;
					TRIGGER_SYMBOL	*psTrig;
					EVENT_SYMBOL	*psEvent;
					UDWORD			numVars;

					// Calculate the code size
					size = 0;
					debug = 0;
					for(psTrig = psTriggers; psTrig; psTrig = psTrig->psNext)
					{
						// Add the trigger code size
						size += psTrig->size;
						debug += psTrig->debugEntries;
					}
					for(psEvent = psEvents; psEvent; psEvent = psEvent->psNext)
					{
						// Add the trigger code size
						size += psEvent->size;
						debug += psEvent->debugEntries;
					}

					// Allocate the program
					numVars = psGlobalVars ? psGlobalVars->index+1 : 0;
					numArrays = psGlobalArrays ? psGlobalArrays->index+1 : 0;
					totalArraySize = 0;
					for(psCurr=psGlobalArrays; psCurr; psCurr=psCurr->psNext)
					{
						arraySize = 1;
						for(dimension = 0; dimension < psCurr->dimensions; dimension += 1)
						{
							arraySize *= psCurr->elements[dimension];
						}
						totalArraySize += arraySize;
					}
					ALLOC_PROG(psFinalProg, size, psCurrBlock->pCode,
						numVars, numArrays, numTriggers, numEvents);
					ALLOC_DEBUG(psFinalProg, debug);
					psFinalProg->debugEntries = 0;
					ip = psFinalProg->pCode;

					// Add the trigger code
					i=0;
					for(psTrig = psTriggers; psTrig; psTrig = psTrig->psNext)
					{
						// Store the trigger offset
						psFinalProg->pTriggerTab[i] = (UWORD)(ip - psFinalProg->pCode);
						if (psTrig->pCode != NULL)
						{
							// Store the label
							DEBUG_LABEL(psFinalProg, psFinalProg->debugEntries,
								psTrig->pIdent);
							// Store debug info
							APPEND_DEBUG(psFinalProg, ip - psFinalProg->pCode, psTrig);
							// Store the code
							PUT_BLOCK(ip, psTrig);
							psFinalProg->psTriggerData[i].code = TRUE;
						}
						else
						{
							psFinalProg->psTriggerData[i].code = FALSE;
						}
						// Store the data
						psFinalProg->psTriggerData[i].type = (UWORD)psTrig->type;
						psFinalProg->psTriggerData[i].time = psTrig->time;
						i = i+1;
					}
					// Note the end of the final trigger
					psFinalProg->pTriggerTab[i] = (UWORD)(ip - psFinalProg->pCode);

					// Add the event code
					i=0;
					for(psEvent = psEvents; psEvent; psEvent = psEvent->psNext)
					{
						// Check the event was declared and has a code body
						if (psEvent->pCode == NULL)
						{
							scr_error("Event %s declared without being defined",
								psEvent->pIdent);
							YYABORT;
						}

						// Store the event offset
						psFinalProg->pEventTab[i] = (UWORD)(ip - psFinalProg->pCode);
						// Store the trigger link
						psFinalProg->pEventLinks[i] = (SWORD)(psEvent->trigger);
						// Store the label
						DEBUG_LABEL(psFinalProg, psFinalProg->debugEntries,
							psEvent->pIdent);
						// Store debug info
						APPEND_DEBUG(psFinalProg, ip - psFinalProg->pCode, psEvent);
						// Store the code
						PUT_BLOCK(ip, psEvent);
						i = i+1;
					}
					// Note the end of the final event
					psFinalProg->pEventTab[i] = (UWORD)(ip - psFinalProg->pCode);

					// Allocate debug info for the variables if necessary
					if (genDebugInfo)
					{
						if (numVars > 0)
						{
							psFinalProg->psVarDebug = MALLOC(sizeof(VAR_DEBUG) * numVars);
							if (psFinalProg->psVarDebug == NULL)
							{
								scr_error("Out of memory");
								YYABORT;
							}
						}
						else
						{
							psFinalProg->psVarDebug = NULL;
						}
						if (numArrays > 0)
						{
							psFinalProg->psArrayDebug = MALLOC(sizeof(ARRAY_DEBUG) * numArrays);
							if (psFinalProg->psArrayDebug == NULL)
							{
								scr_error("Out of memory");
								YYABORT;
							}
						}
						else
						{
							psFinalProg->psArrayDebug = NULL;
						}
					}
					else
					{
						psFinalProg->psVarDebug = NULL;
						psFinalProg->psArrayDebug = NULL;
					}

					/* Now set the types for the global variables */
					for(psCurr = psGlobalVars; psCurr != NULL; psCurr = psCurr->psNext)
					{
						i = psCurr->index;
						psFinalProg->pGlobals[i] = psCurr->type;

						if (genDebugInfo)
						{
							psFinalProg->psVarDebug[i].pIdent =
										MALLOC(strlen(psCurr->pIdent) + 1);
							if (psFinalProg->psVarDebug[i].pIdent == NULL)
							{
								scr_error("Out of memory");
								YYABORT;
							}
							strcpy(psFinalProg->psVarDebug[i].pIdent, psCurr->pIdent);
							psFinalProg->psVarDebug[i].storage = psCurr->storage;
						}
					}

					/* Now store the array info */
					psFinalProg->arraySize = totalArraySize;
					for(psCurr = psGlobalArrays; psCurr != NULL; psCurr = psCurr->psNext)
					{
						i = psCurr->index;

						psFinalProg->psArrayInfo[i].type = (UBYTE)psCurr->type;
						psFinalProg->psArrayInfo[i].dimensions = (UBYTE)psCurr->dimensions;
						for(dimension=0; dimension < psCurr->dimensions; dimension += 1)
						{
							psFinalProg->psArrayInfo[i].elements[dimension] = (UBYTE)psCurr->elements[dimension];
						}

						if (genDebugInfo)
						{
							psFinalProg->psArrayDebug[i].pIdent =
										MALLOC(strlen(psCurr->pIdent) + 1);
							if (psFinalProg->psArrayDebug[i].pIdent == NULL)
							{
								scr_error("Out of memory");
								YYABORT;
							}
							strcpy(psFinalProg->psArrayDebug[i].pIdent, psCurr->pIdent);
							psFinalProg->psArrayDebug[i].storage = psCurr->storage;
						}
					}
					// calculate the base index of each array
					base = psFinalProg->numGlobals;
					for(i=0; i<numArrays; i++)
					{
						psFinalProg->psArrayInfo[i].base = base;

						arraySize = 1;
						for(dimension = 0; dimension < psFinalProg->psArrayInfo[i].dimensions; dimension += 1)
						{
							arraySize *= psFinalProg->psArrayInfo[i].elements[dimension];
						}

						base += arraySize;
					}
				}
			;

	/**************************************************************************************
	 *
	 * Header declarations
	 */

header:			/* NULL token */
			|	header_decl
			|	header header_decl
			;

header_decl:	LINK TYPE ';'
					{
//						if (!scriptAddVariable("owner", $2, ST_PUBLIC, 0))
//						{
							// Out of memory - error already given
//							YYABORT;
//						}
					}
			;


	/**************************************************************************************
	 *
	 * Variable and function declarations
	 */

var_list:		/* NULL token */
			|	variable_decl ';'
				{
					FREE_VARDECL($1);
				}
			|	var_list variable_decl ';'
				{
					FREE_VARDECL($2);
				}
			;

variable_decl_head:		STORAGE TYPE
						{
							ALLOC_VARDECL(psCurrVDecl);
							psCurrVDecl->storage = $1;
							psCurrVDecl->type = $2;

							$$ = psCurrVDecl;
						}
					|	STORAGE	TRIGGER
						{
							ALLOC_VARDECL(psCurrVDecl);
							psCurrVDecl->storage = $1;
							psCurrVDecl->type = VAL_TRIGGER;

							$$ = psCurrVDecl;
						}
					|	STORAGE	EVENT
						{
							ALLOC_VARDECL(psCurrVDecl);
							psCurrVDecl->storage = $1;
							psCurrVDecl->type = VAL_EVENT;

							$$ = psCurrVDecl;
						}
					;

array_sub_decl:		'[' INTEGER ']'
					{
						if ($2 <= 0 || $2 >= VAR_MAX_ELEMENTS)
						{
							scr_error("Invalid array size %d", $2);
							YYABORT;
						}

						ALLOC_VARIDENTDECL(psCurrVIdentDecl, NULL, 1);
						psCurrVIdentDecl->elements[0] = $2;

						$$ = psCurrVIdentDecl;
					}
			;

array_sub_decl_list: array_sub_decl
					{
						$$ = $1;
					}
			|
					array_sub_decl_list '[' INTEGER ']'
					{
						if ($1->dimensions >= VAR_MAX_DIMENSIONS)
						{
							scr_error("Too many dimensions for array");
							YYABORT;
						}
						if ($3 <= 0 || $3 >= VAR_MAX_ELEMENTS)
						{
							scr_error("Invalid array size %d", $3);
							YYABORT;
						}

						$1->elements[$1->dimensions] = $3;
						$1->dimensions += 1;

						$$ = $1;
					}
			;

variable_ident:		IDENT
					{
						ALLOC_VARIDENTDECL(psCurrVIdentDecl, $1, 0);

						$$ = psCurrVIdentDecl;
					}
			|
					IDENT array_sub_decl_list
					{
						$2->pIdent = MALLOC(strlen($1)+1);
						if ($2->pIdent == NULL)
						{
							scr_error("Out of memory");
							YYABORT;
						}
						strcpy($2->pIdent, $1);

						$$ = $2;
					}
			;

variable_decl:	variable_decl_head variable_ident
					{
						if (!scriptAddVariable($1, $2))
						{
							/* Out of memory - error already given */
							YYABORT;
						}

						FREE_VARIDENTDECL($2);

						/* return the variable type */
						$$ = $1;
					}
			|	variable_decl ',' variable_ident
					{
						if (!scriptAddVariable($1, $3))
						{
							/* Out of memory - error already given */
							YYABORT;
						}

						FREE_VARIDENTDECL($3);

						/* return the variable type */
						$$ = $1;
					}
			;

	/**************************************************************************************
	 *
	 * Trigger declarations
	 */

trigger_list:		/* NULL token */
				|	trigger_decl
				|	trigger_list trigger_decl
				;

trigger_subdecl:	boolexp ',' INTEGER
					{
						ALLOC_TSUBDECL(psCurrTDecl, TR_CODE, $1->size, $3);
						ip = psCurrTDecl->pCode;
						PUT_BLOCK(ip, $1);
						FREE_BLOCK($1);

						$$ = psCurrTDecl;
					}
				|	WAIT ',' INTEGER
					{
						ALLOC_TSUBDECL(psCurrTDecl, TR_WAIT, 0, $3);

						$$ = psCurrTDecl;
					}
				|	EVERY ',' INTEGER
					{
						ALLOC_TSUBDECL(psCurrTDecl, TR_EVERY, 0, $3);

						$$ = psCurrTDecl;
					}
				|	INITIALISE
					{
						ALLOC_TSUBDECL(psCurrTDecl, TR_INIT, 0, 0);

						$$ = psCurrTDecl;
					}
				|	CALLBACK_SYM
					{
						if ($1->numParams != 0)
						{
							scr_error("Expected parameters for callback trigger");
							YYABORT;
						}

						ALLOC_TSUBDECL(psCurrTDecl, $1->type, 0, 0);

						$$ = psCurrTDecl;
					}
				|	CALLBACK_SYM ',' param_list
					{
						codeRet = scriptCodeCallbackParams($1, $3, &psCurrTDecl);
						CHECK_CODE_ERROR(codeRet);

						$$ = psCurrTDecl;
					}
				;

trigger_decl:		TRIGGER IDENT '(' trigger_subdecl ')' ';'
					{
						SDWORD	line;
						STRING	*pDummy;

						scriptGetErrorData(&line, &pDummy);
						if (!scriptAddTrigger($2, $4, (UDWORD)line))
						{
							YYABORT;
						}
						FREE_TSUBDECL($4);
					}
				;

	/**************************************************************************************
	 *
	 * Event declarations
	 */

event_list:			event_decl
				|	event_list event_decl
				;

event_subdecl:		EVENT IDENT
					{
						EVENT_SYMBOL	*psEvent;
						if (!scriptDeclareEvent($2, &psEvent))
						{
							YYABORT;
						}

						$$ = psEvent;
					}
				|	EVENT EVENT_SYM
						{
							$$ = $2;
						}
				;

event_decl:			event_subdecl ';'
				|	event_subdecl '(' TRIG_SYM ')' '{' statement_list '}'
					{
						if (!scriptDefineEvent($1, $6, $3->index))
						{
							YYABORT;
						}

						FREE_DEBUG($6);
						FREE_BLOCK($6);
					}
				|	event_subdecl '(' trigger_subdecl ')'
					{
						// Get the line for the implicit trigger declaration
						STRING	*pDummy;
						scriptGetErrorData((SDWORD *)&debugLine, &pDummy);
					}
														'{' statement_list '}'
					{
						// Create a trigger for this event
						if (!scriptAddTrigger("", $3, debugLine))
						{
							YYABORT;
						}
						FREE_TSUBDECL($3);

						if (!scriptDefineEvent($1, $7, numTriggers - 1))
						{
							YYABORT;
						}

						FREE_DEBUG($7);
						FREE_BLOCK($7);
					}
				|	event_subdecl '(' INACTIVE ')' '{' statement_list '}'
					{
						if (!scriptDefineEvent($1, $6, -1))
						{
							YYABORT;
						}

						FREE_DEBUG($6);
						FREE_BLOCK($6);
					}
				;

	/**************************************************************************************
	 *
	 * Statements
	 */

statement_list:		/* NULL token */
						{
							// Allocate a dummy code block
							ALLOC_BLOCK(psCurrBlock, 1);
							psCurrBlock->size = 0;

							$$ = psCurrBlock;
						}
				|	statement
						{
							$$ = $1;
						}
				|	statement_list statement
						{
							ALLOC_BLOCK(psCurrBlock, $1->size + $2->size);
							ALLOC_DEBUG(psCurrBlock, $1->debugEntries +
													 $2->debugEntries);
							ip = psCurrBlock->pCode;

							/* Copy the two code blocks */
							PUT_BLOCK(ip, $1);
							PUT_BLOCK(ip, $2);
							PUT_DEBUG(psCurrBlock, $1);
							APPEND_DEBUG(psCurrBlock, $1->size / sizeof(UDWORD), $2);
							FREE_DEBUG($1);
							FREE_DEBUG($2);
							FREE_BLOCK($1);
							FREE_BLOCK($2);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				;

statement:			assignment ';'
					{
						UDWORD line;
						STRING *pDummy;

						/* Put in debugging info */
						if (genDebugInfo)
						{
							ALLOC_DEBUG($1, 1);
							$1->psDebug[0].offset = 0;
							scriptGetErrorData((SDWORD *)&line, &pDummy);
							$1->psDebug[0].line = line;
						}

						$$ = $1;
					}
				|	func_call ';'
					{
						UDWORD line;
						STRING *pDummy;

						DBP0(("[compile] func_call\n"));

						/* Put in debugging info */
						if (genDebugInfo)
						{
							ALLOC_DEBUG($1, 1);
							$1->psDebug[0].offset = 0;
							scriptGetErrorData((SDWORD *)&line, &pDummy);
							$1->psDebug[0].line = line;
						}

						$$ = $1;
					}
				|	conditional
					{
						$$ = $1;
					}
				|	loop
					{
						$$ = $1;
					}
				|	EXIT ';'
					{
						UDWORD line;
						STRING *pDummy;

						/* Allocate the code block */
						ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE));
						ALLOC_DEBUG(psCurrBlock, 1);
						ip = psCurrBlock->pCode;

						/* Store the instruction */
						PUT_OPCODE(ip, OP_EXIT);

						/* Add the debugging information */
						if (genDebugInfo)
						{
							psCurrBlock->psDebug[0].offset = 0;
							scriptGetErrorData((SDWORD *)&line, &pDummy);
							psCurrBlock->psDebug[0].line = line;
						}

						$$ = psCurrBlock;
					}
				|	PAUSE '(' INTEGER ')' ';'
					{
						UDWORD line;
						STRING *pDummy;

						// can only have a positive pause
						if ($3 < 0)
						{
							scr_error("Invalid pause time");
							YYABORT;
						}

						/* Allocate the code block */
						ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE));
						ALLOC_DEBUG(psCurrBlock, 1);
						ip = psCurrBlock->pCode;

						/* Store the instruction */
						PUT_PKOPCODE(ip, OP_PAUSE, $3);

						/* Add the debugging information */
						if (genDebugInfo)
						{
							psCurrBlock->psDebug[0].offset = 0;
							scriptGetErrorData((SDWORD *)&line, &pDummy);
							psCurrBlock->psDebug[0].line = line;
						}

						$$ = psCurrBlock;
					}
				;

	/* Variable assignment statements, eg:
	 *        foo = 4 + bar ;
	 */
assignment:			NUM_VAR '=' expression
						{
							codeRet = scriptCodeAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	BOOL_VAR '=' boolexp
						{
							codeRet = scriptCodeAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	OBJ_VAR '=' objexp
						{
							if (!interpCheckEquiv($1->type, $3->type))
							{
								scr_error("User type mismatch for assignment");
								YYABORT;
							}
							codeRet = scriptCodeAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	VAR '=' userexp
						{
							if (!interpCheckEquiv($1->type, $3->type))
							{
								scr_error("User type mismatch for assignment");
								YYABORT;
							}
							codeRet = scriptCodeAssignment($1, (CODE_BLOCK *)$3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	num_objvar '=' expression
						{
							codeRet = scriptCodeObjAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	bool_objvar '=' boolexp
						{
							codeRet = scriptCodeObjAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	user_objvar '=' userexp
						{
							if (!interpCheckEquiv($1->psObjVar->type,$3->type))
							{
								scr_error("User type mismatch for assignment");
								YYABORT;
							}
							codeRet = scriptCodeObjAssignment($1, (CODE_BLOCK *)$3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	obj_objvar '=' objexp
						{
							if (!interpCheckEquiv($1->psObjVar->type, $3->type))
							{
								scr_error("User type mismatch for assignment");
								YYABORT;
							}
							codeRet = scriptCodeObjAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	num_array_var '=' expression
						{
							codeRet = scriptCodeArrayAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	bool_array_var '=' boolexp
						{
							codeRet = scriptCodeArrayAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	user_array_var '=' userexp
						{
							if (!interpCheckEquiv($1->psArrayVar->type,$3->type))
							{
								scr_error("User type mismatch for assignment");
								YYABORT;
							}
							codeRet = scriptCodeArrayAssignment($1, (CODE_BLOCK *)$3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				|	obj_array_var '=' objexp
						{
							if (!interpCheckEquiv($1->psArrayVar->type, $3->type))
							{
								scr_error("User type mismatch for assignment");
								YYABORT;
							}
							codeRet = scriptCodeArrayAssignment($1, $3, &psCurrBlock);
							CHECK_CODE_ERROR(codeRet);

							/* Return the code block */
							$$ = psCurrBlock;
						}
				;

	/* Function calls, eg:
	 *         DisplayGlobals() ;
	 *
	 *		   If the function returns a value in a statement context
	 *		   code is generated to remove that value from the stack.
	 */
func_call:		NUM_FUNC '(' param_list ')'
					{
						DBP0(("[compile] NUM_FUNC\n"));

						/* Generate the code for the function call */
						codeRet = scriptCodeFunction($1, $3, FALSE, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrBlock;
					}
			|	BOOL_FUNC '(' param_list ')'
					{
						DBP0(("[compile] BOOL_FUNC\n"));

						/* Generate the code for the function call */
						codeRet = scriptCodeFunction($1, $3, FALSE, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrBlock;
					}
			|	USER_FUNC '(' param_list ')'
					{
						DBP0(("[compile] USER_FUNC\n"));

						/* Generate the code for the function call */
						codeRet = scriptCodeFunction($1, $3, FALSE, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrBlock;
					}
			|	OBJ_FUNC '(' param_list ')'
					{
						DBP0(("[compile] OBJ_FUNC\n"));

						/* Generate the code for the function call */
						codeRet = scriptCodeFunction($1, $3, FALSE, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrBlock;
					}
			|	FUNC '(' param_list ')'
					{
						DBP0(("[compile] FUNC\n"));
						/* Generate the code for the function call */
						codeRet = scriptCodeFunction($1, $3, FALSE, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrBlock;
					}
			;

	/*
	 * Parse parameter lists eg:
	 *		5, foo, (bar + 4.2) / 3, foobar and barfoo
	 *
	 * These rules return PARAM_BLOCKS which contain the type of each
	 * parameter as well as the code for the parameter.
	 */
param_list:		/* NULL token */
					{
						/* create a dummy pblock containing nothing */
						ALLOC_PBLOCK(psCurrPBlock, sizeof(UDWORD), 1);
						psCurrPBlock->size = 0;
						psCurrPBlock->numParams = 0;

						$$ = psCurrPBlock;
					}
			|	parameter
					{
						DBP0(("[compile] parameter\n"));
						$$ = $1;
					}
			|	param_list ',' parameter
					{
						DBP0(("[compile] param_list , parameter\n"));

						ALLOC_PBLOCK(psCurrPBlock, $1->size + $3->size,
												   $1->numParams + $3->numParams);
						ip = psCurrPBlock->pCode;

						/* Copy in the code for the parameters */
						PUT_BLOCK(ip, $1);
						PUT_BLOCK(ip, $3);
						
						/* Copy the parameter types */
						memcpy(psCurrPBlock->aParams, $1->aParams,
								$1->numParams * sizeof(INTERP_TYPE));
						memcpy(psCurrPBlock->aParams + $1->numParams,
								$3->aParams,
								$3->numParams * sizeof(INTERP_TYPE));

						/* Free the old pblocks */
						FREE_PBLOCK($1);
						FREE_PBLOCK($3);

						/* return the pblock */
						$$ = psCurrPBlock;
					}
			;
parameter:		expression
					{
						/* Generate the code for the parameter                     */
						codeRet = scriptCodeParameter($1, VAL_INT, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	boolexp
					{
						/* Generate the code for the parameter */
						codeRet = scriptCodeParameter($1, VAL_BOOL, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	userexp
					{
						/* Generate the code for the parameter */
						codeRet = scriptCodeParameter((CODE_BLOCK *)$1, $1->type, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	objexp
					{
						/* Generate the code for the parameter */
						codeRet = scriptCodeParameter($1, $1->type, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	var_ref
					{
						/* just pass the variable reference up the tree */
						$$ = $1;
					}
			;

var_ref:		REF NUM_VAR
					{
						codeRet = scriptCodeVarRef($2, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	REF BOOL_VAR
					{
						codeRet = scriptCodeVarRef($2, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	REF VAR
					{
						codeRet = scriptCodeVarRef($2, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			|	REF OBJ_VAR
					{
						codeRet = scriptCodeVarRef($2, &psCurrPBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrPBlock;
					}
			;

	/*
	 * Conditional statements, can be :
	 *
	 */

conditional:		cond_clause_list
					{
						codeRet = scriptCodeConditional($1, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						$$ = psCurrBlock;
					}
				|	cond_clause_list ELSE terminal_cond
					{
						ALLOC_CONDBLOCK(psCondBlock,
										$1->numOffsets + $3->numOffsets,
										$1->size + $3->size);
						ALLOC_DEBUG(psCondBlock, $1->debugEntries + $3->debugEntries);
						ip = psCondBlock->pCode;

						/* Store the two blocks of code */
						PUT_BLOCK(ip, $1);
						PUT_BLOCK(ip, $3);

						/* Copy over the offset information       */
						/* (There isn't any in the terminal_cond) */
						memcpy(psCondBlock->aOffsets, $1->aOffsets,
							   $1->numOffsets * sizeof(SDWORD));
						psCondBlock->numOffsets = $1->numOffsets;

						/* Put in the debugging information */
						PUT_DEBUG(psCondBlock, $1);
						APPEND_DEBUG(psCondBlock, $1->size / sizeof(UDWORD), $3);

						/* Free the code blocks */
						FREE_DEBUG($1);
						FREE_DEBUG($3);
						FREE_CONDBLOCK($1);
						FREE_CONDBLOCK($3);

						/* Do the final processing of the conditional */
						codeRet = scriptCodeConditional(psCondBlock, &psCurrBlock);
						CHECK_CODE_ERROR(codeRet);

						$$ = psCurrBlock;
					}
				;

cond_clause_list:	cond_clause
					{
						$$ = $1;
					}
				|	cond_clause_list ELSE cond_clause
					{
						ALLOC_CONDBLOCK(psCondBlock,
										$1->numOffsets + $3->numOffsets,
										$1->size + $3->size);
						ALLOC_DEBUG(psCondBlock, $1->debugEntries + $3->debugEntries);
						ip = psCondBlock->pCode;

						/* Store the two blocks of code */
						PUT_BLOCK(ip, $1);
						PUT_BLOCK(ip, $3);

						/* Copy over the offset information */
						memcpy(psCondBlock->aOffsets, $1->aOffsets,
							   $1->numOffsets * sizeof(SDWORD));
						psCondBlock->aOffsets[$1->numOffsets] =
							$3->aOffsets[0] + $1->size / sizeof(UDWORD);
						psCondBlock->numOffsets = $1->numOffsets + 1;

						/* Put in the debugging information */
						PUT_DEBUG(psCondBlock, $1);
						APPEND_DEBUG(psCondBlock, $1->size / sizeof(UDWORD), $3);

						/* Free the code blocks */
						FREE_DEBUG($1);
						FREE_DEBUG($3);
						FREE_CONDBLOCK($1);
						FREE_CONDBLOCK($3);

						$$ = psCondBlock;
					}
				;


terminal_cond:		'{' statement_list '}'
					{
						/* Allocate the block */
						ALLOC_CONDBLOCK(psCondBlock, 1, $2->size);
						ALLOC_DEBUG(psCondBlock, $2->debugEntries);
						ip = psCondBlock->pCode;

						/* Put in the debugging information */
						PUT_DEBUG(psCondBlock, $2);

						/* Store the statements */
						PUT_BLOCK(ip, $2);
						FREE_DEBUG($2);
						FREE_BLOCK($2);

						$$ = psCondBlock;
					}
				;

cond_clause:		IF '(' boolexp ')'
					{
						STRING *pDummy;

						/* Get the line number for the end of the boolean expression */
						/* and store it in debugLine.                                 */
						scriptGetErrorData((SDWORD *)&debugLine, &pDummy);
					}
									'{' statement_list '}'
					{
						/* Allocate the block */
						ALLOC_CONDBLOCK(psCondBlock, 1,
										$3->size + $7->size + 
										sizeof(OPCODE)*2);
						ALLOC_DEBUG(psCondBlock, $7->debugEntries + 1);
						ip = psCondBlock->pCode;

						/* Store the boolean expression code */
						PUT_BLOCK(ip, $3);
						FREE_BLOCK($3);

						/* Put in the jump to the end of the block if the */
						/* condition is false */
						PUT_PKOPCODE(ip, OP_JUMPFALSE, ($7->size / sizeof(UDWORD)) + 2);

						/* Put in the debugging information */
						if (genDebugInfo)
						{
							psCondBlock->debugEntries = 1;
							psCondBlock->psDebug->line = debugLine;
							psCondBlock->psDebug->offset = 0;
							APPEND_DEBUG(psCondBlock, ip - psCondBlock->pCode, $7);
						}

						/* Store the statements */
						PUT_BLOCK(ip, $7);
						FREE_DEBUG($7);
						FREE_BLOCK($7);

						/* Store the location that has to be filled in   */
						psCondBlock->aOffsets[0] = ip - psCondBlock->pCode;

						/* Put in a jump to skip the rest of the conditional */
						/* The correct offset will be set once the whole   */
						/* conditional has been parsed                     */
						/* The jump should be to the instruction after the */
						/* entire conditonal block                         */
						PUT_PKOPCODE(ip, OP_JUMP, 0);

						$$ = psCondBlock;
					}
				;



	/*
	 * while loops, e.g.
	 *
	 * while (i < 5)
	 * {
	 *    ....
	 *    i = i + 1;
	 * }
	 */
loop:		WHILE '(' boolexp ')'
				{
					STRING *pDummy;

					/* Get the line number for the end of the boolean expression */
					/* and store it in debugLine.                                 */
					scriptGetErrorData((SDWORD *)&debugLine, &pDummy);
				}
								 '{' statement_list '}'
				{

					ALLOC_BLOCK(psCurrBlock, $3->size + $7->size + sizeof(OPCODE) * 2);
					ALLOC_DEBUG(psCurrBlock, $7->debugEntries + 1);
					ip = psCurrBlock->pCode;

					/* Copy in the loop expression */
					PUT_BLOCK(ip, $3);
					FREE_BLOCK($3);

					/* Now a conditional jump out of the loop if the */
					/* expression is false.                          */
					PUT_PKOPCODE(ip, OP_JUMPFALSE, ($7->size / sizeof(UDWORD)) + 2);

					/* Now put in the debugging information */
					if (genDebugInfo)
					{
						psCurrBlock->debugEntries = 1;
						psCurrBlock->psDebug->line = debugLine;
						psCurrBlock->psDebug->offset = 0;
						APPEND_DEBUG(psCurrBlock, ip - psCurrBlock->pCode, $7);
					}

					/* Copy in the body of the loop */
					PUT_BLOCK(ip, $7);
					FREE_DEBUG($7);
					FREE_BLOCK($7);

					/* Put in a jump back to the start of the loop expression */
					PUT_PKOPCODE(ip, OP_JUMP, (SWORD)( -(SWORD)(psCurrBlock->size / sizeof(UDWORD)) + 1));

					$$ = psCurrBlock;
				}
		;

	/**************************************************************************************
	 *
	 * Mathematical expressions
	 */

expression:		expression '+' expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_ADD, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression '-' expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_SUB, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression '*' expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_MUL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression '/' expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_DIV, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	'-' expression %prec UMINUS
				{
					ALLOC_BLOCK(psCurrBlock, $2->size + sizeof(OPCODE));
					ip = psCurrBlock->pCode;

					/* Copy the already generated bits of code into the code block */
					PUT_BLOCK(ip, $2);

					/* Now put a negation operator into the code */
					PUT_PKOPCODE(ip, OP_UNARYOP, OP_NEG);

					/* Free the two code blocks that have been copied */
					FREE_BLOCK($2);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	'(' expression ')'
				{
					/* Just pass the code up the tree */
					$$ = $2;
				}
			|	NUM_FUNC '(' param_list ')'
				{
					/* Generate the code for the function call */
					codeRet = scriptCodeFunction($1, $3, TRUE, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	NUM_VAR
				{
					codeRet = scriptCodeVarGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	NUM_CONSTANT
				{
					codeRet = scriptCodeConstant($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	num_objvar
				{
					codeRet = scriptCodeObjGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	num_array_var
				{
					codeRet = scriptCodeArrayGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	INTEGER
				{
					ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE) + sizeof(UDWORD));
					ip = psCurrBlock->pCode;

					/* Code to store the value on the stack */
					PUT_PKOPCODE(ip, OP_PUSH, VAL_INT);
					PUT_DATA(ip, $1);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			;

	/***************************************************************************************
	 *
	 * Boolean expressions
	 */

boolexp:		boolexp _AND boolexp
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_AND, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	boolexp _OR boolexp
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_OR, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	boolexp BOOLEQUAL boolexp
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_EQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	boolexp NOTEQUAL boolexp
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_NOTEQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	_NOT boolexp
				{
					ALLOC_BLOCK(psCurrBlock, $2->size + sizeof(OPCODE));
					ip = psCurrBlock->pCode;

					/* Copy the already generated bits of code into the code block */
					PUT_BLOCK(ip, $2);

					/* Now put a negation operator into the code */
					PUT_PKOPCODE(ip, OP_UNARYOP, OP_NOT);

					/* Free the two code blocks that have been copied */
					FREE_BLOCK($2);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	'(' boolexp ')'
				{
					/* Just pass the code up the tree */
					$$ = $2;
				}
			|	BOOL_FUNC '(' param_list ')'
				{
					/* Generate the code for the function call */
					codeRet = scriptCodeFunction($1, $3, TRUE, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	BOOL_VAR
				{
					codeRet = scriptCodeVarGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	BOOL_CONSTANT
				{
					codeRet = scriptCodeConstant($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	bool_objvar
				{
					codeRet = scriptCodeObjGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	bool_array_var
				{
					codeRet = scriptCodeArrayGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	BOOLEAN
				{
					ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE) + sizeof(UDWORD));
					ip = psCurrBlock->pCode;

					/* Code to store the value on the stack */
					PUT_PKOPCODE(ip, OP_PUSH, VAL_BOOL);
					PUT_DATA(ip, $1);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression BOOLEQUAL expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_EQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	userexp BOOLEQUAL userexp
				{
					if (!interpCheckEquiv($1->type,$3->type))
					{
						scr_error("Type mismatch for equality");
						YYABORT;
					}
					codeRet = scriptCodeBinaryOperator($1, $3, OP_EQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	objexp BOOLEQUAL objexp
				{
					if (!interpCheckEquiv($1->type,$3->type))
					{
						scr_error("Type mismatch for equality");
						YYABORT;
					}
					codeRet = scriptCodeBinaryOperator($1, $3, OP_EQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression NOTEQUAL expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_NOTEQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	userexp NOTEQUAL userexp
				{
					if (!interpCheckEquiv($1->type,$3->type))
					{
						scr_error("Type mismatch for inequality");
						YYABORT;
					}
					codeRet = scriptCodeBinaryOperator($1, $3, OP_NOTEQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	objexp NOTEQUAL objexp
				{
					if (!interpCheckEquiv($1->type,$3->type))
					{
						scr_error("Type mismatch for inequality");
						YYABORT;
					}
					codeRet = scriptCodeBinaryOperator($1, $3, OP_NOTEQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression LESSEQUAL expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_LESSEQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression GREATEQUAL expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_GREATEREQUAL, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression GREATER expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_GREATER, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	expression LESS expression
				{
					codeRet = scriptCodeBinaryOperator($1, $3, OP_LESS, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			;

	/*************************************************************************************
	 *
	 * User variable expressions
	 */

userexp:		VAR
				{
					codeRet = scriptCodeVarGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	USER_CONSTANT
				{
					codeRet = scriptCodeConstant($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	user_objvar
				{
					codeRet = scriptCodeObjGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);
					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	user_array_var
				{
					codeRet = scriptCodeArrayGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	USER_FUNC '(' param_list ')'
				{
					/* Generate the code for the function call */
					codeRet = scriptCodeFunction($1, $3, TRUE, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	TRIG_SYM
				{
					ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE) + sizeof(UDWORD));
					ip = psCurrBlock->pCode;

					/* Code to store the value on the stack */
					PUT_PKOPCODE(ip, OP_PUSH, VAL_TRIGGER);
					PUT_DATA(ip, $1->index);

					psCurrBlock->type = VAL_TRIGGER;

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	INACTIVE
				{
					ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE) + sizeof(UDWORD));
					ip = psCurrBlock->pCode;

					/* Code to store the value on the stack */
					PUT_PKOPCODE(ip, OP_PUSH, VAL_TRIGGER);
					PUT_DATA(ip, -1);

					psCurrBlock->type = VAL_TRIGGER;

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	EVENT_SYM
				{
					ALLOC_BLOCK(psCurrBlock, sizeof(OPCODE) + sizeof(UDWORD));
					ip = psCurrBlock->pCode;

					/* Code to store the value on the stack */
					PUT_PKOPCODE(ip, OP_PUSH, VAL_EVENT);
					PUT_DATA(ip, $1->index);

					psCurrBlock->type = VAL_EVENT;

					/* Return the code block */
					$$ = psCurrBlock;
				}
			;

	/*************************************************************************************
	 *
	 * Object expressions.
	 */

objexp:			OBJ_VAR
				{
					codeRet = scriptCodeVarGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	OBJ_CONSTANT
				{
					codeRet = scriptCodeConstant($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	OBJ_FUNC '(' param_list ')'
				{
					/* Generate the code for the function call */
					codeRet = scriptCodeFunction($1, $3, TRUE, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	obj_objvar
				{
					codeRet = scriptCodeObjGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			|	obj_array_var
				{
					codeRet = scriptCodeArrayGet($1, &psCurrBlock);
					CHECK_CODE_ERROR(codeRet);

					/* Return the code block */
					$$ = psCurrBlock;
				}
			;

	/*
	 * Seperate rule for the dot after an object so the lookup context
	 * for object variables can be set
	 */
objexp_dot:		objexp '.'
				{
					// Store the object type for the variable lookup
					objVarContext = $1->type;
				}
			;

	/*
	 * Object member variable references.
	 */

num_objvar:		objexp_dot NUM_OBJVAR
				{
					codeRet = scriptCodeObjectVariable($1, $2, &psObjVarBlock);
					CHECK_CODE_ERROR(codeRet);

					// reset the object type
					objVarContext = 0;

					/* Return the code block */
					$$ = psObjVarBlock;
				}
			;

bool_objvar:	objexp_dot BOOL_OBJVAR
				{
					codeRet = scriptCodeObjectVariable($1, $2, &psObjVarBlock);
					CHECK_CODE_ERROR(codeRet);

					// reset the object type
					objVarContext = 0;

					/* Return the code block */
					$$ = psObjVarBlock;
				}
			;

user_objvar:	objexp_dot USER_OBJVAR
				{
					codeRet = scriptCodeObjectVariable($1, $2, &psObjVarBlock);
					CHECK_CODE_ERROR(codeRet);

					// reset the object type
					objVarContext = 0;

					/* Return the code block */
					$$ = psObjVarBlock;
				}
			;
obj_objvar:		objexp_dot OBJ_OBJVAR
				{
					codeRet = scriptCodeObjectVariable($1, $2, &psObjVarBlock);
					CHECK_CODE_ERROR(codeRet);

					// reset the object type
					objVarContext = 0;

					/* Return the code block */
					$$ = psObjVarBlock;
				}
			;

	/*****************************************************************************************************
	 *
	 * Array expressions
	 *
	 */

array_index:		'[' expression ']'
					{
						ALLOC_ARRAYBLOCK(psCurrArrayBlock, $2->size, NULL);
						ip = psCurrArrayBlock->pCode;

						/* Copy the index expression code into the code block */
						PUT_BLOCK(ip, $2);
						FREE_BLOCK($2);

						$$ = psCurrArrayBlock;
					}
			;

array_index_list:	array_index
					{
						$$ = $1;
					}
				|
					array_index_list '[' expression ']'
					{
						ALLOC_ARRAYBLOCK(psCurrArrayBlock, $1->size + $3->size, NULL);
						ip = psCurrArrayBlock->pCode;

						/* Copy the index expression code into the code block */
						psCurrArrayBlock->dimensions = $1->dimensions + 1;
						PUT_BLOCK(ip, $1);
						PUT_BLOCK(ip, $3);
						FREE_ARRAYBLOCK($1);
						FREE_ARRAYBLOCK($3);

						$$ = psCurrArrayBlock;
					}
				;

num_array_var:		NUM_ARRAY array_index_list
					{
						codeRet = scriptCodeArrayVariable($2, $1, &psCurrArrayBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrArrayBlock;
					}
				;

bool_array_var:		BOOL_ARRAY array_index_list
					{
						codeRet = scriptCodeArrayVariable($2, $1, &psCurrArrayBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrArrayBlock;
					}
				;

obj_array_var:		OBJ_ARRAY array_index_list
					{
						codeRet = scriptCodeArrayVariable($2, $1, &psCurrArrayBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrArrayBlock;
					}
				;

user_array_var:		VAR_ARRAY array_index_list
					{
						codeRet = scriptCodeArrayVariable($2, $1, &psCurrArrayBlock);
						CHECK_CODE_ERROR(codeRet);

						/* Return the code block */
						$$ = psCurrArrayBlock;
					}
				;

%%

// Reset all the symbol tables
static void scriptResetTables(void)
{
	VAR_SYMBOL		*psCurr, *psNext;
	TRIGGER_SYMBOL	*psTCurr, *psTNext;
	EVENT_SYMBOL	*psECurr, *psENext;
	FUNC_SYMBOL		*psFCurr, *psFNext;

	/* Reset the global variable symbol table */
	for(psCurr = psGlobalVars; psCurr != NULL; psCurr = psNext)
	{
		psNext = psCurr->psNext;
		FREE(psCurr->pIdent);
		FREE(psCurr);
	}
	psGlobalVars = NULL;

	/* Reset the global array symbol table */
	for(psCurr = psGlobalArrays; psCurr != NULL; psCurr = psNext)
	{
		psNext = psCurr->psNext;
		FREE(psCurr->pIdent);
		FREE(psCurr);
	}
	psGlobalArrays = NULL;

	// Reset the trigger table
	for(psTCurr = psTriggers; psTCurr; psTCurr = psTNext)
	{
		psTNext = psTCurr->psNext;
		if (psTCurr->psDebug)
		{
			FREE(psTCurr->psDebug);
		}
		if (psTCurr->pCode)
		{
			FREE(psTCurr->pCode);
		}
		FREE(psTCurr->pIdent);
		FREE(psTCurr);
	}
	psTriggers = NULL;
	numTriggers = 0;

	// Reset the event table
	for(psECurr = psEvents; psECurr; psECurr = psENext)
	{
		psENext = psECurr->psNext;
		if (psECurr->psDebug)
		{
			FREE(psECurr->psDebug);
		}
		FREE(psECurr->pIdent);
		FREE(psECurr->pCode);
		FREE(psECurr);
	}
	psEvents = NULL;
	numEvents = 0;

	/* Reset the function symbol table */
	for(psFCurr = psFunctions; psFCurr != NULL; psFCurr = psFNext)
	{
		psFNext = psFCurr->psNext;
		FREE_DEBUG(psFCurr);
		FREE(psFCurr->pIdent);
		FREE(psFCurr->pCode);
		FREE(psFCurr);
	}
	psFunctions = NULL;
}

/* Compile a script program */
BOOL scriptCompile(UBYTE *pData, UDWORD fileSize,
				   SCRIPT_CODE **ppsProg, SCR_DEBUGTYPE debugType)
{
	// Tell lex about the input buffer
	scriptSetInputBuffer(pData, fileSize);

	scriptResetTables();
	psFinalProg = NULL;
	if (debugType == SCR_DEBUGINFO)
	{
		genDebugInfo = TRUE;
	}
	else
	{
		genDebugInfo = FALSE;
	}

	if (scr_parse() != 0)
	{
		return FALSE;
	}

	scriptResetTables();

	*ppsProg = psFinalProg;

	return TRUE;
}


/* A simple error reporting routine */
void scr_error(char *pMessage, ...)
{
	int		line;
	char	*text;
	va_list	args;
	STRING	aBuff[1024];

	va_start(args, pMessage);
	vsprintf(aBuff, pMessage, args);
	va_end(args);
	scriptGetErrorData(&line, &text);
#ifdef DEBUG
	ASSERT((FALSE, "script parse error:\n%s at line %d\nToken: %d, Text: '%s'\n",
			  aBuff, line, scr_char, text));
#else
	DBERROR(("script parse error:\n%s at line %d\nToken: %d, Text: '%s'\n",
			  pMessage, line, scr_char, text));
#endif
}


/* Look up a type symbol */
BOOL scriptLookUpType(STRING *pIdent, INTERP_TYPE *pType)
{
	UDWORD	i;

	if (asScrTypeTab)
	{
		for(i=0; asScrTypeTab[i].typeID != 0; i++)
		{
			if (strcmp(asScrTypeTab[i].pIdent, pIdent) == 0)
			{
				*pType = asScrTypeTab[i].typeID;
				return TRUE;
			}
		}
	}

	return FALSE;
}


/* Reset the local variable symbol table at the end of a function */
void scriptClearLocalVariables(void)
{
	VAR_SYMBOL	*psCurr, *psNext;

	for(psCurr = psLocalVars; psCurr != NULL; psCurr = psNext)
	{
		psNext = psCurr->psNext;
		FREE(psCurr->pIdent);
		FREE(psCurr);
	}
}

/* Add a new variable symbol.
 * If localVariableDef is true a local variable symbol is defined,
 * otherwise a global variable symbol is defined.
 */
//BOOL scriptAddVariable(STRING *pIdent, INTERP_TYPE type, STORAGE_TYPE storage, SDWORD elements)
BOOL scriptAddVariable(VAR_DECL *psStorage, VAR_IDENT_DECL *psVarIdent)
{
	VAR_SYMBOL		*psNew;
	SDWORD			i;//, size;

	/* Allocate the memory for the symbol structure */
	psNew = (VAR_SYMBOL *)MALLOC(sizeof(VAR_SYMBOL));
	if (psNew == NULL)
	{
		scr_error("Out of memory");
		return FALSE;
	}
	psNew->pIdent = psVarIdent->pIdent; //(STRING *)MALLOC(strlen(pIdent) + 1);
/*	if (psNew->pIdent == NULL)
	{
		scr_error("Out of memory");
		return FALSE;
	}*/

	/* Intialise the symbol structure */
//	strcpy(psNew->pIdent, pIdent);
	psNew->type = psStorage->type;
	psNew->storage = psStorage->storage;
	psNew->dimensions = psVarIdent->dimensions;
	for(i=0; i<psNew->dimensions; i++)
	{
		psNew->elements[i] = psVarIdent->elements[i];
	}
	if (psNew->dimensions == 0)
	{
		if (psGlobalVars == NULL)
		{
			psNew->index = 0;
		}
		else
		{
			psNew->index = psGlobalVars->index + 1;
		}

		/* Add the symbol to the list */
		psNew->psNext = psGlobalVars;
		psGlobalVars = psNew;
	}
	else
	{
		if (psGlobalArrays == NULL)
		{
			psNew->index = 0;
		}
		else
		{
			psNew->index = psGlobalArrays->index + 1;
		}

		psNew->psNext = psGlobalArrays;
		psGlobalArrays = psNew;
	}

	return TRUE;
}


/* Look up a variable symbol */
BOOL scriptLookUpVariable(STRING *pIdent, VAR_SYMBOL **ppsSym)
{
	VAR_SYMBOL		*psCurr;

	/* See if the symbol is an object variable */
	if (asScrObjectVarTab && objVarContext != 0)
	{
		for(psCurr = asScrObjectVarTab; psCurr->pIdent != NULL; psCurr++)
		{
			if (interpCheckEquiv(psCurr->objType, objVarContext) &&
				strcmp(psCurr->pIdent, pIdent) == 0)
			{
				*ppsSym = psCurr;
				return TRUE;
			}
		}
	}

	/* See if the symbol is an external variable */
	if (asScrExternalTab)
	{
		for(psCurr = asScrExternalTab; psCurr->pIdent != NULL; psCurr++)
		{
			if (strcmp(psCurr->pIdent, pIdent) == 0)
			{
				*ppsSym = psCurr;
				return TRUE;
			}
		}
	}

	/* See if the symbol is in the local variable list */
	for(psCurr = psLocalVars; psCurr != NULL; psCurr = psCurr->psNext)
	{
		if (strcmp(psCurr->pIdent, pIdent) == 0)
		{
			*ppsSym = psCurr;
			return TRUE;
		}
	}

	/* See if the symbol is in the global variable list.
	 * This is not checked for when local variables are being defined.
	 * This allows local variables to have the same name as global ones.
	 */
	if (!localVariableDef)
	{
		for(psCurr = psGlobalVars; psCurr != NULL; psCurr = psCurr->psNext)
		{
			if (strcmp(psCurr->pIdent, pIdent) == 0)
			{
				*ppsSym = psCurr;
				return TRUE;
			}
		}
		for(psCurr = psGlobalArrays; psCurr != NULL; psCurr = psCurr->psNext)
		{
			if (strcmp(psCurr->pIdent, pIdent) == 0)
			{
				*ppsSym = psCurr;
				return TRUE;
			}
		}
	}

	/* Failed to find the variable */
	*ppsSym = NULL;
	return FALSE;
}


/* Add a new trigger symbol */
BOOL scriptAddTrigger(STRING *pIdent, TRIGGER_DECL *psDecl, UDWORD line)
{
	TRIGGER_SYMBOL		*psTrigger, *psCurr, *psPrev;

	// Allocate the trigger
	psTrigger = MALLOC(sizeof(TRIGGER_SYMBOL));
	if (!psTrigger)
	{
		scr_error("Out of memory");
		return FALSE;
	}
	psTrigger->pIdent = MALLOC(strlen(pIdent) + 1);
	if (!psTrigger->pIdent)
	{
		scr_error("Out of memory");
		return FALSE;
	}
	strcpy(psTrigger->pIdent, pIdent);
	if (psDecl->size > 0)
	{
		psTrigger->pCode = MALLOC(psDecl->size);
		if (!psTrigger->pCode)
		{
			scr_error("Out of memory");
			return FALSE;
		}
		memcpy(psTrigger->pCode, psDecl->pCode, psDecl->size);
	}
	else
	{
		psTrigger->pCode = NULL;
	}
	psTrigger->size = psDecl->size;
	psTrigger->type = psDecl->type;
	psTrigger->time = psDecl->time;
	psTrigger->index = numTriggers++;
	psTrigger->psNext = NULL;

	// Add debug info
	if (genDebugInfo)
	{
		psTrigger->psDebug = MALLOC(sizeof(SCRIPT_DEBUG));
		psTrigger->psDebug[0].offset = 0;
		psTrigger->psDebug[0].line = line;
		psTrigger->debugEntries = 1;
	}
	else
	{
		psTrigger->debugEntries = 0;
		psTrigger->psDebug = NULL;
	}


	// Store the trigger
	psPrev = NULL;
	for(psCurr = psTriggers; psCurr; psCurr = psCurr->psNext)
	{
		psPrev = psCurr;
	}
	if (psPrev)
	{
		psPrev->psNext = psTrigger;
	}
	else
	{
		psTriggers = psTrigger;
	}

	return TRUE;
}


/* Lookup a trigger symbol */
BOOL scriptLookUpTrigger(STRING *pIdent, TRIGGER_SYMBOL **ppsTrigger)
{
	TRIGGER_SYMBOL	*psCurr;

	for(psCurr = psTriggers; psCurr; psCurr=psCurr->psNext)
	{
		if (strcmp(pIdent, psCurr->pIdent) == 0)
		{
			*ppsTrigger = psCurr;
			return TRUE;
		}
	}

	return FALSE;
}


/* Lookup a callback trigger symbol */
BOOL scriptLookUpCallback(STRING *pIdent, CALLBACK_SYMBOL **ppsCallback)
{
	CALLBACK_SYMBOL		*psCurr;

	if (!asScrCallbackTab)
	{
		return FALSE;
	}

	for(psCurr = asScrCallbackTab; psCurr->type != 0; psCurr += 1)
	{
		if (strcmp(pIdent, psCurr->pIdent) == 0)
		{
			*ppsCallback = psCurr;
			return TRUE;
		}
	}
	return FALSE;
}

/* Add a new event symbol */
BOOL scriptDeclareEvent(STRING *pIdent, EVENT_SYMBOL **ppsEvent)
{
	EVENT_SYMBOL		*psEvent, *psCurr, *psPrev;

	// Allocate the event
	psEvent = MALLOC(sizeof(EVENT_SYMBOL));
	if (!psEvent)
	{
		scr_error("Out of memory");
		return FALSE;
	}
	psEvent->pIdent = MALLOC(strlen(pIdent) + 1);
	if (!psEvent->pIdent)
	{
		scr_error("Out of memory");
		return FALSE;
	}
	strcpy(psEvent->pIdent, pIdent);
	psEvent->pCode = NULL;
	psEvent->size = 0;
	psEvent->psDebug = NULL;
	psEvent->debugEntries = 0;
	psEvent->index = numEvents++;
	psEvent->psNext = NULL;

	// Add the event to the list
	psPrev = NULL;
	for(psCurr = psEvents; psCurr; psCurr = psCurr->psNext)
	{
		psPrev = psCurr;
	}
	if (psPrev)
	{
		psPrev->psNext = psEvent;
	}
	else
	{
		psEvents = psEvent;
	}

	*ppsEvent = psEvent;

	return TRUE;
}

// Add the code to a defined event
BOOL scriptDefineEvent(EVENT_SYMBOL *psEvent, CODE_BLOCK *psCode, SDWORD trigger)
{
	// Store the event code
	psEvent->pCode = MALLOC(psCode->size);
	if (!psEvent->pCode)
	{
		scr_error("Out of memory");
		return FALSE;
	}
	memcpy(psEvent->pCode, psCode->pCode, psCode->size);
	psEvent->size = psCode->size;
	psEvent->trigger = trigger;

	// Add debug info
	if (genDebugInfo)
	{
		psEvent->psDebug = MALLOC(sizeof(SCRIPT_DEBUG) * psCode->debugEntries);
		if (!psEvent->psDebug)
		{
			scr_error("Out of memory");
			return FALSE;
		}
		memcpy(psEvent->psDebug, psCode->psDebug,
			sizeof(SCRIPT_DEBUG) * psCode->debugEntries);
		psEvent->debugEntries = psCode->debugEntries;
	}
	else
	{
		psEvent->debugEntries = 0;
		psEvent->psDebug = NULL;
	}

	return TRUE;
}

/* Lookup an event symbol */
BOOL scriptLookUpEvent(STRING *pIdent, EVENT_SYMBOL **ppsEvent)
{
	EVENT_SYMBOL	*psCurr;

	for(psCurr = psEvents; psCurr; psCurr=psCurr->psNext)
	{
		if (strcmp(pIdent, psCurr->pIdent) == 0)
		{
			*ppsEvent = psCurr;
			return TRUE;
		}
	}

	return FALSE;
}


/* Look up a constant variable symbol */
BOOL scriptLookUpConstant(STRING *pIdent, CONST_SYMBOL **ppsSym)
{
	CONST_SYMBOL	*psCurr;

	/* Scan the Constant list */
	if (asScrConstantTab)
	{
		for(psCurr = asScrConstantTab; psCurr->type != VAL_VOID; psCurr++)
		{
			if (strcmp(psCurr->pIdent, pIdent) == 0)
			{
				*ppsSym = psCurr;
				return TRUE;
			}
		}
	}

	return FALSE;
}


/* Look up a function symbol */
BOOL scriptLookUpFunction(STRING *pIdent, FUNC_SYMBOL **ppsSym)
{
	UDWORD i;
	FUNC_SYMBOL	*psCurr;

	/* See if the function is defined as an instinct function */
	if (asScrInstinctTab)
	{
		for(i = 0; asScrInstinctTab[i].pFunc != NULL; i++)
		{
			if (strcmp(asScrInstinctTab[i].pIdent, pIdent) == 0)
			{
				*ppsSym = asScrInstinctTab + i;
				return TRUE;
			}
		}
	}

	/* See if the function is defined as a script function */
	for(psCurr = psFunctions; psCurr != NULL; psCurr = psCurr->psNext)
	{
		if (strcmp(psCurr->pIdent, pIdent) == 0)
		{
			*ppsSym = psCurr;
			return TRUE;
		}
	}

	/* Failed to find the indentifier */
	*ppsSym = NULL;
	return FALSE;
}


/* Set the type table */
void scriptSetTypeTab(TYPE_SYMBOL *psTypeTab)
{
#ifdef DEBUG
	SDWORD			i;
	INTERP_TYPE		type;

	for(i=0, type=VAL_USERTYPESTART; psTypeTab[i].typeID != 0; i++)
	{
		ASSERT((psTypeTab[i].typeID == type,
			"scriptSetTypeTab: ID's must be >= VAL_USERTYPESTART and sequential"));
		type += 1;
	}
#endif

	asScrTypeTab = psTypeTab;
}


/* Set the function table */
void scriptSetFuncTab(FUNC_SYMBOL *psFuncTab)
{
	asScrInstinctTab = psFuncTab;
}

/* Set the object variable table */
void scriptSetObjectTab(VAR_SYMBOL *psObjTab)
{
	asScrObjectVarTab = psObjTab;
}

/* Set the external variable table */
void scriptSetExternalTab(VAR_SYMBOL *psExtTab)
{
	asScrExternalTab = psExtTab;
}

/* Set the constant table */
void scriptSetConstTab(CONST_SYMBOL *psConstTab)
{
	asScrConstantTab = psConstTab;
}

/* Set the callback table */
void scriptSetCallbackTab(CALLBACK_SYMBOL *psCallTab)
{
#ifdef DEBUG
	SDWORD			i;
	TRIGGER_TYPE	type;

	for(i=0, type=TR_CALLBACKSTART; psCallTab[i].type != 0; i++)
	{
		ASSERT((psCallTab[i].type == type,
			"scriptSetCallbackTab: ID's must be >= VAL_CALLBACKSTART and sequential"));
		type += 1;
	}
#endif

	asScrCallbackTab = psCallTab;
}
