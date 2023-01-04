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
static char	*sccsid = "@(#)$RCSfile: modf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:03:50 $";
#endif 
/*
 */


#if !defined(lint) && !defined(_NOIDENT)

#endif

/* modf.c
 *
 * SYNOPSIS
 *
 *    #include <math.h>
 *    double modf(double x, double *y)
 *
 * DESCRIPTION
 *
 *	modf decomposes floating point numbers into the integral and fractional
 *	parts.
 *
 * RETURNS
 *
 *	returns the signed fractional part of the input and updates the integral
 *	part pointed to by the second argument.
 *
 * On IEEE-754 conforming machines with "isnan()" primitive,
 *
 *	[NaN] 	if the value of x is NaN
 *
 */

#include "hdr.h"

double
modf( double value, double * iptr )
{
  double frac, ival,aval;

  if (isnan(value)) return(value);

  if (SIGN(value)) {
    /*
     * It's a negative quantity
     */
    ival = ceil(value);
  } else {
    ival = floor(value);
  }

  *iptr = ival;

  return( value - ival );
}
