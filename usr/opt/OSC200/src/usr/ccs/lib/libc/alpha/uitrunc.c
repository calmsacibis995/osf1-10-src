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
static char	*sccsid = "@(#)$RCSfile: uitrunc.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/06/07 19:47:00 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: LIBCCNV uitrunc
 *
 * FUNCTIONS: uitrunc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * uitrunc.c	1.7  com/lib/c/cnv,3.1,8943 10/27/89 16:15:18
 */

/*
 * NAME: uitrunc
 *                                                                    
 * FUNCTION: return an unsigned integer
 *                                                                    
 * NOTES:
 *
 *   unsigned uitrunc(double x);    --- returns unsigned int
 *
 *   Portable Version
 *
 *   Returns the nearest unsigned integer to x in the direction of
 *   0.0. This is equivalent to discarding the fraction of x (truncating x).
 *
 *   Note: This routine differs from trunc() because it returns an
 *         integer rather a double. This function is trivial
 *         on any machine that has hardware/software to correctly
 *         convert the truncated double to an integer as specified
 *         in the IEEE standard. This may not be the case for all
 *         hardware in which case this routine would have to handle
 *         things like infinities, NaN's, etc.
 *
 * RETURNS: an unsigned integer
 *	    0 for NaN or negative number
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak uitrunc = __uitrunc
#endif
#include <math.h>
#include <float.h>
#include <fp.h>
#include <machine/fpu.h>

/* The following static constant is used by rint and uitrunc */

static double L = 4503599627370496.0E0;   /* 2**52 -- the magic number */


unsigned
uitrunc(x)
double x;
{
	unsigned oldmode;
	double y;

	if (( x != x ) || (x < 0.0))  {     /* if NaN or negative number */
	      /* set invalid exception bit */
	      ieee_set_fp_control(ieee_get_fp_control()&FPCR_INV);
	      return(0);                    /* return 0 */
	}

	if ( x >= 4294967296.0) {           /* if x too big to convert   */
	      /* set invalid exception bit */
	      ieee_set_fp_control(ieee_get_fp_control()&FPCR_INV);
	      return(0xffffffff);           /* return max unsigned nbr.  */
	}
/* conditionalized for _NAME_SPACE_WEAK_STRONG */
#ifndef _NAME_SPACE_WEAK_STRONG
	oldmode = write_rnd(FP_RND_RZ);    /* Set to do truncate         */
	y = x + L;                         /* Add the magic number       */
	write_rnd(oldmode);                /* restore caller's rnd mode  */
#else
	oldmode = __write_rnd(FP_RND_RZ);  /* Set to do truncate         */
	y = x + L;                         /* Add the magic number       */
	__write_rnd(oldmode);              /* restore caller's rnd mode  */
#endif

	/* The integer is now right justified in the low part of y  */
	return(VALL(y));
}
