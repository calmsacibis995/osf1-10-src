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
#define IDENT "X-3"
#define MODULE_NAME SFBPOLY
#include "module_ident.h"
#endif

/*
 * $XConsortium: cfbply1rct.c,v 1.6 91/04/10 11:42:16 keith Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "sfb.h"

void sfbFillPolygon(pDrawable, pGC, shape, mode, count, ptsIn)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		count;
    DDXPointPtr	ptsIn;
{
    cfbPrivGCPtr    devPriv;
    int		    nwidth;
    Pixel8	    *addrl, *addr;
    int		    maxy;
    int		    origin;
    register int    vertex1, vertex2;
    int		    c;
    BoxPtr	    extents;
    int		    clip;
    int		    y;
    int		    *vertex1p, *vertex2p;
    int		    *endp;
    int		    x1, x2;
    int		    dx1, dx2;
    int		    dy1, dy2;
    int		    e1, e2;
    int		    step1, step2;
    int		    sign1, sign2;
    int		    h, nmiddle;
    CommandWord         ones = sfbStippleAll1;
    sfbScreenPrivPtr    scrPriv;
    SFB                 sfb;

    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    if (mode == CoordModePrevious 
        || shape != Convex 
	|| REGION_NUM_RECTS(devPriv->pCompositeClip) != 1)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, count, ptsIn);
	return;
    }
    origin = *((int *) &pDrawable->x);
    origin -= (origin & 0x8000) << 1;
    extents = &devPriv->pCompositeClip->extents;
    vertex1 = *((int *) &extents->x1) - origin;
    vertex2 = *((int *) &extents->x2) - origin - 0x00010001;

    if (count < 3) return;

    vertex1p = (int *) ptsIn;
    c = *vertex1p;
    clip = (c - vertex1) | (vertex2 - c);
    maxy = y = Int32ToY(c);
    endp = vertex1p + count;
    vertex2p = (int *) (ptsIn + 1);
    count--;
    do {
	c = *vertex2p;
	clip |= (c - vertex1) | (vertex2 - c);
	c = Int32ToY(c);
	if (c < y) 
	{
	    y = c;
	    vertex1p = vertex2p;
	}
	vertex2p++;
	if (c > maxy)
	    maxy = c;
	count--;
    } while (count);
    if (y == maxy) return;

    if (clip & 0x80008000)
    {
	miFillPolygon (pDrawable, pGC, shape, mode, vertex2p - (int *) ptsIn, ptsIn);
	return;
    }

#define AddrYPlus(a,y)  ((a) + (y) * nwidth)

    DrawableBaseAndWidth(pDrawable, addrl, nwidth);
    addrl = AddrYPlus(addrl,y + pDrawable->y);
    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    origin = pDrawable->x;
    vertex2p = vertex1p;
    vertex2 = vertex1 = *vertex2p++;
    if (vertex2p == endp)
	vertex2p = (int *) ptsIn;
#define Setup(c,x,vertex,dx,dy,e,sign,step) {\
    x = Int32ToX(vertex); \
    if (dy = Int32ToY(c) - y) { \
    	dx = Int32ToX(c) - x; \
	step = 0; \
    	if (dx >= 0) \
    	{ \
	    e = 0; \
	    sign = 1; \
	    if (dx >= dy) {\
	    	step = dx / dy; \
	    	dx = dx % dy; \
	    } \
    	} \
    	else \
    	{ \
	    e = 1 - dy; \
	    sign = -1; \
	    dx = -dx; \
	    if (dx >= dy) { \
		step = - (dx / dy); \
		dx = dx % dy; \
	    } \
    	} \
    } \
    x += origin; \
    vertex = c; \
}

#define Step(x,dx,dy,e,sign,step) {\
    x += step; \
    if ((e += dx) > 0) \
    { \
	x += sign; \
	e -= dy; \
    } \
}
    for (;;)
    {
	if (y == Int32ToY(vertex1))
	{
	    do
	    {
	    	if (vertex1p == (int *) ptsIn)
		    vertex1p = endp;
	    	c = *--vertex1p;
	    	Setup (c,x1,vertex1,dx1,dy1,e1,sign1,step1)
	    } while (y >= Int32ToY(vertex1));
	    h = dy1;
	}
	else
	{
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    h = Int32ToY(vertex1) - y;
	}
	if (y == Int32ToY(vertex2))
	{
	    do
	    {
	    	c = *vertex2p++;
	    	if (vertex2p == endp)
		    vertex2p = (int *) ptsIn;
	    	Setup (c,x2,vertex2,dx2,dy2,e2,sign2,step2)
	    } while (y >= Int32ToY(vertex2));
	    if (dy2 < h)
		h = dy2;
	}
	else
	{
	    Step(x2,dx2,dy2,e2,sign2,step2)
	    if ((c = (Int32ToY(vertex2) - y)) < h)
		h = c;
	}
	/* fill spans for this segment */
	y += h;
	for (;;)
	{
	    nmiddle = x2 - x1;
	    if (nmiddle < 0) {
		SFBSOLIDSPAN(sfb, addrl + x2*SFBPIXELBYTES, -nmiddle, ones);
	    } else if (nmiddle > 0) {
		SFBSOLIDSPAN(sfb, addrl + x1*SFBPIXELBYTES, nmiddle, ones);
	    }
	    if (!--h)
		break;
	    addrl = AddrYPlus (addrl, 1);
	    addrl = CYCLE_FB(addrl);
	    Step(x1,dx1,dy1,e1,sign1,step1)
	    Step(x2,dx2,dy2,e2,sign2,step2)
	}
	if (y == maxy)
	    break;
	addrl = AddrYPlus (addrl, 1);
	addrl = CYCLE_FB(addrl);
    }
}
