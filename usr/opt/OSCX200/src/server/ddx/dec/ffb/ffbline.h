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
 * @(#)$RCSfile: ffbline.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:11:36 $
 */
/*
 */

#ifndef FFBLINE_H
#define FFBLINE_H

#include "ffbfill.h"

extern void ffbLineSS();	    /* Solid single-pixel lines		    */
extern void ffbSegmentSS();	    /* Solid single-pixel segments	    */

extern void ffbDashLine();	    /* Solid single-pixel dashed lines      */
extern void ffbDashSegment();	    /* Solid single-pixel dashed segments   */

/* For dash patterns of length <= 16 */
extern void ffbDashLine16();
extern void ffbDashSegment16();

#define BOTHOUTCODES(oc1, oc2, pt1x, pt1y, pt2x, pt2y, pbox)    \
{								\
    int clipx_, clipy_;						\
    clipx_ = (pbox)->x1;					\
    clipy_ = (pbox)->y1;					\
    oc1 = (pt1x < clipx_) << SHIFT_LEFT;			\
    oc2 = (pt2x < clipx_) << SHIFT_LEFT;			\
    oc1 |= (pt1y < clipy_) << SHIFT_ABOVE;			\
    oc2 |= (pt2y < clipy_) << SHIFT_ABOVE;			\
    clipx_ = (pbox)->x2 - 1;					\
    clipy_ = (pbox)->y2 - 1;					\
    oc1 |= (pt1x > clipx_) << SHIFT_RIGHT;			\
    oc2 |= (pt2x > clipx_) << SHIFT_RIGHT;			\
    oc1 |= (pt1y > clipy_) << SHIFT_BELOW;			\
    oc2 |= (pt2y > clipy_) << SHIFT_BELOW;			\
} /* BOTHOUTCODES */


#define NextDash(pDash, numDashes, major, minor, which)		\
{								\
    /* Move to next dash */					\
    major++;							\
    if (major == numDashes) major = 0;				\
    minor = pDash[major];					\
    which = ~which;						\
} /* NextDash */

/* Accumulate dashed bits into the low 16 bits of dashBits.  For each dash
segment, put either 0 or 1 bits into high 16 bits of dashBits, then shift right
by the appropriate amount.  Note that if the parameter len < 16, then the data
bits will be in the HIGH part of the bottom 16 bits, so we shift it right. */

#if FFBLINEBITS == 16
#define GetDashBits(len, dashBits, pDash, numDashes, major, minor, which) \
{								\
    int tlenGDB, dlenGDB;					\
								\
    tlenGDB = len;						\
    do {							\
	if (which == EVENDASH) {				\
	    /* Put in 1's */					\
	    (dashBits) |= 0xffff0000;				\
	} else {						\
	    /* Put in 0's */					\
	    (dashBits) &= 0x0000ffff;				\
	}							\
	dlenGDB = tlenGDB;					\
	minor -= tlenGDB;					\
	if (minor <= 0) {					\
	    dlenGDB = minor + tlenGDB;				\
	    NextDash(pDash, numDashes, major, minor, which);    \
	}							\
	tlenGDB -= dlenGDB;					\
	(dashBits) >>= dlenGDB;					\
    } while (tlenGDB != 0);					\
} /* GetDashBits */
#else
/* You're on your own. */
#endif

#endif /* FFBLINE_H */

/*
 * HISTORY
 */
