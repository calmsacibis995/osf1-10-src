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
	.asciiz "@(#)$RCSfile: sigaction.s,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/07/30 19:24:31 $"
	.text

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

#define _NAME_SPACE_WEAK_STRONG
#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <syscall.h>
#include "setjmp.h"
#include <pdsc.h>

/*
 * struct sigaction {
 *	int (*sa_handler)();
 *	int sa_mask;
 *	int sa_onstack;
 * };
 *
 * sigaction(sig, action, oaction);
 * int sig;
 * struct sigaction *action, *oaction;
 *
 * NOTE:  Implement this by passing a hidden argument to the kernel which
 * point to the sigmanager.  sendsig then passes actual signal handler
 * to sigmanager as a parameter.
 */

LEAF(sigaction)
	ldgp	gp, 0(pv)

	lda	a3,__sysv_sigtramp	# hidden arg -- address of tramp code
	ldiq	v0,SYS_sigaction
	CHMK()
	bne	a3,err
	RET

err:
        br      gp, 1f
1:
        ldgp    gp, 0(gp)
	jmp	zero, _cerror
END(sigaction)

#define	SIG_SCTXTPTR	0

#define	SIGFRAME	(1*8)	/* only holds sctxtptr	*/

/*
 * sysv_igtramp is called by the SVR4 kernel as:
 * 	sysv_sigtramp(signal, siginfo_ptr, sigcontext_ptr, sighandler)
 *
 * Sigtramp should build a frame appropriate to the language calling
 * conventions and then call the sighandler.  When the sighandler
 * returns, sigtramp does a sigcleanup system call passing the
 * address of the sigcontext struct.
 *
 * I made this an external symbol so that it would have it's own procedure
 *	descriptor entry and I could mark it as an exception frame.
 */
OLDLEAF(__sysv_sigtramp)
	/* the following directive tells the alpha calling standard
	 *	that this is an exception frame and we look for a
	 *	sigcontext at vfp + SIGTRAMP_SIGCONTEXT_OFFSET
	 *	The flag's interpretation is OS specific.
	 */
	.eflag PDSC_FLAGS_EXCEPTION_FRAME
	/*
	 * Save process state.
	 *
	 * NOTE: sp, v0, a0, a1, a2, a3, a4, a5 are saved into sigcontext by
	 * by the kernel in sendsig, on a sigreturn the kernel copies the
	 * entire state indicated by the sigcontext into the exception
	 * frame and then returns to user mode via a special exit that
	 * restore the entire process state from the exception frame
	 * (unlike the normal syscall exit which assumes that the C
	 * calling sequence alleviates the necessity of preserving
	 * certain portions of the process state)
	 *
 	 * This routine is not called with a PV!!!!
	 */
	.set	noat
	stq	AT, JB_AT*8(a2)
	.set	at

	stq	zero, JB_ZERO*8(a2)	# just in case someone looks
	stq	t0, JB_T0*8(a2)
	stq	t1, JB_T1*8(a2)
	stq	t2, JB_T2*8(a2)
	stq	t3, JB_T3*8(a2)
	stq	t4, JB_T4*8(a2)
	stq	t5, JB_T5*8(a2)
	stq	t6, JB_T6*8(a2)
	stq	t7, JB_T7*8(a2)
	stq	t8, JB_T8*8(a2)
	stq	t9, JB_T9*8(a2)
	stq	t10, JB_T10*8(a2)
	stq	t11, JB_T11*8(a2)

	stq	pv, JB_PV*8(a2)

	stq	s0, JB_S0*8(a2)
	stq	s1, JB_S1*8(a2)
	stq	s2, JB_S2*8(a2)
	stq	s3, JB_S3*8(a2)
	stq	s4, JB_S4*8(a2)
	stq	s5, JB_S5*8(a2)
	stq	s6, JB_S6*8(a2)

	stq	gp, JB_GP*8(a2)
	stq	ra, JB_RA*8(a2)
	subq	sp, SIGFRAME, sp
	stq	a2, SIG_SCTXTPTR*8(sp)	# save address of sigcontext

	/*
	 * call a C routine which will call the signal handler.
	 * This routine will fill in a ucontext structure with
	 * the information from the sigcontext, pass the ucontext
	 * and siginfo to the handler, and copy changes from
	 * the ucontext back into the ssigcontext.
	 */
	br	t0, 1f
1:
	ldgp	gp, 0(t0)		# GP needed for jsr
	lda	pv, siginfo_sig		# set up Procedure value
	jsr	ra, siginfo_sig
	ldq	a0, SIG_SCTXTPTR*8(sp)	# sigreturn(&sigcontext)
	/*
	 * sigreturn will restore entire user state from sigcontext
	 * struct
	 */
	ldiq	v0, SYS_sigreturn
	CHMK()
	.set	at
END(__sysv_sigtramp)
