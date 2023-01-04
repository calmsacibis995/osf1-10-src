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
#  define IDENT "X-6"
#  include "module_ident.h"
#  define _FBOFFSET_ fbOffset
#else
#  define _FBOFFSET_ 0
#endif
/****************************************************************************
**                                                                          *
**                 COPYRIGHT (c) 1988, 1989, 1990, 1991 BY                  *
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

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbsline.c,v 1.1.3.13 93/02/18 17:05:48 Don_Haney Exp $ */

/* Handles 0-width, 1 clip rectangle solid PolyLine and PolySegment requests
   via conditional compilation.
*/

#include "X.h"
#include "Xprotostr.h"
#include "sfb.h"
#include "sfbsline.h"

#ifdef MITR5
#include "cfbdecline.h"
#else
#include "cfbline.h"
#endif
/*
 * For VMS, the frame buffer is not necessarily on an 8 meg boundary,
 * so adjust the address to make it look like it is  (fbOffset
 * contains the offset to the nearest 8meg boundary)
 */

#ifdef SEGMENTS
void sfbSegS1(sfb, addrl, nlwidth, pel, nels, pbox, capEnds, fbOffset)
    register SFB	sfb;		/* SFB registers		*/
    register Pixel8     *addrl;		/* origin of drawable		*/
    register int	nlwidth;	/* width in bytes of drawable	*/
             xSegment	*pel;		/* segment list			*/
             int	nels;		/* number of segments		*/
	     BoxPtr	pbox;		/* Translated clip rectangle    */
    register int	capEnds;	/* cap endpoints ?		*/
    register int        fbOffset; 	/* offset from frame buffer to  */
					/*  nearest 8 Meg boundary      */
#else
void sfbLineS1(sfb, addrl, nlwidth, pel, nels, pbox, fbOffset)
    register SFB	sfb;		/* SFB registers		*/
    register Pixel8     *addrl;		/* origin of drawable		*/
    register int	nlwidth;	/* width in bytes of drawable	*/
            DDXPointPtr pel;		/* line list			*/
             int	nels;		/* number of lines		*/
	     BoxPtr	pbox;		/* Translated clip rectangle    */
    register int        fbOffset; 	/* offset from frame buffer to  */
					/*  nearest 8 Meg boundary      */
#endif

{
    register int	    pt1MayBeOut;  /* 0 if pt1 definitely in clip
					     region, non-0 otherwise. */
#ifdef SEGMENTS
    register xSegment       *elEnd;
#else
    register DDXPointPtr    elEnd;
#endif
    register int	clipul, cliplr;
    register int	pt1x, pt1y;
    register int	pt2x, pt2y;
    register int	dx;		/* x2 - x1			*/
    register int	dy;		/* y2 - y1			*/
    register int	len;
    register int	align;
    int			snlwidth;       /* width multiplied by sign bit	*/
    register Pixel8     *addr, *cycle_addr;	/* current pixel pointe	*/
    unsigned long	diff_addr;
    register int	signbits = 0x80008000;
    register int	ones = 0xffff;
    unsigned		oc1, oc2;
    int			signdx;	    /* sign of dx and dy    */
    int			signdy;
    register int	e, e2;  /* bresenham error and incs */
    int			clipCheck;
    int			do_cycle;

    /* Load clip registers.  Keep x and y in a single 32-bit number. */
    clipul = ((int *)pbox)[0];
    cliplr = ((int *)pbox)[1] - 0x00010001;

    elEnd = pel + nels;
#ifndef SEGMENTS
    /* Load initial address */
    SFBADDRESS(sfb, addrl + pel->y*nlwidth + pel->x * SFBPIXELBYTES +
	_FBOFFSET_);
    pt1MayBeOut = TRUE;      /* Because we aren't sure if it is. */
#endif
    while (pel != elEnd) {
	do_cycle = FALSE;
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
	dx = pt2x - pt1x;
	dy = pt2y - pt1y;

	pt1MayBeOut = clipCheck & signbits;
	if (pt1MayBeOut == 0) {
	    /* Okay, line is completely in clip rectangle. */
	    signdx = SFBPIXELBYTES << 16;
	    if (dy == 0) {
		/* Paint line from left to right, keeping endpoint semantics */
		Pixel8 *next;

		addr = addrl + pt1y * nlwidth;
		do_cycle = TRUE;
#ifdef SEGMENTS
		pt2x += capEnds;
		if (dx <= 0) {
		    pt2x = pt1x + 1;
		    if (dx == capEnds) goto NoLine;
		    pt1x = pt2x + dx - capEnds;
		}
		addr += pt1x * SFBPIXELBYTES;
#else
		if (dx <= 0) {
		    pt2x = pt1x + 1;
		    if (dx == 0) goto NoLine;
		    pt1x = pt2x + dx;
		    dx = -1;
		}
		addr += pt1x * SFBPIXELBYTES;
		next = addr + dx * SFBPIXELBYTES;
#endif
		SFBMODE(sfb, TRANSPARENTSTIPPLE);
		SFBSOLIDSPAN(sfb, addr, pt2x-pt1x, SFBSTIPPLEALL1);
		SFBMODE(sfb, TRANSPARENTLINE);
#ifndef SEGMENTS
		next += _FBOFFSET_;
		SFBADDRESS(sfb, next);
#endif
	    } else {	/* sloped line */
#ifdef SEGMENTS
		addr = addrl + pt1y * nlwidth + pt1x * SFBPIXELBYTES 
			+ _FBOFFSET_;;
		do_cycle = TRUE;
#endif
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
		    dx += capEnds;
#endif
		    SFBBRES3(sfb, e | dx);
#ifdef SEGMENTS
# ifdef TLBFAULTS
		    SFBADDRESS(sfb, addr);
		    SFBBRESCONTINUE(sfb, ones);
# else
		    align = (int)addr & SFBBUSBYTESMASK;
		    addr -= align;
		    SFBWRITE(addr, (align << 16) | ones);
# endif

#else /* PolyLine */
		    SFBBRESCONTINUE(sfb, ones);
#endif
		    dx -= SFBLINEBITS;
		    while (dx > 0) {
			SFBBRESCONTINUE(sfb, ones);
			dx -= SFBLINEBITS;
		    }

		} else {
		    /* Verticalish line */
		    e2 = -e2;
		    e = (dx - e2 - (signdy < 0))  << 14;
		    SFBBRES1(sfb, snlwidth | dx);
		    SFBBRES2(sfb, (snlwidth + signdx) | e2);
#ifdef SEGMENTS
		    dy += capEnds;
#endif
		    SFBBRES3(sfb, e | dy);
#ifdef SEGMENTS
# ifdef TLBFAULTS
		    SFBADDRESS(sfb, addr);
		    SFBBRESCONTINUE(sfb, ones);
# else
		    align = (int)addr & SFBBUSBYTESMASK;
		    addr -= align;
		    SFBWRITE(addr, (align << 16) | ones);
# endif

#else /* PolyLine */
		    SFBBRESCONTINUE(sfb, ones);
#endif
		    dy -= SFBLINEBITS;
		    while (dy > 0) {
			SFBBRESCONTINUE(sfb, ones);
			dy -= SFBLINEBITS;
		    }
		} /* if horizontalish else verticalish */
	    } /* if horizontal else sloped line */
	
	} else if (((oc1 = OUTCODES(oc1, pt1x, pt1y, pbox))
	           &(oc2 = OUTCODES(oc2, pt2x, pt2y, pbox))) == 0) {
	    /* Have to clip */
	    DDXPointRec clippt1, clippt2;   /* clipped endpoints    */

	    clippt1.x = pt1x;
	    clippt1.y = pt1y;
	    clippt2.x = pt2x;
	    clippt2.y = pt2y;

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
	    if (cfbClipLine(pbox, &clippt1, &clippt2,
		    oc1, oc2, dx, dy, signdx, signdy) > 0) {

		/* pt2 is now the clipped pt1 */
		pt2x = clippt1.x;
		pt2y = clippt1.y;
		signdx = signdx << SFBLINESHIFT;
		e2 = dx - dy;
		if (e2 >= 0) {
		    /* Horizontalish */
		    SFBBRES1(sfb, signdx | dy);
		    SFBBRES2(sfb, (snlwidth + signdx) | e2);
		    e = (dy - e2) >> 1;
		    len = clippt2.x - pt2x;
		    if (len < 0) {
			e -= 1;
			len = -len;
		    }
		} else {
		    /* Verticalish */
		    SFBBRES1(sfb, snlwidth | dx);
		    SFBBRES2(sfb, (snlwidth + signdx) | -e2);
		    e = (dx + e2) >> 1;
		    len = clippt2.y - pt2y;
		    if (len < 0) {
			e -= 1;
			len = -len;
		    }
		}
#ifdef SEGMENTS
		len += ((oc2 != 0) | capEnds);
#else
		len += (oc2 != 0);
#endif    
		/* We always set the address register.  If len > 0 in the code 
		   below, this is the starting point of the clipped line.  If 
		   len = 0, then we have a line that just barely extends into
		   the clip region, but we don't paint it because it doesn't
		   have an end cap.  In this case, the starting point and 
		   ending point are the same, and we'd better leave the sfb
		   address register filled correctly, because the NEXT line
		   may not be clipped at ALL! */
		addr = addrl + pt2y*nlwidth + pt2x * SFBPIXELBYTES + _FBOFFSET_;
		do_cycle = TRUE;
		SFBADDRESS(sfb, addr);
		if (len > 0) {
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
			} else {
			    /* Verticalish */
			    e += pt1y*dx - pt1x*dy;
			}
		    }
		    SFBBRES3(sfb, (e << 15) | len);
		    do {
			SFBBRESCONTINUE(sfb, ones);
			len -= SFBLINEBITS;
		    } while (len > 0);
		}
	    }
	}
	    
	/*
	 * We are cycling addr and then adding the increment to addrl
	 * because addrl is not really the base of the frame buffer at
	 * this point but is instead the virtual base of the current
	 * window .... somewhat more negative than the FB base.
	 */
	
	if (do_cycle) {
	    cycle_addr = CYCLE_FB(addr);
	    diff_addr = (unsigned long) cycle_addr - (unsigned long) addr;
	    addrl += diff_addr;
	}
NoLine:;
    } /* while points */;

}
#undef _FBOFFSET_
