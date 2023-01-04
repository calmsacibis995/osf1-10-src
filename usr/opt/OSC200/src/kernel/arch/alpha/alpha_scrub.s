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
	.rdata
	.asciiz "@(#)$RCSfile: alpha_scrub.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/01/08 15:08:22 $"
	.text

#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>

/*
 * alpha_scrub_long
 *
 * Memory scrub to fix up after Memory Corrected EDC Errors.
 *
 * Scrub a memory location using a load locked/store conditional sequence.
 * This guarantees that the location is read from memory and written back,
 * or for snooping protocols, that the cached location is dirty so that it
 * is read instead of the bad memory location.
 *
 * Parameters:
 *
 *	a0 : VA (quadword aligned) of memory location to scrub.
 *	     Use PHYS_TO_KSEG to get it.
 *	a1 : Retry limit.  Number of times to retry the ldql/stqc before
 *	     giving up.  (Tries retries + 1 time before giving up).
 *
 * Returns:
 *	Nonzero if success, zero if failed for specified number of retries.
 */

	.align 4
LEAF(alpha_scrub_long)

	/* Try to load/store the location to be scrubbed */

1:	ldq_l	v0, 0(a0)		 /* load quad from memory */
	stq_c	v0, 0(a0)		 /* force it out or make it dirty */
	beq	v0, 2f			 /* if failed, branch to retry */
	RET				 /* return v0 != 0 */

	/* Store failed; retry until retries exhausted */

2:	subq	a1, 1, a1		 /* decrement retry counter */
	bge	a1, 1b			 /* loop while positive */
	RET				 /* return v0 == 0 */
END(alpha_scrub_long)
