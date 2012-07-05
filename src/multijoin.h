/*
 * Multijoin.h 
 * 
 * Alex Lee, pumpkin studios, 
 * multijoin caters for all the player comings and goings of each player
 */
extern BOOL sendVersionCheck			();
extern BOOL recvVersionCheck			(NETMSG *pMsg);
extern BOOL intDisplayMultiJoiningStatus(UBYTE joinCount);
extern BOOL MultiPlayerLeave			(DPID dp);						// A player has left the game.
extern BOOL MultiPlayerJoin				(DPID dp);						// A Player has joined the game.
extern VOID setupNewPlayer				(DPID dpid,UDWORD player);		// stuff to do when player joins.
//extern BOOL UpdateClient				(DPID dest, UDWORD playerToSend);// send info about another player
extern VOID clearPlayer					(UDWORD player,BOOL quietly,BOOL removeOil);// wipe a player off the face of the earth.
//extern BOOL ProcessDroidOrders			(VOID);
//extern BOOL recvFeatures				(NETMSG *pMsg);
//extern UDWORD							arenaPlayersReceived;

typedef struct {
	DROID *psDroid;
	VOID  *psNext;
}DROIDSTORE, *LPDROIDSTORE;

extern DROIDSTORE *tempDroidList;