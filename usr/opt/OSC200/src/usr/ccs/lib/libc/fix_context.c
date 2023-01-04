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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: fix_context.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/24 19:48:34 $";
#endif

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak fix_ucontext = __fix_ucontext 
#pragma weak fix_sigcontext = __fix_sigcontext 
#endif

#include <sys/signal.h>
#include <sys/siginfo.h>
#include <sys/ucontext.h>
#include <setjmp.h>

/*
 * This is a pointer to the previous context.
 * It is updated when a new setcontext is done.
 * It is returned in the uc_link of the context
 * returned by getcontext.  It is also set by
 * the signal trampoline code, which saves the
 * old value in its uc_link structure.
 *
 * N.B. -- this function relies on this global data. It is therefore
 * decidedly not thread-safe and cannot be while still being SVID3
 * compliant.
 */
struct ucontext *_old_ucontext = 0;

fix_ucontext(ucontext_t *ucp, struct sigcontext *scp, int copyall)
{
	ucp->uc_flags = 0L;
	ucp->uc_link = _old_ucontext;
	_old_ucontext = ucp;

	ucp->uc_sigmask = scp->sc_mask;

	/*
	 * copy over stack info 
	 */
	ucp->uc_stack.ss_sp = scp->sc_sbase;
	ucp->uc_stack.ss_flags = (int)scp->sc_onstack;
	ucp->uc_stack.ss_size = scp->sc_ssize;

	/*
	 * mcontext is described as follows in the SVR4
	 * man page (section 5):  "uc_mcontext contains
	 * the saved set of machine registers and any
	 * implementation specific context data.  Portable
	 * applications should not modify or access uc_mcontext."
	 *
	 * Given the assumption that applications should not modify
	 * this field, it is not necessary to supply it in a manner
	 * that makes it easy to modify.  Instead of copying registers
	 * into SVR4 defined mcontext and using one of the uc_filler
	 * fields as a pointer to the sigcontext, we will define the
	 * mcontext structure in OSF/1 as a sigcontext structure.  If
	 * an application really wants to modify the use registers,
	 * they can still do so by writing code which accesses the
	 * registers in the sigcontext structure contained in the
	 * ucontext field uc_mcontext.
	 *
	 * Copy the entire sigcontext structure into uc_mcontext (if
	 * this sigcontext structure is not already the sigcontext defined
	 * by uc_mcontext).
	 */
	if (copyall) {
		/* structure copy */
		ucp->uc_mcontext = *scp;
	}

	return(0);
}


fix_sigcontext(struct sigcontext *scp, ucontext_t *ucp, int copyall) 
{
	/*
	 * If calling this routine, about to sigreturn() to the
	 * kernel, so save uc_link field from this exiting ucontext.
	 */
	_old_ucontext = ucp->uc_link;

	/*
	 * If a separate sigcontext structure from the one defined by
	 * uc_mcontext is passed into this routine, copy the
	 * sigcontext defined by uc_mcontext into this structure.
	 */
	if (copyall) {
		/* structure copy */
		*scp = ucp->uc_mcontext;
	}

	scp->sc_mask = ucp->uc_sigmask;

	/*
	 * Copy signal stack info into sigcontext. Set SS_SETACK
	 * to tell the kernel to use the stack values from this
	 * sigcontext.
	 */
	scp->sc_onstack = (unsigned long)ucp->uc_stack.ss_flags	|
		SS_STACKMAGIC;
	scp->sc_sbase = ucp->uc_stack.ss_sp;
	scp->sc_ssize = ucp->uc_stack.ss_size;
}
