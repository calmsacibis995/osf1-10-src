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
 *	@(#)$RCSfile: atomic_op.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:22:53 $
 */ 
/*
 */
/*
 * atomic_op.s: system call interface for atomic_op(2)
 */

/************************************************************************
 *
 *	Modification History: atomic_op.s
 *
 * 30-May-91 mbs (Michael Schmitz)
 *      Created the file.
 *
 ************************************************************************/

/*
 * atomic_op(op, addr)
 *	int op;		 ATOMIC_SET or ATOMIC_CLEAR
 *	int *addr;
 */

#include <mips/asm.h>
#include <mips/regdef.h>
#include <sys/syscall.h>

SYSCALL(atomic_op)
	RET
.end atomic_op

#ifdef notdef

/*
 * These are notes on extending atomic_op(2) to processors having the
 * MIPS II instruction set.  MIPS II processors have new instructions
 * "load linked" and "store conditional" which can be used to implement
 * atomic_op.
 *
 * There are two possibilities for future designs which will support
 * both MIPS I and MIPS II processors:
 *
 * (A) Continue to export to the user only the atomic_op() interface.
 *     The library code implementing the interface will determine if
 *     the the processor is MIPS I or MIPS II.  In the former case, a
 *     system call will be performed.  In the latter case, the
 *     user level algorithm given below will be excuted.  
 *
 * (B) Export three interfaces to the user:
 *	 get_instruction_set_type()	/* maybe fold this into sysinfo() */
 *	 atomic_op()			/* always does system call */
 *	 atomic_op_in_library()		/* uses load linked and store cond */
 *
 * The advantage of (A) is simplicity on the part of user.  The 
 * disadvantage is that error handling is different.  For example,
 * if the user supplies a misaligned lock address, and a system call is
 * made, the kernel will detect the misaligned address, causing the
 * system call to fail (return -1) with an appropriate value in errno.
 * On the other hand library code (which does not normally check for
 * misaligned user arguments) will simply access the lock which will
 * cause the kernel to generate a signal.  Note that a similar problem
 * exists for segmentation violations.
 *
 * (B) The advantage of (B) is that atomic_op() will behave exactly
 * as it did before, including error conditions.  Users can still write
 * instruction set independent code as follows:
 *
 *  if (get_instruction_set_type == MIPS_I)
 *	atomic_op();			/* a system call */
 *  else
 *	atomic_op_in_library();		/* not a system call */
 * 
 */


/* 
 * This is the implementation for MIPS II machines.  It is untested.
 */

#include <sys/lock.h>
#include <mips/asm.h>
#include <mips/regdef.h>

/* Can't include <errno.h> - has C declarations */
#define EINVAL		22
#define EBUSY		16

LEAF(atomic_op)
	beq	a0, ATOMIC_SET, atomic_set
	beq	a0, ATOMIC_CLEAR, atomic_clear
#ifdef _THREAD_SAFE
	li	a0, EINVAL
	jal	seterrno
#else
	li	v0, EINVAL
	sw	v0, errno
#endif
	li	v0, -1
	j	ra
atomic_set:
	ll	v0, 0(a1)
	li	v1, 1 << ATOMIC_LOCKBIT
	and	t1, v0, v1
	bne	t1, zero, busy
	or	v0, v0, v1
	sc	v0, 0(a1)
	beq	v0, zero, atomic_set
	move	v0, zero
	j	ra
busy:
#ifdef _THREAD_SAFE
	li	a0, EBUSY
	jal	seterrno
#else
	li	v0, EBUSY
	sw	v0, errno
#endif
	li	v0, -1
	j	ra
atomic_clear:
	ll	v0, 0(a1)
	li	v1, ~(1 << ATOMIC_LOCKBIT)
	and	v0, v0, v1
	sc	v0, 0(a1)
	beq	v0, zero, atomic_clear
	move	v0, zero
	j 	ra
	END(atomic_op)
		
#endif
