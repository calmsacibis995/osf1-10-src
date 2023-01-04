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
static char	*sccsid = "@(#)$RCSfile: siglist.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:43:52 $";
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
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: sys_siglist
 *
 * ORIGINS: 26 27
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * siglist.c	1.5  com/lib/c/gen,3.1,8943 9/9/89 09:31:35
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak sys_siglist = __sys_siglist
#endif
#endif
#include <signal.h>

/*
 * NAME: sys_siglist
 *
 * FUNCTION: external array of signal message strings
 *
 * RETURN VALUES: 
 *	NONE (not applicable)
 */

char	*sys_siglist[SIGMAX+1] = {
	"Signal 0",
	"Hangup",			/* 1  SIGHUP */
	"Interrupt",			/* 2  SIGINT */
	"Quit",				/* 3  SIGQUIT */
	"Illegal instruction",		/* 4  SIGILL */
	"Trace/BPT trap",		/* 5  SIGTRAP */
	"IOT/Abort trap",		/* 6  SIGABRT */
	"EMT trap",			/* 7  SIGEMT */
	"Floating point exception",	/* 8  SIGFPE */
	"Killed",			/* 9  SIGKILL */
	"Bus error",			/* 10 SIGBUS */
	"Segmentation fault",		/* 11 SIGSEGV */
	"Bad system call",		/* 12 SIGSYS */
	"Broken pipe",			/* 13 SIGPIPE */
	"Alarm clock",			/* 14 SIGALRM */
	"Terminated",			/* 15 SIGTERM */
	"Urgent I/O condition",		/* 16 SIGURG */
	"Stopped (signal)",		/* 17 SIGSTOP */
	"Stopped",			/* 18 SIGTSTP */
	"Continued",			/* 19 SIGCONT */
	"Child exited",			/* 20 SIGCHLD */
	"Stopped (tty input)",		/* 21 SIGTTIN */
	"Stopped (tty output)",		/* 22 SIGTTOU */
	"I/O possible/complete",	/* 23 SIGIO */
	"Cputime limit exceeded",	/* 24 SIGXCPU */
	"Filesize limit exceeded",	/* 25 SIGXFSZ */
	"Signal 26",			/* 26 SIGVTALRM */
	"Profiling time alarm",		/* 27 SIGPROF */
	"Window size changes",		/* 28 SIGWINCH */
	"Information request",		/* 29 SIGINFO */
	"User defined signal 1",	/* 30 SIGUSR1 */
	"User defined signal 2",	/* 31 SIGUSR2 */
};
