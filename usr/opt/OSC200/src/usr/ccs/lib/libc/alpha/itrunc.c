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
static char	*sccsid = "@(#)$RCSfile: itrunc.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/09/22 13:05:46 $";
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
 * COMPONENT_NAME: LIBCCNV itrunc
 *
 * FUNCTIONS: itrunc
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
 * itrunc.c	1.7  com/lib/c/cnv,3.1,8943 9/13/89 09:23:52
 */

/*
 * NAME: itrunc
 *                                                                    
 * FUNCTION: returns the nearest signed integer to x in the direction of 0
 *                                                                    
 * NOTES:
 *
 *   int  itrunc(double x);          --- returns signed int
 *
 *   Portable Version
 *
 *   Returns the nearest signed integer to x in the direction of
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
 * RETURNS: an integer value
 *	    INT_MIN if argument is a NaN or a too-large negative number
 *	    INT_MAX if argument is a too-large positive number
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak itrunc = __itrunc
#endif
#include <limits.h>
#include <math.h>
#include <float.h>
#include <fp.h>
#include <machine/fpu.h>

/* The following static constant is used by rint, itrunc, and uitrunc */

static double L = 4503599627370496.0E0;   /* 2**52 -- the magic number */

int
itrunc(x)
double x;
{
	unsigned oldmode;
	double y;

	if (x != x) {			/* Check for NaN */
		/* set invalid exception bit */
		ieee_set_fp_control(ieee_get_fp_control()&FPCR_INV);
		return (INT_MIN);
	}

	if (x < (double) INT_MIN) {	/* if x is too-large negative */
	      /* set invalid exception bit */
	      ieee_set_fp_control(ieee_get_fp_control()&FPCR_INV);
	      return(INT_MIN);
	}
	else if (x > (double) INT_MAX) { /* if x is too-large positive */
	      /* set invalid exception bit */
	      ieee_set_fp_control(ieee_get_fp_control()&FPCR_INV);
	      return(INT_MAX);
	}

	oldmode = write_rnd(FP_RND_RZ);  /* Set to do truncate         */
	y = x + L;                       /* Add the magic number       */
	write_rnd(oldmode);              /* Restore caller's rnd mode  */

	/* The integer is now right justified in the low part of y  */
	return(VALL(y));
}

