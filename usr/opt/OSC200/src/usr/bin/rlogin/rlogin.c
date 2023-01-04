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
static char	*sccsid = "@(#)$RCSfile: rlogin.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/11 17:42:52 $";
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
 * rlogin.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
/*
 * Copyright (c) 1983, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"Copyright (c) 1983, 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */


#include <nl_types.h>
#include <locale.h>
#include "rlogin_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_RLOGIN,Num,Str)
nl_catd catd;           /* message catalog file descriptor */

/*
 * rlogin - remote login
 */
#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <sgtty.h>
#include <setjmp.h>
#include <errno.h>
#include <varargs.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#ifndef TIOCPKT_WINDOW
#define	TIOCPKT_WINDOW	0x80
#endif

/* concession to Sun */
#ifndef SIGUSR1
#define	SIGUSR1	30
#endif

extern int errno;
int eight, litout, rem;
char cmdchar;
char *speeds[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600", "1200",
	"1800", "2400", "4800", "9600", "19200", "38400"
};

#ifdef sun
struct winsize {
	unsigned short ws_row, ws_col;
	unsigned short ws_xpixel, ws_ypixel;
};
#endif
struct	winsize winsize;

#ifndef sun
#define	get_window_size(fd, wp)	ioctl(fd, TIOCGWINSZ, wp)
#endif

void exit();
void sigwinch();

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	struct passwd *pw;
	struct servent *sp;
	struct sgttyb ttyb;
	int omask;
	int argoff, ch, dflag, one;
	uid_t uid;
	char *host, *p, *user, term[1024];
	void lostpeer();
	void copytochild();
	char *getenv();

        setlocale(LC_ALL, "");
	catd = NLcatopen(MF_RLOGIN, NL_CAT_LOCALE);

	argoff = dflag = 0;
	one = 1;
	host = user = NULL;
	cmdchar = '~';

	if (p = rindex(argv[0], '/'))
		++p;
	else
		p = argv[0];

	if (strcmp(p, "rlogin"))
		host = p;

	/* handle "rlogin host flags" */
	if (!host && argc > 2 && argv[1][0] != '-') {
		host = argv[1];
		argoff = 1;
	}

#define	OPTIONS	"8KLde:l:"
	while ((ch = getopt(argc - argoff, argv + argoff, OPTIONS)) != EOF)
		switch(ch) {
		case '8':
			eight = 1;
			break;
		case 'K':
			break;
		case 'L':
			litout = 1;
			break;
		case 'd':
			dflag = 1;
			break;
		case 'e':
			cmdchar = optarg[0];
			break;
		case 'l':
			user = optarg;
			break;
		case '?':
		default:
			usage();
		}
	optind += argoff;
	argc -= optind;
	argv += optind;

	/* if haven't gotten a host yet, do so */
	if (!host && !(host = *argv++))
		usage();

	if (*argv)
		usage();

	if (!(pw = getpwuid(uid = getuid()))) {
		(void)fprintf(stderr, MSGSTR( RLOG_UNK_UID, 
			"rlogin: unknown user id.\n"));
		exit(1);
	}
	if (!user)
		user = pw->pw_name;

	sp = NULL;
	if (sp == NULL)
		sp = getservbyname("login", "tcp");
	if (sp == NULL) {
		(void)fprintf(stderr, MSGSTR( RLOG_UNK_SERV,
			"rlogin: login/tcp: unknown service.\n"));
		exit(1);
	}

	(void)strcpy(term, (p = getenv("TERM")) ? p : "network");
	if (ioctl(0, TIOCGETP, &ttyb) == 0) {
		(void)strcat(term, "/");
		(void)strcat(term, speeds[ttyb.sg_ospeed]);
	}

	(void)get_window_size(0, &winsize);

	(void)signal(SIGPIPE, lostpeer);
	/* 
         * Must change signal action before blocking 
         * for SIGURG since the default action is to
         * ignore and ignored signals can't be blocked...
	 * will use SIGUSR1 for window size hack, so hold it off 
         */
	(void)signal(SIGURG, copytochild);
	omask = sigblock(sigmask(SIGURG) | sigmask(SIGUSR1));

	rem = rcmd(&host, sp->s_port, pw->pw_name, user, term, (long)0);

	if (rem < 0)
		exit(1);

	if (dflag &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one, sizeof(one)) < 0)
		(void)fprintf(stderr, MSGSTR( RLOG_SETSOCK, 
			"rlogin: setsockopt: %s.\n"), strerror(errno));
#if  defined(IP_TOS)  && defined(notyet)
	one = IPTOS_LOWDELAY;
	if (setsockopt(rem, IPPROTO_IP, IP_TOS, (char *)&one, sizeof(int)) < 0)
		perror("rlogin: setsockopt TOS (ignored)");
#endif

	(void)setuid(uid);
	doit(omask);
	/*NOTREACHED*/
}

int child = -1, defflags, deflflags, tabflag;
char deferase, defkill;
struct tchars deftc;
struct ltchars defltc;
struct tchars notc = { -1, -1, -1, -1, -1, -1 };
struct ltchars noltc = { -1, -1, -1, -1, -1, -1 };

doit(omask)
	int omask;
{
	struct sgttyb sb;
	void catch_child(), writeroob();

	(void)ioctl(0, TIOCGETP, (char *)&sb);
	defflags = sb.sg_flags;
	tabflag = defflags & TBDELAY;
	defflags &= ECHO | CRMOD;
	deferase = sb.sg_erase;
	defkill = sb.sg_kill;
	(void)ioctl(0, TIOCLGET, (char *)&deflflags);
	(void)ioctl(0, TIOCGETC, (char *)&deftc);
	notc.t_startc = deftc.t_startc;
	notc.t_stopc = deftc.t_stopc;
	(void)ioctl(0, TIOCGLTC, (char *)&defltc);
	(void)signal(SIGINT, SIG_IGN);
	setsignal(SIGHUP, exit);
	setsignal(SIGQUIT, exit);
	(void)signal(SIGUSR1, writeroob);
	child = fork();
	if (child == -1) {
		(void)fprintf(stderr, MSGSTR( RLOG_FORK, 
			"rlogin: fork: %s.\n"), strerror(errno));
		done(1);
	}
	if (child == 0) {
		mode(1);
		if (reader(omask) == 0) {
			msg("connection closed.");
			exit(0);
		}
		sleep(1);
		msg("\007connection closed.");
		exit(1);
	}

	(void)sigsetmask(omask);
	(void)signal(SIGCHLD, catch_child);
	writer();
	msg("closed connection.");
	done(0);
}

/* trap a signal, unless it is being ignored. */
setsignal(sig, act)
	int sig;
	void (*act)();
{
	int omask = sigblock(sigmask(sig));

	if (signal(sig, act) == SIG_IGN)
		(void)signal(sig, SIG_IGN);
	(void)sigsetmask(omask);
}

done(status)
	int status;
{
	int w;

	mode(0);
	if (child > 0) {
		/* make sure catch_child does not snap it up */
		(void)signal(SIGCHLD, SIG_DFL);
		if (kill(child, SIGKILL) >= 0)
			while ((w = wait((union wait *)0)) > 0 && w != child);
	}
	exit(status);
}

int dosigwinch;

/*
 * This is called when the reader process gets the out-of-band (urgent)
 * request to turn on the window-changing protocol.
 */
void
writeroob()
{
	void sigwinch();

	if (dosigwinch == 0) {
		sendwindow();
		(void)signal(SIGWINCH, sigwinch);
	}
	dosigwinch = 1;
}

void
catch_child()
{
	union wait status;
	pid_t pid;

	for (;;) {
		pid = wait3(&status, WNOHANG|WUNTRACED, (struct rusage *)0);
		if (pid == 0)
			return;
		/* if the child (reader) dies, just quit */
		if (pid < 0 || pid == child && !WIFSTOPPED(status))
			done((int)(status.w_termsig | status.w_retcode));
	}
	/* NOTREACHED */
}

/*
 * writer: write to remote: 0 -> line.
 * ~.	terminate
 * ~^Z	suspend rlogin process.
 * ~^Y  suspend rlogin process, but leave reader alone.
 */
writer()
{
	char c;
	register int bol, local, n;

	bol = 1;			/* beginning of line */
	local = 0;
	for (;;) {
		n = read(STDIN_FILENO, &c, 1);
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}
		/*
		 * If we're at the beginning of the line and recognize a
		 * command character, then we echo locally.  Otherwise,
		 * characters are echo'd remotely.  If the command character
		 * is doubled, this acts as a force and local echo is
		 * suppressed.
		 */
		if (bol) {
			bol = 0;
			if (c == cmdchar) {
				bol = 0;
				local = 1;
				continue;
			}
		} else if (local) {
			local = 0;
			if (c == '.' || c == deftc.t_eofc) {
				echo(c);
				break;
			}
			if (c == defltc.t_suspc || c == defltc.t_dsuspc) {
				bol = 1;
				echo(c);
				stop(c);
				continue;
			}
			if (c != cmdchar) {
					(void)write(rem, &cmdchar, 1);
			}
		}

			if (write(rem, &c, 1) == 0) {
				msg("line gone");
				break;
			}
		bol = c == defkill || c == deftc.t_eofc ||
		    c == deftc.t_intrc || c == defltc.t_suspc ||
		    c == '\r' || c == '\n';
	}
}

echo(c)
register char c;
{
	register char *p;
	char buf[8];

	p = buf;
	c &= 0177;
	*p++ = cmdchar;
	if (c < ' ') {
		*p++ = '^';
		*p++ = c + '@';
	} else if (c == 0177) {
		*p++ = '^';
		*p++ = '?';
	} else
		*p++ = c;
	*p++ = '\r';
	*p++ = '\n';
	(void)write(1, buf, p - buf);
}

stop(cmdc)
	char cmdc;
{
	void catch_child();

	mode(0);
	(void)signal(SIGCHLD, SIG_IGN);
	(void)kill(cmdc == defltc.t_suspc ? 0 : getpid(), SIGTSTP);
	(void)signal(SIGCHLD, catch_child);
	mode(1);
	sigwinch();			/* check for size changes */
}

void
sigwinch()
{
	struct winsize ws;

	if (dosigwinch && get_window_size(0, &ws) == 0 &&
	    bcmp(&ws, &winsize, sizeof(ws))) {
		winsize = ws;
		sendwindow();
	}
}

/*
 * Send the window size to the server via the magic escape
 */
sendwindow()
{
	struct winsize *wp;
	char obuf[4 + sizeof (struct winsize)];

	wp = (struct winsize *)(obuf+4);
	obuf[0] = 0377;
	obuf[1] = 0377;
	obuf[2] = 's';
	obuf[3] = 's';
	wp->ws_row = htons(winsize.ws_row);
	wp->ws_col = htons(winsize.ws_col);
	wp->ws_xpixel = htons(winsize.ws_xpixel);
	wp->ws_ypixel = htons(winsize.ws_ypixel);

		(void)write(rem, obuf, sizeof(obuf));
}

/*
 * reader: read from remote: line -> 1
 */
#define	READING	1
#define	WRITING	2

jmp_buf rcvtop;
int rcvcnt, rcvstate;
pid_t ppid;
char rcvbuf[8 * 1024];

void
oob()
{
	struct sgttyb sb;
	int atmark, n, out, rcvd;
	char waste[BUFSIZ], mark;

	out = O_RDWR;
	rcvd = 0;
	while (recv(rem, &mark, 1, MSG_OOB) < 0) {
		switch (errno) {
		case EWOULDBLOCK:
			/*
			 * Urgent data not here yet.  It may not be possible
			 * to send it yet if we are blocked for output and
			 * our input buffer is full.
			 */
			if (rcvcnt < sizeof(rcvbuf)) {
				n = read(rem, rcvbuf + rcvcnt,
				    sizeof(rcvbuf) - rcvcnt);
				if (n <= 0)
					return;
				rcvd += n;
			} else {
				n = read(rem, waste, sizeof(waste));
				if (n <= 0)
					return;
			}
			continue;
		default:
			return;
	      }
	}
	if (mark & TIOCPKT_WINDOW) {
		/* Let server know about window size changes */
		(void)kill(ppid, SIGUSR1);
	}
	if (!eight && (mark & TIOCPKT_NOSTOP)) {
		(void)ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~CBREAK;
		sb.sg_flags |= RAW;
		(void)ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = -1;
		notc.t_startc = -1;
		(void)ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
		(void)ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~RAW;
		sb.sg_flags |= CBREAK;
		(void)ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = deftc.t_stopc;
		notc.t_startc = deftc.t_startc;
		(void)ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (mark & TIOCPKT_FLUSHWRITE) {
		(void)ioctl(1, TIOCFLUSH, (char *)&out);
		for (;;) {
			if (ioctl(rem, SIOCATMARK, &atmark) < 0) {
				(void)fprintf(stderr, MSGSTR( RLOG_IOCTL,
					"rlogin: ioctl: %s.\n"), strerror(errno));
				break;
			}
			if (atmark)
				break;
			n = read(rem, waste, sizeof (waste));
			if (n <= 0)
				break;
		}
		/*
		 * Don't want any pending data to be output, so clear the recv
		 * buffer.  If we were hanging on a write when interrupted,
		 * don't want it to restart.  If we were reading, restart
		 * anyway.
		 */
		rcvcnt = 0;
		longjmp(rcvtop, 1);
	}

	/* oob does not do FLUSHREAD (alas!) */

	/*
	 * If we filled the receive buffer while a read was pending, longjmp
	 * to the top to restart appropriately.  Don't abort a pending write,
	 * however, or we won't know how much was written.
	 */
	if (rcvd && rcvstate == READING) {
		longjmp(rcvtop, 1);
	}
}

/* reader: read from remote: line -> 1 */
reader(omask)
	int omask;
{
	void oob();

#if !defined(BSD) || BSD < 43
	pid_t pid = -getpid();
#else
	pid_t pid = getpid();
#endif
	int n, remaining;
	char *bufp = rcvbuf;

	(void)signal(SIGTTOU, SIG_IGN);
	(void)signal(SIGURG, oob); 
	ppid = getppid();
	(void)fcntl(rem, F_SETOWN, pid); 
	(void)setjmp(rcvtop);
	(void)sigsetmask(omask);
	for (;;) {
		while ((remaining = rcvcnt - (bufp - rcvbuf)) > 0) {
			rcvstate = WRITING;
			n = write(1, bufp, remaining);
			if (n < 0) {
				if (errno != EINTR)
					return(-1);
				continue;
			}
			bufp += n;
		}
		bufp = rcvbuf;
		rcvcnt = 0;
		rcvstate = READING;

			rcvcnt = read(rem, rcvbuf, sizeof (rcvbuf));
		if (rcvcnt == 0) {
			return (0);
		}
		if (rcvcnt < 0) {
			if (errno == EINTR)
				continue;
			(void)fprintf(stderr, MSGSTR( RLOG_READ,
				"rlogin: read: %s.\n"), strerror(errno));
			return(-1);
		}
	}
}

mode(f)
{
	struct ltchars *ltc;
	struct sgttyb sb;
	struct tchars *tc;
	int lflags;

	(void)ioctl(0, TIOCGETP, (char *)&sb);
	(void)ioctl(0, TIOCLGET, (char *)&lflags);
	switch(f) {
	case 0:
		sb.sg_flags &= ~(CBREAK|RAW|TBDELAY);
		sb.sg_flags |= defflags|tabflag;
		tc = &deftc;
		ltc = &defltc;
		sb.sg_kill = defkill;
		sb.sg_erase = deferase;
		lflags = deflflags;
		break;
	case 1:
		sb.sg_flags |= (eight ? RAW : CBREAK);
		sb.sg_flags &= ~defflags;
		/* preserve tab delays, but turn off XTABS */
		if ((sb.sg_flags & TBDELAY) == XTABS)
			sb.sg_flags &= ~TBDELAY;
		tc = &notc;
		ltc = &noltc;
		sb.sg_kill = sb.sg_erase = -1;
		if (litout)
			lflags |= LLITOUT;
		break;
	default:
		return;
	}
	(void)ioctl(0, TIOCSLTC, (char *)ltc);
	(void)ioctl(0, TIOCSETC, (char *)tc);
	(void)ioctl(0, TIOCSETN, (char *)&sb);
	(void)ioctl(0, TIOCLSET, (char *)&lflags);
}

void
lostpeer()
{
	(void)signal(SIGPIPE, SIG_IGN);
	msg("\007connection closed.");
	done(1);
}

/* copy SIGURGs to the child process. */
void
copytochild()
{
	if (child == -1) 
		return;
	(void)kill(child, SIGURG);
}

msg(str)
	char *str;
{
	(void)fprintf(stderr, MSGSTR( RLOG_MSG, "rlogin: %s\r\n"), str);
}


usage()
{
	(void)fprintf(stderr, MSGSTR( RLOG_USAGE,
	    "usage: rlogin [ -%s]%s[-e char] [ -l username ] host\n"),
	    "8L", " ");
	exit(1);
}

/*
 * The following routine provides compatibility (such as it is) between 4.2BSD
 * Suns and others.  Suns have only a `ttysize', so we convert it to a winsize.
 */
#ifdef sun
int
get_window_size(fd, wp)
	int fd;
	struct winsize *wp;
{
	struct ttysize ts;
	int error;

	if ((error = ioctl(0, TIOCGSIZE, &ts)) != 0)
		return(error);
	wp->ws_row = ts.ts_lines;
	wp->ws_col = ts.ts_cols;
	wp->ws_xpixel = 0;
	wp->ws_ypixel = 0;
	return(0);
}
#endif
