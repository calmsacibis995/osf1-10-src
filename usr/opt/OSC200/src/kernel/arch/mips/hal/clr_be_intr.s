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
 * @(#)$RCSfile: clr_be_intr.s,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 10:24:29 $
 */


#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>
#include  <hal/cpuconf.h>


#define KN02ERR_ADDR	0xbfd80000	/* KSEG1 addr of kn02 Error register */
#define KN02CHKSYN_ADDR	0xbfd00000	/* KSEG1 addr of kn02 check/syn reg */

#define KN03ERR_ADDR	0xbfa40000	/* KSEG1 addr of kn03 Error register */
#define KN03CHKSYN_ADDR	0xbfa80000	/* KSEG1 addr of kn03 check/syn reg */


/*
 * Function to get a soft copy of the "erradr" reg, then clear 
 * the "erradr" reg to dismiss the pending interrupt. Since the 
 * exception code saved registers before we get here we know 
 * that "a2" and "k0" are safe to use as temp registers.
 */
LEAF(clr_be_intr)
	lw      a2, cpu                 # get system type
	beq     a2, DS_5000_300,1f	# see if 5000_300, 3max+
	bne     a2, DS_5000, 2f		# see if 5000
	la	a2, KN02ERR_ADDR	# get addr of hardware erradr reg
	lw      k0, (a2)		# get contents of hardware erradr reg
	sw      k0, kn02erradr		# and save a software copy
	sw      zero, (a2)		# clear hardware erradr to clear intr
	la	a2, KN02CHKSYN_ADDR	# get addr of hardware chksyn reg
	lw      k0, (a2)		# get contents of hardware chksyn reg
	sw      k0, kn02chksyn		# and save a software copy
					# latency absorbed in 3 prior instructs
	jal	wbflush			# wait for erradr write
	b	2f

1:	la	a2, KN03ERR_ADDR	# get addr of hardware erradr reg
	lw      k0, (a2)		# get contents of hardware erradr reg
	sw      k0, kn03erradr		# and save a software copy
	sw      zero, (a2)		# clear hardware erradr to clear intr
	la	a2, KN03CHKSYN_ADDR	# get addr of hardware chksyn reg
	lw      k0, (a2)		# get contents of hardware chksyn reg
	sw      k0, kn03chksyn		# and save a software copy
					# latency absorbed in 3 prior instructs
	jal     wbflush			# wait for erradr write
2f:
	j	ra
	END(clr_be_intr)
