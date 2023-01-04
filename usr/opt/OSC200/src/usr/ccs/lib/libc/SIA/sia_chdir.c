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
static char *rcsid = "@(#)$RCSfile: sia_chdir.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/10 22:04:25 $";
#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak sia_timed_action = __sia_timed_action
#pragma weak sia_chdir = __sia_chdir
#endif
#endif

/*****************************************************************************
* Usage:  int sia_chdir(
*			const char *	directory,
*			time_t		timelimit)
*
* Description: The purpose of this routine is to implement a "NFS-safe"
* way to change the current working directory.  This routine will do a chdir()
* call which is protected by alarm() and signal-handling for SIGALRM, SIGHUP,
* SIGINT, SIGQUIT, and SIGTERM.  Receipt of any of these signals will cause an
* the 'chdir' operation to fail.  If the chdir call completes within the time
* limit given, the call succeeds.
*
* Parameter Descriptions: 
*
*	Param1: directory
*       Usage: desired new working directory.
*	Syntax: ASCII string (read-only), if NULL return SIAFAIL.
*
*	Param2: timelimit
*       Usage: How many seconds to allow for the chdir operation.
*	Syntax: if 0, there is no timeout.  if <0, SIA_DEF_TIMEOUT is used.
*
* Success return: SIASUCCESS if the chdir call completes successfully.
*
* Error Conditions: Timeout, interrupted by signal, or syscall failure.
*****************************************************************************/
#include "siad.h"
#include <signal.h>
#include <setjmp.h>

/* ASSUME SIA initialization is done by calling sia_*_authent routine*/
/* ASSUME already thread locked via SIA_AUTHENT_LOCK mutex */

static sigjmp_buf jbuf;

/* This routine is required only to break out of the chdir() call. */
static void
handler(int sig)
{
	(void) alarm(0);
	siglongjmp(&jbuf, sig);
}

/* ASSUME SIA initialization is done by calling sia_*_authent routine*/
/* ASSUME already thread locked via SIA_AUTHENT_LOCK mutex */

int
sia_timed_action(int (*act)(), void *param, time_t timelimit)
{
	time_t save_alarm;
	struct sigaction old_hup, old_int, old_quit, old_alrm, old_term;
	struct sigaction new_act;
	register struct sigaction *newp;
	sigset_t old_mask, new_mask;
	register sigset_t *maskp;
	int status, act_status;

	save_alarm = alarm(0);	/* stop previous alarms */
	if (timelimit < 0)
		timelimit = SIA_DEF_TIMEOUT;
	maskp = &new_mask;
	(void) sigemptyset(maskp);
	(void) sigaddset(maskp, SIGHUP);
	(void) sigaddset(maskp, SIGINT);
	(void) sigaddset(maskp, SIGQUIT);
	(void) sigaddset(maskp, SIGALRM);
	(void) sigaddset(maskp, SIGTERM);
	if (sigprocmask(SIG_BLOCK, maskp, &old_mask)) {
		if (save_alarm > 0)
			(void) alarm(save_alarm);
		return SIAFAIL;
	}
	/* Now that we have blocked the signals, we can start to change how
	   they're delivered. */
	newp = &new_act;
	memset((void *)newp, 0, sizeof *newp);
	newp->sa_handler = handler;
	newp->sa_mask = *maskp;
	status = 0;
	status |= sigaction(SIGHUP, newp, &old_hup);
	status |= sigaction(SIGINT, newp, &old_int);
	status |= sigaction(SIGQUIT, newp, &old_quit);
	status |= sigaction(SIGALRM, newp, &old_alrm);
	status |= sigaction(SIGTERM, newp, &old_term);
	act_status = SIAFAIL;	/* just in case (& shut up lint) */
	if (!status) {	/* ok so far, so try the chdir */
		if (sigsetjmp(&jbuf, 1) == 0) {
			(void) alarm(timelimit);
			status = sigprocmask(SIG_SETMASK, &old_mask, NULL);
			if (status == 0)
				act_status = (*act)(param);
			(void) alarm(0);
		}
		(void) sigprocmask(SIG_BLOCK, newp, &old_mask);
	}
	(void) sigaction(SIGHUP, &old_hup, (struct sigaction *)0);
	(void) sigaction(SIGINT, &old_int, (struct sigaction *)0);
	(void) sigaction(SIGQUIT, &old_quit, (struct sigaction *)0);
	(void) sigaction(SIGALRM, &old_alrm, (struct sigaction *)0);
	(void) sigaction(SIGTERM, &old_term, (struct sigaction *)0);
	(void) sigprocmask(SIG_SETMASK, &old_mask, NULL);
	return status ? SIAFAIL : act_status;
}

static int
do_chdir(const char *directory)
{
	return chdir(directory) < 0 ? SIAFAIL : SIASUCCESS;
}

int
sia_chdir(const char *directory, time_t timelimit)
{
	return sia_timed_action(do_chdir, (void *)directory, timelimit);
}
