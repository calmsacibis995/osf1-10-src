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
 *	@(#)$RCSfile: fp.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/11 19:30:33 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef _FP_H_
#define _FP_H_

#include <machine/endian.h>

/*
 *      VALH(val)
 *
 *      Return the hipart of the double value of val as an unsigned integer.
 *      val must be a simple variable that can have its address taken.
 */

#define VALH(val) *((unsigned int *)&(val) + (BYTE_ORDER!=BIG_ENDIAN))


/*
 *      VALL(val)
 *
 *      Return the lopart of the double value of val as an unsigned integer.
 *      val must be a simple variable that can have its address taken.
 */

#define VALL(val) *(((unsigned int *)&(val))+(BYTE_ORDER==BIG_ENDIAN))

/*
 *      DBL(val,hi,lo)
 *
 *      The hi word of the double variable val is set to hi and
 *      the lo word of the double variable val is set to lo.
 *
 *      val must be a simple variable.
 */
#define DBL(val,hi,lo)	\
{			\
	VALH(val) = hi;	\
	VALL(val) = lo;	\
}

/*
 *      FINITE(x)
 *
 *      Is true if double x is finite (Not NaN or INF).
 *
 *      This macro is similar to the finite(x) function in the IEEE
 *      standard except it is not a function. 
 *	x must be a simple variable and not an expresion.
 */

#define FINITE(x)	( ( VALH(x) & 0x7ff00000 ) != 0x7ff00000 )

/*
 *      IS_INF(x)
 *
 *      Is true if double x is +INF or -INF.
 *      x must be a simple variable and not an expression.
 */

#define IS_INF(x) \
	( ( ( VALH(x) & 0x7fffffff ) == 0x7ff00000 ) && \
		( VALL(x) == 0 ) )

/*
 *      IS_QNAN(x)
 *
 *      Is true if double x is a Quiet NaN.
 *      x must be a simple variable and not an expression.
 */

#define IS_QNAN(x) \
	( ( VALH(x) & 0x7ff80000 ) == 0x7ff80000 )

/*
 *      IS_SNAN(x)
 *
 *      Is true if double x is a Signaling NaN.
 *      x must be a simple variable and not an expression.
 */

#define IS_SNAN(x) \
	( IS_NAN(x) && !IS_QNAN(x))
  
/*
 *      IS_NAN(x)
 *
 *      Is true if double x is any NaN.
 *      x must be a simple variable and not an expression.
 */

#define IS_NAN(x) \
	( ( ( VALH(x) & 0x7ff00000 ) == 0x7ff00000 ) && \
		( ( VALH(x) & 0x000fffff) | VALL(x) ) )

/*
 *      IS_ZERO(x)
 *
 *      Is true if double x is +0 or -0.
 *      x must be a simple variable and not an expression.
 */

#define IS_ZERO(x) \
	( !( ( VALH(x) & 0x7fffffff ) | VALL(x) ) )

/*
 *  INTS2DBL(x,y)
 *
 *  Put two unsigned long integers into IEEE double format for the
 *  current machine architecture.
 */
#if BYTE_ORDER == BIG_ENDIAN
#	define	INTS2DBL(x,y)	(x),(y)
#else
#	define	INTS2DBL(x,y)	(y),(x)
#endif /* BYTE_ORDER==BIG_ENDIAN */

#endif /* _FP_H_ */
