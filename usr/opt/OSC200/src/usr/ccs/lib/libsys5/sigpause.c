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
static char *rcsid = "@(#)$RCSfile: sigpause.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/24 19:49:25 $";
#endif

#include <sys/signal.h>
#include <sys/errno.h>

/*
 * System V version of sigapuse() (SVID3). The caller wants sig unblocked
 * while it sleeps waiting for that (or any) signal.
 * NOTE: This behavior is contrary to the libc/BSD version of
 * sigpause(), which is why libsys5 has its own version.
 */
int
sigpause(int sig)
{
	sigset_t set;

	if (sig == SIGKILL || sig == SIGSTOP ||
	    sig > SIGMAX || sig < 0) {
		_Seterrno(EINVAL);
		return -1;
	}

	(void) sigprocmask(SIG_BLOCK, NULL, &set);
	(void) sigdelset(&set, sig);
	return(sigsuspend(&set));
}
