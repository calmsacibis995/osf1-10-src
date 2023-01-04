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
static char	*sccsid = "@(#)$RCSfile: ldexp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:03:08 $";
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
/* @(#)ldexp.c	3.1 */
/*  */
/* 2/26/91 */
/*LINTLIBRARY*/
/*
 * $ Header: ldexp.c,v 10.1 87/11/04 09:58:36 mbj Exp $
 *	Replaces System 5.2 version
 *
 * double ldexp (value, exp)
 *	double value;
 *	int exp;
 *
 * ldexp returns
 *	value*(2^exp)	if the result is in range
 *	0.0		if underflow occurs
 *	HUGE_VAL	if overflow occurs and value > 0
 *	-HUGE_VAL	if overflow occurs and value < 0
 *
 * If underflow or overflow occurs, errno is set to ERANGE.
 *	Since errno is not modified if there is no error, a
 *	program which tests errno after calling ldexp should
 *	set it to a value other than ERANGE (like 0) before
 *	calling ldexp.
 *
 * Implementation note:
 *	This is a non-portable implementation which works for IEEE floating
 *		point format only.
 */

#include <math.h>

#undef MAXFLOAT
#undef M_PI
#undef M_SQRT2
#undef M_LN2
#include <values.h>
#include <nan.h>
#include <errno.h>

#define	BIAS	1023

double 
ldexp(double value, register int exp)
{
	register union __NaN *px = (union __NaN *)&value;
	register int newexp;

	if (px->ds.de == 0x7FF) {
	  	errno = EDOM;
		return(value);
	}

	if (exp == 0 || value == 0.0) /* nothing to do for zero */
		return (value);

	newexp = exp + px->ds.de - BIAS;
	if (newexp > DMAXEXP) {		/* overflow */
		errno = ERANGE;
		return (value < 0 ? -HUGE_VAL : HUGE_VAL);
	}
	else if (newexp < DMINEXP) {	/* underflow */
		errno = ERANGE;
		return (0.0);
	}
	else {				/* just right */
		px->ds.de = newexp + BIAS;
		return (value);
	}
}
