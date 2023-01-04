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
static char *rcsid = "@(#)$RCSfile: spdinit.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/10/07 15:17:46 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*	Copyright (c) 1987-90 SecureWare, Inc.

	This is proprietary source code of SecureWare, Inc.
*/

/*
 * Based on:

 */

#include	"sys/secdefines.h"

#include <locale.h>
#include "secpolicy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SECPOLICY,n,s) 

#if SEC_ARCH

#include	"sys/types.h"
#include	"stdio.h"
#include	"errno.h"
#include	"pwd.h"
#include	"grp.h"
#if defined(_OSF_SOURCE)
#include	"string.h"
#include	"fcntl.h"
#endif
#ifdef _OSF_SOURCE
#include	"sys/ioctl.h"
#endif

#include	"sys/signal.h"
#include	"sys/errno.h"
#include	"sys/security.h"
#include	"sys/secpolicy.h"

/*	Security Policy Initialization Program

	This program is spawned during startup as a "sysinit" inittab
	entry. It is responsible for reading the security policy
	configuration file that has been set up by the administrator.
	The information is used to initiate the security policy daemons
	with the necessary arguments.

*/

#define LINE_LENGTH	256

FILE	*config_file;

void bad_syscall();

struct	sec_policy_conf sp_conf;
struct	passwd *getpwnam();
struct	group *getgrnam();

char in_line[LINE_LENGTH];

main(argc, argv)
int argc;
char **argv;
{
	struct passwd *pw;
	struct group *gr;
	int ret;
	ushort child_status;
	char *basename;

	if (!security_is_on())
		exit(0);

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_SECPOLICY,NL_CAT_LOCALE);


/* Set signals here to avoid termination problems */

	signal(SIGUSR1,SIG_IGN);
	signal(SIGUSR2,SIG_IGN);
	signal(SIGTERM,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT,SIG_IGN);
	signal(SIGSYS,bad_syscall);

/* Set the process group to the process id to divorce the daemon from its
   parent in case it was started from an administrator shell.		*/

#ifdef _OSF_SOURCE
	/* BSD incantations to divorce process from terminal */
	{
		int t = open("/dev/tty", O_RDWR);
		if (t > 0) {
			ioctl(t, TIOCNOTTY, (char *) 0);
			(void) close(t);
		}
	}
	setpgid(0, 0);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
#else
	setpgrp();
#endif
	
/* Find the tcb entry in the /etc/passwd file and do the
   setluid() so that the setuid() and setgid() are allowed. */

	if((pw = getpwnam("tcb")) == NULL)
		printf(MSGSTR(SPDINIT_1, "Policy daemon spawner: cannot find uid tcb.\n"));
	else {
		setluid(pw->pw_uid);
	}

/* Find the tcb group entry in the /etc/group file and do the
   setgid() for proper file permissions.		*/

	if((gr = getgrnam("tcb")) == NULL)
		printf(MSGSTR(SPDINIT_2, "Policy daemon spawner: cannot find group tcb.\n"));
	else
		setgid(gr->gr_gid);

/* Set the user id after group, otherwise setgid() will fail */

	setuid(pw->pw_uid);

/* Open the Security Policy Configuration File */

	if((config_file = fopen(SP_CONFIG_FILE,"r")) == NULL) {
		perror(MSGSTR(SPDINIT_3, "Policy daemon spawner: error on config file open"));
		exit(1);
	}

/* For each entry in the config file, read it and spawn the specified
   policy daemon with its configuration directory name as an argument. */

	while(fgets(in_line, LINE_LENGTH, config_file) != NULL) {

		ret = sscanf(in_line,"%s",sp_conf.daemon);

	/* Ignore blank/comment lines */

		if(   (*sp_conf.daemon == '\n')
		   || (*sp_conf.daemon == '#'))
			continue;

	/* Fork and Exec the Security Policy Daemon with One Argument */

		fflush(config_file);

		if((ret = fork()) == -1) {
			perror(MSGSTR(SPDINIT_4, "Policy daemon spawner: Error on fork of policy"));
			exit(1);
		}
		else if(ret != 0) {

	/* Parent process thread-wait for the child daemon thread to exit */

			wait(&child_status);

	/* Check the wait status to insure that the child has exited */

			if((child_status & 0xff) == 0) {
			    if((child_status & 0xff00) != 0)
				printf(MSGSTR(SPDINIT_5, "Daemon %s terminated abnormally\n"),
					sp_conf.daemon);
			}
			else if((child_status & 0xff00) == 0) {
				printf(MSGSTR(SPDINIT_6, "Daemon %s terminated by signal %d\n"),
					sp_conf.daemon,child_status & 0x7f);
			     }

	/* Spawn the next daemon process and wait for it as well */

			continue;

		     }

	/* Child process executes here, exec the policy daemon */

		fclose(config_file);
		basename = strrchr(sp_conf.daemon, '/');
		if (basename)
			basename++;
		else
			basename = sp_conf.daemon;

		execlp(sp_conf.daemon, basename, (char *) 0);

		perror(MSGSTR(SPDINIT_7, "Policy daemon spawner: execlp() failed on policy daemon"));
		exit(1);
	}

	fclose(config_file);
	exit(0);

}

/*
	bad_syscall()-trap a bad system call in case setluid() not on system
*/

void
bad_syscall()
{
	printf(MSGSTR(SPDINIT_8, "Policy daemon spawner: bad system call\n"));
	exit(1);
}

#endif /* SEC_ARCH */
