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
static char	*sccsid = "@(#)$RCSfile: isnan.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:03:34 $";
#endif 
/*
 */


#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <math.h>

/*
 * NAME: isnan
 *
 * FUNCTION: Determine if x is NaN (not a number)
 *
 * NOTES: If x passed in from caller as a float, will be silently coerced
 *	to a double.
 *	Note also that this requires that optimizing compilers NOT remove
 *	the *seemingly* unnecessary compare.
 *
 * RETURNS: TRUE is x is NaN.
 */

	int
#ifdef _NO_PROTO
isnan(x)
	volatile double x;
#else /* _NO_PROTO */
isnan (volatile double x)
#endif /* _NO_PROTO */
{
        return (x != x);
}
