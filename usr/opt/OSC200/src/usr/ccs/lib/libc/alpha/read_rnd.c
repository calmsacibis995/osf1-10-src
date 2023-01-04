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
static char *rcsid = "@(#)$RCSfile: read_rnd.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/06/23 21:23:00 $";
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
#pragma weak read_rnd = __read_rnd
#endif

#include <float.h>
#include <fp.h>
#include <machine/fpu.h>

unsigned int read_rnd(void)
{
	unsigned int ieee_rounding_mode;
	unsigned int alpha_rounding_mode;

	alpha_rounding_mode = fegetround();
	/* Unfortunately, float.h doesn't match fpu.h so we map them */
	switch (alpha_rounding_mode) {
	    case FE_TOWARDZERO:
		ieee_rounding_mode = FP_RND_RZ;
		break;
	    case FE_TONEAREST:
		ieee_rounding_mode = FP_RND_RN;
		break;
	    case FE_UPWARD:
		ieee_rounding_mode = FP_RND_RP;
		break;
	    case FE_DOWNWARD:
		ieee_rounding_mode = FP_RND_RM;
		break;
	    default:
		/* Invalid rounding mode. There is no way to report an error
		 * so use "plus infinity" as directed by the Alpha SRM
		 */
		ieee_rounding_mode = FP_RND_RP;
	} /* switch */
	return (ieee_rounding_mode);
}

