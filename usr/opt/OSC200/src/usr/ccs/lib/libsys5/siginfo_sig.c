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
static char *rcsid = "@(#)$RCSfile: siginfo_sig.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/08/24 19:49:13 $";
#endif

#include <signal.h>
#include <sys/siginfo.h>
#include <ucontext.h>

/*
 * Fill in a ucontext structure with information from the sigcontext,
 * pass the ucontext and siginfo to the handler, and copy changes from
 * the ucontext back to the sigcontext. 
 *
 * Note: this is libsys5-only behavior.
 */
siginfo_sig(
	int signal, 
	siginfo_t *sip, 
	struct sigcontext *scp, 
	int (*sighandler)() )
{
	ucontext_t ucontext;

	fix_ucontext(&ucontext, scp, TRUE);

	/*
	 * Call the signal handler.
	 */
	(*sighandler)(signal, sip, &ucontext);

	fix_sigcontext(scp, &ucontext, TRUE);

	/*
	 * siginfo_sig() returns to the signal trampoline code, which
	 * calls sigreturn().
	 */
}
