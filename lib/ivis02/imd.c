#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


// we need BSPIMD defined if we want to read & use the BSP imd files
		   
// Define this if we are compile ivis for the BSP generating tool, also for PIEBIN tool
//#define PIETOOL





#ifdef PIETOOL
#define BSPIMD				 
#define SAVEIMD
#endif



#include "ivisdef.h"
#include "imd.h"		  
//#include "geo.h"
//#include "txt.h"
#include "bug.h"
//#include "pal.h"
#include "tex.h"
#include "ivispatch.h"

#ifdef PIETOOL
#include "bspimd.h"
#endif
		 

//*************************************************************************

//*************************************************************************

#define MAX_SHAPE_RADIUS	160

//*************************************************************************


//*************************************************************************
#ifdef WIN32
void iV_IMDDrawTextureRaise(iIMDShape *shape, float scale);
void iV_IMDDrawTexturedHeightScaled(iIMDShape *shape, float scale);
#endif
void iV_IMDDrawTexturedShade(iIMDShape *shape, int32 lightLevel);





			  

// Output BSP Tree to a file


// This stuff saves out the BSP tree as generated by the BSP tool
//
// Because of the complexities of the BSP generation tool. The IMD is stored in a different
// format when it is generated compared to when it is loaded and used in the game.
// This means that when saving out a BSP tree from the tool other functions need to be included
// most of them are in ptrlist.c which is no longer included in ivis. See the source for BSPIMD
// for this file. Becareful not to confuse this with the same named file in deliverance/src which
// might or might not be the same.
// 
#ifdef PIETOOL

// Prototypes for the linked list handling
void *list_GetFirst( PSBSPPTRLIST pList );
void *list_GetNext( PSBSPPTRLIST pList, void *pData );


int BSPPolys,BSPNodes;
PSBSPTREENODE BSPNodeTable[iV_IMD_MAX_POLYS];
iIMDPoly * BSPPolyTable[iV_IMD_MAX_POLYS];


void OutputTriangleList(FILE *fp,PSBSPPTRLIST TriList)
{
	iIMDPoly *Triangle;
	int TriNum=0;
	int d;

	if (TriList==NULL) return;
	
	Triangle=list_GetFirst(TriList);	

	while (Triangle!=NULL)
	{
		fprintf(fp,"\t%8x %d",Triangle->flags,Triangle->npnts);
		for (d=0; d<Triangle->npnts; d++)
			fprintf(fp," %d",Triangle->pindex[d]);

		if (Triangle->flags & iV_IMD_TEXANIM)
		{
			
			if (Triangle->pTexAnim == NULL)
			{
				DBPRINTF(("No TexAnim pointer!\n"));				
			}
			else
			{
				fprintf(fp," %d %d %d %d",
					Triangle->pTexAnim->nFrames,
					Triangle->pTexAnim->playbackRate,
					Triangle->pTexAnim->textureWidth,
					Triangle->pTexAnim->textureHeight);
			}
		}

		
		// if textured write texture uv's
		if (Triangle->flags & (iV_IMD_TEX | iV_IMD_PSXTEX)) {
			for (d=0; d<Triangle->npnts; d++)
				fprintf(fp," %d %d",Triangle->vrt[d].u,Triangle->vrt[d].v);
		}
		fprintf(fp,"\n");
		Triangle=list_GetNext(TriList,Triangle);
	}
}


void CountTriangleList(PSBSPPTRLIST TriList)
{
	iIMDPoly *Triangle;

	assert(BSPPolys<iV_IMD_MAX_POLYS);

	if (TriList==NULL) return;
	if (TriList->iNumNodes==0) return;
	
	Triangle=list_GetFirst(TriList);	

	while (Triangle!=NULL)
	{
		BSPPolyTable[BSPPolys]=Triangle;
		BSPPolys++;

		Triangle=list_GetNext(TriList,Triangle);
	}
}






void CountBSPPolys( PSBSPTREENODE psNode)
{
	if ( psNode == NULL )
	{
		return;
	}

	BSPNodeTable[BSPNodes]=psNode;
	BSPNodes++;
	
	CountBSPPolys( psNode->link[LEFT]);
	CountTriangleList( psNode->psTriSameDir );
	CountTriangleList( psNode->psTriOppoDir );
	CountBSPPolys( psNode->link[RIGHT]);
}


void OutputBSPPolys( FILE *fp,PSBSPTREENODE psNode)
{
	if ( psNode == NULL )
	{
		
		return;
	}
	OutputBSPPolys( fp, psNode->link[LEFT]);
	OutputTriangleList( fp, psNode->psTriSameDir );
	OutputTriangleList( fp, psNode->psTriOppoDir );
	OutputBSPPolys( fp, psNode->link[RIGHT]);
}


int HuntNodeList(PSBSPTREENODE psNode)
{
	int i;

	for (i=0;i<BSPNodes;i++)
	{
		if (BSPNodeTable[i]==psNode)
			return i;
	}
	return -1;
}


int HuntPolyList(iIMDPoly *Poly)
{
	int i;

	for (i=0;i<BSPPolys;i++)
	{
		if (BSPPolyTable[i]==Poly)
			return i;
	}
	return 999;
}


void DumpTriangleList(FILE *fp,PSBSPPTRLIST TriList)
{
	
	iIMDPoly *Triangle;

	assert(BSPPolys<iV_IMD_MAX_POLYS);


//	DBPRINTF(("Dumping TriList %p\n",TriList));

	if (TriList==NULL) return;
	
	Triangle=list_GetFirst(TriList);	

	while (Triangle!=NULL)
	{
		fprintf(fp,"%d ",HuntPolyList(Triangle));
		Triangle=list_GetNext(TriList,Triangle);
	}
	fprintf(fp,"-1 ");
}							
			   


void OutputBSPNodes( FILE *fp,PSBSPTREENODE psNode)
{
	if ( psNode == NULL )
	{
		return;
	}

	fprintf(fp,"\t%d ",HuntNodeList(psNode->link[LEFT]));
	DumpTriangleList( fp, psNode->psTriSameDir );
	DumpTriangleList( fp, psNode->psTriOppoDir );
	fprintf(fp,"%d\n",HuntNodeList(psNode->link[RIGHT]));

	OutputBSPNodes( fp, psNode->link[LEFT]);

	OutputBSPNodes( fp, psNode->link[RIGHT]);
}



/*

	Count Polys & Nodes in bsp - also makes a table of them

*/
int GetBSPPolyCount( PSBSPTREENODE psNode, int *NodeCount)
{

	BSPPolys=0;
	BSPNodes=0;
	CountBSPPolys(psNode);
	*NodeCount=BSPNodes;
	return BSPPolys;

}


#endif
#ifdef SAVEIMD
// new code the write out the connectors !
void _imd_save_connectors(FILE *fp, iIMDShape *s)
{
	iVector *p;
	int i;

	if (s->nconnectors != 0)
	{
		fprintf(fp,"CONNECTORS %d\n",s->nconnectors);
		p = s->connectors;
		for (i=0; i<s->nconnectors; i++, p++)
		{
			fprintf(fp,"\t%d %d %d\n", p->x,p->y,p->z);		
		}

	}
}



//*************************************************************************
//*** save IMD file
//*
//* pre		shape successfully loaded
//*
//* params	filename = name of file to save to including .IMD extention
//*			s 			= pointer to IMD shape
//*
//* returns TRUE -> ok, FLASE -> error
//*
//******

iBool iV_IMDSave(char *filename, iIMDShape *s, BOOL PieIMD)
{
	FILE *fp;
	iIMDShape *sp;
	iIMDPoly *poly;
	int nlevel, i, j, d;


	if ((fp = fopen(filename,"w")) == NULL)
		return FALSE;

	if (PieIMD==TRUE)
	{
		fprintf(fp,"%s %d\n",PIE_NAME,PIE_VER);

	}
	else
	{
		fprintf(fp,"%s %d\n",IMD_NAME,IMD_VER);
	}

	fprintf(fp,"TYPE %x\n",s->flags);


	// if textured write tex page file info

	if (s->flags & iV_IMD_XTEX) {
		fprintf(fp,"TEXTURE %d %s %d %d\n",iV_TEXTYPE(s->texpage),
				iV_TEXNAME(s->texpage),iV_TEXWIDTH(s->texpage),
				iV_TEXHEIGHT(s->texpage));
	}

	// find number of levels in shape

	for (nlevel=0, sp = s; sp != NULL; sp = sp->next, nlevel++)
		;

	fprintf(fp,"LEVELS %d\n",nlevel);

	for (sp = s, i=0; i<nlevel; sp = sp->next, i++) {
		fprintf(fp,"LEVEL %d\n",(i+1));
		fprintf(fp,"POINTS %d\n",sp->npoints);

		// write shape points

		for (j=0; j<sp->npoints; j++)
			fprintf(fp,"\t%d %d %d\n",sp->points[j].x,sp->points[j].y,
					sp->points[j].z);


		// write shape polys

#ifdef PIETOOL
		if (sp->BSPNode==NULL)
#endif
		{			
			fprintf(fp,"POLYGONS %d\n",sp->npolys);
			for (poly = sp->polys, j=0; j<sp->npolys; j++, poly++) {
				fprintf(fp,"\t%8x %d",poly->flags,poly->npnts);
				for (d=0; d<poly->npnts; d++)
					fprintf(fp," %d",poly->pindex[d]);

				if (poly->flags & iV_IMD_TEXANIM)
				{
					if (poly->pTexAnim == NULL)
					{
						printf("No TexAnim pointer!\n");				
					}
					else
					{
						fprintf(fp," %d %d %d %d",
							poly->pTexAnim->nFrames,
							poly->pTexAnim->playbackRate,
							poly->pTexAnim->textureWidth,
							poly->pTexAnim->textureHeight);

					}
				}


				// if textured write texture uv's

				if (poly->flags & (iV_IMD_TEX | iV_IMD_PSXTEX)) {
					for (d=0; d<poly->npnts; d++)
						fprintf(fp," %d %d",poly->vrt[d].u,poly->vrt[d].v);
				}

				fprintf(fp,"\n");
			}
		}
#ifdef PIETOOL
		else
		{		
			int NodeCount;

			fprintf(fp,"POLYGONS %d\n",GetBSPPolyCount(sp->BSPNode,&NodeCount) );
			OutputBSPPolys(fp,sp->BSPNode);	// 

			fprintf(fp,"BSP %d\n",NodeCount );
			OutputBSPNodes(fp,sp->BSPNode);
		}
#endif

	}

	_imd_save_connectors(fp,s);	// Write out the connectors if they exist


	fclose(fp);

	return TRUE;
}
#endif



//*************************************************************************
//*** print IMD file info
//*
//* pre		shape successfully loaded
//*
//* params	s = pointer to IMD shape
//*
//******

void iV_IMDDebug(iIMDShape *s)
{
#ifdef WIN32
	iIMDShape *sp;
	iIMDPoly *poly;
	int nlevel, i, j, d;


	iV_DEBUG0("iV_IMDSave = file info *****************************\n");

	iV_DEBUG1("SHAPE\nflags\t%x\n",s->flags);
	iV_DEBUG1("texpage\t%d\n",s->texpage);
	iV_DEBUG1("oradius\t%d\n",s->oradius);
	iV_DEBUG1("sradius\t%d\n",s->sradius);
	iV_DEBUG1("radius\t%d\n",s->radius);
	iV_DEBUG2("xmin, xmax\t%d, %d\n",s->xmin,s->xmax);
	iV_DEBUG2("ymin, ymax\t%d, %d\n",s->ymin,s->ymax);
	iV_DEBUG2("zmin, zmax\t%d, %d\n",s->zmin,s->zmax);
	iV_DEBUG3("ocen\t%d %d %d\n",s->ocen.x,s->ocen.y,s->ocen.z);
	iV_DEBUG1("npoints\t%d\n",s->npoints);
	iV_DEBUG1("npolys\t%d\n",s->npolys);
	iV_DEBUG1("points\t%p\n",s->points);
	iV_DEBUG1("polys\t%p\n",s->polys);
	iV_DEBUG1("ntexanims\t%d\n",s->ntexanims);
	iV_DEBUG1("texanims\t%p\n",s->ntexanims);
	iV_DEBUG1("next\t%p\n",s->next);

	// find number of levels in shape

	for (nlevel=0, sp = s; sp != NULL; sp = sp->next, nlevel++)
		;

	iV_DEBUG1("nlevels\t%d\n",nlevel);


	for (sp = s, i=0; i<nlevel; sp = sp->next, i++) {
		iV_DEBUG1("POINTS %d\n",sp->npoints);

		for (j=0; j<sp->npoints; j++)
			iV_DEBUG3("\t%d %d %d\n",sp->points[j].x,sp->points[j].y,
					sp->points[j].z);


		iV_DEBUG1("POLYGONS %d\n",sp->npolys);

		// write shape polys

		for (poly = sp->polys, j=0; j<sp->npolys; j++, poly++) {
			iV_DEBUG2("\t%8x %d",poly->flags,poly->npnts);
			for (d=0; d<poly->npnts; d++)
				iV_DEBUG1(" %d",poly->pindex[d]);

			// if textured write texture uv's

			if (poly->flags & iV_IMD_TEX) {
				for (d=0; d<poly->npnts; d++)
					iV_DEBUG2(" %d %d",poly->vrt[d].u,poly->vrt[d].v);
			}

			iV_DEBUG0("\n");
		}
	}
#endif
}


//*************************************************************************
//*** free IMD shape memory
//*
//* pre		shape successfully allocated
//*
//* params	shape = pointer to IMD shape
//*
//******

void iV_IMDRelease(iIMDShape *s)

{
   int i;
   iIMDShape *d;

   if (s) {

		if (s->flags & iV_IMD_BINARY)
		{
			iV_HeapFree(s,0);
			return;
		}

		if (s->flags & iV_IMD_XEFFECT)
		{
		  iV_HeapFree(s,sizeof(iIMDShapeEffect));		// free the special effect
			return;
		}


		{


	      if (s->points)
				iV_HeapFree(s->points,s->npoints * sizeof(iVector));

	      if (s->connectors)
				iV_HeapFree(s->connectors,s->nconnectors * sizeof(iVector));

		  if (s->BSPNode)
				FREE(s->BSPNode);	// I used MALLOC() so i'm going to use FREE()

	      if (s->polys) {
	         for (i=0; i<s->npolys; i++) {
	            if (s->polys[i].pindex) iV_HeapFree(s->polys[i].pindex,s->polys[i].npnts * sizeof(int));
	            if (s->polys[i].pTexAnim) iV_HeapFree(s->polys[i].pTexAnim,sizeof(iTexAnim));
				if (s->polys[i].vrt) iV_HeapFree(s->polys[i].vrt,s->polys[i].npnts * sizeof(iVertex));
	         }
	         iV_HeapFree(s->polys,s->npolys * sizeof(iIMDPoly));
	      }

			iV_DEBUG0("imd[IMDRelease] = release successful\n");

		      d = s->next;
			iV_HeapFree(s,sizeof(iIMDShape));

			iV_IMDRelease(d);

			
		}

   }
}

