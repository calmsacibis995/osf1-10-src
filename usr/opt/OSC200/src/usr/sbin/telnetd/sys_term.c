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
static char	*sccsid = "@(#)$RCSfile: sys_term.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1994/01/05 18:40:29 $";
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
 * Copyright (c) 1989 Regents of the University of California.
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

#endif /* not lint */

#include "telnetd.h"
#include "pathnames.h"

#define SCPYN(a, b)	(void) strncpy(a, b, sizeof(a))
#define SCMPN(a, b)	strncmp(a, b, sizeof(a))

#ifdef	STREAMS
#include <sys/stream.h>
#endif
#include <sys/tty.h>
#ifdef	t_erase
#undef	t_erase
#undef	t_kill
#undef	t_intrc
#undef	t_quitc
#undef	t_startc
#undef	t_stopc
#undef	t_eofc
#undef	t_brkc
#undef	t_suspc
#undef	t_dsuspc
#undef	t_rprntc
#undef	t_flushc
#undef	t_werasc
#undef	t_lnextc
#endif

# ifndef EXTPROC
# define EXTPROC 0400
# endif
struct termios termbuf, termbuf2;       /* pty control structure */
char *line = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

/*
 * init_termbuf()
 * copy_termbuf(cp)
 * set_termbuf()
 *
 * These three routines are used to get and set the "termbuf" structure
 * to and from the kernel.  init_termbuf() gets the current settings.
 * copy_termbuf() hands in a new "termbuf" to write to the kernel, and
 * set_termbuf() writes the structure into the kernel.
 */

init_termbuf()
{
	(void) tcgetattr(pty, &termbuf);
	termbuf2 = termbuf;
}

#if	defined(LINEMODE) && defined(TIOCPKT_IOCTL)
copy_termbuf(cp, len)
char *cp;
int len;
{
	if (len > sizeof(termbuf))
		len = sizeof(termbuf);
	bcopy(cp, (char *)&termbuf, len);
	termbuf2 = termbuf;
}
#endif	/* defined(LINEMODE) && defined(TIOCPKT_IOCTL) */

set_termbuf()
{
	/*
	 * Only make the necessary changes.
	 */
	if (bcmp((char *)&termbuf, (char *)&termbuf2, sizeof(termbuf)))
		(void) tcsetattr(pty, TCSANOW, &termbuf);
}


/*
 * spcset(func, valp, valpp)
 *
 * This function takes various special characters (func), and
 * sets *valp to the current value of that character, and
 * *valpp to point to where in the "termbuf" structure that
 * value is kept.
 *
 * It returns the SLC_ level of support for this function.
 */


spcset(func, valp, valpp)
int func;
cc_t *valp;
cc_t **valpp;
{

#define	setval(a, b)	*valp = termbuf.c_cc[a]; \
			*valpp = &termbuf.c_cc[a]; \
			return(b);
#define	defval(a) *valp = ((cc_t)a); *valpp = (cc_t *)0; return(SLC_DEFAULT);

	switch(func) {
	case SLC_EOF:
		setval(VEOF, SLC_VARIABLE);
	case SLC_EC:
		setval(VERASE, SLC_VARIABLE);
	case SLC_EL:
		setval(VKILL, SLC_VARIABLE);
	case SLC_IP:
		setval(VINTR, SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_ABORT:
		setval(VQUIT, SLC_VARIABLE|SLC_FLUSHIN|SLC_FLUSHOUT);
	case SLC_XON:
#ifdef	VSTART
		setval(VSTART, SLC_VARIABLE);
#else
		defval(0x13);
#endif
	case SLC_XOFF:
#ifdef	VSTOP
		setval(VSTOP, SLC_VARIABLE);
#else
		defval(0x11);
#endif
	case SLC_EW:
#ifdef	VWERASE
		setval(VWERASE, SLC_VARIABLE);
#else
		defval(0);
#endif
	case SLC_RP:
#ifdef	VREPRINT
		setval(VREPRINT, SLC_VARIABLE);
#else
		defval(0);
#endif
	case SLC_LNEXT:
#ifdef	VLNEXT
		setval(VLNEXT, SLC_VARIABLE);
#else
		defval(0);
#endif
	case SLC_AO:
#ifdef	VFLUSH
		setval(VFLUSH, SLC_VARIABLE|SLC_FLUSHOUT);
#else
		defval(0);
#endif
	case SLC_SUSP:
#ifdef	VSUSP
		setval(VSUSP, SLC_VARIABLE|SLC_FLUSHIN);
#else
		defval(0);
#endif
#ifdef	VEOL
	case SLC_FORW1:
		setval(VEOL, SLC_VARIABLE);
#endif
#ifdef	VEOL2
	case SLC_FORW2:
		setval(VEOL2, SLC_VARIABLE);
#endif

	case SLC_BRK:
	case SLC_SYNCH:
	case SLC_AYT:
	case SLC_EOR:
		defval(0);

	default:
		*valp = 0;
		*valpp = 0;
		return(SLC_NOSUPPORT);
	}
}

#ifdef	LINEMODE
/*
 * tty_flowmode()	Find out if flow control is enabled or disabled.
 * tty_linemode()	Find out if linemode (external processing) is enabled.
 * tty_setlinemod(on)	Turn on/off linemode.
 * tty_isecho()		Find out if echoing is turned on.
 * tty_setecho(on)	Enable/disable character echoing.
 * tty_israw()		Find out if terminal is in RAW mode.
 * tty_binaryin(on)	Turn on/off BINARY on input.
 * tty_binaryout(on)	Turn on/off BINARY on output.
 * tty_isediting()	Find out if line editing is enabled.
 * tty_istrapsig()	Find out if signal trapping is enabled.
 * tty_setedit(on)	Turn on/off line editing.
 * tty_setsig(on)	Turn on/off signal trapping.
 * tty_issofttab()	Find out if tab expansion is enabled.
 * tty_setsofttab(on)	Turn on/off soft tab expansion.
 * tty_islitecho()	Find out if typed control chars are echoed literally
 * tty_setlitecho()	Turn on/off literal echo of control chars
 * tty_tspeed(val)	Set transmit speed to val.
 * tty_rspeed(val)	Set receive speed to val.
 */

tty_flowmode()
{
	return(termbuf.c_iflag & IXON ? 1 : 0);
}

tty_linemode()
{
	return(termbuf.c_lflag & EXTPROC);
}

tty_setlinemode(on)
int on;
{
#ifdef	TIOCEXT
	(void) ioctl(pty, TIOCEXT, (char *)&on);
#else	/* !TIOCEXT */
#ifdef	EXTPROC
	if (on)
		termbuf.c_lflag |= EXTPROC;
	else
		termbuf.c_lflag &= ~EXTPROC;
#endif
	set_termbuf();
#endif	/* TIOCEXT */
}

tty_isecho()
{
	return (termbuf.c_lflag & ECHO);
}
#endif	/* LINEMODE */

tty_setecho(on)
int on;
{
	if (on)
		termbuf.c_lflag |= ECHO;
	else
		termbuf.c_lflag &= ~ECHO;
}

#if	defined(LINEMODE) && defined(KLUDGELINEMODE)
tty_israw()
{
	return(!(termbuf.c_lflag & ICANON));
}
#endif	/* defined(LINEMODE) && defined(KLUDGELINEMODE) */

tty_binaryin(on)
int on;
{
	if (on) {
		termbuf.c_lflag &= ~ISTRIP;
	} else {
		termbuf.c_lflag |= ISTRIP;
	}
}

tty_binaryout(on)
int on;
{
	if (on) 
		termbuf.c_oflag &= ~OPOST;
	 else 
		termbuf.c_oflag |= OPOST;
	
}

tty_isbinaryin()
{
	return(!(termbuf.c_iflag & ISTRIP));
}

tty_isbinaryout()
{
	return(!(termbuf.c_oflag&OPOST));
}

#ifdef	LINEMODE
tty_isediting()
{
	return(termbuf.c_lflag & ICANON);
}

tty_istrapsig()
{
	return(termbuf.c_lflag & ISIG);
}

tty_setedit(on)
int on;
{
	if (on)
		termbuf.c_lflag |= ICANON;
	else
		termbuf.c_lflag &= ~ICANON;
}

tty_setsig(on)
int on;
{
	if (on)
		termbuf.c_lflag |= ISIG;
	else
		termbuf.c_lflag &= ~ISIG;
}
#endif	/* LINEMODE */

tty_issofttab()
{
# ifdef	OXTABS
	return (termbuf.c_oflag & OXTABS);
# endif
# ifdef	TABDLY
	return ((termbuf.c_oflag & TABDLY) == TAB3);
# endif
}

tty_setsofttab(on)
int on;
{
	if (on) {
# ifdef	OXTABS
		termbuf.c_oflag |= OXTABS;
# endif
# ifdef	TABDLY
		termbuf.c_oflag &= ~TABDLY;
		termbuf.c_oflag |= TAB3;
# endif
	} else {
# ifdef	OXTABS
		termbuf.c_oflag &= ~OXTABS;
# endif
# ifdef	TABDLY
		termbuf.c_oflag &= ~TABDLY;
		termbuf.c_oflag |= TAB0;
# endif
	}
}

tty_islitecho()
{
# ifdef	ECHOCTL
	return (!(termbuf.c_lflag & ECHOCTL));
# endif
# ifdef	TCTLECH
	return (!(termbuf.c_lflag & TCTLECH));
# endif
# if	!defined(ECHOCTL) && !defined(TCTLECH)
	return (0);	/* assumes ctl chars are echoed '^x' */
# endif
}

tty_setlitecho(on)
int on;
{
# ifdef	ECHOCTL
	if (on)
		termbuf.c_lflag &= ~ECHOCTL;
	else
		termbuf.c_lflag |= ECHOCTL;
# endif
# ifdef	TCTLECH
	if (on)
		termbuf.c_lflag &= ~TCTLECH;
	else
		termbuf.c_lflag |= TCTLECH;
# endif
}

/*
 * A table of available terminal speeds
 */
struct termspeeds {
	int	speed;
	int	value;
} termspeeds[] = {
	{ 0,     B0 },    { 50,    B50 },   { 75,    B75 },
	{ 110,   B110 },  { 134,   B134 },  { 150,   B150 },
	{ 200,   B200 },  { 300,   B300 },  { 600,   B600 },
	{ 1200,  B1200 }, { 1800,  B1800 }, { 2400,  B2400 },
	{ 4800,  B4800 }, { 9600,  B9600 }, { 19200, B9600 },
	{ 38400, B9600 }, { -1,    B9600 }
};

tty_tspeed(val)
int val;
{
	register struct termspeeds *tp;

	for (tp = termspeeds; (tp->speed != -1) && (val > tp->speed); tp++)
		;
	termbuf.c_ospeed = tp->value;
}

tty_rspeed(val)
int val;
{
	register struct termspeeds *tp;

	for (tp = termspeeds; (tp->speed != -1) && (val > tp->speed); tp++)
		;
	termbuf.c_ispeed = tp->value;
}

/*
 * startslave(t, host)
 *
 * Given a file descriptor (t) for a tty, and a hostname, do whatever
 * is necessary to startup the login process on the slave side of the pty.
 */

/* ARGSUSED */
startslave(t, host)
int t;
char *host;
{
	register int i;

	if ((i = fork()) < 0)
		fatalperror(net, MSGSTR(FORK_ERR, "fork"));
	if (i) {
		(void) close(t);
	} else {
		start_login(t, host);
		/*NOTREACHED*/
	}
}

char	*envinit[3];
extern char **environ;

init_env()
{
	extern char *getenv();
	char env[25];
	char *envp;

	if (envp = getenv("TZ"))
	         strncpy(env,envp,sizeof(env));
        else
	         env[0] = '\0';
	clearenv();
	if (env[0] != '\0')
	        setenv("TZ",env,1);
}

/*
 * start_login(t, host)
 *
 * Assuming that we are now running as a child processes, this
 * function will turn us into the login process.
 */

start_login(t, host)
int t;
char *host;
{
	register char *cp;
	register char **argv;
	char **addarg();
	char *User;
	char *getenv();

	/*
	 * set up standard paths before forking to login
	 */
	if (login_tty(t) == -1)
		fatalperror(net, "login_tty");
	if (net > 2)
		(void) close(net);
	if (pty > 2)
		(void) close(pty);
	/*
	 * -h : pass on name of host.
	 *		WARNING:  -h is accepted by login if and only if
	 *			getuid() == 0.
	 * -p : don't clobber the environment (so terminal type stays set).
	 */
	argv = addarg(0, "login");
	argv = addarg(argv, "-h");
	argv = addarg(argv, host);
	argv = addarg(argv, "-p");
	if ((User = getenv("USER")) != NULL) {
		argv = addarg(argv, getenv("USER"));
	}
	/* 
	 * print login banner for user id
	 */
	if(User != NULL)
		fprintf(stdout, "login: %s\r\n", User);
	
	/* attempt to exec the gateway code before login code */

	putenv("GWSOURCE=telnet");
	execv("/usr/sbin/dnalogin", argv); 

	/* if dnalogin is not present, the above exec will fail */
	/* and execs the login                                  */

	execv(_PATH_LOGIN, argv);

	syslog(LOG_ERR, "%s: %m\n", _PATH_LOGIN);
	fatalperror(net, _PATH_LOGIN);
	/*NOTREACHED*/
}

char **
addarg(argv, val)
register char **argv;
register char *val;
{
	register char **cpp;
	char *malloc();

	if (argv == NULL) {
		/*
		 * 10 entries, a leading length, and a null
		 */
		argv = (char **)malloc(sizeof(*argv) * 12);
		if (argv == NULL)
			return(NULL);
		*argv++ = (char *)10;
		*argv = (char *)0;
	}
	for (cpp = argv; *cpp; cpp++)
		;
	if (cpp == &argv[(int)argv[-1]]) {
		--argv;
		*argv = (char *)((int)(*argv) + 10);
		argv = (char **)realloc(argv, (int)(*argv) + 2);
		if (argv == NULL)
			return(NULL);
		argv++;
		cpp = &argv[(int)argv[-1] - 10];
	}
	*cpp++ = val;
	*cpp = 0;
	return(argv);
}

/*
 * cleanup()
 *
 * This is the routine to call when we are all through, to
 * clean up anything that needs to be cleaned up.
 */
cleanup()
{
	char *p;

	p = line + sizeof("/dev/") - 1;
	if (logout(p))
		logwtmp(p, "", "");
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	*p = 'p';
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	(void) shutdown(net, 2);
	exit(1);
}
