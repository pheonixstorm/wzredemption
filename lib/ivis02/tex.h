#ifndef _tex_
#define _tex_

#include "ivi.h"

//*************************************************************************

#define iV_TEX_MAX		48

//*************************************************************************

#define iV_TEXTEX(i)		((iTexture *) (&_TEX_PAGE[(i)].tex))
#define iV_TEXPAGE(i)	((iTexPage *) (&_TEX_PAGE[(i)]))
#define iV_TEXBMP(i)		((iBitmap *) (&_TEX_PAGE[(i)].tex.bmp))
#define iV_TEXWIDTH(i)	(_TEX_PAGE[(i)].tex.width)
#define iV_TEXHEIGHT(i) (_TEX_PAGE[(i)].tex.height)
#define iV_TEXNAME(i)	((char *) (&_TEX_PAGE[(i)].name))
#define iV_TEXTYPE(i)	(_TEX_PAGE[(i)].type)

//*************************************************************************

typedef struct
{
	iTexture	tex;
	uint8		type;
	char		name[80];
	int			textPage3dfx;	// what page number is it on 3dfx - not the same thing
	int			bResource;		// Was page provided by resource handler?
}
iTexPage;

//*************************************************************************
extern int _TEX_INDEX;
extern iTexPage	_TEX_PAGE[iV_TEX_MAX];

//*************************************************************************

extern int iV_TexLoad( char *path, char *filename, int type,
						iBool palkeep, iBool bColourKeyed );
extern int iV_TexLoadNew( char *path, char *filename, int type,
					iBool palkeep, iBool bColourKeyed );
extern int pie_ReloadTexPage(char *filename,UBYTE *pBuffer);
extern int pie_AddBMPtoTexPages( 	iSprite* s, char* filename, int type, iBool bColourKeyed, iBool bResource);
extern void pie_TexInit(void);

//*************************************************************************

extern void pie_TexShutDown(void);

extern BOOL iV_TexSizeIsLegal(UDWORD Width,UDWORD Height);
extern BOOL iV_IsPower2(UDWORD Value);


BOOL GenerateTEXPAGE(char *Filename, RECT *VramArea, UDWORD Mode, UWORD Clut);
BOOL FindTextureNumber(UDWORD TexNum,int* TexPage);

#endif
