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
static char *rcsid = "@(#)$RCSfile: _getucontext.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/30 21:37:27 $";
#endif

#include <signal.h>
#include <sys/siginfo.h>
#include <ucontext.h>
#include <setjmp.h>
#include "pollution.h"

/*
 * This routine is only called by the assembler routine getcontext(). 
 * _getucontext() extracts the saved sp from the front of the ucontext
 * structure, where it was stored by ucontext.  It gets the signal
 * state (mask and stack), moves the saved sp and stack_t info
 * into the right locations in the sigcontext structure (which is part
 * of the ucontext structure - mcontext is really a sigcontext
 * structure), and calls fix_ucontext to fix the ucontext structure.
 */
struct sigcontext *
_getucontext(ucontext_t *ucp)
{
	long bit;
	int sig;
	long *context_info;
	struct sigaltstack sas, osas;
	struct sigcontext *scp;

	scp = &ucp->uc_mcontext;

	/*
	 * Extract info stored at front of ucontext by getcontext(),
	 * which called this routine
	 */
	context_info = (long *)ucp;
	scp->sc_sp = context_info[0];

	/* get signal mask */
	if (sigprocmask(SIG_BLOCK, NULL, &scp->sc_mask)) {
		perror("sigprocmask()");
		return(NULL);
	}

	/*
	 * Get stack flags by calling special version of sigalstack.
	 */
	if (_sigaltstack(NULL, &sas, TRUE)) {
		perror("sigaltstack()");
		return(NULL);
	}

	/*
	 * Because _sigaltstack() was called with TRUE, we're actually
	 * restoring the stack values from the uarea not the sigaltstack data.
	 */
	scp->sc_onstack = (long)sas.ss_flags;
	scp->sc_sbase = sas.ss_sp;
	scp->sc_ssize = sas.ss_size;

	fix_ucontext(ucp, scp, FALSE);

	return(scp);
}
