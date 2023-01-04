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
static char	*sccsid = "@(#)$RCSfile: frexp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:59:51 $";
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
/*
 * $ Header: frexp.c,v 10.1 87/11/04 09:58:21 mbj Exp $

 *
 * the function call
 *		x = frexp(arg,&exp);
 * returns a double precision floating point quantity x and a corresponding
 * binary exponent exp such that
 *		0.5 <= x < 1.0
 *		arg = x*(2^exp)
 *
 * Implementation notes:
 *	1.  This is a non-portable implementation which works for IEEE floating
 *		point only.
 *	2.  This implementation fixes a bug in the previous implementation.
 *		Previously, the return value met the condition
 *			0.5 <= x <= 1.0
 *		rather than
 *			0.5 <= x < 1.0
 */

#define	BIAS	(1023)		/* exponent bias */
#define	NEWEXP	(-1)		/* new exponent */

#include <math.h>
#include <nan.h>
#include <errno.h>

double
frexp(double x, int *i)
{
	register union __NaN *px;

	px = (union __NaN *) &x;


	if (px->ds.de != 0x7FF) {
	  if(x==0.0) {		/* FINITE, and zero! */
	    *i = 0;
	    return (0.0);
	  } else {		/* FINITE, and non-zero */
	    *i = (px->ds.de - BIAS) - NEWEXP;
	    px->ds.de = NEWEXP + BIAS;
	    return (x);
	  }
	} else {		/* A NaN or INF */
	  errno = EDOM;
	  return(x);
	}
}
