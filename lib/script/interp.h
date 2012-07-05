/*
 * Interp.h
 *
 * Script interpreter definitions
 */
#ifndef _interp_h
#define _interp_h

/* The possible value types for scripts */
typedef enum _interp_type
{
	// Basic types
	VAL_BOOL,
	VAL_INT,
//	VAL_FLOAT,
	VAL_STRING,

	// events and triggers
	VAL_TRIGGER,
	VAL_EVENT,

	/* Type used by the compiler for functions that do not return a value */
	VAL_VOID,

	VAL_USERTYPESTART,		// user defined types should start with this id
} INTERP_TYPE;

// flag to specify a variable reference rather than simple value
#define VAL_REF		0x00100000

/* A value consists of its type and value */
typedef struct _interp_val
{
	INTERP_TYPE		type;
	union
	{
		BOOL		bval;		// VAL_BOOL
		SDWORD		ival;		// VAL_INT
//		float		fval;		// VAL_FLOAT
		STRING		*sval;		// VAL_STRING
		void		*oval;		// VAL_OBJECT
		void		*pVoid;		// VAL_VOIDPTR
	} v;
} INTERP_VAL;


// maximum number of equivalent types for a type
#define INTERP_MAXEQUIV		10

// type equivalences
typedef struct _interp_typeequiv
{
	INTERP_TYPE		base;		// the type that the others are equivalent to
	SDWORD			numEquiv;	// number of equivalent types
	INTERP_TYPE		aEquivTypes[INTERP_MAXEQUIV];
								// the equivalent types
} TYPE_EQUIV;

/* Opcodes for the script interpreter */
typedef enum _op_code
{
	OP_PUSH,		// Push value onto stack
	OP_PUSHREF,		// Push a pointer to a variable onto the stack
	OP_POP,			// Pop value from stack

	OP_PUSHGLOBAL,	// Push the value of a global variable onto the stack
	OP_POPGLOBAL,	// Pop a value from the stack into a global variable

	OP_PUSHARRAYGLOBAL,	// Push the value of a global array variable onto the stack
	OP_POPARRAYGLOBAL,	// Pop a value from the stack into a global array variable

	OP_CALL,		// Call the 'C' function pointed to by the next value
	OP_VARCALL,		// Call the variable access 'C' function pointed to by the next value

	OP_JUMP,		// Jump to a different location in the script
	OP_JUMPTRUE,	// Jump if the top stack value is true
	OP_JUMPFALSE,	// Jump if the top stack value is false

	OP_BINARYOP,	// Call a binary maths/boolean operator
	OP_UNARYOP,		// Call a unary maths/boolean operator

	OP_EXIT,			// End the program
	OP_PAUSE,			// temporarily pause the current event

	// The following operations are secondary data to OP_BINARYOP and OP_UNARYOP

	// Maths operators
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_NEG,

	// Boolean operators
	OP_AND,
	OP_OR,
	OP_NOT,

	// Comparison operators
	OP_EQUAL,
	OP_NOTEQUAL,
	OP_GREATEREQUAL,
	OP_LESSEQUAL,
	OP_GREATER,
	OP_LESS,
} OPCODE;

/* How far the opcode is shifted up a UDWORD to allow other data to be
 * stored in the same UDWORD
 */
#define OPCODE_SHIFT		24
#define OPCODE_DATAMASK		0x00ffffff

// maximum sizes for arrays
#define VAR_MAX_DIMENSIONS		4
#define VAR_MAX_ELEMENTS		UBYTE_MAX

/* The mask for the number of array elements stored in the data part of an opcode */
#define ARRAY_BASE_MASK			0x000fffff
#define ARRAY_DIMENSION_SHIFT	20
#define ARRAY_DIMENSION_MASK	0x00f00000

/* The type of function called by an OP_CALL */
typedef BOOL (*SCRIPT_FUNC)(void);

/* The type of function called to access an object or in-game variable */
typedef BOOL (*SCRIPT_VARFUNC)(UDWORD index);

/* The possible storage types for a variable */
typedef enum _storage_type
{
	ST_PUBLIC,		// Public variable
	ST_PRIVATE,		// Private variable
	ST_OBJECT,		// A value stored in an objects data space.
	ST_EXTERN,		// An external value accessed by function call
} enum_STORAGE_TYPE;

typedef UBYTE STORAGE_TYPE;

/* Variable debugging info for a script */
typedef struct _var_debug
{
	STRING			*pIdent;
	STORAGE_TYPE	storage;
} VAR_DEBUG;

/* Array info for a script */
typedef struct _array_data
{
	UDWORD			base;			// the base index of the array values
	UBYTE			type;			// the array data type
	UBYTE			dimensions;
	UBYTE			elements[VAR_MAX_DIMENSIONS];
} ARRAY_DATA;

/* Array debug info for a script */
typedef struct _array_debug
{
	STRING			*pIdent;
	UBYTE			storage;
} ARRAY_DEBUG;

/* Line debugging information for a script */
typedef struct _script_debug
{
	UDWORD	offset;		// Offset in the compiled script that corresponds to
	UDWORD	line;		// this line in the original script.
	STRING	*pLabel;	// the trigger/event that starts at this line
} SCRIPT_DEBUG;

/* Different types of triggers */
typedef enum _trigger_type
{
	TR_INIT,		// Trigger fires when the script is first run
	TR_CODE,		// Trigger uses script code
	TR_WAIT,		// Trigger after a time pause
	TR_EVERY,		// Trigger at repeated intervals
	TR_PAUSE,		// Event has paused for an interval and will restart in the middle of it's code

	TR_CALLBACKSTART,	// The user defined callback triggers should start with this id
} TRIGGER_TYPE;

/* Description of a trigger for the SCRIPT_CODE */
typedef struct _trigger_data
{
	UWORD			type;		// Type of trigger
	UWORD			code;		// BOOL - is there code with this trigger
	UDWORD			time;		// How often to check the trigger
} TRIGGER_DATA;

/* A compiled script and its associated data */
typedef struct _script_code
{
	UDWORD			size;			// The size (in bytes) of the compiled code
	UDWORD			*pCode;			// Pointer to the compiled code

	UWORD			numTriggers;	// The number of triggers
	UWORD			numEvents;		// The number of events
	UWORD			*pTriggerTab;	// The table of trigger offsets
	TRIGGER_DATA	*psTriggerData;	// The extra info for each trigger
	UWORD			*pEventTab;		// The table of event offsets
	SWORD			*pEventLinks;	// The original trigger/event linkage
									// -1 for no link

	UWORD			numGlobals;		// The number of global variables
	UWORD			numArrays;		// the number of arrays in the program
	UDWORD			arraySize;		// the number of elements in all the defined arrays
	INTERP_TYPE		*pGlobals;		// Types of the global variables
	VAR_DEBUG		*psVarDebug;	// The names and storage types of variables
	ARRAY_DATA		*psArrayInfo;	// The sizes of the program arrays
	ARRAY_DEBUG		*psArrayDebug;	// Debug info for the arrays

	UWORD			debugEntries;	// Number of entries in psDebug
	SCRIPT_DEBUG	*psDebug;		// Debugging info for the script
} SCRIPT_CODE;


/* What type of code should be run by the interpreter */
typedef enum _interp_runtype
{
	IRT_TRIGGER,					// Run trigger code
	IRT_EVENT,						// Run event code
} INTERP_RUNTYPE;


/* The size of each opcode */
extern SDWORD aOpSize[];

/* Check if two types are equivalent */
extern BOOL interpCheckEquiv(INTERP_TYPE to, INTERP_TYPE from);

// Initialise the interpreter
extern BOOL interpInitialise(void);

// TRUE if the interpreter is currently running
extern BOOL interpProcessorActive(void);

#endif

