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
static char *rcsid = "@(#)$RCSfile: ffbline.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:11:30 $";
#endif
/*
 */

/* Handles 0-width solid PolyLine and PolySegment requests via conditional
   compilation.  For lines with 1 clip rectangle, see the ffbsline.c file.

An FFB Bresenham step looks like:

    if (e < 0) {
	address += a1;
	e += e1;
    } else {
	address += a2;
	e -= e2;
    }

For a mostly horizontal line, where 0 <= slope <= 1 (0 <= dy <= dx <= 2^16-1),
initialization of e, e1, and e2 is:

    e1 = dy;
    e2 = dx - dy;
    e = (2*dy - dx) >> 1;

For lines where dx < 0 or dy < 0, we always use absolute values, with some
possible fudging of e by -1 to make a line paint the same pixels backward or
forward.

For e1, we have the range:
    0 <= e1 <= 2^16-1, so an unsigned 16-bit number.

For e2, we have the range:
    0 <= e2 <= 2^16-1, so an unsigned 16-bit number.

For e, with fudging, we have the initial range:
    -(2^15)-1 <= e <= 2^15-1, almost a signed 16-bit number

As time goes on, or if clipping, we have the larger range
    -(2^16-1) <= e <= 2^16-2, a signed 17-bit number.

*/


#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#ifdef MITR5
#include "cfbdecline.h"
#else
#include "cfbline.h"
#endif

#include "ffb.h"
#include "ffbline.h"

/* Single-pixel lines for an 8-bit frame buffer

HORIZONTAL LINES

Draw horizontal lines left to right; if necessary, swap endpoints (and move
right by one).  Horizontal lines are confined to a single band of a region, so
clipping against multiple clip rectangles is reasonably easy.  Find the band
(if one exists); then find the first box in that band that contains part of the
line.  Clip the line to subsequent boxes in that band.

SLOPED AND VERTICAL LINES

When clipping a sloped line, bring the second point inside the clipping box,
rather than one beyond it, and then add 1 to the length of the line before
drawing it.  This lets us use the same box for finding the outcodes for both
endpoints.  Since the equation for clipping the second endpoint to an edge
gives us 1 beyond the edge, we then have to move the point towards the first
point by one step on the major axis.  The method uses Cohen-Sutherland outcodes
to determine outsideness, and a method similar to Pike's layers for doing the
actual clipping.

*/


#ifdef TLBFAULTS
#define FFBSOLIDLINE(ffb, pdst, len)	\
{					\
    int data, align, tlen;		\
    Pixel8 *p;				\
    CYCLE_REGS(ffb); 			\
    FFBADDRESS(ffb, pdst);		\
    data = 0xffff;			\
    FFBCONTINUE(ffb, data);		\
    tlen = len - FFBLINEBITS;		\
    while (tlen > 0) {			\
	CYCLE_REGS(ffb);		\
	FFBCONTINUE(ffb, data);		\
	tlen -= FFBLINEBITS;		\
    }					\
} /* FFBSOLIDLINE */

#else
#define FFBSOLIDLINE(ffb, pdst, len)    \
{					\
    int data, align, tlen;		\
    Pixel8 *p;				\
    p = (pdst);				\
    align = (long)p & (FFBBUSBYTES-1);	\
    p -= align;				\
    data = (align << 16) | 0xffff;	\
    FFBWRITE(p, data);			\
    tlen = len - FFBLINEBITS;		\
    while (tlen > 0) {			\
	CYCLE_REGS(ffb);		\
	FFBCONTINUE(ffb, data);		\
	tlen -= FFBLINEBITS;		\
    }					\
} /* FFBSOLIDLINE */
#endif


#ifdef SEGMENTS
void ffbSegmentSS(pDrawable, pGC, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    nels;	/* number of segments			*/
    xSegment	    *elInit;    /* segment list				*/
#else
void ffbLineSS(pDrawable, pGC, mode, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    mode;	/* Origin or Previous			*/
    int		    nels;	/* number of points			*/
    DDXPointPtr     elInit;     /* point list				*/
#endif
{
    cfbPrivGC	    *gcPriv;    /* Private GC info			*/
    int		    nboxInit;
    register int    nbox;
    BoxPtr	    pboxInit;
    register BoxPtr pbox;
#ifdef SEGMENTS
    int		    capEnds;
    xSegment	    *pel;
#else
    DDXPointPtr     pel;	/* temp points into elInit		*/
#endif

    long	    pt1x, pt1y;
    long	    pt2x, pt2y;

    Pixel8	    *addrl;	/* address of byte containing first point   */
    int		    nlwidth;	/* width in bytes of destination bitmap     */

    int		    xorg, yorg;	/* origin of window			*/

    register int    dx;		/* x2 - x1				*/
    register int    dy;		/* y2 - y1				*/
    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;
    CommandWord     data;
    volatile CommandWord     *slp_ngo_addr;
    FFBMode	    fillmode;
    long	    maxPixels;

    if (nels == 0) return;


    gcPriv   = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);

    nboxInit = REGION_NUM_RECTS(gcPriv->pCompositeClip);
    if (nboxInit == 0) return;
    pboxInit = REGION_RECTS(gcPriv->pCompositeClip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    DrawableBaseAndWidth(pDrawable, addrl, nlwidth);

#ifdef SEGMENTS
    /* Do we paint x2,y2 on each segment? */
    capEnds = (pGC->capStyle != CapNotLast);
#else
    /* Translate the point list to CoordModeOrigin if CoordModePrevious */
    if (mode == CoordModePrevious) {
	register int nelsTmp = nels;
	register DDXPointPtr pp = elInit;
	register int x, y;

	x = 0;
	y = 0;
	do {
	    pp->x = (x += pp->x);
	    pp->y = (y += pp->y);
	    pp++;
	    nelsTmp--;
	} while (nelsTmp > 0);
    }
#endif

#ifndef SEGMENTS
    nels--;
#endif

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDrawable->pScreen, pDrawable, scrPriv, ffb, pGC);

    FFBFILLMODE(pDrawable, pGC, TRUE, maxPixels, /**/,/**/,
		fillmode=ffbFillMode);

#ifndef SEGMENTS
    FFBMODE(ffb, TRANSPARENTLINE);
#else
    FFBMODE(ffb, TRANSPARENTLINE | CAPENDSBIT(capEnds));
#endif

    FFBLOADCOLORREGS(ffb, gcPriv->xor, pDrawable->depth);
    FFBDATA(ffb, ~0);
    FFBBRESWIDTH(ffb, nlwidth/FFBPIXELBYTES);

    if (nboxInit <= 4) {
	/* Offset base address into the drawable */
	addrl += yorg * nlwidth + xorg * FFBPIXELBYTES;
	pbox = pboxInit;
	nbox = nboxInit;
	do {
	    BoxRec  tbox;	/* Translated clip rectangle		*/
    
	    /* Translate clip rectangle */
	    tbox.x1 = pbox->x1 - xorg;
	    tbox.y1 = pbox->y1 - yorg;
	    tbox.x2 = pbox->x2 - xorg;
	    tbox.y2 = pbox->y2 - yorg;

#ifdef SEGMENTS
	    ffbSegS1(ffb, addrl, nlwidth, elInit, nels, &tbox, fillmode,
		    maxPixels, capEnds);
#else
    	    ffbLineS1(ffb, addrl, nlwidth, elInit, nels, &tbox, fillmode,
		    maxPixels);
#endif
	    pbox++;
	    nbox--;
	} while (nbox != 0);
#ifndef SEGMENTS
	WRITE_MEMORY_BARRIER();
	/* paint the last point if the end style isn't CapNotLast.
	   (Assume that a projecting, butt, or round cap that is one
	    pixel wide is the same as the single pixel of the endpoint.)
	*/
	if (pGC->capStyle != CapNotLast) {
	    pel = elInit + nels;
	    pt2x = pel->x;
	    pt2y = pel->y;
	    if ((pt2x != elInit->x) | (pt2y != elInit->y) | (nels == 1)) {
		/* Really want to cap it.  Is it unclipped? */
		addrl += pt2y * nlwidth + pt2x * FFBPIXELBYTES;
		pt2x += xorg;
		pt2y += yorg;
		do {
		    if ((pt2x >= pboxInit->x1) && (pt2y >= pboxInit->y1) &&
		        (pt2x < pboxInit->x2) && (pt2y < pboxInit->y2)) {
			CYCLE_REGS(ffb);
#ifdef PARTIALWRITES
			FFBMODE(ffb, SIMPLE);
#else
			FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif
			addrl = CYCLE_FB(addrl);
			FFBPOINT(addrl, pGC->fgPixel);
			break;
		    }
		    pboxInit++;
		    nboxInit--;
		} while (nboxInit != 0);
	    }
	}
#endif /* Cap end for Polyline */
	return;
    }

    pel = elInit;
    while (nels) {
	nels--;

#ifdef SEGMENTS
	pt1x = pel->x1 + xorg;
	pt1y = pel->y1 + yorg;
	pt2x = pel->x2 + xorg;
	pt2y = pel->y2 + yorg;
	pel++;
#else
	pt1x = pel->x + xorg;
	pt1y = pel->y + yorg;
	pel++;
	pt2x = pel->x + xorg;
	pt2y = pel->y + yorg;
#endif

	dx = pt2x - pt1x;
	dy = pt2y - pt1y;

	if (dy == 0) {
	    /* force line from left to right, keeping endpoint semantics */
#ifdef SEGMENTS
	    if (dx < 0) {
		register int t;
		t = pt2x;
		pt2x = pt1x + 1;
		pt1x = t + 1 - capEnds;
	    } else {
		pt2x += capEnds;
	    }
#else
	    if (dx < 0) {
		register int t;
		t = pt2x;
		pt2x = pt1x + 1;
		pt1x = t + 1;
	    }
#endif
	    /* find the correct band */
	    nbox = nboxInit;
	    pbox = pboxInit;
	    while((nbox) && (pbox->y2 <= pt1y)) {
		pbox++;
		nbox--;
	    }

	    /* try to draw the line, if we haven't gone beyond it */
	    if ((nbox) && (pbox->y1 <= pt1y)) {
		/* when we leave this band, we're done */
		register int bandy1;
		bandy1 = pbox->y1;
		while((nbox) && (pbox->y1 == bandy1)) {
		    register int x1, x2;

		    if (pbox->x2 <= pt1x) {
			/* skip boxes until one might contain start point */
			nbox--;
			pbox++;
			continue;
		    }

		    /* stop if left of box is beyond right of line */
		    if (pbox->x1 >= pt2x) {
			nbox = 0;
			continue;
		    }

		    x1 = max(pt1x, pbox->x1);
		    x2 = min(pt2x, pbox->x2);
		    dx = x2 - x1;

		    if (dx > 0) {
			CYCLE_REGS(ffb);
			FFBMODE(ffb, fillmode);
			addrl = CYCLE_FB(addrl);
			FFBSOLIDSPAN(ffb,
				     addrl + pt1y*nlwidth + x1 * FFBPIXELBYTES,
				     dx, maxPixels);
			CYCLE_REGS(ffb);
#ifndef SEGMENTS
			FFBMODE(ffb, TRANSPARENTLINE);
#else
			FFBMODE(ffb, TRANSPARENTLINE | CAPENDSBIT(capEnds));
#endif
		    }
		    nbox--;
		    pbox++;
		}
	    }

	} else {	/* sloped or vertical line */
	    int		    signdx;	/* sign of dx and dy		*/
	    int		    signdy;
	    register int    e;		/* Bresenham error 		*/
	    Bool	    verticalish;/* major axis			*/
	    register int    len;	/* length of segment		*/
	    int		    ymin;

	    CYCLE_REGS(ffb);    /* For eventual FFBWRITE(slp_ngo_addr, ...) */
	    slp_ngo_addr = &ffb->sng_dx_gt_dy;
	    signdx = 1;
	    if (dx < 0) {
		dx = -dx;
		signdx = -signdx;
                slp_ngo_addr -= 2;
	    }
	    signdy = 1;
	    if (dy < 0) {
		dy = -dy;
		signdy = -1;
		ymin = pt2y;
                slp_ngo_addr -= 1;
	    } else {
		ymin = pt1y;
	    }

	    verticalish = (dx < dy);
	    if (verticalish) {
		len = dy;
		e   = ((2*dx)-dy-(signdy < 0)) >> 1;
                slp_ngo_addr -= 4;
	    } else {
		len = dx;
		e   = ((2*dy)-dx-(signdx < 0)) >> 1;
	    }

            FFBWRITE(slp_ngo_addr , FFBLINEDXDY(dx,dy));

	    /* Skip over clip boxes that are above line */
	    nbox = nboxInit;
	    pbox = pboxInit;
	    while (nbox != 0 && pbox->y2 < ymin) {
		pbox++;
		nbox--;
	    }

	    while(nbox--) {
		register unsigned int oc1;	/* outcode of point 1 */
		register unsigned int oc2;	/* outcode of point 2 */

		BOTHOUTCODES(oc1, oc2, pt1x, pt1y, pt2x, pt2y, pbox);
		if ((oc1 | oc2) == 0) {
		    /* Completely inside this clip rectangle */
#ifdef SEGMENTS
		    len += capEnds;
#endif
		    CYCLE_REGS(ffb);
		    FFBBRES3(ffb, (e << 15) | len);
		    addrl = CYCLE_FB(addrl);
		    FFBSOLIDLINE(ffb, addrl + pt1y*nlwidth + pt1x*FFBPIXELBYTES,
				len);
		    /* Skip remaining rectangles */
		    break;

		} else if ((oc1 & oc2 ) == 0) {
		    /* have to clip */

		    DDXPointRec clippt1, clippt2;   /* clipped endpoints  */

		    clippt1.x = pt1x;
		    clippt1.y = pt1y;
		    clippt2.x = pt2x;
		    clippt2.y = pt2y;

		    if (cfbClipLine(pbox, &clippt1, &clippt2, oc1, oc2,
			dx, dy, signdx, signdy) > 0){

			if (verticalish)
			    len = clippt2.y - clippt1.y;
			else
			    len = clippt2.x - clippt1.x;
			if (len < 0) len = -len;
#ifdef SEGMENTS
			len += ((oc2 != 0) | capEnds);
#else
			len += (oc2 != 0);
#endif    
			if (len) {
			    /* unwind bresenham error term to first point
			       Verticalish: e + clipdy*dx - clipdx*dy
			       Horizontal:  e + clipdx*dy - clipdy*dx
			       We express in terms of e1 and e2, since those
			       are needed anyway.  dx and dy don't need to be
			       restored if we don't use them */
			    int err = e;
			    if (oc1) {
				register int clipdx, clipdy;
    
				/* difference between clipped and
				   unclipped start point */
				clipdx = clippt1.x - pt1x;
				if (clipdx < 0) clipdx = -clipdx;
				clipdy = clippt1.y - pt1y;
				if (clipdy < 0) clipdy = -clipdy;
				if (verticalish)
				    err += clipdy*dx - clipdx*dy;
				else
				    err += clipdx*dy - clipdy*dx;
			    }
			    CYCLE_REGS(ffb);
			    FFBBRES3(ffb, (err << 15) | len);
			    addrl = CYCLE_FB(addrl);
			    FFBSOLIDLINE(ffb,
				addrl + clippt1.y * nlwidth 
				+ clippt1.x * FFBPIXELBYTES, len);
			}
		    }
		} else {
		    /* Are both endpoints of line above clip box? */
		    if (oc1 & oc2 & OUT_ABOVE) break;
		}
		pbox++;
	    } /* while (nbox--) */
	} /* sloped line */
    } /* while points */;

#ifndef SEGMENTS
    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if (pGC->capStyle != CapNotLast &&
	((pel->x != elInit->x) ||
	 (pel->y != elInit->y) ||
	 (pel == elInit + 1))) {
	pt1x = pel->x + xorg;
	pt1y = pel->y + yorg;

	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--) {
	    if ((pt1x >= pbox->x1) &&
		(pt1y >= pbox->y1) &&
		(pt1x <  pbox->x2) &&
		(pt1y <  pbox->y2)) {
		addrl += pt1y * nlwidth + pt1x * FFBPIXELBYTES;
		CYCLE_REGS(ffb);
#ifdef PARTIALWRITES
		FFBMODE(ffb, SIMPLE);
#else
		FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif
		addrl = CYCLE_FB(addrl);
		FFBPOINT(addrl, pGC->fgPixel);
		break;
	    }
	    else
		pbox++;
	}
    }
#endif /* SEGMENTS */
}


#define DashedBres(ffb, len, pDash, numDashes, major, minor, which)	    \
{									    \
    register int    tlenDB;	    /* Length for current dash	*/	    \
    register Bits32 dashBitsDB;						    \
									    \
    /* Set up for first 1-FFBLINEBITS bits */				    \
    tlenDB = ((len-1) & FFBLINEBITSMASK) + 1;				    \
    GetDashBits(tlenDB, dashBitsDB, pDash, numDashes, major, minor, which); \
    dashBitsDB >>= (FFBLINEBITS - tlenDB);				    \
    FFBCONTINUE(ffb, dashBitsDB);					    \
									    \
    /* Now paint rest of bits in groups of FFBLINEBITS */		    \
    len -= FFBLINEBITS;							    \
    while (len > 0) {							    \
	GetDashBits(FFBLINEBITS, dashBitsDB, pDash, numDashes,		    \
	    major, minor, which);					    \
        CYCLE_REGS(ffb);						    \
	FFBCONTINUE(ffb, dashBitsDB);					    \
	len -= FFBLINEBITS;						    \
    }									    \
} /* DashedBres */

    
#ifdef SEGMENTS
void ffbDashSegment(pDrawable, pGC, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    nels;	/* number of segments			*/
    xSegment	    *elInit;    /* segment list				*/
#else
void ffbDashLine(pDrawable, pGC, mode, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    mode;	/* Origin or Previous			*/
    int		    nels;	/* number of points			*/
    DDXPointPtr     elInit;     /* point list				*/
#endif
{
    cfbPrivGC	    *gcPriv;    /* Private GC info			*/
    int		    nboxInit;
    register int    nbox;
    BoxPtr	    pboxInit;
    register BoxPtr pbox;

    DashDesc	    dashDesc;
    DashPos	    dashPos;
    
    unsigned char   *pDash;
    int		    numDashes;
    int		    major, minor, which;

#ifdef SEGMENTS
    int		    capEnds;
    xSegment	    *pel;
#else
    DDXPointPtr     pel;	/* temp points into elInit		*/
#endif

    register int    pt1x, pt1y;
    register int    pt2x, pt2y;

    Pixel8	    *addrl;	/* address of byte containing first point*/
    int		    nlwidth;	/* width in bytes of destination bitmap   */
    int		    xorg, yorg;	/* origin of window			*/

    register int    dx;		/* x2 - x1				*/
    register int    dy;		/* y2 - y1				*/

    int		    signdx;	/* sign of dx and dy			*/
    int		    signdy;
    register int    e;		/* bresenham error 			*/
    Bool	    verticalish;/* major axis				*/
    register int    len;	/* length of segment			*/
    int		    savelen;    /* save for moving to next phase	*/

    ffbScreenPrivPtr scrPriv;
    CommandWord	    data;
    volatile CommandWord     *slp_ngo_addr;
    FFB		    ffb;
   
    if (nels == 0) return;

    gcPriv   = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    nboxInit = REGION_NUM_RECTS(gcPriv->pCompositeClip);
    if (nboxInit == 0) return;
    pboxInit = REGION_RECTS(gcPriv->pCompositeClip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    
    DrawableBaseAndWidth(pDrawable, addrl, nlwidth);

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDrawable->pScreen, pDrawable, scrPriv, ffb, pGC);

    /* Which line style ? */
#ifndef SEGMENTS
    if (pGC->lineStyle == LineOnOffDash) {
        FFBMODE(ffb, TRANSPARENTLINE);
    } else {
	FFBMODE(ffb, OPAQUELINE);
    }
#else
    /* Do we paint x2,y2 on each segment? */
    capEnds = (pGC->capStyle != CapNotLast);
    if (pGC->lineStyle == LineOnOffDash) {
        FFBMODE(ffb, TRANSPARENTLINE | CAPENDSBIT(capEnds));
    } else {
        FFBMODE(ffb, OPAQUELINE | CAPENDSBIT(capEnds));
    }
#endif

    FFBBRESWIDTH(ffb, nlwidth/FFBPIXELBYTES);
	
#ifndef SEGMENTS
    /* Translate the point list to CoordModeOrigin if CoordModePrevious */
    if (mode == CoordModePrevious) {
	register int nelsTmp = nels;
	register DDXPointPtr pp = elInit;
	register int x, y;

	x = 0;
	y = 0;
	do {
	    pp->x = (x += pp->x);
	    pp->y = (y += pp->y);
	    pp++;
	    nelsTmp--;
	} while (nelsTmp > 0);
    }
#endif

    /* Set up initial dashed line offset conditions specified in gc */
    InitDashStuff(&dashDesc, &dashPos, pGC);

    pDash = dashDesc.pDash;
    numDashes = dashDesc.numDashes;
#ifndef SEGMENTS
    /* major, minor, and which are always current values, and in normal course
       of events (non-clipped lines) are updated automatically as the line is
       painted. */
    major = dashPos.major;
    minor = dashPos.minor;
    which = dashPos.which;
    nels--;
#endif
    pel = elInit;
    while (nels) {
	nels--;
	nbox = nboxInit;
	pbox = pboxInit;

#ifdef SEGMENTS
	/* major, minor, and which are reset for each segment */
	major = dashPos.major;
	minor = dashPos.minor;
	which = dashPos.which;
	pt1x = pel->x1 + xorg;
	pt1y = pel->y1 + yorg;
	pt2x = pel->x2 + xorg;
	pt2y = pel->y2 + yorg;
	pel++;
#else
	pt1x = pel->x + xorg;
	pt1y = pel->y + yorg;
	pel++;
	pt2x = pel->x + xorg;
	pt2y = pel->y + yorg;
#endif

	dx = pt2x - pt1x;
	dy = pt2y - pt1y;
	
	CYCLE_REGS(ffb);    /* For eventual FFBWRITE(slp_ngo_addr, ...) */
	slp_ngo_addr = &ffb->sng_dx_gt_dy;
	signdx = 1;
	if (dx < 0) {
	    dx = -dx;
	    signdx = -1;
	    slp_ngo_addr -= 2;
	}
	signdy = 1;
	if (dy < 0) {
	    dy = -dy;
	    signdy = -1;
	    slp_ngo_addr -= 1;
	}

	verticalish = (dx < dy);
	if (verticalish) {
	    len = dy;
	    e   = ((2*dx)-dy-(signdy < 0)) >> 1;
            slp_ngo_addr -= 4;
	} else {
	    len = dx;
	    e   = ((2*dy)-dx-(signdx < 0)) >> 1;
	}
	savelen = len;

        FFBWRITE(slp_ngo_addr , FFBLINEDXDY(dx,dy));

	while (nbox--) {
	    register unsigned int oc1;	/* outcode of point 1 */
	    register unsigned int oc2;	/* outcode of point 2 */

	    if (pt1x > pbox->x1-1 & pt2x > pbox->x1-1 &
		pt1x < pbox->x2   & pt2x < pbox->x2 &
		pt1y > pbox->y1-1 & pt2y > pbox->y1-1 &
		pt1y < pbox->y2   & pt2y < pbox->y2) {
		/* Completely inside this clip rectangle */
#ifdef SEGMENTS
		len += capEnds;
#endif
		if (len > 0) {
		    CYCLE_REGS(ffb);
		    FFBBRES3(ffb, (e << 15) | len);
		    FFBADDRESS(ffb,
			       addrl + pt1y*nlwidth + pt1x * FFBPIXELBYTES);
		    DashedBres(ffb, len, pDash, numDashes, major, minor, which);
		}
		/* Skip remaining rectangles */
		goto LineDrawn;

	    } else if (((oc1 = OUTCODES(oc1, pt1x, pt1y, pbox)) &
			(oc2 = OUTCODES(oc2, pt2x, pt2y, pbox))) == 0) {
		/* have to clip */

		DDXPointRec clippt1, clippt2;   /* clipped endpoints    */

		clippt1.x = pt1x;
		clippt1.y = pt1y;
		clippt2.x = pt2x;
		clippt2.y = pt2y;

		if (cfbClipLine(pbox, &clippt1, &clippt2, oc1, oc2,
			dx, dy, signdx, signdy) > 0) {

		    if (verticalish)
			len = clippt2.y - clippt1.y;
		    else
			len = clippt2.x - clippt1.x;
		    if (len < 0) len = -len;
#ifdef SEGMENTS
		    len += ((oc2 != 0) | capEnds);
#else
		    len += (oc2 != 0);
#endif    
		    if (len) {
			/* unwind bresenham error term to first point */
			DashPos clipDashPos;
			int err = e;

			clipDashPos.major = major;
			clipDashPos.minor = minor;
			clipDashPos.which = which;
			if (oc1) {
			    register int clipdx, clipdy;

			    /* difference between clipped and
			       unclipped start point */
			    clipdx = clippt1.x - pt1x;
			    if (clipdx < 0) clipdx = -clipdx;
			    clipdy = clippt1.y - pt1y;
			    if (clipdy < 0) clipdy = -clipdy;
			    if (verticalish) {
				err += clipdy*dx - clipdx*dy;
				NewDashPos(&dashDesc, &clipDashPos, clipdy);
			    } else {
				err += clipdx*dy - clipdy*dx;
				NewDashPos(&dashDesc, &clipDashPos, clipdx);
			    }
			}
			CYCLE_REGS(ffb);
			FFBBRES3(ffb, (err << 15) | len);
			FFBADDRESS(ffb,
				   addrl + clippt1.y*nlwidth +
				   clippt1.x*FFBPIXELBYTES);
			DashedBres(ffb, len, pDash, numDashes, 
			    clipDashPos.major, clipDashPos.minor, 
			    clipDashPos.which);
		    }
		}
	    }
	    pbox++;
	} /* while (nbox--) */
#ifndef SEGMENTS
	dashPos.major = major;
	dashPos.minor = minor;
	dashPos.which = which;
	NewDashPos(&dashDesc, &dashPos, savelen);
	major = dashPos.major;
	minor = dashPos.minor;
	which = dashPos.which;
#endif
    
LineDrawn:;
    } /* while points */;

#ifndef SEGMENTS
    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
	(pel->x != elInit->x || pel->y != elInit->y || pel == elInit + 1) &&
	(dashPos.which == EVENDASH || pGC->lineStyle == LineDoubleDash)) {

	PixelWord     pixel;

	if (dashPos.which == EVENDASH) {
	    pixel = pGC->fgPixel;
	} else {
	    pixel = pGC->bgPixel;
	}
	pt1x = pel->x + xorg;
	pt1y = pel->y + yorg;

	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--) {
	    if ((pt1x >= pbox->x1) &&
		(pt1y >= pbox->y1) &&
		(pt1x <  pbox->x2) &&
		(pt1y <  pbox->y2)) {
		addrl += pt1y * nlwidth + pt1x * FFBPIXELBYTES;
    		CYCLE_REGS(ffb);
#ifdef PARTIALWRITES
		FFBMODE(ffb, SIMPLE);
#else
		FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif
		addrl = CYCLE_FB(addrl);
		FFBPOINT(addrl, pixel);
		break;
	    }
	    else
		pbox++;
	}
    }
#endif /* SEGMENTS */
}


#define DashedBres16(ffb, len, dPattern, dLen, dOffset, dAdvance)	    \
{									    \
    /* Paint first 1-FFBLINEBITS bits */				    \
    FFBCONTINUE(ffb, dPattern >> dOffset);				    \
									    \
    len -= FFBLINEBITS;							    \
    if (len > 0) {							    \
	/* Compute dOffset = (dOffset + length painted) % dLen */	    \
	dOffset += ((len-1) & FFBLINEBITSMASK) + 1;			    \
	while (dOffset > dLen) dOffset -= dLen;				    \
									    \
	/* Paint rest of line.  Easier, because can use dAdvance. */	    \
	do {								    \
    	    CYCLE_REGS(ffb);						    \
	    FFBCONTINUE(ffb, dPattern >> dOffset);			    \
	    dOffset += dAdvance;					    \
	    if (dOffset > dLen) dOffset -= dLen;			    \
	    len -= FFBLINEBITS;						    \
	} while (len > 0);						    \
    }									    \
} /* DashedBres16 */

    
#ifdef SEGMENTS
void ffbDashSegment16(pDrawable, pGC, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    nels;	/* number of segments			*/
    xSegment	    *elInit;    /* segment list				*/
#else
void ffbDashLine16(pDrawable, pGC, mode, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    mode;	/* Origin or Previous			*/
    int		    nels;	/* number of points			*/
    DDXPointPtr     elInit;     /* point list				*/
#endif
{
    cfbPrivGC	    *gcPriv;    /* Private GC info			*/
    int		    nboxInit;
    register int    nbox;
    BoxPtr	    pboxInit;
    register BoxPtr pbox;

    Bits32	    dPattern;
    int		    dLen, dOffset, dAdvance;
    int		    dOffsetInit;

#ifdef SEGMENTS
    int		    capEnds;
    int		    ymul;
    xSegment	    *pel;
#else
    DDXPointPtr     pel;	/* temp points into elInit		*/
#endif

    long	    pt1x, pt1y;
    long	    pt2x, pt2y;

    Pixel8	    *addrl;	/* address of byte containing first point*/
    int		    nlwidth;	/* width in bytes of destination bitmap   */
    int		    xorg, yorg;	/* origin of window			*/

    register int    dx;		/* x2 - x1				*/
    register int    dy;		/* y2 - y1				*/

    int		    signdx;	/* sign of dx and dy			*/
    int		    signdy;
    register int    e;		/* bresenham error 			*/
    Bool	    verticalish;/* major axis				*/
    register int    len;	/* length of segment			*/
    int		    savelen;    /* save for moving to next phase	*/

    ffbScreenPrivPtr scrPriv;
    FFB		    ffb;
    CommandWord	    data;
    volatile CommandWord	    *slp_ngo_addr;
    ffbGCPrivPtr    ffbGCPriv;

    if (nels == 0) return;

    gcPriv   = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    nboxInit = REGION_NUM_RECTS(gcPriv->pCompositeClip);
    if (nboxInit == 0) return;
    pboxInit = REGION_RECTS(gcPriv->pCompositeClip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    
    DrawableBaseAndWidth(pDrawable, addrl, nlwidth);

    WRITE_MEMORY_BARRIER();
    CHECKSTATE(pDrawable->pScreen, pDrawable, scrPriv, ffb, pGC);
 
    /* Which line style ? */
#ifndef SEGMENTS
    if (pGC->lineStyle == LineOnOffDash) {
        FFBMODE(ffb, TRANSPARENTLINE);
    } else {
	FFBMODE(ffb, OPAQUELINE);
    }
#else
    /* Do we paint x2,y2 on each segment? */
    capEnds = (pGC->capStyle != CapNotLast);
    if (pGC->lineStyle == LineOnOffDash) {
        FFBMODE(ffb, TRANSPARENTLINE | CAPENDSBIT(capEnds));
    } else {
        FFBMODE(ffb, OPAQUELINE | CAPENDSBIT(capEnds));
    }
#endif


    FFBBRESWIDTH(ffb, nlwidth/FFBPIXELBYTES);

#ifndef SEGMENTS
    /* Translate the point list to CoordModeOrigin if CoordModePrevious */
    if (mode == CoordModePrevious) {
	register int nelsTmp = nels;
	register DDXPointPtr pp = elInit;
	register int x, y;

	x = 0;
	y = 0;
	do {
	    pp->x = (x += pp->x);
	    pp->y = (y += pp->y);
	    pp++;
	    nelsTmp--;
	} while (nelsTmp > 0);
    }
#endif

    /* Set up initial dashed line offset conditions specified in gc */
    ffbGCPriv = FFBGCPRIV(pGC);
    dPattern = ffbGCPriv->dashPattern;
    dAdvance = ffbGCPriv->dashAdvance;
    dLen     = ffbGCPriv->dashLength;
#ifdef SEGMENTS
    dOffsetInit = pGC->dashOffset % dLen;
#else
    dOffset  = pGC->dashOffset % dLen;
    nels--;
#endif

    pel = elInit;
    if (nboxInit == 1) {
	/* Special case for 1 clip rectangle */
	BoxRec		tbox;	/* Translated clip rectangle		*/
	int		clipul, cliplr;
	int		signbits = 0x80008000;
	int		clipCheck, pt1MayBeOut;

	addrl += yorg * nlwidth + xorg * FFBPIXELBYTES;
	/* Translate clip rectangle */
	pbox = pboxInit;
	tbox.x1 = pbox->x1 - xorg;
	tbox.y1 = pbox->y1 - yorg;
	tbox.x2 = pbox->x2 - xorg;
	tbox.y2 = pbox->y2 - yorg;
	/* Load clip registers.  Keep x and y in a single 32-bit number. */
	clipul = ((int *)(&tbox))[0];
	cliplr = ((int *)(&tbox))[1] - 0x00010001;
#ifndef SEGMENTS
	CYCLE_REGS(ffb);
	FFBADDRESS(ffb, addrl + pel->y*nlwidth + pel->x * FFBPIXELBYTES);
	pt1MayBeOut = TRUE;
#endif
	while (nels) {
	    nels--;
	    pt1x = ((int *)pel)[0];
	    pt1y = pt1x >> 16;
	    pt2x = ((int *)pel)[1];
	    pt2y = pt2x >> 16;

	    /* Are both points wholly within clip box? */
	    clipCheck = (pt2x-clipul) | (cliplr-pt2x);
#ifdef SEGMENTS
	    ymul = pt1y * nlwidth;
	    clipCheck |= (pt1x-clipul) | (cliplr-pt1x);
#else
	    if (pt1MayBeOut != 0) {
		clipCheck |= (pt1x-clipul) | (cliplr-pt1x);
	    }
#endif
	    pel++;
	    pt1x = (pt1x << (LONG_BIT - 16)) >> (LONG_BIT - 16);
	    pt2x = (pt2x << (LONG_BIT - 16)) >> (LONG_BIT - 16);
	    dx = pt2x - pt1x;
	    dy = pt2y - pt1y;
	    pt1MayBeOut = clipCheck & signbits;
	    if (pt1MayBeOut == 0) {
		/* Okay, line is completely in clip rectangle. */
		CYCLE_REGS(ffb);    /* For eventual slp_ngo_addr write */
#ifdef SEGMENTS
		FFBADDRESS(ffb, addrl + ymul + pt1x * FFBPIXELBYTES); 
#endif

		slp_ngo_addr = &ffb->sng_dx_gt_dy;
		if (dx < 0) {
		    dx = -dx;
		    slp_ngo_addr -= 2;
		}
		if (dy < 0) {
		    dy = -dy;
		    slp_ngo_addr -= 1;
		}
#ifdef SEGMENTS
		len = dx + capEnds;
#else
		len = dx;
#endif
		if (dx <= dy) {
		    /* Verticalish line */
		    slp_ngo_addr -= 4;
#ifdef SEGMENTS
		    len = dy + capEnds;
#else
		    len = dy;
#endif
		}
		/* We had to CYCLE_REG before computing slp_ngo_addr, so we
		   damn well better do a write regardless of len */
		FFBWRITE(slp_ngo_addr, FFBLINEDXDY(dx, dy));
		if (len > 0) {
#ifdef SEGMENTS
		    dOffset = dOffsetInit;
#endif
		    DashedBres16(ffb, len, dPattern, dLen, dOffset, dAdvance);
		}
	    } else {
		unsigned    oc1;	/* outcode of point 1 */
		unsigned    oc2;	/* outcode of point 2 */

		slp_ngo_addr = (CommandWord *)NULL;
		signdx = 1;
		if (dx < 0) {
		    dx = -dx;
		    signdx = -1;
		    slp_ngo_addr -= 2;
		}
		signdy = 1;
		if (dy < 0) {
		    dy = -dy;
		    signdy = -1;
		    slp_ngo_addr -= 1;
		}
#ifndef SEGMENTS
		if (dx > dy) {
		    dOffsetInit = (dOffset + dx) % dLen;
		} else {
		    dOffsetInit = (dOffset + dy) % dLen;
		}
#endif
		BOTHOUTCODES(oc1, oc2, pt1x, pt1y, pt2x, pt2y, &tbox);
		if ((oc1 & oc2) == 0) {
		    /* Have to clip */
		    DDXPointRec clippt1, clippt2;   /* clipped endpoints    */
    
		    clippt1.x = pt1x;
		    clippt1.y = pt1y;
		    clippt2.x = pt2x;
		    clippt2.y = pt2y;
		    if (cfbClipLine(&tbox, &clippt1, &clippt2, oc1, oc2,
			    dx, dy, signdx, signdy) > 0) {
    
			/* pt2 is now the clipped pt1 */
			pt2x = clippt1.x;
			pt2y = clippt1.y;
			signdx = signdx << FFBLINESHIFT;
			if (dx > dy) {
			    /* Horizontalish */
			    e = 2*dy - dx;
			    len = clippt2.x - pt2x;
			} else {
			    /* Verticalish */
			    e = 2*dx - dy;
			    len = clippt2.y - pt2y;
			    slp_ngo_addr -= 4;
			}
			if (len < 0) {
			    e -= 1;
			    len = -len;
			}
			e >>= 1;

			CYCLE_REGS(ffb);    /* For FFBWRITE(slp_ngo_addr, */
			slp_ngo_addr = (CommandWord *)
			    ((long)(slp_ngo_addr) + (long)(&ffb->sng_dx_gt_dy));
			FFBWRITE(slp_ngo_addr, FFBLINEDXDY(dx,dy));
#ifdef SEGMENTS
			len += ((oc2 != 0) | capEnds);
#else
			len += (oc2 != 0);
#endif    
			/* We always set the address register.  If len > 0
			   in the code below, this is the starting point
			   of the clipped line.  If len = 0, then we have
			   a line that just barely extends into the clip
			   region, but we don't paint it because it doesn't
			   have an end cap.  In this case, the starting
			   point and ending point are the same, and we'd 
			   better leave the ffb address register filled 
			   correctly, because the NEXT line may not be
			   clipped at ALL! */
			FFBADDRESS(ffb,
				   addrl + pt2y*nlwidth + pt2x*FFBPIXELBYTES);
			if (len > 0) {
#ifdef SEGMENTS
			    dOffset = dOffsetInit;
#endif
			    if (oc1) {
				/* unwind bresenham error term to first point */
				/* difference between clipped and
				   unclipped start point */
				pt1x = pt2x - pt1x;
				pt1y = pt2y - pt1y;
				if (pt1x < 0) pt1x = -pt1x;
				if (pt1y < 0) pt1y = -pt1y;
				if (dx > dy) {
				    /* Horizontalish */
				    e += pt1x*dy - pt1y*dx;
				    dOffset = (dOffset + pt1x) % dLen;
				} else {
				    /* Verticalish */
				    e += pt1y*dx - pt1x*dy;
				    dOffset = (dOffset + pt1y) % dLen;
				}
			    }
			    CYCLE_REGS(ffb);
			    FFBADDRESS(ffb,
				       addrl + clippt1.y*nlwidth +
				       clippt1.x*FFBPIXELBYTES);
			    FFBBRES3(ffb, (e << 15) | len);
			    DashedBres16(ffb, len, 
				dPattern, dLen, dOffset, dAdvance);
			} /* end if len > 0 */
		    } /* end if cfbClipLine() > 0 */
		} /* end if (oc1 & oc2) == 0 */
#ifndef SEGMENTS
		dOffset = dOffsetInit;
#endif
	    } /* end if unclipped else clipped */
	} /* while points */;

#ifndef SEGMENTS
	/* paint the last point if the end style isn't CapNotLast.
	   (Assume that a projecting, butt, or round cap that is one
	    pixel wide is the same as the single pixel of the endpoint.)
	*/
	if (pGC->capStyle != CapNotLast
	    && ((dPattern & 1) || pGC->lineStyle == LineDoubleDash)) {
	    pel = elInit + nels;
	    pt2x = pel->x;
	    pt2y = pel->y;
	    if ((pt2x != elInit->x) | (pt2y != elInit->y) | (nels == 1)) {
		/* Really want to cap it.  Is it unclipped? */
		addrl += pt2y * nlwidth + pt2x * FFBPIXELBYTES;
		pt2x += xorg;
		pt2y += yorg;
		do {
		    if ((pt2x >= pboxInit->x1) && (pt2y >= pboxInit->y1) &&
		        (pt2x < pboxInit->x2) && (pt2y < pboxInit->y2)) {
			PixelWord     pixel;
			dPattern >>= dOffset;
			if (dPattern & 1) {
			    pixel = pGC->fgPixel;
			} else {
			    pixel = pGC->bgPixel;
			}
#ifdef PARTIALWRITES
			FFBMODE(ffb, SIMPLE);
#else
			FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif
			addrl = CYCLE_FB(addrl);
			FFBPOINT(addrl, pixel);
			break;
		    }
		    pboxInit++;
		    nboxInit--;
		} while (nboxInit != 0);
	    }
	}
#endif /* Cap end for Polyline */

    } else { /* nboxInit > 1 */
	while (nels) {
	    nels--;
#ifdef SEGMENTS
	    pt1x = pel->x1 + xorg;
	    pt1y = pel->y1 + yorg;
	    pt2x = pel->x2 + xorg;
	    pt2y = pel->y2 + yorg;
	    pel++;
#else
	    pt1x = pel->x + xorg;
	    pt1y = pel->y + yorg;
	    pel++;
	    pt2x = pel->x + xorg;
	    pt2y = pel->y + yorg;
#endif
    
	    dx = pt2x - pt1x;
	    dy = pt2y - pt1y;
    
	    CYCLE_REGS(ffb);    /* For eventual FFBWRITE(slp_ngo_addr, ...) */
	    slp_ngo_addr = &ffb->sng_dx_gt_dy;	
	    signdx = 1;
	    if (dx < 0) {
		dx = -dx;
		signdx = -1;
		slp_ngo_addr -= 2;
	    }
	    signdy = 1;
	    if (dy < 0) {
		dy = -dy;
		signdy = -1;
		slp_ngo_addr -=1;
	    }
    
	    verticalish = (dx < dy);
	    if (verticalish) {
		len = dy;
		e = (2*dx - dy - (signdy < 0)) >> 1;
		slp_ngo_addr -= 4;
	    } else {
		len = dx;
		e = (2*dy - dx - (signdx < 0)) >> 1;
	    }
	    savelen = len;

	   FFBWRITE(slp_ngo_addr, FFBLINEDXDY(dx,dy));    
#ifndef SEGMENTS
	    dOffsetInit = dOffset;
#endif
    
	    nbox = nboxInit;
	    pbox = pboxInit;
	    while (nbox--) {
		register unsigned int oc1;	/* outcode of point 1 */
		register unsigned int oc2;	/* outcode of point 2 */
    
		if (pt1x > pbox->x1-1 & pt2x > pbox->x1-1 &
		    pt1x < pbox->x2   & pt2x < pbox->x2 &
		    pt1y > pbox->y1-1 & pt2y > pbox->y1-1 &
		    pt1y < pbox->y2   & pt2y < pbox->y2) {
		    /* Completely inside this clip rectangle */
#ifdef SEGMENTS
		    len += capEnds;
		    dOffset = dOffsetInit;
#endif
		    if (len > 0) {
			CYCLE_REGS(ffb);
		        FFBADDRESS(ffb,
				    addrl + pt1y*nlwidth + pt1x*FFBPIXELBYTES);
			FFBBRES3(ffb, (e << 15)  | len);
			DashedBres16(
			    ffb, len, dPattern, dLen, dOffset, dAdvance);
		    }
		    /* Skip remaining rectangles */
		    goto LineDrawn;
    
		} else if (((oc1 = OUTCODES(oc1, pt1x, pt1y, pbox)) &
			    (oc2 = OUTCODES(oc2, pt2x, pt2y, pbox))) == 0) {
		    /* have to clip */
    
		    DDXPointRec clippt1, clippt2;   /* clipped endpoints    */
    
		    clippt1.x = pt1x;
		    clippt1.y = pt1y;
		    clippt2.x = pt2x;
		    clippt2.y = pt2y;
    
		    if (cfbClipLine(pbox, &clippt1, &clippt2, oc1, oc2,
			    dx, dy, signdx, signdy) > 0) {
    
			if (verticalish)
			    len = clippt2.y - clippt1.y;
			else
			    len = clippt2.x - clippt1.x;
			if (len < 0) len = -len;
#ifdef SEGMENTS
			len += ((oc2 != 0) | capEnds);
#else
			len += (oc2 != 0);
#endif    
			if (len) {
			    /* unwind bresenham error term to first point */
			    int err = e;
			    dOffset = dOffsetInit;
			    if (oc1) {
				register int clipdx, clipdy;
    
				/* difference between clipped and
				   unclipped start point */
				clipdx = clippt1.x - pt1x;
				if (clipdx < 0) clipdx = -clipdx;
				clipdy = clippt1.y - pt1y;
				if (clipdy < 0) clipdy = -clipdy;
				if (verticalish) {
				    err += clipdy*dx - clipdx*dy;
				    dOffset = (dOffset + clipdy) % dLen;
				} else {
				    err += clipdx*dy - clipdy*dx;
				    dOffset = (dOffset + clipdx) % dLen;
				}
			    }
			    CYCLE_REGS(ffb);
			    FFBADDRESS(ffb,
				       addrl + clippt1.y*nlwidth +
				       clippt1.x*FFBPIXELBYTES);
			    FFBBRES3(ffb, (err << 15) | len);
			    DashedBres16(ffb, len, 
				dPattern, dLen, dOffset, dAdvance);
			}
		    }
		}
		pbox++;
	    } /* while (nbox--) */
#ifndef SEGMENTS
	    dOffset = (dOffsetInit + savelen) % dLen;
#endif
	
    LineDrawn:;
	} /* while points */;
    
#ifndef SEGMENTS
	/* paint the last point if the end style isn't CapNotLast.
	   (Assume that a projecting, butt, or round cap that is one
	    pixel wide is the same as the single pixel of the endpoint.)
	*/
    
	dPattern >>= dOffset;
	if ((pGC->capStyle != CapNotLast) &&
	    (pel->x != elInit->x || pel->y != elInit->y || pel == elInit + 1) &&
	    ((dPattern & 1) || pGC->lineStyle == LineDoubleDash)) {
    
	    PixelWord     pixel;
    
	    if (dPattern & 1) {
		pixel = pGC->fgPixel;
	    } else {
		pixel = pGC->bgPixel;
	    }
	    pt1x = pel->x + xorg;
	    pt1y = pel->y + yorg;
    
	    nbox = nboxInit;
	    pbox = pboxInit;
	    while (nbox--) {
		if ((pt1x >= pbox->x1) &&
		    (pt1y >= pbox->y1) &&
		    (pt1x <  pbox->x2) &&
		    (pt1y <  pbox->y2)) {
		    addrl += pt1y * nlwidth + pt1x * FFBPIXELBYTES;
#ifdef PARTIALWRITES
		    FFBMODE(ffb, SIMPLE);
#else
		    FFBMODE(ffb, TRANSPARENTSTIPPLE);
#endif
		    addrl = CYCLE_FB(addrl);
		    FFBPOINT(addrl, pixel);
		    break;
		}
		else
		    pbox++;
	    }
	}
#endif /* SEGMENTS */
    } /* end 1 clip rectangle else many */
}

/*
 * HISTORY
 */

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbline.c,v 1.1.2.2 1993/11/19 21:11:30 Robert_Lembree Exp $ */

