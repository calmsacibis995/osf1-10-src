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
/*	
 *	@(#)$RCSfile: alloca.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:58:00 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* alloca:	Allocate n bytes of memory on stack, so that it will
 *		be freed automatically by returning from the procedure.
 *		This is more awkward on an NS32000 than a Vax, due to where
 *		the enter and exit instructions put the saved registers.
 *		NS32000 alloca has to copy what may possibly be saved regs
 *		from the top few words of the current stack to the top of
 *		the newly allocated area, so that the caller's exit will
 *		restore regs properly.  This will happen each time the
 *		caller calls alloca, so overhead-per-call will eventually
 *		amount to 32 bytes (enough to hold all 8 registers, which
 *		is what we must assume enter may have saved).
 * Calling sequence:	ptr = alloca (nbytes);
 * Returns:	pointer to allocated memory.
*/
/* Mark D. Guzzi at Encore Computer, 8-Sep-89
 *	The original routine was no-reentrant because it saved the return
 *	pc in a static area in bss.  The routine has been recoded to save
 *	the pc in a register and then on the stack.
*/

#include "SYS.h"

	.file	"alloca.s"
	.text
	.align	1
ENTRY(alloca)
#ifdef PROF
	movd	TOS,r0			# old fp
	lprd	fp,r0			# restore old fp
#else
	addr	0(fp),r0		# old fp
#endif
	movd	TOS,r2  		# Pop ret addr to r2
	addr	4(sp),r1		# r1 == old sp, before call
	subd	r1,r0			# No. regs saved by enter of caller
					#  is min ( (fp-sp)/4, 8)
	cmpd	r0,$32			# See if is > than storage for all
					#   8 registers (from enter).
	bls	nregs_ok		# If r2<=32 bytes, we are ok
	movd	$32,r0			# Else set it to 32 (max)
nregs_ok:
	addd	r0,0(sp)		# top of stack has # of bytes
	adjspd	TOS			# sp -= (nbytes + regsave area)
	movd	r2, TOS			# put return pc on top of stack
	addr	4(sp),r2		# Prepare for MOVSB
	movsb				# Copy saved registers (or random
					#   bytes, but we have no way of
					#   knowing what callers enter saved)
	movd	r2,r0			# Return bottom of nbyte-sized area
					#   to caller (just above resaved
					#   registers)
	movd	0(sp),r1		# And set up for return, leaving
	jump	0(r1)			#   value for caller to pop off

