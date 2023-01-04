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
#ifndef _VALUES_H_
/*
 *       @(#)$RCSfile: values.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/03/03 15:43:51 $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/*
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/PMAX/values.h,v 4.2.4.3 1992/03/03 15:43:51 Al_Delorey Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#define _VALUES_H_ 

#ifndef _BITSPERBYTE
#define _BITSPERBYTE

/* These values work with any binary representation of integers
 * where the high-order bit contains the sign. */

/* a number used normally for size of a shift */
#if __gcos
#define BITSPERBYTE	9
#else
#define BITSPERBYTE	8
#endif
#define BITS(type)	(BITSPERBYTE * (int)sizeof(type))

/* short, regular and long ints with only the high-order bit turned on */
#define HIBITS	((short)(1 << BITS(short) - 1))
#define HIBITI	(1 << BITS(int) - 1)
#define HIBITL	(1L << BITS(long) - 1)
#if defined(_LONGLONG)
#define HIBITLL	(1 << BITS(long long) - 1)
#endif

/* largest short, regular and long int */
#define MAXSHORT	((short)~HIBITS)
#define MAXINT	(~HIBITI)
#define MAXLONG	(~HIBITL)
#if defined(_LONGLONG)
#define MAXLONGLONG	(~HIBITLL)
#endif

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
 * LN_MAXFLOAT 	- the natural log of the largest float  -- log(MAXFLOAT)
 * LN_MINFLOAT 	- the natural log of the smallest float -- log(MINFLOAT)
 */
#if __u3b || _M32 || __u3b15 || __u3b5 || __u3b2 || __mips__
#define MAXDOUBLE	1.7976931348623157e+308
#ifndef _MAXFLOAT
#define _MAXFLOAT
#define MAXFLOAT	((float)3.40282347e+38)
#endif
#define MINDOUBLE	2.2250738585072014e-308
#define MINFLOAT	((float)1.17549435e-38)
#define	_IEEE		1
#define _DEXPLEN	11
#define _HIDDENBIT	1
#define DMINEXP	(-(DMAXEXP + DSIGNIF - _HIDDENBIT - 3))
#define FMINEXP	(-(FMAXEXP + FSIGNIF - _HIDDENBIT - 3))
#endif
#if __pdp11 || __vax 
#define MAXDOUBLE	1.701411834604692293e+38

#ifndef _MAXFLOAT
#define _MAXFLOAT
#define MAXFLOAT	((float)1.701411733192644299e+38)
#endif 
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
#if __gcos
#define MAXDOUBLE	1.7014118346046923171e+38
#ifndef _MAXFLOAT
#define _MAXFLOAT
#define MAXFLOAT	((float)1.7014118219281863150e+38)
#endif
#define MINDOUBLE	2.9387358770557187699e-39
#define MINFLOAT	((float)MINDOUBLE)
#define _IEEE		0
#define _DEXPLEN	8
#define _HIDDENBIT	0
#define DMINEXP	(-(DMAXEXP + 1))
#define FMINEXP	(-(FMAXEXP + 1))
#endif
#if __u370
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
#ifndef M_LN2
#define M_LN2      6.9314718055994530942E-1 /*Hex  2^-1 * 1.62E42FEFA39EF */
#endif
#ifndef M_PI
#define M_PI       3.1415926535897932846E0  /*Hex  2^ 1 * 1.921FB54442D18 */
#endif
#ifndef M_SQRT2
#define M_SQRT2    1.4142135623730950488E0  /*Hex  2^ 0 * 1.6A09E667F3BCD */
#endif
#define LN_MAXDOUBLE	(M_LN2 * DMAXEXP)
#define LN_MINDOUBLE	(M_LN2 * (DMINEXP - 1))
#define LN_MAXFLOAT 	(M_LN2 * FMAXEXP)
#define LN_MINFLOAT 	(M_LN2 * (FMINEXP - 1))
#define H_PREC	(DSIGNIF % 2 ? (1L << DSIGNIF/2) * M_SQRT2 : 1L << DSIGNIF/2)
#define X_EPS	(1.0/H_PREC)
#define X_PLOSS	((double)(long)(M_PI * H_PREC))
#define X_TLOSS	(M_PI * DMAXPOWTWO)
#define MAXBEXP	DMAXEXP /* for backward compatibility */
#define MINBEXP	DMINEXP /* for backward compatibility */
#define MAXPOWTWO	DMAXPOWTWO /* for backward compatibility */
#endif
#endif /* _VALUES_H_ */
