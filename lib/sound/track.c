/***************************************************************************/

#pragma warning (disable : 4201 4214 4115 4514)
#include <windows.h>
#pragma warning (default : 4201 4214 4115)

#include "Frame.h"

#include "tracklib.h"
#include "priority.h"

/***************************************************************************/
/* defines */

#define	MAX_TRACKS			(600)

/***************************************************************************/
/* static global variables */

/* array of pointers to sound effects */
static TRACK **	g_apTrack;

/* number of tracks loaded */
static SDWORD	g_iCurTracks = 0;

static SDWORD	g_iSamples = 0;
static SDWORD	g_iMaxSamples;
static SDWORD	g_iMaxSameSamples;

/* flag set when system is active (for callbacks etc) */
static BOOL		g_bSystemActive = FALSE;

static BOOL		g_bDevVolume = FALSE;

static AUDIO_CALLBACK g_pStopTrackCallback = NULL;

/***************************************************************************/

BOOL
sound_CheckDevice( void )
{
	WAVEOUTCAPS	waveCaps;
	MMRESULT	mmRes;

	/* check wave out device(s) present */
	if ( waveOutGetNumDevs() == 0 )
	{
		DBPRINTF( ("sound_CheckDevice: error in waveOutGetNumDevs\n") );
		return FALSE;

	}

	/* default to using first device: check volume control caps */
	mmRes = waveOutGetDevCaps( 0, (LPWAVEOUTCAPS) &waveCaps,
								sizeof(WAVEOUTCAPS) );
	if ( mmRes != MMSYSERR_NOERROR )
	{
		DBPRINTF( ("sound_CheckDevice: error in waveOutGetDevCaps\n") );
		return FALSE;
	}

	/* verify device supports volume changes */
	if ( waveCaps.dwSupport & WAVECAPS_VOLUME )
	{
		return TRUE;
	}
	else
	{
		DBPRINTF( ("sound_CheckDevice: wave out device doesn't support volume changes\n") );
		return FALSE;
	}
}

/***************************************************************************/

BOOL
sound_Init( HWND hWnd, SDWORD iMaxSameSamples )
{
#if USE_COMPRESSED_SPEECH
	LPVOID	lpMsgBuf;
#endif

	SDWORD	i;

	hWnd;
	g_iMaxSameSamples = iMaxSameSamples;

	g_iCurTracks = 0;

	g_bDevVolume = sound_CheckDevice();

#if USE_COMPRESSED_SPEECH
	if ( !LoadLibrary( "MSACM32.DLL" ) )
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf, 0, NULL );
		DBPRINTF( ("sound_Init: couldn't load compression manager MSACM32.DLL\n") );
	}

	if ( !LoadLibrary( "MSADP32.ACM" ) )
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf, 0, NULL );
		DBPRINTF( ("sound_Init: couldn't load ADPCM codec MSADP32.ACM\n") );
	}
#endif

	if ( sound_InitLibrary() == FALSE )
	{
		DBPRINTF( ("Cannot init sound library\n") );
		return FALSE;
	}

	/* init audio array */
	g_apTrack = (TRACK **) MALLOC( sizeof(TRACK *) * MAX_TRACKS );
	for ( i=0; i<MAX_TRACKS; i++ )
	{
		g_apTrack[i] = NULL;
	}

	/* set system active flag for callbacks */
	g_bSystemActive = TRUE;

	return TRUE;
}

/***************************************************************************/

BOOL
sound_Shutdown()
{
	FREE( g_apTrack );

	/* set inactive flag to prevent callbacks coming after shutdown */
	g_bSystemActive = FALSE;

	sound_ShutdownLibrary();

	return TRUE;
}

/***************************************************************************/

BOOL
sound_GetSystemActive( void )
{
	return g_bSystemActive;
}

/***************************************************************************/
/*
 * Vag ID is just used on PSX
 * szFileName just used on PC
 */
/***************************************************************************/

BOOL
sound_SetTrackVals( TRACK *psTrack, BOOL bLoop, SDWORD iTrack, SDWORD iVol,
	SDWORD iPriority, SDWORD iAudibleRadius, SDWORD VagID )
{
	ASSERT( (iPriority>=LOW_PRIORITY && iPriority<=HIGH_PRIORITY,
			"sound_CreateTrack: priority %i out of bounds\n", iPriority) );
	
	/* add to sound array */
	if ( iTrack < MAX_TRACKS )
	{
		if ( g_apTrack[iTrack] != NULL )
		{
			DBERROR( ("sound_SetTrackVals: track %i already set\n", iTrack ) );
			return FALSE;
		}

		/* set track members */
		psTrack->bLoop             = bLoop;
		psTrack->iVol              = iVol;
		psTrack->iPriority         = iPriority;
		psTrack->iAudibleRadius    = iAudibleRadius;
		psTrack->iTimeLastFinished = 0;
		psTrack->iNumPlaying       = 0;

#ifdef WIN32
		VagID;
#else
		psTrack->VAGid=VagID;	// set the vag id for the playstation
#endif

		/* set global */
		g_apTrack[iTrack] = psTrack;

		/* increment current sound */
		g_iCurTracks++;

		return TRUE;
	}

	return FALSE;
}

/***************************************************************************/

BOOL
sound_AddTrack( TRACK *pTrack )
{
	/* add to sound array */
	if ( g_iCurTracks < MAX_TRACKS )
	{
		/* set pointer in table */
		g_apTrack[g_iCurTracks] = pTrack;

		/* increment current sound */
		g_iCurTracks++;

		return TRUE;
	}
	else
	{
		DBERROR( ("sound_AddTrack: all tracks used: increase MAX_TRACKS\n") );
		return FALSE;
	}
}

/***************************************************************************/

void *
sound_LoadTrackFromBuffer( UBYTE *pBuffer, UDWORD udwSize )
{
#ifdef WIN32
	TRACK *	pTrack;

	/* allocate track */
	pTrack = (TRACK *) MALLOC( sizeof(TRACK) );

	if ( pTrack == NULL )
	{
		DBERROR( ("sound_LoadTrackFromBuffer: couldn't allocate memory\n") );
		return NULL;	}
	else
	{
		pTrack->bMemBuffer = TRUE;
		pTrack->pName = MALLOC(strlen(GetLastResourceFilename()) + 1);
		if (pTrack->pName == NULL)
		{
			DBERROR(("sound_LoadTrackFromBuffer: couldn't allocate memory\n") );
			FREE(pTrack);
			return NULL;
		}
		strcpy(pTrack->pName, GetLastResourceFilename());
		pTrack->resID = GetLastHashName();

		if ( sound_ReadTrackFromBuffer( pTrack, pBuffer, udwSize ) == FALSE )
		{
			return NULL;
		}
		else
		{

#if !USE_COMPRESSED_SPEECH
			/* flag compressed audio load */
			if ( pTrack->bCompressed == TRUE )
			{
				DBPRINTF( ("sound_LoadTrackFromBuffer: %s is compressed!\n",
							pTrack->pName ) );
			}
#endif
			return pTrack;
		}
	}
#else
	printf("sound_LoadTrackFromBuffer() called\n");
#endif
}

/***************************************************************************/

BOOL
sound_LoadTrackFromFile( char szFileName[] )
{
#ifdef WIN32
	TRACK *	pTrack;

	/* allocate track */
	pTrack = (TRACK *) MALLOC( sizeof(TRACK) );

	if ( pTrack != NULL )
	{
		pTrack->bMemBuffer = FALSE;
		pTrack->pName = MALLOC(strlen(szFileName)+1);
		if (pTrack->pName == NULL)
		{
			DBERROR(("sound_LoadTrackFromFile: Out of memory"));
			return FALSE;
		}
		strcpy(pTrack->pName, szFileName);
		pTrack->resID = HashStringIgnoreCase(szFileName);

		if ( sound_ReadTrackFromFile( pTrack, szFileName ) == FALSE )
		{
			return FALSE;
		}

		return sound_AddTrack( pTrack );
	}

	return FALSE;
#else
	printf("sound_LoadTrackFromFile() called\n");
#endif
}

/***************************************************************************/

BOOL
sound_ReleaseTrack( TRACK * psTrack )
{
	SDWORD	iTrack;

	if (psTrack->pName != NULL)
	{
		FREE(psTrack->pName);
	}

	for ( iTrack=0; iTrack<g_iCurTracks; iTrack++ )
	{
		if ( g_apTrack[iTrack] == psTrack )
		{
			g_apTrack[iTrack] = NULL;
		}
	}

	sound_FreeTrack( psTrack );

	return TRUE;
}

/***************************************************************************/

void
sound_CheckAllUnloaded( void )
{
	SDWORD	iTrack;

	for ( iTrack=0; iTrack<MAX_TRACKS; iTrack++ )
	{
		ASSERT( (g_apTrack[iTrack] == NULL,
			"sound_CheckAllUnloaded: check audio.cfg for duplicate IDs\n") );
	}
}

/***************************************************************************/

BOOL
sound_TrackLooped( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->bLoop;
}

/***************************************************************************/

SDWORD
sound_TrackAudibleRadius( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iAudibleRadius;
}

/***************************************************************************/

SDWORD
sound_GetNumPlaying( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iNumPlaying;
}

/***************************************************************************/

void
sound_CheckSample( AUDIO_SAMPLE *psSample )
{
	ASSERT( (PTRVALID(psSample,sizeof(AUDIO_SAMPLE)),
			"sound_CheckSample: sample pointer invalid\n") );

	ASSERT( ( psSample->iSample >=0 ||
			  psSample->iSample == SAMPLE_NOT_ALLOCATED,
			  "sound_CheckSample: sample %i out of range\n",
			  psSample->iSample ) );

	psSample;
}

/***************************************************************************/

BOOL
sound_CheckTrack( SDWORD iTrack )
{
	if ( iTrack<0 || iTrack>g_iCurTracks-1 )
	{
		DBPRINTF( ("sound_CheckTrack: track number %i outside max %i\n",
						iTrack, g_iCurTracks) );
		return FALSE;
	}

	if ( g_apTrack[iTrack] == NULL )
	{
		DBPRINTF( ("sound_CheckTrack: track %i NULL\n", iTrack) );
		return FALSE;
	}

	return TRUE;
}

/***************************************************************************/

SDWORD
sound_GetTrackTime( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iTime;
}

/***************************************************************************/

SDWORD
sound_GetTrackPriority( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iPriority;
}

/***************************************************************************/

SDWORD
sound_GetTrackVolume( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iVol;
}

/***************************************************************************/

SDWORD
sound_GetTrackAudibleRadius( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iAudibleRadius;
}

/***************************************************************************/

char *
sound_GetTrackName( SDWORD iTrack )
{
	ASSERT((g_apTrack[iTrack] != NULL,
		"sound_GetTrackName: unallocated track"));
#ifdef WIN32
	return g_apTrack[iTrack]->pName;
#else
	DBERROR(("sound_GetTrackName: not valid on PSX"));
	return NULL;
#endif
}

/***************************************************************************/

UDWORD
sound_GetTrackHashName( SDWORD iTrack )
{
	ASSERT((g_apTrack[iTrack] != NULL,
		"sound_GetTrackName: unallocated track"));
#ifdef WIN32
	return g_apTrack[iTrack]->resID;
#else
	DBERROR(("sound_GetTrackHashName: not valid on PSX"));
	return 0;
#endif
}

/***************************************************************************/

BOOL
sound_Play2DTrack( AUDIO_SAMPLE *psSample, BOOL bQueued )
{
	TRACK	*psTrack;

	psTrack = g_apTrack[psSample->iTrack];

	/* check only playing compressed audio on queue channel */
#if USE_COMPRESSED_SPEECH
	if ( bQueued && !psTrack->bCompressed )
	{
		DBPRINTF( ("sound_PlayTrack: trying to play uncompressed speech %s!\n",
					psTrack->pName) );
		return FALSE;
	}
	if ( !bQueued && psTrack->bCompressed )
	{
		DBPRINTF( ("sound_PlayTrack: trying to play compressed audio %s!\n",
					psTrack->pName) );
		return FALSE;
	}
#else
	if ( psTrack->bCompressed )
	{
		DBPRINTF( ("sound_PlayTrack: trying to play compressed speech %s!\n",
					psTrack->pName) );
		return FALSE;
	}
#endif

	return sound_Play2DSample( psTrack, psSample, bQueued );
}

/***************************************************************************/

BOOL
sound_Play3DTrack( AUDIO_SAMPLE *psSample )
{
	TRACK	*psTrack;

	psTrack = g_apTrack[psSample->iTrack];

	if ( psTrack->bCompressed )
	{
		DBPRINTF( ("sound_PlayTrack: trying to play compressed audio %s!\n",
					psTrack->pName) );
		return FALSE;
	}

	return sound_Play3DSample( psTrack, psSample );
}

/***************************************************************************/

void
sound_StopTrack( AUDIO_SAMPLE *psSample )
{
	sound_CheckSample( psSample );

	if ( psSample->iSample != SAMPLE_NOT_ALLOCATED )
	{
		sound_StopSample( psSample->iSample );
	}

	/* do stopped callback */
	if ( g_pStopTrackCallback != NULL && psSample->psObj != NULL )
	{
		(g_pStopTrackCallback)(psSample);
	}

	/* update number of samples playing */
	g_iSamples--;
}

/***************************************************************************/

void
sound_PauseTrack( AUDIO_SAMPLE *psSample )
{
	if ( psSample->iSample != SAMPLE_NOT_ALLOCATED )
	{
		sound_StopSample( psSample->iSample );
	}
}

/***************************************************************************/

void
sound_FinishedCallback( AUDIO_SAMPLE *psSample )
{
	sound_CheckSample( psSample );

	if ( g_apTrack[psSample->iTrack] != NULL )
	{
		g_apTrack[psSample->iTrack]->iTimeLastFinished = sound_GetGameTime();
	}

	/* call user callback if specified */
	if ( psSample->pCallback != NULL )
	{
		(psSample->pCallback) (psSample);
	}

	/* set remove flag */
	psSample->bRemove = TRUE;
}

/***************************************************************************/

SDWORD
sound_GetTrackID( TRACK *psTrack )
{
	SDWORD	i = 0;

	/* find matching track */
	for ( i=0; i<MAX_TRACKS; i++ )
	{
		if ( (g_apTrack[i] != NULL) && (g_apTrack[i] == psTrack) )
		{
			break;
		}
	}

	/* if matching track found return it else find empty track */
	if ( i<MAX_TRACKS )
	{
		return i;
	}
	else
	{
		return SAMPLE_NOT_FOUND;
	}
}

/***************************************************************************/

SDWORD
sound_GetAvailableID( void )
{
	SDWORD	i;

	for ( i=0; i<MAX_TRACKS; i++ )
	{
		if ( g_apTrack[i] == NULL )
		{
			break;
		}
	}

	ASSERT( (i<MAX_TRACKS, "sound_GetTrackID: unused track not found!\n") );

	if ( i<MAX_TRACKS )
	{
		return i;
	}
	else
	{
		return SAMPLE_NOT_ALLOCATED;
	}
}

/***************************************************************************/

SDWORD
sound_GetGlobalVolume( void )
{
	MMRESULT	mmRes;
	SDWORD		iVol;
	SDWORD		iGlobVol = AUDIO_VOL_MAX;

	if ( g_bDevVolume == TRUE )
	{
		mmRes = waveOutGetVolume( 0, (LPDWORD) &iVol );
		if ( mmRes == MMSYSERR_NOERROR )
		{
			iGlobVol = ((SDWORD) LOWORD( iVol )) * AUDIO_VOL_MAX / 0xFFFF;
		}
		else
		{
			DBPRINTF( ("sound_GetGlobalVolume: waveOutGetVolume failed\n") );
		}
	}

	return iGlobVol;
}

/***************************************************************************/

void
sound_SetGlobalVolume( SDWORD iVol )
{
	MMRESULT	mmRes;
	SDWORD		iNewVol, iWinVol;

	if ( g_bDevVolume == TRUE )
	{
		iWinVol = iVol * 0xFFFF / AUDIO_VOL_MAX;
		iNewVol = (iWinVol << 16) | iWinVol;

		mmRes = waveOutSetVolume( 0, iNewVol );
		if ( mmRes != MMSYSERR_NOERROR )
		{
			DBPRINTF( ("sound_GetGlobalVolume: waveOutSetVolume failed\n") );
		}
	}
}

/***************************************************************************/

void
sound_SetStoppedCallback( AUDIO_CALLBACK pStopTrackCallback )
{
	g_pStopTrackCallback = pStopTrackCallback;
}

/***************************************************************************/

UDWORD
sound_GetTrackTimeLastFinished( SDWORD iTrack )
{
	sound_CheckTrack( iTrack );

	return g_apTrack[iTrack]->iTimeLastFinished;
}

/***************************************************************************/

void
sound_SetTrackTimeLastFinished( SDWORD iTrack, UDWORD iTime )
{
	sound_CheckTrack( iTrack );

	g_apTrack[iTrack]->iTimeLastFinished = iTime;
}

/***************************************************************************/
