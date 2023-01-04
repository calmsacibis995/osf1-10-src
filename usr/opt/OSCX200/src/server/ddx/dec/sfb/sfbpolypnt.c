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
#define IDENT "X-5"
#define MODULE_NAME SFBPOLYPNT
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

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/sfb/sfbpolypnt.c,v 1.1.7.2 1994/01/19 14:06:31 Don_Haney Exp $ */
/*
**++
**  FACILITY:
**
**      DDXSFB - VMS SFB server
**
**  ABSTRACT:
**
**      
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**
**  CREATION DATE:     20-Nov-1991
**
**  MODIFICATION HISTORY:
**
**
** X-5		BIM0011		Irene McCartney			27-Jan-1992
**		Merge changes from Joel
**
** X-4		TLB0004		Tracy Bragdon 			05-Dec-1991
**		Again, change FFS to FFSS to avoid collision with clib.olb
**
** X-3		TLB0003		Tracy Bragdon 			03-Dec-1991
**		Fix typo - ffs vs FFS
**
** X-2		BIM0009		Irene McCartney			02-Dec-1991
** 		Change routine ffs to FFS for VMS since the first is the name 
**		of a builtit.
**		Add edit history
**--
**/
#include "X.h"
#include "Xprotostr.h"

#include "pixmapstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "scrnintstr.h"

#include "sfb.h"
#include "sfbpolypnt.h"

#ifdef VMS
int FFSS(word)
    unsigned word;
{
    int retval = 0;
    if (word != 0) {
	while ((word & 0xff) == 0) {
	    retval += 8;
	    word >>= 8;
	}
	retval++;
	while ((word & 0x1) == 0) {
	    retval++;
	    word >>= 1;
	}
    }
    return retval;
}
#endif

void sfbPolyPoint(pDraw, pGC, mode, npt, pptInit)
    DrawablePtr 	pDraw;
    GCPtr 		pGC;
    int 		mode;		/* Origin or Previous */
    int			npt;
    xPoint		*pptInit;
{
    register int	dstwidth;   /* width of drawable		    */
    register int	shift, shift2;
    register Pixel8     *pdstBase;  /* pointer to start pixel of drawable   */
    register Pixel8     *pdst;      /* pointer to point pixel 		    */
    cfbPrivGC		*gcPriv;
    RegionPtr		prgnClip;
    BoxPtr		pbox;
    int			nbox;
    register xPoint	*ppt, *pptEnd;
    sfbScreenPrivPtr    scrPriv;
    SFB			sfb;
    int			signbits = 0x80008000;
#ifdef PARTIALWRITES
    PixelWord		foreground;
#endif
    if (npt == 0) return;

    gcPriv      = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    prgnClip    = gcPriv->pCompositeClip;
    nbox	= REGION_NUM_RECTS(prgnClip);
    if (nbox == 0) return;
    pbox	= REGION_RECTS(prgnClip);

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);

    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef PARTIALWRITES
    SFBMODE(sfb, SIMPLE);
    foreground = pGC->fgPixel;
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif

    pptEnd = pptInit + npt;

    /* make pointlist origin relative */
    if (mode == CoordModePrevious) {
	register int    offsetx = 0;
	register int    offsety = 0;

        ppt = pptInit;
	do {
	    ppt->x = (offsetx += ppt->x);
	    ppt->y = (offsety += ppt->y);
	    ppt++;
	} while (ppt != pptEnd);
    }

    shift = FFSS(dstwidth) - 1;

    if ((dstwidth & (dstwidth - 1)) == 0) {
	/* Oh so gross.  We can use a shift instead of a multiply */
	pptEnd--;
	do { /* While clip boxes */
	    register int clipul, cliplr;
	    register int x, y, yx;
    
	    yx     = ((int *)&pDraw->x)[0];
	    yx     -= (yx & 0x8000) << 1;
	    clipul = ((int *)pbox)[0] - yx;
	    cliplr = ((int *)pbox)[1] - yx - 0x00010001;

	    ppt = pptInit;
	    x = ppt->x;
	    while (ppt != pptEnd) {
		y = ppt->y;
		yx = ((int *)ppt)[0];
		y <<= shift;
		if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		    pdstBase = CYCLE_FB(pdstBase);
		    pdst = pdstBase + (pDraw->y * dstwidth) + y 
			   + (x + pDraw->x) * SFBPIXELBYTES;
		    SFBPOINT(pdst, foreground);
		}
		ppt++;
		x = ppt->x;
	    };
	    /* Do last dot */
	    y = ppt->y;
	    yx = ((int *)ppt)[0];
	    y <<= shift;
	    if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		pdstBase = CYCLE_FB(pdstBase);
		pdst = pdstBase + (pDraw->y * dstwidth) + y
		       + (x + pDraw->x) * SFBPIXELBYTES;
		SFBPOINT(pdst, foreground);
	    }

	    pbox++;
	    nbox--;
	} while (nbox > 0);
	return;
    }

    shift2 = dstwidth - (1 << shift);
    if ((shift2 & (shift2 - 1)) == 0) {
	/* Even grosser: We can use two shifts and an add. */
	shift2 = FFSS(shift2) - 1;
	pptEnd--;
	do { /* While clip boxes */
	    register int clipul, cliplr;
	    register int x, y, yx;
    
	    yx     = ((int *)&pDraw->x)[0];
	    yx     -= (yx & 0x8000) << 1;
	    clipul = ((int *)pbox)[0] - yx;
	    cliplr = ((int *)pbox)[1] - yx - 0x00010001;

	    ppt = pptInit;
	    x = ppt->x;
	    while (ppt != pptEnd) {
		y = ppt->y;
		yx = ((int *)ppt)[0];
		if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		    pdstBase = CYCLE_FB(pdstBase);
		    pdst = pdstBase + (pDraw->y * dstwidth) + (y << shift) 
			   + (y << shift2) + (x + pDraw->x) * SFBPIXELBYTES;
		    SFBPOINT(pdst, foreground);
		}
		ppt++;
		x = ppt->x;
	    };
	    /* Do last dot */
	    y = ppt->y;
	    yx = ((int *)ppt)[0];
	    if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		pdstBase = CYCLE_FB(pdstBase);
		pdst = pdstBase + (pDraw->y * dstwidth) + (y << shift) 
		       + (y << shift2) + (x + pDraw->x) * SFBPIXELBYTES;
		SFBPOINT(pdst, foreground);
	    }

	    pbox++;
	    nbox--;
	} while (nbox > 0);
	return;
    }

    /* dstwidth not a power of two */
    pptEnd--;
    do { /* While clip boxes */
	register int clipul, cliplr;
	register int x, y, yx, ymul;

	yx     = ((int *)&pDraw->x)[0];
	yx     -= (yx & 0x8000) << 1;
	clipul = ((int *)pbox)[0] - yx;
	cliplr = ((int *)pbox)[1] - yx - 0x00010001;

	ppt = pptInit;
	x = ppt->x;
	while (ppt != pptEnd) {
	    y = ppt->y;
	    ymul = y * dstwidth;
	    yx = ((int *)ppt)[0];
	    if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
		pdstBase = CYCLE_FB(pdstBase);
		pdst = pdstBase + (pDraw->y * dstwidth) + ymul 
		       + (x + pDraw->x) * SFBPIXELBYTES;
		SFBPOINT(pdst, foreground);
	    }
	    ppt++;
	    x = ppt->x;
	} while (ppt != pptEnd);
	/* Do last dot */
	y = ppt->y;
	ymul = y * dstwidth;
	yx = ((int *)ppt)[0];
	if ((((yx - clipul) | (cliplr - yx)) & signbits) == 0) {
	    pdstBase = CYCLE_FB(pdstBase);
	    pdst = pdstBase + (pDraw->y * dstwidth) + ymul
		   + (x + pDraw->x) * SFBPIXELBYTES;
	    SFBPOINT(pdst, foreground);
	}
	pbox++;
	nbox--;
    } while (nbox > 0);
}


