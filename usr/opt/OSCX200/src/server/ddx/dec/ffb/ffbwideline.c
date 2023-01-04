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
static char *rcsid = "@(#)$RCSfile: ffbwideline.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:19:05 $";
#endif
/*
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
#include "ffbwideline.h"

#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "ffb.h"

#if (defined(SVR4) || defined(SYSV) && defined(SYSV386)) && __STDC__
extern double hypot(double, double);
#endif

#if defined(__alpha) && 0
/* ||| This code is terribly slow on EV4, as round to minus infinity is not
   supported, and PAL code is death on performance.  Try it out on EV5, which
   is going to have a different binary anyway. */
#   include <c_asm.h>
#   define ICEILTEMPDECL double _cTmp2;
#define ICEIL(x)  (_cTmp2 = dasm("cvttqm %f16, %f0;", -x), \
	   -*(long *)&_cTmp2)
/* ||| This code should be okay on EV4 (if it's correct!), but the asm
   statement is not happy recognizing some of the fp ops, like stt */
#define ICEILTEMPDECL   static long _cTmp[2];
#   define ICEIL(x) asm("						\
	cvttqc  %f16, %f0;      /* (long)(x)			    */  \
	stt     %f0, 0(%a0);						\
	cvtqt   %f0, %f0;       /* (double)(long)(x)		    */  \
	cmptlt  %f0, %f16, %f0; /* (double)(long)(x) < (x) << 62    */  \
	stt     %f0, 8(%a0);						\
	ldq     %v0, 0(%a0);    /* (long)(x)			    */  \
	ldq     %t0, 8(%a0);    /* (double)(long)(x) < (x) << 62    */  \
	srl     %t0, 62, %t0;   /* (double)(long)(x) < (x)	    */  \
	addq    %v0, %t0, %v0;  /* ICEIL			    */  \
    ", _cTmp)

#else
#   define ICEIL(x) ((long)(x) + ((double)(long)(x) < (x)))
#endif


#   define IFLOOR(x)   ((long)(x) - ((double)(long)(x) > (x)))


#ifdef ICEILTEMPDECL
ICEILTEMPDECL
#endif

static void ffbLineArc();

#define FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel)   \
{							\
    WRITE_MEMORY_BARRIER();				\
    if (fillMode == BLOCKFILL) {			\
	FFBLOADCOLORREGS(ffb, pixel, pDrawable->depth); \
    } else {						\
	FFBFOREGROUND(ffb, pixel);			\
    }							\
} /* FFBLOADPIXEL */

/*
 * polygon filler
 */

void
ffbFillPolyHelper (pDrawable, pGC, maxPixels, y, overall_height,
		  left, right, left_count, right_count)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    long	maxPixels;
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

    int	height, width;
    int	left_height, right_height;
    int			xorg;
    int			ymul;
    BoxPtr		extents;
    int			x1, x2;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase;
    int			dstwidth;
    FFB                 ffb;
    Bool		xNeedsClipping;

    y += pDrawable->y;
    extents = &(CFBGCPRIV(pGC)->pCompositeClip->extents);
    clipx1 = extents->x1;
    clipy1 = extents->y1;
    clipx2 = extents->x2;
    clipy2 = extents->y2;
    
    if (y >= clipy2 || y + overall_height < clipy1) return;

    DrawableBaseAndWidth(pDrawable, pdstBase, dstwidth);
    xorg = pDrawable->x;
    ffb = FFBSCREENPRIV(pDrawable->pScreen)->ffb;
    WRITE_MEMORY_BARRIER();
    pdstBase += y * dstwidth;
    /* 1 <= left_count, right_count <= 2 */
    /* The assumption is, most lines don't need clipping. */
    x1 = left[0].x + xorg;
    if (left[0].signdx < 0)
	x1 += left[0].height * (left[0].stepx-1);
    xNeedsClipping = (x1 < clipx1);
    if (left_count > 1) {
	x1 = left[1].x + xorg;
	if (left[1].signdx < 0)
	    x1 += left[1].height * (left[1].stepx-1);
	xNeedsClipping |= (x1 < clipx1);
    }
    x2 = right[0].x + xorg;
    if (right[0].signdx > 0)
	x2 += right[0].height * (right[0].stepx+1);
    xNeedsClipping |= (x2 >= clipx2);
    if (right_count > 1) {
	x2 = right[1].x + xorg;
	if (right[1].signdx > 0)
	    x2 += right[1].height * (right[1].stepx+1);
	xNeedsClipping |= (x2 >= clipx2);
    }

    left_height = 0;
    right_height = 0;
    
    /* Get to portion of line within clip rectangle */
    while (y < clipy1 
	    && (left_count | left_height) && (right_count | right_height)) {
	MIPOLYRELOADLEFT;
	MIPOLYRELOADRIGHT;

	height = min(left_height, right_height);
	if (height > clipy1 - y)
	    height = clipy1 - y;
	left_height -= height;
	right_height -= height;

	while (height > 0) {
	    height--;
	    y++;
	    pdstBase += dstwidth;
	    MIPOLYSTEPLEFT;
	    MIPOLYSTEPRIGHT;
	}
    }
	
    while ((left_count | left_height) &&
	   (right_count | right_height) && y < clipy2) {
	MIPOLYRELOADLEFT;
	MIPOLYRELOADRIGHT;

	height = left_height;
	if (height > right_height)
	    height = right_height;
	if (height > (clipy2 - y)) {
	    height = clipy2 - y;
	}
	left_height -= height;
	right_height -= height;
	y += height;

	if (!xNeedsClipping) {
	    if (clipx2 - clipx1 <= maxPixels) {
		/* Span length fits into single command */
		while (height > 0) {
		    width = right_x - left_x + 1;
		    if (width > 0) {
			Pixel8 * pdst;
			long align;
			pdst = pdstBase + left_x * FFBPIXELBYTES;
			align = (long)pdst & FFBBUSBYTESMASK;
			pdst -= align;
			FFBWRITE(pdst, FFBLOADBLOCKDATA(align, width));
		    }
		    height--;
		    pdstBase += dstwidth;
		    MIPOLYSTEPLEFT;
		    MIPOLYSTEPRIGHT;
		}
	    } else {
		while (height > 0) {
		    width = right_x - left_x + 1;
		    if (width > 0) {
			FFBSOLIDSPAN(ffb, pdstBase + left_x * FFBPIXELBYTES, 
				width, maxPixels);
		    }
		    height--;
		    pdstBase += dstwidth;
		    MIPOLYSTEPLEFT;
		    MIPOLYSTEPRIGHT;
		}
	    }
	} else { /* x does need clipping */
	    while (height > 0) {
		x1 = left_x;
		x2 = right_x + 1;
		if (x1 < clipx1) x1 = clipx1;
		if (x2 > clipx2) x2 = clipx2;
		width = x2 - x1;
		if (width > 0) {
		    FFBSOLIDSPAN(ffb, pdstBase + x1 * FFBPIXELBYTES, 
			width, maxPixels);
		}
		height--;
		pdstBase += dstwidth;
		MIPOLYSTEPLEFT;
		MIPOLYSTEPRIGHT;
	    }
	}
    }
}

static void
ffbFillRectPolyHelper (pDrawable, pGC, maxPixels, x, y, w, h)
    DrawablePtr     pDrawable;
    GCPtr	    pGC;
    long	    maxPixels;
    int		    x, y, w, h;
{
    int			width, align;
    CommandWord		leftMask, rightMask, mask;
    BoxPtr		extents;
    int			x2, y2, ymul, m;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase, *pdst, *p;
    int			dstwidth;
    FFB                 ffb;

    DrawableBaseAndWidth(pDrawable, pdstBase, dstwidth);
    ffb = FFBSCREENPRIV(pDrawable->pScreen)->ffb;
    WRITE_MEMORY_BARRIER(); 

    extents = &(CFBGCPRIV(pGC)->pCompositeClip->extents);

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
	pdst = pdstBase + ymul + x * FFBPIXELBYTES;
	align = (long)pdst & FFBBUSBYTESMASK;
	pdst -= align;
    
	if (width <= maxPixels) {
	    /* One block fill command per scan line */
	    mask = FFBLOADBLOCKDATA(align,width);
	    while (h != 0) {
		FFBWRITE(pdst, mask);
		pdst += dstwidth;
		h -= 1;
	    };
	} else {
	    /* Mask requires multiple words */
	    mask = FFBLOADBLOCKDATA(align, maxPixels);
	    do {
		p = pdst;
		w = width;
		while (w > maxPixels) {
		    FFBWRITE(p, mask);
		    p += maxPixels * FFBPIXELBYTES;
		    w -= maxPixels;
		}
		FFBWRITE(p, FFBLOADBLOCKDATA(align, w));
		pdst += dstwidth;
		h--;
	    } while (h !=0);
	} /* if skinny else fat rectangle */
   }
}

static int
ffbPolyBuildEdge (y0, k, dx, dy, xi, yi, left, edge)
    double	y0;
    double	k;  /* x0 * dy - y0 * dx */
    long	dx, dy;
    int		xi, yi;
    int		left;
    register PolyEdgePtr edge;
{
    long	x, y, e;
    long	xady, axady;
    long	stepx;

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
	    x = - (int)((float)axady / dy) - 1;
	    e = xady - x * dy;
	}
    } else {
	if (xady <= dy) {
	    x = 0;
	    e = xady;
	} else {
	    x = (int)((float)(xady - 1) / dy);
	    e = xady - x * dy;
	}
    }

#define USEFLOAT    /* Seems faster than call __divl, __reml */

    stepx = 0;
    if (dx >= 0) {
	edge->signdx = 1;
	if (dx >= dy) {
#ifdef USEFLOAT
	    stepx = ((float)dx / dy);
	    dx = dx - stepx * dy;
#else
	    stepx = dx / dy;
	    dx = dx % dy;
#endif
	}
    } else {
	edge->signdx = -1;
	dx = -dx;
	if (dx >= dy) {
#ifdef USEFLOAT
	    stepx = -((float)dx / dy);
	    dx = dx + stepx * dy;
#else
	    stepx = - (dx / dx);
	    dx = dx % dy;
#endif
	}
	e = dy - e + 1;
    }
    edge->dx = dx;
    edge->dy = dy;
    edge->stepx = stepx;
    edge->x = x + left + xi;
    edge->e = e - dy;	/* bias to compare against 0 instead of dy */
    return y + yi;
}

#define StepAround(v, incr, max) (((v) + (incr) < 0) ? (max - 1) : ((v) + (incr) == max) ? 0 : ((v) + (incr)))

static int
ffbPolyBuildPoly (vertices, slopes, count, xi, yi,
	left, right, pnleft, pnright, h)
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
	    y = ffbPolyBuildEdge (vertices[i].y,
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
	    y = ffbPolyBuildEdge (vertices[i].y,
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
ffbLineJoin (pDrawable, pGC, maxPixels, pLeft, pRight)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    long	    maxPixels;
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
	ffbLineArc(pDrawable, pGC, maxPixels, pLeft, pRight,
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

    y = ffbPolyBuildPoly (vertices, slopes, edgecount, pLeft->x, pLeft->y,
		   left, right, &nleft, &nright, &height);
    ffbFillPolyHelper (pDrawable, pGC, maxPixels,
		    y, height, left, right, nleft, nright);
}

static void
ffbLineArcI (pDraw, pGC, maxPixels, xorg, yorg)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    long	    maxPixels;
    int		    xorg, yorg;
{
    register int x, y, e, ex, slw;
    int			x1, x2, y1, y2;
    BoxPtr		extents;
    int			ymul, align;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase, *pdst;
    int			dstwidth;
    FFB                 ffb;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    ffb = FFBSCREENPRIV(pDraw->pScreen)->ffb;
    WRITE_MEMORY_BARRIER();
    extents = &(CFBGCPRIV(pGC)->pCompositeClip->extents);

    xorg += pDraw->x;
    yorg += pDraw->y;
    slw = pGC->lineWidth;
    pdstBase += yorg * dstwidth;
    if (slw == 1) {
	if (extents->x1 <= xorg & xorg < extents->x2 &
	    extents->y1 <= yorg & yorg < extents->y2) {
	    pdst = pdstBase + xorg * FFBPIXELBYTES;

	    align = (long)pdst & FFBBUSBYTESMASK;
	    pdst -= align;
	    FFBWRITE(pdst, FFBLOADBLOCKDATA( align, 1));
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
	    FFBSOLIDSPAN(ffb, pdstBase - ymul + x1 * FFBPIXELBYTES, slw,
		maxPixels);
	    if ((y != 0) && ((slw > 1) || (e != ex))) {
		pdstBase = CYCLE_FB(pdstBase);
		FFBSOLIDSPAN(ffb, pdstBase + ymul + x1 * FFBPIXELBYTES,
		    slw, maxPixels);
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
		    FFBSOLIDSPAN(ffb, pdstBase - ymul + x1 * FFBPIXELBYTES,
			x2 - x1, maxPixels);
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
			FFBSOLIDSPAN(ffb, pdstBase + ymul + x1 * FFBPIXELBYTES,
			    x2 - x1, maxPixels);
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
ffbLineArcD(pDraw, pGC, maxPixels, xorg, yorg, 
	    edge1, edgey1, edgeleft1, edge2, edgey2, edgeleft2)
    DrawablePtr	    pDraw;
    GCPtr	    pGC;
    long	    maxPixels;
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
    BoxPtr		extents;
    int			ymul, align;
    int			clipx1, clipy1, clipx2, clipy2;
    Pixel8		*pdstBase, *pdst;
    int			dstwidth;
    FFB                 ffb;

    DrawableBaseAndWidth(pDraw, pdstBase, dstwidth);
    ffb = FFBSCREENPRIV(pDraw->pScreen)->ffb;
    WRITE_MEMORY_BARRIER();
    extents = &(CFBGCPRIV(pGC)->pCompositeClip->extents);

    xbase = IFLOOR(xorg);
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
    y = IFLOOR(radius - y0 + 1.0);
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
		    FFBSOLIDSPAN(ffb, pdstBase + xcl * FFBPIXELBYTES,
			x2 - xcl, maxPixels);
		}
	    }
/*	} */
    }
    er = xrk - (xr << 1) - er;
    el = (xl << 1) - xlk - el;
    boty = IFLOOR(-y0 - radius) + 1;
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
		    FFBSOLIDSPAN(ffb, pdstBase + xcl * FFBPIXELBYTES,
			x2 - xcl, maxPixels);
		}
	    }
/*	} */
    }
}

#ifdef FFB_DEPTH_INDEPENDENT
ffbRoundJoinFace (face, edge, leftEdge)
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
	y = ffbPolyBuildEdge (ya, 0.0, dx, dy, face->x, face->y, !left, edge);
	edge->height = 32767;
    }
    *leftEdge = !left;
    return y;
}

ffbRoundJoinClip (pLeft, pRight, edge1, edge2, y1, y2, left1, left2)
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
    *y1 = ffbRoundJoinFace (pLeft, edge1, left1);
    *y2 = ffbRoundJoinFace (pRight, edge2, left2);
}

ffbRoundCapClip (face, isInt, edge, leftEdge)
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
	y = ffbPolyBuildEdge (ya, k, dx, dy, face->x, face->y, !left, edge);
	edge->height = 32767;
    }
    *leftEdge = !left;
    return y;
}
#endif /* FFB_DEPTH_INDEPENDENT */

static void
ffbLineArc (pDraw, pGC, maxPixels, leftFace, rightFace, 
	xorg, yorg, isInt)
    DrawablePtr	    pDraw;
    register GCPtr  pGC;
    long	    maxPixels;
    register LineFacePtr leftFace, rightFace;
    double	    xorg, yorg;
    Bool	    isInt;
{
    int xorgi, yorgi;
    PolyEdgeRec	edge1, edge2;
    int		edgey1, edgey2;
    Bool	edgeleft1, edgeleft2;

    if (isInt) {
	if (leftFace) {
            xorgi = leftFace->x;
            yorgi = leftFace->y; 
	} else {
            xorgi = rightFace->x;
            yorgi = rightFace->y; 
	}
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
	    ffbRoundJoinClip (leftFace, rightFace, &edge1, &edge2,
			     &edgey1, &edgey2, &edgeleft1, &edgeleft2);
	}
	else if (leftFace)
	{
	    edgey1 = ffbRoundCapClip (leftFace, isInt, &edge1, &edgeleft1);
	}
	else if (rightFace)
	{
	    edgey2 = ffbRoundCapClip (rightFace, isInt, &edge2, &edgeleft2);
	}
	ffbLineArcD(pDraw, pGC, maxPixels, xorg, yorg,
		       &edge1, edgey1, edgeleft1,
		       &edge2, edgey2, edgeleft2);
    } else if (isInt) {
	ffbLineArcI(pDraw, pGC, maxPixels, xorgi, yorgi);
    } else {
	ffbLineArcD(pDraw, pGC, maxPixels, xorg, yorg,
		       &edge1, edgey1, edgeleft1,
		       &edge2, edgey2, edgeleft2);
    }
}

ffbLineProjectingCap (pDrawable, pGC, maxPixels, face, isLeft, xorg,
    yorg, isInt)
    DrawablePtr	    pDrawable;
    register GCPtr  pGC;
    long	    maxPixels;
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
    double      maxy;
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
	ffbFillPolyHelper (pDrawable, pGC, maxPixels, 
		yorgi - (lw >> 1), lw, lefts, rights, 1, 1);
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
	ffbFillPolyHelper (pDrawable, pGC, maxPixels, 
		topy, bottomy - topy, lefts, rights, 1, 1);
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
	    righty = ffbPolyBuildEdge (ya, k, dx, dy, xorgi, yorgi, 0, right);
	    
	    xa = -xa;
	    ya = -ya;
	    k = -k;
	    lefty = ffbPolyBuildEdge (ya - projectYoff,
				     k, dx, dy, xorgi, yorgi, 1, left);
	    if (dx > 0)
	    {
		ya = -ya;
		xa = -xa;
	    }
	    xap = xa - projectXoff;
	    yap = ya - projectYoff;
	    topy = ffbPolyBuildEdge (yap, xap * dx + yap * dy,
				    -dy, dx, xorgi, yorgi, dx > 0, top);
	    bottomy = ffbPolyBuildEdge (ya, 0.0, -dy, dx, xorgi, yorgi,
				    dx < 0, bottom);
	    maxy = -ya;
	}
	else
	{
	    righty = ffbPolyBuildEdge (ya - projectYoff,
		     k, dx, dy, xorgi, yorgi, 0, right);
	    
	    xa = -xa;
	    ya = -ya;
	    k = -k;
	    lefty = ffbPolyBuildEdge (ya, k, dx, dy, xorgi, yorgi, 1, left);
	    if (dx > 0)
	    {
		ya = -ya;
		xa = -xa;
	    }
	    xap = xa - projectXoff;
	    yap = ya - projectYoff;
	    topy = ffbPolyBuildEdge (ya, 0.0, -dy, dx, xorgi, xorgi, dx > 0, top);
	    bottomy = ffbPolyBuildEdge (yap, xap * dx + yap * dy,
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
	ffbFillPolyHelper (pDrawable, pGC, maxPixels, 
		topy, bottom->height + bottomy - topy, lefts, rights, 2, 2);
    }
}

static void
ffbWideSegment (pDrawable, pGC, maxPixels,
	       x1, y1, x2, y2, projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    GCPtr	    pGC;
    long	    maxPixels;
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
	ffbFillRectPolyHelper (pDrawable, pGC, maxPixels, x, y, dx, dy);
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
	ffbFillRectPolyHelper (pDrawable, pGC, maxPixels, x, y, dx, dy);
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
	    righty = ffbPolyBuildEdge (ya - projectYoff,
				      k, dx, dy, x1, y1, 0, right);
	else
	    righty = ffbPolyBuildEdge (ya,
				      k, dx,dy, x1, y1, 0, right);
	/* coord of lower bound at integral y */
	ya = -ya;
	xa = -xa;

	/* xa * dy - ya * dx */
	k = - k;

	if (projectLeft)
	    lefty = ffbPolyBuildEdge (ya - projectYoff,
					k, dx, dy, x1, y1, 1, left);
	else
	    lefty = ffbPolyBuildEdge (ya,
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
	    topy = ffbPolyBuildEdge (yap, xap * dx + yap * dy,
				    -dy, dx, x1, y1, dx > 0, top);
	}
	else
	    topy = ffbPolyBuildEdge (ya, 0.0, -dy, dx, x1, y1, dx > 0, top);

	/* coord of bottom face at integral y */

	if (projectRight)
	{
	    double xap = xa + projectXoff;
	    double yap = ya + projectYoff;
	    bottomy = ffbPolyBuildEdge (yap, xap * dx + yap * dy,
				       -dy, dx, x2, y2, dx < 0, bottom);
	    maxy = -ya + projectYoff;
	}
	else
	{
	    bottomy = ffbPolyBuildEdge (ya, 0.0, 
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
	ffbFillPolyHelper (pDrawable, pGC, maxPixels, 
		topy, bottom->height + bottomy - topy, lefts, rights, 2, 2);
    }
}

void
ffbWideLine (pDrawable, pGC, mode, npt, pPts)
    DrawablePtr	pDrawable;
    register GCPtr pGC;
    int		mode;
    register int npt;
    register DDXPointPtr pPts;
{
    int		    x1, y1, x2, y2;
    Bool	    projectLeft, projectRight;
    LineFaceRec	    leftFace, rightFace, prevRightFace;
    LineFaceRec	    firstFace;
    register int    first;
    Bool	    somethingDrawn = FALSE;
    Bool	    selfJoin;
    ffbScreenPrivPtr    scrPriv;
    FFB             ffb;
    long	    maxPixels;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDrawable->pScreen, pDrawable, scrPriv, ffb, pGC);

    FFBFILLMODE(pDrawable, pGC, TRUE, maxPixels,
		{ cfbPrivGCPtr cfbPriv = CFBGCPRIV(pGC);
		  FFBLOADCOLORREGS(ffb,cfbPriv->xor, pDrawable->depth); },
		/**/,
		{ CYCLE_REGS(ffb); FFBMODE(ffb, ffbFillMode); } );
    FFBDATA(ffb, ~0);

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
	    ffbWideSegment (pDrawable, pGC, maxPixels, x1, y1, x2, y2,
		       	   projectLeft, projectRight, &leftFace, &rightFace);
	    if (first)
	    {
	    	if (selfJoin)
		    firstFace = leftFace;
	    	else if (pGC->capStyle == CapRound)
		    ffbLineArc (pDrawable, pGC, maxPixels,
			       &leftFace, (LineFacePtr) NULL,
 			       (double)0.0, (double)0.0,
			       TRUE);
	    }
	    else
	    {
	    	ffbLineJoin (pDrawable, pGC, maxPixels, &leftFace,
			&prevRightFace);
	    }
	    prevRightFace = rightFace;
	    first = FALSE;
	    projectLeft = FALSE;
	}
	if (npt == 1 && somethingDrawn)
 	{
	    if (selfJoin)
		ffbLineJoin (pDrawable, pGC, maxPixels, 
			&firstFace, &rightFace);
	    else if (pGC->capStyle == CapRound)
		ffbLineArc (pDrawable, pGC, maxPixels,
			   (LineFacePtr) NULL, &rightFace,
			   (double)0.0, (double)0.0,
			   TRUE);
	}
    }
    /* handle crock where all points are coincedent */
    if (!somethingDrawn)
    {
	projectLeft = pGC->capStyle == CapProjecting;
	ffbWideSegment (pDrawable, pGC, maxPixels,
		       x2, y2, x2, y2, projectLeft, projectLeft,
		       &leftFace, &rightFace);
	if (pGC->capStyle == CapRound)
	{
	    ffbLineArc (pDrawable, pGC, maxPixels,
		       &leftFace, (LineFacePtr) NULL,
		       (double)0.0, (double)0.0,
		       TRUE);
	    rightFace.dx = -1;	/* sleezy hack to make it work */
	    ffbLineArc (pDrawable, pGC, maxPixels,
		       (LineFacePtr) NULL, &rightFace,
 		       (double)0.0, (double)0.0,
		       TRUE);
	}
    }

    scrPriv->lastGC = (GCPtr)NULL;      
}

#define V_TOP	    0
#define V_RIGHT	    1
#define V_BOTTOM    2
#define V_LEFT	    3

static void
ffbWideDashSegment (pDrawable, pGC, fillMode, maxPixels,
	    pDashOffset, pDashIndex, x1, y1, x2, y2,
	    projectLeft, projectRight, leftFace, rightFace)
    DrawablePtr	    pDrawable;
    register GCPtr  pGC;
    FFBMode	    fillMode;
    long	    maxPixels;
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
    FFB		    ffb;

    dx = x2 - x1;
    dy = y2 - y1;
    dashIndex = *pDashIndex;
    pDash = pGC->dash;
    dashRemain = pDash[dashIndex] - *pDashOffset;

    /* Get properly massaged version of fgPixel and bgPixel */
    if (fillMode == BLOCKFILL) {
	cfbPrivGCPtr devPriv;
	devPriv = CFBGCPRIV(pGC);
	fgPixel = devPriv->xor;
	bgPixel = devPriv->and;
    } else {
	fgPixel = pGC->fgPixel;
	bgPixel = pGC->bgPixel;
    }
    ffb = FFBSCREENPRIV(pDrawable->pScreen)->ffb;

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
	    y = ffbPolyBuildPoly (vertices, slopes, 4, x1, y1,
			     	 left, right, &nleft, &nright, &h);
	    pixel = (dashIndex & 1) ? bgPixel : fgPixel;
	    FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel);
	    ffbFillPolyHelper (pDrawable, pGC, maxPixels, 
		    y, h, left, right, nleft, nright);
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
		    	ffbLineArc (pDrawable, pGC, maxPixels,
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
		    ffbLineArc (pDrawable, pGC, maxPixels,
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


	y = ffbPolyBuildPoly (vertices, slopes, 4, x2, y2,
			     left, right, &nleft, &nright, &h);

	pixel= (dashIndex & 1) ? bgPixel : fgPixel;
	FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel);
	ffbFillPolyHelper (pDrawable, pGC, maxPixels, 
		y, h, left, right, nleft, nright);
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
	    ffbLineArc (pDrawable, pGC, maxPixels,
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
ffbWideDash (pDrawable, pGC, mode, npt, pPts)
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
    ffbScreenPrivPtr    scrPriv;
    FFB             ffb;
    long	    maxPixels;
    long	    bgPixel, fgPixel;
    FFBMode	    fillMode;

    if (npt == 0)
	return;

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDrawable->pScreen, pDrawable, scrPriv, ffb, pGC);

    FFBFILLMODE(pDrawable, pGC, TRUE, maxPixels,
		{ cfbPrivGCPtr cfbPriv = CFBGCPRIV(pGC);
		  fgPixel = cfbPriv->xor; bgPixel = cfbPriv->and; },
		{ fgPixel = pGC->fgPixel; bgPixel = pGC->bgPixel; },
		fillMode = ffbFillMode;
    );
    CYCLE_REGS(ffb);
    FFBMODE(ffb, fillMode);
    FFBDATA(ffb, ~0);

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
	    ffbWideDashSegment (pDrawable, pGC, fillMode, maxPixels,
				&dashOffset, &dashIndex, x1, y1, x2, y2,
				projectLeft, projectRight,
				&leftFace, &rightFace);
	    startIsFg = !(prevDashIndex & 1);
	    endIsFg = (dashIndex & 1) ^ (dashOffset != 0);
	    if (pGC->lineStyle == LineDoubleDash || startIsFg)
	    {
	    	pixel = startIsFg ? fgPixel : bgPixel;
		FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel);
		if (first || (pGC->lineStyle == LineOnOffDash && !prevIsFg))
	    	{
	    	    if (first && selfJoin)
		    {
		    	firstFace = leftFace;
			firstIsFg = startIsFg;
		    }
	    	    else if (pGC->capStyle == CapRound)
		    	ffbLineArc(pDrawable, pGC, maxPixels,
			       	   &leftFace, (LineFacePtr) NULL,
			       	   (double)0.0, (double)0.0, TRUE);
	    	}
	    	else
	    	{
	    	    ffbLineJoin (pDrawable, pGC, maxPixels, &leftFace,
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
		pixel = endIsFg ? fgPixel : bgPixel;
		FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel);
		if (selfJoin && (pGC->lineStyle == LineDoubleDash || firstIsFg))
		{
		    ffbLineJoin (pDrawable, pGC, maxPixels, &firstFace,
				&rightFace);
		}
		else 
		{
		    if (pGC->capStyle == CapRound)
			ffbLineArc (pDrawable, pGC, maxPixels,
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
		    pixel = fgPixel;
		    FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel);
		    if (pGC->capStyle == CapProjecting)
		        ffbLineProjectingCap (pDrawable, pGC, maxPixels,
				    &firstFace, TRUE,
				    (double)0.0, (double)0.0, TRUE);
		    else if (pGC->capStyle == CapRound)
			ffbLineArc (pDrawable, pGC, maxPixels,
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
	pixel = (dashIndex & 1) ? bgPixel : fgPixel;
	FFBLOADPIXEL(ffb, pDrawable, fillMode, pixel);
	projectLeft = pGC->capStyle == CapProjecting;
	ffbWideDashSegment (pDrawable, pGC, fillMode, maxPixels,
			    &dashOffset, &dashIndex,
			    x1, y1, x2, y2,
			    projectLeft, projectRight, &leftFace, &rightFace);
	if (pGC->capStyle == CapRound)
	{
	    ffbLineArc (pDrawable, pGC, maxPixels,
		       &leftFace, (LineFacePtr) NULL,
		       (double)0.0, (double)0.0,
		       TRUE);
	    rightFace.dx = -1;	/* sleezy hack to make it work */
	    ffbLineArc (pDrawable, pGC, maxPixels,
		       (LineFacePtr) NULL, &rightFace,
 		       (double)0.0, (double)0.0,
		       TRUE);
	}
    }

    scrPriv->lastGC = (GCPtr)NULL;

}

/*
 * HISTORY
 */
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbwideline.c,v 1.1.2.2 1993/11/19 21:19:05 Robert_Lembree Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

