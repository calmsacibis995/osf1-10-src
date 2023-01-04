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
#define IDENT "X-1"
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991, 1992 BY            *
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

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/sfb/sfbline.c,v 1.1.5.2 1994/01/10 22:08:02 Madeline_Barcia Exp $ */

/* Handles 0-width solid PolyLine and PolySegment requests via conditional
   compilation.  For lines with 1 clip rectangle, see the sfbsline.c file.

An SFB Bresenham step looks like:

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

#include "sfb.h"
#include "sfbline.h"

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

DIVISION

When clipping the lines, we want to round the answer, rather than truncating.
We want to avoid floating point; we also want to avoid the special code
required when the dividend and divisor have different signs.

We work a little to make all the numbers in the division positive, then use the
signs of the major and minor axes to decide whether to add or subtract.  This
takes the special-case code out of the rounding division (making it easier for
a compiler or inline to do something clever).

CEILING

Sometimes we want the ceiling.  ceil(m/n) == floor((m+n-1)/n), for n > 0.  In C,
integer division results in floor.

MULTIPLICATION
 
When multiplying by signdx or signdy, we KNOW that it will be a multiplication
by 1 or -1, but most compilers can't figure this out.  If your
compiler/hardware combination does better at the ?: operator and 'move negated'
instructions that it does at multiplication, you should consider using the
alternate macros.

*/

#define round(dividend, divisor) \
	(((dividend) + (divisor) / 2) / (divisor))

#define ceiling(m,n) ( ((m) + (n) - 1) / (n) )

#define SignTimes(sign, n) ((sign) * ((int)(n)))
/*
#define SignTimes(sign, n) \
    ( ((sign)<0) ? -(n) : (n) )
*/

#ifdef TLBFAULTS
#define SFBSOLIDLINE(sfb, pdst, len)	\
{					\
    int data, align, tlen;		\
    Pixel8 *p;				\
    SFBADDRESS(sfb, pdst+SFBSCRPRIVOFF);\
    data = 0xffff;			\
    SFBBRESCONTINUE(sfb, data);		\
    tlen = len - SFBLINEBITS;		\
    while (tlen > 0) {			\
	SFBBRESCONTINUE(sfb, data);	\
	tlen -= SFBLINEBITS;		\
    }					\
} /* SFBSOLIDLINE */

#else
#define SFBSOLIDLINE(sfb, pdst, len)    \
{					\
    int data, align, tlen;		\
    Pixel8 *p;				\
    p = (pdst);				\
    align = (int)p & (SFBBUSBYTES-1);	\
    p -= align;				\
    data = (align << 16) | 0xffff;	\
    SFBWRITE(p, data);			\
    tlen = len - SFBLINEBITS;		\
    while (tlen > 0) {			\
	SFBBRESCONTINUE(sfb, data);	\
	tlen -= SFBLINEBITS;		\
    }					\
} /* SFBSOLIDLINE */
#endif


#ifdef SEGMENTS
void sfbSegmentSS(pDrawable, pGC, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    nels;	/* number of segments			*/
    xSegment	    *elInit;    /* segment list				*/
#else
void sfbLineSS(pDrawable, pGC, mode, nels, elInit)
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

    register int    pt1x, pt1y;
    register int    pt2x, pt2y;

    Pixel8	    *addrl;	/* address of byte containing first point   */
    int		    nlwidth;	/* width in bytes of destination bitmap     */
    int		    snlwidth;   /* width multiplied by sign bit		*/

    int		    xorg, yorg;	/* origin of window			*/

    register int    dx;		/* x2 - x1				*/
    register int    dy;		/* y2 - y1				*/

    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

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

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
    SFBMODE(sfb, TRANSPARENTLINE);


    if (nboxInit <= 4) {
	/* Offset base address into the drawable */
	addrl += yorg * nlwidth + xorg * SFBPIXELBYTES;
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
	    sfbSegS1(sfb, addrl, nlwidth, elInit, nels, &tbox, capEnds,
			SFBSCRPRIVOFF);
#else
	    sfbLineS1(sfb, addrl, nlwidth, elInit, nels, &tbox,
			SFBSCRPRIVOFF);
#endif
	    pbox++;
	    nbox--;
	    addrl = CYCLE_FB(addrl);
	} while (nbox != 0);
#ifndef SEGMENTS
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
		addrl += pt2y * nlwidth + pt2x * SFBPIXELBYTES;
		pt2x += xorg;
		pt2y += yorg;
		do {
		    if ((pt2x >= pboxInit->x1) && (pt2y >= pboxInit->y1) &&
		        (pt2x < pboxInit->x2) && (pt2y < pboxInit->y2)) {
#ifdef PARTIALWRITES
			SFBMODE(sfb, SIMPLE);
#else
			SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
			addrl = CYCLE_FB(addrl);
			SFBPOINT(addrl, pGC->fgPixel);
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
			addrl = CYCLE_FB(addrl);
			SFBMODE(sfb, TRANSPARENTSTIPPLE);
			SFBSOLIDSPAN(sfb,
			    addrl + pt1y*nlwidth + x1 * SFBPIXELBYTES, dx,
			    SFBSTIPPLEALL1);
			SFBMODE(sfb, TRANSPARENTLINE);
		    }
		    nbox--;
		    pbox++;
		}
	    }

	} else {	/* sloped or vertical line */
	    int		    signdx;	/* sign of dx and dy		*/
	    int		    signdy;
	    register int    e, e1, e2;  /* Bresenham error and incs     */
	    int		    a1, a2;     /* Bresenham address incs       */
	    Bool	    verticalish;/* major axis			*/
	    register int    len;	/* length of segment		*/
	    int		    ymin;

	    signdx = 1;
	    if (dx < 0) {
		dx = -dx;
		signdx = -signdx;
	    }
	    signdy = 1;
	    if (dy < 0) {
		snlwidth = -nlwidth;
		dy = -dy;
		signdy = -1;
		ymin = pt2y;
	    } else {
		snlwidth = nlwidth;
		ymin = pt1y;
	    }

	    verticalish = (dx < dy);
	    if (verticalish) {
		len = dy;
		e1  = dx;
		e   = (signdy < 0);
		a1  = snlwidth;
	    } else {
		len = dx;
		e1  = dy;
		e   = (signdx < 0);
		a1  = signdx * SFBPIXELBYTES;
	    }
	    e2  = len - e1;
	    e = (e1 - e2 - e) >> 1;
	    a2  = snlwidth + signdx * SFBPIXELBYTES;

	    /* we have bresenham parameters and two points.
	       all we have to do now is clip and draw.  Load up e1, a1, e2, a2,
	       as they will remain constant across all line drawing.
	    */
	    SFBBRES1(sfb, (a1 << 16) | e1);
	    SFBBRES2(sfb, (a2 << 16) | e2);

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

		addrl = CYCLE_FB(addrl);
		BOTHOUTCODES(oc1, oc2, pt1x, pt1y, pt2x, pt2y, pbox);
		if ((oc1 | oc2) == 0) {
		    /* Completely inside this clip rectangle */
#ifdef SEGMENTS
		    len += capEnds;
#endif
		    SFBBRES3(sfb, (e << 15) | len);
		    SFBSOLIDLINE(sfb, addrl + pt1y*nlwidth + pt1x*SFBPIXELBYTES,
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
				    err += clipdy*e1 - clipdx*(e1+e2);
				else
				    err += clipdx*e1 - clipdy*(e1+e2);
			    }
			    SFBBRES3(sfb, (err << 15) | len);
			    SFBSOLIDLINE(sfb,
				addrl + clippt1.y * nlwidth 
				+ clippt1.x * SFBPIXELBYTES, len);
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
		addrl += pt1y * nlwidth + pt1x * SFBPIXELBYTES;
		addrl = CYCLE_FB(addrl);
#ifdef PARTIALWRITES
		SFBMODE(sfb, SIMPLE);
#else
		SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
		SFBPOINT(addrl, pGC->fgPixel);
		break;
	    }
	    else
		pbox++;
	}
    }
#endif /* SEGMENTS */
}


#define DashedBres(sfb, len, pDash, numDashes, major, minor, which)	    \
{									    \
    register int    tlenDB;	    /* Length for current dash	*/	    \
    register Bits32 dashBitsDB;						    \
									    \
    /* Set up for first 1-SFBLINEBITS bits */				    \
    tlenDB = ((len-1) & SFBLINEBITSMASK) + 1;				    \
    GetDashBits(tlenDB, dashBitsDB, pDash, numDashes, major, minor, which); \
    dashBitsDB >>= (SFBLINEBITS - tlenDB);				    \
    SFBBRESCONTINUE(sfb, dashBitsDB);					    \
									    \
    /* Now paint rest of bits in groups of SFBLINEBITS */		    \
    len -= SFBLINEBITS;							    \
    while (len > 0) {							    \
	GetDashBits(SFBLINEBITS, dashBitsDB, pDash, numDashes,		    \
	    major, minor, which);					    \
	SFBBRESCONTINUE(sfb, dashBitsDB);				    \
	len -= SFBLINEBITS;						    \
    }									    \
} /* DashedBres */

    
#ifdef SEGMENTS
void sfbDashSegment(pDrawable, pGC, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    nels;	/* number of segments			*/
    xSegment	    *elInit;    /* segment list				*/
#else
void sfbDashLine(pDrawable, pGC, mode, nels, elInit)
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
    int		    snlwidth;   /* width multiplied by sign bit		*/
    int		    xorg, yorg;	/* origin of window			*/

    register int    dx;		/* x2 - x1				*/
    register int    dy;		/* y2 - y1				*/

    int		    signdx;	/* sign of dx and dy			*/
    int		    signdy;
    register int    e, e1, e2;  /* bresenham error and increments       */
    int		    a1, a2;     /* Bresenham address incs		*/
    Bool	    verticalish;/* major axis				*/
    register int    len;	/* length of segment			*/
    int		    savelen;    /* save for moving to next phase	*/

    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;

    if (nels == 0) return;

    gcPriv   = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    nboxInit = REGION_NUM_RECTS(gcPriv->pCompositeClip);
    if (nboxInit == 0) return;
    pboxInit = REGION_RECTS(gcPriv->pCompositeClip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    
    DrawableBaseAndWidth(pDrawable, addrl, nlwidth);

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
    /* Which line style ? */
    if (pGC->lineStyle == LineOnOffDash) {
	SFBMODE(sfb, TRANSPARENTLINE);
    } else {
	SFBMODE(sfb, OPAQUELINE);
    }
	
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

	signdx = 1;
	if (dx < 0) {
	    dx = -dx;
	    signdx = -1;
	}
	signdy = 1;
	snlwidth = nlwidth;
	if (dy < 0) {
	    dy = -dy;
	    signdy = -1;
	    snlwidth = -snlwidth;
	}

	verticalish = (dx < dy);
	if (verticalish) {
	    len = dy;
	    e1 = dx;
	    e = (signdy < 0);
	    a1 = snlwidth;
	} else {
	    len = dx;
	    e1 = dy;
	    e = (signdx < 0);
	    a1 = signdx * SFBPIXELBYTES;
	}
	e2 = len - e1;
	e = (e1 - e2 - e) >> 1;
	a2 = snlwidth + signdx * SFBPIXELBYTES;
	savelen = len;

	/* we have bresenham parameters and two points.
	   all we have to do now is clip and draw.  Load up e1, a1, e2, a2,
	   as they will remain constant across all line drawing.
	*/
	SFBBRES1(sfb, (a1 << 16) | e1);
	SFBBRES2(sfb, (a2 << 16) | e2);

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
		    SFBBRES3(sfb, (e << 15) | len);
		    SFBADDRESS(sfb, addrl + pt1y*nlwidth + pt1x*SFBPIXELBYTES
		    	+ SFBSCRPRIVOFF);
		    DashedBres(sfb, len, pDash, numDashes, major, minor, which);
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
				err += clipdy*e1 - clipdx*(e1+e2);
				NewDashPos(&dashDesc, &clipDashPos, clipdy);
			    } else {
				err += clipdx*e1 - clipdy*(e1+e2);
				NewDashPos(&dashDesc, &clipDashPos, clipdx);
			    }
			}
			SFBBRES3(sfb, (err << 15) | len);
			SFBADDRESS(sfb, addrl + clippt1.y*nlwidth 
				    + clippt1.x*SFBPIXELBYTES +
				    SFBSCRPRIVOFF);
			DashedBres(sfb, len, pDash, numDashes, 
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
		addrl += pt1y * nlwidth + pt1x * SFBPIXELBYTES;
		addrl = CYCLE_FB(addrl);
#ifdef PARTIALWRITES
		SFBMODE(sfb, SIMPLE);
#else
		SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
		SFBPOINT(addrl, pixel);
		break;
	    }
	    else
		pbox++;
	}
    }
#endif /* SEGMENTS */
}


#define DashedBres16(sfb, len, dPattern, dLen, dOffset, dAdvance)	    \
{									    \
    /* Paint first 1-SFBLINEBITS bits */				    \
    SFBBRESCONTINUE(sfb, dPattern >> dOffset);				    \
									    \
    len -= SFBLINEBITS;							    \
    if (len > 0) {							    \
	/* Compute dOffset = (dOffset + length painted) % dLen */	    \
	dOffset += ((len-1) & SFBLINEBITSMASK) + 1;			    \
	while (dOffset > dLen) dOffset -= dLen;				    \
									    \
	/* Paint rest of line.  Easier, because can use dAdvance. */	    \
	do {								    \
	    SFBBRESCONTINUE(sfb, dPattern >> dOffset);			    \
	    dOffset += dAdvance;					    \
	    if (dOffset > dLen) dOffset -= dLen;			    \
	    len -= SFBLINEBITS;						    \
	} while (len > 0);						    \
    }									    \
} /* DashedBres16 */

    
#ifdef SEGMENTS
void sfbDashSegment16(pDrawable, pGC, nels, elInit)
    DrawablePtr     pDrawable;
    register GCPtr  pGC;
    int		    nels;	/* number of segments			*/
    xSegment	    *elInit;    /* segment list				*/
#else
void sfbDashLine16(pDrawable, pGC, mode, nels, elInit)
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
    xSegment	    *pel;
#else
    DDXPointPtr     pel;	/* temp points into elInit		*/
#endif

    register int    pt1x, pt1y;
    register int    pt2x, pt2y;

    Pixel8	    *addrl;	/* address of byte containing first point*/
    int		    nlwidth;	/* width in bytes of destination bitmap   */
    int		    snlwidth;   /* width multiplied by sign bit		*/
    int		    xorg, yorg;	/* origin of window			*/

    register int    dx;		/* x2 - x1				*/
    register int    dy;		/* y2 - y1				*/

    int		    signdx;	/* sign of dx and dy			*/
    int		    signdy;
    register int    e, e1, e2;  /* bresenham error and increments       */
    int		    a1, a2;     /* Bresenham address incs		*/
    Bool	    verticalish;/* major axis				*/
    register int    len;	/* length of segment			*/
    int		    savelen;    /* save for moving to next phase	*/

    sfbScreenPrivPtr scrPriv;
    SFB		    sfb;
    sfbGCPrivPtr    sfbGCPriv;

    if (nels == 0) return;

    gcPriv   = (cfbPrivGC *)(pGC->devPrivates[cfbGCPrivateIndex].ptr);
    nboxInit = REGION_NUM_RECTS(gcPriv->pCompositeClip);
    if (nboxInit == 0) return;
    pboxInit = REGION_RECTS(gcPriv->pCompositeClip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    
    DrawableBaseAndWidth(pDrawable, addrl, nlwidth);

    CHECKSTATE(pDrawable->pScreen, scrPriv, sfb, pGC);
    /* Which line style ? */
    if (pGC->lineStyle == LineOnOffDash) {
	SFBMODE(sfb, TRANSPARENTLINE);
    } else {
	SFBMODE(sfb, OPAQUELINE);
    }
	
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

    /* Set up initial dashed line offset conditions specified in gc */
    sfbGCPriv = SFBGCPRIV(pGC);
    dPattern = sfbGCPriv->dashPattern;
    dAdvance = sfbGCPriv->dashAdvance;
    dLen     = sfbGCPriv->dashLength;
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

	addrl += yorg * nlwidth + xorg * SFBPIXELBYTES;
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
	SFBADDRESS(sfb, addrl + pel->y*nlwidth + pel->x * SFBPIXELBYTES
		+ SFBSCRPRIVOFF);
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
	    clipCheck |= (pt1x-clipul) | (cliplr-pt1x);
#else
	    if (pt1MayBeOut != 0) {
		clipCheck |= (pt1x-clipul) | (cliplr-pt1x);
	    }
#endif
	    pt1x = ((short *)pel)[0];
	    pt2x = ((short *)pel)[2];
	    pel++;
#ifdef SEGMENTS
	    SFBADDRESS(sfb, addrl + pt1y * nlwidth + pt1x * SFBPIXELBYTES
		+ SFBSCRPRIVOFF);
#endif

	    dx = pt2x - pt1x;
	    dy = pt2y - pt1y;
    
	    pt1MayBeOut = clipCheck & signbits;
	    if (pt1MayBeOut == 0) {
		/* Okay, line is completely in clip rectangle. */
		signdx = SFBPIXELBYTES << 16;
		signdy = 1;
		if (dx < 0) {
		    dx = -dx;
		    signdx = -signdx;
		}
		snlwidth = (nlwidth << 16);
		if (dy < 0) {
		    dy = -dy;
		    signdy = -signdy;
		    snlwidth = -snlwidth;
		}
		e2 = dx - dy;
		if (e2 >= 0) {
		    /* Horizontalish line */
		    e = (dy - e2 - (signdx < 0)) << 14;
		    SFBBRES1(sfb, signdx | dy);
		    SFBBRES2(sfb, (snlwidth+signdx) | e2);
#ifdef SEGMENTS
		    len = dx + capEnds;
#else
		    len = dx;
#endif
		} else {
		    /* Verticalish line */
		    e2 = -e2;
		    e = (dx - e2 - (signdy < 0)) << 14;
		    SFBBRES1(sfb, snlwidth | dx);
		    SFBBRES2(sfb, (snlwidth + signdx) | e2);
#ifdef SEGMENTS
		    len = dy + capEnds;
#else
		    len = dy;
#endif
		} /* if horizontalish else verticalish */
		if (len > 0) {
		    SFBBRES3(sfb, e | len);
#ifdef SEGMENTS
		    dOffset = dOffsetInit;
#endif
		    DashedBres16(sfb, len, dPattern, dLen, dOffset, dAdvance);
		}
	    } else {
		unsigned    oc1;	/* outcode of point 1 */
		unsigned    oc2;	/* outcode of point 2 */

		signdx = 1;
		if (dx < 0) {
		    dx = -dx;
		    signdx = -1;
		}
		signdy = 1;
		snlwidth = (nlwidth << 16);
		if (dy < 0) {
		    dy = -dy;
		    signdy = -1;
		    snlwidth = -snlwidth;
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
			signdx = signdx << SFBLINESHIFT;
			e2 = dx - dy;
			if (e2 >= 0) {
			    /* Horizontalish */
			    SFBBRES1(sfb, signdx | dy);
			    SFBBRES2(sfb, (snlwidth + signdx) | e2);
			    e = dy - e2;
			    len = clippt2.x - pt2x;
			} else {
			    /* Verticalish */
			    SFBBRES1(sfb, snlwidth | dx);
			    SFBBRES2(sfb, (snlwidth + signdx) | -e2);
			    e = dx + e2;
			    len = clippt2.y - pt2y;
			}
			if (len < 0) {
			    e -= 1;
			    len = -len;
			}
			e >>= 1;
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
			   better leave the sfb address register filled 
			   correctly, because the NEXT line may not be
			   clipped at ALL! */
			SFBADDRESS(sfb,
			    addrl + pt2y*nlwidth + pt2x * SFBPIXELBYTES
				+ SFBSCRPRIVOFF);
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
				if (e2 >= 0) {
				    /* Horizontalish */
				    e += pt1x*dy - pt1y*dx;
				    dOffset = (dOffset + pt1x) % dLen;
				} else {
				    /* Verticalish */
				    e += pt1y*dx - pt1x*dy;
				    dOffset = (dOffset + pt1y) % dLen;
				}
			    }
			    SFBBRES3(sfb, (e << 15) | len);
			    SFBADDRESS(sfb, addrl + clippt1.y*nlwidth 
					    + clippt1.x*SFBPIXELBYTES +
					    SFBSCRPRIVOFF);
			    DashedBres16(sfb, len, 
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
		addrl += pt2y * nlwidth + pt2x * SFBPIXELBYTES;
		addrl = CYCLE_FB(addrl);
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
			SFBMODE(sfb, SIMPLE);
#else
			SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
			SFBPOINT(addrl, pixel);
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
    
	    signdx = 1;
	    if (dx < 0) {
		dx = -dx;
		signdx = -1;
	    }
	    signdy = 1;
	    snlwidth = nlwidth;
	    if (dy < 0) {
		dy = -dy;
		signdy = -1;
		snlwidth = -snlwidth;
	    }
    
	    verticalish = (dx < dy);
	    if (verticalish) {
		len = dy;
		e1 = dx;
		e = (signdy < 0);
		a1 = snlwidth;
	    } else {
		len = dx;
		e1 = dy;
		e = (signdx < 0);
		a1 = signdx * SFBPIXELBYTES;
	    }
	    e2 = len - e1;
	    e = (e1 - e2 - e) >> 1;
	    a2 = snlwidth + signdx * SFBPIXELBYTES;
	    savelen = len;
    
	    /* we have bresenham parameters and two points.
	       all we have to do now is clip and draw.  Load up e1, a1, e2, a2,
	       as they will remain constant across all line drawing.
	    */
	    SFBBRES1(sfb, (a1 << 16) | e1);
	    SFBBRES2(sfb, (a2 << 16) | e2);
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
			SFBBRES3(sfb, (e << 15) | len);
			SFBADDRESS(sfb, addrl + pt1y*nlwidth 
			    + pt1x*SFBPIXELBYTES +
			    SFBSCRPRIVOFF);
			DashedBres16(
			    sfb, len, dPattern, dLen, dOffset, dAdvance);
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
				    err += clipdy*e1 - clipdx*(e1+e2);
				    dOffset = (dOffset + clipdy) % dLen;
				} else {
				    err += clipdx*e1 - clipdy*(e1+e2);
				    dOffset = (dOffset + clipdx) % dLen;
				}
			    }
			    SFBBRES3(sfb, (err << 15) | len);
			    SFBADDRESS(sfb, addrl + clippt1.y*nlwidth 
					    + clippt1.x*SFBPIXELBYTES +
					    SFBSCRPRIVOFF);
			    DashedBres16(sfb, len, 
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
		    addrl += pt1y * nlwidth + pt1x * SFBPIXELBYTES;
		    addrl = CYCLE_FB(addrl);
#ifdef PARTIALWRITES
		    SFBMODE(sfb, SIMPLE);
#else
		    SFBMODE(sfb, TRANSPARENTSTIPPLE);
#endif
		    SFBPOINT(addrl, pixel);
		    break;
		}
		else
		    pbox++;
	    }
	}
#endif /* SEGMENTS */
    } /* end 1 clip rectangle else many */
}
