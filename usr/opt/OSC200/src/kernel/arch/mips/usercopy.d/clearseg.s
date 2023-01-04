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
 * @(#)$RCSfile: clearseg.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:58:01 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




/*
 * clearseg(dst_ppn)
 *
 *	Performance
 *	Config	Cycles/	Speed vs VAX
 *		4K Page	
 *	08V11	6,144	1.09X
 *	08M44	1,229	5.46X	(could be made faster by unroll to 64)
 *                              (done April '87 per djl)
 *	since writes only occur at best 1 per two cycles(m500) and unroll
 *	shouldn't help, in fact we probably don't want many instructions
 *	so that it is easy to get into icache-- so changing back to two
 *	sw's per loop (two cycles + two cycles for loop overhead) which
 *	will keep the write buffers busy and not stall the cpu.
 */
LEAF(clearseg)
	sll	a0,PGSHIFT
	addu	a0,K0BASE		# reference via k0seg
	addu	t0,a0,NBPG-8		# dst on last pass of loop
1:	sw	zero,0(a0)
	sw	zero,4(a0)
	.set	noreorder
	bne	a0,t0,1b
	addu	a0,8			# BDSLOT: inc dst, NOTE after test
	.set	reorder
#ifdef EXTRA_CACHETRICKS		
	subu	a0,8			# back to copied page
	srl	a0,PGSHIFT
	lw	v0,dcachemask
	and	a0,v0			#  figure appropriate cache alias
	sll	a0,1
	lhu	v0,dcachecnt(a0)
	addu	v0,1
	sh	v0,dcachecnt(a0)
#endif /* EXTRA_CACHETRICKS */
	j	ra
	END(clearseg)

