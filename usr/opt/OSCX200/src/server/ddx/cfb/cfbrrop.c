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
/*
 * $XConsortium: cfbrrop.c,v 1.5 91/12/11 14:03:14 rws Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/* cfb reduced rasterop computations */

#include "X.h"
#include "Xmd.h"
#include "Xproto.h"
#include "cfb.h"
#include "cfbmskbits.h"

/* A description:
 *
 * There are four possible operations on each bit in the destination word,
 *
 *	    1	2   3	4
 *
 *    0	    0	0   1	1
 *    1	    0	1   0	1
 *
 * On examination of the reduced rop equation (dst = (dst & and) ^ xor),
 * these four fall to reduced rops as follows:
 *
 *  and	    0	1   1	0
 *  xor	    0	0   1	1
 *
 * or, (if 'and' is expensive) (dst = (dst | or) ^ xor)
 *
 *  or	    1	0   0	1
 *  xor	    1	0   1	0
 *
 * The trouble with using this later equation is that trivial
 * rasterop reduction is more difficult; some common rasterops
 * use complicated expressions of xor/and instead of the simple
 * ones while other common rasterops are not made any simpler:
 *
 * GXcopy:	*dst = ~xor		instead of  *dst = xor
 * GXand:	*dst = *dst & ~or	instead of  *dst = *dst & and
 * GXor:	*dst = *dst | or	instead of  *dst = *dst | xor
 * GXxor:	*dst = *dst ^ xor	instead of  *dst = *dst ^ xor
 *
 * If you're really set on using this second mechanism, the changes
 * are pretty simple.
 *
 * All that remains is to provide a mechanism for computing and/xor values
 * based on the raster op and foreground value.
 *
 * The 16 rops fall as follows, with the associated reduced
 * rop and/xor and or/xor values.  The values in parenthesis following the
 * reduced values gives an equation using the source value for
 * the reduced value, and is one of {0, src, ~src, 1} as appropriate.
 *
 *	clear		and		andReverse	copy
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   0	0	0   0   0	0   0	1	0   0	1
 *	1   0	0	1   0   1	1   0	0	1   0	1
 *
 *  and	    0	0 (0)	    0   1 (src)	    0	1 (src)	    0	0 (0)
 *  xor	    0	0 (0)	    0   0 (0)	    0	1 (src)	    0	1 (src)
 *
 *  or	    1	1 (1)	    1	0 (~src)    1	0 (~src)    1	1 (1)
 *  xor	    1	1 (1)	    1	0 (~src)    1	1 (1)	    1	0 (~src)
 *
 *	andInverted	noop		xor		or
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   0	0	0   0   0	0   0	1	0   0	1
 *	1   1	0	1   1   1	1   1	0	1   1	1
 *
 *  and	    1	0 (~src)    1   1 (1)	    1	1 (1)	    1	0 (~src)
 *  xor	    0	0 (0)	    0   0 (0)	    0	1 (src)	    0	1 (src)
 *
 *  or	    0	1 (src)	    0	0 (0)	    0	0 (0)	    0	1 (src)
 *  xor	    0	1 (src)	    0	0 (0)	    0	1 (src)	    0	0 (0)
 *
 *	nor		equiv		invert		orReverse
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   1	0	0   1   0	0   1	1	0   1	1
 *	1   0	0	1   0   1	1   0	0	1   0	1
 *
 *  and	    1	0 (~src)    1   1 (1)	    1	1 (1)	    1	0 (~src)
 *  xor	    1	0 (~src)    1   0 (~src)    1	1 (1)	    1	1 (1)
 *
 *  or	    0	1 (src)	    0	0 (0)	    0	0 (0)	    0	1 (src)
 *  xor	    1	1 (1)	    1	0 (~src)    1	1 (1)	    1	0 (~src)
 *
 *	copyInverted	orInverted	nand		set
 *     src  0	1	    0   1	    0	1	    0	1
 *  dst	0   1	0	0   1   0	0   1	1	0   1	1
 *	1   1	0	1   1   1	1   1	0	1   1	1
 *
 *  and	    0	0 (0)	    0   1 (src)	    0	1 (src)	    0	0 (0)
 *  xor	    1	0 (~src)    1   0 (~src)    1	1 (1)	    1	1 (1)
 *
 *  or	    1	1 (1)	    1	0 (~src)    1	0 (~src)    1	1 (1)
 *  xor	    0	1 (src)	    0	0 (0)	    0	1 (src)	    0	0 (0)
 */

int
cfbReduceRasterOp (rop, fg, pm, andp, xorp)
    int		    rop;
    unsigned long   fg, pm;
    unsigned long   *andp, *xorp;
{
    unsigned long   and, xor;
    int		    rrop;

    fg = PFILL (fg);
    pm = PFILL (pm);
    switch (rop)
    {
    case GXclear:
    	and = 0L;
    	xor = 0L;
	break;
    case GXand:
	and = fg;
	xor = 0L;
	break;
    case GXandReverse:
	and = fg;
	xor = fg;
	break;
    case GXcopy:
	and = 0L;
	xor = fg;
	break;
    case GXandInverted:
	and = ~fg;
	xor = 0L;
	break;
    case GXnoop:
	and = ~0L;
	xor = 0L;
	break;
    case GXxor:
	and = ~0L;
	xor = fg;
	break;
    case GXor:
	and = ~fg;
	xor = fg;
	break;
    case GXnor:
	and = ~fg;
	xor = ~fg;
	break;
    case GXequiv:
	and = ~0L;
	xor = ~fg;
	break;
    case GXinvert:
	and = ~0L;
	xor = ~0L;
	break;
    case GXorReverse:
	and = ~fg;
	xor = ~0L;
	break;
    case GXcopyInverted:
	and = 0L;
	xor = ~fg;
	break;
    case GXorInverted:
	and = fg;
	xor = ~fg;
	break;
    case GXnand:
	and = fg;
	xor = ~0L;
	break;
    case GXset:
	and = 0L;
	xor = ~0L;
	break;
    }
    and |= ~pm;
    xor &= pm;
    *andp = and;
    *xorp = xor;
    /* check for some special cases to reduce computation */
    if (and == 0L) 
	rrop = GXcopy;
    /* nothing checks for GXnoop 
    else if (and == ~0L && xor == 0L)
	rrop = GXnoop;
    */
    else if (and == ~0L) 
	rrop = GXxor;
    else if (xor == 0L) 
	rrop = GXand;
    else if (and ^ xor == ~0L) 
	rrop = GXor;
    else 
	rrop = GXset;   /* rop not reduced */
    return rrop;
}
