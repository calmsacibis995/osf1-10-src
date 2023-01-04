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
static char	*sccsid = "@(#)$RCSfile: tanh.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:04:11 $";
#endif 
/*
 */
/*
 *	tanh returns the hyperbolic tangent of its double-precision argument.
 *	It calls exp for absolute values of the argument > ~0.55.
 *	There are no error returns.
 *	Algorithm and coefficients from Cody and Waite (1980).
 */

#include <math.h>
#include <values.h>
#define X_BIG	(0.5 * (LN_MAXDOUBLE + M_LN2))
#define LN_3_2	0.54930614433405484570

double
tanh(x)
register double x;
{
	register int neg = 0;

	if (x < 0) {
		x = -x;
		neg++;
	}
	if (x > X_BIG)
		x = 1.0;
	else if (x > LN_3_2) {
		x = 0.5 - 1.0/(exp(x + x) + 1.0); /* two steps recommended */
		x += x; /* for wobbling-precision machines (like IBM) */
	} else if (x > X_EPS) { /* skip for efficiency and to prevent underflow */
		static double p[] = {
			-0.96437492777225469787e0,
			-0.99225929672236083313e2,
			-0.16134119023996228053e4,
		}, q[] = {
			 1.0,
			 0.11274474380534949335e3,
			 0.22337720718962312926e4,
			 0.48402357071988688686e4,
		};
		register double y = x * x;

		x += x * y * _POLY2(y, p)/_POLY3(y, q);
	}
	return (neg ? -x : x);
}
