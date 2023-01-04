/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: vgabitbl.c,v $ $Revision: 1.1.4.6 $ (DEC) $Date: 1993/11/22 17:34:26 $";
#endif

#include <stdio.h>
#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "mi.h"
#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"
#ifdef DWDOS386
#include "alloca.h"
#endif /* DWDOS386 */

void(*doBitBlt)(
#ifndef __alpha
	struct _Drawable *pSrcDrawable, 
	struct _Drawable *pDstDrawable, 
	struct _GC *pGC, 
	struct _Region *prgnDst, 
	struct _DDXPoint *pptSrc
#endif
) 
		= vgaDoBitBlt;


RegionPtr
 vgaCopyArea(pSrcDrawable, pDstDrawable, 
	pGC, srcx, srcy, width, height, dstx, dsty)
	register DrawablePtr pSrcDrawable;
	register DrawablePtr pDstDrawable;
	GC *pGC;
	int srcx, srcy;
	int width, height;
	int dstx, dsty;

	{
	RegionPtr prgnSrcClip;	/* may be a new region, or just a copy */
	Bool freeSrcClip = FALSE;

	RegionPtr prgnExposed;
	RegionRec rgnDst;
	DDXPointPtr pptSrc;
	register DDXPointPtr ppt;
	register BoxPtr pbox;
	int i;
	register int dx;
	register int dy;
	xRectangle origSource;
	DDXPointRec origDest;
	int numRects;
	BoxRec fastBox;
	int fastClip = 0;	/* for fast clipping with pixmap source */
	int fastExpose = 0;	/* for fast exposures with pixmap source */

	origSource.x = srcx;
	origSource.y = srcy;
	origSource.width = width;
	origSource.height = height;
	origDest.x = dstx;
	origDest.y = dsty;

	srcx += pSrcDrawable->x;
	srcy += pSrcDrawable->y;

	/* clip the source */

	if (pSrcDrawable->type == DRAWABLE_PIXMAP)
		{
		fastClip = 1;
		}
	else
		{
		if (pGC->subWindowMode == IncludeInferiors)
			{
			if (!((WindowPtr)pSrcDrawable)->parent)
				{
				/*
				* special case bitblt from root window in
				* IncludeInferiors mode; just like from a pixmap
				*/
				fastClip = 1;
				}
			else 
			if ((pSrcDrawable == pDstDrawable) && 
			    (pGC->clientClipType == CT_NONE))
				{
				prgnSrcClip = ((mfbPrivGC*)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
				}
			else
				{
				prgnSrcClip = NotClippedByChildren((WindowPtr)pSrcDrawable);
				freeSrcClip = TRUE;
				}
			}
		else
			{
			prgnSrcClip = &((WindowPtr)pSrcDrawable)->clipList;
			}
		}

	fastBox.x1 = srcx;
	fastBox.y1 = srcy;
	fastBox.x2 = srcx+width;
	fastBox.y2 = srcy+height;

	/* Don't create a source region if we are doing a fast clip */
	if (fastClip)
		{
		fastExpose = 1;
		/*
		* clip the source; if regions extend beyond the source size,
		* make sure exposure events get sent
		*/
		if (fastBox.x1 < pSrcDrawable->x)
			{
			fastBox.x1 = pSrcDrawable->x;
			fastExpose = 0;
			}
		if (fastBox.y1 < pSrcDrawable->y)
			{
			fastBox.y1 = pSrcDrawable->y;
			fastExpose = 0;
			}
		if (fastBox.x2 > pSrcDrawable->x+(int)pSrcDrawable->width)
			{
			fastBox.x2 = pSrcDrawable->x+(int)pSrcDrawable->width;
			fastExpose = 0;
			}
		if (fastBox.y2 > pSrcDrawable->y+(int)pSrcDrawable->height)
			{
			fastBox.y2 = pSrcDrawable->y+(int)pSrcDrawable->height;
			fastExpose = 0;
			}
		}
	else
		{
		(*pGC->pScreen->RegionInit)(&rgnDst, &fastBox, 1);
		(*pGC->pScreen->Intersect)(&rgnDst, &rgnDst, prgnSrcClip);
		}

	dstx += pDstDrawable->x;
	dsty += pDstDrawable->y;

	if (pDstDrawable->type == DRAWABLE_WINDOW)
		{
		if (!((WindowPtr)pDstDrawable)->realized)
			{
			if (!fastClip)
				(*pGC->pScreen->RegionUninit)(&rgnDst);
			if (freeSrcClip)
				(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
			return NULL;
			}
		}

	dx = srcx-dstx;
	dy = srcy-dsty;

	/* Translate and clip the dst to the destination composite clip */
	if (fastClip)
		{
		RegionPtr cclip;

		/* Translate the region directly */
		fastBox.x1 -= dx;
		fastBox.x2 -= dx;
		fastBox.y1 -= dy;
		fastBox.y2 -= dy;

		/* If the destination composite clip is one rectangle we can
		do the clip directly.  Otherwise we have to create a full
		blown region and call intersect */

		cclip = ((mfbPrivGC*)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
		if (REGION_NUM_RECTS(cclip) == 1)
			{
			BoxPtr pBox = REGION_RECTS(cclip);

			if (fastBox.x1 < pBox->x1)
				fastBox.x1 = pBox->x1;
			if (fastBox.x2 > pBox->x2)
				fastBox.x2 = pBox->x2;
			if (fastBox.y1 < pBox->y1)
				fastBox.y1 = pBox->y1;
			if (fastBox.y2 > pBox->y2)
				fastBox.y2 = pBox->y2;

			/* Check to see if the region is empty */
			if (fastBox.x1 >= fastBox.x2 || fastBox.y1 >= fastBox.y2)
				(*pGC->pScreen->RegionInit)(&rgnDst, NullBox, 0);
			else
				(*pGC->pScreen->RegionInit)(&rgnDst, &fastBox, 1);
			}
		else
			{
			/* We must turn off fastClip now, since we must create
			a full blown region.  It is intersected with the
			composite clip below. */
			fastClip = 0;
			(*pGC->pScreen->RegionInit)(&rgnDst, &fastBox, 1);
			}
		}
	else
		{
		(*pGC->pScreen->TranslateRegion)(&rgnDst, -dx, -dy);
		}

	if (!fastClip)
		{
		(*pGC->pScreen->Intersect)(&rgnDst, 
		    &rgnDst, 
		    ((mfbPrivGC*)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip);
		}

	/* Do bit blitting */
	numRects = REGION_NUM_RECTS(&rgnDst);
	if (numRects)
		{
		if (!(pptSrc = (DDXPointPtr)ALLOCATE_LOCAL(numRects*
		    sizeof(DDXPointRec))))
			{
			(*pGC->pScreen->RegionUninit)(&rgnDst);
			if (freeSrcClip)
				(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
			return NULL;
			}
		pbox = REGION_RECTS(&rgnDst);
		ppt = pptSrc;
		for (i = numRects; --i >= 0; pbox++, ppt++)
			{
			ppt->x = pbox->x1+dx;
			ppt->y = pbox->y1+dy;
			}

		(*doBitBlt)(pSrcDrawable, pDstDrawable, pGC, &rgnDst, pptSrc);
		DEALLOCATE_LOCAL(pptSrc);
		}

	prgnExposed = NULL;
	if (((mfbPrivGC*)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->fExpose)
		{
		/* Pixmap sources generate a NoExposed (we return NULL to do this) */
		if (!fastExpose)
			prgnExposed = 
			    miHandleExposures(pSrcDrawable, pDstDrawable, pGC, 
			    origSource.x, origSource.y, 
			    (int)origSource.width, 
			    (int)origSource.height, 
			    origDest.x, origDest.y, (unsigned long)0);
		}
	(*pGC->pScreen->RegionUninit)(&rgnDst);
	if (freeSrcClip)
		(*pGC->pScreen->RegionDestroy)(prgnSrcClip);
	return prgnExposed;
	} 


void  vgaDoBitBlt(pSrcDrawable, pDstDrawable, pGC, prgnDst, pptSrc)
	DrawablePtr pSrcDrawable;
	DrawablePtr pDstDrawable;
	GC *pGC;
	RegionPtr prgnDst;
	DDXPointPtr pptSrc;
	{
	BoxPtr pbox;
	int nbox;
	BoxPtr pboxBase, pboxNext, pboxTmp, pboxNew;
	DDXPointPtr pptTmp, pptNew;
	int dx, dy;
	int xSrc, ySrc, xDst, yDst, wDst, hDst;
	IMAGE_PTR psrcBase, pdstBase;
	long widthSrc, widthDst;
	int first;

#ifdef SOFTWARE_CURSOR
	ShowCursorFuncPtr ShowCursorFunc = 
	    ((DrawFuncs*)(pGC->pScreen->devPrivate))->ShowCursorFunc;

	HideCursorInXYWHFuncPtr HideCursorInXYWHFunc = 
	    ((DrawFuncs*)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

	pbox = REGION_RECTS(prgnDst);
	nbox = REGION_NUM_RECTS(prgnDst);

	pboxNew = NULL;
	pptNew = NULL;

	if (pSrcDrawable->type == DRAWABLE_PIXMAP && 
	    pDstDrawable->type == DRAWABLE_WINDOW)
		{
		/* Pixmap to Window Blit */
		DrawColorImageFuncPtr DrawColorImageFunc = 
		    ((DrawFuncs*)(pGC->pScreen->devPrivate))->DrawColorImageFunc;
		unsigned int alu = pGC->alu;
		unsigned int pm = (unsigned int)pGC->planemask;

		psrcBase = (IMAGE_PTR)(((PixmapPtr)pSrcDrawable)->devPrivate.ptr);
		widthSrc = (long)((PixmapPtr)pSrcDrawable)->devKind;
		first = TRUE;
		while (nbox-->0)
			{
			xDst = pbox->x1;
			yDst = pbox->y1;
			wDst = pbox->x2-pbox->x1;
			hDst = pbox->y2-pbox->y1;

#ifdef SOFTWARE_CURSOR
			(*HideCursorInXYWHFunc)(xDst, yDst, wDst, hDst);
#endif

			(*DrawColorImageFunc)(pGC->pScreen, xDst, yDst, wDst, 
			    hDst, psrcBase+(pptSrc->y*widthSrc)+pptSrc->x, 
			    (int)widthSrc, alu, pm, first);
			first = FALSE;
			pbox++;
			pptSrc++;
			}

#ifdef SOFTWARE_CURSOR
		(*ShowCursorFunc)();
#endif

		}

	else 
	if (pSrcDrawable->type == DRAWABLE_WINDOW && 
	    pDstDrawable->type == DRAWABLE_PIXMAP)
		{
		/* Window to Pixmap Blit */
		/*
		* XXXXX ALU ignored
		* XXXXX Fix this someday...
		*/
		ReadColorImageFuncPtr ReadColorImageFunc = 
		    ((DrawFuncs*)(pGC->pScreen->devPrivate))->ReadColorImageFunc;
		pdstBase = (IMAGE_PTR)(((PixmapPtr)pDstDrawable)->devPrivate.ptr);
		widthDst = (long)((PixmapPtr)pDstDrawable)->devKind;
		while (nbox-->0)
			{
			xSrc = pptSrc->x;
			ySrc = pptSrc->y;
			xDst = pbox->x1;
			yDst = pbox->y1;
			wDst = pbox->x2-pbox->x1;
			hDst = pbox->y2-pbox->y1;

#ifdef SOFTWARE_CURSOR
			(*HideCursorInXYWHFunc)(xSrc, ySrc, wDst, hDst);
#endif

			(*ReadColorImageFunc)(pGC->pScreen, xSrc, ySrc, wDst, 
			    hDst, pGC->planemask, pdstBase+(yDst*widthDst)+xDst,
			    (int)widthDst);
			pbox++;
			pptSrc++;
			}

#ifdef SOFTWARE_CURSOR
		(*ShowCursorFunc)();
#endif

		}

	else
		{
		/*
		* At this point the drawables are either both pixmaps
		* or both windows.
		*/
		dx = (pbox->x1 > pptSrc->x);	/* dx = (Destination X > Source X) */
		dy = (pbox->y1 > pptSrc->y);	/* dy = (Destination Y > Source Y) */

		if ((nbox > 1) && 
		    ((pSrcDrawable == pDstDrawable) || 
		    (pSrcDrawable->type == DRAWABLE_WINDOW)) && 
		    (dx || dy))
			{
			pboxNew = (BoxPtr)ALLOCATE_LOCAL(sizeof(BoxRec)*nbox);
			if (!pboxNew)
				return;
			pptNew = (DDXPointPtr)ALLOCATE_LOCAL(sizeof(DDXPointRec)*nbox);
			if (!pptNew)
				{
				DEALLOCATE_LOCAL(pboxNew);
				return;
				}
			if (dx && dy)
				{
				/* Destination X greater than Source X
				* Destination Y greater than Source Y
				* Do bands bottom to top, right to left
				* Reverse order of every rect
				*/
				pboxTmp = pbox+nbox;
				pptTmp = pptSrc+nbox;
				while (pboxTmp != pbox)
					{
					*pboxNew++ = *--pboxTmp;
					*pptNew++ = *--pptTmp;
					}
				}
			else 
			if (dx)
				{
				/* Destination X greater than Source X
				* Destination Y less than or equal to Source Y
				* Do bands top to botton, right to left
				* Reverse order of rects in each band
				*/
				pboxBase = pboxNext = pbox;
				while (pboxBase < pbox+nbox)
					{
					while ((pboxNext < pbox+nbox) && 
					    (pboxNext->y1 == pboxBase->y1))
						pboxNext++;
					pboxTmp = pboxNext;
					pptTmp = pptSrc+(pboxTmp-pbox);
					while (pboxTmp != pboxBase)
						{
						*pboxNew++ = *--pboxTmp;
						*pptNew++ = *--pptTmp;
						}
					pboxBase = pboxNext;
					}
				}
			else 
			if (dy)
				{
				/* Destination X less than or equal to Source X
				* Destination Y greater than Source Y
				* Do bands botton to top, left to right
				* Revese order of each band
				*/
				pboxBase = pboxNext = pbox+nbox-1;
				while (pboxBase >= pbox)
					{
					while ((pboxNext >= pbox) && 
					    (pboxBase->y1 == pboxNext->y1))
						pboxNext--;
					pboxTmp = pboxNext+1;
					pptTmp = pptSrc+(pboxTmp-pbox);
					while (pboxTmp <= pboxBase)
						{
						*pboxNew++ = *pboxTmp++;
						*pptNew++ = *pptTmp++;
						}
					pboxBase = pboxNext;
					}
				}
			pboxNew -= nbox;
			pptNew -= nbox;
			pbox = pboxNew;
			pptSrc = pptNew;
			}

		if (pSrcDrawable->type == DRAWABLE_WINDOW)
			{
			/* Window to Window Blit */
			BlitFuncPtr BlitFunc = 
			    ((DrawFuncs*)(pGC->pScreen->devPrivate))->BlitFunc;
			unsigned int alu = pGC->alu;
			unsigned int pm = (unsigned int)pGC->planemask;

			while (nbox-->0)
				{
				xSrc = pptSrc->x;
				ySrc = pptSrc->y;
				xDst = pbox->x1;
				yDst = pbox->y1;
				wDst = pbox->x2-pbox->x1;
				hDst = pbox->y2-pbox->y1;

#ifdef SOFTWARE_CURSOR
				(*HideCursorInXYWHFunc)(xSrc, ySrc, wDst, hDst);
				(*HideCursorInXYWHFunc)(xDst, yDst, wDst, hDst);
#endif

				(*BlitFunc)(pGC->pScreen, xSrc, ySrc, wDst, 
						hDst, xDst, yDst, alu, pm);
				pbox++;
				pptSrc++;
				}

#ifdef SOFTWARE_CURSOR
			(*ShowCursorFunc)();
#endif

			}
		else
			{
			/* Pixmap to Pixmap Blit */
			register IMAGE_PTR psrc, pdst;
			IMAGE_PTR psrcLine, pdstLine;
			int wdstTmp;

			unsigned char alu = pGC->alu;
			unsigned char pm = (unsigned char)pGC->planemask;
			unsigned char npm = ~pm;
			unsigned char allplanes = (unsigned char)((1<<pGC->depth)-1);

			psrcBase = (IMAGE_PTR)(((PixmapPtr)pSrcDrawable)->devPrivate.ptr);
			widthSrc = (long)((PixmapPtr)pSrcDrawable)->devKind;
			pdstBase = (IMAGE_PTR)(((PixmapPtr)pDstDrawable)->devPrivate.ptr);
			widthDst = (long)((PixmapPtr)pDstDrawable)->devKind;

			if (dy)
				{
				widthSrc = -widthSrc;
				widthDst = -widthDst;
				}

			if (alu == GXcopy && (pm&allplanes) == allplanes)
				{
				while (nbox-->0)
					{
					wDst = pbox->x2-pbox->x1;
					hDst = pbox->y2-pbox->y1;

					if (dy)
						{
						/* start at last scanline of rectangle */
						psrcLine = psrcBase+((pptSrc->y+hDst-1)*-widthSrc);
						pdstLine = pdstBase+((pbox->y2-1)*-widthDst);
						}
					else
						{
						/* start at first scanline of rectangle */
						psrcLine = psrcBase+(pptSrc->y*widthSrc);
						pdstLine = pdstBase+(pbox->y1*widthDst);
						}

					if (!dx)
						{
						/* start at first byte of scanline */
						psrcLine += pptSrc->x;
						pdstLine += pbox->x1;
						while (hDst-->0)
							{
							psrc = psrcLine;
							pdst = pdstLine;
							psrcLine += widthSrc;
							pdstLine += widthDst;
							wdstTmp = wDst;
							while (wdstTmp-->0)
								{
								*pdst++ = *psrc++;
								}
							}
						}
					else
						{
						/* start at last byte of scanline */
						psrcLine += pptSrc->x+wDst-1;
						pdstLine += pbox->x2-1;
						while (hDst-->0)
							{
							psrc = psrcLine;
							pdst = pdstLine;
							psrcLine += widthSrc;
							pdstLine += widthDst;
							wdstTmp = wDst;
							while (wdstTmp-->0)
								{
								*pdst-- = *psrc--;
								}
							}
						}
					pbox++;
					pptSrc++;
					}
				}
			else
				{
				unsigned char result;

				while (nbox-->0)
					{
					wDst = pbox->x2-pbox->x1;
					hDst = pbox->y2-pbox->y1;

					if (dy)
						{
						/* start at last scanline of rectangle */
						psrcLine = psrcBase+((pptSrc->y+hDst-1)*-
						    widthSrc);
						pdstLine = pdstBase+((pbox->y2-1)*-widthDst);
						}
					else
						{
						/* start at first scanline of rectangle */
						psrcLine = psrcBase+(pptSrc->y*
						    widthSrc);
						pdstLine = pdstBase+(pbox->y1*widthDst);
						}

					if (!dx)
						{	/* start at first byte of scanline */
						psrcLine += pptSrc->x;
						pdstLine += pbox->x1;
						while (hDst-->0)
							{
							psrc = psrcLine;
							pdst = pdstLine;
							psrcLine += widthSrc;
							pdstLine += widthDst;
							wdstTmp = wDst;
							while (wdstTmp-->0)
								{
								DoRop(result, alu, *psrc++, *
								    pdst);
								*pdst++ = (*pdst&npm)|(
								    result&pm);
								}
							}
						}
					else
						{
						/* start at last byte of scanline */
						psrcLine += pptSrc->x+wDst-1;
						pdstLine += pbox->x2-1;
						while (hDst-->0)
							{
							psrc = psrcLine;
							pdst = pdstLine;
							psrcLine += widthSrc;
							pdstLine += widthDst;
							wdstTmp = wDst;
							while (wdstTmp-->0)
								{
								DoRop(result, alu, *
								    psrc--, *pdst);
								*pdst-- = (*pdst&
								    npm)|(result&
								    pm);
								}
							}
						}
					pbox++;
					pptSrc++;
					}
				}
			}
		}
	/* free up stuff */
	if (pboxNew)
		{
		DEALLOCATE_LOCAL(pboxNew);
		DEALLOCATE_LOCAL(pptNew);
		}
	}


void vgaCopyPlane1to4(pSrcDrawable, pDstDrawable, pGC, prgnDst, pptSrc)
	DrawablePtr pSrcDrawable;
	DrawablePtr pDstDrawable;
	GC *pGC;
	RegionPtr prgnDst;
	DDXPointPtr pptSrc;
	{
	BoxPtr pbox;
	int nbox;
	int xSrc, ySrc, xDst, yDst, wDst, hDst;
	IMAGE_PTR psrcBase;
	long widthSrc;
	IMAGE_PTR pdstBase;
	long widthDst;
	int first;

	pbox = REGION_RECTS(prgnDst);
	nbox = REGION_NUM_RECTS(prgnDst);

	psrcBase = (IMAGE_PTR)(((PixmapPtr)pSrcDrawable)->devPrivate.ptr);
	widthSrc = (long)((PixmapPtr)pSrcDrawable)->devKind;

	if (pDstDrawable->type == DRAWABLE_PIXMAP)
		{
		/* Pixmap to Pixmap Plane Blit */
		IMAGE_UNIT fg = (IMAGE_UNIT) pGC->fgPixel;
		IMAGE_UNIT bg = (IMAGE_UNIT) pGC->bgPixel;

		unsigned char alu = pGC->alu;
		IMAGE_UNIT pm = (IMAGE_UNIT) pGC->planemask;
		IMAGE_UNIT npm = ~pm;

		pdstBase = (IMAGE_PTR)(((PixmapPtr)pDstDrawable)->devPrivate.ptr);
		widthDst = (long)((PixmapPtr)pDstDrawable)->devKind;

		while (nbox-->0)
			{
			IMAGE_PTR pSrc, pDst;
			IMAGE_UNIT tmp;
			int srcInc, dstInc, wTmp;
			IMAGE_UNIT sMask, mask;

			xSrc = pptSrc->x;
			ySrc = pptSrc->y;
			xDst = pbox->x1;
			yDst = pbox->y1;
			wDst = pbox->x2-pbox->x1;
			hDst = pbox->y2-pbox->y1;

			pSrc = psrcBase+(ySrc*widthSrc)+(xSrc>>3);
			pDst = pdstBase+(yDst*widthDst)+xDst;

			srcInc = widthSrc-(((xSrc&0x7)+wDst)>>3);
			dstInc = (int)widthDst-wDst;


#ifdef	notdef	/* tml, lcs, 11/91 */
			sMask = (IMAGE_UNIT)(0x80>>(xSrc&0x7));
#else
			/*
			 * Use a LSB bit order to describe display in connection setup.
			 * Convert to MSB here.
			 */
			sMask = (IMAGE_UNIT)(0x01<<(xSrc&0x7));
#endif
			while (hDst--)
				{
				tmp = *pSrc;
				mask = sMask;
				wTmp = wDst;
				while (wTmp--)
					{
					IMAGE_UNIT result;
					if (tmp&mask)
						{
						DoRop(result, alu, fg, *pDst);
						}
					else
						{
						DoRop(result, alu, bg, *pDst);
						}
					*pDst++ = (*pDst&npm)|(result&pm);
#ifdef	notdef
					if (mask == 0x01)
						{
						pSrc++;
						tmp = *pSrc;
						mask = 0x80;
						}
					else
						{
						mask >>= 1;
						}
					}
#else
					if (mask == 0x80)
						{
						pSrc++;
						tmp = *pSrc;
						mask = 0x01;
						}
					else
						{
						mask <<= 1;
						}
					}
#endif
				pSrc += srcInc;
				pDst += dstInc;
				}

			pbox++;
			pptSrc++;
			}
		}
	else
		{
		/* Pixmap to Window Plane Blit */
		IMAGE_UNIT fg = (IMAGE_UNIT) pGC->fgPixel;
		IMAGE_UNIT bg = (IMAGE_UNIT) pGC->bgPixel;
		unsigned char alu = pGC->alu;
		IMAGE_UNIT pm = (IMAGE_UNIT) pGC->planemask;

		OpaqueStippleFuncPtr OpaqueStippleFunc = 
		    ((DrawFuncs*)(pGC->pScreen->devPrivate))->OpaqueStippleFunc;

#ifdef SOFTWARE_CURSOR
		ShowCursorFuncPtr ShowCursorFunc = 
		    ((DrawFuncs*)(pGC->pScreen->devPrivate))->ShowCursorFunc;

		HideCursorInXYWHFuncPtr HideCursorInXYWHFunc = 
		    ((DrawFuncs*)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

		first = TRUE;
		while (nbox-->0)
			{
			xSrc = pptSrc->x;
			ySrc = pptSrc->y;
			xDst = pbox->x1;
			yDst = pbox->y1;
			wDst = pbox->x2-pbox->x1;
			hDst = pbox->y2-pbox->y1;

#ifdef SOFTWARE_CURSOR
			(*HideCursorInXYWHFunc)(xDst, yDst, wDst, hDst);
#endif

			(*OpaqueStippleFunc)(pGC->pScreen, xSrc, ySrc, wDst, 
			    hDst, psrcBase, widthSrc, 
			    xDst, yDst, first, alu, pm, fg, bg);
			first = FALSE;

#ifdef SOFTWARE_CURSOR
			(*ShowCursorFunc)();
#endif

			pbox++;
			pptSrc++;
			}
		}
	}

RegionPtr vgaCopyPlane(pSrcDrawable, pDstDrawable, 
	pGC, srcx, srcy, width, height, dstx, dsty, bitPlane)
	DrawablePtr pSrcDrawable;
	DrawablePtr pDstDrawable;
	GCPtr pGC;
	int srcx, srcy;
	int width, height;
	int dstx, dsty;
	unsigned long bitPlane;
	{
	RegionPtr ret;
	if (pSrcDrawable->depth == 1 && pDstDrawable->depth != 1)
		{
		if (bitPlane == 1)
			{
			doBitBlt = vgaCopyPlane1to4;

			ret = vgaCopyArea(pSrcDrawable, pDstDrawable, pGC, 
			    srcx, srcy, width, height, dstx, dsty);
			doBitBlt = vgaDoBitBlt;
			}
		else
			{
			/* Copying no planes from a depth-1 pixmap */
			ret = miHandleExposures(pSrcDrawable, pDstDrawable, pGC, 
			    srcx, srcy, width, height, 
			    dstx, dsty, bitPlane);
			}
		}
	else
		{
		/* Pass all other types to MI */
		ret = miCopyPlane(pSrcDrawable, pDstDrawable, 
		    pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
		}
	return ret;
	}

void vgaPutImage(dst, pGC, depth, x, y, w, h, leftPad, format, pImage)
	DrawablePtr dst;
	GCPtr pGC;
	int depth, x, y, w, h;
	int leftPad;
	unsigned int format;
	char *pImage;
	{
	PixmapRec FakePixmap;
	if ((w == 0) || (h == 0))
		return;

	if (format != XYPixmap)
		{
		FakePixmap.drawable.type = DRAWABLE_PIXMAP;
		FakePixmap.drawable.class = 0;
		FakePixmap.drawable.pScreen = dst->pScreen;
		FakePixmap.drawable.depth = (unsigned char)depth;
		FakePixmap.drawable.bitsPerPixel = (unsigned char)depth;
		FakePixmap.drawable.id = 0;
		FakePixmap.drawable.serialNumber = NEXT_SERIAL_NUMBER;
		FakePixmap.drawable.x = 0;
		FakePixmap.drawable.y = 0;
		FakePixmap.drawable.width = w+leftPad;
		FakePixmap.drawable.height = h;
		FakePixmap.devKind = PixmapBytePad(FakePixmap.drawable.width, depth);
		FakePixmap.refcnt = 1;
		FakePixmap.devPrivate.ptr = (pointer)pImage;
		((mfbPrivGC*)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->fExpose = FALSE;
		if (format == ZPixmap)
			(void)(*pGC->ops->CopyArea)(&FakePixmap, dst, pGC, leftPad, 0, 
			    w, h, x, y);
		else
			(void)(*pGC->ops->CopyPlane)(&FakePixmap, dst, pGC, leftPad, 0, 
			    w, h, x, y, 1L);
		((mfbPrivGC*)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->fExpose = TRUE;
		}
	else
		{
		miPutImage(dst, pGC, depth, x, y, w, h, leftPad, format, 
		    (unsigned char*)pImage);
		}
	}

static
void vgaGetZPixImage(pDrawable, sx, sy, w, h, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int sx, sy, w, h;
    unsigned long planeMask;
    pointer pdstLine;

{
    int x, y, widthDst;

    ReadColorImageFuncPtr ReadColorImageFunc = 
	    ((DrawFuncs*)(pDrawable->pScreen->devPrivate))->ReadColorImageFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc = 
	    ((DrawFuncs*)(pDrawable->pScreen->devPrivate))->ShowCursorFunc;

    HideCursorInXYWHFuncPtr HideCursorInXYWHFunc = 
	    ((DrawFuncs*)(pDrawable->pScreen->devPrivate))->HideCursorInXYWHFunc;
#endif

    widthDst = PixmapBytePad(w, pDrawable->depth);

    x = sx+pDrawable->x;
    y = sy+pDrawable->y;

#ifdef SOFTWARE_CURSOR
    (*HideCursorInXYWHFunc)(x, y, w, h);
#endif

    (*ReadColorImageFunc)(pDrawable->pScreen, x, y, w, h, planeMask, 
			  pdstLine, widthDst);

#ifdef SOFTWARE_CURSOR
    (*ShowCursorFunc)();
#endif

}



void vgaGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine)
    DrawablePtr pDrawable;
    int sx, sy, w, h;
    unsigned int format;
    unsigned long planeMask;
    pointer pdstLine;

{
    int x, y, widthDst;
    PixmapPtr tempPixmap;

    if ((w == 0) || (h == 0))
	return;

    if (pDrawable->depth == 1)
    {
	mfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	return;
    }
    if (pDrawable->type == DRAWABLE_PIXMAP)
    {
	cfbGetImage(pDrawable, sx, sy, w, h, format, planeMask, pdstLine);
	return;
    }
    if (pDrawable->type == DRAWABLE_WINDOW)
	if (format == ZPixmap)
	    vgaGetZPixImage(pDrawable, sx, sy, w, h, planeMask, pdstLine);
	
	if (format != XYPixmap)
	    return;

	tempPixmap = vgaCreatePixmap(pDrawable->pScreen, w, h, pDrawable->depth);
	if (!tempPixmap)
	    return;
	vgaGetZPixImage(pDrawable, sx, sy, w, h, planeMask, tempPixmap->devPrivate.ptr);
	cfbGetImage(tempPixmap, 0, 0, w, h, XYPixmap, planeMask, pdstLine);
	vgaDestroyPixmap(tempPixmap);
}
			 
