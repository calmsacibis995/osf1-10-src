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

#include <alpha/asm.h>
#include <alpha/regdef.h>
#include <alpha/fpu.h>

/*
 * extern unsigned long _get_fpcr();
 *
 * return current contents of fpcr register
 */
	.align	4
#ifndef _NAME_SPACE_WEAK_STRONG
NESTED(_get_fpcr, 16, ra)
#else
OLDNESTED(_get_fpcr, 16, ra)
#endif
	lda	sp, -16(sp)
	.prologue	0
	excb			# make sure all preceding traps occur
	mf_fpcr	$f0
	excb			# make sure fpcr is read before any traps occur
	stt	$f0, 0(sp)
	ldq	v0, 0(sp)
	lda	sp, 16(sp)
	RET
END(_get_fpcr)

/*
 * extern unsigned long _set_fpcr(unsigned long fpcr);
 *
 * set fpcr register
 */
	.align	4
#ifndef _NAME_SPACE_WEAK_STRONG
NESTED(_set_fpcr, 16, ra)
#else
OLDNESTED(_set_fpcr, 16, ra)
#endif
	lda	sp, -16(sp)
	.prologue	0
	excb			# make sure all preceding traps occur
	stq	a0, 0(sp)	# stack new fpcr
	mf_fpcr	$f0		# get current fpcr
	stt	$f0, 8(sp)	# stack current fpcr
	ldt	$f1, 0(sp)	# get new fpcr
	mt_fpcr	$f1		# and set it
	excb			# make sure fpcr is written before any traps occur
	ldq	v0, 8(sp)	# return previous fpcr
	lda	sp, 16(sp)
	RET
END(_set_fpcr)

/*
 * extern int fegetround(void);
 *
 * NCEG proposed extension to read current rounding mode
 */
	.align	4
NESTED(fegetround, 16, ra)
	lda	sp, -16(sp)
	.prologue	0
	excb			# make sure all preceding traps occur
	mf_fpcr	$f0		# get current fpcr
	excb			# make sure fpcr is read before any traps occur
	stt	$f0, 0(sp)	# stack current fpcr
	ldq	v0, 0(sp)	# get fpcr into an integer reg
	srl	v0, 58, v0	# get round mode into bits 0:1
	and	v0, 3, v0	# isolate round mode
	lda	sp, 16(sp)
	RET
END(fegetround)

/*
 * extern int fesetround(int round);
 *
 * NCEG proposed extension to set the current rounding mode.
 * Return non-zero if the passed value is a valid rounding mode; zero otherwise
 */
	.align	4
NESTED(fesetround, 16, ra)
	lda	sp, -16(sp)
	.prologue	0
	cmpule	a0, 3, v0	# determine if rounding mode is valid
	sll	a0, 58, t0	# shift into correct position for fpcr
	beq	v0, 10f		# quit now if invalid rounding mode
	excb			# make sure all preceding traps occur
	mf_fpcr	$f0		# get current fpcr
	ldiq	t1, 3		# construct mask for dyn_rm
	sll	t1, 58, t1
	stt	$f0, 0(sp)	# stack current fpcr
	ldq	t2, 0(sp)	# get into integer reg
	bic	t2, t1, t1	# clear out old rounding mode
	bis	t0, t1, t0	# merge in new rounding mode
	stq	t0, 0(sp)	# stack new fpcr
	ldt	$f0, 0(sp)	# move into float reg
	mt_fpcr	$f0		# set new rounding mode
	excb			# make sure fpcr is written before any traps occur
10:	lda	sp, 16(sp)
	RET
END(fesetround)
