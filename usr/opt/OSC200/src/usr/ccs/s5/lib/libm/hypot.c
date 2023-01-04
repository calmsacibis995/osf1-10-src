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
static char	*sccsid = "@(#)$RCSfile: hypot.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:03:30 $";
#endif 
/*
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 *	hypot(a, b) returns sqrt(a^2 + b^2), avoiding unnecessary overflows.
 *	Returns ERANGE error and value HUGE if the correct value would overflow.
 */

#include <math.h>
#include <values.h>
#include <errno.h>
#define ITERATIONS	4

double
hypot(a, b)
register double a, b;
{
	register double t;
	register int i = ITERATIONS;
	struct exception exc;

	if ((exc.arg1 = a) < 0)
		a = -a;
	if ((exc.arg2 = b) < 0)
		b = -b;
	if (a > b) {				/* make sure |a| <= |b| */
		t = a;
		a = b;
		b = t;
	}
	if(b == 0.0) return(b);			/* both args are 0, result=0 */
	if ((t = a/b) < X_EPS)			/* t <= 1 */
		return (b);			/* t << 1 */
	a = 1 + t * t;				/* a = 1 + (a/b)^2 */
	t = 0.5 + 0.5 * a;			/* first guess for sqrt */
	do {
		t = 0.5 * (t + a/t);
	} while (--i > 0);			/* t <= sqrt(2) */
	if (b < MAXDOUBLE/M_SQRT2)		/* result can't overflow */
		return (t * b);
	if ((t *= 0.5 * b) < MAXDOUBLE/2)	/* result won't overflow */
		return (t + t);
	exc.type = OVERFLOW;
	exc.name = "hypot";
	exc.retval = HUGE;
	if (!matherr(&exc))
		errno = ERANGE;
	return (exc.retval);
}
