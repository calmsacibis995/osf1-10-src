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

/* $XConsortium: cfbzerarc.c,v 5.19 91/12/19 14:17:28 keith Exp $ */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#include "X.h"
#include "Xprotostr.h"
#include "miscstruct.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "mizerarc.h"
#include "cfbrrop.h"

#ifdef PIXEL_ADDR

extern void miPolyArc(), miZeroPolyArc();

static void
RROP_NAME(cfbZeroArcSS8) (pDraw, pGC, arc)
    DrawablePtr pDraw;
    GCPtr pGC;
    xArc *arc;
{
    miZeroArcRec info;
    Bool do360;
    register int x;
    PixelType *addrp;
    register PixelType *yorgp, *yorgop;
    RROP_DECLARE
    register int yoffset;
    int npwidth, dyoffset;
    register int y, a, b, d, mask;
    register int k1, k3, dx, dy;

    cfbGetPixelWidthAndPointer(pDraw,npwidth, addrp)

    RROP_FETCH_GC (pGC);
    do360 = miZeroArcSetup(arc, &info, TRUE);
    yorgp = addrp + ((info.yorg + pDraw->y) * npwidth);
    yorgop = addrp + ((info.yorgo + pDraw->y) * npwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    MIARCSETUP();
    yoffset = y ? npwidth : 0;
    dyoffset = 0;
    mask = info.initialMask;
    if (!(arc->width & 1))
    {
	if (mask & 2)
	    RROP_SOLID((yorgp + info.xorgo));
	if (mask & 8)
	    RROP_SOLID((yorgop + info.xorgo));
    }
    if (!info.end.x || !info.end.y)
    {
	mask = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	register int xoffset = npwidth;
	PixelType *yorghb = yorgp + (info.h * npwidth) + info.xorg;
	PixelType *yorgohb = yorghb - info.h;

	yorgp += info.xorg;
	yorgop += info.xorg;
	yorghb += info.h;
	while (1)
	{
	    RROP_SOLID(yorgp + yoffset + x);
	    RROP_SOLID(yorgp + yoffset - x);
	    RROP_SOLID(yorgop - yoffset - x);
	    RROP_SOLID(yorgop - yoffset + x);
	    if (a < 0)
		break;
	    RROP_SOLID(yorghb - xoffset - y);
	    RROP_SOLID(yorgohb - xoffset + y);
	    RROP_SOLID(yorgohb + xoffset + y);
	    RROP_SOLID(yorghb + xoffset - y);
	    xoffset += npwidth;
	    MIARCCIRCLESTEP(yoffset += npwidth;);
	}
	yorgp -= info.xorg;
	yorgop -= info.xorg;
	x = info.w;
	yoffset = info.h * npwidth;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = npwidth;);
	    RROP_SOLID(yorgp + yoffset + info.xorg + x);
	    RROP_SOLID(yorgp + yoffset + info.xorgo - x);
	    RROP_SOLID(yorgop - yoffset + info.xorgo - x);
	    RROP_SOLID(yorgop - yoffset + info.xorg + x);
	    MIARCSTEP(yoffset += dyoffset;, yoffset += npwidth;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = npwidth;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		mask = info.start.mask;
		info.start = info.altstart;
	    }
	    if (mask & 1)
		RROP_SOLID(yorgp + yoffset + info.xorg + x);
	    if (mask & 2)
		RROP_SOLID(yorgp + yoffset + info.xorgo - x);
	    if (mask & 4)
		RROP_SOLID(yorgop - yoffset + info.xorgo - x);
	    if (mask & 8)
		RROP_SOLID(yorgop - yoffset + info.xorg + x);
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		mask = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(yoffset += dyoffset;, yoffset += npwidth;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	mask = info.start.mask;
    if (mask & 1)
	RROP_SOLID(yorgp + yoffset + info.xorg + x);
    if (mask & 4)
	RROP_SOLID(yorgop - yoffset + info.xorgo - x);
    if (arc->height & 1)
    {
	if (mask & 2)
	    RROP_SOLID(yorgp + yoffset + info.xorgo - x);
	if (mask & 8)
	    RROP_SOLID(yorgop - yoffset + info.xorg + x);
    }
}

void
RROP_NAME (cfbZeroPolyArcSS8) (pDraw, pGC, narcs, parcs)
    register DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;

    cclip = ((cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr))->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miCanZeroArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if ((*pDraw->pScreen->RectIn)(cclip, &box) == rgnIN)
		RROP_NAME (cfbZeroArcSS8) (pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}
#endif
