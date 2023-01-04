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
static char	*sccsid = "@(#)$RCSfile: signals.c,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/04/20 17:19:56 $";
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
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when combined with the
 * aggregated modules for this product) OBJECT CODE ONLY SOURCE MATERIALS (C)
 * COPYRIGHT International Business Machines Corp. 1989 All Rights Reserved 
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp. 
 */

#include	<sys/param.h>
#include	<sys/signal.h>
#include	<sys/wait.h>
#include	<ctype.h>
#include	"init.h"
#if defined(NLS) || defined(KJI)
#include	<sys/NLchar.h>
#endif
#include	<stdio.h>

#ifdef	MSG
#include	"init_msg.h"
#endif


/****************************/
/****    init_signals    ****/
/****************************/

/* Initialize all signals to either be caught or ignored.		 */

init_signals()
{
	extern int      siglvl(int), alarmclk(void), idle(void);
	extern int	childeath(void), powerfail(void), danger(void);
	extern int	usr1(void);

#ifdef	UDEBUG
	extern int      abort();
#endif
	struct sigaction action;
	int             i;

#ifdef	XDEBUG
	debug("We have entered init_signals().\n");
#endif

	/* let's set up our actions for the sigaction calls */
	action.sa_flags = 0;	/* no flags */
	action.sa_mask = 0;	/* no masked signals */

	for (i = 1; i <= SIGMAX; i++) {
		switch (i) {
		case LVLQ:
		case LVL0:
		case LVL1:
		case LVL5:
		case LVL6:
		case LVL7:
		case LVL8:
		case LVL9:
		case SINGLE_USER:
		case LVLa:
		case LVLb:
		case LVLc:
			action.sa_handler = (void (*) (int)) siglvl;
			sigaction(i, &action, NULL);
			break;
		case LVL2:
		case LVL3:
		case LVL4:
#ifdef	UDEBUG
			action.sa_handler = SIG_DFL;
#else
			action.sa_handler = (void (*) (int)) siglvl;
#endif
			sigaction(i, &action, NULL);
			break;
		case SIGTERM:
#ifdef	UDEBUG
			action.sa_handler = SIG_DFL;
#else
			action.sa_mask = sigmask(SIGHUP);
			action.sa_handler = (void (*) (int)) siglvl;
#endif
			sigaction(i, &action, NULL);
			break;

		case SIGUSR1:
			action.sa_handler = (void (*) (int)) usr1;
			sigaction(i, &action, NULL);
			break;

		case SIGUSR2:
#ifdef	UDEBUG
			action.sa_handler = (void (*) (int)) abort;
#else
			action.sa_handler = SIG_IGN;
#endif
			sigaction(i, &action, NULL);
			break;
		case SIGALRM:
			action.sa_handler = (void (*) (int)) alarmclk;
			sigaction(i, &action, NULL);
			break;
		case SIGCHLD:
			action.sa_handler = (void (*) (int)) childeath;
			sigaction(i, &action, NULL);
			break;
#if	0	/* We do not at present support SIGPWR */
		case SIGPWR:
			action.sa_handler = (void (*) (int)) powerfail;
			sigaction(i, &action, NULL);
			break;
#endif
		case SIGTSTP:
			action.sa_handler = (void (*) (int)) idle;
			sigaction(i, &action, NULL);
			break;
		default:
			action.sa_handler = SIG_IGN;
			sigaction(i, &action, NULL);
		}
	}
	alarmclk();
}

/**********************/
/****    siglvl    ****/
/**********************/

siglvl(int sig)
{
	register struct proc *p;
	extern volatile pid_t cur_pid;

#ifdef	XDEBUG
	debug("We have entered siglvl().\n");
#endif

	/* Set the flag saying that a "user signal" was received. */
	wakeup_flags |= W_USRSIGNAL;

	if (sig == SIGTERM) {
		wakeup_flags |= W_SHUTDOWN;
		kill(cur_pid, SIGTERM);
		cur_pid = PID_MAX + 1;
		return;
	}
	/*
	 * If the signal received is a "LVLQ" signal, do not really
	 * change levels, just restate the current level.
	 */
	if (sig == LVLQ)
		new_state = cur_state;
	/*
	 * If the signal received is something other than "LVLQ", set
	 * the new level to the value of the signal received.
	 */
	else
		new_state = sig;
	/*
	 * Clear all times and repeat counts in the process table
	 * since either the level is changing or the user has editted
	 * the "/etc/inittab" file and wants us to look at it again.  If
	 * the user has fixed a typo, we don't want residual timing data
	 * preventing the fixed command line from executing.
	 */
	for (p = proc_table; p != NULL; p = p->p_next) {
		p->p_time = 0;
		p->p_count = 0;
	}
	return (0);
}

/************************/
/****    alarmclk    ****/
/************************/

int             time_up;	/* Flag set to TRUE by alarm interupt routine */
				/* each time an alarm interupt takes place. */
alarmclk(void)
{
#ifdef	XDEBUG
	debug("We have entered alarmclk().\n");
#endif
	time_up = TRUE;
	return (0);
}

/*************************/
/****    childeath    ****/
/*************************/

childeath(void)
{
	register struct proc *p;
	register pid_t    pid;
	int             status;

#ifdef	XDEBUG
	debug("We have entered childeath().\n");
#endif
	/*
	 * Perform wait to get the process id of the child who died and
	 * then scan the process table to see if we are interested in
	 * this process.
	 */
	pid = 0;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
#ifdef	XDEBUG
		debug("childeath: pid- %d status- %x\n", pid, status);
#endif
		for (p = proc_table; p != NULL; p = p->p_next) {
			if (p->p_pid == pid) {
				/*
				 * Mark this process as having died and store
				 * the exit status. 
				 * Also set the wakeup flag for a dead child
				 * and break out of loop.
				 */
#ifdef DEBUG1
				debug("childeath: found pid %d\n", pid);
#endif
				p->p_flags &= ~LIVING;
				p->p_exit = status;
				wakeup_flags |= W_CHILDEATH;
				break;
			}
		}
#ifdef	UDEBUG
		if (p == NULL)
			debug("Didn't find process %d.\n", pid);
#endif
	}			/* end while */
	return (0);
}

/*********************/
/****    timer    ****/
/*********************/

/* "timer" is a substitute for "sleep" which uses "alarm" and	 */
/* "pause".							 */

timer(waitime)
	register int    waitime;

{
	setimer(waitime);
	while (time_up == FALSE)
		pause();
}

/***********************/
/****    setimer    ****/
/***********************/

setimer(timelimit)
	int             timelimit;
{
	alarmclk();
	alarm(timelimit);
	time_up = (timelimit ? FALSE : TRUE);
}



/***********************/
/****    usr1    *******/
/***********************/

/* usr1() when invoked via a signal from /sbin/rc0      */
/* resets the value of 'utmp_exists' so that account()  */
/* is alerted to the inaccessibility of /var/adm/utmp.  */

usr1(void)
{
	extern int utmp_exists;

	utmp_exists = 0;
}
