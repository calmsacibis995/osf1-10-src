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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/server/ddx/dec/sfb/sfbscanfill.h,v 1.1.3.2 92/01/06 16:06:46 David_Coleman Exp $ */
#ifndef SCANFILLINCLUDED
#define SCANFILLINCLUDED
/*
 *     scanfill.h
 *
 *     Originally written by Brian Kelleher; Jan 1985
 *     Transformed algebraically to much simpler and faster functions
 *     by Joel McCormack; Dec 1988
 *
 *     This file contains a few macros to help track
 *     the edge of a filled object.  The object is assumed
 *     to be filled in scanline order, and thus the
 *     algorithm used is an extension of Bresenham's line
 *     drawing algorithm which assumes that y is always the
 *     major axis.
 *     Since these pieces of code are the same for any filled shape,
 *     it is more convenient to gather the library in one
 *     place, but since these pieces of code are also in
 *     the inner loops of output primitives, procedure call
 *     overhead is out of the question.
 *     See the author for a derivation if needed.
 */

/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */

#define BRESINITPGON(dy, x1, x2, xStart, e, sign, step, e1, e2) {   \
    register int dx;      /* local storage */			    \
								    \
    /*								    \
     *  If the edge is horizontal, then it was already ignored.     \
     *  Otherwise, do this stuff, as we know that dy > 0.	    \
     */								    \
    xStart = (x1);						    \
    dx = (x2) - xStart;						    \
    step = dx / (dy);						    \
    e2 = 2 * (dy);						    \
    if (dx < 0) {						    \
	step = -1;						    \
	e1 = -(dy);						    \
	if (dx + (dy) <= 0) { /* abs(dx) >= dy */		    \
	    step += dx / (dy);					    \
	    e1 = (dy) * step;					    \
	}							    \
	sign = -1;						    \
	e1 = 2 * (e1 - dx);					    \
	e = e1 + 1;						    \
    } else {							    \
	step = 1;						    \
	e1 = dy;						    \
	if (dx >= (dy)) { /* abs(dx) >= dy */			    \
	    step += dx / (dy);					    \
	    e1 = (dy) * step;					    \
	}							    \
	sign = 1;						    \
	e1 = 2 * (dx - e1);					    \
	e = e1 + e2;						    \
    }								    \
} /* BRESINITPGON */



#define BRESINCRPGON(e, minval, sign, step, e1, e2) {   \
    if (e <= 0) {					\
	minval -= sign;					\
	e += e2;					\
    }							\
    minval += step;					\
    e += e1;						\
} /* BRESINCRPGON */

#endif SCANFILLINCLUDED
