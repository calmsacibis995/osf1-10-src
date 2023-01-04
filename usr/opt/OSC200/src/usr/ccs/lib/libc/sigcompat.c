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
static char	*sccsid = "@(#)$RCSfile: sigcompat.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/08/24 19:48:36 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak sigblock = __sigblock
#pragma weak sigsetmask = __sigsetmask
#pragma weak sigvec = __sigvec
#endif
#endif
#include <sys/param.h>
#include <sys/signal.h>

sigvec(signo, sv, osv)
	int signo;
	struct sigvec *sv, *osv;
{

        struct sigaction act, *sa;
	int ret;
#ifdef __alpha
        struct sigaction oact, *osa;
#endif

	/*
	 * Have to copy the user's structure, since we're going to invert
	 * the SV_INTERRUPT bit, and we may not be able to write into
	 * caller's input argument.
	 */
	if (sv) {
		act.sa_handler = sv->sv_handler;
		act.sa_mask = sv->sv_mask;
		act.sa_flags = sv->sv_flags ^ SV_INTERRUPT; /* !SA_INTERRUPT */
		sa = &act;
	}
	else {
		sa = (struct sigaction *)0;
	}

#ifdef __alpha
	if (osv)
		osa = &oact;
	else
		osa = (struct sigaction *)0;

	ret = sigaction(signo, sa, osa);
#else
	ret = sigaction(signo, sa, (struct sigaction *)osv);
#endif

	if (ret == 0 && osv) {
#ifdef __alpha
		/*
		 * Have to copy out to the user's structure, since the sigvec
		 * structure may be a different size than sigaction.
		 */
		osv->sv_handler = osa->sa_handler;
		osv->sv_mask = osa->sa_mask;
		osv->sv_flags = osa->sa_flags ^ SV_INTERRUPT;/* !SA_INTERRUPT */
#else
		osv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
#endif
	}
	return (ret);
}

sigsetmask(mask)
	int mask;
{
	int n;
#ifdef __alpha
	/*
	 * Must copy int mask into sigset_t before calling
	 * sigprocmask().
	 */
	sigset_t omask, nmask = (sigset_t) mask;
	n = sigprocmask(SIG_SETMASK, &nmask, &omask);
#else
	int omask;
	n = sigprocmask(SIG_SETMASK, (sigset_t *) &mask, (sigset_t *) &omask);
#endif
	if (n)
		return (n);
	return (omask);
}

sigblock(mask)
	int mask;
{
	int n;
#ifdef __alpha
	/*
	 * Must copy int mask into sigset_t before calling
	 * sigprocmask().
	 */
	sigset_t omask, nmask = (sigset_t) mask;
	n = sigprocmask(SIG_BLOCK, &nmask, &omask);
#else
	int omask;
	n = sigprocmask(SIG_BLOCK, (sigset_t *) &mask, (sigset_t *) &omask);
#endif
	if (n)
		return (n);
	return (omask);
}
