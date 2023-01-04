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
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
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
/* $XConsortium: mfbbres.c,v 1.15 89/09/14 16:26:53 rws Exp $ */
#include "X.h"
#include "misc.h"
#include "mfb.h"
#include "maskbits.h"

/* Solid bresenham line */
/* NOTES
   e2 is used less often than e1, so it's not in a register
*/

mfbBresS(rop, addrl, nlwidth, signdx, signdy, axis, x1, y1, e, e1, e2, len)
int rop;		/* a reduced rasterop */
long *addrl;		/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
int signdx, signdy;	/* signs of directions */
int axis;		/* major axis (Y_AXIS or X_AXIS) */
int x1, y1;		/* initial point */
register int e;		/* error accumulator */
register int e1;	/* bresenham increments */
int e2;
int len;		/* length of line */
{
    register int yinc;	/* increment to next scanline, in bytes */
    register unsigned char *addrb;		/* bitmask long pointer 
						 * cast to char pointer */
    register unsigned long bit;	/* current bit being set/cleared/etc.  */
    unsigned long leftbit = mask[0]; /* leftmost bit to process in new word */
    unsigned long rightbit = mask[WRDSZ-1]; /* rightmost bit to process in new word */

    register int e3 = e2-e1;
    unsigned long	tmp;

    /* point to longword containing first point */
    addrb = (unsigned char *)(addrl + (y1 * nlwidth) + (x1 >> PWSH));
    yinc = signdy * nlwidth * sizeof(long); 
    e = e-e1;			/* to make looping easier */
    bit = mask[x1 & (WRDSZ-1)];

    if (!len)
	return;
    if (rop == RROP_BLACK)
    {
        if (axis == X_AXIS)
        {
	    if (signdx > 0)
	    {
		tmp = *(unsigned long*)addrb;
		for (;;)
		{ 
		    tmp &= ~bit;
		    if (!--len)
			break;
		    bit = SCRRIGHT(bit,1);
		    e += e1;
 		    if (e >= 0)
		    {
			*(unsigned long *) addrb = tmp;
			addrb += yinc;
			e += e3;
			if (!bit)
			{
			    bit = leftbit;
			    addrb += sizeof(long);
			}
			tmp = *(unsigned long *) addrb;
		    }
		    else if (!bit)
 		    {
			*(unsigned long *) addrb = tmp;
			bit = leftbit;
			addrb += sizeof(long);
			tmp = *(unsigned long *) addrb;
		    }
		}
		*(unsigned long *) addrb = tmp;
	    }
	    else
	    {
		tmp = *(unsigned long *)addrb;
		for (;;)
		{ 
		    tmp &= ~bit;
		    if (!--len)
			break;
		    e += e1;
		    bit = SCRLEFT(bit,1);
		    if (e >= 0)
		    {
			*(unsigned long *) addrb = tmp;
			addrb += yinc;
			e += e3;
			if (!bit)
			{
			    bit = rightbit;
			    addrb -= sizeof(long);
			}
			tmp = *(unsigned long *) addrb;
		    }
		    else if (!bit)
 		    {
			*(unsigned long *) addrb = tmp;
			bit = rightbit;
			addrb -= sizeof(long);
			tmp = *(unsigned long *) addrb;
		    }
		}
		*(unsigned long *) addrb = tmp;
	    }
        } /* if X_AXIS */
        else
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *(unsigned long *)addrb &= ~bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRRIGHT(bit,1);
			if (!bit) { bit = leftbit;addrb += sizeof(long); }
			e += e3;
		    }
		    addrb += yinc;
		}
	    }
	    else
	    {
		while(len--)
		{
		    *(unsigned long *)addrb &= ~bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRLEFT(bit,1);
			if (!bit) { bit = rightbit;addrb -= sizeof(long); }
			e += e3;
		    }
		    addrb += yinc;
		}
	    }
        } /* else Y_AXIS */
    } 
    else if (rop == RROP_WHITE)
    {
        if (axis == X_AXIS)
        {
	    if (signdx > 0)
	    {
		tmp = *(unsigned long *)addrb;
		for (;;)
		{
		    tmp |= bit;
		    if (!--len)
			break;
		    e += e1;
		    bit = SCRRIGHT(bit,1);
		    if (e >= 0)
		    {
			*(unsigned long *) addrb = tmp;
			addrb += yinc;
			e += e3;
			if (!bit)
			{
			    bit = leftbit;
			    addrb += sizeof(long);
			}
			tmp = *(unsigned long *) addrb;
		    }
		    else if (!bit)
 		    {
			*(unsigned long *) addrb = tmp;
			bit = leftbit;
			addrb += sizeof(long);
			tmp = *(unsigned long *) addrb;
		    }
		}
		*(unsigned long *) addrb = tmp;
	    }
	    else
	    {
		tmp = *(unsigned long *) addrb;
		for (;;)
		{
		    tmp |= bit;
		    if (!--len)
			break;
		    e += e1;
		    bit = SCRLEFT(bit,1);
		    if (e >= 0)
		    {
			*(unsigned long *) addrb = tmp;
			addrb += yinc;
			e += e3;
			if (!bit)
			{
			    bit = rightbit;
			    addrb -= sizeof(long);
			}
			tmp = *(unsigned long *) addrb;
		    }
		    else if (!bit)
		    {
			*(unsigned long *) addrb = tmp;
			bit = rightbit;
			addrb -= sizeof(long);
			tmp = *(unsigned long *) addrb;
		    }
		}
		*(unsigned long *) addrb = tmp;
	    }
        } /* if X_AXIS */
        else
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *(unsigned long *)addrb |= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRRIGHT(bit,1);
			if (!bit) { bit = leftbit;addrb += sizeof(long); }
			e += e3;
		    }
		    addrb += yinc;
		}
	    }
	    else
	    {
		while(len--)
		{
		    *(unsigned long *)addrb |= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRLEFT(bit,1);
			if (!bit) { bit = rightbit;addrb -= sizeof(long); }
			e += e3;
		    }
		    addrb += yinc;
		}
	    }
        } /* else Y_AXIS */
    }
    else if (rop == RROP_INVERT)
    {
        if (axis == X_AXIS)
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *(unsigned long *)addrb ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			addrb += yinc;
			e += e3;
		    }
		    bit = SCRRIGHT(bit,1);
		    if (!bit) { bit = leftbit;addrb += sizeof(long); }
		}
	    }
	    else
	    {
		while(len--)
		{
		    *(unsigned long *)addrb ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			addrb += yinc;
			e += e3;
		    }
		    bit = SCRLEFT(bit,1);
		    if (!bit) { bit = rightbit;addrb -= sizeof(long); }
		}
	    }
        } /* if X_AXIS */
        else
        {
	    if (signdx > 0)
	    {
		while(len--)
		{
		    *(unsigned long *)addrb ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRRIGHT(bit,1);
			if (!bit) { bit = leftbit;addrb += sizeof(long); }
			e += e3;
		    }
		    addrb += yinc;
		}
	    }
	    else
	    {
		while(len--)
		{
		    *(unsigned long *)addrb ^= bit;
		    e += e1;
		    if (e >= 0)
		    {
			bit = SCRLEFT(bit,1);
			if (!bit) { bit = rightbit;addrb -= sizeof(long); }
			e += e3;
		    }
		    addrb += yinc;
		}
	    }
        } /* else Y_AXIS */
    }
} 
