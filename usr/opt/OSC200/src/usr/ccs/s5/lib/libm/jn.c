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
static char	*sccsid = "@(#)$RCSfile: jn.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:03:42 $";
#endif 
/*
 */
/*
 *	Double-precision Bessel's function of
 *	the first and second kinds and of
 *	integer order.
 *
 *	jn(n, x) returns the value of Jn(x) for all
 *	integer values of n and all real values
 *	of x.
 *	Returns ERANGE error and value 0 for large arguments.
 *	Calls j0, j1.
 *
 *	For n = 0, j0(x) is called,
 *	For n = 1, j1(x) is called.
 *	For n < x, forward recursion is used starting
 *	from values of j0(x) and j1(x).
 *	For n > x, a continued fraction approximation to
 *	j(n, x)/j(n - 1, x) is evaluated and then backward
 *	recursion is used starting from a supposed value
 *	for j(n, x).  The resulting value of j(0, x) is
 *	compared with the actual value to correct the
 *	supposed value of j(n, x).
 *
 *	yn(n, x) is similar in all respects, except
 *	that y0 and y1 are called, and that forward recursion
 *	is used for values of n > 1.
 *	Returns EDOM error and value -HUGE if argument <= 0.
 */

#include <math.h>
#include <values.h>
#include <errno.h>

static double jn_error();

double
jn(n, x)
register int n;
register double x;
{
	double a, b, temp, t;
	int i;

	if (_ABS(x) > X_TLOSS)
		return (jn_error(n, x, 1));
	if (n == 0)
		return (j0(x));
	if (x == 0)
		return (x);
	if (n < 0) {
		n = -n;
		x = -x;
	}
	if (n == 1)
		return (j1(x));
	if (n <= x) {
		a = j0(x);
		b = j1(x);
		for (i = 1; i < n; i++) {
			temp = b;
			b = (i + i)/x * b - a;
			a = temp;
		}
		return (b);
	}
	temp = x * x;
	for (t = 0, i = n + 16; i > n; i--)
		t = temp/(i + i - t);
	a = t = x/(n + n - t);
	b = 1;
	for (i = n - 1; i > 0; i--) {
		temp = b;
		b = (i + i)/x * b - a;
		a = temp;
	}
	return (t * j0(x)/b);
}

double
yn(n, x)
register int n;
register double x;
{
	double a, b, temp;
	int i, neg;

	if (x <= 0) {
		struct exception exc;

		exc.type = DOMAIN;
		exc.name = "yn";
		exc.arg1 = n;
		exc.arg2 = x;
		exc.retval = -HUGE;
		if (!matherr(&exc)) {
			(void) write(2, "yn: DOMAIN error\n", 17);
			errno = EDOM;
		}
		return (exc.retval);
	}
	if (x > X_TLOSS)
		return (jn_error(n, x, 0));
	if (n == 0)
		return (y0(x));
	neg = 0;
	if (n < 0) {
		n = -n;
		neg = n % 2;
	}
	b = y1(x);
	if (n > 1) {
		a = y0(x);
		for (i = 1; i < n; i++) {
			temp = b;
			b = (i + i)/x * b - a;
			a = temp;
		}
	}
	return (neg ? -b : b);
}

static double
jn_error(n, x, jnflag)
int n;
double x;
int jnflag;
{
	struct exception exc;

	exc.type = TLOSS;
	exc.name = jnflag ? "jn" : "yn";
	exc.arg1 = n;
	exc.arg2 = x;
	exc.retval = 0.0;
	if (!matherr(&exc)) {
		(void) write(2, exc.name, 2);
		(void) write(2, ": TLOSS error\n", 14);
		errno = ERANGE;
	}
	return (exc.retval);
}
