#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "ivisdef.h"
#include "pieState.h"
//#include "ivi.h"
//#include "v3d.h"
#include "rendmode.h"
#ifdef WIN32
#include "pieMode.h"
#endif
//#include "geo.h"
#include "bug.h"
//#include "pio.h"
#include "piePalette.h"
#include "pieMatrix.h"
//#include "kyb.h"
#include "tex.h"
//#include "pdv.h"
#include "ivispatch.h"
#ifdef INC_GLIDE
	#include "3dfxmode.h"
	#include "3dfxText.h"
#endif

//*************************************************************************

//*************************************************************************

iError	_iVERROR;

//*************************************************************************
// pass in true to reset the palette too.
void iV_Reset(int bPalReset)
{
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		reset3dfx();
	}
	_TEX_INDEX = 0;
	iV_ClearFonts();		// Initialise the IVIS font module.
}


void iV_ShutDown(void)

{

	iV_DEBUG0("1\n");

#ifdef iV_DEBUG
	iV_HeapLog();
#endif

	iV_DEBUG0("4\n");
#ifdef WIN32
	pie_ShutDown();
#endif
	iV_DEBUG0("5\n");

	iV_VideoMemoryUnlock();

	iV_DEBUG0("6\n");

	pie_TexShutDown();

	iV_DEBUG0("7\n");

//	_iv_heap_tidy();

	iV_DEBUG0("8\n");

	iV_DEBUG0("9\n");

	iV_DEBUG0("iVi[ShutDown] = successful\n");
}

//*************************************************************************

void iV_Stop(char *string, ...)

{
	va_list argptr;

	iV_ShutDown();
	va_start(argptr,string);
	vprintf(string,argptr);
	va_end(argptr);
	exit(0);
}

//*************************************************************************

void iV_Abort(char *string, ...)

{
	va_list argptr;

	va_start(argptr,string);
	vprintf(string,argptr);
	va_end(argptr);
}

//*************************************************************************

#ifndef FINALBUILD
void iV_Error(long errorn, char *msge, ...)
{
	va_list argptr;

	va_start(argptr,msge);
	vsprintf(&_iVERROR.msge[0],msge,argptr);
	va_end(argptr);
	_iVERROR.n = errorn;
}
#endif