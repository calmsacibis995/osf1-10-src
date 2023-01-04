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
static char	*sccsid = "@(#)$RCSfile: rexecd.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/08 15:53:51 $";
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

/*
 * rexecd.c
 *
 *	Revision History:
 *
 * 16-Apr-91    Mary Walker
 *      added error checking to setuid
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/syslog.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <paths.h>

#define _PATH_DEFPATH   "PATH=/usr/bin:."

extern	errno;
char	*strncat();
/*VARARGS1*/
int	error();


#include <nl_types.h>
#include <locale.h>
#include <sia.h>
#include "rexecd_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_REXECD, n, s) 
nl_catd catd;

SIAENTITY *entity=NULL;
int oargc;
char **oargv;
/*
 * remote execute server:
 *	username\0
 *	password\0
 *	command\0
 *	data
 */
/*ARGSUSED*/
main(argc, argv)
	int argc;
	char **argv;
{
	struct sockaddr_in from;
	int fromlen;

	setlocale(LC_ALL, "");
        catd = NLcatopen(MF_REXECD, NL_CAT_LOCALE);

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, MSGSTR(GPEERERR, "%s: "), argv[0]); /*MSG*/
		perror(MSGSTR(PEERERR, "getpeername")); /*MSG*/
		exit(1);
	}
	oargc = argc;
	oargv = argv;
	doit(0, &from);
}

char	username[20] = "USER=";
char	homedir[64] = "HOME=";
char	shell[64] = "SHELL=";
char	time_zone[20] = "TZ=";
char	*envinit[] =
	    {homedir, shell, _PATH_DEFPATH, username, time_zone, 0};
extern  char	**environ;

struct	sockaddr_in asin = { AF_INET };

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp, *pw_shell;
	char user[16], pass[16];
	struct passwd *pwd;
	int s;
	u_short port;
	int pv[2], ready, readfrom, cc;
	pid_t pid;
	char buf[BUFSIZ], sig, *getenv(), *tzp;
	int one = 1, len = 0, euid;
	struct hostent *rhp;
	int (*sia_collect)()=sia_collect_trm;

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef TCP_DEBUG
	{ int t = open(_PATH_TTY, 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if (read(f, &c, 1) != 1)
			exit(1);
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}
	(void) alarm(0);
	if (port != 0) {
		s = socket(AF_INET, SOCK_STREAM, 0);
		if (s < 0)
			exit(1);
		if (bind(s, &asin, sizeof (asin)) < 0)
			exit(1);
		(void) alarm(60);
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0)
			exit(1);
		(void) alarm(0);
	}
	getstr(user, sizeof(user), "username");
	getstr(pass, sizeof(pass), "password");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	rhp = gethostbyaddr((char *)&fromp->sin_addr, sizeof(struct in_addr),fromp->sin_family);
	if ((sia_ses_init(&entity,oargc,oargv,rhp?rhp->h_name:(char *)NULL,user,(char *)NULL,FALSE,(char *)NULL)) == SIASUCCESS) {
	   if ((sia_ses_authent(sia_collect,pass,entity)) == SIASUCCESS) {
	      if ((sia_ses_estab(sia_collect,entity)) == SIASUCCESS) {
		 if (sia_ses_launch(sia_collect,entity) == SIASUCCESS) {

	      	    (void) write(2, "\0", 1);
		    pwd = entity->pwd;

		    if (port) {
		       (void) pipe(pv);
			pid = vfork();	/* fork not work */
			if (pid == -1)  {
			   error(MSGSTR(RETRY, "Try again.\n")); /*MSG*/
			   exit(1);
			}
			if (pid) {
			   (void) close(0); (void) close(1); (void) close(2);
			   (void) close(f); (void) close(pv[1]);
			   readfrom = (1<<s) | (1<<pv[0]);
			   ioctl(pv[1], FIONBIO, (char *)&one);
			   /* should set s nbio! */
			   do {
				ready = readfrom;
				(void) select(16, &ready, (fd_set *)0,
			        (fd_set *)0, (struct timeval *)0);
				if (ready & (1<<s)) {
					if (read(s, &sig, 1) <= 0)
						readfrom &= ~(1<<s);
					else
						killpg(pid, sig);
				}
				if (ready & (1<<pv[0])) {
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						shutdown(s, 1+1);
						readfrom &= ~(1<<pv[0]);
					} else
						(void) write(s, buf, cc);
				}
			   } while (readfrom);
			   exit(0);
		    	}
			setpgrp(0, getpid());
			(void) close(s); (void)close(pv[0]);
			dup2(pv[1], 2);
		    }
		    if (f > 2)
		       (void) close(f);
		    /* Now we need to TZ= from the environment */
		    if ((tzp = getenv("TZ")) != NULL)
		       strncat(time_zone, tzp, sizeof(time_zone)-4);

        	    environ = envinit;
        	    addenvvar("HOME",    pwd->pw_dir);
        	    addenvvar("SHELL",   pwd->pw_shell);
        	    addenvvar("LOGNAME", pwd->pw_name);
        	    addenvvar("USER",    pwd->pw_name);

		    /* save the shell before release the pointer */
		    len = strlen(pwd->pw_shell);
		    if ((pw_shell=malloc(len+1)) == NULL) {
		       error(MSGSTR(RETRY, "Try again.\n")); /*MSG*/
		       logerror(entity, "malloc");
		       exit(1);
		    }
		    strcpy(pw_shell,pwd->pw_shell);

		    cp = rindex(pw_shell, '/');
		    if (cp)
		       cp++;
		    else
		       cp = pw_shell;
		    sia_ses_release(&entity);
		    euid = geteuid();
		    if (setreuid(euid,euid) < 0) {
		       openlog("rexecd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
		       syslog(LOG_ERR, "setreuid: %m");
		       closelog();
		       exit(1);
		    }
		    execl(pw_shell, cp, "-c", cmdbuf, (char *)0);
		    perror(pw_shell);
		    exit(1);
		 } /* SIA session launch failure */
	      } /* SIA session establish failure */
	   } /*  SIA session authentication failure  */
	   else
	      error(MSGSTR(LOGINERR2, "Login incorrect.\n"));
	} /* SIA session init failure */
	logerror(entity, "SIA init");
	exit(1);
}

logerror(entity, err)
SIAENTITY *entity;
char	*err;
{
	if (entity != NULL)
		sia_ses_release(entity);
	openlog("rexecd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	syslog(LOG_INFO, MSGSTR(LOGINERR2," REXECD FAILURE:%s\n"), err);
	closelog();
}

/*VARARGS1*/
error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt, a1, a2, a3);
	(void) write(2, buf, strlen(buf));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1) {
			exit(1);
		}
		*buf++ = c;
		if (--cnt == 0) {
			error(MSGSTR(TOOLNG, "%s too long\n"), err); /*MSG*/
			exit(1);
		}
	} while (c != 0);
}

/* Build an environment table entry of the form 'tag=value' */

addenvvar(tag,value)
  char *tag,*value;

{       char    *penv;
        unsigned int len = strlen(tag)+1;       /* allow for '=' */

        if (!tag) {
		errno = EINVAL;
                perror("rexecd, addenvvar");
                exit(1);
        }
        if (value) len+= strlen(value);

        penv = malloc(len+1);
        strcpy(penv,tag);
        strcat(penv,"=");
        if (value) strcat(penv,value);

        if (putenv(penv)<0) {
		perror("rexecd, putenv");
                exit(1);
        }
        return;
}

