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
#define IDENT "X-2"
#define MODULE_NAME CFBDECLINE
#include "module_ident.h"
#endif
/****************************************************************************
**                                                                          *
**                   COPYRIGHT (c) 1988, 1989, 1990 BY                      *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
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
/**
**++
**  FACILITY:
**
**      DDXCFB - VMS CFB server
**
**  ABSTRACT:
**
**      
**
**  AUTHORS:
**
**      Irene McCartney (from Joel McCormack)
**
**
**  CREATION DATE:     20-Nov-1991
**
**  MODIFICATION HISTORY:
**
**
** X-2		TLB0003		Tracy Bragdon			03-Dec-1991
**		add typedef for VoidProc 
**--
**/

/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/cfb/cfbdecline.c,v 1.1.2.5 92/03/30 17:46:09 Jim_Ludwig Exp $ */

/* Handles 0-width solid PolyLine and PolySegment requests via conditional
   compilation.  For lines with 1 clip rectangle, see the cfbsline.c file,
   which contains a very fast version with almost every procedure inline.
*/


#include "X.h"

#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "scrnintstr.h"
#include "mistruct.h"

#include "cfb.h"
#ifdef MITR5
#include "cfbdecbres.h"
#include "cfbdecsline.h"
#include "cfbdecline.h"
typedef void (* VoidProc) ();		/*  Procedure returning void   */
#else
#include "cfbbres.h"
#include "cfbsline.h"
#include "cfbline.h"
#endif


/* Single-pixel lines for an 8-bit frame buffer

NON-SLOPED LINES

Draw horizontal lines left to right; if necessary, swap endpoints (and move
right by one).  Horizontal lines are confined to a single band of a region, so
clipping against multiple clip rectangles is reasonably easy.  Find the band
(if one exists); then find the first box in that band that contains part of the
line.  Clip the line to subsequent boxes in that band.

Draw vertical lines from top to bottom (y-increasing.) This requires adding one
to the y-coordinate of each endpoint after swapping.

SLOPED LINES

When clipping a sloped line, bring the second point inside the clipping box,
rather than one beyond it, and then add 1 to the length of the line before
drawing it.  This lets us use the same box for finding the outcodes for both
endpoints.  Since the equation for clipping the second endpoint to an edge
gives us 1 beyond the edge, we then have to move the point towards the first
point by one step on the major axis.  Eventually, there will be a diagram here
to explain what's going on.  The method uses Cohen-Sutherland outcodes to
determine outsideness, and a method similar to Pike's layers for doing the
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
/*

static VoidProc solidLines1[4] = {
    cfb8LineSS1RectCopy,  cfb8LineSS1RectGeneral,   cfb8LineSS1RectXor,   cfb8LineSS1RectGeneral
};

static VoidProc solidSegs1[4] = {
    cfb8SegmentSS1RectCopy,   cfb8SegmentSS1RectGeneral,    cfb8SegmentSS1RectXor,    cfb8SegmentSS1RectGeneral
};

static VoidProc horzLines[4] = {
    cfbHorzSCopy,   cfbHorzSGeneral,    cfbHorzSXor,    cfbHorzSGeneral
};

static VoidProc vertLines[4] = {
    cfbVertSCopy,   cfbVertSGeneral,    cfbVertSXor,    cfbVertSGeneral
};

static VoidProc bresLines[4] = {
    cfbBresSCopy,   cfbBresSGeneral,    cfbBresSXor,    cfbBresSGeneral
};

static VoidProc ooDashLines[4] = {
    cfbOODashCopy,  cfbOODashGeneral,   cfbOODashXor,   cfbOODashGeneral
};

static VoidProc dDashLines[4] = {
    cfbDDashCopy,   cfbDDashGeneral,    cfbDDashXor,    cfbDDashGeneral
};

*/
void NewDashPos(dashDesc, dashPos, length)
    DashDesc	    *dashDesc;
    DashPos	    *dashPos;
    int		    length;
{
    int		    numPatterns;

    /* First try to get through an integral number of complete dash patterns */
    numPatterns = length / dashDesc->dashLength;
    /* Did we switch EVEN/ODD due to odd numPatterns and odd numDashes ? */
    if (numPatterns & dashDesc->numDashes & 1) {
	dashPos->which = ~dashPos->which;
    }

    /* Now go through individual dashes until we've eaten up all of length */
    length = length % dashDesc->dashLength;

    while (length >= dashPos->minor) {
	length -= dashPos->minor;
	dashPos->major++;
	if (dashPos->major == dashDesc->numDashes) {
	    dashPos->major = 0;
	}
	dashPos->minor = dashDesc->pDash[dashPos->major];
	dashPos->which = ~dashPos->which;
    }

    dashPos->minor -= length;
} /*NewDashPos */


void InitDashStuff(dashDesc, dashPos, gc)
    DashDesc	    *dashDesc;
    DashPos	    *dashPos;
    GCPtr	    gc;
{
    int		    i, dashLength;

    dashDesc->pDash = gc->dash;
    dashLength = 0;
    i = gc->numInDashList;
    dashDesc->numDashes = i;
    do {
	i--;
	dashLength += dashDesc->pDash[i];
    } while (i != 0);
    dashDesc->dashLength = dashLength;

    /* Process offset into dash pattern */
    dashPos->major = 0;
    dashPos->minor = dashDesc->pDash[0];
    dashPos->which = EVENDASH;
    NewDashPos(dashDesc, dashPos, gc->dashOffset);
} /* InitDashStuff */

#define SWAPPT(p1, p2)      \
{			    \
    DDXPointPtr t;	    \
    t = p1;		    \
    p1 = p2;		    \
    p2 = t;		    \
}

#define SWAPINT(i, j)       \
{			    \
    register int t;	    \
    t = i;		    \
    i = j;		    \
    j = t;		    \
}


int cfbClipLine(pbox, pt1, pt2, oc1, oc2,
	    adx, ady, signdx, signdy)
    register BoxPtr pbox;			/* box to clip to */
    register DDXPointPtr pt1, pt2;
    register int oc1;		/* preliminary outcodes */
	     int oc2;		
    register int adx, ady;
    register int signdx, signdy;
{
    register int  xorig, yorig;
    int swapped = 0;
    register unsigned long utmp;
    register int offset;
    register int t;

    xorig = pt1->x;
    yorig = pt1->y;
    do {
	/* only clip one point at a time */
	if (!oc1) {
	    SWAPPT(pt1, pt2);
	    SWAPINT(oc1, oc2);
	    swapped = !swapped;
	}

	if (oc1 & (OUT_LEFT | OUT_RIGHT)) {
	    if (oc1 & OUT_LEFT) {
		t = pbox->x1;
	    } else {
		t = pbox->x2 - 1;
	    }
	    pt1->x = t;
	    t = t - xorig;
	    utmp = t * ady;
	    if (t < 0) utmp = -utmp;
	    if (adx >= ady) {
		pt1->y = yorig + SignTimes(signdy, round(utmp, adx));
	    } else {
		utmp += utmp;
		if (swapped) {
		    utmp += ady;
		    offset = SignTimes(signdy, ceiling(utmp, 2*adx));
		    pt1->y = yorig + offset - signdy;
		} else {
		    utmp -= ady;
		    offset = SignTimes(signdy, ceiling(utmp, 2*adx));
		    pt1->y = yorig + offset;
		}
	    }

	} else { /* oc1 & (OUT_ABOVE | OUT_BELOW) */
	    if (oc1 & OUT_ABOVE) {
		t = pbox->y1;
	    } else {
		t = pbox->y2 - 1;
	    }
	    pt1->y = t;
	    t = t - yorig;
	    utmp = t * adx;
	    if (t < 0) utmp = -utmp;
	    if (adx < ady) {
		pt1->x = xorig + SignTimes(signdx, round(utmp, ady));
	    } else {
		utmp += utmp;
		if (swapped) {
		    utmp += adx;
		    offset = SignTimes(signdx, ceiling(utmp, 2*ady));
		    pt1->x = xorig + offset - signdx;
		} else {
		    utmp -= adx;
		    offset = SignTimes(signdx, ceiling(utmp, 2*ady));
		    pt1->x = xorig + offset;
		}
	    }
	}

	/* Next round? */
	OUTCODES(oc1, pt1->x, pt1->y, pbox);
	if (oc1 & oc2)
	    return -1;
	else if ((oc1 | oc2) == 0) {
	    if (swapped) {
		SWAPPT(pt1, pt2);
		SWAPINT(oc1, oc2);
	    }
	    return 1;
	}
    } while(1);
}
