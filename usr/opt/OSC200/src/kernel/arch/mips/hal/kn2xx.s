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
 * @(#)$RCSfile: kn2xx.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:19:07 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>
#include  "assym.s"

/*
 * This is a routine that reads a 32-bit word from an address
 * that may not exist, or may cause a bus timeout.
 *
 * NOTE:  This routine is used by the interrupt handler code for kn220 &
 *	  kn210 to read the interrupt vector from the vector registers.
 *	  If there is a passive release from the interrupting devices,
 *	  the reading of the vector register will cause a bus timeout
 *	  (DBE exception).  Don't generalise this routine to do more
 *	  because speed is important here.
 *
 * synopsis:
 *	int read_nofault(nofault_src)
 *	int *nofault_src;
 *
 *	nofault_src is the address to read from.  If the read fails,
 *	a zero will be returned.  If the read is successful, the value
 *	from nofault_src will be returned.
 */
LEAF(read_nofault)
	.set noreorder
	mfc0	t0,C0_SR		# save status register
        li      a1,PCB_WIRED_ADDRESS
	lw	t1,PCB_NOFAULT(a1)
	mtc0	zero,C0_SR
	.set reorder
	li	v1,NF_INTR		#
 	sw	v1,PCB_NOFAULT(a1)
	lw	v0,0(a0)		# v0 = *nofault_src;
	/* if we get here the read was successful */
 	sw	t1,PCB_NOFAULT(a1)
	.set noreorder
	mtc0	t0,C0_SR		# restore status register
	nop
	.set reorder
	j	ra
	END(read_nofault)

