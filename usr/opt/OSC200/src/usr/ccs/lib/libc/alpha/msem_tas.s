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

/*
 * extern int _msem_tas(int *loc);
 *
 * Perform an atomic test-and-set of the longword pointed to by "loc".
 * This routine is written with the following considerations as
 * suggested by the Alpha SRM:
 *
 *	1. Only register-to-register operate instructions are used to
 *	   construct the value to be stored.
 *
 *	2. Branches "fall-through" in the success case.
 *
 *	3. Both conditional branches are forward branches so they
 *	   are properly predicted not to be taken to match the common
 *	   case of no contention for the lock.
 */
	.align	4
LEAF(_msem_tas)
10:	ldl_l	v0, 0(a0)		# get the location to be tested
	bis	zero, 1, t0		# get the "set" value
	bne	v0, 20f			# don't write if already set
	stl_c	t0, 0(a0)		# "set" the location
	beq	t0, 30f			# skip if the write failed
20:	RET

30:	br	zero, 10b		# try again
END(_msem_tas)
