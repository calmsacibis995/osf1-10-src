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
 * @(#)$RCSfile: clr_bev.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:38:48 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 * clear_bev -- Change the control of TLB and general exception vectors to be
 * handled by the kernel and not the console.  This needs to be done after
 * the exception handling bcopy of the vector code in the processor specific.
 */
LEAF(clear_bev)
	.set noreorder
	mfc0	v0,C0_SR		# get contents of SR
	li	t0,(~SR_BEV)		# set up not of BEV bit
	nop
	and	v0,t0			# clear the BEV bit
	nop
	mtc0	v0,C0_SR
	j	ra
	nop
	.set reorder
	END(clear_bev)
