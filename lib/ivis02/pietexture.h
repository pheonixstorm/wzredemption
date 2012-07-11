/***************************************************************************/
/*
 * pieState.h
 *
 * render State controlr all pumpkin image library functions.
 *
 */
/***************************************************************************/

#ifndef _pieTexture_h
#define _pieTexture_h


/***************************************************************************/

#include "frame.h"
#include "piedef.h"

/***************************************************************************/
/*
 *	Global Definitions
 */
/***************************************************************************/


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
extern BOOL pie_Download8bitTexturePage(void* bitmap,UWORD Width,UWORD Height);//assumes 256*256 page
extern BOOL pie_Reload8bitTexturePage(void* bitmap,UWORD Width,UWORD Height, SDWORD index);
extern UDWORD pie_GetLastPageDownloaded(void);
extern int pie_AddBMPtoTexPages( 	iSprite* s, char* filename, int type,
					iBool bColourKeyed, iBool bResource);
#endif // _pieTexture_h
