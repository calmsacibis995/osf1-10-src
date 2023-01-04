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
/***********************************************************
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/* $XConsortium: cfbbresd.c,v 1.12 91/12/26 14:36:31 keith Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"

/* Dashed bresenham line */

cfbBresD(rrops,
	 pdashIndex, pDash, numInDashList, pdashOffset, isDoubleDash,
	 addrl, nlwidth,
	 signdx, signdy, axis, x1, y1, e, e1, e2, len)
    cfbRRopPtr	    rrops;
    int		    *pdashIndex;	/* current dash */
    unsigned char   *pDash;		/* dash list */
    int		    numInDashList;	/* total length of dash list */
    int		    *pdashOffset;	/* offset into current dash */
    int		    isDoubleDash;
    unsigned long   *addrl;		/* pointer to base of bitmap */
    int		    nlwidth;		/* width in longwords of bitmap */
    int		    signdx, signdy;	/* signs of directions */
    int		    axis;		/* major axis (Y_AXIS or X_AXIS) */
    int		    x1, y1;		/* initial point */
    register int    e;			/* error accumulator */
    register int    e1;			/* bresenham increments */
    int		    e2;
    int		    len;		/* length of line */
{
    register PixelType	*addrp;
    register		int e3 = e2-e1;
    int			dashIndex;
    int			dashOffset;
    int			dashRemaining;
    unsigned long	xorFg, andFg, xorBg, andBg;
    Bool		isCopy;
    int			thisDash;

    dashOffset = *pdashOffset;
    dashIndex = *pdashIndex;
    isCopy = (rrops[0].rop == GXcopy && rrops[1].rop == GXcopy);
    xorFg = rrops[0].xor;
    andFg = rrops[0].and;
    xorBg = rrops[1].xor;
    andBg = rrops[1].and;
    dashRemaining = pDash[dashIndex] - dashOffset;
    if ((thisDash = dashRemaining) >= len)
    {
	thisDash = len;
	dashRemaining -= len;
    }
    e = e-e1;			/* to make looping easier */

#define BresStep(minor,major) {if ((e += e1) >= 0) { e += e3; minor; } major;}

#define NextDash {\
    dashIndex++; \
    if (dashIndex == numInDashList) \
	dashIndex = 0; \
    dashRemaining = pDash[dashIndex]; \
    if ((thisDash = dashRemaining) >= len) \
    { \
	dashRemaining -= len; \
	thisDash = len; \
    } \
}

#ifdef PIXEL_ADDR

#define Loop(store) while (thisDash--) {\
			store; \
 			BresStep(addrp+=signdy,addrp+=signdx) \
		    }
    /* point to first point */
    nlwidth <<= PWSH;
    addrp = (PixelType *)(addrl) + (y1 * nlwidth) + x1;
    signdy *= nlwidth;
    if (axis == Y_AXIS)
    {
	int t;

	t = signdx;
	signdx = signdy;
	signdy = t;
    }

    if (isCopy)
    {
	for (;;)
	{ 
	    len -= thisDash;
	    if (dashIndex & 1) {
		if (isDoubleDash) {
		    Loop(*addrp = xorBg)
		} else {
		    Loop(;)
		}
	    } else {
		Loop(*addrp = xorFg)
	    }
	    if (!len)
		break;
	    NextDash
	}
    }
    else
    {
	for (;;)
	{ 
	    len -= thisDash;
	    if (dashIndex & 1) {
		if (isDoubleDash) {
		    Loop(*addrp = DoRRop(*addrp,andBg, xorBg))
		} else {
		    Loop(;)
		}
	    } else {
		Loop(*addrp = DoRRop(*addrp,andFg, xorFg))
	    }
	    if (!len)
		break;
	    NextDash
	}
    }
#else /* !PIXEL_ADDR */
    {
    	register unsigned long	tmp;
	unsigned long		startbit, bit;

    	/* point to longword containing first point */
    	addrl = (addrl + (y1 * nlwidth) + (x1 >> PWSH));
    	signdy = signdy * nlwidth;

	if (signdx > 0)
	    startbit = cfbmask[0];
	else
	    startbit = cfbmask[PPW-1];
    	bit = cfbmask[x1 & PIM];

#define X_Loop(store)	while(thisDash--) {\
			    store; \
		    	    BresStep(addrl += signdy, \
		    	     	     if (signdx > 0) \
		    	     	     	 bit = SCRRIGHT(bit,1); \
		    	     	     else \
		    	     	     	 bit = SCRLEFT(bit,1); \
		    	     	     if (!bit) \
		    	     	     { \
		    	     	     	 bit = startbit; \
		    	     	     	 addrl += signdx; \
		    	     	     }) \
			}
#define Y_Loop(store)	while(thisDash--) {\
			    store; \
		    	    BresStep(if (signdx > 0) \
		    	     	     	 bit = SCRRIGHT(bit,1); \
		    	     	     else \
		    	     	     	 bit = SCRLEFT(bit,1); \
		    	     	     if (!bit) \
		    	     	     { \
		    	     	     	 bit = startbit; \
		    	     	     	 addrl += signdx; \
		    	     	     }, \
				     addrl += signdy) \
			}

    	if (axis == X_AXIS)
    	{
	    for (;;)
	    {
	    	len -= thisDash;
	    	if (dashIndex & 1) {
		    if (isDoubleDash) {
		    	X_Loop(*addrl = DoMaskRRop(*addrl, andBg, xorBg, bit));
		    } else {
		    	X_Loop(;)
		    }
	    	} else {
		    X_Loop(*addrl = DoMaskRRop(*addrl, andFg, xorFg, bit));
	    	}
	    	if (!len)
		    break;
	    	NextDash
	    }
    	} /* if X_AXIS */
    	else
    	{
	    for (;;)
	    {
	    	len -= thisDash;
	    	if (dashIndex & 1) {
		    if (isDoubleDash) {
		    	Y_Loop(*addrl = DoMaskRRop(*addrl, andBg, xorBg, bit));
		    } else {
		    	Y_Loop(;)
		    }
	    	} else {
		    Y_Loop(*addrl = DoMaskRRop(*addrl, andFg, xorFg, bit));
	    	}
	    	if (!len)
		    break;
	    	NextDash
	    }
    	} /* else Y_AXIS */
    }
#endif
    *pdashIndex = dashIndex;
    *pdashOffset = pDash[dashIndex] - dashRemaining;
}
