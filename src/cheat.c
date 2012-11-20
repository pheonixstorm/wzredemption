/* Handles cheat codes for Warzone */
/* Alex M 19th - Jan. 1999 */

#include "frame.h"
#include "cheat.h"
#include "console.h"
#include "keybind.h"

static	BOOL	bAllowCheatCodes = TRUE;

typedef struct _cheat_entry
{
STRING	*pName;
void (*function)(void);	// pointer to void* function
} CHEAT_ENTRY;

/* Non Xor cheat list. Run if/else to switch between or remove xor section ?? */
CHEAT_ENTRY	cheatCodes[] = 
{
//	{"VQKZMY^\\Z",kf_ToggleOverlays},       //interface
	{"give all",kf_AllAvailable},	        // give all TEST FUNCTION - MAKE EVERYTHING AVAILABLE
//	{"LWPH R^OOVQXL",kf_ShowMappings},      // show mappings - Dumps all the keyboard mappings to the console display
//	{"KZROS^KZL",kf_GiveTemplateSet},       // templates - ?? unknown
//	{"LZSZ\\K ^SS",kf_SelectAllCombatUnits},// select all
	{"from above",kf_ToggleGodMode},        // from above - god mode (Not sure what it does...)
//	{"SZK KWZMZ ]Z SVXWK",kf_RecalcLighting},// let there be light - Recalculates the lighting values for a tile
//	{"PJKSVQZ,",kf_ToggleOutline},
//	{"L\\MZZQ[JRO",kf_ScreenDump},	        // screendump
//	{"M^QXZL",kf_ToggleSensorDisplay},      // ranges ?? not defined
    {"udriveit",kf_ToggleDrivingMode},      // udriveit - Driving mode, use at your own risk!!
	{"unit stats",kf_DebugDroidInfo},       // unit stats - Writes out debug info about all the selected droids
    {"show me the power",kf_UpThePower},    // show me the power - 1000 extra power kf_UpThePower
	{"hallo mein schatz",kf_AddMissionOffWorld},// hallo mein schatz - Tell the scripts to start a mission
	{"timedemo",kf_FrameRate},              // timedemo - View frame rate and graphics engine data
	{"kill selected",kf_KillSelected},      // kill selected - kills the selected units
	{"john kettley",kf_ToggleWeather},      // john kettley - toggle weather rain,snow,clear
	{"how fast",kf_FrameRate},              // how fast?
	{"shakey",kf_ToggleShakeStatus},        // shakey - Toggles screen shaking when unit explodes
	{"mouseflip",kf_ToggleMouseInvert},     // mouseflip
    {"double up",kf_SetDoubleUnitsLevel},	// double up - harder units
	{"biffer baker",kf_SetBakerLevel},      // biffer baker - almost invulnerable
	{"easy",kf_SetEasyLevel},               // easy  - sets easy difficulty level. 
	{"normal",kf_SetNormalLevel},           // normal - sets normal difficulty level
	{"hard",kf_SetHardLevel},               // hard - sets hard difficulty level
    {"elite",kf_SetToughUnitsLevel},    	// elite - something new
    {"killer",kf_SetKillerLevel},           // killer - gamer suicide
	{"whale fin",kf_TogglePower},	        // whale fin - turns on/off infinte power
	{"get off my land",kf_KillEnemy},	    // get off my land - kills all enemy units on map
	{"version",kf_BuildInfo},	            // version - tells you when the game code was compiled
	{"time toggle",kf_ToggleMissionTimer},  // time toggle - starts, stops timer
	{"work harder",kf_FinishResearch},      // work harder - completes all research
	{"carol vorderman",kf_NoFaults},        // carol vorderman
	{"end of list",NULL}
};

/*
CHEAT_ENTRY	cheatCodes[] = 
{
//	{"VQKZMY^\\Z",kf_ToggleOverlays},       //interface
//	{"XVIZ ^SS",kf_AllAvailable},	        // give all TEST FUNCTION - MAKE EVERYTHING AVAILABLE
//	{"LWPH R^OOVQXL",kf_ShowMappings},      // show mappings - Dumps all the keyboard mappings to the console display
//	{"KZROS^KZL",kf_GiveTemplateSet},       // templates - ?? unknown
//	{"LZSZ\\K ^SS",kf_SelectAllCombatUnits},// select all
//	{"YMPR ^]PIZ",kf_ToggleGodMode},        // from above - god mode
//	{"SZK KWZMZ ]Z SVXWK",kf_RecalcLighting},// let there be light - Recalculates the lighting values for a tile
//	{"PJKSVQZ,",kf_ToggleOutline},
//	{"L\\MZZQ[JRO",kf_ScreenDump},	        // screendump
//	{"M^QXZL",kf_ToggleSensorDisplay},      // ranges ?? not defined
//	{"JQVK LK^KL",kf_DebugDroidInfo},       // unit stats - Writes out debug info about all the selected droids

	{"W^SSP RZVQ L\\W^KE",kf_AddMissionOffWorld},// hallo mein schatz - Tell the scripts to start a mission
	{"KVRZ[ZRP",kf_FrameRate},	            // timedemo
	{"TVSS LZsZ\\KZ[",kf_KillSelected},     // kill selected - kills the selected units
	{"UPWQ TZKKSZF",kf_ToggleWeather},      // john kettley - toggle weather rain,snow,clear
	{"WPH Y^LK",kf_FrameRate},              // how fast?
	{"LW^TZF",kf_ToggleShakeStatus},        // shakey - Toggles screen shaking when unit explodes
	{"RPJLZYSVO",kf_ToggleMouseInvert},     // mouseflip
	{"]VYYZM ]^TZM",kf_SetKillerLevel},     // biffer baker - almost invulnerable ?? Why 2
	{"LO^MTSZ XMZZQ",kf_SetKillerLevel},    // biffer baker - almost invulnerable ?? sparkle green?
	{"Z^LF",kf_SetEasyLevel},               // easy  - sets easy difficulty level. 
	{"QPMR^S",kf_SetNormalLevel},           // normal - sets normal difficulty level
	{"W^M[",kf_SetHardLevel},               // hard - sets hard difficulty level
	{"[PJ]SZ JO",kf_SetToughUnitsLevel},	// double up - harder units
	{"HW^SZ YVQ",kf_TogglePower},	        // whale fin - turns on/off infinte power
	{"XZK PYY RF S^Q[",kf_KillEnemy},	    // get off my land - kills all enemy units on map
	{"IZMLVPQ",kf_BuildInfo},	            // version - tells you when the game code was compiled
	{"KVRZ KPXXSZ",kf_ToggleMissionTimer},  // time toggle - starts, stops timer
	{"HPMT W^M[ZM",kf_FinishResearch},      // work harder - completes all research
	{"\\^MPS IPM[ZMR^Q",kf_NoFaults},       // carol vorderman
	{"end of list",NULL}
};
*/
unsigned char	cheatString[255];

unsigned char	*xorString(unsigned char *string)
{
unsigned char	*pReturn;

	pReturn = string;

	while(*pReturn)
	{
		if(*pReturn>32)
		{
			*pReturn = (UBYTE)(*pReturn ^ 0x3f);
		}
		pReturn++;
	}
	return(string);
}

void	setCheatCodeStatus(BOOL val)
{
	bAllowCheatCodes = val;
}

BOOL	getCheatCodeStatus( void )
{
	return(bAllowCheatCodes);
}

BOOL	attemptCheatCode( STRING *pName )
{
UDWORD	index;
STRING	errorString[255];
unsigned char	*xored;

	index = 0;
 
	while(cheatCodes[index].function!=NULL)
	{
		strcpy(cheatString,cheatCodes[index].pName);
		xored = cheatString; //xorString(cheatString);
		if(strcmp(pName,xored) == FALSE)	// strcmp oddity
		{
			/* We've got our man... */
			cheatCodes[index].function();	// run it
			/* And get out of here */
			return(TRUE);
		}
		index++;
	}
	/* We didn't find it */
	sprintf(errorString,"%s?",pName);
	addConsoleMessage(errorString,LEFT_JUSTIFY);
	return(FALSE);
}

