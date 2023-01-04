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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbwideline.c,v 1.1.3.9 93/01/22 16:05:05 Jim_Ludwig Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
 * $XConsortium: sfbwideline.c,v 1.41 91/07/18 22:53:33 keith Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * Mostly integer wideline code.  Uses a technique similar to
 * bresenham zero-width lines, except walks an X edge
 */

#include <stdio.h>
#include <math.h>
#include "X.h"
#include "windowstr.h"
#include "gcstruct.h"
#include "miscstruct.h"
#include "sfbwideline.h"

#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "sfb.h"

#if (defined(SVR4) || defined(SYSV) && defined(SYSV386)) && __STDC__
extern double hypot(double, double);
#endif

#ifdef ICEILTEMPDECL
ICEILTEMPDECL
#endif

static void sfbLineArc();

/*
 * polygon filler
 */

void
sfbFillPolyHelper (pDrawable, pGC, pixel, y, overall_height,
		  left, right, left_count, right_count)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    unsigned long   pixel;
    int		y;			/* start y coordinate */
    int		overall_height;		/* height of entire segment */
    PolyEdgePtr	left, right;
    int		left_count, right_count;
{
    register int left_x, left_e;
    int	left_stepx;
    int	left_signdx;
    int	left_dy, left_dx;

    register int right_x, right_e;
    int	right_stepx;
    int	right_signdx;
    int	right_dy, right_dx;

    int	height;
    int	left_height, right_height;
    int		xorg;

    cfbPrivGCPtr	devPriv;
    BoxPtr		extents;
    int			x1, x2;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase;
    int			dstwidth;
    CommandWord         ones = sfbStippleAll1;
    SFB                 sfb;

    DrawableBaseAndWidth(pDrawable, pdstBase, dstwidth);
    sfb = SFBSCREENPRIV(pDrawable->pScreen)->sfb;
    SFBFOREGROUND(sfb, pixel);
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    extents = &devPriv->pCompositeClip->extents;
    clipx1 = extents->x1;
    clipy1 = extents->y1;
    clipx2 = extents->x2;
    clipy2 = extents->y2;
    
    left_height = 0;
    right_height = 0;
    
    y += pDrawable->y;
    pdstBase += y * dstwidth;
    xorg = pDrawable->x;
    while ((left_count || left_height) &&
	   (right_count || right_height) && y < clipy2) {
	MIPOLYRELOADLEFT
	MIPOLYRELOADRIGHT

	height = left_height;
	if (height > right_height)
	    height = right_height;

	left_height -= height;
	right_height -= height;

	while (height > 0 && y < clipy1) {
	    height--;
	    y++;
	    pdstBase += dstwidth;
	    MIPOLYSTEPLEFT
	    MIPOLYSTEPRIGHT
	}

	while (height > 0 && y < clipy2) {
	    x1 = left_x + xorg;
/*	    if (right_x >= left_x) { */
		x2 = right_x + xorg + 1;
		if (x1 < clipx1) x1 = clipx1;
		if (x2 > clipx2) x2 = clipx2;
		if (x1 < x2) {
		    pdstBase = CYCLE_FB(pdstBase);
		    SFBSOLIDSPAN(sfb, pdstBase + x1 * SFBPIXELBYTES, 
			x2 - x1, ones);
		}
/*	    } */
	    height--;
	    y++;
	    pdstBase += dstwidth;
	    MIPOLYSTEPLEFT
	    MIPOLYSTEPRIGHT
	}
    }
}

static void
sfbFillRectPolyHelper (pDrawable, pGC, pixel, x, y, w, h)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    unsigned long   pixel;
    int		x, y, w, h;
{
    int			width, align;
    CommandWord		leftMask, rightMask, mask;
    cfbPrivGCPtr	devPriv;
    BoxPtr		extents;
    int			x2, y2, ymul, m;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase, *pdst, *p;
    int			dstwidth;
    CommandWord         ones = sfbStippleAll1;
    SFB                 sfb;

    DrawableBaseAndWidth(pDrawable, pdstBase, dstwidth);
    sfb = SFBSCREENPRIV(pDrawable->pScreen)->sfb;
    SFBFOREGROUND(sfb, pixel);
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    extents = &devPriv->pCompositeClip->extents;

    clipx1 = extents->x1;
    clipy1 = extents->y1;
    clipx2 = extents->x2;
    clipy2 = extents->y2;

    x += pDrawable->x;
    y += pDrawable->y;
    ymul = y * dstwidth;
    x2 = x + w;
    y2 = y + h;

    if (x < clipx1) x = clipx1;
    if (x2 > clipx2) x2 = clipx2;
    if (y < clipy1) {
	y = clipy1;
	ymul = y * dstwidth;
    }
    if (y2 > clipy2) y2 = clipy2;

    width = w = x2 - x;
    h = y2 - y;

    if (w > 0 && h > 0) {
	pdst = pdstBase + ymul + x * SFBPIXELBYTES;
	align = (int)pdst & SFBALIGNMASK;
	pdst -= align;
	SFBBYTESTOPIXELS(align);
	width += align;
	leftMask = SFBLEFTSTIPPLEMASK(align, ones);
	rightMask = SFBRIGHTSTIPPLEMASK(width, ones);
    
	if (width <= SFBSTIPPLEBITS) {
	    /* Mask fits in one word.  Measurements show that it it fastest
	       to use a single write, and take the TLB misses. */
	    mask = leftMask & rightMask;
	    if (h & 1) {
		SFBWRITE(pdst, mask);
		pdst += dstwidth;
		h--;
	    }
	    while (h != 0) {
		SFBWRITE(pdst, mask);
		pdst += dstwidth;
		SFBWRITE(pdst, mask);
		pdst += dstwidth;
		h -= 2;
	    };
	} else {
	    /* Mask requires multiple words */
	    do {
		p = pdst;
		SFBWRITE(p, leftMask);
		for (m = width - 2*SFBSTIPPLEBITS; m > 0;
		    m -= SFBSTIPPLEBITS) {
		    p += SFBSTIPPLEBYTESDONE;
		    SFBWRITE(p, ones);
		}
		SFBWRITE(p+SFBSTIPPLEBYTESDONE, rightMask);
		pdst += dstwidth;
		h--;
	    } while (h !=0);
	} /* if skinny else fat rectangle */
    }
}

static int
sfbPolyBuildEdge (y0, k, dx, dy, xi, yi, left, edge)
    double	y0;
    double	k;  /* x0 * dy - y0 * dx */
    register int dx, dy;
    int		xi, yi;
    int		left;
    register PolyEdgePtr edge;
{
    int	    x, y, e;
    int	    xady, axady;

    if (dy < 0)
    {
	dy = -dy;
	dx = -dx;
	k = -k;
    }

    y = ICEIL (y0);
    xady = ICEIL (k) + y * dx;

    if (xady <= 0) {
	axady = -xady;
	if (axady < dy) {
	    x = -1;
	    e = xady + dy;
	} else {
	    x = - (axady / dy) - 1;
	    e = xady - x * dy;
	}
    } else {
	if (xady <= dy) {
	    x = 0;
	    e = xady;
	} else {
	    x = (xady - 1) / dy;
	    e = xady - x * dy;
	}
    }

    if (dx >= 0)
    {
	edge->signdx = 1;
	if (dx < dy) {
	    edge->stepx = 0;
	    edge->dx = dx;
	} else {
	    edge->stepx = dx / dy;
	    edge->dx = dx % dy;
	}
    }
    else
    {
	edge->signdx = -1;
	dx = -dx;
	if (dx < dy) {
	    edge->stepx = 0;
	    edge->dx = dx;
	} else {
	    edge->stepx = - (dx / dy);
	    edge->dx = dx % dy;
	}
	e = dy - e + 1;
    }
    edge->dy = dy;
    edge->x = x + left + xi;
    edge->e = e - dy;	/* bias to compare against 0 instead of dy */
    return y + yi;
}

#define StepAround(v, incr, max) (((v) + (incr) < 0) ? (max - 1) : ((v) + (incr) == max) ? 0 : ((v) + (incr)))

static int
sfbPolyBuildPoly (vertices, slopes, count, xi, yi, left, right, pnleft, pnright, h)
    register PolyVertexPtr vertices;
    register PolySlopePtr  slopes;
    int		    count;
    int		    xi, yi;
    PolyEdgePtr	    left, right;
    int		    *pnleft, *pnright;
    int		    *h;
{
    int	    top, bottom;
    double  miny, maxy;
    register int i;
    int	    j;
    int	    clockwise;
    int	    slopeoff;
    register int s;
    register int nright, nleft;
    int	    y, lasty, bottomy, topy;

    /* find the top and bottom of the polygon */
    maxy = miny = vertices[0].y;
    bottom = top = 0;
    if (vertices[1].y < miny) {
	top = 1;
	miny = vertices[1].y;
    }
    if (vertices[1].y >= maxy) {
	bottom = 1;
	maxy = vertices[1].y;
    }
    if (vertices[2].y < miny) {
	top = 2;
	miny = vertices[2].y;
    }
    if (vertices[2].y >= maxy) {
	bottom = 2;
	maxy = vertices[2].y;
    }
    if (count == 4) {
	if (vertices[3].y < miny) {
	    top = 3;
	    miny = vertices[3].y;
	}
	if (vertices[3].y >= maxy) {
	    bottom = 3;
	    maxy = vertices[3].y;
	}
    }
/*
    for (i = 1; i < count; i++)
    {
	if (vertices[i].y < miny)
	{
	    top = i;
	    miny = vertices[i].y;
	}
	if (vertices[i].y >= maxy)
	{
	    bottom = i;
	    maxy = vertices[i].y;
	}
    }
*/
    clockwise = 1;
    slopeoff = 0;

    i = top;
    j = StepAround (top, -1, count);

    if (slopes[j].dy * slopes[i].dx > slopes[i].dy * slopes[j].dx)
    {
	clockwise = -1;
	slopeoff = -1;
    }

    bottomy = ICEIL (maxy) + yi;

    nright = 0;

    s = StepAround (top, slopeoff, count);
    i = top;
    while (i != bottom)
    {
	if (slopes[s].dy != 0)
	{
	    y = sfbPolyBuildEdge (vertices[i].y,
			slopes[s].k,
			slopes[s].dx, slopes[s].dy,
			xi, yi, 0,
			&right[nright]);
	    if (nright != 0)
	    	right[nright-1].height = y - lasty;
	    else
	    	topy = y;
	    nright++;
	    lasty = y;
	}

	i = StepAround (i, clockwise, count);
	s = StepAround (s, clockwise, count);
    }
    if (nright != 0)
	right[nright-1].height = bottomy - lasty;

    slopeoff = -1 - slopeoff;

    nleft = 0;
    s = StepAround (top, slopeoff, count);
    i = top;
    while (i != bottom)
    {
	if (slopes[s].dy != 0)
	{
	    y = sfbPolyBuildEdge (vertices[i].y,
			   slopes[s].k,
		       	   slopes[s].dx,  slopes[s].dy, xi, yi, 1,
		       	   &left[nleft]);
    
	    if (nleft != 0)
	    	left[nleft-1].height = y - lasty;
	    nleft++;
	    lasty = y;
	}
	i = StepAround (i, -clockwise, count);
	s = StepAround (s, -clockwise, count);
    }
    if (nleft != 0)
	left[nleft-1].height = bottomy - lasty;
    *pnleft = nleft;
    *pnright = nright;
    *h = bottomy - topy;
    return topy;
}

static void
sfbLineJoin (pDrawable, pGC, pixel, pLeft, pRight)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    unsigned long   pixel;
    register LineFacePtr pLeft, pRight;
{
    LineFacePtr     pFace;
    double	    mx, my;
    int		    denom;
    PolyVertexRec   vertices[4];
    PolySlopeRec    slopes[4];
    int		    edgecount;
    PolyEdgeRec	    left[4], right[4];
    int		    nleft, nright;
    int		    y, height;
    int		    swapslopes;
    int		    joinStyle = pGC->joinStyle;
    int		    lw = pGC->lineWidth;

    if (joinStyle == JoinRound)
    {
	sfbLineArc(pDrawable, pGC, pixel, pLeft, pRight,
		  (double)0.0, (double)0.0, TRUE);
	return;
    }
    denom = - pLeft->dx * pRight->dy + pRight->dx * pLeft->dy;
    if (denom == 0)
	return;	/* no join to draw */

    swapslopes = 0;
    if (denom > 0)
    {
	pLeft->xa = -pLeft->xa;
	pLeft->ya = -pLeft->ya;
	pLeft->dx = -pLeft->dx;
	pLeft->dy = -pLeft->dy;
    }
    else
    {
	swapslopes = 1;
	pRight->xa = -pRight->xa;
	pRight->ya = -pRight->ya;
	pRight->dx = -pRight->dx;
	pRight->dy = -pRight->dy;
    }

    vertices[0].x = pRight->xa;
    vertices[0].y = pRight->ya;
    slopes[0].dx = -pRight->dy;
    slopes[0].dy =  pRight->dx;
    slopes[0].k = 0;

    vertices[1].x = 0;
    vertices[1].y = 0;
    slopes[1].dx =  pLeft->dy;
    slopes[1].dy = -pLeft->dx;
    slopes[1].k = 0;

    vertices[2].x = pLeft->xa;
    vertices[2].y = pLeft->ya;

    if (joinStyle == JoinMiter)
    {
    	my = (pLeft->dy  * (pRight->xa * pRight->dy - pRight->ya * pRight->dx) -
              pRight->dy * (pLeft->xa  * pLeft->dy  - pLeft->ya  * pLeft->dx ))
		/ (double) denom;
	pFace = pRight;
	if (pLeft->dy != 0) pFace = pLeft;
	mx = pFace->xa +
		(my - pFace->ya) * (double) pFace->dx / (double) pFace->dy;
	/* check miter limit */
	if ((mx * mx + my * my) > (SQSECANT/4.0 * lw * lw))
	    joinStyle = JoinBevel;
    }

    if (joinStyle == JoinMiter) {
	vertices[3].x = mx;
	vertices[3].y = my;
	if (swapslopes)	{
	    slopes[2].dx = -pLeft->dx;
	    slopes[2].dy = -pLeft->dy;
	    slopes[2].k  = -pLeft->k;
	    slopes[3].dx = -pRight->dx;
	    slopes[3].dy = -pRight->dy;
	    slopes[3].k  = -pRight->k;
	} else {
	    slopes[2].dx = pLeft->dx;
	    slopes[2].dy = pLeft->dy;
	    slopes[2].k =  pLeft->k;
	    slopes[3].dx = pRight->dx;
	    slopes[3].dy = pRight->dy;
	    slopes[3].k  = pRight->k;
	}
	edgecount = 4;
    }
    else
    {
	double	scale, dx, dy, adx, ady;

	adx = dx = pRight->xa - pLeft->xa;
	ady = dy = pRight->ya - pLeft->ya;
	if (adx < 0)
	    adx = -adx;
	if (ady < 0)
	    ady = -ady;
	scale = ady;
	if (adx > ady)
	    scale = adx;
	slopes[2].dx = (dx * 65536) / scale;
	slopes[2].dy = (dy * 65536) / scale;
	slopes[2].k = ((pLeft->xa + pRight->xa) * slopes[2].dy -
		       (pLeft->ya + pRight->ya) * slopes[2].dx) / 2.0;
	edgecount = 3;
    }

    y = sfbPolyBuildPoly (vertices, slopes, edgecount, pLeft->x, pLeft->y,
		   left, right, &nleft, &nright, &height);
    sfbFillPolyHelper (pDrawable, pGC, pixel, y, height, left, right, nleft, nright);
}

static void
sfbLineArcI (pDraw, pGC, xorg, yorg)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    int		    xorg, yorg;
{
    register int x, y, e, ex, slw;
    int			x1, x2, y1, y2;
    cfbPrivGCPtr	devPriv;
    BoxPtr		extents;
    int			ymul, align;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase, *pdst;
    int			dstwidth;
    CommandWord         ones = sfbStippleAll1;
    SFB                 sfb;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    sfb = SFBSCREENPRIV(pDraw->pScreen)->sfb;
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    extents = &devPriv->pCompositeClip->extents;

    xorg += pDraw->x;
    yorg += pDraw->y;
    slw = pGC->lineWidth;
    pdstBase += yorg * dstwidth;
    if (slw == 1) {
	if (extents->x1 <= xorg & xorg < extents->x2 &
	    extents->y1 <= yorg & yorg < extents->y2) {
	    pdst = pdstBase + xorg * SFBPIXELBYTES;
	    align = (int)pdst & SFBALIGNMASK;
	    pdst -= align;
	    SFBWRITE(pdst, 1 << align);
	}
	return;
    }
    y = (slw >> 1) + 1;
    if (slw & 1)
	e = - ((y << 2) + 3);
    else
	e = - (y << 3);
    ex = -4;
    x = 0;
    clipx1 = extents->x1;
    clipy1 = extents->y1;
    clipx2 = extents->x2;
    clipy2 = extents->y2;
    ymul = y * dstwidth;
    align = y-1;
    y1 = yorg - align;
    y2 = yorg + align;
    x1 = xorg - align;
    x2 = xorg + align;
    if (clipx1 <= x1 & x2 < clipx2 & clipy1 <= y1 & y2 < clipy2) {
	/* Don't have to clip inside loop, as join completely in clip rect */
	while (y) {
	    e += (y << 3) - 4;
	    while (e >= 0) {
		x++;
		e += (ex = -((x << 3) + 4));
	    }
	    y--;
	    ymul -= dstwidth;
	    slw = (x << 1) + 1;
	    if ((e == ex) && (slw > 1))
		slw--;
	    x1 = xorg - x;
	    pdstBase = CYCLE_FB(pdstBase);
	    SFBSOLIDSPAN(sfb, pdstBase - ymul + x1 * SFBPIXELBYTES, slw, ones);
	    if ((y != 0) && ((slw > 1) || (e != ex))) {
	    	pdstBase = CYCLE_FB(pdstBase);
		SFBSOLIDSPAN(sfb, pdstBase + ymul + x1 * SFBPIXELBYTES,
		    slw, ones);
	    }
	}
    } else {
	while (y)
	{
	    e += (y << 3) - 4;
	    while (e >= 0)
	    {
		x++;
		e += (ex = -((x << 3) + 4));
	    }
	    y--;
	    ymul -= dstwidth;
	    slw = (x << 1) + 1;
	    if ((e == ex) && (slw > 1))
		slw--;
	    y1 = yorg - y;
	    x1 = xorg - x;
	    x2 = x1 + slw;
	    if (clipy1 <= y1 && y1 < clipy2) {
		if (x1 < clipx1) x1 = clipx1;
		if (x2 > clipx2) x2 = clipx2;
		if (x1 < x2) {
	    	    pdstBase = CYCLE_FB(pdstBase);
		    SFBSOLIDSPAN(sfb, pdstBase - ymul + x1 * SFBPIXELBYTES,
			x2 - x1, ones);
		}
	    }
	    if ((y != 0) && ((slw > 1) || (e != ex)))
	    {
		y1 = yorg + y;
		x1 = xorg - x;
		x2 = x1 + slw;
		if (clipy1 <= y1 && y1 < clipy2) {
		    if (x1 < clipx1) x1 = clipx1;
		    if (x2 > clipx2) x2 = clipx2;
		    if (x1 < x2) {
	    	        pdstBase = CYCLE_FB(pdstBase);
			SFBSOLIDSPAN(sfb, pdstBase + ymul + x1 * SFBPIXELBYTES,
			    x2 - x1, ones);
		    }
		}
	    }
	}
    } /* if no clipping else clipping per line */
}

#define CLIPSTEPEDGE(edgey,edge,edgeleft) \
    if (ybase == edgey) \
    { \
	if (edgeleft) \
	{ \
	    if (edge->x > xcl) \
		xcl = edge->x; \
	} \
	else \
	{ \
	    if (edge->x < xcr) \
		xcr = edge->x; \
	} \
	edgey++; \
	edge->x += edge->stepx; \
	edge->e += edge->dx; \
	if (edge->e > 0) \
	{ \
	    edge->x += edge->signdx; \
	    edge->e -= edge->dy; \
	} \
    }

static void
sfbLineArcD (pDraw, pGC, xorg, yorg, 
	    edge1, edgey1, edgeleft1, edge2, edgey2, edgeleft2)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    double	    xorg, yorg;
    PolyEdgePtr	    edge1, edge2;
    int		    edgey1, edgey2;
    Bool	    edgeleft1, edgeleft2;
{
    double radius, x0, y0, el, er, yk, xlk, xrk, k;
    int xbase, ybase, y, boty, xl, xr, xcl, xcr;
    int ymin, ymax;
    Bool edge1IsMin, edge2IsMin;
    int ymin1, ymin2;
    int			x1, x2, y1, y2;
    cfbPrivGCPtr	devPriv;
    BoxPtr		extents;
    int			ymul, align;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase, *pdst;
    int			dstwidth;
    CommandWord         ones = sfbStippleAll1;
    SFB                 sfb;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    sfb = SFBSCREENPRIV(pDraw->pScreen)->sfb;
    devPriv = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    extents = &devPriv->pCompositeClip->extents;

    xbase = floor(xorg);
    x0 = xorg - xbase;
    ybase = ICEIL (yorg);
    y0 = yorg - ybase;
    xbase += pDraw->x;
    ybase += pDraw->y;
    edge1->x += pDraw->x;
    edge2->x += pDraw->x;
    edgey1 += pDraw->y;
    edgey2 += pDraw->y;

    xlk = x0 + x0 + 1.0;
    xrk = x0 + x0 - 1.0;
    yk = y0 + y0 - 1.0;
    radius = ((double)pGC->lineWidth) / 2.0;
    y = floor(radius - y0 + 1.0);
    ybase -= y;
    pdstBase += ybase * dstwidth;
    ymin = ybase;
    ymax = 65536;
    edge1IsMin = FALSE;
    ymin1 = edgey1;
    if (edge1->dy >= 0)
    {
    	if (!edge1->dy)
    	{
	    if (edgeleft1)
	    	edge1IsMin = TRUE;
	    else
	    	ymax = edgey1;
	    edgey1 = 65536;
    	}
    	else
    	{
	    if ((edge1->signdx < 0) == edgeleft1)
	    	edge1IsMin = TRUE;
    	}
    }
    edge2IsMin = FALSE;
    ymin2 = edgey2;
    if (edge2->dy >= 0)
    {
    	if (!edge2->dy)
    	{
	    if (edgeleft2)
	    	edge2IsMin = TRUE;
	    else
	    	ymax = edgey2;
	    edgey2 = 65536;
    	}
    	else
    	{
	    if ((edge2->signdx < 0) == edgeleft2)
	    	edge2IsMin = TRUE;
    	}
    }
    if (edge1IsMin)
    {
	ymin = ymin1;
	if (edge2IsMin && ymin1 > ymin2)
	    ymin = ymin2;
    } else if (edge2IsMin)
	ymin = ymin2;
    el = radius * radius - ((y + y0) * (y + y0)) - (x0 * x0);
    er = el + xrk;
    xl = 1;
    xr = 0;
    if (x0 < 0.5)
    {
	xl = 0;
	el -= xlk;
    }
    boty = (y0 < -0.5) ? 1 : 0;
    if (ybase + y - boty > ymax)
	boty = ymax - ybase - y;
    clipx1 = extents->x1;
    clipy1 = extents->y1;
    clipx2 = extents->x2;
    clipy2 = extents->y2;
    while (y > boty)
    {
	k = (y << 1) + yk;
	er += k;
	while (er > 0.0)
	{
	    xr++;
	    er += xrk - (xr << 1);
	}
	el += k;
	while (el >= 0.0)
	{
	    xl--;
	    el += (xl << 1) - xlk;
	}
	y--;
	ybase++;
	pdstBase += dstwidth;
	if (ybase < ymin)
	    continue;
	xcl = xl + xbase;
	xcr = xr + xbase;
	CLIPSTEPEDGE(edgey1, edge1, edgeleft1);
	CLIPSTEPEDGE(edgey2, edge2, edgeleft2);
/*	if (xcr >= xcl)	{ */
	    if (clipy1 <= ybase && ybase < clipy2) {
		x2 = xcr + 1;
		if (xcl < clipx1) xcl = clipx1;
		if (x2 > clipx2) x2 = clipx2;
		if (xcl < x2) {
		    pdstBase = CYCLE_FB(pdstBase);
		    SFBSOLIDSPAN(sfb, pdstBase + xcl * SFBPIXELBYTES,
			x2 - xcl, ones);
		}
	    }
/*	} */
    }
    er = xrk - (xr << 1) - er;
    el = (xl << 1) - xlk - el;
    boty = floor(-y0 - radius + 1.0);
    if (ybase + y - boty > ymax)
	boty = ymax - ybase - y;
    while (y > boty)
    {
	k = (y << 1) + yk;
	er -= k;
	while ((er >= 0.0) && (xr >= 0))
	{
	    xr--;
	    er += xrk - (xr << 1);
	}
	el -= k;
	while ((el > 0.0) && (xl <= 0))
	{
	    xl++;
	    el += (xl << 1) - xlk;
	}
	y--;
	ybase++;
	pdstBase += dstwidth;
	if (ybase < ymin)
	    continue;
	xcl = xl + xbase;
	xcr = xr + xbase;
	CLIPSTEPEDGE(edgey1, edge1, edgeleft1);
	CLIPSTEPEDGE(edgey2, edge2, edgeleft2);
/*	if (xcr >= xcl)	{ */
	    if (clipy1 <= ybase && ybase < clipy2) {
		x2 = xcr + 1;
		if (xcl < clipx1) xcl = clipx1;
		if (x2 > clipx2) x2 = clipx2;
		if (xcl < x2) {
		    pdstBase = CYCLE_FB(pdstBase);
		    SFBSOLIDSPAN(sfb, pdstBase + xcl * SFBPIXELBYTES,
			x2 - xcl, ones);
		}
	    }
/*	} */
    }
}

sfbRoundJoinFace (face, edge, leftEdge)
    register LineFacePtr face;
    register PolyEdgePtr edge;
    Bool	*leftEdge;
{
    int	    y;
    int	    dx, dy;
    double  ya;
    Bool	left;

    dx = -face->dy;
    dy = face->dx;
    ya = face->ya;
    left = 1;
    if (ya > 0)	ya = 0.0;
    if (dy < 0 || dy == 0 && dx > 0)
    {
	dx = -dx;
	dy = -dy;
	left = !left;
    }
    if (dx == 0 && dy == 0)
	dy = 1;
    if (dy == 0)
    {
	y = ICEIL (face->ya) + face->y;
	edge->x = -32767;
	edge->stepx = 0;
	edge->signdx = 0;
	edge->e = -1;
	edge->dy = 0;
	edge->dx = 0;
	edge->height = 0;
    }
    else
    {
	y = sfbPolyBuildEdge (ya, 0.0, dx, dy, face->x, face->y, !left, edge);
	edge->height = 32767;
    }
    *leftEdge = !left;
    return y;
}

sfbRoundJoinClip (pLeft, pRight, edge1, edge2, y1, y2, left1, left2)
    register LineFacePtr pLeft, pRight;
    PolyEdgePtr	edge1, edge2;
    int		*y1, *y2;
    Bool	*left1, *left2;
{
    int	denom;

    denom = - pLeft->dx * pRight->dy + pRight->dx * pLeft->dy;

    if (denom >= 0)
    {
	pLeft->xa = -pLeft->xa;
	pLeft->ya = -pLeft->ya;
    }
    else
    {
	pRight->xa = -pRight->xa;
	pRight->ya = -pRight->ya;
    }
    *y1 = sfbRoundJoinFace (pLeft, edge1, left1);
    *y2 = sfbRoundJoinFace (pRight, edge2, left2);
}

sfbRoundCapClip (face, isInt, edge, leftEdge)
    register LineFacePtr face;
    Bool	isInt;
    register PolyEdgePtr edge;
    Bool	*leftEdge;
{
    int	    y;
    register int dx, dy;
    double  ya, k;
    Bool	left;

    dx = -face->dy;
    dy = face->dx;
    ya = face->ya;
    k = 0.0;
    if (!isInt)
	k = face->k;
    left = 1;
    if (dy < 0 || dy == 0 && dx > 0)
    {
	dx = -dx;
	dy = -dy;
	ya = -ya;
	left = !left;
    }
    if (dx == 0 && dy == 0)
	dy = 1;
    if (dy == 0)
    {
	y = ICEIL (face->ya) + face->y;
	edge->x = -32767;
	edge->stepx = 0;
	edge->signdx = 0;
	edge->e = -1;
	edge->dy = 0;
	edge->dx = 0;
	edge->height = 0;
    }
    else
    {
	y = sfbPolyBuildEdge (ya, k, dx, dy, face->x, face->y, !left, edge);
	edge->height = 32767;
    }
    *leftEdge = !left;
    return y;
}

static void
sfbLineArc (pDraw, pGC, pixel, leftFace, rightFace, xorg, yorg, isInt)
    DrawablePtr	    pDraw;
    register GCPtr  pGC;
    unsigned long   pixel;
    register LineFacePtr leftFace, rightFace;
    double	    xorg, yorg;
    Bool	    isInt;
{
    int xorgi, yorgi;
    PolyEdgeRec	edge1, edge2;
    int		edgey1, edgey2;
    Bool	edgeleft1, edgeleft2;
    SFB		sfb;

    sfb = SFBSCREENPRIV(pDraw->pScreen)->sfb;
    SFBFOREGROUND(sfb, pixel);
    if (isInt) {
	xorgi = leftFace ? leftFace->x : rightFace->x;
	yorgi = leftFace ? leftFace->y : rightFace->y;
    }
    edgey1 = 65536;
    edgey2 = 65536;
    edge1.dy = -1;
    edge2.dy = -1;
    edgeleft1 = FALSE;
    edgeleft2 = FALSE;
    if ((pGC->lineStyle != LineSolid || pGC->lineWidth > 2) &&
	(pGC->capStyle == CapRound && pGC->joinStyle != JoinRound ||
	 pGC->joinStyle == JoinRound && pGC->capStyle == CapButt))
    {
	if (isInt) {
	    xorg = (double) xorgi;
	    yorg = (double) yorgi;
	}
	if (leftFace && rightFace)
	{
	    sfbRoundJoinClip (leftFace, rightFace, &edge1, &edge2,
			     &edgey1, &edgey2, &edgeleft1, &edgeleft2);
	}
	else if (leftFace)
	{
	    edgey1 = sfbRoundCapClip (leftFace, isInt, &edge1, &edgeleft1);
	}
	else if (rightFace)
	{
	    edgey2 = sfbRoundCapClip (rightFace, isInt, &edge2, &edgeleft2);
	}
	sfbLineArcD(pDraw, pGC, xorg, yorg,
		       &edge1, edgey1, edgeleft1,
		       &edge2, edgey2, edgeleft2);
    } else if (isInt) {
	sfbLineArcI(pDraw, pGC, xorgi, yorgi);
    } else {
	sfbLineArcD(pDraw, pGC, xorg, yorg,
		       &edge1, edgey1, edgeleft1,
		       &edge2, edgey2, edgeleft2);
    }
}

sfbLineProjectingCap (pDrawable, pGC, pixel, face, isLeft, xorg, yorg, isInt)
    DrawablePtr	    pDrawable;
    register GCPtr  pGC;
    unsigned long   pixel;
    register LineFacePtr face;
    Bool	    isLeft;
    double	    xorg, yorg;
    Bool	    isInt;
{
    int	xorgi, yorgi;
    int	lw;
    PolyEdgeRec	lefts[2], rights[2];
    int		lefty, righty, topy, bottomy;
    PolyEdgePtr left, right;
    PolyEdgePtr	top, bottom;
    double	xa,ya;
    double	k;
    double	xap, yap;
    int		dx, dy;
    double	projectXoff, projectYoff;
    double	maxy;
    int		finaly;
    
    if (isInt)
    {
	xorgi = face->x;
	yorgi = face->y;
    }
    lw = pGC->lineWidth;
    dx = face->dx;
    dy = face->dy;
    k = face->k;
    if (dy == 0)
    {
	lefts[0].height = lw;
	lefts[0].x = xorgi;
	if (isLeft)
	    lefts[0].x -= (lw >> 1);
	lefts[0].stepx = 0;
	lefts[0].signdx = 1;
	lefts[0].e = -lw;
	lefts[0].dx = 0;
	lefts[0].dy = lw;
	rights[0].height = lw;
	rights[0].x = xorgi;
	if (!isLeft)
	    rights[0].x += (lw + 1 >> 1);
	rights[0].stepx = 0;
	rights[0].signdx = 1;
	rights[0].e = -lw;
	rights[0].dx = 0;
	rights[0].dy = lw;
	sfbFillPolyHelper (pDrawable, pGC, pixel, yorgi - (lw >> 1), lw,
		     lefts, rights, 1, 1);
    }
    else if (dx == 0)
    {
	topy = yorgi;
	bottomy = yorgi + dy;
	if (isLeft)
	    topy -= (lw >> 1);
	else
	    bottomy += (lw >> 1);
	lefts[0].height = bottomy - topy;
	lefts[0].x = xorgi - (lw >> 1);
	lefts[0].stepx = 0;
	lefts[0].signdx = 1;
	lefts[0].e = -dy;
	lefts[0].dx = dx;
	lefts[0].dy = dy;

	rights[0].height = bottomy - topy;
	rights[0].x = lefts[0].x + (lw-1);
	rights[0].stepx = 0;
	rights[0].signdx = 1;
	rights[0].e = -dy;
	rights[0].dx = dx;
	rights[0].dy = dy;
	sfbFillPolyHelper (pDrawable, pGC, pixel, topy, bottomy - topy, lefts, rights, 1, 1);
    }
    else
    {
	xa = face->xa;
	ya = face->ya;
	projectXoff = -ya;
	projectYoff = xa;
	if (dx < 0)
	{
	    right = &rights[1];
	    left = &lefts[0];
	    top = &rights[0];
	    bottom = &lefts[1];
	}
	else
	{
	    right = &rights[0];
	    left = &lefts[1];
	    top = &lefts[0];
	    bottom = &rights[1];
	}
	if (isLeft)
	{
	    righty = sfbPolyBuildEdge (ya, k, dx, dy, xorgi, yorgi, 0, right);
	    
	    xa = -xa;
	    ya = -ya;
	    k = -k;
	    lefty = sfbPolyBuildEdge (ya - projectYoff,
				     k, dx, dy, xorgi, yorgi, 1, left);
	    if (dx > 0)
	    {
		ya = -ya;
		xa = -xa;
	    }
	    xap = xa - projectXoff;
	    yap = ya - projectYoff;
	    topy = sfbPolyBuildEdge (yap, xap * dx + yap * dy,
				    -dy, dx, xorgi, yorgi, dx > 0, top);
	    bottomy = sfbPolyBuildEdge (ya, 0.0, -dy, dx, xorgi, yorgi,
				    dx < 0, bottom);
	    maxy = -ya;
	}
	else
	{
	    righty = sfbPolyBuildEdge (ya - projectYoff,
		     k, dx, dy, xorgi, yorgi, 0, right);
	    
	    xa = -xa;
	    ya = -ya;
	    k = -k;
	    lefty = sfbPolyBuildEdge (ya, k, dx, dy, xorgi, yorgi, 1, left);
	    if (dx > 0)
	    {
		ya = -ya;
		xa = -xa;
	    }
	    xap = xa - projectXoff;
	    yap = ya - projectYoff;
	    topy = sfbPolyBuildEdge (ya, 0.0, -dy, dx, xorgi, xorgi, dx > 0, top);
	    bottomy = sfbPolyBuildEdge (yap, xap * dx + yap * dy,
				       -dy, dx, xorgi, xorgi, dx < 0, bottom);
	    maxy = -ya + projectYoff;
	}
	finaly = ICEIL (maxy) + yorgi;
	if (dx < 0)
	{
	    left->height = bottomy - lefty;
	    right->height = finaly - righty;
	    top->height = righty - topy;
	}
	else
	{
	    right->height =  bottomy - righty;
	    left->height = finaly - lefty;
	    top->height = lefty - topy;
	}
	bottom->height = finaly - bottomy;
	sfbFillPolyHelper (pDrawable, pGC, pixel, topy,
		     bottom->height + bottomy - topy, lefts, rights, 2, 2);
    }
}

static void
sfbWideSegment (pDrawable, pGC, pixel,
	       x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    unsigned long   pixel;
    register int    x1, y1, x2, y2;
    Bool	    projectLeft, projectRight;
    register LineFacePtr leftFace, rightFace;
{
    double	l, L, r;
    double	xa, ya;
    double	projectXoff, projectYoff;
    double	k;
    double	maxy;
    int		x, y;
    int		dx, dy;
    int		finaly;
    PolyEdgePtr left, right;
    PolyEdgePtr	top, bottom;
    int		lefty, righty, topy, bottomy;
    int		signdx;
    PolyEdgeRec	lefts[2], rights[2];
    LineFacePtr	tface;
    int		lw = pGC->lineWidth;

    /* draw top-to-bottom always */
    if (y2 < y1 || y2 == y1 && x2 < x1)
    {
	x = x1;
	x1 = x2;
	x2 = x;

	y = y1;
	y1 = y2;
	y2 = y;

	x = projectLeft;
	projectLeft = projectRight;
	projectRight = x;

	tface = leftFace;
	leftFace = rightFace;
	rightFace = tface;
    }

    dy = y2 - y1;
    signdx = 1;
    dx = x2 - x1;
    if (dx < 0)
	signdx = -1;

    leftFace->x = x1;
    leftFace->y = y1;
    leftFace->dx = dx;
    leftFace->dy = dy;

    rightFace->x = x2;
    rightFace->y = y2;
    rightFace->dx = -dx;
    rightFace->dy = -dy;

    if (dy == 0)
    {
	rightFace->xa = 0;
	rightFace->ya = (double) lw / 2.0;
	rightFace->k = -(double) (lw * dx) / 2.0;
	leftFace->xa = 0;
	leftFace->ya = -rightFace->ya;
	leftFace->k = rightFace->k;
	x = x1;
	if (projectLeft)
	    x -= (lw >> 1);
	y = y1 - (lw >> 1);
	dx = x2 - x;
	if (projectRight)
	    dx += (lw + 1 >> 1);
	dy = lw;
	sfbFillRectPolyHelper (pDrawable, pGC, pixel, x, y, dx, dy);
    }
    else if (dx == 0)
    {
	leftFace->xa =  (double) lw / 2.0;
	leftFace->ya = 0;
	leftFace->k = (double) (lw * dy) / 2.0;
	rightFace->xa = -leftFace->xa;
	rightFace->ya = 0;
	rightFace->k = leftFace->k;
	y = y1;
	if (projectLeft)
	    y -= lw >> 1;
	x = x1 - (lw >> 1);
	dy = y2 - y;
	if (projectRight)
	    dy += (lw + 1 >> 1);
	dx = lw;
	sfbFillRectPolyHelper (pDrawable, pGC, pixel, x, y, dx, dy);
    }
    else
    {
    	l = ((double) lw) / 2.0;
    	L = hypot ((double) dx, (double) dy);

	if (dx < 0)
	{
	    right = &rights[1];
	    left = &lefts[0];
	    top = &rights[0];
	    bottom = &lefts[1];
	}
	else
	{
	    right = &rights[0];
	    left = &lefts[1];
	    top = &lefts[0];
	    bottom = &rights[1];
	}
	r = l / L;

	/* coord of upper bound at integral y */
	ya = -r * dx;
	xa = r * dy;

	if (projectLeft | projectRight)
	{
	    projectXoff = -ya;
	    projectYoff = xa;
	}

    	/* xa * dy - ya * dx */
	k = l * L;

	leftFace->xa = xa;
	leftFace->ya = ya;
	leftFace->k = k;
	rightFace->xa = -xa;
	rightFace->ya = -ya;
	rightFace->k = k;

	if (projectLeft)
	    righty = sfbPolyBuildEdge (ya - projectYoff,
				      k, dx, dy, x1, y1, 0, right);
	else
	    righty = sfbPolyBuildEdge (ya,
				      k, dx, dy, x1, y1, 0, right);

	/* coord of lower bound at integral y */
	ya = -ya;
	xa = -xa;

	/* xa * dy - ya * dx */
	k = - k;

	if (projectLeft)
	    lefty = sfbPolyBuildEdge (ya - projectYoff,
				     k, dx, dy, x1, y1, 1, left);
	else
	    lefty = sfbPolyBuildEdge (ya,
				     k, dx, dy, x1, y1, 1, left);

	/* coord of top face at integral y */

	if (signdx > 0)
	{
	    ya = -ya;
	    xa = -xa;
	}

	if (projectLeft)
	{
	    double xap = xa - projectXoff;
	    double yap = ya - projectYoff;
	    topy = sfbPolyBuildEdge (yap, xap * dx + yap * dy,
				    -dy, dx, x1, y1, dx > 0, top);
	}
	else
	    topy = sfbPolyBuildEdge (ya, 0.0, -dy, dx, x1, y1, dx > 0, top);

	/* coord of bottom face at integral y */

	if (projectRight)
	{
	    double xap = xa + projectXoff;
	    double yap = ya + projectYoff;
	    bottomy = sfbPolyBuildEdge (yap, xap * dx + yap * dy,
				       -dy, dx, x2, y2, dx < 0, bottom);
	    maxy = -ya + projectYoff;
	}
	else
	{
	    bottomy = sfbPolyBuildEdge (ya, 0.0, 
					-dy, dx, x2, y2, dx < 0, bottom);
	    maxy = -ya;
	}

	finaly = ICEIL (maxy) + y2;

	if (dx < 0)
	{
	    left->height = bottomy - lefty;
	    right->height = finaly - righty;
	    top->height = righty - topy;
	}
	else
	{
	    right->height =  bottomy - righty;
	    left->height = finaly - lefty;
	    top->height = lefty - topy;
	}
	bottom->height = finaly - bottomy;
	sfbFillPolyHelper (pDrawable, pGC, pixel, topy,
		     bottom->height + bottomy - topy, lefts, rights, 2, 2);
    }
}

void
sfbWideLine (pDrawable, pGC, mode, npt, pPts)
    DrawablePtr	pDrawable;
    register GCPtr pGC;
    int		mode;
    register int npt;
    register DDXPointPtr pPts;
{
    int		    x1, y1, x2, y2;
    unsigned long   pixel;
    Bool	    projectLeft, projectRight;
    LineFaceRec	    leftFace, rightFace, prevRightFace;
    LineFaceRec	    firstFace;
    register int    first;
    Bool	    somethingDrawn = FALSE;
    Bool	    selfJoin;
    sfbScreenPrivPtr    scrPriv;
    SFB                 sfb;

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    pixel = pGC->fgPixel;
    x2 = pPts->x;
    y2 = pPts->y;
    first = TRUE;
    selfJoin = FALSE;
    if (npt > 1)
    {
    	if (mode == CoordModePrevious)
    	{
	    int nptTmp;
	    DDXPointPtr pPtsTmp;
    
	    x1 = x2;
	    y1 = y2;
	    nptTmp = npt;
	    pPtsTmp = pPts + 1;
	    while (--nptTmp)
	    {
	    	x1 += pPtsTmp->x;
	    	y1 += pPtsTmp->y;
	    	++pPtsTmp;
	    }
	    if (x2 == x1 && y2 == y1)
	    	selfJoin = TRUE;
    	}
    	else if (x2 == pPts[npt-1].x && y2 == pPts[npt-1].y)
    	{
	    selfJoin = TRUE;
    	}
    }
    projectLeft = pGC->capStyle == CapProjecting && !selfJoin;
    projectRight = FALSE;
    while (--npt)
    {
	x1 = x2;
	y1 = y2;
	++pPts;
	x2 = pPts->x;
	y2 = pPts->y;
	if (mode == CoordModePrevious)
	{
	    x2 += x1;
	    y2 += y1;
	}
	if (x1 != x2 || y1 != y2)
	{
	    somethingDrawn = TRUE;
	    if (npt == 1 && pGC->capStyle == CapProjecting && !selfJoin)
	    	projectRight = TRUE;
	    sfbWideSegment (pDrawable, pGC, pixel, x1, y1, x2, y2,
		       	   projectLeft, projectRight, &leftFace, &rightFace);
	    if (first)
	    {
	    	if (selfJoin)
		    firstFace = leftFace;
	    	else if (pGC->capStyle == CapRound)
		    sfbLineArc (pDrawable, pGC, pixel, 
			       &leftFace, (LineFacePtr) NULL,
 			       (double)0.0, (double)0.0,
			       TRUE);
	    }
	    else
	    {
	    	sfbLineJoin (pDrawable, pGC, pixel, &leftFace,
		            &prevRightFace);
	    }
	    prevRightFace = rightFace;
	    first = FALSE;
	    projectLeft = FALSE;
	}
	if (npt == 1 && somethingDrawn)
 	{
	    if (selfJoin)
		sfbLineJoin (pDrawable, pGC, pixel, &firstFace, &rightFace);
	    else if (pGC->capStyle == CapRound)
		sfbLineArc (pDrawable, pGC, pixel, 
			   (LineFacePtr) NULL, &rightFace,
			   (double)0.0, (double)0.0,
			   TRUE);
	}
    }
    /* handle crock where all points are coincedent */
    if (!somethingDrawn)
    {
	projectLeft = pGC->capStyle == CapProjecting;
	sfbWideSegment (pDrawable, pGC, pixel, 
		       x2, y2, x2, y2, projectLeft, projectLeft,
		       &leftFace, &rightFace);
	if (pGC->capStyle == CapRound)
	{
	    sfbLineArc (pDrawable, pGC, pixel, 
		       &leftFace, (LineFacePtr) NULL,
		       (double)0.0, (double)0.0,
		       TRUE);
	    rightFace.dx = -1;	/* sleezy hack to make it work */
	    sfbLineArc (pDrawable, pGC, pixel, 
		       (LineFacePtr) NULL, &rightFace,
 		       (double)0.0, (double)0.0,
		       TRUE);
	}
    }
    SFBFOREGROUND(sfb, pGC->fgPixel);
}

#define V_TOP	    0
#define V_RIGHT	    1
#define V_BOTTOM    2
#define V_LEFT	    3

static void
sfbWideDashSegment (pDrawable, pGC, pDashOffset, pDashIndex,
	   x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    register GCPtr  pGC;
    int		    *pDashOffset, *pDashIndex;
    int		    x1, y1, x2, y2;
    Bool	    projectLeft, projectRight;
    LineFacePtr	    leftFace, rightFace;
{
    int		    dashIndex, dashRemain;
    unsigned char   *pDash;
    double	    L, l;
    double	    k;
    PolyVertexRec   vertices[4];
    PolyVertexRec   saveRight, saveBottom;
    PolySlopeRec    slopes[4];
    PolyEdgeRec	    left[2], right[2];
    LineFaceRec	    lcapFace, rcapFace;
    int		    nleft, nright;
    int		    h;
    int		    y;
    int		    dy, dx;
    unsigned long   pixel;
    double	    LRemain;
    double	    r;
    double	    rdx, rdy;
    double	    dashDx, dashDy;
    double	    saveK;
    Bool	    first = TRUE;
    double	    lcenterx, lcentery, rcenterx, rcentery;
    unsigned long   fgPixel, bgPixel;
    
    dx = x2 - x1;
    dy = y2 - y1;
    dashIndex = *pDashIndex;
    pDash = pGC->dash;
    dashRemain = pDash[dashIndex] - *pDashOffset;
    fgPixel = pGC->fgPixel;
    bgPixel = pGC->bgPixel;

    l = ((double) pGC->lineWidth) / 2.0;
    if (dx == 0)
    {
	L = dy;
	rdx = 0;
	rdy = l;
	if (dy < 0)
	{
	    L = -dy;
	    rdy = -l;
	}
    }
    else if (dy == 0)
    {
	L = dx;
	rdx = l;
	rdy = 0;
	if (dx < 0)
	{
	    L = -dx;
	    rdx = -l;
	}
    }
    else
    {
	L = hypot ((double) dx, (double) dy);
	r = l / L;

	rdx = r * dx;
	rdy = r * dy;
    }
    k = l * L;
    LRemain = L;
    /* All position comments are relative to a line with dx and dy > 0,
     * but the code does not depend on this */
    /* top */
    slopes[V_TOP].dx = dx;
    slopes[V_TOP].dy = dy;
    slopes[V_TOP].k = k;
    /* right */
    slopes[V_RIGHT].dx = -dy;
    slopes[V_RIGHT].dy = dx;
    slopes[V_RIGHT].k = 0;
    /* bottom */
    slopes[V_BOTTOM].dx = -dx;
    slopes[V_BOTTOM].dy = -dy;
    slopes[V_BOTTOM].k = k;
    /* left */
    slopes[V_LEFT].dx = dy;
    slopes[V_LEFT].dy = -dx;
    slopes[V_LEFT].k = 0;

    /* preload the start coordinates */
    vertices[V_RIGHT].x = vertices[V_TOP].x = rdy;
    vertices[V_RIGHT].y = vertices[V_TOP].y = -rdx;

    vertices[V_BOTTOM].x = vertices[V_LEFT].x = -rdy;
    vertices[V_BOTTOM].y = vertices[V_LEFT].y = rdx;

    if (projectLeft)
    {
	vertices[V_TOP].x -= rdx;
	vertices[V_TOP].y -= rdy;

	vertices[V_LEFT].x -= rdx;
	vertices[V_LEFT].y -= rdy;

	slopes[V_LEFT].k = rdx * dx + rdy * dy;
    }

    lcenterx = x1;
    lcentery = y1;

    if (pGC->capStyle == CapRound)
    {
	lcapFace.dx = dx;
	lcapFace.dy = dy;
	lcapFace.x = x1;
	lcapFace.y = y1;

	rcapFace.dx = -dx;
	rcapFace.dy = -dy;
	rcapFace.x = x1;
	rcapFace.y = y1;
    }
    while (LRemain > dashRemain)
    {
	dashDx = (dashRemain * dx) / L;
	dashDy = (dashRemain * dy) / L;

	rcenterx = lcenterx + dashDx;
	rcentery = lcentery + dashDy;

	vertices[V_RIGHT].x += dashDx;
	vertices[V_RIGHT].y += dashDy;

	vertices[V_BOTTOM].x += dashDx;
	vertices[V_BOTTOM].y += dashDy;

	slopes[V_RIGHT].k = vertices[V_RIGHT].x * dx + vertices[V_RIGHT].y * dy;

	if (pGC->lineStyle == LineDoubleDash || !(dashIndex & 1))
	{
	    if (pGC->lineStyle == LineOnOffDash &&
 	        pGC->capStyle == CapProjecting)
	    {
		saveRight = vertices[V_RIGHT];
		saveBottom = vertices[V_BOTTOM];
		saveK = slopes[V_RIGHT].k;
		
		if (!first)
		{
		    vertices[V_TOP].x -= rdx;
		    vertices[V_TOP].y -= rdy;
    
		    vertices[V_LEFT].x -= rdx;
		    vertices[V_LEFT].y -= rdy;

		    slopes[V_LEFT].k = vertices[V_LEFT].x *
				       slopes[V_LEFT].dy -
				       vertices[V_LEFT].y *
				       slopes[V_LEFT].dx;
		}
		
		vertices[V_RIGHT].x += rdx;
		vertices[V_RIGHT].y += rdy;

		vertices[V_BOTTOM].x += rdx;
		vertices[V_BOTTOM].y += rdy;

		slopes[V_RIGHT].k = vertices[V_RIGHT].x *
				   slopes[V_RIGHT].dy -
				   vertices[V_RIGHT].y *
				   slopes[V_RIGHT].dx;
	    }
	    y = sfbPolyBuildPoly (vertices, slopes, 4, x1, y1,
			     	 left, right, &nleft, &nright, &h);
	    pixel = (dashIndex & 1) ? bgPixel : fgPixel;
	    sfbFillPolyHelper (pDrawable, pGC, pixel, y, h, left, right, nleft, nright);

	    if (pGC->lineStyle == LineOnOffDash)
	    {
		switch (pGC->capStyle)
		{
		case CapProjecting:
		    vertices[V_BOTTOM] = saveBottom;
		    vertices[V_RIGHT] = saveRight;
		    slopes[V_RIGHT].k = saveK;
		    break;
		case CapRound:
		    if (!first)
		    {
		    	if (dx < 0)
		    	{
		    	    lcapFace.xa = -vertices[V_LEFT].x;
		    	    lcapFace.ya = -vertices[V_LEFT].y;
			    lcapFace.k = slopes[V_LEFT].k;
		    	}
		    	else
		    	{
		    	    lcapFace.xa = vertices[V_TOP].x;
		    	    lcapFace.ya = vertices[V_TOP].y;
			    lcapFace.k = -slopes[V_LEFT].k;
		    	}
		    	sfbLineArc (pDrawable, pGC, pixel,
			       	   &lcapFace, (LineFacePtr) NULL,
			       	   lcenterx, lcentery, FALSE);
		    }
		    if (dx < 0)
		    {
		    	rcapFace.xa = vertices[V_BOTTOM].x;
		    	rcapFace.ya = vertices[V_BOTTOM].y;
			rcapFace.k = slopes[V_RIGHT].k;
		    }
		    else
		    {
		    	rcapFace.xa = -vertices[V_RIGHT].x;
		    	rcapFace.ya = -vertices[V_RIGHT].y;
			rcapFace.k = -slopes[V_RIGHT].k;
		    }
		    sfbLineArc (pDrawable, pGC, pixel, 
			       (LineFacePtr) NULL, &rcapFace,
			       rcenterx, rcentery, FALSE);
		    break;
	    	}
	    }
	}
	LRemain -= dashRemain;
	++dashIndex;
	if (dashIndex == pGC->numInDashList)
	    dashIndex = 0;
	dashRemain = pDash[dashIndex];

	lcenterx = rcenterx;
	lcentery = rcentery;

	vertices[V_TOP] = vertices[V_RIGHT];
	vertices[V_LEFT] = vertices[V_BOTTOM];
	slopes[V_LEFT].k = -slopes[V_RIGHT].k;
	first = FALSE;
    }

    if (pGC->lineStyle == LineDoubleDash || !(dashIndex & 1))
    {
    	vertices[V_TOP].x -= dx;
    	vertices[V_TOP].y -= dy;

	vertices[V_LEFT].x -= dx;
	vertices[V_LEFT].y -= dy;

	vertices[V_RIGHT].x = rdy;
	vertices[V_RIGHT].y = -rdx;

	vertices[V_BOTTOM].x = -rdy;
	vertices[V_BOTTOM].y = rdx;

	
	if (projectRight)
	{
	    vertices[V_RIGHT].x += rdx;
	    vertices[V_RIGHT].y += rdy;
    
	    vertices[V_BOTTOM].x += rdx;
	    vertices[V_BOTTOM].y += rdy;
	    slopes[V_RIGHT].k = vertices[V_RIGHT].x *
				slopes[V_RIGHT].dy -
				vertices[V_RIGHT].y *
				slopes[V_RIGHT].dx;
	}
	else
	    slopes[V_RIGHT].k = 0;

	if (!first && pGC->lineStyle == LineOnOffDash &&
	    pGC->capStyle == CapProjecting)
	{
	    vertices[V_TOP].x -= rdx;
	    vertices[V_TOP].y -= rdy;

	    vertices[V_LEFT].x -= rdx;
	    vertices[V_LEFT].y -= rdy;
	    slopes[V_LEFT].k = vertices[V_LEFT].x *
			       slopes[V_LEFT].dy -
			       vertices[V_LEFT].y *
			       slopes[V_LEFT].dx;
	}
	else
	    slopes[V_LEFT].k += dx * dx + dy * dy;


	y = sfbPolyBuildPoly (vertices, slopes, 4, x2, y2,
			     left, right, &nleft, &nright, &h);

	pixel = (dashIndex & 1) ? pGC->bgPixel : pGC->fgPixel;
	sfbFillPolyHelper (pDrawable, pGC, pixel, y, h, left, right, nleft, nright);
	if (!first && pGC->lineStyle == LineOnOffDash &&
	    pGC->capStyle == CapRound)
	{
	    lcapFace.x = x2;
	    lcapFace.y = y2;
	    if (dx < 0)
	    {
		lcapFace.xa = -vertices[V_LEFT].x;
		lcapFace.ya = -vertices[V_LEFT].y;
		lcapFace.k = slopes[V_LEFT].k;
	    }
	    else
	    {
		lcapFace.xa = vertices[V_TOP].x;
		lcapFace.ya = vertices[V_TOP].y;
		lcapFace.k = -slopes[V_LEFT].k;
	    }
	    sfbLineArc (pDrawable, pGC, pixel, 
		       &lcapFace, (LineFacePtr) NULL,
		       rcenterx, rcentery, FALSE);
	}
    }
    dashRemain = ((double) dashRemain) - LRemain;
    if (dashRemain == 0)
    {
	dashIndex++;
	if (dashIndex == pGC->numInDashList)
	    dashIndex = 0;
	dashRemain = pDash[dashIndex];
    }

    leftFace->x = x1;
    leftFace->y = y1;
    leftFace->dx = dx;
    leftFace->dy = dy;
    leftFace->xa = rdy;
    leftFace->ya = -rdx;
    leftFace->k = k;

    rightFace->x = x2;
    rightFace->y = y2;
    rightFace->dx = -dx;
    rightFace->dy = -dy;
    rightFace->xa = -rdy;
    rightFace->ya = rdx;
    rightFace->k = k;

    *pDashIndex = dashIndex;
    *pDashOffset = pDash[dashIndex] - dashRemain;
}

void
sfbWideDash (pDrawable, pGC, mode, npt, pPts)
    DrawablePtr	pDrawable;
    register GCPtr pGC;
    int		mode;
    register int npt;
    register DDXPointPtr pPts;
{
    int		    x1, y1, x2, y2;
    unsigned long   pixel;
    Bool	    projectLeft, projectRight;
    LineFaceRec	    leftFace, rightFace, prevRightFace;
    LineFaceRec	    firstFace;
    int		    first;
    int		    dashIndex, dashOffset;
    register int    prevDashIndex;
    Bool	    somethingDrawn = FALSE;
    Bool	    selfJoin;
    Bool	    endIsFg, startIsFg, firstIsFg = FALSE, prevIsFg;
    sfbScreenPrivPtr    scrPriv;
    SFB                 sfb;

    if (npt == 0)
	return;

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTSTIPPLE);

    x1 = x2 = pPts->x;
    y1 = y2 = pPts->y;
    first = TRUE;
    selfJoin = FALSE;
    if (mode == CoordModePrevious)
    {
	int nptTmp;
	DDXPointPtr pPtsTmp;

	nptTmp = npt;
	pPtsTmp = pPts + 1;
	while (--nptTmp)
	{
	    x1 += pPtsTmp->x;
	    y1 += pPtsTmp->y;
	    ++pPtsTmp;
	}
	if (x2 == x1 && y2 == y1)
	    selfJoin = TRUE;
    }
    else if (x2 == pPts[npt-1].x && y2 == pPts[npt-1].y)
    {
	selfJoin = TRUE;
    }
    projectLeft = pGC->capStyle == CapProjecting && !selfJoin;
    projectRight = FALSE;
    dashIndex = 0;
    dashOffset = 0;
    miStepDash ((int)pGC->dashOffset, &dashIndex,
	        pGC->dash, (int)pGC->numInDashList, &dashOffset);
    while (--npt)
    {
	x1 = x2;
	y1 = y2;
	++pPts;
	x2 = pPts->x;
	y2 = pPts->y;
	if (mode == CoordModePrevious)
	{
	    x2 += x1;
	    y2 += y1;
	}
	if (x1 != x2 || y1 != y2)
	{
	    somethingDrawn = TRUE;
	    if (npt == 1 && pGC->capStyle == CapProjecting && 
		(!selfJoin || !firstIsFg))
		projectRight = TRUE;
	    prevDashIndex = dashIndex;
	    sfbWideDashSegment (pDrawable, pGC, &dashOffset, &dashIndex,
				x1, y1, x2, y2,
				projectLeft, projectRight, &leftFace, &rightFace);
	    startIsFg = !(prevDashIndex & 1);
	    endIsFg = (dashIndex & 1) ^ (dashOffset != 0);
	    if (pGC->lineStyle == LineDoubleDash || startIsFg)
	    {
	    	pixel = startIsFg ? pGC->fgPixel : pGC->bgPixel;
	    	if (first || (pGC->lineStyle == LineOnOffDash && !prevIsFg))
	    	{
	    	    if (first && selfJoin)
		    {
		    	firstFace = leftFace;
			firstIsFg = startIsFg;
		    }
	    	    else if (pGC->capStyle == CapRound)
		    	sfbLineArc (pDrawable, pGC, pixel,
			       	   &leftFace, (LineFacePtr) NULL,
			       	   (double)0.0, (double)0.0, TRUE);
	    	}
	    	else
	    	{
	    	    sfbLineJoin (pDrawable, pGC, pixel, &leftFace,
		            	&prevRightFace);
	    	}
	    }
	    prevRightFace = rightFace;
	    prevIsFg = endIsFg;
	    first = FALSE;
	    projectLeft = FALSE;
	}
	if (npt == 1 && somethingDrawn)
	{
	    if (pGC->lineStyle == LineDoubleDash || endIsFg)
	    {
		pixel = endIsFg ? pGC->fgPixel : pGC->bgPixel;
		if (selfJoin && (pGC->lineStyle == LineDoubleDash || firstIsFg))
		{
		    sfbLineJoin (pDrawable, pGC, pixel, &firstFace,
				&rightFace);
		}
		else 
		{
		    if (pGC->capStyle == CapRound)
			sfbLineArc (pDrawable, pGC, pixel, 
				    (LineFacePtr) NULL, &rightFace,
				    (double)0.0, (double)0.0, TRUE);
		}
	    }
	    else
	    {
		/* glue a cap to the start of the line if
		 * we're OnOffDash and ended on odd dash
		 */
		if (selfJoin && firstIsFg)
		{
		    pixel = pGC->fgPixel;
		    if (pGC->capStyle == CapProjecting)
			sfbLineProjectingCap (pDrawable, pGC, pixel,
				    &firstFace, TRUE,
				    (double)0.0, (double)0.0, TRUE);
		    else if (pGC->capStyle == CapRound)
			sfbLineArc (pDrawable, pGC, pixel,
				    &firstFace, (LineFacePtr) NULL,
				    (double)0.0, (double)0.0, TRUE);
		}
	    }
	}
    }
    /* handle crock where all points are coincedent */
    if (!somethingDrawn && (pGC->lineStyle == LineDoubleDash || !(dashIndex & 1)))
    {
	/* not the same as endIsFg computation above */
	pixel = (dashIndex & 1) ? pGC->bgPixel : pGC->fgPixel;
	projectLeft = pGC->capStyle == CapProjecting;
	sfbWideDashSegment (pDrawable, pGC, &dashOffset, &dashIndex,
			    x1, y1, x2, y2,
			    projectLeft, projectRight, &leftFace, &rightFace);
	if (pGC->capStyle == CapRound)
	{
	    sfbLineArc (pDrawable, pGC, pixel, 
		       &leftFace, (LineFacePtr) NULL,
		       (double)0.0, (double)0.0,
		       TRUE);
	    rightFace.dx = -1;	/* sleezy hack to make it work */
	    sfbLineArc (pDrawable, pGC, pixel,
		       (LineFacePtr) NULL, &rightFace,
 		       (double)0.0, (double)0.0,
		       TRUE);
	}
    }
    SFBFOREGROUND(sfb, pGC->fgPixel);
}
