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
static char	*sccsid = "@(#)$RCSfile: update.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/11/04 09:03:10 $";
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
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * update.c	4.3 (Berkeley) 3/28/87
 */


/*
 * Update the file system every 30 seconds.
 */

#include <sys/time.h>
#include <sys/signal.h>
#include <syslog.h>


main()
{
	struct itimerval	value;
	extern int	sync();
	int ind, chpid;

	if (chpid = fork()) {
		if (chpid < 0) {
			perror("fork");
			exit(1);
		}
		sleep(1);
		exit(0);
	}

	for (ind = 0; ind < 3; ind++)
		(void)close(ind);

	(void)setsid();

	(void)signal(SIGALRM, (void (*)(int))sync);
	value.it_interval.tv_sec = 30;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;

	if (setitimer(ITIMER_REAL, &value, (struct itimerval *)NULL)) {
		if (openlog("update", LOG_CONS, LOG_DAEMON) >= 0) {
			syslog(LOG_ERR, "setitimer failure, unable to start");
			closelog();
		}
		exit(1);
	}
	if (openlog("update", LOG_CONS, LOG_DAEMON) >= 0) {
		syslog(LOG_INFO, "started");
		closelog();
	}
	for (;;)
		pause();
	/*NOTREACHED*/
}
