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
 * @(#)$RCSfile: config_cache.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:01:38 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>

#if	MIPSEB		/* Unaligned acceses */
#    define LWS lwl
#    define LWB lwr
#    define SWS swl
#    define SWB swr
#else	/* MIPSEL */
#    define LWS lwr
#    define LWB lwl
#    define SWS swr
#    define SWB swl
#endif



/*
 * Config_cache() -- determine sizes of i and d caches
 * Sizes stored in globals dcache_size and icache_size
 */
CONFIGFRM=	(4*4)+4+4		# 4 arg saves, ra, and a saved register
NESTED(config_cache, CONFIGFRM, zero)
	subu	sp,CONFIGFRM
	sw	ra,CONFIGFRM-4(sp)
	sw	s0,CONFIGFRM-8(sp)	# save s0 on stack
	.set	noreorder
	mfc0	s0,C0_SR		# save SR
	mtc0	zero,C0_SR		# disable interrupts
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

1:	jal	size_cache
	nop
	sw	v0,dcache_size
	nop				# make sure sw out of pipe
	nop
	nop
	nop
	li	v0,SR_SWC		# swap caches
	mtc0	v0,C0_SR
	nop				# insure caches stable
	nop
	nop
	nop
	jal	size_cache
	nop
	nop				# make sure sw out of pipe
	nop
	nop
	nop
	mtc0	zero,C0_SR		# swap back caches
	nop
	nop
	nop
	nop
	la	t0,1f
	j	t0			# back to cached mode
	nop

1:	mtc0	s0,C0_SR		# restore SR
	nop
	sw	v0,icache_size
	lw	s0,CONFIGFRM-8(sp)	# restore old s0
	lw	ra,CONFIGFRM-4(sp)
	addu	sp,CONFIGFRM
	j	ra
	nop
	.set	reorder
	END(config_cache)

