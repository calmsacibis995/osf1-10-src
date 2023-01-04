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
static char	*sccsid = "@(#)$RCSfile: signals.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:02:35 $";
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
 * Copyright (c) 1989 SecureWare, Inc.  All rights reserved.
 */



/* signal handlers
 */

#include <sys/signal.h>
#include <stdio.h>
#include "kitch_sink.h"

#ifdef DEBUG
extern	FILE	*logfp;
#endif DEBUG

int
hup_catch (sig)
int sig;
{
	restorescreen();
	switch (sig) {
	case SIGHUP:
		fprintf (stderr, "Program terminated due to user request\n");
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
			"Premature program termination: code %d\n", sig);
		break;
	case SIGXFSZ:
		fprintf (stderr,
		"Premature program termination: file size exceeded.\n");
		break;
	default:
		fprintf (stderr,
			"Program lost it's mind: code %d\n", sig);
		break;
	}
	fflush (stderr);
	exit(-1);
}



int
time_catch (sig)
int sig;
{
	updatetimedate();
	(unsigned) alarm ((unsigned) UPD_SECS);
}

#if defined(_BSD) || defined(BSD44)
#define BSD 1
#endif _BSD

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
#if defined (BSD) || defined (OSF)
	0, 1, 1, 2, 2, 2, 2, 2, 0, 2,
	2, 2, 2, 0, 2, 0, 0, 1, 1, 0,
	1, 1, 2, 1, 2, 1, 0, 0, 0, 1,	/* no sig # 29 documented */
	1
};
#else BSD
# ifdef SYSV
#  ifdef SVR3
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0
};
#  else  SVR3
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 1, 1, 0, 0, 1,
	1, 1, 0, 1, 0, 1, 1, 0, 0, 0,
	0
};
#  endif SVR3
# endif SYSV
#endif BSD

#define MAX_SIGS (sizeof (sig_set) / sizeof (int))



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
