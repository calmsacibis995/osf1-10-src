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
 *	@(#)$RCSfile: math.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:04:48 $
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

/*
 * COMPONENT_NAME: (math.h) math header file
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifdef _ANSI_C_SOURCE

#define HUGE_VAL 1.7e308

#endif /*_ANSI_C_SOURCE */

/*
 *
 * The following function prototypes define functions available in the
 * XIX system but not required by the ANSI standard. They will not be
 * included in _ANSI_C_SOURCE is defined (strict ANSI conformance).
 *
 */

#ifdef _XOPEN_SOURCE

/*
 *      Useful mathmatical constants:
 *
 * M_E          - e
 * M_LOG2E      - log2(e)
 * M_LOG10E     - log10(e)
 * M_LN2        - ln(2)
 * M_PI         - pi
 * M_PI_2       - pi/2
 * M_PI_4       - pi/4
 * M_1_PI       - 1/pi
 * M_2_PI       - 2/pi
 * M_2_SQRTPI   - 2/(sqrt(pi)
 * M_SQRT2      - sqrt(2)
 * M_SQRT_2     - 1/sqrt(2)
*/

#define M_E        2.7182818284590452354E0  /*Hex  2^ 0 * 1.5bf0a8b145769 */
#define M_LOG2E    1.4426950408889633870E0  /*Hex  2^ 0 * 1.71547652B82FE */
#define M_LOG10E   4.3429448190325181667E-1 /*Hex  2^-2 * 1.BCB7B1526E50E */
#define M_LN2      6.9314718055994530942E-1 /*Hex  2^-1 * 1.62E42FEFA39EF */
#define M_LN10     2.3025850929940456840E0  /*Hex  2^ 1 * 1.26bb1bbb55516 */
#define M_PI       3.1415926535897931160E0  /*Hex  2^ 1 * 1.921FB54442D18 */
#define M_PI_2     1.5707963267948965580E0  /*Hex  2^ 0 * 1.921FB54442D18 */
#define M_PI_4     7.8539816339744827900E-1 /*Hex  2^-1 * 1.921FB54442D18 */
#define M_1_PI     3.1830988618379067154E-1 /*Hex  2^-2 * 1.45f306dc9c883 */
#define M_2_PI     6.3661977236758134308E-1 /*Hex  2^-1 * 1.45f306dc9c883 */
#define M_2_SQRTPI 1.1283791670955125739E0  /*Hex  2^ 0 * 1.20dd750429b6d */
#define M_SQRT2    1.4142135623730951455E0  /*Hex  2^ 0 * 1.6A09E667F3BCD */
#define M_SQRT1_2  7.0710678118654752440E-1 /*Hex  2^-1 * 1.6a09e667f3bcd */


#ifndef MAXFLOAT
#define MAXFLOAT            ((float)3.40282346638528860e+38)
#endif


#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE

/*
 *      Useful mathmatical constants:
 *
 * HUGE         - +infinity
 * M_2PI        - 2*pi
 *
 */
#define HUGE       HUGE_VAL
#define M_2PI      6.2831853071795862320E0  /*Hex  2^ 2 * 1.921FB54442D18 */

/* This is the nearest number to the cube root of MAXDOUBLE that   */
/*      doesn't cause the cube of it to overflow.                  */
/* In double precision hex this constant is: 554428a2 f98d728a     */
#define CUBRTHUGE      5.6438030941223618e102
#define INV_CUBRTHUGE  1.7718548704178434e-103

#endif /* _OSF_SOURCE */
#endif /* _MATH_H_ */
