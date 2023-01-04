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
static char *rcsid = "@(#)$RCSfile: wgaline.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/11/22 17:35:45 $";
#endif

/*
 *  WGALINE.C
 *
 *  Created by Fred Kleinsorge on 18-Aug-1993
 *
 *  The module template was derived from the VGALINE module, but
 *  inner loop logic has been rewritten to take advantage of the
 *  Bresenham engine on the QVision SVGA.  The engine is capable
 *  of being fed two points and drawing a line, and of drawing
 *  polylines by continued feeding of points.  It also is capable
 *  of not drawing the endpoint of the line.
 *
 */
#include "X.h"
#include "Xproto.h"
#include "miscstruct.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "mfb.h"
#include "vga.h"
#include "vgaprocs.h"
#include "wga.h"

/*
 *  The Bresenham routines as MACRO's...
 */
#include "wgamacros.h"

#ifdef VMS
#include "qvga.h"
#include "stdio.h"
#else
#include <c_asm.h>
#include <stdio.h>
#endif

extern int vgaScreenPrivateIndex;
extern int vgaScreenActive;

typedef unsigned char BITMAP_UNIT;

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

/*
 *  Draw zero-width solid polylines or solid segments ...
 *
 */
void
#ifdef POLYSEGMENT
wgaSegmentSS (pDrawable, pGC, nseg, pSeg)
    DrawablePtr pDrawable;
    GCPtr	pGC;
    int         nseg;
    register xSegment   *pSeg;
#else
wgaLineSS (pDrawable, pGC, mode, npt, pptInit)
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
    vgaScreenPrivPtr vgaPriv = 
      (vgaScreenPrivPtr) (pGC->pScreen->devPrivates[vgaScreenPrivateIndex].ptr);
    wgaShadowRegPtr pShadow = (wgaShadowRegPtr) (vgaPriv->avail);

    BITMAP_UNIT oc1;		/* outcode of point 1 */
    BITMAP_UNIT oc2;		/* outcode of point 2 */

    int xorg, yorg,		/* origin of window */
	adx, ady,		/* abs values of dx and dy */
	signdx, signdy,		/* sign of dx and dy */
	e, e1, e2,		/* Bresenham error and increments */
	len,			/* length of segment */
	axis,			/* major axis */
#ifndef POLYSEGMENT
	Valid = 0,		/* Engine start point is valid */
#endif
	BresValid;		/* Error terms are valid (used when clipping) */
				/* they only need to be setup once for a line */

    /* a bunch of temporaries */
    register int y1, y2;
    register int x1, x2;
    RegionPtr cclip;
#ifndef POLYSEGMENT
    register DDXPointPtr ppt;	/* pointer to list of translated points */
#endif
    unsigned int fg  = (unsigned int) pGC->fgPixel;
    unsigned int alu = pGC->alu;
    unsigned int pm  = (unsigned int) pGC->planemask;

   /*
    *  These are used for clipping.
    *
    */
    BoxRec box;
    DDXPointRec pt1Copy;	/* clipped start point */
    DDXPointRec pt2Copy;	/* clipped end point */
    DDXPointRec	pt1;

    int clip1, clip2;		/* clippedness of the endpoints */
    int clipdx, clipdy;		/* difference between clipped and
				       	       	   unclipped start point */


    /*
     *	The following shouldn't really be necessary but there seems to
     *	be some problem in getting the CapNotLast drawn correctly by 
     *	the Triton engine so this is a workaround.
     */

     if (pGC->capStyle == CapNotLast) {
#ifdef POLYSEGMENT
       vgaSegmentSS(pDrawable, pGC, nseg, pSeg);
#else
       vgaLineSS(pDrawable, pGC, mode, npt, pptInit);
#endif
       return;
     }

    cclip = ((mfbPrivGC *)(pGC->devPrivates[mfbGCPrivateIndex].ptr))->pCompositeClip;

    pboxInit = REGION_RECTS(cclip);
    nboxInit = REGION_NUM_RECTS(cclip);

    xorg = pDrawable->x;
    yorg = pDrawable->y;
#ifndef POLYSEGMENT
    ppt = pptInit;
    x2 = ppt->x + xorg;
    y2 = ppt->y + yorg;
#endif

    /*
     *  Set up the line drawing engine, and make sure that all the other
     *  hardware context like the plane mask and color is valid.
     *
     */
#ifdef POLYSEGMENT
    WGASETUPSEGMENT(pGC->pScreen->myNum, pShadow, fg, alu, pm, pGC->capStyle);
    while(nseg--)
#else
    WGASETUPLINE(pGC->pScreen->myNum, pShadow, fg, alu, pm, pGC->capStyle);
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

	  {
	    /*
	     *   We have 2 points, so clip it and feed the
	     *   engine.
	     */
	    while(nbox--)
	      {
		BresValid = 0; /* Bresenham values have not been computed */
		oc1 = 0;
		oc2 = 0;
		OUTCODES(oc1, x1, y1, pbox);
		OUTCODES(oc2, x2, y2, pbox);
		if ((oc1 | oc2) == 0)
		  {

		   /*
		    *  The line is not clipped, and is contained inside
		    *  the current box - draw the line.  The end point
		    *  of the line will already be set in the HW for the
		    *  next line start.
		    *
		    */
#ifdef POLYSEGMENT
		    WGABRESSEGMENT(pGC->pScreen->myNum, x1, y1, x2, y2);
#else
		    WGABRESLINE(pGC->pScreen->myNum, Valid, x1, y1, x2, y2);
#endif
		    break;
		  }
		else if (oc1 & oc2)
		  {
		   /*
		    *  Trivial reject.  The line is totally clipped.  When the
		    *  Next line is drawn, the initial point must be set.
		    *
		    */
#ifndef POLYSEGMENT
		    Valid = 0;
#endif
		    pbox++;
		  }
		else if (x1 == x2)
		  {
		   /*
		    *  Clipped vertical line, draw the line
		    *  through the clip rectangle.
		    *
		    *  The simplest way is to make sure that
		    *  the direction of the line is top to
		    *  bottom.  And then do a simple clip.
		    *
		    *  Always assume that since we are in the
		    *  clipping code, and may be swapping end
		    *  points, that the initial position is not
		    *  valid in the hardware, and wont be for the
		    *  next line either.
		    *
		    */

		    int y1t = y1, y2t = y2, y1p, y2p;

	    	    if (y1 > y2)
		      {
		       /*
			*  Swap the end points for this line.  In the
			*  direction of the line, the first point is
			*  drawn, but the last point is not.  So we
			*  need to not only swap, but adjust the points
			*  to draw the same pixels.
			*
			*/
			y2t = y1 + 1;
			y1t = y2 + 1;
		      }

		    y1p = max(y1t, pbox->y1);
		    y2p = min(y2t, pbox->y2);
#ifndef POLYSEGMENT
		    Valid = 0;
		    if (y1p < y2p) WGABRESLINE(pGC->pScreen->myNum, Valid, 
					       x1, y1p, x2, y2p-1);
		    Valid = 0;
#else
		    if (y1p < y2p) WGABRESSEGMENT(pGC->pScreen->myNum, 
						  x1, y1p, x2, y2p-1);
#endif
		    pbox++;
		  }
		else if (y1 == y2)
		  {
		   /*
		    *  Clipped horizontal line
		    *
		    *  The simplest way is to make sure that
		    *  the direction of the line is left to
		    *  right.  And then do a simple clip.
		    *
		    *  Always assume that since we are in the
		    *  clipping code, and may be swapping end
		    *  points, that the initial position is not
		    *  valid in the hardware, and wont be for the
		    *  next line either.
		    */

		    int x1t = x1, x2t = x2, x1p, x2p;

	    	    if (x1 > x2)
		      {
		       /*
			*  Swap the end points for this line.  In the
			*  direction of the line, the first point is
			*  drawn, but the last point is not.  So we
			*  need to not only swap, but adjust the points
			*  to draw the same pixels.
			*
			*/
			x2t = x1 + 1;
			x1t = x2 + 1;
		      }

		    x1p = max(x1t, pbox->x1);
		    x2p = min(x2t, pbox->x2);

#ifndef POLYSEGMENT
		    Valid = 0;
		    if (x1p < x2p) WGABRESLINE(pGC->pScreen->myNum, Valid, 
					       x1p, y1, x2p-1, y2);
		    Valid = 0;
#else
		    if (x1p < x2p) WGABRESSEGMENT(pGC->pScreen->myNum, 
						  x1p, y1, x2p-1, y2);
#endif

		    pbox++;
		  }
		else
		  {
		    /*
		     *  The line intesects with the clipping rectangle, but
		     *  is not fully contained.  So a partial line will be
		     *  drawn.  The line is sloped, so we need to do it the
		     *  hard way, and generate the points in the line...
	     	     *  let the mfb helper routine do our work - it's better
		     *  than duplicating code...  and clipped lines already
		     *  have a lot of extra work to do...
		     *
	     	     */
    	
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
    	
		    if (!BresValid)
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

			BresValid = 1; /* Bresenham terms are valid */
		      }

		    if (mfbClipLine (pbox, box,
				     &pt1, &pt1Copy, &pt2Copy, 
				     adx, ady, signdx, signdy, axis,
				     &clip1, &clip2) == 1)
		      {

			/*
			 *  Assume that the initial point must be reloaded
			 *  in the hardware.
			 *
			 */
#ifndef POLYSEGMENT
			Valid = 0;
#endif
			if (clip1)
			  {
			    if (clip2)
			      {
#ifdef POLYSEGMENT
			        WGABRESSEGMENT(pGC->pScreen->myNum, 
					       pt1Copy.x, pt1Copy.y, 
					       pt2Copy.x, pt2Copy.y);
#else
				WGABRESLINE(pGC->pScreen->myNum, Valid, 
					    pt1Copy.x, pt1Copy.y, 
					    pt2Copy.x, pt2Copy.y);
#endif
			      }
			    else
			      {
#ifdef POLYSEGMENT
			        WGABRESSEGMENT(pGC->pScreen->myNum, 
					       pt1Copy.x, pt1Copy.y, x2, y2);
#else
			        WGABRESLINE(pGC->pScreen->myNum, Valid, 
					    pt1Copy.x, pt1Copy.y, x2, y2);
#endif
			      }
			  }
			else
			  {
#ifdef POLYSEGMENT
			    WGABRESSEGMENT(pGC->pScreen->myNum, 
					   x1, y1, pt2Copy.x, pt2Copy.y);
#else
			    WGABRESLINE(pGC->pScreen->myNum, Valid, 
					x1, y1, pt2Copy.x, pt2Copy.y);
#endif
			  }
		      } /* clipping check OK */
#ifndef POLYSEGMENT
		    Valid = 0;
#endif
		    pbox++; /* Next clip */
		} /* end clipped sloped line */
	    } /* while (nbox--) */
	} /* sloped line */
    } /* while (nline--) */

#ifndef POLYSEGMENT
   /*
    *  Draw the endpoint if we must...
    *
    */
    if ((pGC->capStyle != CapNotLast) &&
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
              WGABRESLINE(pGC->pScreen->myNum, Valid, x2, y2, x2, y2);
	      break;
	    }
	    else
		pbox++;
	}
    }
#endif
}
