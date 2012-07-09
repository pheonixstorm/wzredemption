/***************************************************************************/

#include <stdio.h>

#include "frame.h"
#include "gtime.h"
#include "tracklib.h"
#include "priority.h"
#include "aud.h"
#include "trig.h"

/***************************************************************************/
/* defines */

#define	NO_SAMPLE				-2

#define	AUDIO_SAMPLE_HEAP_INIT	1000

#define	AUDIO_SAMPLE_HEAP_EXT	10

#define	MAX_SAME_SAMPLES		2

#define	AUDIO_QUEUE_SIZE		30

#define	LOWERED_VOL				AUDIO_VOL_MAX/4

/***************************************************************************/
/* structs */

typedef struct SDWVEC3D
{
	SDWORD	x, y, z;
}
SDWVEC3D;

/***************************************************************************/
/* externs */


/***************************************************************************/
/* static functions */


/***************************************************************************/
/* global variables */

static OBJ_HEAP		*g_psSampleHeap  = NULL;
static AUDIO_SAMPLE	*g_psSampleList  = NULL;
static AUDIO_SAMPLE	*g_psSampleQueue = NULL;

static BOOL		g_bAudioEnabled = FALSE;
static BOOL		g_bAudioPaused  = FALSE;
static BOOL		g_bStopAll      = FALSE;

static AUDIO_SAMPLE	g_sPreviousSample = { NO_SAMPLE,
										  SAMPLE_COORD_INVALID,
										  SAMPLE_COORD_INVALID,
										  SAMPLE_COORD_INVALID  };

static SDWORD	g_i3DVolume = AUDIO_VOL_MAX;

static CRITICAL_SECTION		critSecAudio;

/***************************************************************************/

BOOL
audio_Disabled( void )
{
	return !g_bAudioEnabled;
}

/***************************************************************************/

BOOL
audio_Init( HWND hWnd, BOOL bEnabled, AUDIO_CALLBACK pStopTrackCallback )
{
	/* if audio not enabled return TRUE to carry on game without audio */
	if ( bEnabled == FALSE )
	{
		g_bAudioEnabled = FALSE;
		return TRUE;
	}

	/* init audio system */
	g_bAudioEnabled = sound_Init( hWnd, MAX_SAME_SAMPLES );

	if ( g_bAudioEnabled == TRUE )
	{
		/* allocate sample heap */
		if ( !HEAP_CREATE( &g_psSampleHeap, AUDIO_SAMPLE_HEAP_INIT,
							AUDIO_SAMPLE_HEAP_EXT, sizeof(AUDIO_SAMPLE)) )
		{
			DBERROR( ("audio_Init: couldn't create sample queue\n") );
			return FALSE;
		}

		sound_SetStoppedCallback( pStopTrackCallback );
		
		InitializeCriticalSection( &critSecAudio );

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/***************************************************************************/

BOOL
audio_Shutdown()
{
	AUDIO_SAMPLE	*psSample = NULL, *psSampleTemp = NULL;
	BOOL			bOK;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return TRUE;
	}

	sound_StopAll();

	bOK = sound_Shutdown();

	/* empty sample heap */
	EnterCriticalSection( &critSecAudio );

	/* empty sample list */
	psSample = g_psSampleList;
	while ( psSample != NULL )
	{
		psSampleTemp = psSample->psNext;
		HEAP_FREE( g_psSampleHeap, psSample );
		psSample = psSampleTemp;
	}

	/* empty sample queue */
	psSample = g_psSampleQueue;
	while ( psSample != NULL )
	{
		psSampleTemp = psSample->psNext;
		HEAP_FREE( g_psSampleHeap, psSample );
		psSample = psSampleTemp;
	}

	LeaveCriticalSection( &critSecAudio );

	/* free sample heap */
	HEAP_DESTROY( g_psSampleHeap );
	g_psSampleHeap  = NULL;
	g_psSampleList  = NULL;
	g_psSampleQueue = NULL;

	DeleteCriticalSection( &critSecAudio );

	return bOK;
}

/***************************************************************************/

void
audio_PlayPreviousQueueTrack( void )
{
	if ( g_sPreviousSample.iTrack != NO_SAMPLE )
	{
		audio_PlayTrack( g_sPreviousSample.iTrack );
	}
}

/***************************************************************************/

BOOL
audio_GetPreviousQueueTrackPos( SDWORD *iX, SDWORD *iY, SDWORD *iZ )
{
	if ( g_sPreviousSample.x != SAMPLE_COORD_INVALID &&
		 g_sPreviousSample.y != SAMPLE_COORD_INVALID &&
		 g_sPreviousSample.z != SAMPLE_COORD_INVALID    )
	{
		*iX = g_sPreviousSample.x;
		*iY = g_sPreviousSample.y;
		*iZ = g_sPreviousSample.z;
		return TRUE;
	}
	else
	{
		*iX = *iY = *iZ;
		return FALSE;
	}
}

/***************************************************************************/

static void
audio_AddSampleToHead( AUDIO_SAMPLE **ppsSampleList, AUDIO_SAMPLE *psSample )
{
	EnterCriticalSection( &critSecAudio );
	psSample->psNext = (*ppsSampleList);
	psSample->psPrev = NULL;
	if ( (*ppsSampleList) != NULL )
	{
		(*ppsSampleList)->psPrev = psSample;
	}
	(*ppsSampleList) = psSample;
	LeaveCriticalSection( &critSecAudio );
}

/***************************************************************************/

static void
audio_AddSampleToTail( AUDIO_SAMPLE **ppsSampleList, AUDIO_SAMPLE *psSample )
{
	AUDIO_SAMPLE	*psSampleTail = NULL;

	EnterCriticalSection( &critSecAudio );

	if ( (*ppsSampleList) == NULL )
	{
		(*ppsSampleList) = psSample;
	}
	else
	{
		psSampleTail = (*ppsSampleList);
		while ( psSampleTail->psNext != NULL )
		{
			psSampleTail = psSampleTail->psNext;
		}
		psSampleTail->psNext = psSample;
		psSample->psPrev = psSampleTail;
		psSample->psNext = NULL;
	}

	LeaveCriticalSection( &critSecAudio );
}

/***************************************************************************/
/*
 * audio_RemoveSample
 *
 * Removes sample from list but doesn't free its memory
 */
/***************************************************************************/

static void
audio_RemoveSample( AUDIO_SAMPLE **ppsSampleList, AUDIO_SAMPLE *psSample )
{
	if ( psSample == NULL )
	{
		return;
	}

	EnterCriticalSection( &critSecAudio );
	if ( psSample == (*ppsSampleList) )
	{
		/* first sample in list */
		(*ppsSampleList) = psSample->psNext;
	}
	else
	{
		if ( psSample->psPrev != NULL )
		{
			psSample->psPrev->psNext = psSample->psNext;
		}
		if ( psSample->psNext != NULL )
		{
			psSample->psNext->psPrev = psSample->psPrev;
		}
	}

	/* set sample pointers NULL for safety */
	psSample->psPrev = NULL;
	psSample->psNext = NULL;

	LeaveCriticalSection( &critSecAudio );
}

/***************************************************************************/

static BOOL
audio_CheckSameQueueTracksPlaying( SDWORD iTrack )
{
	SDWORD			iCount;
	AUDIO_SAMPLE	*psSample = NULL;
	BOOL			bOK = TRUE;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE )
	{
		return TRUE;
	}

	iCount = 0;

	/* loop through queue sounds and check whether too many already in it */
	psSample = g_psSampleQueue;
	while ( psSample != NULL )
	{
		if ( psSample->iTrack == iTrack )
		{
			iCount++;
		}

		if ( iCount > MAX_SAME_SAMPLES )
		{
			bOK = FALSE;
			break;
		}

		psSample = psSample->psNext;
	}

	return bOK;
}

/***************************************************************************/

AUDIO_SAMPLE *
audio_QueueSample( SDWORD iTrack )
{
	AUDIO_SAMPLE	*psSample = NULL;
	SDWORD			iSameSamples = 0;

	printf("audio_queuesample called - track=%d\n",iTrack);

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE ||
		 g_bStopAll == TRUE )
	{
		return NULL;
	}

	ASSERT( ( sound_CheckTrack( iTrack ) == TRUE,
			"audio_QueueSample: track %i outside limits\n", iTrack ) );

	/* reject track if too many of same ID already in queue */
	if ( audio_CheckSameQueueTracksPlaying( iTrack ) == FALSE )
	{
		return NULL;
	}

	printf("audio_queuetrack called1\n");
	HEAP_ALLOC( g_psSampleHeap, &psSample );

	if ( psSample != NULL )
	{
		memset( psSample, 0, sizeof(AUDIO_SAMPLE) );
		psSample->iTrack  = iTrack;
		psSample->x       = SAMPLE_COORD_INVALID;
		psSample->y       = SAMPLE_COORD_INVALID;
		psSample->z       = SAMPLE_COORD_INVALID;
		psSample->bRemove = FALSE;

		/* add to queue */
		audio_AddSampleToTail( &g_psSampleQueue, psSample );
	}

	return psSample;
}

/***************************************************************************/

void
audio_QueueTrack( SDWORD iTrack )
{
	AUDIO_SAMPLE	*psSample = NULL;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE ||
		 g_bStopAll == TRUE )
	{
		return;
	}

	psSample = audio_QueueSample( iTrack );
}

/***************************************************************************/
/*
 * audio_QueueTrackMinDelay
 *
 * Will only play track if iMinDelay has elapsed since track last finished
 */
/***************************************************************************/

void
audio_QueueTrackMinDelay( SDWORD iTrack, UDWORD iMinDelay )
{
	AUDIO_SAMPLE	*psSample = NULL;
	UDWORD			iDelay;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE )
	{
		return;
	}

	iDelay = sound_GetGameTime() - sound_GetTrackTimeLastFinished( iTrack );

	if ( iDelay > iMinDelay )
	{
		psSample = audio_QueueSample( iTrack );

		if ( psSample != NULL )
		{
			sound_SetTrackTimeLastFinished( iTrack, sound_GetGameTime() );
		}
	}
}

/***************************************************************************/

void
audio_QueueTrackMinDelayPos( SDWORD iTrack, UDWORD iMinDelay,
								SDWORD iX, SDWORD iY, SDWORD iZ )
{
	AUDIO_SAMPLE	*psSample = NULL;
	UDWORD			iDelay;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE )
	{
		return;
	}

	iDelay = sound_GetGameTime() - sound_GetTrackTimeLastFinished( iTrack );

	if ( iDelay > iMinDelay )
	{
		psSample = audio_QueueSample( iTrack );

		if ( psSample != NULL )
		{
			sound_SetTrackTimeLastFinished( iTrack, sound_GetGameTime() );
			psSample->x = iX;
			psSample->y = iY;
			psSample->z = iZ;
		}
	}
}

/***************************************************************************/

void
audio_QueueTrackPos( SDWORD iTrack, SDWORD iX, SDWORD iY, SDWORD iZ )
{
	AUDIO_SAMPLE	*psSample = NULL;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE )
	{
		return;
	}

	psSample = audio_QueueSample( iTrack );
	if ( psSample != NULL )
	{
		psSample->x = iX;
		psSample->y = iY;
		psSample->z = iZ;
	}
}

/***************************************************************************/

void
audio_UpdateQueue( void )
{
	AUDIO_SAMPLE	*psSample = NULL;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE )
	{
		return;
	}

	if ( sound_QueueSamplePlaying() == TRUE )
	{
		/* lower volume whilst playing queue audio */
		audio_Set3DVolume( LOWERED_VOL );
	}
	else
	{
		/* set full global volume */
		audio_Set3DVolume( AUDIO_VOL_MAX );

		/* check queue for members */
		if ( g_psSampleQueue != NULL )
		{
			/* remove queue head */
			psSample = g_psSampleQueue;
			audio_RemoveSample( &g_psSampleQueue, psSample );

			/* add sample to list if able to play */
			if ( sound_Play2DTrack( psSample, TRUE ) == TRUE )
			{
				audio_AddSampleToHead( &g_psSampleList, psSample );

				/* update last queue sound coords */
				if ( psSample->x != SAMPLE_COORD_INVALID &&
					 psSample->y != SAMPLE_COORD_INVALID &&
					 psSample->z != SAMPLE_COORD_INVALID    )
				{
					g_sPreviousSample.x = psSample->x;
					g_sPreviousSample.y = psSample->y;
					g_sPreviousSample.z = psSample->z;
				}
			}
			else
			{
				DBPRINTF( ("audio_UpdateQueue: couldn't play sample\n") );
				HEAP_FREE( g_psSampleHeap, psSample );
			}
		}
	}
}

/***************************************************************************/

BOOL
audio_Update( void )
{
	SDWVEC3D		vecPlayer;
	SDWORD			iA;
	AUDIO_SAMPLE	*psSample, *psSampleTemp;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return TRUE;
	}

	audio_UpdateQueue();

	/* get player position */
	if ( audio_Display3D() == TRUE )
	{
		audio_Get3DPlayerPos( &vecPlayer.x, &vecPlayer.y, &vecPlayer.z );
	}
	else
	{
		audio_Get2DPlayerPos( &vecPlayer.x, &vecPlayer.y, &vecPlayer.z );
	}

	sound_SetPlayerPos( vecPlayer.x, vecPlayer.y, vecPlayer.z );
	
	audio_Get3DPlayerRotAboutVerticalAxis( &iA );
	sound_SetPlayerOrientation( 0, 0, iA );

	/* loop through 3D sounds and remove if finished or update position */
	psSample = g_psSampleList;
	while ( psSample != NULL )
	{
		/* remove finished samples from list */
		if ( psSample->bRemove == TRUE )
		{
			audio_RemoveSample( &g_psSampleList, psSample );
			psSampleTemp = psSample->psNext;
			HEAP_FREE( g_psSampleHeap, psSample );
			psSample = psSampleTemp;
		}
		/* check looping sound callbacks for finished condition */
		else
		{
			if ( psSample->psObj != NULL )
			{
				if ( audio_ObjectDead( psSample->psObj ) ||
					 ( psSample->pCallback != NULL &&
				      (psSample->pCallback)(psSample) == FALSE) )
				{
					sound_StopTrack( psSample );
					psSample->psObj = NULL;
				}
				else
				{
					/* update sample position */
					{
						audio_GetObjectPos( psSample->psObj,
								&psSample->x, &psSample->y, &psSample->z );
						sound_SetObjectPosition( psSample->iSample,
								psSample->x, psSample->y, psSample->z );
					}
				}
			}

			/* next sample */
			psSample = psSample->psNext;
		}
	}

	sound_Update();

	return TRUE;
}

/***************************************************************************/

BOOL
audio_LoadTrackFromFile( char szFileName[] )
{
	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return TRUE;
	}

	return sound_LoadTrackFromFile( szFileName );
}

/***************************************************************************/

void *
audio_LoadTrackFromBuffer( UBYTE *pBuffer, UDWORD udwSize )
{
	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return NULL;
	}

	return sound_LoadTrackFromBuffer( pBuffer, udwSize );
}

/***************************************************************************/

// Routine to convert wav filename into a track number
//
//  ... This is really not going to be practical on the PSX is it?
//
//    What is the point of all the scripts storing .WAV file names like: "Incoming Intelligence Report-22hz Mon.wav"
//	when all that is required is a single byte saying which sound effect is required.
//
//  A typical example of using 50 times the amount of memory that is needed.
//   ... bloody PC programmers they're spoiled !
//
//  
//
BOOL
audio_SetTrackVals( char szFileName[], BOOL bLoop, int *piID, int iVol,
						int iPriority, int iAudibleRadius, int VagID )
{
	TRACK	*psTrack;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return TRUE;
	}

	/* get track pointer from resource */
	psTrack = resGetData( "WAV", szFileName );

	if ( psTrack == NULL )
	{
		DBPRINTF( ("audio_SetTrackVals: track %s resource not found\n", szFileName) );
		return FALSE;
	}
	else
	{
		/* get current ID or spare one */
		if ( audio_GetIDFromStr( szFileName, piID ) == FALSE )
		{
			*piID = sound_GetAvailableID();
		}

		if ( *piID == SAMPLE_NOT_ALLOCATED )
		{
			DBPRINTF( ("audio_SetTrackVals: couldn't get spare track ID\n") );
			return FALSE;
		}
		else
		{
			return sound_SetTrackVals( psTrack, bLoop, *piID, iVol,
									iPriority, iAudibleRadius, VagID );
		}
	}
}

/***************************************************************************/

BOOL
audio_SetTrackValsHashName( UDWORD hash, BOOL bLoop, int iTrack, int iVol,
							int iPriority, int iAudibleRadius, int VagID )
{
	TRACK	*psTrack;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return TRUE;
	}

	/* get track pointer from resource */
	psTrack = resGetDataFromHash( "WAV", hash );

	if ( psTrack == NULL )
	{
		return FALSE;
	}
	else
	{
		return sound_SetTrackVals( psTrack, bLoop, iTrack, iVol,
								iPriority, iAudibleRadius, VagID );
	}
}

/***************************************************************************/

void
audio_ReleaseTrack( TRACK *psTrack )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	sound_ReleaseTrack ( psTrack );
}

/***************************************************************************/
/*
 * audio_CheckSame3DTracksPlaying
 *
 * Reject samples if too many already playing in same area
 */
/***************************************************************************/

static BOOL
audio_CheckSame3DTracksPlaying( SDWORD iTrack, SDWORD iX, SDWORD iY, SDWORD iZ )
{
	SDWORD			iCount, iDx, iDy, iDz, iDistSq, iMaxDistSq, iRad;
	AUDIO_SAMPLE	*psSample = NULL;
	BOOL			bOK = TRUE;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE )
	{
		return TRUE;
	}

	iCount = 0;

	/* loop through 3D sounds and check whether too many already in earshot */
	psSample = g_psSampleList;
	while ( psSample != NULL )
	{
		if ( psSample->iTrack == iTrack )
		{
			iDx = iX - psSample->x;
			iDy = iY - psSample->y;
			iDz = iZ - psSample->z;
			iDistSq = (iDx*iDx) + (iDy*iDy) + (iDz*iDz);
			iRad = sound_GetTrackAudibleRadius( iTrack );
			iMaxDistSq = iRad*iRad;

			if ( iDistSq < iMaxDistSq )
			{
				iCount++;
			}

			if ( iCount > MAX_SAME_SAMPLES )
			{
				bOK = FALSE;
				break;
			}
		}

		psSample = psSample->psNext;
	}

	return bOK;
}

/***************************************************************************/

static BOOL
audio_Play3DTrack( SDWORD iX, SDWORD iY, SDWORD iZ, int iTrack,
						void *psObj, AUDIO_CALLBACK pUserCallback )
{
	AUDIO_SAMPLE	*psSample;
	
	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE ||
		 g_bStopAll == TRUE)
	{
		return FALSE;
	}

	if ( audio_CheckSame3DTracksPlaying( iTrack, iX, iY, iZ ) == FALSE )
	{
		return FALSE;
	}

	HEAP_ALLOC( g_psSampleHeap, &psSample );
	if ( psSample == NULL )
	{
		return FALSE;
	}
	else
	{
		/* setup sample */
		memset( psSample, 0, sizeof(AUDIO_SAMPLE) );
		psSample->iTrack    = iTrack;
		psSample->x         = iX;
		psSample->y         = iY;
		psSample->z         = iZ;
		psSample->bRemove   = FALSE;
		psSample->psObj     = psObj;
		psSample->pCallback = pUserCallback;

		/* add sample to list if able to play */
		if ( sound_Play3DTrack( psSample ) == TRUE )
		{
			audio_AddSampleToHead( &g_psSampleList, psSample );
			return TRUE;
		}
		else
		{
			DBPRINTF( ("audio_Play3DTrack: couldn't play sample\n") );
			HEAP_FREE( g_psSampleHeap, psSample );
			return FALSE;
		}
	}
}

/***************************************************************************/

BOOL
audio_PlayStaticTrack( SDWORD iMapX, SDWORD iMapY, int iTrack )
{
	SDWORD			iX, iY, iZ;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return FALSE;
	}

	audio_GetStaticPos( iMapX, iMapY, &iX, &iY, &iZ );
	return audio_Play3DTrack( iX, iY, iZ, iTrack, NULL, NULL );
}

/***************************************************************************/

BOOL
audio_PlayObjStaticTrack( void * psObj, int iTrack )
{
	SDWORD			iX, iY, iZ;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return FALSE;
	}

	audio_GetObjectPos( psObj, &iX, &iY, &iZ );
	return audio_Play3DTrack( iX, iY, iZ, iTrack, psObj, NULL );
}

/***************************************************************************/

BOOL
audio_PlayObjStaticTrackCallback( void * psObj, int iTrack,
									AUDIO_CALLBACK pUserCallback )
{
	SDWORD			iX, iY, iZ;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return FALSE;
	}

	audio_GetObjectPos( psObj, &iX, &iY, &iZ );
	return audio_Play3DTrack( iX, iY, iZ, iTrack, psObj, pUserCallback );
}

/***************************************************************************/

BOOL
audio_PlayObjDynamicTrack( void * psObj, int iTrack,
							AUDIO_CALLBACK pUserCallback )
{
	SDWORD			iX, iY, iZ;
	
	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return FALSE;
	}

	audio_GetObjectPos( psObj, &iX, &iY, &iZ );
	return audio_Play3DTrack( iX, iY, iZ, iTrack, psObj, pUserCallback );
}

/***************************************************************************/

BOOL
audio_PlayStream( char szFileName[], SDWORD iVol,
					AUDIO_CALLBACK pUserCallback )
{
	AUDIO_SAMPLE	*psSample;

	/* if audio not enabled return TRUE to carry on game without audio */
	if ( g_bAudioEnabled == FALSE )
	{
		return FALSE;
	}

	HEAP_ALLOC( g_psSampleHeap, &psSample );

	if ( psSample != NULL )
	{
		memset( psSample, 0, sizeof(AUDIO_SAMPLE) );
		psSample->pCallback = pUserCallback;
		psSample->bRemove   = FALSE;

		audio_Set3DVolume( AUDIO_VOL_MAX );
		if ( sound_PlayStream( psSample, szFileName, iVol ) == TRUE )
		{
			return TRUE;
		}
	}

	return FALSE;

}

/***************************************************************************/

void
audio_StopObjTrack( void * psObj, int iTrack )
{
	AUDIO_SAMPLE	*psSample;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bStopAll == TRUE )
	{
		return;
	}

	/* find sample */
	psSample = g_psSampleList;
	while ( psSample != NULL )
	{
		if ( psSample->psObj == psObj && psSample->iTrack == iTrack )
		{
			break;
		}

		/* get next sample from hash table */
		psSample = psSample->psNext;
	}

	if ( psSample != NULL )
	{
		sound_StopTrack( psSample );
	}
}

/***************************************************************************/
/*
 * audio_PlayTrack
 *
 * Play immediate 2D FX track
 */
/***************************************************************************/

void audio_PlayTrack( int iTrack )
{
	AUDIO_SAMPLE	*psSample;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE || g_bAudioPaused == TRUE ||
		 g_bStopAll == TRUE )
	{
		return;
	}

	HEAP_ALLOC( g_psSampleHeap, &psSample );
	if ( psSample != NULL )
	{
		/* setup sample */
		memset( psSample, 0, sizeof(AUDIO_SAMPLE) );
		psSample->iTrack  = iTrack;
		psSample->bRemove = FALSE;

		/* add sample to list if able to play */
		if ( sound_Play2DTrack( psSample, FALSE ) == TRUE )
		{
			audio_AddSampleToHead( &g_psSampleList, psSample );
		}
		else
		{
			DBPRINTF( ("audio_PlayTrack: couldn't play sample\n") );
			HEAP_FREE( g_psSampleHeap, psSample );
		}
	}
}

/***************************************************************************/

void
audio_StopTrack( int iTrack )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	iTrack;
}

/***************************************************************************/

void
audio_SetTrackPan( int iTrack, int iPan )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	iTrack;
	iPan;
}

/***************************************************************************/

void
audio_SetTrackVol( int iTrack, int iVol )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	iTrack;
	iVol;
}

/***************************************************************************/

void
audio_SetTrackFreq( int iTrack, int iFreq )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	iTrack;
	iFreq;
}

/***************************************************************************/

void
audio_PauseAll( void )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	g_bAudioPaused = TRUE;

	sound_PauseAll();
}

/***************************************************************************/

void
audio_ResumeAll( void )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	g_bAudioPaused = FALSE;

	sound_ResumeAll();
}

/***************************************************************************/

void
audio_StopAll( void )
{
	AUDIO_SAMPLE	*psSample, *psSampleTemp;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return;
	}

	DBPRINTF( ("audio_StopAll called\n") );

	g_bStopAll = TRUE;

	/* empty list - audio_Update will free samples
	 * because callbacks have to come in first
	 */
	psSample = g_psSampleList;
	while ( psSample != NULL )
	{
		sound_StopTrack( psSample );
		psSample = psSample->psNext;
	}

	/* empty sample queue */
	psSample = g_psSampleQueue;
	while ( psSample != NULL )
	{
		psSampleTemp = psSample->psNext;
		HEAP_FREE( g_psSampleHeap, psSample );
		psSample = psSampleTemp;
	}
	g_psSampleQueue = NULL;

	g_bStopAll = FALSE;

	DBPRINTF( ("audio_StopAll done\n") );
}

/***************************************************************************/

void
audio_CheckAllUnloaded()
{
	sound_CheckAllUnloaded();
}

/***************************************************************************/

LPDIRECTSOUND
audio_GetDirectSoundObj( void )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return NULL;
	}

	return sound_GetDirectSoundObj();
}

/***************************************************************************/

SDWORD
audio_GetTrackID( char szFileName[] )
{
	TRACK	*psTrack;
	SDWORD	iID;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return SAMPLE_NOT_FOUND;
	}
	else
	{
		psTrack = resGetData( "WAV", szFileName );

		if ( psTrack == NULL )
		{
			return SAMPLE_NOT_FOUND;
		}
		else
		{
			iID = sound_GetTrackID( psTrack );
			return iID;
		}
	}
}

/***************************************************************************/


SDWORD
audio_GetTrackIDFromHash( UDWORD hash )
{
	TRACK	*psTrack;
	SDWORD	iID;

	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return SAMPLE_NOT_FOUND;
	}
	else
	{
		psTrack = resGetDataFromHash( "WAV", hash );

		if ( psTrack == NULL )
		{
			return SAMPLE_NOT_FOUND;
		}
		else
		{
			iID = sound_GetTrackID( psTrack );
			return iID;
		}
	}
}

/***************************************************************************/

SDWORD
audio_GetAvailableID( void )
{
	/* return if audio not enabled */
	if ( g_bAudioEnabled == FALSE )
	{
		return 0;
	}
	else
	{
		return sound_GetAvailableID();
	}
}

/***************************************************************************/

SDWORD
audio_Get3DVolume( void )
{
	return g_i3DVolume;
}

/***************************************************************************/

void
audio_Set3DVolume( SDWORD iVol )
{
	g_i3DVolume = iVol;
}

/***************************************************************************/
/*
 * audio_GetMixVol
 *
 * iVol and audio_Get3DVolume need to be scaled by AUDIO_VOL_RANGE
 */
/***************************************************************************/

SDWORD
audio_GetMixVol( SDWORD iVol )
{
	SDWORD	iMixVol = (iVol*sound_GetMaxVolume()*audio_Get3DVolume())/
						(AUDIO_VOL_RANGE*AUDIO_VOL_RANGE);
	return iMixVol;
}

/***************************************************************************/
/*
 * audio_GetSampleMixVol
 *
 * iVol, audio_Get3DVolume and sound_GetTrackVolume all need to be scaled
 * by AUDIO_VOL_RANGE
 */
/***************************************************************************/

SDWORD
audio_GetSampleMixVol( AUDIO_SAMPLE * psSample, SDWORD iVol, BOOL bScale3D )
{
	SDWORD	iMixVol;
	
	iMixVol = iVol*sound_GetMaxVolume()/AUDIO_VOL_RANGE;
	iMixVol = iMixVol*sound_GetTrackVolume(psSample->iTrack)/AUDIO_VOL_RANGE;
	if ( bScale3D )
	{
		iMixVol = iMixVol*audio_Get3DVolume()/AUDIO_VOL_RANGE;
	}

	return iMixVol;
}

/***************************************************************************/
