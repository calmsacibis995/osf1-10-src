#define	FAILURE3	99
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
static char	*sccsid = "@(#)$RCSfile: login.c,v $ $Revision: 4.2.14.7 $ (DEC) $Date: 1994/01/21 20:29:32 $";
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
 * Copyright (c) 1980, 1987, 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1987, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */
/********************************************************************************
* sia_login -    This program is the security integration architecture, SIA,   	*
*		 conformant version of login. This program utilizes libsia to  	*
*		 isolate it from security mechanism dependent operations like  	*
*		 checking passwords and setting up session information.        	*
*		 In general this program must authenticate, establish, and     	*
*		 launch the new session for the user or entity specified.      	*
*										*
*		 The sia session init code is called from sia_ses_authent and	*
*		 is used to check for system wide limitations like MAXUSERS and *
*		 /etc/.nologin.							*
*									       	*
* The following options are supported:						*
*										*
* Available to all users:	 login [ username ]				*
*										*
* Available to root only:							*
*										*
*										*
* login -f username	(for pre-authenticated login: datakit, xterm, etc)	*
*	Used with a username user on the command line to indicate that proper   *
*	authentication was already done and that no password needs to be        *
*	requested.  								*
*										*
* login -p									*
* 	Causes the remainder of the environment to be preserved, otherwise any  *
*	previous environment is discarded.					*
*										*
*****************	DISABLED OPTION  	*********************************
* SIA has moved all remote security functionality to sia_ses_r* commands	*
* Consequenty telnet, ftp, and rlogin will not utilize the following option     *
*										*
* login -h hostname	(for telnetd, etc.)					*
*	Used by telnetd(8) and other servers to list the host from which the	*
*	connection was received in utmp and wtmp.				*
*********************************************************************************/

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sgtty.h>
#include <utmp.h>
#include <signal.h>
#include <errno.h>
#include <ttyent.h>
#include <syslog.h>
#include <grp.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdio.h>
#include <strings.h>
#include <lastlog.h>
#include <sia.h>
#include "pathnames.h"

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "login_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LOGIN,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#define TTYGRPNAME      "tty"           /* name of group to own ttys */

char	shell[PATH_MAX+1] = _PATH_BSHELL;
char	 term[64], username[256], *tty;
char 	*password=NULL;
char 	*hostname=NULL;

struct	sgttyb sgttyb;
struct	tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct	ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};

SIAENTITY *entity=NULL;

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno, optind;
	extern char *optarg, **environ;
	register char *p;
	int fflag, pflag, cnt;
	char *ttyn, *pp, *loginname=NULL;
	char tbuf[MAXPATHLEN + 2], tname[sizeof(_PATH_TTY) + 10];
	char *ctime(), *ttyname(), *stypeof();
	char *strerror();
	off_t lseek();
	int (*sia_collect)()=sia_collect_trm;
	int passwd_req, ch, quietlog=0, failures;
	int oargc;
	int authret=0;
	char **oargv;
	char hush_path [PATH_MAX];			/* 004 */

	oargc = argc;
	oargv = argv;
#ifdef NLS
	(void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
	catd = catopen(MF_LOGIN,NL_CAT_LOCALE);
#endif
/************ set up signals, priority, and quota *******************/

	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
	(void)setpriority(PRIO_PROCESS, 0, 0);
#ifdef Q_SETUID
	(void)quota(Q_SETUID, 0, 0, (char *)0);
#endif
	/*  parse arguments
	 * -p is used by getty to tell login not to destroy the environment
 	 * -f is used to skip a second login authentication 
	 * -h DISABLED
	 */

	fflag = pflag = 0;
	passwd_req = 1;
	while ((ch = getopt(argc, argv, "fh:p")) != EOF)
		switch (ch) {
		case 'f':
/* if(sia_chk_invoker() == SIASUCCESS) */
			if(getuid() == 0)	/* DAL001 */
				fflag = 1;
			else {
				(void)fputs(MSGSTR(F_SU_ONLY,
					"login: -f for super-user only.\n"), stderr);
				exit(1);
			}
			break;
		case 'p':
			pflag = 1;
			break;
		case 'h':
/* if(sia_chk_invoker() == SIASUCCESS) */
			if(getuid() == 0)	/* DAL001 */
				hostname = optarg;
			else {
				(void)fputs(MSGSTR(SU_ONLY,
					"login: -h for super-user only.\n"), stderr);
				exit(1);
			}
			break;
		case '?':
		default:
			(void)fputs(MSGSTR(USAGE,
			    "usage: login [-fp] [-h hostname] [username]\n"), stderr);	/* DAL003 */
			/* Removed the "exit(1);" from this point to  */
			/* make login behavior more user friendly. To */
			/* exit here will blow away the x-term even   */
			/* if a wrong option is accidentally entered. */
							/*GA001*/
		}
	argc -= optind;
	argv += optind;

/***** Collect username if available ******/
	if (*argv) /***** get our own copy to avoid tampering *****/
		{
		bzero(username,sizeof(username));
		strcpy(username,*argv);
		loginname=username;
		}
/**** define tty ********/
/**** Note: tty setup and checking happens as part of sia_ses_authent in sia_ses_init ****/

	for (cnt = getdtablesize(); cnt > 2; cnt--)
		(void)close(cnt);

	ttyn = ttyname(0);
	if (ttyn == NULL || *ttyn == '\0') {
		(void)sprintf(tname, "%s??", _PATH_TTY);
		ttyn = tname;
	}
	if (tty = rindex(ttyn, '/'))
		++tty;
	else
		tty = ttyn;

	openlog("login", LOG_ODELAY, LOG_AUTH);

/********** SIA LOGIN PROCESS BEGINS *****************/
/********** logging of failures to sia_log is always done within the libsia ****/
/********** logging to syslog is the responsibility of the calling routine  ****/

	if((sia_ses_init(&entity, oargc, oargv, hostname, loginname, ttyn, 1, NULL)) == SIASUCCESS) {
		/***** SIA SESSION AUTHENTICATION *****/
		if(!fflag) {
			for(cnt=5; cnt; cnt--) {
				if((authret=sia_ses_authent(sia_collect,NULL,entity)) == SIASUCCESS)
					break;
				else	if(authret & SIASTOP)
					break;
				fputs(MSGSTR(INCORRECT, "Login incorrect\n"), stderr);	/* DAL002 */
			}
			if(cnt <= 0) {
				sia_ses_release(&entity);
				exit(1);
			}
		}
		/***** SIA SESSION ESTABLISHMENT *****/
		if(sia_ses_estab(sia_collect,entity) == SIASUCCESS) {
			/****** set up environment   *******/
			/* destroy environment unless user has requested preservation */
			if (!pflag) {
				pp = getenv("TERM");
				if (pp)
					strncpy(term, pp, sizeof term);
				clearenv();
			}
			(void)setenv("HOME", entity->pwd->pw_dir, 1);
			if(entity->pwd->pw_shell &&
			   *entity->pwd->pw_shell)
				strncpy(shell, entity->pwd->pw_shell, sizeof shell);
			(void)setenv("SHELL", shell, 1);
			if (term[0] == '\0')
				(void)strncpy(term, stypeof(tty), sizeof(term));
			(void)setenv("TERM", term, 0);
			(void)setenv("USER", entity->pwd->pw_name, 1);
			(void)setenv("LOGNAME", entity->pwd->pw_name, 1);
			(void)setenv("PATH", _PATH_DEFPATH, 0);
			/***** SIA LAUNCHING SESSION *****/
			if(sia_ses_launch(sia_collect,entity) == SIASUCCESS) {
			/* 004 - start */
			if ((entity -> pwd           != NULL) &&
			    (entity -> pwd -> pw_dir != NULL) &&
			    (entity -> pwd -> pw_dir [0] != 0))
				sprintf (hush_path, "%s/%s",
					 entity -> pwd -> pw_dir,
					 _PATH_HUSHLOGIN);
			else	strcpy (hush_path, _PATH_HUSHLOGIN);
			quietlog = access(hush_path, F_OK) == 0;
			/* 004 - end */
			if(!quietlog)
				quietlog = !*entity->pwd->pw_passwd && !usershell(entity->pwd->pw_shell);
				if (!quietlog) {
					struct stat st;
					
					motd();
					(void)sprintf(tbuf, "%s/%s", _PATH_MAILDIR, entity->pwd->pw_name);
					if (stat(tbuf, &st) == 0 && st.st_size != 0)
						(void)printf(MSGSTR(MAIL, "You have %smail.\n"),
							     (st.st_mtime > st.st_atime) ? MSGSTR(NEW, "new ") : "");
				}
				sia_ses_release(&entity);
				/******* Setup default signals **********/
				(void)signal(SIGALRM, SIG_DFL);
				(void)signal(SIGQUIT, SIG_DFL);
				(void)signal(SIGINT, SIG_DFL);
				(void)signal(SIGTSTP, SIG_IGN);
				
				tbuf[0] = '-';
				(void)strcpy(tbuf + 1, (p = rindex(shell, '/')) ?
					     p + 1 : shell);
				
				/****** Nothing left to fail *******/
				if(setreuid(geteuid(),geteuid()) < 0) {
					perror("setreuid()");
					exit(3);
				}
				execlp(shell, tbuf, 0);
				(void)fprintf(stderr, MSGSTR(NO_SHELL, "login: no shell: %s.\n"), strerror(errno));
				exit(0);
			}
			/***** SIA session launch failure *****/
		}
		/***** SIA session establishment failure *****/
	}
	logerror(entity);
	exit(1);
}


logerror(entity)
SIAENTITY *entity;
{	
	if(entity != NULL)
		{
		sia_ses_release(&entity);
		}
	syslog(LOG_ERR, MSGSTR(FAILURE3," LOGIN FAILURE "));
}
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/******************This is as far as I got ********************************/
/*****The remaining of this code is directly from login.c *****************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

jmp_buf motdinterrupt;

motd()
{
	register int fd, nchars;
	void (*oldint)(), sigint();
	char tbuf[8192];

	if ((fd = open(_PATH_MOTDFILE, O_RDONLY, 0)) < 0)
		return;
	oldint = signal(SIGINT, sigint);
	if (setjmp(motdinterrupt) == 0)
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
	(void)signal(SIGINT, oldint);
	(void)close(fd);
}

void sigint()
{
	longjmp(motdinterrupt, 1);
}

#undef	UNKNOWN
#ifdef	OSF
#define	UNKNOWN	"unknown"
#else
#define	UNKNOWN	 "su"
#endif

char *
stypeof(ttyid)
	char *ttyid;
{
	struct ttyent *t;

	return(ttyid && (t = getttynam(ttyid)) ? t->ty_type : UNKNOWN);
}


sleepexit(eval)
	int eval;
{
	sleep((u_int)5);
	exit(eval);
}

#ifdef OSF
/*
 *  Check if the specified program is really a shell (e.g. "sh" or "csh").
 */
usershell(shell)
	char *shell;
{
	register char *p;
	char *getusershell();

	setusershell();
	while ((p = getusershell()) != NULL)
		if (strcmp(p, shell) == 0)
			break;
	endusershell();
	return(!!p);
}

#endif	/* OSF */
