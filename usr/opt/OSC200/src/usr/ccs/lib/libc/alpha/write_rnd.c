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
static char	*sccsid = "@(#)$RCSfile: write_rnd.c,v $ $Revision: 1.2.4.4 $ (DEC) $Date: 1993/06/07 19:47:07 $";
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

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak write_rnd = __write_rnd
#endif

#include <float.h>
#include <fp.h>
#include <machine/fpu.h>

unsigned int write_rnd(rnd)
unsigned int rnd;
{
	unsigned int	original_rounding_mode;
	unsigned int	alpha_rounding_mode;

	original_rounding_mode = fegetround();

	/* map original alpha rounding mode to ieee rounding mode */
	switch (original_rounding_mode) {
	case FE_TOWARDZERO:
		original_rounding_mode = FP_RND_RZ;
		break;
	case FE_TONEAREST:
		original_rounding_mode = FP_RND_RN;
		break;
	case FE_UPWARD:
		original_rounding_mode = FP_RND_RP;
		break;
	case FE_DOWNWARD:
		original_rounding_mode = FP_RND_RM;
		break;
	default:
		/* invalid rounding mode. there is no way to report an error
		 * 	so use "plus infinity" as directed by the Alpha SRM
		 */
		original_rounding_mode = FP_RND_RP;
	} /* switch */

	/* unfortunately, float.h doesn't match fpu.h so we map them */
	switch(rnd) {
	case FP_RND_RZ:
		alpha_rounding_mode = FE_TOWARDZERO;
		break;
	case FP_RND_RN:
		alpha_rounding_mode = FE_TONEAREST;
		break;
	case FP_RND_RP:
		alpha_rounding_mode = FE_UPWARD;
		break;
	case FP_RND_RM:
		alpha_rounding_mode = FE_DOWNWARD;
		break;
	default:
		/* invalid rounding mode. there is no way to report an error
		 *	so leave the current rounding mode and return the
		 *	original.
		 */
		return (original_rounding_mode);
	} /* switch */

	fesetround(alpha_rounding_mode);

	return (original_rounding_mode);
}

