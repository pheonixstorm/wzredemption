/*
 * Netplay.h
 *
 * Alex Lee sep97.
 */ 

#ifndef _netplay_h
#define _netplay_h

// ////////////////////////////////////////////////////////////////////////
// Include this file in your game to add multiplayer facilities.

#pragma warning (disable : 4201 4214 4115 4514)
#include <dplay.h>
#include <dplobby.h>
#pragma warning (default : 4201 4214 4115)

#define IDIRECTPLAY2_OR_GREATER

// Constants
#define MaxNumberOfPlayers	8					// max number of players in a game.
#define MaxMsgSize			8000				// max size of a message in bytes.
#define	StringSize			64					// size of strings used.
#define ConnectionSize		255					// max size of a connection description.
#define MaxProtocols		12					// max number of returnable protocols.
#define MaxGames			12					// max number of concurrently playable games to allow.
typedef	unsigned	int		UDWORD;				// for similarity to warzone
//#define USE_DIRECTPLAY_PROTOCOL				// use DX6 protocol.

//typedef struct {								//Available game storage... JUST FOR REFERENCE!
//    DWORD dwSize;
//    DWORD dwFlags;
//    GUID  guidInstance;
//    GUID  guidApplication;
//    DWORD dwMaxPlayers;
//    DWORD dwCurrentPlayers;
//    union  {
//        LPWSTR lpszSessionName;
//        LPSTR  lpszSessionNameA;};
//    union  {
//        LPWSTR lpszPassword;
//        LPSTR  lpszPasswordA;  };
//    DWORD dwReserved1;
//    DWORD dwReserved2;
//    DWORD dwUser1;
//    DWORD dwUser2;
//    DWORD dwUser3;
//    DWORD dwUser4;
//} DPSESSIONDESC2, FAR *LPDPSESSIONDESC2;

// Games Storage Structures
typedef struct{
	char			name[StringSize];	
	DPSESSIONDESC2	desc;
} GAMESTRUCT;

// ////////////////////////////////////////////////////////////////////////
// Protocol storage....
typedef struct{
	char	name[StringSize];
	GUID	guid;
	DWORD	size;
	char	connection[ConnectionSize];
} PROTO;
	
// ////////////////////////////////////////////////////////////////////////
// Message information. ie. the packets sent between machines.
typedef struct {
	unsigned short	size;		
	unsigned char	paddedBytes;		// numberofbytes appended for encryption
	unsigned char	type;		
	char 			body[MaxMsgSize];
} NETMSG, *LPNETMSG;

#define		ENCRYPTFLAG		100			// added to type to determine packet is encrypted.
#define		AUDIOMSG		255			// an audio packet (special message);
#define		FILEMSG			254			// a file packet

// ////////////////////////////////////////////////////////////////////////
// Player information. Update using NETplayerinfo
typedef struct{
	DPID dpid;
	char name[StringSize];
	BOOL bHost;						// a bool.
	BOOL bSpectator;
}PLAYER;

// ////////////////////////////////////////////////////////////////////////
// all the luvly Netplay info....
typedef struct{
	PROTO				protocols[MaxProtocols];	// the array of available protocols.
	GAMESTRUCT			games[MaxGames];			// the collection of games
	PLAYER				players[MaxNumberOfPlayers];// the array of players.
	UDWORD				playercount;				// number of players in game.

	LPDIRECTPLAY4A		lpDirectPlay4A;				// IDirectPlay4A interface pointer
	HANDLE				hPlayerEvent;				// player event to use
	DPID				dpidPlayer;					// ID of player created

	BOOL				bComms;						// actually do the comms?
	BOOL				bHost;						// TRUE if we are hosting the session
	BOOL				bLobbyLaunched;				// true if app launched by a lobby
	BOOL				bSpectator;					// true if just spectating

	BOOL				bEncryptAllPackets;			// set to true to encrypt all communications.
	UDWORD				cryptKey[4];				// 4*32 bit encryption key

	BOOL				bCaptureInUse;				// true if someone is speaking.
	BOOL				bAllowCaptureRecord;		// true if speech can be recorded.
	BOOL				bAllowCapturePlay;			// true if speech can be played.
}NETPLAY, *LPNETPLAY;

// ////////////////////////////////////////////////////////////////////////
// variables

extern NETPLAY				NetPlay;
extern LPNETPLAY			lpNetPlay;

extern GUID					GAME_GUID;					// id unique to the game under devlopment.
extern LPDIRECTPLAY4		glpDP;						// pointer to the dplay interface.
extern LPDIRECTPLAYLOBBY3	glpDPL3;					// pointer to the dplay lobby interface.
extern LPDIRECTPLAYLOBBYA	glpDPL;						// pointer to the dplay lobby interface.

// ////////////////////////////////////////////////////////////////////////
// functions available to you.
extern BOOL	   NETinit			(GUID g,BOOL bFirstCall);				//init(guid can be NULL)		
extern BOOL	   NETfindProtocol	(BOOL Lob);								//put connections in Protocols[] (Lobbies optional)
extern BOOL	   NETselectProtocol(LPVOID lpConnection);					//choose one. 
extern BOOL	   NETsend			(NETMSG *msg, DPID player, BOOL guarantee);		// send to player, possibly guaranteed
extern BOOL	   NETbcast			(NETMSG *msg,BOOL guarantee);			// broadcast to everyone, possibly guaranteed
extern BOOL	   NETrecv			(NETMSG *msg);							// recv a message if possible

extern UCHAR   NETsendFile		(BOOL newFile, CHAR *fileName, DPID player);	// send file chunk.
extern UCHAR   NETrecvFile		(NETMSG *pMsg);							// recv file chunk

extern HRESULT NETclose					(VOID);							// close current game
extern HRESULT NETshutdown				(VOID);							// leave the game in play.

extern UDWORD  NETgetBytesSent			(VOID);							// return bytes sent/recv.  call regularly for good results
extern UDWORD  NETgetPacketsSent		(VOID);							// return packets sent/recv.  call regularly for good results
extern UDWORD  NETgetBytesRecvd			(VOID);							// return bytes sent/recv.  call regularly for good results
extern UDWORD  NETgetPacketsRecvd		(VOID);							// return packets sent/recv.  call regularly for good results
extern UDWORD  NETgetRecentBytesSent	(VOID);							// more immediate functions.
extern UDWORD  NETgetRecentPacketsSent	(VOID);
extern UDWORD  NETgetRecentBytesRecvd	(VOID);
extern UDWORD  NETgetRecentPacketsRecvd	(VOID);

// from netjoin.c
extern DWORD   NETgetGameFlags			(UDWORD flag);					// return one of the four flags(dword) about the game.
extern DWORD   NETgetGameFlagsUnjoined	(UDWORD gameid,UDWORD flag);	// return one of the four flags(dword) about the game.
extern BOOL	   NETsetGameFlags			(UDWORD flag,DWORD value);		// set game flag(1-4) to value.		
extern BOOL	   NEThaltJoining			(VOID);							// stop new players joining this game
extern BOOL	   NETfindGame				(BOOL asynchronously);			// find games being played(uses GAME_GUID);
extern BOOL	   NETjoinGame				(GUID guidSessionInstance, LPSTR playername);	// join game given with playername
extern BOOL	   NEThostGame				(LPSTR SessionName, LPSTR PlayerName,			// host a game 
											DWORD one,DWORD two,DWORD three,DWORD four,UDWORD plyrs);

//from netusers.c
extern BOOL	   NETuseNetwork			(BOOL val);						// TURN on/off networking.
extern UDWORD  NETplayerInfo			(LPGUID guidinstance);			// count players in this game.
extern BOOL	   NETchangePlayerName		(UDWORD dpid, char *newName);	// change a players name.
extern BOOL	   NETgetLocalPlayerData	(DPID dpid,VOID *pData, DWORD *pSize);
extern BOOL	   NETgetGlobalPlayerData	(DPID dpid,VOID *pData, DWORD *pSize);
extern BOOL    NETsetLocalPlayerData	(DPID dpid,VOID *pData, DWORD size);
extern BOOL    NETsetGlobalPlayerData	(DPID dpid,VOID *pData, DWORD size);

extern BOOL	   NETspectate				(GUID guidSessionInstance);		// create a spectator
extern BOOL	   NETisSpectator			(DPID dpid);					// check for spectator staus.

//from netsupp
extern BOOL		NETlogEntry				(CHAR *str,UDWORD a,UDWORD b);
extern BOOL		NETstopLogging			(VOID);
extern BOOL		NETstartLogging			(VOID);
	
// from net audio.
extern BOOL		NETprocessAudioCapture	(VOID);							//capture
extern BOOL		NETstopAudioCapture		(VOID);
extern BOOL		NETstartAudioCapture	(VOID);
extern BOOL		NETshutdownAudioCapture	(VOID);
extern BOOL		NETinitAudioCapture		(VOID);

extern BOOL		NETinitPlaybackBuffer	(VOID *pDs);					// playback
extern VOID		NETplayIncomingAudio	(NETMSG *pMsg);
extern BOOL		NETqueueIncomingAudio	(LPBYTE lpbSoundData, DWORD dwSoundBytes,BOOL bStream);
extern BOOL		NETshutdownAudioPlayback(VOID);

// encryption
extern BOOL		NETsetKey				(UDWORD c1,UDWORD c2,UDWORD c3, UDWORD c4);
extern NETMSG*  NETmanglePacket			(NETMSG *msg);
extern VOID		NETunmanglePacket		(NETMSG *msg);
extern BOOL		NETmangleData			(long *input, long *result, UDWORD dataSize);
extern BOOL		NETunmangleData			(long *input, long *result, UDWORD dataSize);
extern UDWORD	NEThashFile				(char *pFileName);
extern UCHAR	NEThashVal				(UDWORD value);
extern UDWORD	NEThashBuffer			(unsigned char *pData, UDWORD size);


// YOU MUST PROVIDE THIS FUNCTION!!!!
extern BOOL DirectPlaySystemMessageHandler(LPVOID);						// what to do with system messages.

#include "netprov.h"													// more functions to override dialog boxes
#include "netlobby.h"													// more functions to provide lobby facilities.

// Some shortcuts to help you along!
#define NetAdd(m,pos,thing) \
	memcpy(&(m.body[pos]),&(thing),sizeof(thing)) 

#define NetAdd2(m,pos,thing) \
	memcpy( &((*m).body[pos]), &(thing), sizeof(thing)) 

#define NetGet(m,pos,thing) \
	memcpy(&(thing),&(m->body[pos]),sizeof(thing))

#define NetAddSt(m,pos,stri) \
	strcpy(&(m.body[pos]),stri)

#define NetGetSt(m,pos,stri) \
	strcpy(stri,&(m->body[pos]))

#endif