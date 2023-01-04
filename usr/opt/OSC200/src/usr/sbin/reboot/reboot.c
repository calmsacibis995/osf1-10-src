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
static char	*sccsid = "@(#)$RCSfile: reboot.c,v $ $Revision: 4.2.14.2 $ (DEC) $Date: 1993/10/08 15:53:20 $";
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
 * FUNCTIONS: reboot
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <prot.h>

extern priv_t *privvec();
#endif

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <locale.h>
#include <sys/reboot.h>
#include <utmp.h>
#include <dec/binlog/binlog.h>            /* binary event log support */
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
struct utmp wtmp;

#ifdef MSG
#include "reboot_msg.h"
nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_REBOOT,n,s)
#ifdef SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_REBOOT_SEC,n,s)
#endif
#else
#define MSGSTR(n,s) s
#endif

char binlog_msg[1024];            /* binary event log support */
char UMOUNT[] = "/sbin/umount";
char UTMP[] = "/var/adm/utmp";


/*
 * NAME:  Reboot
 * FUNCTION:  restarts the machine.
 *   -l   Do not log the reboot in the system accounting records.
 *        file. The -n and -q options imply -l.
 *   -n   Do not perform the sync.
 *   -q   Reboot quickly and ungracefully, without shutting down running 
 *        processes first.
 */
main(argc, argv)
	int argc;
	char **argv;
{
	int howto;
	int qflag = 0;
	int nflag = 0;
	int needlog = 1;
	int ufd = -1;
	int i;
	int pid;
	char *user;
	struct passwd *pw;

#ifdef NLS
	(void) setlocale(LC_ALL, "");
#endif
#ifdef MSG
	catd = NLcatopen(MF_REBOOT,NL_CAT_LOCALE);
#endif

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("sysadmin")) {
		fprintf(stderr, MSGSTR_SEC(AUTH,
			"%s: command requires 'sysadmin' authorization.\n"),
			command_name);
		exit(1);
	}
	if (forceprivs(privvec(SEC_ALLOWDACACCESS, SEC_SHUTDOWN,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr,
			MSGSTR(PRIV, "%s: insufficient privileges\n"),
			command_name);
		exit(1);
	}

#else /* SEC_BASE */
	if (geteuid()) {
                fprintf(stderr, MSGSTR(NO_ROOT, "NOT super-user\n"));
                exit(1);
        }

#endif /* SEC_BASE */

	argc--, argv++;
	howto = RB_AUTOBOOT;
	while (argc > 0) {
		if (!strcmp(*argv, "-q"))	/* quick reboot */
			qflag++;
		else if (!strcmp(*argv, "-n"))	/* do not sync disks */
			howto |= RB_NOSYNC;
		else if (!strcmp(*argv, "-l"))	/* do not log reboot */
			needlog = 0;
		else {
			fprintf(stderr,
			    MSGSTR(USAGE,"usage: reboot [-l][-n][-q]\n"));
			exit(1);
		}
		argc--, argv++;
	}

	if (needlog) {
		openlog("reboot", 0, LOG_AUTH);
		user = getlogin();
		if (user == (char *)0 && (pw = getpwuid(getuid())))
			user = pw->pw_name;
		if (user == (char *)0)
			user = "root";
		syslog(LOG_CRIT,MSGSTR(LOGIT,"rebooted by %s"), user);

		/* put message into the binary event log */
		sprintf(binlog_msg, "System rebooted by %s", user);
		binlogmsg(ELMSGT_SD, binlog_msg);
	}

	signal(SIGHUP, SIG_IGN);	/* for remote connections */
	signal(SIGTERM, SIG_IGN);	/* to not kill ourselves */
	if (kill(1, SIGTSTP) == -1) {
		fprintf(stderr, MSGSTR(IDLE, "reboot: can't idle init\n"));
		exit(1);
	}
	sleep(1);
	(void) kill(-1, SIGTERM);	/* one chance to catch it */
	sleep(5);			/* give processes with long cleanup */
					/* a chance to exit */
	if (!qflag) {
		if (!qflag && !(howto & RB_NOSYNC)) {
			markdown();
			sync();
		}
		for (i = 1; ; i++) {
			if (kill(-1, SIGKILL) == -1) {
				extern int errno;

				if (errno == ESRCH)
					break;

				perror("reboot: kill");
				kill(1, SIGHUP);
				exit(1);
			}
			if (i > 5) {
				fprintf(stderr, MSGSTR(CAUTION,"CAUTION: some process(es) wouldn't die\n"));
				break;
			}
			setalarm(2 * i);
			pause();
		}
		if (!(howto & RB_NOSYNC))
			sync();
	}
	chdir("/");

	if ((ufd = open(UTMP, O_TRUNC, 0644)) < 0) {
		fprintf(stderr, MSGSTR(UTMPZ, "Couldn't open /var/adm/utmp\n"));
	} else
		close(ufd);

	/* Perform file system dismounts */
	/* -A: Remove all mounts; ufs, nfs auto/manual */
	/* -f: Remove nfs mounts without server confirmation */

	if((pid = fork()) < 0) { 
		perror("fork");
	} else if  (pid == 0) {
		execl(UMOUNT, "umount", "-Af", NULL);
		exit(-1);
	} 
	if(waitpid(pid, NULL, 0) < 0)
		perror("waitpid");

	printf(MSGSTR(REBOOT,"Rebooting . . .\n"));
	fflush(stdout);
	sleep((unsigned)3);
	reboot(howto, (time_t *)0);
	perror("reboot");
	kill(1, SIGHUP);
	exit(1);
}

/*
 * NAME: dingdong
 * FUNCTION:  catch SIGALRM
 */
dingdong(void)
{
	/* RRRIIINNNGGG RRRIIINNNGGG */
}

/*
 * NAME: setalarm
 * FUNCTION: set up alarm.
 */
setalarm(n)
{
	signal(SIGALRM, (void (*)(int))dingdong);
	alarm((unsigned)n);
}

/*
 * NAME: markdown
 * FUNCTION:  write shutdown entry in /usr/adm/wtmp file.
 */
markdown()
{
	register f = open(WTMP_FILE, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "shutdown");
		SCPYN(wtmp.ut_host, "");
		wtmp.ut_type=USER_PROCESS;
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
}
