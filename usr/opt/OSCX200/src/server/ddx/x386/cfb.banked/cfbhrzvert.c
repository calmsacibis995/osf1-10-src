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
/* $XConsortium: cfbhrzvert.c,v 1.5 90/01/31 12:31:37 keith Exp $ */
#include "X.h"

#include "gc.h"
#include "window.h"
#include "pixmap.h"
#include "region.h"

#include "cfb.h"
#include "cfbmskbits.h"
#include "vgaBank.h"

/* horizontal solid line
   abs(len) > 1
*/
cfbHorzS(rop, and, xor, addrl, nlwidth, x1, y1, len)
register int rop;
register unsigned long and;
register unsigned long xor;
register int *addrl;	/* pointer to base of bitmap */
int nlwidth;		/* width in longwords of bitmap */
int x1;			/* initial point */ 
int y1;
int len;		/* length of line */
{
    register int nlmiddle;
    register int startmask;
    register int endmask;

    BANK_FLAG(addrl)

    addrl = addrl + (y1 * nlwidth) + (x1 >> PWSH);
    SETRW(addrl);

    /* all bits inside same longword */
    if ( ((x1 & PIM) + len) < PPW)
    {
	maskpartialbits(x1, len, startmask);
	*addrl = DoMaskRRop (*addrl, and, xor, startmask);
    }
    else
    {
	maskbits(x1, len, startmask, endmask, nlmiddle);
	if (rop == GXcopy)
	{
	    if (startmask)
	    {
		*addrl = (*addrl & ~startmask) | (xor & startmask);
		addrl++; CHECKRWO(addrl);
	    }
	    while (nlmiddle--) 
	      {
	    	*addrl++ = xor; CHECKRWO(addrl);
	      }
	    if (endmask)
		*addrl = (*addrl & ~endmask) | (xor & endmask);
	}
	else
	{
	    if (startmask)
	    {
		*addrl = DoMaskRRop (*addrl, and, xor, startmask);
		addrl++; CHECKRWO(addrl);
	    }
	    if (rop == GXxor)
	    {
		while (nlmiddle--)
		  {
		    *addrl++ ^= xor; CHECKRWO(addrl);
		  }
	    }
	    else
	    {
		while (nlmiddle--)
		{
		    *addrl = DoRRop (*addrl, and, xor);
		    addrl++; CHECKRWO(addrl);
		}
	    }
	    if (endmask)
		*addrl = DoMaskRRop (*addrl, and, xor, endmask);
	}
    }
}

/* vertical solid line */

cfbVertS(rop, and, xor, addrl, nlwidth, x1, y1, len)
int rop;
register unsigned long and, xor;
register int *addrl;	/* pointer to base of bitmap */
register int nlwidth;	/* width in longwords of bitmap */
int x1, y1;		/* initial point */
register int len;	/* length of line */
{
#if (PPW == 4)
    register unsigned char    *bits = (unsigned char *) addrl;

    BANK_FLAG(addrl);

    nlwidth <<= 2;
    bits = bits + (y1 * nlwidth) + x1;
    SETRW(bits);

    /*
     * special case copy and xor to avoid a test per pixel
     */
    if (rop == GXcopy)
    {
	while (len--)
	{
	    *bits = xor;
	    bits += nlwidth; CHECKRWO(bits);
	}
    }
    else if (rop == GXxor)
    {
	while (len--)
	{
	    *bits ^= xor;
	    bits += nlwidth; CHECKRWO(bits);
	}
    }
    else
    {
	while (len--)
	{
	    *bits = DoRRop(*bits, and, xor);
	    bits += nlwidth; CHECKRWO(bits);
	}
    }
#else
    addrl = addrl + (y1 * nlwidth) + (x1 >> PWSH);

    and |= ~cfbmask[x1 & PIM];
    xor &= cfbmask[x1 & PIM];

    while (len--)
    {
	*addrl = DoRRop (*addrl, and, xor);
	addrl += nlwidth;
    }
#endif
}
