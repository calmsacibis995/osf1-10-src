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
 * @(#)$RCSfile: size_cache.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:52:16 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




/*
 * size_cache()
 * return size of current data cache
 */
LEAF(size_cache)
	.set	noreorder
	mfc0	t0,C0_SR		# save current sr
	nop				# LDSLOT
	or	v0,t0,SR_ISC		# isolate cache
	nop				# make sure no stores in pipe
	mtc0	v0,C0_SR
	nop				# make sure isolated
	nop
	nop
	/*
	 * Clear cache size boundries to known state.
	 */
	li	v0,MINCACHE
1:
	sw	zero,K0BASE(v0)
	sll	v0,1
	ble	v0,+MAXCACHE,1b
	nop				# BDSLOT

	li	v0,-1
	sw	v0,K0BASE(zero)		# store marker in cache
	li	v0,MINCACHE		# MIN cache size

2:	lw	v1,K0BASE(v0)		# Look for marker
	nop				# LDSLOT
	bne	v1,zero,3f		# found marker
	nop				# BDSLOT

	sll	v0,1			# cache size * 2
	ble	v0,+MAXCACHE,2b		# keep looking
	nop
	move	v0,zero			# must be no cache

3:	mtc0	t0,C0_SR		# restore sr
	nop				# make sure unisolated
	nop
	nop
	nop
	.set	reorder
	j	ra
	END(size_cache)

