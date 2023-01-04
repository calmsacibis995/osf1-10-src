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
static char	*sccsid = "@(#)$RCSfile: floor.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1993/06/07 18:03:34 $";
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
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: floor, ceil, nearest, trunc
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
 * floor.c	1.13  com/lib/m,3.1,8946 11/15/89 15:17:07
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak nearest = __nearest
#pragma weak trunc = __trunc
#endif
#include <math.h>
#include <float.h>
#include <fp.h>

/*
 *
 *   This module contains the following routines:
 *
 *      double    floor(double x);
 *
 *      double    ceil(double x);
 *
 *      double    nearest(double x);
 *
 *      double    trunc(double x);
 *
 *
 *      Most of the code in this module was adapted from floor.c
 *      in the Berkeley math library. It has been modified to assume
 *      that the underlying hardware conforms to the IEEE standard and
 *      that the IEEE required and recommended functions are
 *      available.
 *
 *      Where applicable, the routines have been changed
 *      to reflect the ANSI C standard.
 *
 *      The routines assume that double IEEE floating point numbers are
 *      encoded with the sign in the ms bit followed by the exponent in
 *      the next 11 ms bits. (This is the common representation found in
 *      in almost all current implementations of the IEEE standard.)
 *
 *      Lastly, these routines assume the existence of a hardware
 *      specific function (write_rnd) that writes a new IEEE rounding
 *      mode and returns the previous rounding mode. The function takes
 *      and returns rounding modes with the encodings specified by the
 *      ANSI C Standard.
 *
 */


/*
 * NAME: floor
 *                                                                    
 * FUNCTION: RETURNS THE GREATEST DOUBLE INTEGER <= X
 *                                                                    
 * NOTES:
 *
 * RETURNS: the largest fp integer not greate than x
 *
 */

double
#ifdef _FRTINT
_floor(double x)
#else
floor(double x)
#endif
{
	int oldmode;
	double retval;

#ifndef _NAME_SPACE_WEAK_STRONG
	oldmode = write_rnd((unsigned int) FP_RND_RM);
	retval = rint(x);
	write_rnd((unsigned int) oldmode);
#else
	oldmode = __write_rnd((unsigned int) FP_RND_RM);
	retval = rint(x);
	__write_rnd((unsigned int) oldmode);
#endif
	return(retval);
}


#ifndef _FRTINT

/*
 * NAME: ceil
 *                                                                    
 * FUNCTION: RETURNS THE SMALLEST DOUBLE INTEGER >= X
 *                                                                    
 * NOTES:
 *
 * RETURNS: the smallest fp integer not less than x
 *
 */


double
ceil(double x)
{
	int oldmode;
	double retval;

#ifndef _NAME_SPACE_WEAK_STRONG
	oldmode = write_rnd((unsigned int) FP_RND_RP);
	retval = rint(x);
	write_rnd((unsigned int) oldmode);
#else
	oldmode = __write_rnd((unsigned int) FP_RND_RP);
	retval = rint(x);
	__write_rnd((unsigned int) oldmode);
#endif
	return(retval);
}

/*
 * NAME: nearest
 *                                                                    
 * FUNCTION: RETURNS THE DOUBLE INTEGER NEAREST TO X.
 *                                                                    
 * NOTES:
 *   If x lies half way between two double integers then the even 
 *   integer is returned.
 *
 * RETURNS: nearest fp integer to x
 *
 */


double
nearest(double x)
{
	int oldmode;
	double retval;

#ifndef _NAME_SPACE_WEAK_STRONG
	oldmode = write_rnd((unsigned int) FP_RND_RN);
	retval = rint(x);
	write_rnd((unsigned int) oldmode);
#else
	oldmode = __write_rnd((unsigned int) FP_RND_RN);
	retval = rint(x);
	__write_rnd((unsigned int) oldmode);
#endif
	return(retval);
}



/*
 * NAME: trunc
 *                                                                    
 * FUNCTION: RETURNS THE NEAREST DOUBLE INTEGER TO X IN THE DIRECTION OF 0.0.
 *                                                                    
 * NOTES:
 *
 * RETURNS: nearest fp integer to x in the direction of 0
 *
 */


double
trunc(double x)
{
	int oldmode;
	double retval;

#ifndef _NAME_SPACE_WEAK_STRONG
	oldmode = write_rnd((unsigned int) FP_RND_RZ);
	retval = rint(x);
        write_rnd((unsigned int) oldmode);
#else
	oldmode = __write_rnd((unsigned int) FP_RND_RZ);
	retval = rint(x);
	__write_rnd((unsigned int) oldmode);
#endif
	return(retval);
}

#endif
