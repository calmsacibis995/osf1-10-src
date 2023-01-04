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
static	char	*sccsid = "@(#)frexp.c	9.1	(ULTRIX)	9/20/90";
#endif /* lint */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/**/
/*
 *
 *   File name:
 *
 *	frexp.c
 *
 *   Source file description:
 *
 *	This file contains the standard C library routine frexp(3).
 *
 *   Function:
 *
 *	frexp	Decompose a floating point number
 *
 *   Modification history:
 *
 *	Created November 12, 1987, Jon Reeves.  Based on algorithm in
 *	BSD 4.3 source.  Motivations: fix bug with 2**n case in BSD
 *	machine independent case/ULTRIX V2.2; improve speed to near
 *	BSD 4.3 assembler version by depending on VAX floating point 
 *	format without introducing dependence on VAX instruction set.
 *
 *	Revised, September 18, 1990, Jon Reeves.  Handle IEEE format
 *	for ALPHA.
 */

/*
 * Function:
 *
 *	frexp
 *
 * Function Description:
 *
 *	This function accepts a double precision number and returns its
 *	mantissa and exponent components separately.
 *
 *	Note: this function is almost like "assembly code in C"; it
 *	knows a great deal about the format of a double.
 *
 * Arguments:
 *
 *	name  	type  	description  		side effects 
 *	value	double	value to decompose	none
 *	eptr	int *	destination of exponent	exponent stored here
 *
 * Return value:
 *
 *	name 	type	description		values
 *	--	double	mantissa portion	0.5<=x<1.0 or x=0.0
 *	*eptr	int	exponent portion	-2**n<=*eptr<=2**n
 *						(n=10 for IEEE float)
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#define _IEEE 1
#include <nan.h>
#include <alpha/softfp.h>

#define BIAS (DEXP_BIAS-1)

/*
  Should be a union with a double, but the compiler won't let me
  statically initialize that, so we kludge at assignment time. 
 */
static unsigned int nanconst[2] = {DQUIETNAN_LESS, DQUIETNAN_LEAST};

double
frexp(value, eptr)
double	value;
int	*eptr;
{
	union {
		dnan n;
		unsigned long l;
	} kludge;

	kludge.n.d = value;
/*
	If the exponent field is nonzero (and nonspecial), we unbias it
	and return the fraction with a biased zero exponent.
*/
/* & 0x7ff is kludge around compiler bug */
	if ((*eptr = kludge.n.inf_parts.exponent & 0x7ff) != 0) {
		if (IsNANorINF(kludge.n.d)) {
#if 0 /* Compiler bug makes IsINF not work right */
			if (!IsINF(kludge.n.d))
#else
			if (kludge.n.inf_parts.bits & 0xfffff != 0 ||
				kludge.n.inf_parts.fraction_low != 0)
#endif
			/* NaN, clean it up */
				kludge.n.d = *(double *)nanconst;
			/* In either case, max out exponent */
			*eptr = WORD_MAX;
		} else {
		/* Normal case */
			*eptr -= BIAS;
			kludge.n.inf_parts.exponent = BIAS;
		}
	} else if (kludge.l != 0L) {
	/* Denormalized number */
		unsigned long temp;
		unsigned shiftct = 0;
		int sign;

		sign = kludge.n.inf_parts.sign;
		if (sign) kludge.n.d = - kludge.n.d;
		temp = kludge.l;
		if (!(temp>>32)) {shiftct += 32; temp <<= 32;}
		if (!(temp>>48)) {shiftct += 16; temp <<= 16;}
		if (!(temp>>56)) {shiftct += 8; temp <<= 8;}
		if (!(temp>>60)) {shiftct += 4; temp <<= 4;}
		if (!(temp>>62)) {shiftct += 2; temp <<= 2;}
		if (!(temp>>63)) shiftct += 1;
		shiftct -= DFRAC_LEAD0S;
		*eptr = -DEXP_BIAS - shiftct;
/* Last constant is DFRAC_MASK, but compiler doesn't handle it right */
		kludge.l = (((long)BIAS) << DEXP_SHIFT) | ((kludge.l << shiftct) & ~(0xfffL<<52));
		if (sign) kludge.n.d = - kludge.n.d;
	}
/*	Else zero, and everything is already right (*eptr set in first if) */

	return kludge.n.d;
}
