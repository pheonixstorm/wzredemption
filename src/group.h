/*
 * Group.h
 *
 * Link droids together into a group for AI etc.
 *
 */
#ifndef _group_h
#define _group_h

#include "order.h"
#include "orderdef.h"

typedef enum _group_type
{
	GT_NORMAL,			// standard group
	GT_COMMAND,			// command droid group
	GT_TRANSPORTER,		// transporter group
} GROUP_TYPE;

typedef struct _droid_group
{
	SWORD		type;
	SWORD		refCount;
	DROID		*psList;			// list of droids in the group
	DROID		*psCommander;		// the command droid of a command group
	RUN_DATA	sRunData;			// where the group should retreat to
} DROID_GROUP;

// initialise the group system
BOOL grpInitialise(void);

// shutdown the group system
void grpShutDown(void);

// create a new group
BOOL grpCreate(DROID_GROUP	**ppsGroup);

// add a droid to a group
void grpJoin(DROID_GROUP *psGroup, DROID *psDroid);

// add a droid to a group at the end of the list
void grpJoinEnd(DROID_GROUP *psGroup, DROID *psDroid);

// remove a droid from a group
void grpLeave(DROID_GROUP *psGroup, DROID *psDroid);

// count the members of a group
SDWORD grpNumMembers(DROID_GROUP *psGroup);

// remove all droids from a group
void grpReset(DROID_GROUP *psGroup);

/* Give a group an order */
struct _droid_order_data;
//extern void orderGroupBase(DROID_GROUP *psGroup, struct _droid_order_data *psData);

/* Give a group an order */
extern void orderGroup(DROID_GROUP *psGroup, DROID_ORDER order);

/* Give a group of droids an order */
extern void orderGroupLoc(DROID_GROUP *psGroup, DROID_ORDER order, UDWORD x, UDWORD y);

/* Give a group of droids an order */
extern void orderGroupObj(DROID_GROUP *psGroup, DROID_ORDER order, BASE_OBJECT *psObj);

/* set the secondary state for a group of droids */
extern void grpSetSecondary(DROID_GROUP *psGroup, SECONDARY_ORDER sec, SECONDARY_STATE state);

#endif


