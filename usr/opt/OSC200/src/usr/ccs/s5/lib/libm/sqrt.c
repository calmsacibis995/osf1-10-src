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
static char	*sccsid = "@(#)$RCSfile: sqrt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:04:02 $";
#endif 
/*
 */
/*
 *	sqrt returns the square root of its double-precision argument,
 *	using Newton's method.
 *	Returns EDOM error and value 0 if argument negative.
 *	Calls frexp and ldexp.
 */

#include <errno.h>
#include <math.h>
#define ITERATIONS	4

double
sqrt(x)
register double x;
{
	register double y;
	int iexp; /* can't be in register because of frexp() below */
	register int i = ITERATIONS;

	if (x <= 0) {
		struct exception exc;

		if (!x)
			return (x); /* sqrt(0) == 0 */
		exc.type = DOMAIN;
		exc.name = "sqrt";
		exc.arg1 = x;
		exc.retval = 0.0;
		if (!matherr(&exc)) {
			(void) write(2, "sqrt: DOMAIN error\n", 19);
			errno = EDOM;
		}
		return (exc.retval);
	}
	y = frexp(x, &iexp); /* 0.5 <= y < 1 */
	if (iexp % 2) { /* iexp is odd */
		--iexp;
		y += y; /* 1 <= y < 2 */
	}
	y = ldexp(y + 1.0, iexp/2 - 1); /* first guess for sqrt */
	do {
		y = 0.5 * (y + x/y);
	} while (--i > 0);
	return (y);
}
