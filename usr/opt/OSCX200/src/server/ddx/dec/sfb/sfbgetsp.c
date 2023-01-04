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
#ifdef VMS
#define IDENT "X-4"
#define MODULE_NAME SFBGETSP
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991 BY                  *
**              DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.               *
**			     ALL RIGHTS RESERVED                            *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
****************************************************************************/
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**  CREATION DATE:     19-Nov-1991
**
**  MODIFICATION HISTORY:
**
** X-002	BIM0011		Irene McCartney			27-Jan-1992
**		Add edit history
**		Merge latest changes from Joel
**
**--
**/


#include "X.h"
#include "Xmd.h"
#include "servermd.h"

#include "misc.h"
#include "region.h"
#include "gc.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "sfb.h"
#include "sfbgetsp.h"
#include "cfbmfb.h"
#include "cfbgetsp.h"
#include "cfbmskbits.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */

#define MYCFBCOPY(src, pdst)    DCCOPY(src, pdst, 0, 0, -1, 0);

void sfbGetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart)
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
    CommandWord		ones = sfbCopyAll1;
    CommandWord		mask, leftMask, rightMask;
    int			m;
    SFB			sfb;

    switch (pSrcDraw->bitsPerPixel) {
	case 1:
	    mfbGetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart);
	    return;
	case SFBPIXELBITS:
	    break;
	default:
	    FatalError("sfbGetSpans: invalid depth\n");
    }

    DrawableBaseAndWidthPlus(pSrcDraw, psrcBase, widthSrc,
	/* Nothing  */  /* if window */,
	if (MAINMEMORY((PixmapPtr)pSrcDraw)) {
	    cfbGetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart);
	    return;
	}		/* if pixmap */);

    /* Don't need hardware state check, as just copying data out of sfb. */
    sfb = SFBSCREENPRIV(pSrcDraw->pScreen)->sfb;
#ifndef SOFTWARE_MODEL
#if defined(VMS) || defined(__alpha)
    {
#else
    if (ws_cpu == DS_5000) {
#endif
	/* Barf.  We can't effectively use COPY mode to read from the screen,
	   because the 3MAX hardware performs read-around-write, and so may
	   try to read data from the sfb buffer register BEFORE the write to
	   fill those registers has gone out over the TURBOChannel.  So we'll
	   use cfb code instead.  Mike Nielsen assures me that the 3MAX is the
	   only machine that will do this on the MIPS side of the world. */
	SFBFLUSHMODE(sfb, SIMPLE);
	cfbGetSpans(pSrcDraw, wMax, ppt, pwidth, nspans, pdstStart);
	return;
    }
#endif /* SOFTWARE_MODEL */
    SFBMODE(sfb, COPY);

    pptLast = ppt + nspans;
    pptInit = ppt;
    pwidthInit = pwidth;

    widthSrcPixels = widthSrc / SFBPIXELBYTES;
    pdst = (Pixel8 *) pdstStart;
    while (ppt != pptLast) {
	psrc = psrcBase + (ppt->y * widthSrc) + ppt->x * SFBPIXELBYTES; 
	psrcBase = CYCLE_FB(psrcBase);
	width = *pwidth;
	if (width > widthSrcPixels) width = widthSrcPixels;
	pwidth += 1;

	pdstNext = pdst +
#  ifdef SLEAZOID32
	    PixmapWidthInPadUnits(width, 8) * (BITMAP_SCANLINE_PAD/8);
#  else
	    PixmapWidthInPadUnits(width,SFBPIXELBITS) * (BITMAP_SCANLINE_PAD/8);
#  endif

	srcAlign = (int)psrc & SFBALIGNMASK;
	dstAlign = (int)pdst & SFBSLEAZEALIGNMASK;
	shift = dstAlign*SFBSLEAZEMULTIPLIER - srcAlign;
	if (shift < 0) {
	    /* Ooops.  First source word has less data in it than we need
	       to write to destination, so first word written to internal
	       sfb copy buffer will be junk that just primes the pump.
	       Adjust shift and dstAlign to reflect this fact. */
	    shift += SFBALIGNMENT;
	    dstAlign += SFBALIGNMENT/SFBSLEAZEMULTIPLIER;
	}
	SFBSHIFT(sfb, shift);
	psrc -= srcAlign;
	pdst -= dstAlign;
	dstAlign /= SFBSLEAZEPIXELBYTES;
	width += dstAlign;
	leftMask = SFBLEFTCOPYMASK(dstAlign, ones);
	rightMask = SFBRIGHTCOPYMASK(width, ones);
	if (width <= SFBCOPYBITS) {
	    /* The mask fits into a single word */
	    mask = leftMask & rightMask;
	    SFBWRITE(psrc, rightMask);
	    SFBBUFDRAIN(sfb, pdst, mask);
	} else {
	    /* Mask requires multiple words */
	    SFBWRITE(psrc, ones);
	    SFBBUFDRAIN(sfb, pdst, leftMask);
	    for (m = width - 2*SFBCOPYBITS; m > 0; m -= SFBCOPYBITS) {
		psrc += SFBCOPYBYTESDONE;
		pdst += SFBSLEAZEBYTESDONE;
		SFBWRITE(psrc, ones);
		SFBBUFDRAINALL(sfb, psrc);
	    }
	    SFBWRITE(psrc+SFBCOPYBYTESDONE, rightMask);
	    SFBBUFDRAIN(sfb, pdst+SFBSLEAZEBYTESDONE, rightMask);
	} /* if small copy else big copy */

        ppt++;
	pwidth++;
	pdst = pdstNext;
    }
}

