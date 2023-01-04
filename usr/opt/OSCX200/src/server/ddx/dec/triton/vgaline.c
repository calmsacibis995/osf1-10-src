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
static char *rcsid = "@(#)$RCSfile: vgaline.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/11/22 17:34:44 $";
#endif

#include "X.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"

#ifdef __alpha
typedef unsigned char BITMAP_UNIT;
#endif

#define OUTCODES(result, x, y, pbox) \
    if (x < pbox->x1) \
	result |= OUT_LEFT; \
    else if (x >= pbox->x2) \
	result |= OUT_RIGHT; \
    if (y < pbox->y1) \
	result |= OUT_ABOVE; \
    else if (y >= pbox->y2) \
	result |= OUT_BELOW;

#define round(dividend, divisor) \
( (((dividend)<<1) + (divisor)) / ((divisor)<<1) )
#define ceiling(m,n)  (((m)-1)/(n) + 1)

/*
#define SignTimes(sign, n) ((sign) * ((int)(n)))
*/

#define SignTimes(sign, n) \
    ( ((sign)<0) ? -(n) : (n) )

#define SWAPINT(i, j) \
{  register int _t = i; \
   i = j; \
   j = _t; \
}

#define SWAPPT(i, j) \
{  DDXPointRec _t; \
   _t = i; \
   i = j; \
   j = _t; \
}
   

void
#ifdef POLYSEGMENT
vgaSegmentSS (pDrawable, pGC, nseg, pSeg)
    DrawablePtr	pDrawable;
    GCPtr	pGC;
    int		nseg;
    register xSegment	*pSeg;
#else
vgaLineSS (pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int		mode;		/* Origin or Previous */
    int		npt;		/* number of points */
    DDXPointPtr pptInit;
#endif
{
    int nboxInit;
    register int nbox;
    BoxPtr pboxInit;
    register BoxPtr pbox;
#ifndef POLYSEGMENT
    register DDXPointPtr ppt;	/* pointer to list of translated points */
#endif

    BITMAP_UNIT oc1;		/* outcode of point 1 */
    BITMAP_UNIT oc2;		/* outcode of point 2 */

    int xorg, yorg;		/* origin of window */

    int adx;		/* abs values of dx and dy */
    int ady;
    int signdx;		/* sign of dx and dy */
    int signdy;
    int e, e1, e2;		/* bresenham error and increments */
    int len;			/* length of segment */
    int axis;			/* major axis */

				/* a bunch of temporaries */
    register int y1, y2;
    register int x1, x2;
    RegionPtr cclip;
    int first;

    FillSolidFuncPtr FillSolidFunc
      = ((DrawFuncs *)(pGC->pScreen->devPrivate))->FillSolidFunc;

    BresSFuncPtr BresSFunc
      = ((DrawFuncs *)(pGC->pScreen->devPrivate))->BresSFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;

    HideCursorInLineFuncPtr HideCursorInLineFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInLineFunc;
#endif

    unsigned int fg  = (unsigned int) pGC->fgPixel;
    unsigned int alu = pGC->alu;
    unsigned int pm  = (unsigned int) pGC->planemask;

    cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifdef POLYSEGMENT
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
    while(--npt)
#endif
    {
	nbox = nboxInit;
	pbox = pboxInit;

#ifdef POLYSEGMENT
	x1 = pSeg->x1 + xorg;
	y1 = pSeg->y1 + yorg;
	x2 = pSeg->x2 + xorg;
	y2 = pSeg->y2 + yorg;
	pSeg++;
#else
	x1 = x2;
	y1 = y2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
#endif

	if (x1 == x2)
	{
	    /* make the line go top to bottom of screen, keeping
	       endpoint semantics
	    */
	    if (y1 > y2)
	    {
		register int tmp;

		tmp = y2;
		y2 = y1 + 1;
		y1 = tmp + 1;
#ifdef POLYSEGMENT
		if (pGC->capStyle != CapNotLast)
		    y1--;
#endif
	    }
#ifdef POLYSEGMENT
	    else if (pGC->capStyle != CapNotLast)
		y2++;
#endif
	    /* get to first band that might contain part of line */
	    while ((nbox) && (pbox->y2 <= y1))
	    {
		pbox++;
		nbox--;
	    }

	    if (nbox)
	    {
		first = TRUE;
		/* stop when lower edge of box is beyond end of line */
		while((nbox) && (y2 >= pbox->y1))
		{
		    if ((x1 >= pbox->x1) && (x1 < pbox->x2))
		    {
			int y1t, y2t;
			/* this box has part of the line in it */
			y1t = max(y1, pbox->y1);
			y2t = min(y2, pbox->y2);
			if (y1t != y2t)
			{
			  /* y2 is always greater than y1 */
#ifdef SOFTWARE_CURSOR
			  (*HideCursorInXYWHFunc)(x1, y1t, 1, y2t-y1t);
#endif
			  (*FillSolidFunc)(pGC->pScreen, fg, alu, pm, x1, y1t, 
					   1, y2t-y1t, first);
			  first = FALSE;
#ifdef SOFTWARE_CURSOR
			  (*ShowCursorFunc)();
#endif
			}
		    }
		    nbox--;
		    pbox++;
		}
	    }
#ifndef POLYSEGMENT
	    y2 = ppt->y + yorg;
#endif
	}
	else if (y1 == y2)
	{
	    /* force line from left to right, keeping
	       endpoint semantics
	    */
	    if (x1 > x2)
	    {
		register int tmp;

		tmp = x2;
		x2 = x1 + 1;
		x1 = tmp + 1;
#ifdef POLYSEGMENT
		if (pGC->capStyle != CapNotLast)
		    x1--;
#endif
	    }
#ifdef POLYSEGMENT
	    else if (pGC->capStyle != CapNotLast)
		x2++;
#endif

	    /* find the correct band */
	    while( (nbox) && (pbox->y2 <= y1))
	    {
		pbox++;
		nbox--;
	    }

	    /* try to draw the line, if we haven't gone beyond it */
	    if ((nbox) && (pbox->y1 <= y1))
	    {
		int tmp;

		first = TRUE;
		/* when we leave this band, we're done */
		tmp = pbox->y1;
		while((nbox) && (pbox->y1 == tmp))
		{
		    int	x1t, x2t;

		    if (pbox->x2 <= x1)
		    {
			/* skip boxes until one might contain start point */
			nbox--;
			pbox++;
			continue;
		    }

		    /* stop if left of box is beyond right of line */
		    if (pbox->x1 >= x2)
		    {
			nbox = 0;
			break;
		    }

		    x1t = max(x1, pbox->x1);
		    x2t = min(x2, pbox->x2);
		    if (x1t != x2t)
		    {
		      /* x2 is always greater than x1 */
#ifdef SOFTWARE_CURSOR
		      (*HideCursorInXYWHFunc)(x1t, y1, x2t-x1t, 1);
#endif
		      (*FillSolidFunc)(pGC->pScreen, fg, alu, pm, x1t, y1, 
					x2t-x1t, 1, first);
		      first = FALSE;
#ifdef SOFTWARE_CURSOR
		      (*ShowCursorFunc)();
#endif
		    }
		    nbox--;
		    pbox++;
		}
	    }
#ifndef POLYSEGMENT
	    x2 = ppt->x + xorg;
#endif
	}
	else	/* sloped line */
	{
	    adx = x2 - x1;
	    ady = y2 - y1;
	    signdx = sign(adx);
	    signdy = sign(ady);
	    adx = abs(adx);
	    ady = abs(ady);

	    if (adx > ady)
	    {
		axis = X_AXIS;
		e1 = ady << 1;
		e2 = e1 - (adx << 1);
		e = e1 - adx;

	    }
	    else
	    {
		axis = Y_AXIS;
		e1 = adx << 1;
		e2 = e1 - (ady << 1);
		e = e1 - ady;
	    }

	    /* we have bresenham parameters and two points.
	       all we have to do now is clip and draw.
	    */

	    while(nbox--)
	    {
		oc1 = 0;
		oc2 = 0;
		OUTCODES(oc1, x1, y1, pbox);
		OUTCODES(oc2, x2, y2, pbox);
		if ((oc1 | oc2) == 0)
		{
		    if (axis == X_AXIS)
			len = adx;
		    else
			len = ady;
#ifdef POLYSEGMENT
		    if (pGC->capStyle != CapNotLast)
			len++;
#endif
#ifdef SOFTWARE_CURSOR
		    (*HideCursorInLineFunc)(x1, y1, x2, y2);
#endif
		    (*BresSFunc)(pGC->pScreen, fg, alu, pm, signdx, signdy, 
				 axis, x1, y1, e, e1, e2, len);
#ifdef SOFTWARE_CURSOR
		    (*ShowCursorFunc)();
#endif
		    break;
		}
		else if (oc1 & oc2)
		{
		    pbox++;
		}
		else
		{
	    	    /*
	     	     * let the mfb helper routine do our work;
	     	     * better than duplicating code...
	     	     */
	    	    BoxRec box;
    	    	    DDXPointRec pt1Copy;	/* clipped start point */
    	    	    DDXPointRec pt2Copy;	/* clipped end point */
    	    	    int err;			/* modified bresenham error term */
    	    	    int clip1, clip2;		/* clippedness of the endpoints */
    	    	
    	    	    int clipdx, clipdy;		/* difference between clipped and
				       	       	   unclipped start point */
		    DDXPointRec	pt1;
    	    	
    	
	    	    pt1.x = pt1Copy.x = x1;
		    pt1.y = pt1Copy.y = y1;
	    	    pt2Copy.x = x2;
		    pt2Copy.y = y2;
	    	    box.x1 = pbox->x1;
	    	    box.y1 = pbox->y1;
	    	    box.x2 = pbox->x2-1;
	    	    box.y2 = pbox->y2-1;
	    	    clip1 = 0;
	    	    clip2 = 0;
    	
		    if (mfbClipLine (pbox, box,
				     &pt1, &pt1Copy, &pt2Copy, 
				     adx, ady, signdx, signdy, axis,
				     &clip1, &clip2) == 1)
		    {
		    	if (axis == X_AXIS)
			    len = abs(pt2Copy.x - pt1Copy.x);
		    	else
			    len = abs(pt2Copy.y - pt1Copy.y);
    
#ifdef POLYSEGMENT
		    	if (clip2 != 0 || pGC->capStyle != CapNotLast)
			    len++;
#else
		    	len += (clip2 != 0);
#endif
		    	if (len)
		    	{
			    /* unwind bresenham error term to first point */
			    if (clip1)
			    {
			    	clipdx = abs(pt1Copy.x - x1);
			    	clipdy = abs(pt1Copy.y - y1);
			    	if (axis == X_AXIS)
				    err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			    	else
				    err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
			    }
			    else
			      err = e;
#ifdef SOFTWARE_CURSOR
			    (*HideCursorInLineFunc)(pt1Copy.x, pt1Copy.y,
						    pt2Copy.x, pt2Copy.y);
#endif
			    (*BresSFunc)(pGC->pScreen, fg, alu, pm, signdx, 
					 signdy, axis, pt1Copy.x, pt1Copy.y,
					 err, e1, e2, len);
#ifdef SOFTWARE_CURSOR
			    (*ShowCursorFunc)();
#endif
		    	}
		    }
		    pbox++;
		}
	    } /* while (nbox--) */
	} /* sloped line */
    } /* while (nline--) */

#ifndef POLYSEGMENT

    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
	((ppt->x != pptInit->x) ||
	 (ppt->y != pptInit->y) ||
	 (ppt == pptInit + 1)))
    {
	nbox = nboxInit;
	pbox = pboxInit;
	first = TRUE;
	while (nbox--)
	{
	    if ((x2 >= pbox->x1) &&
		(y2 >= pbox->y1) &&
		(x2 <  pbox->x2) &&
		(y2 <  pbox->y2))
	    {
#ifdef SOFTWARE_CURSOR
	      (*HideCursorInXYWHFunc)(x2, y2, 1, 1);
#endif
	      (*FillSolidFunc)(pGC->pScreen, fg, alu, pm, x2, y2, 1, 1, first);
	      first = FALSE;
#ifdef SOFTWARE_CURSOR
	      (*ShowCursorFunc)();
#endif
	      break;
	    }
	    else
		pbox++;
	}
    }
#endif
}


/*
 * Draw dashed 1-pixel lines.
 */

void
#ifdef POLYSEGMENT
vgaSegmentSD (pDrawable, pGC, nseg, pSeg)
    DrawablePtr	pDrawable;
    register GCPtr	pGC;
    int		nseg;
    register xSegment	*pSeg;
#else
vgaLineSD( pDrawable, pGC, mode, npt, pptInit)
    DrawablePtr pDrawable;
    register GCPtr pGC;
    int mode;		/* Origin or Previous */
    int npt;		/* number of points */
    DDXPointPtr pptInit;
#endif
{
    int nboxInit;
    register int nbox;
    BoxPtr pboxInit;
    register BoxPtr pbox;
#ifndef POLYSEGMENT
    register DDXPointPtr ppt;	/* pointer to list of translated points */
#endif

    register BITMAP_UNIT oc1;	/* outcode of point 1 */
    register BITMAP_UNIT oc2;	/* outcode of point 2 */

    int xorg, yorg;		/* origin of window */

    int adx;		/* abs values of dx and dy */
    int ady;
    int signdx;		/* sign of dx and dy */
    int signdy;
    int e, e1, e2;		/* bresenham error and increments */
    int len;			/* length of segment */
    int axis;			/* major axis */
    int x1, x2, y1, y2;
    RegionPtr cclip;
    unsigned char   *pDash;
    int		    dashOffset;
    int		    numInDashList;
    int		    dashIndex;
    int		    isDoubleDash;
    int		    dashIndexTmp, dashOffsetTmp;
    int		    unclippedlen;

    FillSolidFuncPtr FillSolidFunc
      = ((DrawFuncs *)(pGC->pScreen->devPrivate))->FillSolidFunc;

    BresOnOffFuncPtr BresOnOffFunc
      = ((DrawFuncs *)(pGC->pScreen->devPrivate))->BresOnOffFunc;

    BresDoubleFuncPtr BresDoubleFunc
      = ((DrawFuncs *)(pGC->pScreen->devPrivate))->BresDoubleFunc;

#ifdef SOFTWARE_CURSOR
    ShowCursorFuncPtr ShowCursorFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->ShowCursorFunc;
  
    HideCursorInXYWHFuncPtr HideCursorInXYWHFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInXYWHFunc;

    HideCursorInLineFuncPtr HideCursorInLineFunc =
      ((DrawFuncs *)(pGC->pScreen->devPrivate))->HideCursorInLineFunc;
#endif

    unsigned int fg  = (unsigned int) pGC->fgPixel;
    unsigned int bg  = (unsigned int) pGC->bgPixel;
    unsigned int alu = pGC->alu;
    unsigned int pm  = (unsigned int) pGC->planemask;

    cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;
    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    /* compute initial dash values */
     
    pDash = (unsigned char *) pGC->dash;
    numInDashList = pGC->numInDashList;
    isDoubleDash = (pGC->lineStyle == LineDoubleDash);
    dashIndex = 0;
    dashOffset = 0;
    miStepDash ((int)pGC->dashOffset, &dashIndex, pDash,
		numInDashList, &dashOffset);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifdef POLYSEGMENT
    while (nseg--)
#else
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
    while(--npt)
#endif
    {
	nbox = nboxInit;
	pbox = pboxInit;

#ifdef POLYSEGMENT
	x1 = pSeg->x1 + xorg;
	y1 = pSeg->y1 + yorg;
	x2 = pSeg->x2 + xorg;
	y2 = pSeg->y2 + yorg;
	pSeg++;
#else
	x1 = x2;
	y1 = y2;
	++ppt;
	if (mode == CoordModePrevious)
	{
	    xorg = x1;
	    yorg = y1;
	}
	x2 = ppt->x + xorg;
	y2 = ppt->y + yorg;
#endif

	adx = x2 - x1;
	ady = y2 - y1;
	signdx = sign(adx);
	signdy = sign(ady);
	adx = abs(adx);
	ady = abs(ady);

	if (adx > ady)
	{
	    axis = X_AXIS;
	    e1 = ady << 1;
	    e2 = e1 - (adx << 1);
	    e = e1 - adx;
	    unclippedlen = adx;
	}
	else
	{
	    axis = Y_AXIS;
	    e1 = adx << 1;
	    e2 = e1 - (ady << 1);
	    e = e1 - ady;
	    unclippedlen = ady;
	}

	/* we have bresenham parameters and two points.
	   all we have to do now is clip and draw.
	*/

	while(nbox--)
	{
	    oc1 = 0;
	    oc2 = 0;
	    OUTCODES(oc1, x1, y1, pbox);
	    OUTCODES(oc2, x2, y2, pbox);
	    if ((oc1 | oc2) == 0)
	    {
#ifdef POLYSEGMENT
		if (pGC->capStyle != CapNotLast)
		    unclippedlen++;
		dashIndexTmp = dashIndex;
		dashOffsetTmp = dashOffset;
#ifdef SOFTWARE_CURSOR
		(*HideCursorInLineFunc)(x1, y1, x2, y2);
#endif
		if (isDoubleDash)
		  (*BresDoubleFunc) (pGC->pScreen, fg, bg, alu, pm,
				     &dashIndexTmp, pDash, numInDashList,
				     &dashOffsetTmp,
				     signdx, signdy, axis, x1, y1,
				     e, e1, e2, unclippedlen);
		else
		  (*BresOnOffFunc) (pGC->pScreen, fg, alu, pm,
				    &dashIndexTmp, pDash, numInDashList,
				    &dashOffsetTmp,
				    signdx, signdy, axis, x1, y1,
				    e, e1, e2, unclippedlen);
#ifdef SOFTWARE_CURSOR
		(*ShowCursorFunc)();
#endif
		break;
#else
#ifdef SOFTWARE_CURSOR
		(*HideCursorInLineFunc)(x1, y1, x2, y2);
#endif
		if (isDoubleDash)
		  (*BresDoubleFunc) (pGC->pScreen, fg, bg, alu, pm,
				     &dashIndex, pDash, numInDashList,
				     &dashOffset,
				     signdx, signdy, axis, x1, y1,
				     e, e1, e2, unclippedlen);
		else
		  (*BresOnOffFunc) (pGC->pScreen, fg, alu, pm,
				    &dashIndex, pDash, numInDashList,
				    &dashOffset,
				    signdx, signdy, axis, x1, y1,
				    e, e1, e2, unclippedlen);
#ifdef SOFTWARE_CURSOR
		(*ShowCursorFunc)();
#endif
		goto dontStep;
#endif
	    }
	    else if (oc1 & oc2)
	    {
		pbox++;
	    }
	    else /* have to clip */
	    {
		/*
		 * let the mfb helper routine do our work;
		 * better than duplicating code...
		 */
		BoxRec box;
		DDXPointRec pt1Copy;	/* clipped start point */
		DDXPointRec pt2Copy;	/* clipped end point */
		int err;			/* modified bresenham error term */
		int clip1, clip2;		/* clippedness of the endpoints */
	    
		int clipdx, clipdy;		/* difference between clipped and
					       unclipped start point */
		DDXPointRec	pt1;
    
		pt1.x = pt1Copy.x = x1;
		pt1.y = pt1Copy.y = y1;
		pt2Copy.x = x2;
		pt2Copy.y = y2;
		box.x1 = pbox->x1;
		box.y1 = pbox->y1;
		box.x2 = pbox->x2-1;
		box.y2 = pbox->y2-1;
		clip1 = 0;
		clip2 = 0;
    
		if (mfbClipLine (pbox, box,
				       &pt1, &pt1Copy, &pt2Copy, 
				       adx, ady, signdx, signdy, axis,
				       &clip1, &clip2) == 1)
		{
    
		    dashIndexTmp = dashIndex;
		    dashOffsetTmp = dashOffset;
		    if (clip1)
		    {
		    	int dlen;
    
		    	if (axis == X_AXIS)
			    dlen = abs(pt1Copy.x - x1);
		    	else
			    dlen = abs(pt1Copy.y - y1);
		    	miStepDash (dlen, &dashIndexTmp, pDash,
				    numInDashList, &dashOffsetTmp);
		    }
		    if (axis == X_AXIS)
		    	len = abs(pt2Copy.x - pt1Copy.x);
		    else
		    	len = abs(pt2Copy.y - pt1Copy.y);
    
#ifdef POLYSEGMENT
		    if (clip2 != 0 || pGC->capStyle != CapNotLast)
		    	len++;
#else
		    len += (clip2 != 0);
#endif
		    if (len)
		    {
		    	/* unwind bresenham error term to first point */
		    	if (clip1)
		    	{
			    clipdx = abs(pt1Copy.x - x1);
			    clipdy = abs(pt1Copy.y - y1);
			    if (axis == X_AXIS)
			    	err = e+((clipdy*e2) + ((clipdx-clipdy)*e1));
			    else
			    	err = e+((clipdx*e2) + ((clipdy-clipdx)*e1));
		    	}
		    	else
			    err = e;
#ifdef SOFTWARE_CURSOR
			(*HideCursorInLineFunc)(pt1Copy.x, pt1Copy.y,
						pt2Copy.x, pt2Copy.y);
#endif
			if (isDoubleDash)
			  (*BresDoubleFunc) (pGC->pScreen, fg, bg, alu, pm,
					     &dashIndexTmp, pDash,
					     numInDashList,
					     &dashOffsetTmp,
					     signdx, signdy, axis,
					     pt1Copy.x, pt1Copy.y,
					     err, e1, e2, len);
			else
			  (*BresOnOffFunc) (pGC->pScreen, fg, alu, pm,
					    &dashIndexTmp, pDash,
					    numInDashList,
					    &dashOffsetTmp,
					    signdx, signdy, axis,
					    pt1Copy.x, pt1Copy.y,
					    err, e1, e2, len);
#ifdef SOFTWARE_CURSOR
			(*ShowCursorFunc)();
#endif
		    }
		}
		pbox++;
	    }
	} /* while (nbox--) */
#ifndef POLYSEGMENT
	/*
	 * walk the dash list around to the next line
	 */
	miStepDash (unclippedlen, &dashIndex, pDash,
		    numInDashList, &dashOffset);
dontStep:	;
#endif
    } /* while (nline--) */

#ifndef POLYSEGMENT
    /* paint the last point if the end style isn't CapNotLast.
       (Assume that a projecting, butt, or round cap that is one
        pixel wide is the same as the single pixel of the endpoint.)
    */

    if ((pGC->capStyle != CapNotLast) &&
        ((dashIndex & 1) == 0 || isDoubleDash) &&
	((ppt->x != pptInit->x) ||
	 (ppt->y != pptInit->y) ||
	 (ppt == pptInit + 1)))
    {
	nbox = nboxInit;
	pbox = pboxInit;
	while (nbox--)
	{
	    if ((x2 >= pbox->x1) &&
		(y2 >= pbox->y1) &&
		(x2 <  pbox->x2) &&
		(y2 <  pbox->y2))
	    {
#ifdef SOFTWARE_CURSOR
	      (*HideCursorInXYWHFunc)(x2, y2, 1, 1);
#endif
	      if (isDoubleDash && (dashIndex & 1))
		(*FillSolidFunc)(pGC->pScreen, bg, alu, pm, x2, y2, 1, 1, TRUE);
	      else
		(*FillSolidFunc)(pGC->pScreen, fg, alu, pm, x2, y2, 1, 1, TRUE);
#ifdef SOFTWARE_CURSOR
	      (*ShowCursorFunc)();
#endif
	      break;
	    }
	    else
		pbox++;
	}
    }
#endif
}
