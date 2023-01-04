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

#ifdef _NAME_SPACE_WEAK_STRONG
#undef _NAME_SPACE_WEAK_STRONG
#endif
#include <alpha/regdef.h>
#include <alpha/asm.h>

/* abs - absolute value */

	.align	4
LEAF(abs)
	.prologue 0

	subl	zero, a0, v0		# assume param was negative
	addl	a0, 0, a0		# make sure param is in canonical form
	cmovgt	a0, a0, v0		# param was positive, return it
	RET
END(abs)
