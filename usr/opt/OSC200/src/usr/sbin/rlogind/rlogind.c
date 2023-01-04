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
static char	*sccsid = "@(#)$RCSfile: rlogind.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/11/19 16:29:02 $";
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

#endif /* not lint */

#include <nl_types.h>
#include <locale.h>
#include "rlogind_msg.h"
#define MSGSTR(n,s) catgets(catd, MS_RLOGIND, n, s)
nl_catd catd;

/*
 * remote login server:
 *	\0
 *	remuser\0
 *	locuser\0
 *	terminal_type/speed\0
 *	data
 */

/* #define FD_SETSIZE      16               don't need many bits for select */
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#define TTYDEFCHARS
#include <sys/termios.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <errno.h>
#include <pwd.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <paths.h>


#ifndef TIOCPKT_WINDOW
#define TIOCPKT_WINDOW 0x80
#endif

#define         ARGSTR                  "aln"

char	*env[2];
#define	NMAX 30
char	lusername[NMAX+1], rusername[NMAX+1];
static	char term[64] = "TERM=";
#define	ENVSIZE	(sizeof("TERM=")-1)	/* skip null for concatenation */
int	keepalive = 1;
int	check_all = 0;

extern	int errno;
int	reapchild();
struct	passwd *getpwnam(), *pwd;
char	*malloc();

#ifdef SEC_BASE
struct pr_passwd *ppr;
#endif
char *progName;

main(argc, argv)
	int argc;
	char **argv;
{
	extern int opterr, optind;
	extern int  _check_rhosts_file;
	int ch;
	int on = 1, fromlen;
	struct sockaddr_in from;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif
	progName = argv[0];
	
	setlocale(LC_ALL, "");
	catd =  NLcatopen(MF_RLOGIND, NL_CAT_LOCALE);
	openlog("rlogind", LOG_PID | LOG_CONS, LOG_AUTH);

	opterr = 0;
	while ((ch = getopt(argc, argv, ARGSTR)) != EOF)
		switch (ch) {
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
			syslog(LOG_ERR, "usage: rlogind [-l] [-n] [-s]");
			break;
		}
	argc -= optind;
	argv += optind;

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		syslog(LOG_ERR,  MSGSTR(PEERNM, "Couldn't get peer name of remote host: %m %d"), errno); /*MSG*/
		fatal(0, MSGSTR(PEERNMERR, "getpeername"), 1); /*MSG*/
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING, MSGSTR(SETSOCKOPT, "setsockopt (SO_KEEPALIVE): %m")); /*MSG*/

	on = IPTOS_LOWDELAY;
        if (setsockopt(0, IPPROTO_IP, IP_TOS, (char *)&on, sizeof(int)) < 0)
                syslog(LOG_WARNING, "setsockopt (IP_TOS): %m");
	doit(0, &from);
}

int	child;
void	cleanup();
int	netf;
char    line[MAXPATHLEN];
int     confirmed;
extern	char	*inet_ntoa();

struct winsize win = { 0, 0, 0, 0 };

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	int i, master, on = 1;
	pid_t pid;
        int authenticated = 0, hostok = 0;
        register struct hostent *hp;
        char remotehost[2 * MAXHOSTNAMELEN + 1];
        struct hostent hostent;
        char c;

	alarm(60);
	read(f, &c, 1);

	if (c != 0)
		exit(1);

	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		/*
		 * Only the name is used below.
		 */
		hp = &hostent;
		hp->h_name = inet_ntoa(fromp->sin_addr);
		hostok++;
	}
	else if (check_all || local_domain(hp->h_name)) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		strncpy(remotehost, hp->h_name, sizeof(remotehost) - 1);
		remotehost[sizeof(remotehost) - 1] = 0;
		hp = gethostbyname(remotehost);
		if (hp)
		    for (; hp->h_addr_list[0]; hp->h_addr_list++)
			if (!bcmp(hp->h_addr_list[0], (caddr_t)&fromp->sin_addr,
			    sizeof(fromp->sin_addr))) {
				hostok++;
				break;
			}
	} else
		hostok++;

	{
	    if (fromp->sin_family != AF_INET ||
	   	fromp->sin_port >= IPPORT_RESERVED ||
	        fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, MSGSTR(ILLEGALPORT,"Connection from %s on illegal port"),
			inet_ntoa(fromp->sin_addr)); /*MSG*/
		fatal(f, MSGSTR(NOPERMIT,"Permission denied"), 0); /*MSG*/
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
	    if (getsockopt(0, ipproto, IP_OPTIONS, (char *)optbuf,
		&optsize) == 0 && optsize != 0) {
		    lp = lbuf;
		    for (cp = optbuf; optsize > 0; cp++, optsize--, lp += 3)
			    sprintf(lp, " %2.2x", *cp);
		    syslog(LOG_NOTICE, MSGSTR(INVIPOPT,
		        "Connection received using IP options (ignored):%s"), lbuf); /*MSG*/
		    if (setsockopt(0, ipproto, IP_OPTIONS,
		    (char *)NULL, &optsize) != 0) {
			syslog(LOG_ERR, MSGSTR(NULLIPOPT, "setsockopt IP_OPTIONS NULL: %m")); /*MSG*/
			exit(1);
		    }
	    	}
      	    }
#endif
	    if (do_rlogin(hp->h_name) == 0 && hostok) 
		    authenticated++;
	    if (confirmed == 0) {
                write(f, "", 1);
                confirmed = 1;          /* we sent the null! */
            }
		if (!authenticated && !hostok)
		    write(f, MSGSTR(ADMISMATCH, "rlogind: Host address mismatch.\r\n"), NLstrlen(MSGSTR(ADMISMATCH, "rlogind: Host address mismatch.\r\n")) - 1); /*MSG*/
	}

#ifdef SEC_BASE
	/* must put this code AFTER the above call to do_rlogin() */

	ppr = getprpwnam(lusername);
	audit_rcmd(ppr, pwd, term+ENVSIZE, (char *) 0, hp->h_name, rusername,
		   "rlogin", progName, (char *) 0);
#endif
	netf = f;

	pid = forkpty(&master, line, NULL, &win);
        if (pid < 0) {
                if (errno == ENOENT)
                        fatal(f, "Out of ptys", 0);
                else
                        fatal(f, "Forkpty", 1);
        }
	if (pid == 0) {
		char remid[MAXHOSTNAMELEN+NMAX+2]; /* for audit info */
		(void) sprintf(remid, "%s@%s", rusername, hp->h_name);
                if (f > 2)      /* f should always be 0, but... */
                        (void) close(f);
                setup_term(0);
                if (authenticated) {

                        execl(_PATH_LOGIN, "login", "-p",
                            "-h", remid, "-f", lusername, (char *)0);
                } else
                        execl(_PATH_LOGIN, "login", "-p",
                            "-h", remid, lusername, (char *)0);
                fatal(STDERR_FILENO, _PATH_LOGIN, 1);
                /*NOTREACHED*/
        }
		ioctl(f, FIONBIO, &on);
        ioctl(master, FIONBIO, &on);
        ioctl(master, TIOCPKT, &on);
        signal(SIGCHLD, cleanup);
        protocol(f, master);
        signal(SIGCHLD, SIG_IGN);
        cleanup();
}

char	magic[2] = { 0377, 0377 };
char	oobdata[] = {TIOCPKT_WINDOW};

/*
 * Handle a "control" request (signaled by magic being present)
 * in the data stream.  For now, we are only willing to handle
 * window size changes.
 */
control(pty, cp, n)
	int pty;
	char *cp;
	int n;
{
	struct winsize w;

	if (n < 4+sizeof (w) || cp[2] != 's' || cp[3] != 's')
		return (0);
	oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
	bcopy(cp+4, (char *)&w, sizeof(w));
	w.ws_row = ntohs(w.ws_row);
	w.ws_col = ntohs(w.ws_col);
	w.ws_xpixel = ntohs(w.ws_xpixel);
	w.ws_ypixel = ntohs(w.ws_ypixel);
	(void) ioctl(pty, TIOCSWINSZ, &w);
	return (4+sizeof (w));
}

/*
 * rlogin "protocol" machine.
 */
protocol(f, p)
	register int f, p;
{
	char pibuf[1024], fibuf[1024], *pbp, *fbp;
	register pcc = 0, fcc = 0;
	int cc, nfd, n;
	char cntl;

	/*
	 * Must ignore SIGTTOU, otherwise we'll stop
	 * when we try and set slave pty's window shape
	 * (our controlling tty is the master pty).
	 */
	(void) signal(SIGTTOU, SIG_IGN);
	/* delay TIOCPKT_WINDOW oobdata, for backward compatibility */
	sleep(1);
	send(f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
	if (f > p)
		nfd = f + 1;
	else
		nfd = p + 1;
	if (nfd > FD_SETSIZE) {
                syslog(LOG_ERR, "select mask too small, increase FD_SETSIZE");
                fatal(f, "internal error (select mask too small)", 0);
        }
	for (;;) {
		fd_set ibits, obits, ebits, *omask;

		FD_ZERO(&ebits);
                FD_ZERO(&ibits);
                FD_ZERO(&obits);
                omask = (fd_set *)NULL;
                if (fcc) {
                        FD_SET(p, &obits);
                        omask = &obits;
                } else
                        FD_SET(f, &ibits);
                if (pcc >= 0)
			if (pcc) {
                                FD_SET(f, &obits);
                                omask = &obits;
                        } else
                                FD_SET(p, &ibits);
                FD_SET(p, &ebits);
                if ((n = select(nfd, &ibits, omask, &ebits, (struct timeval *)0)) < 0) {
                        if (errno == EINTR)
				continue;
                        fatal(f, "select", 1);
                }
                if (n == 0) {
                        /* shouldn't happen... */
                        sleep(5);
                        continue;
		}
#define	pkcontrol(c)	((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))
		if (FD_ISSET(p, &ebits)) {
			cc = read(p, &cntl, 1);
			if (cc == 1 && pkcontrol(cntl)) {
				cntl |= oobdata[0];
				send(f, &cntl, 1, MSG_OOB);
				if (cntl & TIOCPKT_FLUSHWRITE) {
					pcc = 0;
					FD_CLR(p, &ibits);
				}
			}
		}
		if (FD_ISSET(f, &ibits)) {
			fcc = read(f, fibuf, sizeof(fibuf));
			if (fcc < 0 && errno == EWOULDBLOCK)
				fcc = 0;
			else {
				register char *cp;
				int left, n;

				if (fcc <= 0)
					break;
				fbp = fibuf;

			top:
				for (cp = fibuf; cp < fibuf+fcc-1; cp++)
					if (cp[0] == magic[0] &&
					    cp[1] == magic[1]) {
						left = fcc - (cp-fibuf);
						n = control(p, cp, left);
						if (n) {
							left -= n;
							if (left > 0)
								bcopy(cp+n, cp, left);
							fcc -= n;
							goto top; /* n^2 */
						}
					}
				 FD_SET(p, &obits);		/* try write */
			}
		}

		if (FD_ISSET(p, &obits) && fcc > 0) {
			cc = write(p, fbp, fcc);
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}

		if (FD_ISSET(p, &ibits)) {
			pcc = read(p, pibuf, sizeof (pibuf));
			pbp = pibuf;
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else if (pcc <= 0)
				break;
			else if (pibuf[0] == 0) {
				pbp++, pcc--;
				FD_SET(f, &obits);	/* try a write */
			} else {
				if (pkcontrol(pibuf[0])) {
					pibuf[0] |= oobdata[0];
					send(f, &pibuf[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
		}
		if ((FD_ISSET(f, &obits)) && pcc > 0) {
			cc = write(f, pbp, pcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/*
                                 * This happens when we try write after read
                                 * from p, but some old kernels balk at large
                                 * writes even when select returns true.
                                 */
				if (!FD_ISSET(p, &ibits))
					sleep(5);
				continue;
			}
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
	}
}

void
cleanup()
{
	char *p;

        p = line + sizeof(_PATH_DEV) - 1;
        if (logout(p))
                logwtmp(p, "", "");
        (void)chmod(line, 0666);
        (void)chown(line, 0, 0);
        *p = 'p';
        (void)chmod(line, 0666);
        (void)chown(line, 0, 0);
	shutdown(netf, 2);
	exit(1);
}

fatal(f, msg, syserr)
	int f, syserr;
	char *msg;
{
	int len;
	char buf[BUFSIZ], *bp = buf;
	
	/*
         * Prepend binary one to message if we haven't sent
         * the magic null as confirmation.
         */
        if (!confirmed)
                *bp++ = '\01';          /* error indicator */
        if (syserr)
                len = (int)sprintf(bp, MSGSTR(FATAL, "rlogind: %s: %s.\r\n"),
                    msg, strerror(errno));
        else
		len = (int)sprintf(bp,  MSGSTR(FATAL2, "%s: Error %d"), msg, errno);
        (void) write(f, buf, bp + len - buf);
        exit(1);
}

do_rlogin(host)
	char *host;
{
	getstr(rusername, sizeof(rusername), MSGSTR(REMUSER, "remuser too long")); /*MSG*/
	getstr(lusername, sizeof(lusername), MSGSTR(LOCUSER, "locuser too long")); /*MSG*/
	getstr(term+ENVSIZE, sizeof(term)-ENVSIZE, MSGSTR(TTTOOLONG, "Terminal type too long")); /*MSG*/

	pwd = getpwnam(lusername);
	if (pwd == NULL)
                return(-1);
        if (pwd->pw_uid == 0) {
		if (!(getttynam("ptys")))
                	return(-1);
	}
        return(ruserok(host, pwd->pw_uid == 0 , rusername, lusername));

}


getstr(buf, cnt, errmsg)
	char *buf;
	int cnt;
	char *errmsg;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0)
			fatal(1, errmsg, 0);
		*buf++ = c;
	} while (c != 0);
}

extern	char **environ;

setup_term(fd)
	int fd;
{
	register char *cp = index(term+ENVSIZE, '/');
	char *speed;
	struct termios tt;

        tcgetattr(fd, &tt);
        if (cp) {
                *cp++ = '\0';
                speed = cp;
                cp = index(speed, '/');
                if (cp)
                        *cp++ = '\0';
                cfsetspeed(&tt, atoi(speed));
        }

        tt.c_iflag = TTYDEF_IFLAG;
        tt.c_oflag = TTYDEF_OFLAG;
        tt.c_lflag = TTYDEF_LFLAG;
	bcopy(ttydefchars, tt.c_cc, sizeof(tt.c_cc));
        tcsetattr(fd, TCSAFLUSH, &tt);

	env[0] = term;
	env[1] = 0;
	environ = env;
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
