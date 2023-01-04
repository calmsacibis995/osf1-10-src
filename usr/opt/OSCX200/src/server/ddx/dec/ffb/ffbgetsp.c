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
static char *rcsid = "@(#)$RCSfile: ffbgetsp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:10:36 $";
#endif
/*
 */


#include "X.h"
#include "Xmd.h"
#include "servermd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ffb.h"
#include "ffbgetsp.h"
#include "cfbmfb.h"
#include "cfbgetsp.h"
#include "cfbmskbits.h"

#include "ffbblt.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
#ifdef FFB_DEPTH_INDEPENDENT
void ffbGetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pSrcDraw;	/* drawable from which to get bits  */
    int			wMax;		/* largest value of all *pwidths    */
    register DDXPointPtr ppt;		/* points to start copying from     */
    int			*pwidth;	/* list of number of bits to copy   */
    int			nspans;		/* number of scanlines to copy      */
    unsigned int	*pdstStart;	/* where to put the bits	    */
{
    int row;
    if (pSrcDraw->bitsPerPixel == 1) {
	mfbGetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart);
	return;
    }

    FFB_SELECTROW(pSrcDraw->depth, pSrcDraw->bitsPerPixel,
		  pSrcDraw->depth, row);
    (*ffbCopyTab[row][_GET_SPANS])(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart);
}
#endif


#define MYCFBCOPY(src, pdst)    DCCOPY(src, pdst, 0, 0, -1, 0);

void ffb_GetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart)
    DrawablePtr		pSrcDraw;	/* drawable from which to get bits  */
    int			wMax;		/* largest value of all *pwidths    */
    register DDXPointPtr ppt;		/* points to start copying from     */
    int			*pwidth;	/* list of number of bits to copy   */
    int			nspans;		/* number of scanlines to copy      */
    unsigned int	*pdstStart;	/* where to put the bits	    */
{
    register int	dstAlign;	/* Last few bits of destination ptr */
    register int	srcAlign;       /* last few bits of source ptr      */
    register int	shift;		/* Mostly dstAlign-srcAlign	    */
    register Pixel8	*psrc;		/* pointer to current src longword  */
    register Pixel8	*pdst;		/* pointer to current dst longword  */
    Pixel8		*psrcBase;	/* start of src pixmap		    */
    int			widthSrc;	/* width of pixmap in bytes	    */
    int			widthSrcPixels; /* width of pixmap in pixels	    */
    DDXPointPtr 	pptLast;	/* one past last point to get       */
    int			width;
    Pixel8		*pdstNext;
    DDXPointPtr	  	pptInit;	/* these are for mi backing store */
    int	    	  	*pwidthInit;
    CommandWord		ones = ffbCopyAll1;
    CommandWord		mask, leftMask, rightMask, bits_mask = 0;
    int			m;
    FFB			ffb;

    WRITE_MEMORY_BARRIER();
    DrawableBaseAndWidthPlus(pSrcDraw, psrcBase, widthSrc,
	/* Nothing  */  /* if window */,
	if (MAINMEMORY((PixmapPtr)pSrcDraw)) {
	    CFB_NAME(GetSpans)(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart);
	    return;
	}		/* if pixmap */);

    /* Don't need hardware state check, as just copying data out of ffb. */
    ffb = FFBSCREENPRIV(pSrcDraw->pScreen)->ffb;

    FFBMODE(ffb, COPY);

#if FFBDEPTHBITS==32		/* mask extra bits for 12 and 24 visuals */
    COMPUTE_EXTRA_BITS_MASK(pSrcDraw->depth, bits_mask);
#endif

    pptLast = ppt + nspans;
    pptInit = ppt;
    pwidthInit = pwidth;

    widthSrcPixels = widthSrc / FFBPIXELBYTES;
    pdst = (Pixel8 *) pdstStart;
    while (ppt != pptLast) {
	psrcBase = CYCLE_FB(psrcBase);
	psrc = psrcBase + (ppt->y * widthSrc) + ppt->x * FFBPIXELBYTES; 
	width = *pwidth;
	if (width > widthSrcPixels) width = widthSrcPixels;
	pwidth += 1;

	pdstNext = pdst +
	    PixmapWidthInPadUnits(width,FFBPIXELBITS) * (BITMAP_SCANLINE_PAD/8);

	CONJUGATE_FORWARD_ARGUMENTS(psrc,pdst,srcAlign,dstAlign,shift,
				    width,leftMask,rightMask,0,0,
				    FFBCOPYALL1,FFBMASKEDCOPYPIXELSMASK);
	CYCLE_REGS(ffb);
	FFBSHIFT(ffb, shift);

	if (width <= FFBCOPYBITS) {
	    /* The mask fits into a single word */
	    mask = leftMask & rightMask;
	    COPY_ONE_SCRMEM(ffb,psrc,pdst,rightMask,mask, bits_mask,0,0);
	} else {
	    /* Mask requires multiple words */
	    COPY_MULTIPLE_SCRMEM(ffb, psrc, pdst, width, 0, 0,
				 leftMask, rightMask, bits_mask);
	} /* if small copy else big copy */

        ppt++;
	pwidth++;
	pdst = pdstNext;
    }
}

/*
 * HISTORY
 */

