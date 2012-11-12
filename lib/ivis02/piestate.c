/***************************************************************************/
/*
 * pieState.c
 *
 * renderer setup and state control routines for 3D rendering
 *
 */
/***************************************************************************/

#include "frame.h"
#include "piestate.h"
#include "piedef.h"
#include "d3drender.h"
#include "dx6texman.h"
#include "tex.h"
#include "texd3d.h"
#ifdef INC_GLIDE
	#include "rendfunc.h"
	#include "dGlide.h"
	#include "3dfxFunc.h"
	#include "3dfxText.h"
#endif


/***************************************************************************/
/*
 *	Global Variables
 */
/***************************************************************************/

SDWORD		pieStateCount = 0;
BOOL		bSwirls = FALSE;
BOOL		bWave = FALSE;
/***************************************************************************/
/*
 *	Local Definitions
 */
/***************************************************************************/

typedef	enum	COLOUR_MODE
				{
					COLOUR_FLAT_CONSTANT,
					COLOUR_FLAT_ITERATED,
					COLOUR_TEX_ITERATED,
					COLOUR_TEX_CONSTANT
				}
				COLOUR_MODE;

typedef	enum	TEX_MODE
				{
					TEX_LOCAL,
					TEX_NONE
				}
				TEX_MODE;

typedef	enum	ALPHA_MODE
				{
					ALPHA_ITERATED,
					ALPHA_CONSTANT
				}
				ALPHA_MODE;

typedef struct _renderState
{
	REND_ENGINE			rendEngine;
	BOOL				bHardware;
	DEPTH_MODE			depthBuffer;
	BOOL				translucent;
	BOOL				additive;
	FOG_CAP				fogCap;
	BOOL				fogEnabled;
	BOOL				fog;
	UDWORD				fogColour;
	TEX_CAP				texCap;
	SDWORD				texPage;
	REND_MODE			rendMode;
	BOOL				bilinearOn;
	BOOL				keyingOn;
	COLOUR_MODE			colourCombine;
	TEX_MODE			texCombine;
	ALPHA_MODE			alphaCombine;
	TRANSLUCENCY_MODE	transMode;
	UDWORD				colour;
#ifdef STATES
	BOOL				textured;
	UBYTE				lightLevel;
#endif
	UBYTE				DDrawDriverName[256];
	UBYTE				D3DDriverName[256];
} RENDER_STATE;

/***************************************************************************/
/*
 *	Local Variables
 */
/***************************************************************************/

RENDER_STATE	rendStates;

/***************************************************************************/
/*
 *	Local ProtoTypes
 */
/***************************************************************************/
static void pie_SetColourCombine(COLOUR_MODE colCombMode);
static void pie_SetTexCombine(TEX_MODE texCombMode);
static void pie_SetAlphaCombine(ALPHA_MODE alphaCombMode);
static void pie_SetTranslucencyMode(TRANSLUCENCY_MODE transMode);
//static void pie_SetAlphaCombine(BOOL trans);

/***************************************************************************/
/*
 *	Source
 */
/***************************************************************************/
void pie_SetDefaultStates(void)//Sets all states
{
//		pie_SetFogColour(0x00B08f5f);//nicks colour
	//fog off
	rendStates.fogEnabled = FALSE;// enable fog before renderer
	rendStates.fog = FALSE;//to force reset to false
	pie_SetFogStatus(FALSE);
	pie_SetFogColour(0x00000000);//nicks colour

	//depth Buffer on
	rendStates.depthBuffer = FALSE;//to force reset to true
	pie_SetDepthBufferStatus(DEPTH_CMP_LEQ_WRT_ON);

	//set render mode
	pie_SetTranslucent(TRUE);
	pie_SetAdditive(TRUE);
	
	//basic gouraud textured rendering
	rendStates.texCombine = TEX_NONE;//to force reset to GOURAUD_TEX
	pie_SetTexCombine(TEX_LOCAL);
	rendStates.colourCombine = COLOUR_FLAT_CONSTANT;//to force reset to GOURAUD_TEX
	pie_SetColourCombine(COLOUR_TEX_ITERATED);
	rendStates.alphaCombine = ALPHA_ITERATED;//to force reset to GOURAUD_TEX
	pie_SetAlphaCombine(ALPHA_CONSTANT);
	rendStates.transMode = TRANS_ALPHA;//to force reset to DECAL
	pie_SetTranslucencyMode(TRANS_DECAL);

	//chroma keying on black
	rendStates.keyingOn = FALSE;//to force reset to true
	pie_SetColourKeyedBlack(TRUE);

	//bilinear filtering
	rendStates.bilinearOn = FALSE;//to force reset to true
	pie_SetBilinear(TRUE);
}

/***************************************************************************/
/***************************************************************************/
void pie_ResetStates(void)//Sets all states
{
	SDWORD		temp;

//		pie_SetFogColour(0x00B08f5f);//nicks colour
	rendStates.fog = !rendStates.fog;//to force reset
	pie_SetFogStatus(!rendStates.fog);

	//depth Buffer on
	temp = rendStates.depthBuffer;
	rendStates.depthBuffer = -1;//to force reset
	pie_SetDepthBufferStatus(temp);

	//set render mode
//	pie_SetTranslucent(TRUE);
//	pie_SetAdditive(TRUE);
	
	//basic gouraud textured rendering
	temp = rendStates.texCombine;
	rendStates.texCombine = -1;//to force reset
	pie_SetTexCombine(temp);

	temp = rendStates.colourCombine;
	rendStates.colourCombine = -1;//to force reset
	pie_SetColourCombine(temp);

	temp = rendStates.alphaCombine;
	rendStates.alphaCombine = -1;//to force reset
	pie_SetAlphaCombine(temp);

	temp = rendStates.transMode;
	rendStates.transMode = -1;//to force reset
	pie_SetTranslucencyMode(temp);

	//chroma keying on black
	temp = rendStates.keyingOn;
	rendStates.keyingOn = -1;//to force reset
	pie_SetColourKeyedBlack(temp);

	//bilinear filtering
	temp = rendStates.bilinearOn;
	rendStates.bilinearOn = -1;//to force reset
	pie_SetBilinear(temp);
}

/***************************************************************************/
/***************************************************************************/
void pie_SetRenderEngine(REND_ENGINE rendEngine)
{
	rendStates.rendEngine = rendEngine;
	if ((rendEngine == ENGINE_GLIDE) || (rendEngine == ENGINE_D3D))
	{
		rendStates.bHardware = TRUE;
	}
	else
	{
		rendStates.bHardware = FALSE;
	}
}

REND_ENGINE pie_GetRenderEngine(void)
{
	return rendStates.rendEngine;
}

BOOL	pie_Hardware(void)
{
	return rendStates.bHardware;
}

/***************************************************************************/
/***************************************************************************/
void pie_SetDirectDrawDeviceName(char* pDDDeviceName)
{
	ASSERT((strlen(pDDDeviceName) < 255,"DirectDraw device string exceeds max string length."));
	if (strlen(pDDDeviceName) >= 255)
	{
		pDDDeviceName[255] = 0;
	}
	strcpy((char*)(rendStates.DDrawDriverName),pDDDeviceName);
}

char* pie_GetDirectDrawDeviceName(void)
{
	return (char*)(rendStates.DDrawDriverName);
}

/***************************************************************************/
/***************************************************************************/
void pie_SetDirect3DDeviceName(char* pD3DDeviceName)
{
	ASSERT((strlen(pD3DDeviceName) < 255,"Direct3D device string exceeds max string length."));
	if (strlen(pD3DDeviceName) >= 255)
	{
		pD3DDeviceName[255] = 0;
	}
	strcpy((char*)(rendStates.D3DDriverName),pD3DDeviceName);
}

char* pie_GetDirect3DDeviceName(void)
{
	return (char*)(rendStates.D3DDriverName);
}

/***************************************************************************/
/***************************************************************************/

void pie_SetDepthBufferStatus(DEPTH_MODE depthMode)
{
#ifndef PIETOOL		
	if (rendStates.depthBuffer != depthMode)
	{
		rendStates.depthBuffer = depthMode;
		if (rendStates.rendEngine == ENGINE_D3D)
		{
			switch(depthMode)
			{
				case DEPTH_CMP_LEQ_WRT_ON:
					D3DSetDepthCompare(D3DCMP_LESSEQUAL);
					D3DSetDepthWrite(TRUE);
					break;
				case DEPTH_CMP_ALWAYS_WRT_ON:
					D3DSetDepthCompare(D3DCMP_ALWAYS);
					D3DSetDepthWrite(TRUE);
					break;
				case DEPTH_CMP_LEQ_WRT_OFF:
					D3DSetDepthCompare(D3DCMP_LESSEQUAL);
					D3DSetDepthWrite(FALSE);
					break;
				case DEPTH_CMP_ALWAYS_WRT_OFF:
					D3DSetDepthCompare(D3DCMP_ALWAYS);
					D3DSetDepthWrite(FALSE);
					break;
			}
		}
		else if (rendStates.rendEngine == ENGINE_GLIDE)
		{ 
			switch(depthMode)
			{
				case DEPTH_CMP_LEQ_WRT_ON:
					grDepthBufferFunction(GR_CMP_LEQUAL);
					grDepthMask(TRUE);
					break;
				case DEPTH_CMP_ALWAYS_WRT_ON:
					grDepthBufferFunction(GR_CMP_ALWAYS);
					grDepthMask(TRUE);
					break;
				case DEPTH_CMP_LEQ_WRT_OFF:
					grDepthBufferFunction(GR_CMP_LEQUAL);
					grDepthMask(FALSE);
					break;
				case DEPTH_CMP_ALWAYS_WRT_OFF:
					grDepthBufferFunction(GR_CMP_ALWAYS);
					grDepthMask(FALSE);
					break;
			}
		}
	}
#endif
}

//***************************************************************************
//
// pie_SetTranslucent(BOOL val);
//
// Global enable/disable Translucent effects 
//
//***************************************************************************

void pie_SetTranslucent(BOOL val)
{
	rendStates.translucent = val;
}

BOOL pie_Translucent(void)
{
	return rendStates.translucent;
}

//***************************************************************************
//
// pie_SetAdditive(BOOL val);
//
// Global enable/disable Additive effects 
//
//***************************************************************************

void pie_SetAdditive(BOOL val)
{
	rendStates.additive = val;
}

BOOL pie_Additive(void)
{
	return rendStates.additive;
}

//***************************************************************************
//
// pie_SetCaps(BOOL val);
//
// HIGHEST LEVEL enable/disable modes 
//
//***************************************************************************


void pie_SetFogCap(FOG_CAP val)
{
	rendStates.fogCap = val;
}

FOG_CAP pie_GetFogCap(void)
{
	return rendStates.fogCap;
}

void pie_SetTexCap(TEX_CAP val)
{
	rendStates.texCap = val;
}

TEX_CAP pie_GetTexCap(void)
{
	return rendStates.texCap;
}

//***************************************************************************
//
// pie_EnableFog(BOOL val)
//
// Global enable/disable fog to allow fog to be turned of ingame 
//
//***************************************************************************

void pie_EnableFog(BOOL val)
{
	if (rendStates.fogCap == FOG_CAP_NO)
	{
		val = FALSE;
	}
	if (rendStates.fogEnabled != val)
	{
		rendStates.fogEnabled = val;
		if (val == TRUE)
		{
//			pie_SetFogColour(0x0078684f);//(nicks colour + 404040)/2
			pie_SetFogColour(0x00B08f5f);//nicks colour
		}
		else
		{
			pie_SetFogColour(0x00000000);//clear background to black
		}

	}
}

BOOL pie_GetFogEnabled(void)
{
	return  rendStates.fogEnabled;
}

//***************************************************************************
//
// pie_SetFogStatus(BOOL val)
//
// Toggle fog on and off for rendering objects inside or outside the 3D world
//
//***************************************************************************

void pie_SetFogStatus(BOOL val)
{
	if (rendStates.fogEnabled)
	{
		//fog enabled so toggle if required 
		if (rendStates.fog != val)
		{
			rendStates.fog = val;
			if (pie_GetRenderEngine() == ENGINE_GLIDE)
			{
				pieStateCount++;
				gl_SetFogStatus(val);
			}
		}
	}
	else
	{
		//fog disabled so turn it off if not off already 
		if (rendStates.fog != FALSE)
		{
			rendStates.fog = FALSE;
			if (pie_GetRenderEngine() == ENGINE_GLIDE)
			{
				pieStateCount++;
				gl_SetFogStatus(FALSE);
			}
		}
	}
}

BOOL pie_GetFogStatus(void)
{
	return  rendStates.fog;
}
/***************************************************************************/
void pie_SetFogColour(UDWORD colour)
{
	UDWORD grey;
 	if (rendStates.fogCap == FOG_CAP_GREY)
	{
		grey = colour & 0xff;
		colour >>= 8;
		grey += (colour & 0xff);
		colour >>= 8;
		grey += (colour & 0xff);
		grey /= 3;
		grey &= 0xff;//check only
		colour = grey + (grey<<8) + (grey<<16);
		rendStates.fogColour = colour;
	}
	else if (rendStates.fogCap == FOG_CAP_NO)
	{
		rendStates.fogColour = 0;
	}
	else
	{
		rendStates.fogColour = colour;
	}
}

UDWORD pie_GetFogColour(void)
{
	return rendStates.fogColour;
}
/***************************************************************************/
void pie_SetTexturePage(SDWORD num)
{
#ifndef PIETOOL
	if (num != rendStates.texPage)
	{
		rendStates.texPage = num;
		if (num < 0)
		{
			if (rendStates.rendEngine == ENGINE_D3D)
			{
				dtm_SetTexturePage(-1);
			}
		}
		else
		{
			if (rendStates.rendEngine == ENGINE_GLIDE)
			{
				gl_SelectTexturePage(num);
			}
			else if (rendStates.rendEngine == ENGINE_D3D)
			{
				dtm_SetTexturePage(num);
			}
		}
	}
#endif
}

/***************************************************************************/
void pie_SetRendMode(REND_MODE rendMode)
{
	if (rendMode != rendStates.rendMode)
	{
		rendStates.rendMode = rendMode;
		switch (rendMode)
		{
			case REND_GOURAUD_TEX:
				pie_SetColourCombine(COLOUR_TEX_ITERATED);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_CONSTANT);
				pie_SetTranslucencyMode(TRANS_DECAL);
				break;
			case REND_ALPHA_TEX:
				pie_SetColourCombine(COLOUR_TEX_ITERATED);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_ITERATED);
				pie_SetTranslucencyMode(TRANS_ALPHA);
				break;
			case REND_ADDITIVE_TEX:
				pie_SetColourCombine(COLOUR_TEX_ITERATED);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_ITERATED);
				pie_SetTranslucencyMode(TRANS_ADDITIVE);
				break;
			case REND_TEXT:
				pie_SetColourCombine(COLOUR_TEX_CONSTANT);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_CONSTANT);
				pie_SetTranslucencyMode(TRANS_DECAL);
				break;
			case REND_ALPHA_TEXT:
				pie_SetColourCombine(COLOUR_TEX_CONSTANT);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_CONSTANT);
				pie_SetTranslucencyMode(TRANS_ALPHA);
				break;
			case REND_ALPHA_FLAT:
				pie_SetColourCombine(COLOUR_FLAT_CONSTANT);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_CONSTANT);
				pie_SetTranslucencyMode(TRANS_ALPHA);
				break;
			case REND_ALPHA_ITERATED:
				pie_SetColourCombine(COLOUR_FLAT_ITERATED);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_ITERATED);
				pie_SetTranslucencyMode(TRANS_ADDITIVE);
				break;
		   	case REND_FILTER_FLAT:
				pie_SetColourCombine(COLOUR_FLAT_CONSTANT);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_CONSTANT);
				pie_SetTranslucencyMode(TRANS_FILTER);
				break;
			case REND_FILTER_ITERATED:
				pie_SetColourCombine(COLOUR_FLAT_CONSTANT);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_ITERATED);
				pie_SetTranslucencyMode(TRANS_ALPHA);
				break;
			case REND_FLAT:
				pie_SetColourCombine(COLOUR_FLAT_CONSTANT);
				pie_SetTexCombine(TEX_LOCAL);
				pie_SetAlphaCombine(ALPHA_CONSTANT);
				pie_SetTranslucencyMode(TRANS_DECAL);
			default:
				break;
		}
	}
	return;
}

/*
BOOL pie_GetTranslucent(void)
{
	if (rendStates.transMode == TRANS_DECAL)
	{
		return FALSE;
	}
	return TRUE;

}
*/
/***************************************************************************/

void pie_SetColourKeyedBlack(BOOL keyingOn)
{
#ifndef PIETOOL
	if (keyingOn != rendStates.keyingOn)
	{
		rendStates.keyingOn = keyingOn;
		pieStateCount++;
		if (rendStates.rendEngine == ENGINE_GLIDE)
		{
			grChromakeyMode(keyingOn);
			grChromakeyValue(0x00000000);
		}
		else if (rendStates.rendEngine == ENGINE_D3D)
		{
			D3DSetColourKeying(keyingOn);
		}
	}
#endif
}

/***************************************************************************/
void pie_SetBilinear(BOOL bilinearOn)
{
#ifndef PIETOOL
	if (bilinearOn != rendStates.bilinearOn)
	{
		rendStates.bilinearOn = bilinearOn;
		pieStateCount++;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			if (bilinearOn == TRUE)
			{
				grTexFilterMode( GR_TMU0,
								 GR_TEXTUREFILTER_BILINEAR,
								 GR_TEXTUREFILTER_BILINEAR );
			}
			else
			{
				grTexFilterMode( GR_TMU0,
								 GR_TEXTUREFILTER_POINT_SAMPLED,
								 GR_TEXTUREFILTER_POINT_SAMPLED );
			}
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			dx6_SetBilinear(bilinearOn);
		}
	}
#endif
}

BOOL pie_GetBilinear(void)
{
#ifndef PIETOOL
	return rendStates.bilinearOn;
#else
	return FALSE;
#endif
}

/***************************************************************************/
static void pie_SetColourCombine(COLOUR_MODE colCombMode)
{
#ifndef PIETOOL	//ffs

	if (colCombMode != rendStates.colourCombine)
	{
		rendStates.colourCombine = colCombMode;
		pieStateCount++;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			switch (colCombMode)
			{
				case COLOUR_TEX_CONSTANT:
					grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
									GR_COMBINE_FACTOR_LOCAL,
									GR_COMBINE_LOCAL_CONSTANT,
									GR_COMBINE_OTHER_TEXTURE,
									FXFALSE );
					break;
				case COLOUR_FLAT_CONSTANT:
					grColorCombine( GR_COMBINE_FUNCTION_LOCAL,
									GR_COMBINE_FACTOR_NONE,
									GR_COMBINE_LOCAL_CONSTANT,
									GR_COMBINE_OTHER_NONE,
									FXFALSE );
					break;
				case COLOUR_TEX_ITERATED:
				default:
					grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
									GR_COMBINE_FACTOR_LOCAL,
									GR_COMBINE_LOCAL_ITERATED,
									GR_COMBINE_OTHER_TEXTURE,
									FXFALSE );
					break;
				case COLOUR_FLAT_ITERATED:
					grColorCombine( GR_COMBINE_FUNCTION_LOCAL,
									GR_COMBINE_FACTOR_NONE,
									GR_COMBINE_LOCAL_ITERATED,
									GR_COMBINE_OTHER_NONE,
									FXFALSE );
					break;
			}
		}
		else if (pie_GetRenderEngine() == ENGINE_D3D)
		{
			switch (colCombMode)
			{
				case COLOUR_TEX_CONSTANT:
					break;
				case COLOUR_FLAT_CONSTANT:
				case COLOUR_FLAT_ITERATED:
					pie_SetTexturePage(-1);
					break;
				case COLOUR_TEX_ITERATED:
				default:
					break;
			}
		}
	}
#endif
}

/***************************************************************************/
static void pie_SetTexCombine(TEX_MODE texCombMode)
{
#ifndef PIETOOL	//ffs
	if (texCombMode != rendStates.texCombine)
	{
		rendStates.texCombine = texCombMode;
		pieStateCount++;
		switch (texCombMode)
		{
			case TEX_LOCAL:
			case TEX_NONE:
			default:
				if (pie_GetRenderEngine() == ENGINE_GLIDE)
				{
					grTexCombine( GR_TMU0,
								  GR_COMBINE_FUNCTION_LOCAL,
								  GR_COMBINE_FACTOR_NONE,
								  GR_COMBINE_FUNCTION_NONE,
								  GR_COMBINE_FACTOR_NONE,
								  FXFALSE, FXFALSE );
				}
				break;
		}
	}
#endif
}

/***************************************************************************/
static void pie_SetAlphaCombine(ALPHA_MODE alphaCombMode)
{
#ifndef PIETOOL	//ffs
	if (alphaCombMode != rendStates.alphaCombine)
	{
		rendStates.alphaCombine = alphaCombMode;
		pieStateCount++;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			switch (alphaCombMode)
			{
				case ALPHA_ITERATED:
					grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
								  GR_COMBINE_FACTOR_ONE,
								  GR_COMBINE_LOCAL_NONE,
								  GR_COMBINE_OTHER_ITERATED,
								  FXFALSE);
					break;
				case ALPHA_CONSTANT:
				default:
					grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
								  GR_COMBINE_FACTOR_ONE,
								  GR_COMBINE_LOCAL_NONE,
								  GR_COMBINE_OTHER_CONSTANT,
								  FXFALSE);
					break;
			}
		}
	}
#endif
}

/***************************************************************************/
static void pie_SetTranslucencyMode(TRANSLUCENCY_MODE transMode)
{
#ifndef PIETOOL
	if (transMode != rendStates.transMode)
	{
		rendStates.transMode = transMode;
		pieStateCount++;
		if (rendStates.rendEngine == ENGINE_GLIDE)
		{
			switch (transMode)
			{
				case TRANS_DECAL:
					grAlphaBlendFunction(GR_BLEND_ONE,
							GR_BLEND_ZERO,
							GR_BLEND_ZERO,
							GR_BLEND_ZERO);
					break;
				case TRANS_ALPHA:
					grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
							GR_BLEND_ONE_MINUS_SRC_ALPHA,
							GR_BLEND_ZERO,
							GR_BLEND_ZERO);
					break;
				case TRANS_ADDITIVE:
					grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
								GR_BLEND_ONE,
								GR_BLEND_ZERO,
								GR_BLEND_ZERO);
					break;
				case TRANS_FILTER:						
					grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
							GR_BLEND_SRC_COLOR,
							GR_BLEND_ZERO,
							GR_BLEND_ZERO);
					break;
				default:
					rendStates.transMode = TRANS_DECAL;
					grAlphaBlendFunction(GR_BLEND_ONE,
							GR_BLEND_ZERO,
							GR_BLEND_ZERO,
							GR_BLEND_ZERO);
					break;
			}
		}
		else if (rendStates.rendEngine == ENGINE_D3D)
		{
			D3DSetTranslucencyMode(transMode);
			rendStates.transMode = transMode;
		}

	}
#endif
}

/***************************************************************************/
// set the constant colour used in text and flat render modes
/***************************************************************************/
void pie_SetColour(UDWORD colour)
{
	if (colour != rendStates.colour)
	{
		rendStates.colour = colour;
		pieStateCount++;
		if (pie_GetRenderEngine() == ENGINE_GLIDE)
		{
			grConstantColorValue(colour);
		}
	}
}

/***************************************************************************/
// get the constant colour used in text and flat render modes
/***************************************************************************/
UDWORD pie_GetColour(void)
{
	return	rendStates.colour;
}

/***************************************************************************/
void pie_SetGammaValue(float val)
{
	pieStateCount++;
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		gl_SetGammaValue(val);
	}
}

/***************************************************************************/
void pie_DrawMouse(SDWORD x,SDWORD y)
{
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		iV_DrawMousePointer(x, y);
	}
}

/***************************************************************************/
UWORD	presentMouseID;
void pie_SetMouse(IMAGEFILE *psImageFile,UWORD ImageID)
{
	if (pie_GetRenderEngine() == ENGINE_GLIDE)
	{
		iV_SetMousePointer(psImageFile,ImageID);
	}
	presentMouseID = ImageID;
}

/***************************************************************************/
UDWORD	pie_GetMouseID( void )
{
	return(presentMouseID);
}
/***************************************************************************/
BOOL	pie_SwirlyBoxes( void )
{
	return(bSwirls);
}

void	pie_SetSwirlyBoxes( BOOL val )
{
	bSwirls = val;
}

BOOL	pie_WaveBlit( void )
{
	return(bWave);
}

void	pie_SetWaveBlit( BOOL val )
{
	bWave = val;
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
