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
static char *rcsid = "@(#)$RCSfile: signals.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:14:02 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	signals.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:52:43  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:41  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:42  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:54:07  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved.
 *
 * Based on OSF version:
 *	@(#)signals.c	2.6 16:31:30 5/17/91 SecureWare
 */

/* #ident "@(#)signals.c	1.1 11:18:50 11/8/91 SecureWare" */

/*
 * signal handlers for ASCII screens.
 * Restore the non-curses mode and print an error message before exiting
 */

#include <sys/secdefines.h>
#include <sys/signal.h>
#include <stdio.h>
#include "If.h"
#include "aif_msg.h"
nl_catd catd;

/* Just to get rid of the definition in ../../common/If.h */
#ifdef MSGSTR
#undef MSGSTR
#endif

#define MSGSTR(n,s) catgets(catd,MS_AIF,n,s)

/* generic program failure signal handler */

void
hup_catch (sig)
int sig;
{
	restorescreen();
	switch (sig) {
	case SIGHUP:
		fprintf (stderr, MSGSTR(SIGNALS_1, "Program terminated due to user request\n"));
		break;
	case SIGILL:
	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
	case SIGPIPE:
	case SIGTERM:
	case SIGIO:
		fprintf (stderr,
			MSGSTR(SIGNALS_2, "Premature program termination: code %d\n"), sig);
		break;
	case SIGXFSZ:
		fprintf (stderr,
		MSGSTR(SIGNALS_3, "Premature program termination: file size exceeded.\n"));
		break;
	default:
		fprintf (stderr,
			MSGSTR(SIGNALS_4, "Unexpected program error: signal code %d\n"), sig);
		break;
	}
	fflush (stderr);
	exit(-1);
}

/* SIGALRM signal handler */

int
time_catch (sig)
int sig;
{
	updatetimedate();
	(unsigned) alarm ((unsigned) UPD_SECS);
}

/*
 *	sig_set[n] references signal # n+1
 *	if 0, the corresp. signal is handled normally
 *	if 1, the corresp. signal should be ignored
 *	if 2, the corresp. signal should be hup_catch()ed (sic)
 *
 *	SIG_HUP should be left alone here,
 *	as it is normally set elsewhere to use hup_catch.
 *
 *	SIG_ALRM should be left alone here,
 *	as it is normally set elsewhere to use time_catch.
 */

int sig_set[] = {
#if defined(BSD) || defined(OSF) || defined(_BSD) || defined(_POSIX_SOURCE)
	0, 1, 1, 2, 2, 2, 2, 2, 0, 2,
	2, 2, 2, 0, 2, 0, 0, 1, 1, 0,
	1, 1, 2, 1, 2, 1, 0, 0, 0, 1,	/* no sig # 29 documented */
	1
#else /* BSD */
# ifdef SYSV
#  ifdef SVR3
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0
#  else  /* SVR3 */
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
	1, 1, 0, 1, 0, 1, 1, 0, 0, 0,
	0
#  endif /* SVR3 */
# endif /* SYSV */
#endif /* BSD */
};

#define MAX_SIGS (sizeof (sig_set) / sizeof (int))

/* Signal setup routine */

void
setup_signals()
{
	register int i;
	register int *sp;

	for (i=0, sp=sig_set; i<MAX_SIGS; i++) {
		switch (sp[i]) {
			case 1:
			signal (i+1, SIG_IGN);
			break;

			case 2:
			signal (i+1, hup_catch);
			break;
		}
	}
}
