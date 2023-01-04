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
static char	*sccsid = "@(#)$RCSfile: rshd.c,v $ $Revision: 4.2.12.4 $ (DEC) $Date: 1993/10/19 20:58:26 $";
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
 * rshd.c
 *
 *	Revision History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

 */
/* 
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1988 The Regents of the University of California.
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
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983, 1988, 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "rshd.c      5.34 (Berkeley) 6/29/90";
#endif  not lint */

/*
 * remote shell server:
 *	[port]\0
 *	remuser\0
 *	locuser\0
 *	command\0
 *	data
 *
 * modification history
 *
 *      19-June-91  walker
 *          fixes to qar's 177 and 178.
 */
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/syslog.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <ctype.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "pathnames.h"

#include <nl_types.h>
#include <locale.h>
#include "rshd_msg.h" 
#include <sia.h>					/* SIA */
#define MSGSTR(Num,Str) catgets(catd,MS_RSHD,Num,Str)
nl_catd catd;           /* message catalog file descriptor */

int	errno;
int	keepalive = 1;
int	check_all = 0;
int 	slth	  = 0;
/*char	*index(), *rindex(), *strncat(), *getenv(), *malloc();*/


#define OPTIONS "aln"

int
#ifndef	_NO_PROTO
error(char *fmt, ...);
#else
error();
#endif

int sia_argc ;						/* SIA */
char **sia_argv ;					/* SIA */
SIAENTITY *entity=NULL ;				/* SIA */

/*ARGSUSED*/
main(argc, argv)
	int argc;
	char **argv;
{
	extern int opterr, optind;
	extern int _check_rhosts_file;
	struct linger linger;
	int ch, on = 1, fromlen;
	struct sockaddr_in from;

	sia_argc = argc ;		/* SIA */
	sia_argv = argv ;		/* SIA */

	setlocale(LC_ALL, "");
	catd = NLcatopen(MF_RSHD, NL_CAT_LOCALE);
        openlog("rshd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	opterr = 0;
	while ((ch = getopt(argc, argv, OPTIONS)) != EOF)
		switch((char)ch) {
		case 'a':
			check_all = 1;
			break;
		case 'l':
			_check_rhosts_file = 0;
			break;
		case 'n':
			keepalive = 0;
			break;
		case '?':
		default:
			syslog(LOG_ERR, MSGSTR(USAGE, "usage: rshd [-lns]"));
			break;
		}

	argc -= optind;
	argv += optind;

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, MSGSTR( PEERNMERR, "rshd: %s: "), argv[0]);
		perror(MSGSTR(PEERNM, "getpeername")); /*MSG*/
		_exit(1);
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
	    sizeof(on)) < 0)
		syslog(LOG_WARNING, MSGSTR(SETSOCKOPT, "setsockopt (SO_KEEPALIVE): %m")); /*MSG*/
	linger.l_onoff = 1;
	linger.l_linger = 60;			/* XXX */
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, (char *)&linger,
	    sizeof (linger)) < 0)
		syslog(LOG_WARNING, MSGSTR(SETSOCKLING, "setsockopt (SO_LINGER): %m")); /*MSG*/
	doit(&from);
}

char	logname[24] = "LOGNAME=";
char    username[20] = "USER=";
char    homedir[64] = "HOME=";
char    shell[64] = "SHELL=";
char    *envinit[] =

            {homedir, shell, _PATH_DEFPATH, username, 0};

char    **environ;

doit(fromp)
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp;
	char locuser[16], remuser[16];
	struct passwd *pwd;
	int s;
	struct hostent *hp;
	char *hostname, *errorstr = NULL, *errorhost;
        u_short port;
	int pv[2], cc;
	pid_t pid;
	int nfd;
	fd_set ready, readfrom;
	char buf[BUFSIZ], sig ;
	char pw_shell[256];
	int one = 1;
	uid_t euid ;
	char remotehost[2 * MAXHOSTNAMELEN + 1];
        int (*sia_collect) () = sia_collect_trm ;/* SIA - collect routine for */
						 /* error messages from sia */
						 /* interface */ 

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef DEBUG
	{ int t = open(_PATH_TTY, 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif DEBUG
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET) {
		syslog(LOG_ERR, MSGSTR(FRMADDRERR, "malformed \"from\" address (af %d)\n"), fromp->sin_family); /*MSG*/
		exit(1);
	}
#ifdef IP_OPTIONS
      {
	u_char optbuf[BUFSIZ/3], *cp;
	char lbuf[BUFSIZ], *lp;
	int optsize = sizeof(optbuf), ipproto;
	struct protoent *ip;

	if ((ip = getprotobyname("ip")) != NULL)
		ipproto = ip->p_proto;
	else
		ipproto = IPPROTO_IP;
	if (!getsockopt(0, ipproto, IP_OPTIONS, (char *)optbuf, &optsize) &&
	    optsize != 0) {
		lp = lbuf;
		for (cp = optbuf; optsize > 0; cp++, optsize--, lp += 3)
			sprintf(lp, " %2.2x", *cp);
		syslog(LOG_NOTICE,MSGSTR(CONN_IP, "Connection from %s received using IP options (ignored):%s"), inet_ntoa(fromp->sin_addr), lbuf);
		if (setsockopt(0, ipproto, IP_OPTIONS,
		    (char *)NULL, &optsize) != 0) {
			syslog(LOG_ERR, MSGSTR(SETSCKIP, "setsockopt IP_OPTIONS NULL: %m"));
			exit(1);
		}
	}
      }
#endif

	if (fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, MSGSTR(ILLEGAL_PORT, "Connection from %s on illegal port"),
			inet_ntoa(fromp->sin_addr));
		exit(1);
	}

	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if ((cc = read(0, &c, 1)) != 1) {
			if (cc < 0)
				syslog(LOG_ERR, MSGSTR(RDERR, "read: %m")); /*MSG*/
			shutdown(0, 1+1);
			exit(1);
		}
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}

	(void) alarm(0);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		s = rresvport(&lport);
		if (s < 0) {
			syslog(LOG_ERR, MSGSTR(STDERR, "can't get stderr port: %m")); /*MSG*/
			exit(1);
		}
		if (port >= IPPORT_RESERVED) {
			syslog(LOG_ERR, MSGSTR(NO2PORT, "2nd port not reserved\n")); /*MSG*/
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0) {
			syslog(LOG_INFO, MSGSTR(NO2CONN, "connect second port: %m")); /*MSG*/
			exit(1);
		}
	}
#ifdef notdef
        /* from inetd, socket is already on 0, 1, 2 */
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
#endif
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		hostname = hp->h_name;
		if (check_all || local_domain(hp->h_name)) {
			strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1);
			remotehost[sizeof(remotehost) - 1] = 0;
#ifdef RES_DNSRCH
			_res.options &= ~RES_DNSRCH;
#endif
			errorhost = remotehost;
			hp = gethostbyname(remotehost);
			if (hp == NULL) {
				syslog(LOG_INFO,
				    MSGSTR(NOADDR, "Couldn't look up address for %s"),
				    remotehost);
				error(MSGSTR(NOADDR2, "Couldn't look up address for your host"));
				 hostname = inet_ntoa(fromp->sin_addr);
			} else for (; ; hp->h_addr_list++) {
				if (hp->h_addr_list[0] == NULL) {
					syslog(LOG_NOTICE,MSGSTR(HOSTADDR1,
					"Host addr %s not listed for host %s"),
					    inet_ntoa(fromp->sin_addr),
					    hp->h_name);
					error(MSGSTR(HOSTADDR2, "Host address mismatch"));
                                        errorstr =
                                            "Host address mismatch for %s\n";
                                        hostname = inet_ntoa(fromp->sin_addr);
                                        break;
				}
				if (!bcmp(hp->h_addr_list[0],
				    (caddr_t)&fromp->sin_addr,
				    sizeof(fromp->sin_addr))) {
					hostname = hp->h_name;
					break;
				}
			}
		}
	} else 
		errorhost = hostname = inet_ntoa(fromp->sin_addr);

	getstr(remuser, sizeof(remuser), "remuser");
	getstr(locuser, sizeof(locuser), "locuser");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
		error(MSGSTR(LOGINERR, "Login incorrect.\n")); /*MSG*/
		exit(1);
	}

	/* SIA - sia  session initialization */

	(void) sprintf(remotehost, "%s@%s", remuser, hostname);
	if ((sia_ses_init(&entity,sia_argc,sia_argv,remotehost,locuser,(char *)NULL,FALSE,(char *)NULL)) != SIASUCCESS) {
	   error(MSGSTR(NOPERMIT, "Permission denied.\n"));
	   sia_ses_release(&entity) ;
	   exit(1) ;
	}

	/* SIA session authentication done if ruserok fails */

	if (ruserok(hostname, pwd->pw_uid == 0, remuser, locuser) < 0) {
           if(sia_ses_authent(sia_collect,NULL,entity) 
		  != SIASUCCESS) {
		error(MSGSTR(NOPERMIT, "Permission denied.\n")); /*MSG*/
	        sia_ses_release(&entity) ;
		exit(1);
	    } 
	}

	/* SIA session establishment */
 
	if (sia_ses_estab(sia_collect,entity) != SIASUCCESS) {
            error(MSGSTR(NOPERMIT, "Permission denied. \n")) ;
            sia_ses_release(&entity) ;
            exit(1) ;
         }

	/* SIA session launch */

	
	if (sia_ses_launch(sia_collect,entity) != SIASUCCESS) {
            error(MSGSTR(NOPERMIT, "Permission denied.\n"));
            sia_ses_release(&entity) ;
            exit(1);
        }

	pwd = entity->pwd ;	/* SIA - get a pointer to passwd entry */

	(void) write(2, "\0", 1);
	if (port) {
		if (pipe(pv) < 0) {
			error(MSGSTR(PIPERR, "Can't make pipe.\n")); /*MSG*/
			sia_ses_release(&entity) ;
			exit(1);
		}
		pid = vfork();
		if (pid == -1)  {
			error(MSGSTR(RETRY, "Try again.\n")); /*MSG*/
			sia_ses_release(&entity) ;
			exit(1);
		}
		if (pid) {
			(void) close(0); (void) close(1); (void) close(2);
			(void) close(pv[1]);
			FD_ZERO(&readfrom);
			FD_SET(s, &readfrom);
			FD_SET(pv[0], &readfrom);
			if (pv[0] > s)
                                nfd = pv[0];
                        else
                                nfd = s;
			ioctl(pv[0], FIONBIO, (char *)&one);
			/* should set s nbio! */
			nfd++;
			do {
				ready = readfrom;
				if (select(nfd, &ready, (fd_set *)0,
				    (fd_set *)0, (struct timeval *)0) < 0)
					break;
				if (FD_ISSET(s, &ready)) {
					int	ret;
					if ((ret = read(s, &sig, 1)) <= 0)
						FD_CLR(s, &readfrom);
					else
						killpg(pid, sig);
				}
				if (FD_ISSET(pv[0], &ready)) {
					errno = 0;
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						shutdown(s, 1+1);
						FD_CLR(pv[0], &readfrom);
					} else
						(void) write(s, buf, cc);
				}
			} while (FD_ISSET(s, &readfrom) ||
			    FD_ISSET(pv[0], &readfrom));
			exit(0);
		}/* if pid */
		setpgid(0, getpid());
		(void) close(s); (void) close(pv[0]);
		dup2(pv[1], 2);
		close(pv[1]);
	} /* if port */
	
        environ = envinit;
	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6);
        strncat(shell, pwd->pw_shell, sizeof(shell)-7);
	strncat(logname, pwd->pw_name, sizeof(logname)-9);
        strncat(username, pwd->pw_name, sizeof(username)-6);

	/* SIA -  save shell before sia session release */
	strncpy(pw_shell,pwd->pw_shell,sizeof(pw_shell)); /* SIA */

        cp = rindex(pw_shell, '/');
        if (cp)
                cp++;
        else
                cp = pw_shell;
	if (pwd->pw_uid == 0) {

	/* Should check the length of the command.  syslog has  */
	/* a restriction of 2K on the message size.  The cmdbuf */
        /* is of size 4K.					*/

	        char tcmdbuf[LINE_MAX]; 
	
		if ( slth > LINE_MAX){
		    memset(tcmdbuf,'\0',LINE_MAX);
	            memcpy(tcmdbuf,cmdbuf,LINE_MAX);
	            syslog(LOG_INFO|LOG_AUTH, MSGSTR(ROOT_SHELL,
                    "ROOT shell from %s@%s, comm: %s\n"),
                        remuser, hostname, tcmdbuf); 
	        }else		
                    syslog(LOG_INFO|LOG_AUTH, MSGSTR(ROOT_SHELL, 
		    "ROOT shell from %s@%s, comm: %s\n"),
                	remuser, hostname, cmdbuf);
	}

	
	sia_ses_release(&entity) ;		/* SIA */

	if ((euid = geteuid()) < 0) {		/* SIA */
	 syslog(LOG_ERR, "Failed to geteuid\n") ;
	 exit(1) ;
	}

	/* SIA - set real and effective user id */

	if((setreuid(euid,euid)) < 0) {		
         syslog(LOG_ERR, "Failed to seteuid\n") ;
	 exit(1) ;
        } 
        /* if user's shell is empty, assign the default shell */
	if (pw_shell[0] == NULL)
	    strcpy(pw_shell,"/bin/sh");
        execl(pw_shell, cp, "-c", cmdbuf, (long)0);
	sprintf(buf,"rshd, %s",pw_shell);
	perror(buf);
	exit(1);
}

/*
 * Report error to client.
 * Note: can't be used until second socket has connected
 * to client, or older clients will hang waiting
 * for that connection first.
 */
/*VARARGS1*/
int
#ifndef	_NO_PROTO
error(char *fmt, ...)
#else
error(fmt, va_alist)
char *fmt;
va_dcl
#endif
{
	va_list ap;
        char buf[BUFSIZ], *bp = &buf[1];

#ifdef __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif /* __STDC__ */
	(void) vsprintf(bp, fmt, ap);
	buf[0] = 1;
        (void) write(2, buf, strlen(buf));
        va_end(ap);
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;
	
	slth = 0;
	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		slth++;
		if (--cnt == 0) {
			error(MSGSTR(TOOLONG, "%s too long\n"), err); /*MSG*/
			exit(1);
		}
	} while (c != 0);
}

/*
 * Check whether host h is in our local domain,
 * as determined by the part of the name following
 * the first '.' in its name and in ours.
 * If either name is unqualified (contains no '.'),
 * assume that the host is local, as it will be
 * interpreted as such.
 */
local_domain(h)
	char *h;
{
	char localhost[MAXHOSTNAMELEN];
	char *p1, *p2, *topdomain();

	localhost[0] = 0;
        (void) gethostname(localhost, sizeof(localhost));
        p1 = topdomain(localhost);
        p2 = topdomain(h);
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2))
		return(1);
	return(0);
}
char *
topdomain(h)
        char *h;
{
        register char *p;
        char *maybe = NULL;
        int dots = 0;

	for (p = h + strlen(h); p >= h; p--) {
                if (*p == '.') {
                        if (++dots == 2)
                                return (p);
                        maybe = p;
                }
        }
	 return (maybe);
}
