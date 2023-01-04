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
/* $XConsortium: cfbbres.c,v 1.11 91/12/19 14:16:00 keith Exp $ */
#include "X.h"
#include "misc.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "servermd.h"

/* Solid bresenham line */
/* NOTES
   e2 is used less often than e1, so it's not in a register
*/

cfbBresS(rop, and, xor, addrl, nlwidth, signdx, signdy, axis, x1, y1, e, e1, e2, len)
int rop;
unsigned long and, xor;
unsigned long *addrl;		/* pointer to base of bitmap */
int nlwidth;			/* width in longwords of bitmap */
register int signdx;
int signdy;			/* signs of directions */
int axis;			/* major axis (Y_AXIS or X_AXIS) */
int x1, y1;			/* initial point */
register int e;			/* error accumulator */
register int e1;		/* bresenham increments */
int e2;
int len;			/* length of line */
{
    register int    e3 = e2-e1;
#ifdef PIXEL_ADDR
    register PixelType	 *addrp;		/* bitmask long pointer 
					     	 * cast to char pointer */
    if (len == 0)
	    return;
    /* point to first point */
    nlwidth <<= PWSH;

    addrp = (PixelType *)(addrl) + (y1 * nlwidth) + x1;
    if (signdy < 0)
    	nlwidth = -nlwidth;
    e = e-e1;			/* to make looping easier */
    
    if (axis == Y_AXIS)
    {
	int	t;

	t = nlwidth;
	nlwidth = signdx;
	signdx = t;
    }
    if (rop == GXcopy)
    {
	--len;
#define body {\
	    *addrp = xor; \
	    addrp += signdx; \
	    e += e1; \
	    if (e >= 0) \
	    { \
		addrp += nlwidth; \
		e += e3; \
	    } \
	}
	while (len >= 4)
	{
	    body body body body
	    len -= 4;
	}
	switch (len)
	{
	case  3: body case  2: body case  1: body
	}
#undef body
	*addrp = xor;
    }
    else
    {
	while(len--)
	{ 
	    *addrp = DoRRop (*addrp, and, xor);
	    e += e1;
	    if (e >= 0)
	    {
		addrp += nlwidth;
		e += e3;
	    }
	    addrp += signdx;
	}
    }
#else /* !PIXEL_ADDR */
    register unsigned long   tmp, bit;
    unsigned long leftbit, rightbit;

    /* point to longword containing first point */
    addrl = (addrl + (y1 * nlwidth) + (x1 >> PWSH));
    if (signdy < 0)
	    nlwidth = -nlwidth;
    e = e-e1;			/* to make looping easier */

    leftbit = cfbmask[0];
    rightbit = cfbmask[PPW-1];
    bit = cfbmask[x1 & PIM];

    if (axis == X_AXIS)
    {
	if (signdx > 0)
	{
	    while (len--)
	    { 
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		bit = SCRRIGHT(bit,1);
		e += e1;
		if (e >= 0)
		{
		    addrl += nlwidth;
		    e += e3;
		}
		if (!bit)
		{
		    bit = leftbit;
		    addrl++;
		}
	    }
	}
	else
	{
	    while (len--)
	    { 
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		e += e1;
		bit = SCRLEFT(bit,1);
		if (e >= 0)
		{
		    addrl += nlwidth;
		    e += e3;
		}
		if (!bit)
		{
		    bit = rightbit;
		    addrl--;
		}
	    }
	}
    } /* if X_AXIS */
    else
    {
	if (signdx > 0)
	{
	    while(len--)
	    {
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		e += e1;
		if (e >= 0)
		{
		    bit = SCRRIGHT(bit,1);
		    if (!bit)
		    {
			bit = leftbit;
			addrl++;
		    }
		    e += e3;
		}
		addrl += nlwidth;
	    }
	}
	else
	{
	    while(len--)
	    {
		*addrl = DoMaskRRop (*addrl, and, xor, bit);
		e += e1;
		if (e >= 0)
		{
		    bit = SCRLEFT(bit,1);
		    if (!bit)
		    {
			bit = rightbit;
			addrl--;
		    }
		    e += e3;
		}
		addrl += nlwidth;
	    }
	}
    } /* else Y_AXIS */
#endif
}
