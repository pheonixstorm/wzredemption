/***************************************************************************/
/*
 * pieMatrix.h
 *
 * matrix functions for pumpkin image library.
 *
 */
/***************************************************************************/
#ifndef _pieMatrix_h
#define _pieMatrix_h

#include "pieDef.h"

/***************************************************************************/
/*
 *	Global Definitions
 */
/***************************************************************************/

typedef struct {SDWORD a, b, c,  d, e, f,  g, h, i,  j, k, l;} SDMATRIX;

/***************************************************************************/
/*
 *	Global Variables
 */
/***************************************************************************/

extern SDMATRIX *psMatrix;
extern SDWORD	aSinTable[];

//*************************************************************************

#define SIN(X)					aSinTable[(uint16)(X) >> 4]
#define COS(X)					aSinTable[((uint16)(X) >> 4) + 1024]
#define pie_INVTRANSX(X)		psMatrix->j = (X)<<FP12_SHIFT
#define pie_INVTRANSY(Y)		psMatrix->k = (Y)<<FP12_SHIFT
#define pie_INVTRANSZ(Z)		psMatrix->l = (Z)<<FP12_SHIFT
#define pie_MATTRANS(X,Y,Z)		{	psMatrix->j = (X)<<FP12_SHIFT;				\
									psMatrix->k = (Y)<<FP12_SHIFT;				\
									psMatrix->l = (Z)<<FP12_SHIFT;	}
//*************************************************************************

#define pie_SETUP_ROTATE_PROJECT																\
	int32 _ivzfx, _ivzfy, _ivx, _ivy, _ivz

#define pie_ROTATE_PROJECT(x,y,z,xs,ys)										\
{																			\
	_ivx = (x) * psMatrix->a + (y) * psMatrix->d + (z) * psMatrix->g +	\
				psMatrix->j;												\
	_ivy = (x) * psMatrix->b + (y) * psMatrix->e + (z) * psMatrix->h +	\
				psMatrix->k;												\
	_ivz = (x) * psMatrix->c + (y) * psMatrix->f + (z) * psMatrix->i +	\
				psMatrix->l;												\
	_ivzfx = _ivz >> psRendSurface->xpshift;							\
	_ivzfy = _ivz >> psRendSurface->ypshift;							\
	if ((_ivzfx > 0) && (_ivzfy > 0))										\
	{																		\
		(xs) = psRendSurface->xcentre + (int32) (_ivx / _ivzfx);		\
		(ys) = psRendSurface->ycentre - (int32) (_ivy / _ivzfy);		\
	}																		\
	else																	\
	{																		\
		(xs) = 1<<15;														\
		(ys) = 1<<15;														\
	}																		\
}

//*************************************************************************

#define pie_ROTATE_TRANSLATE(x,y,z,xs,ys,zs)										\
{																			\
	xs = (x) * psMatrix->a + (y) * psMatrix->d + (z) * psMatrix->g +	\
				psMatrix->j;												\
	ys = (x) * psMatrix->b + (y) * psMatrix->e + (z) * psMatrix->h +	\
				psMatrix->k;												\
	zs = (x) * psMatrix->c + (y) * psMatrix->f + (z) * psMatrix->i +	\
				psMatrix->l;												\
	xs >>=FP12_SHIFT;												\
	ys >>=FP12_SHIFT;												\
	zs >>=FP12_SHIFT;												\
}

//*************************************************************************

#define pie_TRANSLATE(x, y, z)																\
{																									\
	psMatrix->j += ((x) * psMatrix->a + (y) * psMatrix->d + 					\
						(z) * psMatrix->g);													\
	psMatrix->k += ((x) * psMatrix->b + (y) * psMatrix->e + 					\
						(z) * psMatrix->h);													\
	psMatrix->l += ((x) * psMatrix->c + (y) * psMatrix->f + 					\
						(z) * psMatrix->i);													\
}

//*************************************************************************

#define pie_CLOCKWISE(x0,y0,x1,y1,x2,y2) ((((y1)-(y0)) * ((x2)-(x1))) <=	\
															(((x1)-(x0)) * ((y2)-(y1))))

//*************************************************************************

#ifdef WIN32
extern void pie_MatInit(void);
#else//tim you may want to rename the playstation function aswell
extern void _iv_geo_setup(void);
#endif

//*************************************************************************

extern void pie_MatBegin(void);
extern void pie_MatEnd(void);
extern void pie_MatRotX(int x);
extern void pie_MatRotY(int y);
extern void pie_MatRotZ(int z);
extern int32 pie_RotProj(iVector *v3d, iPoint *v2d);

//*************************************************************************

extern void pie_VectorNormalise(iVector *v);
extern void pie_VectorInverseRotate0(iVector *v1, iVector *v2);
extern void pie_SurfaceNormal(iVector *p1, iVector *p2, iVector *p3, iVector *v);
extern BOOL pie_Clockwise(iVertex *s);
extern void pie_SetGeometricOffset(int x, int y);

#ifdef WIN32
// PIEVERTEX structure contains much infomation that is not required on the playstation ... and hence is not currently used
extern BOOL pie_PieClockwise(PIEVERTEX *s);	
#endif


#endif
