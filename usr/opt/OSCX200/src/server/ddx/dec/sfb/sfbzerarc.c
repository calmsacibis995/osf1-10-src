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
#define IDENT "X-2"
#define MODULEME SFBZERARC
#include "module_ident.h"
#endif
/************************************************************
Copyright 1989 by The Massachusetts Institute of Technology

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of MIT not be used in
advertising or publicity pertaining to distribution of the
software without specific prior written permission.
M.I.T. makes no representation about the suitability of
this software for any purpose. It is provided "as is"
without any express or implied warranty.

********************************************************/

/****************************************************************************
**                                                                          *
**                       COPYRIGHT (c) 1990, 1991 BY                        *
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


/* $XConsortium: cfbzerarc.c,v 5.16 89/11/25 15:22:46 rws Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "sfb.h"
#include "mizerarc.h"
#include "sfbzerarc.h"
#include "sfbcirclebits.h"

extern void miPolyArc(), miZeroPolyArc();

#define FULLCIRCLE (360 * 64)

#define SFBPOINT_CYCLE(pdst, foreground)	    	\
	SFBPOINT(CYCLE_FB_GLOBAL(pdst), forground)


void sfbZeroPolyArc(pDraw, pGC, narcs, parcs)
    register DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    xArc	*arc;
    int		i;
    BoxRec	box;
    RegionPtr   cclip;
    Pixel8      *addrb;
    int		nlwidth;
    int		xorg, yorg;
    cfbPrivGC   *gcPriv;
    sfbScreenPrivPtr scrPriv;
    SFB		sfb;
#ifdef PARTIALWRITES
    PixelWord   foreground;
#endif
    CYCLE_FB_GLOBAL_DECL;

    xorg = pDraw->x;
    yorg = pDraw->y;
    DrawableBaseAndWidth(pDraw, addrb, nlwidth);
    CHECKSTATE(pDraw->pScreen, scrPriv, sfb, pGC);
#ifdef PARTIALWRITES
    SFBMODE(sfb, SIMPLE);
    foreground = pGC->fgPixel;
#else
    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif

    gcPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    cclip = gcPriv->pCompositeClip;

    for (arc = parcs, i = narcs; --i >= 0; arc++) {
	if (miCanZeroArc(arc)) {
	    box.x1 = arc->x + xorg;
	    box.y1 = arc->y + yorg;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN) {
		miZeroArcRec info;
		Bool do360;
		register int x;
		register Pixel8 *yorgb, *yorgob;
		register int yoffset;
		int dyoffset;
		register int y, a, b, d, mask;
		register int k1, k3, dx, dy;
	    
		if (arc->width == arc->height && arc->width < SFBBUSBITS &&
		    (arc->angle2 >= FULLCIRCLE || arc->angle2 <= -FULLCIRCLE)) {
		    int j, align, rightShift;
		    Pixel8 *pdst, *pdst2;
		    CommandWord *psrc;
		    CommandWord srcbits, srcbits2;

		    /* Paint precomputed image from table */
		    SFBMODE(sfb, TRANSPARENTSTIPPLE);
		    j = arc->width;
		    pdst = addrb + box.y1 * nlwidth + box.x1 * SFBPIXELBYTES;
		    psrc = zeroCircles[j];
		    align = (int)pdst & SFBALIGNMASK;
		    pdst -= align;
		    /* Always keep pdst2 on a different fb alias
		     * to prevent collisions with pdst
		     */
		    pdst2 = CYCLE_FB(pdst) + j*nlwidth;
		    SFBBYTESTOPIXELS(align);
		    if (j + align < SFBSTIPPLEBITS) {
			while (j > 0) {
			    srcbits = psrc[0] << align;
			    SFBWRITE(pdst, srcbits);
			    SFBWRITE(pdst2, srcbits);
			    pdst += nlwidth;
			    pdst2 -= nlwidth;
			    psrc++;
			    j -= 2;
			}
			if (j == 0) {
			    SFBWRITE(pdst, psrc[0] << align);
			}
#if SFBSTIPPLEBITS < SFBBUSBITS 
		    } else if (j + align < 2 * SFBSTIPPLEBITS) {
#else
		    } else {
#endif
			rightShift = SFBSTIPPLEBITS - align;
			while (j > 0) {
			    srcbits = psrc[0];
			    srcbits2 = srcbits >> rightShift;
			    srcbits <<= align;
			    SFBWRITE(pdst, srcbits);
			    SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits2);
			    SFBWRITE(pdst2, srcbits);
			    SFBWRITE(pdst2+SFBSTIPPLEBYTESDONE, srcbits2);
			    pdst += nlwidth;
			    pdst2 -= nlwidth;
			    psrc++;
			    j -= 2;
			}
			if (j == 0) {
			    srcbits = psrc[0];
			    SFBWRITE(pdst, srcbits << align);
			    SFBWRITE(pdst+SFBSTIPPLEBYTESDONE,
				srcbits >> rightShift);
			}
#if SFBSTIPPLEBITS < SFBBUSBITS
		    } else {
			/* Lots of writes. */
			Pixel8  *pd, *pd2;
			CommandWord tsrcbits;
    
			rightShift = SFBSTIPPLEBITS - align;
			while (j > 0) {
			    srcbits = psrc[0];
			    srcbits2 = srcbits >> rightShift;
			    srcbits <<= align;
			    SFBWRITE(pdst, srcbits);
			    SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits2);
			    pd = pdst + 2*SFBSTIPPLEBYTESDONE;
			    pd2 = pdst2 + 2*SFBSTIPPLEBYTESDONE;
			    tsrcbits = srcbits2 >> SFBSTIPPLEBITS;
			    while (tsrcbits != 0) {
				SFBWRITE(pd, tsrcbits);
				SFBWRITE(pd2, tsrcbits);
				pd += SFBSTIPPLEBYTESDONE;
				pd2 += SFBSTIPPLEBYTESDONE;
				tsrcbits >>= SFBSTIPPLEBITS;
			    }
			    SFBWRITE(pdst2, srcbits);
			    SFBWRITE(pdst2+SFBSTIPPLEBYTESDONE, srcbits2);
				
			    pdst += nlwidth;
			    pdst2 -= nlwidth;
			    psrc++;
			    j -= 2;
			}
			if (j == 0) {
			    srcbits = psrc[0];
			    SFBWRITE(pdst, srcbits << align);
			    srcbits >>= rightShift;
			    SFBWRITE(pdst+SFBSTIPPLEBYTESDONE, srcbits);
			    srcbits >>= SFBSTIPPLEBITS;
			    do {
				pdst += SFBSTIPPLEBYTESDONE;
				SFBWRITE(pdst + SFBSTIPPLEBYTESDONE, srcbits);
				srcbits >>= SFBSTIPPLEBITS;
			    } while (srcbits != 0);
			}
#endif
		    }
		    continue;
		}

#ifdef PARTIALWRITES
		SFBMODE(sfb, SIMPLE);
#else
		SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
		do360 = miZeroArcSetup(arc, &info, TRUE);
		yorgb = addrb + ((info.yorg + yorg) * nlwidth);
		yorgob = addrb + ((info.yorgo + yorg) * nlwidth);
		info.xorg += xorg;
		info.xorgo += xorg;
		MIARCSETUP();
		yoffset = y ? nlwidth : 0;
		dyoffset = 0;
		mask = info.initialMask;
		if (!(arc->width & 1))
		{
		    if (mask & 2)
			SFBPOINT_CYCLE(yorgb + info.xorgo*SFBPIXELBYTES, foreground);
		    if (mask & 8)
			SFBPOINT_CYCLE(yorgob + info.xorgo*SFBPIXELBYTES, foreground);
		}
		if (!info.end.x || !info.end.y)
		{
		    mask = info.end.mask;
		    info.end = info.altend;
		}
		if (do360 && (arc->width == arc->height) &&
		    !(arc->width & 1))
		{
		    register int xoffset = nlwidth;
		    Pixel8 *yorghb = yorgb + info.h*nlwidth
				+ info.xorg*SFBPIXELBYTES;
		    Pixel8 *yorgohb = yorghb - info.h*SFBPIXELBYTES;

		    yorgb += info.xorg*SFBPIXELBYTES;
		    yorgob += info.xorg*SFBPIXELBYTES;
		    yorghb += info.h*SFBPIXELBYTES;

		    while (1)
		    {
			SFBPOINT_CYCLE(yorgb + yoffset + x*SFBPIXELBYTES, foreground);
			SFBPOINT_CYCLE(yorgb + yoffset - x*SFBPIXELBYTES, foreground);
			SFBPOINT_CYCLE(yorgob - yoffset - x*SFBPIXELBYTES,foreground);
			SFBPOINT_CYCLE(yorgob - yoffset + x*SFBPIXELBYTES,foreground);
			if (a < 0)
			    break;
			SFBPOINT_CYCLE(yorghb - xoffset - y*SFBPIXELBYTES,foreground);
			SFBPOINT_CYCLE(yorgohb - xoffset + y*SFBPIXELBYTES,foreground);
			SFBPOINT_CYCLE(yorgohb + xoffset + y*SFBPIXELBYTES,foreground);
			SFBPOINT_CYCLE(yorghb + xoffset - y*SFBPIXELBYTES,foreground);
			xoffset += nlwidth;
			MIARCCIRCLESTEP(yoffset += nlwidth;);
		    }
		    yorgb -= info.xorg*SFBPIXELBYTES;
		    yorgob -= info.xorg*SFBPIXELBYTES;
		    x = info.w;
		    yoffset = info.h * nlwidth;
		}
		else if (do360)
		{
		    while (y < info.h || x < info.w)
		    {
			MIARCOCTANTSHIFT(dyoffset = nlwidth;);
			SFBPOINT_CYCLE(yorgb + yoffset
			    + (info.xorg + x)*SFBPIXELBYTES, foreground);
			SFBPOINT_CYCLE(yorgb + yoffset
			    + (info.xorgo - x)*SFBPIXELBYTES, foreground);
			SFBPOINT_CYCLE(yorgob - yoffset 
			    + (info.xorgo - x)*SFBPIXELBYTES, foreground);
			SFBPOINT_CYCLE(yorgob - yoffset 
			    + (info.xorg + x)*SFBPIXELBYTES, foreground);
			MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
		    }
		}
		else
		{
		    while (y < info.h || x < info.w)
		    {
			MIARCOCTANTSHIFT(dyoffset = nlwidth;);
			if ((x == info.start.x) || (y == info.start.y))
			{
			    mask = info.start.mask;
			    info.start = info.altstart;
			}
			if (mask & 1)
			    SFBPOINT_CYCLE(yorgb + yoffset 
				+ (info.xorg + x)*SFBPIXELBYTES, foreground);
			if (mask & 2)
			    SFBPOINT_CYCLE(yorgb + yoffset 
				+ (info.xorgo - x)*SFBPIXELBYTES, foreground);
			if (mask & 4)
			    SFBPOINT_CYCLE(yorgob - yoffset 
				+ (info.xorgo - x)*SFBPIXELBYTES, foreground);
			if (mask & 8)
			    SFBPOINT_CYCLE(yorgob - yoffset 
				+ (info.xorg + x)*SFBPIXELBYTES, foreground);
			if ((x == info.end.x) || (y == info.end.y))
			{
			    mask = info.end.mask;
			    info.end = info.altend;
			}
			MIARCSTEP(yoffset += dyoffset;,yoffset += nlwidth;);
		    }
		}
		if ((x == info.start.x) || (y == info.start.y))
		    mask = info.start.mask;
		if (mask & 1)
		    SFBPOINT_CYCLE(yorgb + yoffset 
			+ (info.xorg + x)*SFBPIXELBYTES, foreground);
		if (mask & 4)
		    SFBPOINT_CYCLE(yorgob - yoffset 
			+ (info.xorgo - x)*SFBPIXELBYTES, foreground);
		if (arc->height & 1)
		{
		    if (mask & 2)
			SFBPOINT_CYCLE(yorgb + yoffset 
			    + (info.xorgo - x)*SFBPIXELBYTES, foreground);
		    if (mask & 8)
			SFBPOINT_CYCLE(yorgob - yoffset 
			    + (info.xorg + x)*SFBPIXELBYTES, foreground);
		}

	    } else {
		miZeroPolyArc(pDraw, pGC, 1, arc);
	    }
	} else {
	    miPolyArc(pDraw, pGC, 1, arc);
	}
    }
}

