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
static char *rcsid = "@(#)$RCSfile: ffbsline.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:17:42 $";
#endif
/*
 */

#include "X.h"
#include "Xprotostr.h"
#include "ffb.h"
#include "ffbsline.h"
#include "gcstruct.h"

#ifdef MITR5
#include "cfbdecline.h"
#else
#include "cfbline.h"
#endif

/* The CLIP64 code should be faster.  But the compiler doesn't generate better
   code.  I suspect this technique will work better in assembly, where the
   code won't constantly be loading up variables from memory, but will keep
   them in registers. */
#define CLIP64  (WORDBITS == 64) && 0


#ifdef SEGMENTS
void ffbSegS1(ffb, addrl, nlwidth, pel, nels, pbox, fillmode, maxPixels,
								    capEnds)
    FFB		ffb;		/* FFB registers		*/
    Pixel8      *addrl;		/* origin of drawable		*/
    int		nlwidth;	/* width in bytes of drawable	*/
    xSegment	*pel;		/* segment list			*/
    int		nels;		/* number of segments		*/
    BoxPtr	pbox;		/* Translated clip rectangle    */
    FFBMode     fillmode;       /* Mode for horizontal lines    */
    long	maxPixels;      /* Max pixels for horiz fill    */
    int		capEnds;	/* cap endpoints ?		*/
#else
void ffbLineS1(ffb, addrl, nlwidth, pel, nels, pbox, fillmode, maxPixels)
    FFB		ffb;		/* FFB registers		*/
    Pixel8      *addrl;		/* origin of drawable		*/
    int		nlwidth;	/* width in bytes of drawable	*/
    DDXPointPtr pel;		/* line list			*/
    int		nels;		/* number of lines		*/
    BoxPtr	pbox;		/* Translated clip rectangle    */
    FFBMode     fillmode;       /* Mode for horizontal lines    */
    long	maxPixels;      /* Max pixels for horiz fill    */
#endif

{
#ifdef SEGMENTS
    xSegment    *elEnd;
#else
    DDXPointPtr elEnd;
#endif
    long		clipul, cliplr;
    long		pt1x, pt1y;
    long		pt2x, pt2y;
    long		dx;		/* x2 - x1			*/
    long		dy;		/* y2 - y1			*/
    Pixel8		*addr;		/* current pixel pointer	*/
    unsigned long	clipCheck;
    unsigned long	ones = ~0;
    int			ymul;
    long		data;
    volatile CommandWord *slp_ngo_addr;	/* start of slope no go registers */
#if CLIP64
    Bits64		signbits;
#else
    unsigned long	signbits = 0x80008000;
#endif

#if CLIP64
    /* Load clip registers.  Do both end-point clipping 64 bits at a time. */
#   ifdef VMS
    signbits = 0x80008000;
    signbits |= signbits << 32;
#   else
    signbits = 0x8000800080008000L;
#   endif
    clipul = ((unsigned *)pbox)[0];
    clipul |= (clipul << 32);
    cliplr = ((unsigned *)pbox)[1] - 0x00010001;
    cliplr |= (cliplr << 32);
#else
    /* Load clip registers.  Keep x and y in a single 32-bit number. */
    clipul = ((int *)pbox)[0];
    cliplr = ((int *)pbox)[1] - 0x00010001;
#endif

    elEnd = pel + nels;
    WRITE_MEMORY_BARRIER();
#ifndef SEGMENTS
    /* Load initial address */
    FFBADDRESS(ffb, addrl + pel->y*nlwidth + pel->x*FFBPIXELBYTES);
#endif

    while (pel != elEnd) {
	pt1x = ((int *)pel)[0];
	pt2x = ((int *)pel)[1];
	pt1y = pt1x >> 16;
	pt2y = pt2x >> 16;
	pel++;
#   ifdef SEGMENTS
	ymul = pt1y * nlwidth;
#   endif

#if CLIP64
	/* Watch the slime fly! */
	pt2x = (pt2x & 0xffffffff) | (pt1x << 32);
	/* Are both points wholly within clip box? */
	clipCheck = ((pt2x-clipul) | (cliplr-pt2x));

#else
	/* Are both points wholly within clip box? */
	clipCheck = (pt2x-clipul) | (cliplr-pt2x);
	clipCheck |= (pt1x-clipul) | (cliplr-pt1x);
#endif

	if ((clipCheck & signbits) == 0) {
	    /* Okay, line is completely in clip rectangle.  So we also know
	       that pt1x, pt2x >= 0.  */
	    pt1x &= 0xffff;
	    pt2x &= 0xffff;
	    dx = pt2x - pt1x;
	    dy = pt2y - pt1y;
	    if (dy == 0) {             /* horizontal line */
		/* Paint line from left to right, keeping endpoint semantics */
#ifdef SEGMENTS
		pt2x += capEnds;
		if (dx <= 0) {
		    pt2x = pt1x + 1;
		    /* if dx = 0 and capEnds = 0, no line at all */
		    if (dx == capEnds) continue;
		    pt1x = pt2x + dx - capEnds;
		}
		dx = pt2x - pt1x;
		CYCLE_REGS(ffb);
		if (dx <= FFBLINEBITS) {
		    /* Avoid mode switching for short horizontal line */
		    addr = addrl + ymul + pt1x * FFBPIXELBYTES;
		    FFBADDRESS(ffb, addr);
		    FFBSLP7(ffb, dx-capEnds);	    /* |+dx| >= |+dy| */
		} else {
		    addrl = CYCLE_FB(addrl);
		    addr = addrl + ymul + pt1x * FFBPIXELBYTES;
		    FFBMODE(ffb, fillmode);
		    FFBSOLIDSPAN(ffb, addr, dx, maxPixels);
		    CYCLE_REGS(ffb);
		    FFBMODE(ffb, TRANSPARENTLINE | CAPENDSBIT(capEnds));
		}
#else
		Pixel8 *next;
		if (dx <= 0) {
		    if (dx == 0) continue;
		    if (dx >= -FFBLINEBITS) {
			/* Avoid mode switching */
			CYCLE_REGS(ffb);
			FFBSLP5(ffb, -dx);  /* |-dx| >= |+dy| */
			continue;
		    }
		    pt2x = pt1x + 1;
		    pt1x = pt2x + dx;
		    dx = -1;
		} else if (dx <= FFBLINEBITS) {
		    /* Avoid mode switching */
		    CYCLE_REGS(ffb);
		    FFBSLP7(ffb, dx);	    /* |+dx| >= |+dy| */
		    continue;
		}
		addrl = CYCLE_FB(addrl);
		addr = addrl + pt1y * nlwidth + pt1x * FFBPIXELBYTES;
		next = addr + dx * FFBPIXELBYTES;
		CYCLE_REGS(ffb);
		FFBMODE(ffb, fillmode);
		FFBSOLIDSPAN(ffb, addr, pt2x-pt1x, maxPixels);
		CYCLE_REGS(ffb);
		FFBMODE(ffb, TRANSPARENTLINE);
		FFBADDRESS(ffb, next);
#endif

	    } else {	/* sloped line, length > 0 */
		CYCLE_REGS(ffb);
#ifdef SEGMENTS
		addr = addrl + ymul + pt1x*FFBPIXELBYTES;
		FFBADDRESS(ffb, addr);
#endif
		if (dx < 0) {
		    dx = -dx;
		    if (dy < 0) {
			dy = -dy;
			data = FFBLINEDXDY(dx, dy);
			if (dx < dy) {
#ifdef SEGMENTS
			    dy += capEnds;
#endif
			    FFBSLP0(ffb, data);   /* |-dx| < |-dy| */
			    dy -= FFBLINEBITS; 
			    while (dy > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dy -= FFBLINEBITS;
			    }
			} else { /* dx >= dy */
#ifdef SEGMENTS
			    dx += capEnds;
#endif
			    FFBSLP4(ffb, data);   /* |-dx| >= |-dy| */
			    dx -= FFBLINEBITS;
			    while (dx > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dx -= FFBLINEBITS;
			    }
			} /* if dx < dy ... else ... */
		    } else { /* dy >= 0 */
			data = FFBLINEDXDY(dx, dy);
			if (dx < dy) {
#ifdef SEGMENTS
			    dy += capEnds;
#endif
			    FFBSLP1(ffb, data);    /* |-dx| < |+dy| */
			    dy -= FFBLINEBITS; 
			    while (dy > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dy -= FFBLINEBITS;
			    }
			} else { /* dx >= dy */
#ifdef SEGMENTS
			    dx += capEnds;
#endif
			    FFBSLP5(ffb, data);	  /* |-dx| >= |+dy| */
			    dx -= FFBLINEBITS;
			    while (dx > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dx -= FFBLINEBITS;
			    }
			}
		    } /* end if dy < 0 ... else ... */
		} else { /* dx >= 0 */
		    if (dy < 0) {
			dy = -dy;
			data = FFBLINEDXDY(dx, dy);
			if (dx < dy) {
#ifdef SEGMENTS
			    dy += capEnds;
#endif
			    FFBSLP2(ffb, data);	   /* |+dx| < |-dy| */
			    dy -= FFBLINEBITS;
			    while (dy > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dy -= FFBLINEBITS;
			    }
			} else {
#ifdef SEGMENTS
			    dx += capEnds;
#endif
			    FFBSLP6(ffb, data);     /* |+dx| >= |-dy| */
			    dx -= FFBLINEBITS;
			    while (dx > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dx -= FFBLINEBITS;
			    }
		        }
		   } else {
			data = FFBLINEDXDY(dx, dy);
		        if (dx < dy) {
#ifdef SEGMENTS
			    dy += capEnds;
#endif
			    FFBSLP3(ffb, data);	   /* |+dx| < |+dy| */
			    dy -= FFBLINEBITS;
			    while (dy > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dy -= FFBLINEBITS;
			    }
			} else {
#ifdef SEGMENTS
			    dx += capEnds;
#endif
			    FFBSLP7(ffb, data);	    /* |+dx| >= |+dy| */
			    dx -= FFBLINEBITS;
			    while (dx > 0) {
				CYCLE_REGS(ffb);
				FFBCONTINUE(ffb, ones);
				dx -= FFBLINEBITS;
			    }
			}
		    }
		}	
	    } /* if horizontal else sloped line */

	} else {
	    unsigned long	oc1, oc2;

	    /* pt1x, pt2x may be negative, so sign-extend */
	    pt1x = (pt1x << (LONG_BIT - 16)) >> (LONG_BIT - 16);
	    pt2x = (pt2x << (LONG_BIT - 16)) >> (LONG_BIT - 16);
	    dx = pt2x - pt1x;
	    dy = pt2y - pt1y;

	    if (((oc1 = OUTCODES(oc1, pt1x, pt1y, pbox)) 
	       & (oc2 = OUTCODES(oc2, pt2x, pt2y, pbox))) == 0) {
		/* Have to clip */
		long	    e, e2;  /* bresenham error and incs */
		DDXPointRec clippt1, clippt2;   /* clipped endpoints    */
		int	    signdx;	    /* sign of dx and dy    */
		int	    signdy;
		int	    len;
    
		clippt1.x = pt1x;
		clippt1.y = pt1y;
		clippt2.x = pt2x;
		clippt2.y = pt2y;
    
		signdx = 1;
		signdy = 1;
#ifdef ndef
		CYCLE_REGS(ffb);
		if (dx < 0) {
		    dx = -dx;
		    signdx = -1;
		    if (dy < 0) {
			dy = -dy;
			signdy = -1;
			data = FFBLINEDXDY(dx,dy);
			if (dx < dy) {
			    FFBSLPNGO0(ffb,data);  /* |-dx| < |-dy| */
			} else {
			    FFBSLPNGO4(ffb,data);  /* |-dx| >= |-dy| */
			}
		    } else {
			data = FFBLINEDXDY(dx,dy);
			if (dx < dy) {
			    FFBSLPNGO1(ffb,data);   /* |-dx| < |+dy| */
			} else {
			    FFBSLPNGO5(ffb,data);   /* |-dx| >= |+dy| */
			}
		    }
		} else {
		    if (dy < 0) {
			dy = -dy;
			signdy = -1;
			data = FFBLINEDXDY(dx,dy);
			if (dx < dy){
			    FFBSLPNGO2(ffb,data);   /* |+dx| < |-dy| */
			} else {
			    FFBSLPNGO6(ffb,data);   /* |+dx| >= |-dy| */
			}
		    } else {
			data = FFBLINEDXDY(dx,dy);
			if (dx < dy){
			    FFBSLPNGO3(ffb,data);    /* |+dx| < |+dy| */
			} else {
			    FFBSLPNGO7(ffb,data);   /* |+dx| >= |+dy| */
			}
		    }
		}
#else
		CYCLE_REGS(ffb);
		slp_ngo_addr = &ffb->sng_dx_gt_dy;
		if (dx < 0) {
		    dx = -dx;
		    signdx = -1;
		    slp_ngo_addr -= 2;
		} 
		if (dy < 0) {
		    dy = -dy;
		    signdy = -1;
		    slp_ngo_addr -= 1;
		}
		slp_ngo_addr -= ((dx < dy) << 2);
		FFBWRITE(slp_ngo_addr, FFBLINEDXDY(dx,dy));
#endif
		if (cfbClipLine(pbox, &clippt1, &clippt2,
			oc1, oc2, dx, dy, signdx, signdy) > 0) {
    
		    /* pt2 is now the clipped pt1 */
		    pt2x = clippt1.x;
		    pt2y = clippt1.y;
		    e2 = dx - dy;
		    if (e2 >= 0) {
			/* Horizontalish */
			e = (dy - e2) >> 1;
			len = clippt2.x - pt2x;
			if (len < 0) {
			    e -= 1;
			    len = -len;
			}
		    } else {
			/* Verticalish */
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
		    /* We always set the address register.  If len > 0 in the
		       code below, this is the starting point of the clipped 
		       line.  If len = 0, then we have a line that just barely
		       extends into the clip region, but we don't paint it
		       because it doesn't have an end cap.  In this case, the 
		       starting point and ending point are the same, and we'd
		       better leave the ffb address register filled correctly,
		       because the NEXT line may not be clipped at ALL! */
		    addr = addrl + pt2y*nlwidth + pt2x * FFBPIXELBYTES;
		    CYCLE_REGS(ffb);
		    FFBADDRESS(ffb, addr);
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
			FFBBRES3(ffb, (e << 15) | len);
			do {
			    CYCLE_REGS(ffb);
			    FFBCONTINUE(ffb, ones);
			    len -= FFBLINEBITS;
			} while (len > 0);
		    }
		}
	    } /* if OUTCODES believes portion of line may be visible */
	} /* if fully visible ... else ... */
    } /* while points */;
}

/*
 * HISTORY
 */

/* $Header: /usr/sde/osf1/rcs/x11/src/server/ddx/dec/ffb/ffbsline.c,v 1.1.2.2 1993/11/19 21:17:42 Robert_Lembree Exp $ */
/* Handles 0-width, 1 clip rectangle solid PolyLine and PolySegment requests
   via conditional compilation.
*/
