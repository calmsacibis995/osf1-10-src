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
static char *rcsid = "@(#)$RCSfile: ffbzerarc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:19:34 $";
#endif
/*
 */
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
#include "ffb.h"
#include "mizerarc.h"
#include "ffbzerarc.h"
#include "ffbcirclebits.h"

extern void miPolyArc(), miZeroPolyArc();

#define FULLCIRCLE (360 * 64)

#define FFBPOINT_CYCLE(pdst, foreground)	    	\
	FFBPOINT(CYCLE_FB_GLOBAL(pdst), foreground)

#define DoPix(idx, pdst, foreground) \
	if (mask & (1 << idx)) FFBPOINT_CYCLE(pdst, foreground)

void ffbZeroPolyArc(pDraw, pGC, narcs, parcs)
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
    ffbScreenPrivPtr scrPriv;
    FFB		ffb;
#ifdef PARTIALWRITES
    PixelWord   fg;
#endif
    CYCLE_FB_GLOBAL_DECL;

    xorg = pDraw->x;
    yorg = pDraw->y;
    DrawableBaseAndWidth(pDraw, addrb, nlwidth);

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDraw->pScreen, pDraw, scrPriv, ffb, pGC);
#ifdef PARTIALWRITES
    FFBMODE(ffb, SIMPLE);
    fg = pGC->fgPixel;
#else
    FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif

    gcPriv = CFBGCPRIV(pGC);
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
	    
		if (arc->width == arc->height && arc->width < FFBBUSBITS &&
		    (arc->angle2 >= FULLCIRCLE || arc->angle2 <= -FULLCIRCLE)){
		    int j, align, rightShift;
		    Pixel8 *pdst, *pdst2;
		    CommandWord *psrc;
		    CommandWord srcbits, srcbits2;

		    /* Paint precomputed image from table */
		    CYCLE_REGS(ffb);
		    FFBMODE(ffb, TRANSPARENTSTIPPLE);
		    j = arc->width;
		    pdst = CYCLE_FB_GLOBAL(
			    addrb + box.y1 * nlwidth + box.x1 * FFBPIXELBYTES);
		    psrc = zeroCircles[j];
		    align = (long)pdst & FFBSTIPPLEALIGNMASK;
		    pdst -= align;
		    pdst2 = pdst + j*nlwidth;
		    FFBBYTESTOPIXELS(align);
		    if (j + align < FFBSTIPPLEBITS) {
			while (j > 0) {
			    srcbits = psrc[0] << align;
			    FFBWRITE(pdst, srcbits);
			    FFBWRITE(pdst2, srcbits);
			    pdst += nlwidth;
			    pdst2 -= nlwidth;
			    psrc++;
			    j -= 2;
			}
			if (j == 0) {
			    FFBWRITE(pdst, psrc[0] << align);
			}
#if FFBSTIPPLEBITS < FFBBUSBITS 
		    } else if (j + align < 2 * FFBSTIPPLEBITS) {
#else
		    } else {
#endif
			rightShift = FFBSTIPPLEBITS - align;
			while (j > 0) {
			    srcbits = psrc[0];
			    srcbits2 = srcbits >> rightShift;
			    srcbits <<= align;
			    FFBWRITE(pdst, srcbits);
			    FFBWRITE(pdst+FFBSTIPPLEBYTESDONE, srcbits2);
			    FFBWRITE(pdst2, srcbits);
			    FFBWRITE(pdst2+FFBSTIPPLEBYTESDONE, srcbits2);
			    pdst += nlwidth;
			    pdst2 -= nlwidth;
			    psrc++;
			    j -= 2;
			}
			if (j == 0) {
			    srcbits = psrc[0];
			    FFBWRITE(pdst, srcbits << align);
			    FFBWRITE(pdst+FFBSTIPPLEBYTESDONE,
				srcbits >> rightShift);
			}
#if FFBSTIPPLEBITS < FFBBUSBITS
		    } else {
			/* Lots of writes. */
			Pixel8  *pd, *pd2;
			CommandWord tsrcbits;
    
			rightShift = FFBSTIPPLEBITS - align;
			while (j > 0) {
			    srcbits = psrc[0];
			    srcbits2 = srcbits >> rightShift;
			    srcbits <<= align;
			    FFBWRITE(pdst, srcbits);
			    FFBWRITE(pdst+FFBSTIPPLEBYTESDONE, srcbits2);
			    pd = pdst + 2*FFBSTIPPLEBYTESDONE;
			    pd2 = pdst2 + 2*FFBSTIPPLEBYTESDONE;
			    tsrcbits = srcbits2 >> FFBSTIPPLEBITS;
			    while (tsrcbits != 0) {
				FFBWRITE(pd, tsrcbits);
				FFBWRITE(pd2, tsrcbits);
				pd += FFBSTIPPLEBYTESDONE;
				pd2 += FFBSTIPPLEBYTESDONE;
				tsrcbits >>= FFBSTIPPLEBITS;
			    }
			    FFBWRITE(pdst2, srcbits);
			    FFBWRITE(pdst2+FFBSTIPPLEBYTESDONE, srcbits2);
				
			    pdst += nlwidth;
			    pdst2 -= nlwidth;
			    psrc++;
			    j -= 2;
			}
			if (j == 0) {
			    srcbits = psrc[0];
			    FFBWRITE(pdst, srcbits << align);
			    srcbits >>= rightShift;
			    FFBWRITE(pdst+FFBSTIPPLEBYTESDONE, srcbits);
			    srcbits >>= FFBSTIPPLEBITS;
			    do {
				pdst += FFBSTIPPLEBYTESDONE;
				FFBWRITE(pdst + FFBSTIPPLEBYTESDONE, srcbits);
				srcbits >>= FFBSTIPPLEBITS;
			    } while (srcbits != 0);
			}
#endif
		    }
		    continue;
		}
		/* Can't use precomputed bitmap...use algorithm for points */
		CYCLE_REGS(ffb);
#ifdef PARTIALWRITES
		FFBMODE(ffb, SIMPLE);
#else
		FFBMODE(ffb, TRANSPARENTSTIPPLE);
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
		    DoPix(1, yorgb + info.xorgo*FFBPIXELBYTES, fg);
		    DoPix(3, yorgob + info.xorgo*FFBPIXELBYTES, fg);
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
				+ info.xorg*FFBPIXELBYTES;
		    Pixel8 *yorgohb = yorghb - info.h*FFBPIXELBYTES;

		    yorgb += info.xorg*FFBPIXELBYTES;
		    yorgob += info.xorg*FFBPIXELBYTES;
		    yorghb += info.h*FFBPIXELBYTES;

		    while (1)
		    {
			FFBPOINT_CYCLE(yorgb + yoffset + x*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgb + yoffset - x*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgob - yoffset - x*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgob - yoffset + x*FFBPIXELBYTES, fg);
			if (a < 0)
			    break;
			FFBPOINT_CYCLE(yorghb - xoffset - y*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgohb - xoffset + y*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgohb + xoffset + y*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorghb + xoffset - y*FFBPIXELBYTES, fg);
			xoffset += nlwidth;
			MIARCCIRCLESTEP(yoffset += nlwidth;);
		    }
		    yorgb -= info.xorg*FFBPIXELBYTES;
		    yorgob -= info.xorg*FFBPIXELBYTES;
		    x = info.w;
		    yoffset = info.h * nlwidth;
		}
		else if (do360)
		{
		    while (y < info.h || x < info.w)
		    {
			MIARCOCTANTSHIFT(dyoffset = nlwidth;);
			FFBPOINT_CYCLE(yorgb + yoffset
			    + (info.xorg + x)*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgb + yoffset
			    + (info.xorgo - x)*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgob - yoffset 
			    + (info.xorgo - x)*FFBPIXELBYTES, fg);
			FFBPOINT_CYCLE(yorgob - yoffset 
			    + (info.xorg + x)*FFBPIXELBYTES, fg);
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
			DoPix(0, yorgb + yoffset 
				+ (info.xorg+x)*FFBPIXELBYTES, fg);
			DoPix(1, yorgb + yoffset 
				+ (info.xorgo-x)*FFBPIXELBYTES, fg);
			DoPix(2, yorgob - yoffset 
				+ (info.xorgo-x)*FFBPIXELBYTES, fg);
			DoPix(3, yorgob - yoffset 
				+ (info.xorg+x)*FFBPIXELBYTES, fg);
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
		DoPix(0, yorgb + yoffset + (info.xorg+x)*FFBPIXELBYTES, fg);
		DoPix(2, yorgob - yoffset + (info.xorgo-x)*FFBPIXELBYTES, fg);
		if (arc->height & 1)
		{
		    DoPix(1, yorgb + yoffset
			    + (info.xorgo-x)*FFBPIXELBYTES, fg);
		    DoPix(3, yorgob - yoffset
			    + (info.xorg+x)*FFBPIXELBYTES, fg);
		}

	    } else {
		miZeroPolyArc(pDraw, pGC, 1, arc);
		WRITE_MEMORY_BARRIER();
	    }
	} else {
	    miPolyArc(pDraw, pGC, 1, arc);
	    WRITE_MEMORY_BARRIER();
	}
    }
}
		    
typedef struct {
    int skipStart;
    int haveStart;
    DDXPointRec startPt;
    int haveLast;
    int skipLast;
    DDXPointRec endPt;
    int dashIndex;
    int dashOffset;
    int dashIndexInit;
    int dashOffsetInit;
} DashInfo;

#if 0
static void
miZeroArcDashPts(pGC, arc, dinfo, points, maxPts, evenPts, oddPts)
    GCPtr pGC;
    xArc *arc;
    DashInfo *dinfo;
    int maxPts;
    register DDXPointPtr points, *evenPts, *oddPts;
{
    miZeroArcRec info;
    register int x, y, a, b, d, mask;
    register int k1, k3, dx, dy;
    int dashRemaining;
    DDXPointPtr arcPts[4];
    DDXPointPtr startPts[5], endPts[5];
    int deltas[5];
    DDXPointPtr startPt, pt, lastPt, pts;
    int i, j, delta, ptsdelta, seg, startseg;

    for (i = 0; i < 4; i++)
	arcPts[i] = points + (i * maxPts);
    (void)miZeroArcSetup(arc, &info, FALSE);
    MIARCSETUP();
    mask = info.initialMask;
    startseg = info.startAngle / QUADRANT;
    startPt = arcPts[startseg];
    if (!(arc->width & 1))
    {
	DoPix(1, info.xorgo, info.yorg);
	DoPix(3, info.xorgo, info.yorgo);
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    while (y < info.h || x < info.w)
    {
	MIARCOCTANTSHIFT(;);
	if ((x == info.firstx) || (y == info.firsty))
	    startPt = arcPts[startseg];
	if ((x == info.start.x) || (y == info.start.y))
	{
	    mask = info.start.mask;
	    info.start = info.altstart;
	}
	DoPix(0, info.xorg + x, info.yorg + y);
	DoPix(1, info.xorgo - x, info.yorg + y);
	DoPix(2, info.xorgo - x, info.yorgo - y);
	DoPix(3, info.xorg + x, info.yorgo - y);
	if ((x == info.end.x) || (y == info.end.y))
	{
	    mask = info.end.mask;
	    info.end = info.altend;
	}
	MIARCSTEP(;,;);
    }
    if ((x == info.firstx) || (y == info.firsty))
	startPt = arcPts[startseg];
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
    DoPix(0, info.xorg + x, info.yorg + y);
    DoPix(2, info.xorgo - x, info.yorgo - y);
    if (arc->height & 1)
    {
	DoPix(1, info.xorgo - x, info.yorg + y);
	DoPix(3, info.xorg + x, info.yorgo - y);
    }
    for (i = 0; i < 4; i++)
    {
	seg = (startseg + i) & 3;
	pt = points + (seg * maxPts);
	if (seg & 1)
	{
	    startPts[i] = pt;
	    endPts[i] = arcPts[seg];
	    deltas[i] = 1;
	}
	else
	{
	    startPts[i] = arcPts[seg] - 1;
	    endPts[i] = pt - 1;
	    deltas[i] = -1;
	}
    }
    startPts[4] = startPts[0];
    endPts[4] = startPt;
    startPts[0] = startPt;
    if (startseg & 1)
    {
	if (startPts[4] != endPts[4])
	    endPts[4]--;
	deltas[4] = 1;
    }
    else
    {
	if (startPts[0] > startPts[4])
	    startPts[0]--;
	if (startPts[4] < endPts[4])
	    endPts[4]--;
	deltas[4] = -1;
    }
    if (arc->angle2 < 0)
    {
	DDXPointPtr tmps, tmpe;
	int tmpd;

	tmpd = deltas[0];
	tmps = startPts[0] - tmpd;
	tmpe = endPts[0] - tmpd;
	startPts[0] = endPts[4] - deltas[4];
	endPts[0] = startPts[4] - deltas[4];
	deltas[0] = -deltas[4];
	startPts[4] = tmpe;
	endPts[4] = tmps;
	deltas[4] = -tmpd;
	tmpd = deltas[1];
	tmps = startPts[1] - tmpd;
	tmpe = endPts[1] - tmpd;
	startPts[1] = endPts[3] - deltas[3];
	endPts[1] = startPts[3] - deltas[3];
	deltas[1] = -deltas[3];
	startPts[3] = tmpe;
	endPts[3] = tmps;
	deltas[3] = -tmpd;
	tmps = startPts[2] - deltas[2];
	startPts[2] = endPts[2] - deltas[2];
	endPts[2] = tmps;
	deltas[2] = -deltas[2];
    }
    for (i = 0; i < 5 && startPts[i] == endPts[i]; i++)
	;
    if (i == 5)
	return;
    pt = startPts[i];
    for (j = 4; startPts[j] == endPts[j]; j--)
	;
    lastPt = endPts[j] - deltas[j];
    if (dinfo->haveLast &&
	(pt->x == dinfo->endPt.x) && (pt->y == dinfo->endPt.y))
    {
	startPts[i] += deltas[i];
    }
    else
    {
	dinfo->dashIndex = dinfo->dashIndexInit;
	dinfo->dashOffset = dinfo->dashOffsetInit;
    }
    if (!dinfo->skipStart && (info.startAngle != info.endAngle))
    {
	dinfo->startPt = *pt;
	dinfo->haveStart = TRUE;
    }
    else if (!dinfo->skipLast && dinfo->haveStart &&
	     (lastPt->x == dinfo->startPt.x) &&
	     (lastPt->y == dinfo->startPt.y) &&
	     (lastPt != startPts[i]))
	endPts[j] = lastPt;
    if (info.startAngle != info.endAngle)
    {
	dinfo->haveLast = TRUE;
	dinfo->endPt = *lastPt;
    }
    dashRemaining = pGC->dash[dinfo->dashIndex] - dinfo->dashOffset;
    for (i = 0; i < 5; i++)
    {
	pt = startPts[i];
	lastPt = endPts[i];
	delta = deltas[i];
	while (pt != lastPt)
	{
	    if (dinfo->dashIndex & 1)
	    {
		pts = *oddPts;
		ptsdelta = -1;
	    }
	    else
	    {
		pts = *evenPts;
		ptsdelta = 1;
	    }
	    while ((pt != lastPt) && --dashRemaining >= 0)
	    {
		*pts = *pt;
		pts += ptsdelta;
		pt += delta;
	    }
	    if (dinfo->dashIndex & 1)
		*oddPts = pts;
	    else
		*evenPts = pts;
	    if (dashRemaining <= 0)
	    {
		if (++(dinfo->dashIndex) == pGC->numInDashList)
		    dinfo->dashIndex = 0;
		dashRemaining = pGC->dash[dinfo->dashIndex];
	    }
	}
    }
    dinfo->dashOffset = pGC->dash[dinfo->dashIndex] - dashRemaining;
}
#endif

/*
 * HISTORY
 */
