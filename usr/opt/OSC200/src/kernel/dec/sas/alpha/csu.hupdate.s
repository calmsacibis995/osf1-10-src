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
 * @(#)$RCSfile: csu.hupdate.s,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/07/30 18:30:05 $
 */


#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/pal.h>

	.text
	.globl	start
	.ent	start
	.frame sp, 8*4, ra
	.mask	M_RA, -8
	.align	3
start:
	.set	noreorder
	bis	zero,zero,zero		# align the _gp quad word
	br	t0, 1f			# addr of pad bytes preceding .quad
	.quad	_gp			# the gp value
1:	subq	sp, 16, sp		# save space for qp and ra on stack
	stq	gp, 0(sp)		# store incoming qp on stack
	stq	ra, 8(sp)		# store incoming ra on stack
	ldq	gp,0(t0)		# load gp.
	.set	reorder

	jsr	ra,zerobss 		# zerobss does not use a0 or a1
	jsr	ra,main
	ldq	gp, 0(sp)		# restore qp for return
	ldq	ra, 8(sp)		# restore ra for return
	addq	sp, 16, sp		# restore stack for return
	ret	zero, (ra)
	call_pal PAL_halt		# should never get here!!
	.end

/*
 * zero bss
 */
	.extern	_fbss 0
	.extern	end 0
LEAF(zerobss)
	.set	noreorder
	lda	t0, _fbss		# edata address
	lda	t1, end			# end address
5:	stq	zero, 0(t0)		# zap quad words in data
	addq	t0, 8			# next quad word
	subq	t1, t0, t3		# are we at "end"?
	bgt	t3, 5b			# do next quad word if not
	ret	zero, (ra)		# return to start
	.set reorder
	END(zerobss)
