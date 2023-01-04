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
 * @(#)$RCSfile: dtoa_ieee.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/07 22:46:06 $
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991 by AT&T.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR AT&T MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* #define switches:
 * SUDDEN_UNDERFLOW	for IEEE-format machines without gradual
 *			underflow (i.e., that flush to zero on underflow).
 * IBM			for IBM mainframe-style floating-point arithmetic.
 * VAX			for VAX-style floating-point arithmetic.
 * UNSIGNED_SHIFTS	if >> does treats its left operand as unsigned.
 * NO_LEFTRIGHT		to omit left-right logic in fast floating-point
 *			computation of _dtoa.
 * CHECK_FLT_ROUNDS	if FLT_ROUNDS can assume the values 2 or 3.
 * RND_PRODQUOT		to use rnd_prod and rnd_quot (assembly routines
 *			that use extended-precision instructions to compute
 *			rounded products and quotients) with IBM.
 * ROUND_BIASED		for IEEE-format with biased rounding.
 * INNACCURATE_DIVIDE	for IEEE-format with correctly rounded
 *			products but inaccurate quotients, e.g., for Intel i860.
 *
 * #define values:
 * EXACTPOWTEN	largest power of ten which can be represented
 *		exactly in a double (floor(DBL_MANT_DIG*log(2)/log(5)))
 * BLETCH	(highest power of 2 < DBL_MAX_10_EXP) / 16
 * QUICK_MAXP	floor((DBL_MANT_DIG-1)*log(FLT_RADIX)/log(10) - 1)
 * INT_MAXP	floor(DBL_MANT_DIG*log(FLT_RADIX)/log(10) - 1)
 */


#define	EXP_SHIFT	20
#define	EXP_SHIFT1	20
#define	EXP_MSK1	0x100000
#define	EXP_MSK11	0x100000
#define	EXP_MASK	0x7ff00000
#define	BIAS		1023
#define	EMIN		(-1022)
#define	EXP_1		0x3ff00000
#define	EXP_11		0x3ff00000
#define	EBITS		_DEXPLEN
#define	FRAC_MASK	0xfffff
#define	FRAC_MASK1	0xfffff
#define	EXACTPOWTEN	22
#define	BLETCH		0x10
#define	BNDRY_MASK	0xfffff
#define	BNDRY_MASK1	0xfffff
#define	LSB		1
#define	SIGN_BIT	0x80000000
#define	LOG2P		1
#define	TINY0		0
#define	TINY1		1
#define	QUICK_MAXP	14
#define	INT_MAXP	14
