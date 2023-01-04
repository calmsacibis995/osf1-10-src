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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: fault.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 15:25:37 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.17  com/cmd/sh/sh/fault.c, cmdsh, bos320, 9125320 6/6/91 23:10:38
 */

#include	"defs.h"
#include        "timeout.h"

extern void	done(int);
extern void	fault(int);

static struct sigaction fault_action	= { fault, 0, 0 };
static struct sigaction done_action	= { done, 0, 0 };
static struct sigaction dfl_action	= { SIG_DFL, 0, 0 };
static struct sigaction ign_action	= { SIG_IGN, 0, 0 };
static struct sigaction old_action	= { 0, 0, 0 };

uchar_t	*trapcom[SIGMAX+1];
BOOL	trapflg[SIGMAX+1];	/*  init to 0  */


	/*	Initialize signal handling.
	*	Changed all signal handling to use sigaction call
	*	instead of signal call. 
	*/
struct sigaction	*sigact [] = {
		&dfl_action ,		/*  0 */
	        &done_action ,		/*  1 - SIGHUP */
	        &fault_action ,		/*  2 - SIGINT */
	        &fault_action ,		/*  3 - SIGQUIT */
	        &done_action ,		/*  4 - SIGILL */
	        &done_action ,		/*  5 - SIGTRAP */
	        &done_action ,		/*  6 - SIGABRT */
	        &done_action ,		/*  7 - SIGEMT */
	        &done_action ,		/*  8 - SIGFPE */
	        &dfl_action ,		/*  9 - SIGKILL */
	        &done_action ,		/* 10 - SIGBUS */
	        &fault_action ,		/* 11 - SIGSEGV */
	        &done_action ,		/* 12 - SIGSYS */
	        &done_action ,		/* 13 - SIGPIPE */
	        &fault_action ,		/* 14 - SIGALRM */
	        &fault_action ,		/* 15 - SIGTERM */
	        &dfl_action ,		/* 16 - SIGURG */
	        &dfl_action ,		/* 17 - SIGSTOP */
	        &dfl_action ,		/* 18 - SIGTSTP */
	        &dfl_action ,		/* 19 - SIGCONT */
	        &done_action ,		/* 20 - SIGCHLD */
	        &dfl_action ,		/* 21 - SIGTTIN */
	        &dfl_action ,		/* 22 - SIGTTOU */
	        &dfl_action ,		/* 23 - SIGIO */
	        &dfl_action ,		/* 24 - SIGXCPU */
	        &dfl_action ,		/* 25 - SIGXFSZ */
	        &dfl_action ,		/* 26 - SIGVTALRM */
	        &dfl_action ,		/* 27 - SIGPROF */
	        &dfl_action ,		/* 28 - SIGWINCH */
	        &dfl_action ,		/* 29 - SIGINFO */
	        &done_action ,		/* 30 - SIGUSR1 */
	        &done_action };		/* 31 - SIGUSR2 */


int	trap_waitrc = 0;
int	trap_status = 0;

/* ========	fault handling routines	   ======== */

extern long mailtime;
extern int mailchk;

void
stdsigs()
{
	int i;

	for(i=1; i < SIGMAX; i++) {
		switch(i) {
			case SIGHUP:
			case SIGINT:
			case SIGILL:
			case SIGTRAP:
			case SIGABRT:
			case SIGEMT:
			case SIGFPE:
			case SIGBUS:
			case SIGSEGV:
			case SIGSYS:
			case SIGPIPE:
			case SIGALRM:
			case SIGTERM:
			case SIGUSR1:
			case SIGUSR2:
				setsig(i);
				break;
			case SIGQUIT:
			case SIGXFSZ:
				ignsig(i);
				break;
			default:
				sigaction(i,&dfl_action,(struct sigaction *)0);
				break;
		}
	}
}

void
specsigs()
{
#define SIGS	14
int count;
static int signals[SIGS]={SIGHUP, SIGINT, SIGILL, SIGQUIT,
			  SIGTRAP, SIGABRT, SIGEMT, SIGFPE, 
			  SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, 
			  SIGALRM, SIGTERM};

	for(count = 0; count < SIGS; count++) {
		if(signals[count] == SIGQUIT)
			ignsig(signals[count]);
		else
			setsig(signals[count]);
	}
}

void
fault(int sig)
{
	register int	flag;

	if ( sig == SIGCHLD )		/* Wait for child here */
		trap_waitrc = wait (&trap_status);


	if (sig == SIGSEGV)
	{
		if (setbrk(brkincr) == -1)
			error(MSGSTR(M_NOSPACE,(char *)nospace));
	}
	else if (sig == SIGALRM)
	{
		long int curtime;

		time(&curtime);
		if(mailchk) {
			chkmail();
			mailalarm = 1;
			alarm(mailchk);
			mailtime = curtime;
		}
		if (timecroak && flags&waiting && (curtime >= timecroak))
			done(0);
	}
	else
	{
		flag = (trapcom[sig] ? TRAPSET : SIGSET);
		trapnote |= flag;
		trapflg[sig] |= flag;
		if (sig == SIGINT)
			wasintr++;
	}
}

int
ignsig(int i)
{
	register int    s;

	if (i == SIGSEGV)
	{
		clrsig(i);
		{
		  uchar_t buf[NL_TEXTMAX];
		  strcpy ((char *)buf, MSGSTR(M_BADTRAP, (char *)badtrap));
		  failed(buf, MSGSTR(M_TRAP,"cannot trap SIGSEGV"));
		}
	}
	else {
		sigaction(i, &ign_action, &old_action);
		if ((s = (old_action.sa_handler == SIG_IGN)) == 0)
			trapflg[i] |= SIGMOD;
	}
	return(s);
}

int
setsig(int n)
{
	register int i;

	if ( n == SIGSEGV )
		sigaction( n, sigact[n], (struct sigaction *)0 );
	else if (ignsig(n) == 0)
		sigaction( n, sigact[n], (struct sigaction *)0 );
}

void
getsig(int n)
{
	register int i;

	if (trapflg[i = n] & SIGMOD || ignsig(i) == 0)
		sigaction(i,&fault_action,(struct sigaction *)0);
}


void
oldsigs()
{
	register int	i;
	register uchar_t	*t;

	i = SIGMAX;
	while (i--)
	{
		t = trapcom[i];
		if (t == 0 || *t)
			clrsig(i);
		trapflg[i] = 0;
	}
	trapnote = 0;
}

int
clrsig(int i)
{
	alloc_free(trapcom[i]);
	trapcom[i] = 0;
	if (trapflg[i] & SIGMOD)
	{
		trapflg[i] &= ~SIGMOD;
		sigaction(i, sigact[i], (struct sigaction *)0);
	}
}

/*
 * check for traps
 */
void
chktrap()
{
	register int	i = SIGMAX;
	register uchar_t	*t;

	trapnote &= ~TRAPSET;

	while (--i)
	{
		if (trapflg[i] & TRAPSET)
		{
			trapflg[i] &= ~TRAPSET;
			if (t = trapcom[i])
			{
				int	savxit = exitval;

				execexp(t, 0);
				exitval = savxit;
				exitset();
			}
		}
	}
}
