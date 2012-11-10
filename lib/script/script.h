/*
 * Script.h
 *
 * Interface to the script library
 */
#ifndef _script_h
#define _script_h

#include "interp.h"
#include "stack.h"
#include "codeprint.h"
#include "parse.h"
#include "event.h"
#include "evntsave.h"


/* Whether to include debug info when compiling */
typedef enum _scr_debugtype
{
	SCR_DEBUGINFO,		// Generate debug info
	SCR_NODEBUG,		// Do not generate debug info
} SCR_DEBUGTYPE;


// If this is defined we save out the compiled scripts
#define SCRIPTTYPE SCR_DEBUGINFO

// Initialise the script library
extern BOOL scriptInitialise(EVENT_INIT *psInit);

// Shutdown the script library
extern void scriptShutDown(void);

/***********************************************************************************
 *
 * Compiler setup functions
 */

/* Set the type table */
extern void scriptSetTypeTab(TYPE_SYMBOL *psTypeTab);

/* Set the function table */
extern void scriptSetFuncTab(FUNC_SYMBOL *psFuncTab);

/* Set the external variable table */
extern void scriptSetExternalTab(VAR_SYMBOL *psExtTab);

/* Set the object variable table */
extern void scriptSetObjectTab(VAR_SYMBOL *psObjTab);

/* Set the constant table */
extern void scriptSetConstTab(CONST_SYMBOL *psConstTab);

/* Set the callback table */
extern void scriptSetCallbackTab(CALLBACK_SYMBOL *psCallTab);

/* Set the type equivalence table */
extern void scriptSetTypeEquiv(TYPE_EQUIV *psTypeTab);

/***********************************************************************************
 *
 * Compiler functions
 */

/* Compile a script program */
extern BOOL scriptCompile(UBYTE *pData, UDWORD fileSize,
						  SCRIPT_CODE **ppsProg, SCR_DEBUGTYPE debugType);

/* Free a SCRIPT_CODE structure */
extern void scriptFreeCode(SCRIPT_CODE *psCode);

/* Display the contents of a program in readable form */
extern void cpPrintProgram(SCRIPT_CODE *psProg);

// Save a binary version of a program
extern BOOL scriptSaveProg(SCRIPT_CODE *psProg, UDWORD *pSize, UBYTE **ppData);

// Load a binary version of a program
extern BOOL scriptLoadProg(UDWORD size, UBYTE *pData, SCRIPT_CODE **ppsProg);

/* Lookup a script variable */
extern BOOL scriptGetVarIndex(SCRIPT_CODE *psCode, STRING *pID, UDWORD *pIndex);

/* Run a compiled script */
extern BOOL interpRunScript(SCRIPT_CONTEXT *psContext, INTERP_RUNTYPE runType,
							UDWORD index, UDWORD offset);


/***********************************************************************************
 *
 * Event system functions
 */

// Whether a context is released when there are no active triggers for it
typedef enum _context_release
{
	CR_RELEASE,		// release the context
	CR_NORELEASE,	// do not release the context
} CONTEXT_RELEASE;

// reset the event system
extern void eventReset(void);

// Initialise the create/release function array - specify the maximum value type
extern BOOL eventInitValueFuncs(SDWORD maxType);

// a create function for data stored in an INTERP_VAL
typedef BOOL (*VAL_CREATE_FUNC)(INTERP_VAL *psVal);

// a release function for data stored in an INTERP_VAL
typedef void (*VAL_RELEASE_FUNC)(INTERP_VAL *psVal);

// Add a new value create function
extern BOOL eventAddValueCreate(INTERP_TYPE type, VAL_CREATE_FUNC create);

// Add a new value release function
extern BOOL eventAddValueRelease(INTERP_TYPE type, VAL_RELEASE_FUNC release);

// Create a new context for a script
extern BOOL eventNewContext(SCRIPT_CODE *psCode,
							CONTEXT_RELEASE release, SCRIPT_CONTEXT **ppsContext);

// Copy a context, including variable values
extern BOOL eventCopyContext(SCRIPT_CONTEXT *psContext, SCRIPT_CONTEXT **ppsNew);

// Add a new object to the trigger system
// Time is the application time at which all the triggers are to be started
extern BOOL eventRunContext(SCRIPT_CONTEXT *psContext, UDWORD time);

// Remove a context from the event system
extern void eventRemoveContext(SCRIPT_CONTEXT *psContext);

// Set a global variable value for a context
extern BOOL eventSetContextVar(SCRIPT_CONTEXT *psContext, UDWORD index,
							   INTERP_TYPE type, UDWORD data);

// Get the value pointer for a variable index
extern BOOL eventGetContextVal(SCRIPT_CONTEXT *psContext, UDWORD index,
							   INTERP_VAL **ppsVal);

// Process all the currently active triggers
// Time is the application time at which all the triggers are to be processed
extern void eventProcessTriggers(UDWORD currTime);

// Activate a callback trigger
extern void eventFireCallbackTrigger(TRIGGER_TYPE callback);

/***********************************************************************************
 *
 * Support functions for writing instinct functions
 */

/* Pop a number of values off the stack checking their types
 * This is used by instinct functions to get their parameters
 * The varargs part is a set of INTERP_TYPE, UDWORD * pairs.
 * The value of the parameter is stored in the DWORD pointed to by the UDWORD *
 */
extern BOOL stackPopParams(SDWORD numParams, ...);

/* Push a value onto the stack without using a value structure */
extern BOOL stackPushResult(INTERP_TYPE type, SDWORD data);

/***********************************************************************************
 *
 * Script library instinct functions
 *
 * These would be declared in the function symbol array:
 *
 *	{ "traceOn",				interpTraceOn,			VAL_VOID,
 *		0, { VAL_VOID } },
 *	{ "traceOff",				interpTraceOff,			VAL_VOID,
 *		0, { VAL_VOID } },
 *	{ "setEventTrigger",		eventSetTrigger,		VAL_VOID,
 *		2, { VAL_EVENT, VAL_TRIGGER } },
 *	{ "eventTraceLevel",		eventSetTraceLevel,		VAL_VOID,
 *		1, { VAL_INT } },
 *
 */

/* Instinct function to turn on tracing */
extern BOOL interpTraceOn(void);

/* Instinct function to turn off tracing */
extern BOOL interpTraceOff(void);

// Change the trigger assigned to an event
// This is an instinct function that takes a VAL_EVENT and VAL_TRIGGER as parameters
extern BOOL eventSetTrigger(void);

// set the event tracing level
//   0 - no tracing
//   1 - only fired triggers
//   2 - added and fired triggers
//   3 - as 2 but show tested but not fired triggers as well
extern BOOL eventSetTraceLevel(void);

#endif

