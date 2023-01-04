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
static char	*sccsid = "@(#)$RCSfile: abort.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/06/08 01:22:55 $";
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
 * FUNCTIONS: abort
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * abort.c	1.14  com/lib/c/gen,3.1,8943 10/10/89 10:21:36
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <signal.h>
#include <stdlib.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _abort_rmutex;
#endif

/*
 * FUNCTION:	Causes abnormal program termination to occur, unless the
 *		signal SIGABRT is being caught and the signal handler does
 *		not return.
 *
 * RETURN VALUE DESCRIPTION:
 *              The abort function cannot return to its caller.
 */

static	int abortcnt;

void
abort(void)
{
	struct sigaction action;
	sigset_t mask;

        TS_LOCK(&_abort_rmutex);
	if (!abortcnt) {
		abortcnt++;
                _cleanup();
	}
        TS_UNLOCK(&_abort_rmutex);

	(void) raise(SIGABRT);	/* Terminate the process */

	/*
	 * If the code reaches here then the user has a signal handler
         * that both handles aborts and generates them. So, replace the
	 * the user handler with the default handler. 
	 * This will keep the task from looping forever when the abort
	 * signal is unblocked.
	 */
	action.sa_handler = SIG_DFL;
	action.sa_flags = 0;
	(void) sigemptyset(&action.sa_mask);
 	(void) sigaction(SIGABRT, &action, (struct sigaction *)NULL);

	/*
	 * The signal handler for SIGABRT has returned or SIGABRT is blocked.
	 * Take additional steps to terminate the process.  If there is a
	 * pending SIGABRT then unblocking it may terminate the process.
	 */
	(void) sigemptyset(&mask);
	(void) sigaddset(&mask, SIGABRT);
 	(void) sigprocmask(SIG_UNBLOCK, &mask, (sigset_t *)NULL);

	(void) raise(SIGABRT);

	exit(SIGABRT);	/* Should never ever get this far */
}
