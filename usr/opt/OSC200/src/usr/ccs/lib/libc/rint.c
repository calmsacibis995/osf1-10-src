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
static char	*sccsid = "@(#)$RCSfile: rint.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:28:04 $";
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
 * COMPONENT_NAME: LIBCCNV rint
 *
 * FUNCTIONS: rint
 *
 * ORIGINS: 11, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * 
 * Use and reproduction of this software are granted  in  accordance  with
 * the terms and conditions specified in  the  Berkeley  Software  License
 * Agreement (in particular, this entails acknowledgement of the programs'
 * source, and inclusion of this notice) with the additional understanding
 * that  all  recipients  should regard themselves as participants  in  an
 * ongoing  research  project and hence should  feel  obligated  to report
 * their  experiences (good or bad) with these elementary function  codes,
 * using "sendbug 4bsd-bugs@BERKELEY", to the authors.
 *
 * rint.c	1.7  com/lib/c/cnv,3.1,8943 9/13/89 09:27:51
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak rint = __rint
#endif
#endif
#include <math.h>
#include <float.h>

/*
 * NAME: rint
 *                                                                    
 * FUNCTION: Returns a double integer that is one of the two integers that
 *	     bounds x depending on the rounding mode
 *                                                                    
 * NOTES:
 *
 *      double    rint(double x);
 *
 *      Most of the code in this module was adapted from floor.c
 *      in the Berkeley math library. It has been modified to assume
 *      that the underlying hardware conforms to the IEEE standard and
 *      that the IEEE required and recommended functions are
 *      available.
 *
 *      Rint assumes that double IEEE floating point numbers are
 *      encoded with the sign in the ms bit followed by the exponent in
 *      the next 11 ms bits. (This is the common representation found in
 *      in almost all current implementations of the IEEE standard.)
 *
 *      Lastly, rint assumes the existence of a hardware
 *      specific function (write_rnd) that writes a new IEEE rounding
 *      mode and returns the previous rounding mode. The function takes
 *      and returns rounding modes with the encodings specified by the
 *      ANSI C Standard.
 *
 *   double rint(double x)
 *
 *   Returns a double integer that is one of the two integers that
 *   bound x. The integer that is returned is determined by the
 *   current IEEE rounding mode in affect at the time rint is called.
 *
 * ******************************************************************
 *
 * algorithm for rint(x) in pseudo-pascal form ...
 *
 * Adapted from Berkeley version. Added check to make sure that
 * the properly signed 0.0 is returned. Also added code to make sure
 * that some floating point operation is performed on a SNaN to cause
 * it to turn into a QNaN
 *
 * real rint(x): real x;
 * const        L = 2**52         ... largest integer / 2
 * real s,t,retval;
 * begin
 *      if x != x then return x*1.0;         ... NaN
 *      if |x| >= L then return x;           ... already an integer
 *      s := copysign(L,x);
 *      t := x + s;                          ... = (x+s) rounded to integer
 *      retval = t - s;                      ... remove s leaving the int
 *      if retval EQ 0.0                     ... get correct sign of 0.0
 *              retval = retval with sign of x;
 *      return retval;
 * end;
 *
 * Note: Inexact will be signaled if x is not an integer, as is
 *      customary for IEEE 754.  Invalid (SNaN) will be signaled if
 *      x is a signalling NaN.
 *
 */

 /* The following static constant is used by rint */

static double L = 4503599627370496.0E0;   /* 2**52 -- the magic number */


double
rint(x)
double x;
{
	double s,t,retval;

	if (x != x)			/* NaN?*/
		return (x + 1.0);	/* convert SNaN to QNaN */
	if (fabs(x) >= L)  return(x);	/* already an integer? */
	s = copysign(L,x);		/* get proper sign on magic no. */
	t = x + s;			/* x+s rounded to integer */

	/* Make sure result has the sign of x, even if its zero. */
	if ( (retval = (t - s)) == 0.0 ) 
		retval = copysign(retval,x);
	return (retval);
}
