/*
 * SeqDisp.h
 *
 * Functions for the display of the Escape Sequences
 */

#ifndef _SeqDisp_h
#define _SeqDisp_h

#ifdef WIN32			// ffs js (bastard)
#include "pieMode.h"
#endif
/***************************************************************************/
/*
 *	Global Definitions
 */
/***************************************************************************/

#define  SEQUENCE_PLAY 0//play once and exit
#define  SEQUENCE_LOOP 1//loop till stopped externally
#define  SEQUENCE_PAUSE 2//pause time
#define  SEQUENCE_KILL 3//stop
#define  SEQUENCE_HOLD 4//play once and hold last frame

#define  SEQ_TEXT_POSITION  0//position text 
#define  SEQ_TEXT_FOLLOW_ON 1//justify if less than 3/4 length
#define  SEQ_TEXT_JUSTIFY   2//justify if less than 520/600

/***************************************************************************/
/*
 *	Global Variables
 */
/***************************************************************************/

/***************************************************************************/
/*
 *	Global ProtoTypes
 */
/***************************************************************************/
//buffer render
extern BOOL	seq_RenderVideoToBuffer(iSurface *pSurface, char* sequenceName, int time, int seqCommand);
extern BOOL	seq_BlitBufferToScreen(char* screen, SDWORD screenStride, SDWORD xOffset, SDWORD yOffset);

//full screen render
//extern BOOL seq_PlayVideo(char* pSeq, char* pAudio);
//extern BOOL seq_StartFullScreenVideo(char* sequenceFile, char* audioFile);//start videos through seqList 
#ifdef WIN32
extern BOOL seq_UpdateFullScreenVideo(CLEAR_MODE *bClear);
#else
extern BOOL seq_UpdateFullScreenVideo(void);
#endif
extern BOOL seq_StopFullScreenVideo(void);
//control
extern BOOL	seq_SetupVideoBuffers(void);
extern BOOL	seq_ReleaseVideoBuffers(void);
extern BOOL seq_GetVideoSize(SDWORD* pWidth, SDWORD* pHeight);
//text
extern BOOL seq_AddTextForVideo(UBYTE* pText, SDWORD xOffset, SDWORD yOffset, SDWORD startTime, SDWORD endTime, SDWORD bJustify, UDWORD PSXSeqNumber);
extern BOOL seq_ClearTextForVideo(void);
//clear the sequence list
extern void seq_ClearSeqList(void);
//add a sequence to the list to be played
extern void seq_AddSeqToList(STRING *pSeqName, STRING *pAudioName, STRING *pTextName, BOOL bLoop, UDWORD PSXSeqNumber);
/*checks to see if there are any sequences left in the list to play*/
extern BOOL seq_AnySeqLeft(void);

//set and check subtitle mode, TRUE subtitles on
extern void seq_SetSubtitles(BOOL bNewState);
extern BOOL seq_GetSubtitles(void);

/*returns the next sequence in the list to play*/
extern void seq_StartNextFullScreenVideo(void);
#endif	//SeqDisp.h
