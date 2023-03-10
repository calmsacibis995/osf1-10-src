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
 *	@(#)$RCSfile: values.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:06:12 $
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
/*	@(#)values.h	3.1	*/
/*		*/
/*	2/26/91	*/
/*	5.2 SID values.h	1.16	*/

#ifndef BITSPERBYTE
/* These values work with any binary representation of integers
 * where the high-order bit contains the sign. */

/* a number used normally for size of a shift */
#if gcos
#define BITSPERBYTE	9
#else
#define BITSPERBYTE	8
#endif
#define BITS(type)	(BITSPERBYTE * (int)sizeof(type))

/* short, regular and long ints with only the high-order bit turned on */
#define HIBITS	((short)(1 << BITS(short) - 1))
#define HIBITI	(1 << BITS(int) - 1)
#define HIBITL	(1L << BITS(long) - 1)

/* largest short, regular and long int */
#define MAXSHORT	((short)~HIBITS)
#define MAXINT	(~HIBITI)
#define MAXLONG	(~HIBITL)

/* various values that describe the binary floating-point representation
 * _EXPBASE	- the exponent base
 * DMAXEXP 	- the maximum exponent of a double (as returned by frexp())
 * FMAXEXP 	- the maximum exponent of a float  (as returned by frexp())
 * DMINEXP 	- the minimum exponent of a double (as returned by frexp())
 * FMINEXP 	- the minimum exponent of a float  (as returned by frexp())
 * MAXDOUBLE	- the largest double
			((_EXPBASE ** DMAXEXP) * (1 - (_EXPBASE ** -DSIGNIF)))
 * MAXFLOAT	- the largest float
			((_EXPBASE ** FMAXEXP) * (1 - (_EXPBASE ** -FSIGNIF)))
 * MINDOUBLE	- the smallest double (_EXPBASE ** (DMINEXP - 1))
 * MINFLOAT	- the smallest float (_EXPBASE ** (FMINEXP - 1))
 * DSIGNIF	- the number of significant bits in a double
 * FSIGNIF	- the number of significant bits in a float
 * DMAXPOWTWO	- the largest power of two exactly representable as a double
 * FMAXPOWTWO	- the largest power of two exactly representable as a float
 * _IEEE	- 1 if IEEE standard representation is used
 * _DEXPLEN	- the number of bits for the exponent of a double
 * _FEXPLEN	- the number of bits for the exponent of a float
 * _HIDDENBIT	- 1 if high-significance bit of mantissa is implicit
 * LN_MAXDOUBLE	- the natural log of the largest double  -- log(MAXDOUBLE)
 * LN_MINDOUBLE	- the natural log of the smallest double -- log(MINDOUBLE)
 */
#if (u3b || u3b5 || m68) && !ns32000
#define MAXDOUBLE	1.79769313486231470e+308
#define MAXFLOAT	((float)3.40282346638528860e+38)
#define MINDOUBLE	4.94065645841246544e-324
#define MINFLOAT	((float)1.40129846432481707e-45)
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
#define FMINEXP	(-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))
#endif
#if ns32000 
#define MAXDOUBLE	1.7976931348623140e+308
#define MAXFLOAT	((float)3.40282346e+38)
#define MINDOUBLE	2.2250738585072030e-308
#define MINFLOAT	((float)1.17549436e-38)
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP  - 3))
#define FMINEXP	(-(FMAXEXP  - 3))
#endif
#if ( pdp11 || vax ) && !ns32000
#define MAXDOUBLE	1.701411733192644299e+38
#define MAXFLOAT	((float)1.701411733192644299e+38)
/* The following is kludged because the PDP-11 compilers botch the simple form.
   The kludge causes the constant to be computed at run-time on the PDP-11,
   even though it is still "folded" at compile-time on the VAX. */
#define MINDOUBLE	(0.01 * 2.938735877055718770e-37)
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	1
#define DMINEXP	(-DMAXEXP)
#define FMINEXP	(-FMAXEXP)
#endif
#if gcos
#define MAXDOUBLE	1.7014118346046923171e+38
#define MAXFLOAT	((float)1.7014118219281863150e+38)
#define MINDOUBLE	2.9387358770557187699e-39
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	0
#define DMINEXP	(-(DMAXEXP + 1))
#define FMINEXP	(-(FMAXEXP + 1))
#endif
#if u370
#define _LENBASE	4
#else
#define _LENBASE	1
#endif
#define _EXPBASE	(1 << _LENBASE)
#define _FEXPLEN	8
#define DSIGNIF	(BITS(double) - _DEXPLEN + _HIDDENBIT - 1)
#define FSIGNIF	(BITS(float)  - _FEXPLEN + _HIDDENBIT - 1)
#define DMAXPOWTWO	((double)(1L << BITS(long) - 2) * \
				(1L << DSIGNIF - BITS(long) + 1))
#define FMAXPOWTWO	((float)(1L << FSIGNIF - 1))
#define DMAXEXP	((1 << _DEXPLEN - 1) - 1 + _IEEE)
#define FMAXEXP	((1 << _FEXPLEN - 1) - 1 + _IEEE)
#define LN_MAXDOUBLE	(M_LN2 * DMAXEXP)
#define LN_MINDOUBLE	(M_LN2 * (DMINEXP - 1))
#define H_PREC	(DSIGNIF % 2 ? (1L << DSIGNIF/2) * M_SQRT2 : 1L << DSIGNIF/2)
#define X_EPS	(1.0/H_PREC)
#define X_PLOSS	((double)(long)(M_PI * H_PREC))
#define X_TLOSS	(M_PI * DMAXPOWTWO)
#define M_LN2	0.69314718055994530942
#define M_PI	3.14159265358979323846
#define M_SQRT2	1.41421356237309504880
#define MAXBEXP	DMAXEXP /* for backward compatibility */
#define MINBEXP	DMINEXP /* for backward compatibility */
#define MAXPOWTWO	DMAXPOWTWO /* for backward compatibility */
#endif
