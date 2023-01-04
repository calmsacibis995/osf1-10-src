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
	.rdata
	.asciiz "@(#)$RCSfile: usercopy.s,v $ $Revision: 1.1.2.11 $ (DEC) $Date: 1992/10/30 15:57:22 $"
	.text

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/thread.h>
#include <sys/errno.h>
#include "assym.s"

/*
 * The following routines (su/fu/i/byte/word/qword) routines are used
 * to access user space from within the kernel.  Each does the proper
 * protection check and returns -1 or the indicated value in the case
 * of the fetch versions.  The `i' variants of the routines are an
 * artifact from split I/D addressing machines such as the pdp11's
 */

/*
 * fubyte  - fetch a byte from user space with the appropriate
 *	     checks.
 *
 */
LEAF(fubyte)
XLEAF(fuibyte)
	blt	a0,_uerror		# if the address is neg, it's kernel

	ldq	t0,current_pcb		# get the address of our pcb
	ldiq	t1,NF_FSUMEM		# setup nofault
	stq	t1,PCB_NOFAULT(t0)

	ldq_u	v0,(a0)			# get the quad word containing byte
	extbl	v0,a0,v0		# get the correct byte

	stq	zero,PCB_NOFAULT(t0)	# reset nofault
	ret	zero,(ra)
	END(fubyte)

/*
 * fuword  - fetch a word from user space with the appropriate
 *	     checks.
 */
LEAF(fuword)
XLEAF(fuiword)
	blt	a0,_uerror		# if the address is neg, it's kernel

	ldq	t0,current_pcb		# get the address of our pcb
	ldiq	t1,NF_FSUMEM		# setup nofault
	stq	t1,PCB_NOFAULT(t0)

	ldl	v0,(a0)

	stq	zero,PCB_NOFAULT(t0)	# reset nofault
	ret	zero,(ra)
	END(fuword)

/*
 * fuqword  - fetch a quad word from user space with the appropriate
 *	     checks.
 */
LEAF(fuqword)
XLEAF(fuiqword)
	blt	a0,_uerror		# if the address is neg, it's kernel

	ldq	t0,current_pcb		# get the address of our pcb
	ldiq	t1,NF_FSUMEM		# setup nofault
	stq	t1,PCB_NOFAULT(t0)

	ldq	v0,(a0)

	stq	zero,PCB_NOFAULT(t0)	# reset nofault
	ret	zero,(ra)
	END(fuqword)

/*
 * subyte  - store a byte into user space with the appropriate
 *	     checks.
 */
LEAF(subyte)
XLEAF(suibyte)
	blt	a0,_uerror		# if the address is neg, it's kernel

	ldq	t0,current_pcb		# get the address of our pcb
	ldiq	t1,NF_FSUMEM		# setup nofault
	stq	t1,PCB_NOFAULT(t0)

	ldq_u	v0,(a0)			# get the destination quad word
	insbl	a1,a0,a1		# get the byte in the right place
	mskbl	v0,a0,v0		# clear out destination byte
	or	v0,a1,v0		# set the byte into the word
	stq_u	v0,(a0)			# put it back
	bis	zero,zero,v0		# set up return value

	stq	zero,PCB_NOFAULT(t0)	# reset nofault
	ret	zero,(ra)
	END(subyte)

/*
 * suword  - store a word into user space with the appropriate
 *	     checks.
 */
LEAF(suword)
XLEAF(suiword)
	blt	a0,_uerror		# if the address is neg, it's kernel

	ldq	t0,current_pcb		# get the address of our pcb
	ldiq	t1,NF_FSUMEM		# setup nofault
	stq	t1,PCB_NOFAULT(t0)

	stl	a1,(a0)
	bis	zero,zero,v0		# set up return value

	stq	zero,PCB_NOFAULT(t0)	# reset nofault
	ret	zero,(ra)
	END(suword)

/*
 * suqword  - store a quad word into user space with the appropriate
 *	     checks.
 */
LEAF(suqword)
XLEAF(suiqword)
	blt	a0,_uerror		# if the address is neg, it's kernel

	ldq	t0,current_pcb		# get the address of our pcb
	ldiq	t1,NF_FSUMEM		# setup nofault
	stq	t1,PCB_NOFAULT(t0)

	stq	a1,(a0)
	bis	zero,zero,v0		# set up return value

	stq	zero,PCB_NOFAULT(t0)	# reset nofault
	ret	zero,(ra)
	END(suword)

/*
 * address error: return -1
 */
LEAF(_uerror)
	ornot	zero,zero,v0	# -1
	ret	zero,(ra)
	END(_uerror)

/*
 * Copy a null terminated string from the user address space into
 * the kernel address space. This routine has to be smart about the
 * way it probes, the maxlength may result in a byte that isn't mapped.
 *
 * copyinstr(user_src, kernel_dest, maxlength, &lencopied)
 *      returns:
 *              0               - success
 *              EFAULT          - user_src not accessable
 *              ENAMETOOLONG    - string exceeded maxlength
 */



LEAF(copyinstr)
	blt	a0, cstrerror		/* if(user_src < 0) */

	ldq	t4, current_pcb		/* current_pcb->pcb_nofault = NF_COPYSTR*/
	ldiq	t5,NF_COPYSTR
	stq	t5,PCB_NOFAULT(t4)

	bis     zero, zero, t0		/* len = 0 */
        br	zero, 1f		/* while(maxlenght > 0) */
2:
        lda     t4, 1(a0)		/* c = *kerne_dest++ = *user_src++ */
        ldq_u   t3, 0(a0)
        extqh   t3, t4, t3
        sra     t3, 0x38, t3
        ldq_u   t4, 0(a1)
        insbl   t3, a1, t6
        mskbl   t4, a1, t4
        bis     t4, t6, t4
        stq_u   t4, 0(a1)
        sll     t3, 0x18, t1
        addl    t1, 0x0, t1
        sra     t1, 0x18, t1
        addq    a0, 0x1, a0
        addq    a1, 0x1, a1
        addl    t0, 0x1, t0		/* len++ */
        bne     t1, 1f			/* if(c == '/0') */
        br      zero, 10f
1:
        cmpult  zero, a2, t2
        subl    a2, 0x1, a2
        bne     t2, 2b

10:
	ldq	t4, current_pcb		/* current_pcb->pcb_nofault = 0 */
	stq	zero,PCB_NOFAULT(t4)
   	beq	a3, 5f			/* if(lencopied != 0) */
   	stl	t0, 0(a3)		/* 	*lencopied = len */
5:
   	beq	t1, 6f			/* if(c != '\0') */
   	bis	zero, ENAMETOOLONG, v0	/*	return ENAMETOOLONG */
   	br	zero, 7f
6:
   	bis	zero, zero, v0		/* else return 0 */
7:
   	ret	zero, (ra), 1
	END(copyinstr)


/*
 * Copy a null terminated string from the kernel address space into
 * user address space. Like copyinstr this routine probes as it goes
 * along to avoid areas that aren't mapped yet.
 *
 * copyoutstr(kernel_src, user_dest, maxlength, &lencopied)
 *	returns:
 *		0		- success
 *		EFAULT		- user_dest not accessable
 *		ENAMETOOLONG	- string exceeded maxlength
 */

LEAF(copyoutstr)
   	blt	a1, cstrerror		/* if(user_dest < 0) */

	ldq	t4,current_pcb		/* current_pcb->pcb_nofault = NF_COPYSTR*/
	ldiq	t5,NF_COPYSTR
	stq	t5,PCB_NOFAULT(t4)

        bis     zero, zero, t0		/* len = 0 */
	br	zero, 1f		/* while(maxlength-- > 0) */
2:
        lda     t4, 1(a0)		/* c = *user_dest++ = *kernel_src++ */
        ldq_u   t3, 0(a0)
        extqh   t3, t4, t3
        sra     t3, 0x38, t3
        ldq_u   t4, 0(a1)
        insbl   t3, a1, t6
        mskbl   t4, a1, t4
        bis     t4, t6, t4
        stq_u   t4, 0(a1)
        sll     t3, 0x18, t1
        addl    t1, 0x0, t1
        sra     t1, 0x18, t1
        addq    a0, 0x1, a0
        addq    a1, 0x1, a1
        addl    t0, 0x1, t0		/* len++ */
        bne     t1, 1f			/* if(c == '\0') */
        br      zero, 5f
1:
        cmpult  zero, a2, t2
        subl    a2, 0x1, a2
        bne     t2, 2b

5:
	ldq	t4,current_pcb		/* current_pcb->pcb_nofault = 0 */
	stq	zero,PCB_NOFAULT(t4)
   	beq	a3, 8f			/* if(lencopied != 0) */
   	stl	t0, 0(a3)		/* 	*lencopied = len */
8:
   	beq	t1, 7f			/* if(c != '\0') */
   	bis	zero, ENAMETOOLONG, v0	/*	return ENAMETOOLONG */
   	br	zero, 10f
7:
   	bis	zero, zero, v0		/* else return 0 */
10:
   	ret	zero, (ra), 1
	END(copyoutstr)


LEAF(cstrerror)
	ldiq	v0,EFAULT
	RET
	END(cstrerror)
