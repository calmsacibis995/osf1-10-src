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
 * @(#)$RCSfile: copyout.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:58:57 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




/*
 * copyout(kernel_src, user_dst, bcount)
 */
COPYIOFRM=      (4*4)+4 
NESTED(copyout, COPYIOFRM, zero)
	subu	sp,COPYIOFRM
	sw	ra,COPYIOFRM-4(sp)

        bge     a1, 0, 4f               # cerror chnged to sub call
        jal     cerror
        b       1f

4:
        addu    v0,a1,a2
	addiu	v0,v0,-1		# user addr rng checking
        bge     v0, 0, 3f               # cerror chnged to sub call
        jal     cerror
        b       1f
3:
	li	a3,PCB_WIRED_ADDRESS
#ifdef ASSERTIONS
	lw	v0,PCB_NOFAULT(a3)
	beq	v0,zero,2f
	PANIC("recursive nofault")
2:
#endif
	.set	noreorder
	li	v0,NF_COPYIO
	sw	v0,PCB_NOFAULT(a3)
	jal	bcopy
	nop
	.set	reorder
	li	a3,PCB_WIRED_ADDRESS	# got any spares?
	sw	zero,PCB_NOFAULT(a3)
	move	v0,zero
1:
	lw	ra,COPYIOFRM-4(sp)
	addu	sp,COPYIOFRM
	j	ra
	END(copyout)

