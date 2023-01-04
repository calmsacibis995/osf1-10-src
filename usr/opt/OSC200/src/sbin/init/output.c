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
static char	*sccsid = "@(#)$RCSfile: output.c,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1992/11/03 15:32:46 $";
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
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE
 * ONLY SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp.
 * 1989 All Rights Reserved 
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp. 
 *
 */

#include	<sys/limits.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdarg.h>
#include	<sys/termios.h>
#include	<sys/fcntl.h>
#include	<ctype.h>
#define TTYDEFCHARS
#include	"init.h"
#if defined(NLS) || defined(KJI)
#include	<sys/NLchar.h>
#endif

#ifdef	MSG
#include	"init_msg.h"
#endif


/*************************/
/****    switchcon    ****/
/*************************/

switchcon(int sig)
{
	extern pid_t    own_pid;/* changed by staffan@osf.de */
	extern int      fd_syscon;
	extern int      switchcon(int);
	struct sigaction action;

#ifdef	XDEBUG
	debug("We have entered switchcon().\n");
#endif

	action.sa_flags = 0;	/* no flags */
	action.sa_handler = SIG_IGN;

	sigaction(sig, &action, (struct sigaction *) NULL);

	/*
	 * If this is the first time a <del> has been typed on the
	 * /dev/syscon, then issue the console change command to
	 * /dev/syscon.  Also reestablish file pointers.
	 */
	if (fd_syscon != -1) {
		reset_console();
		openconsole();

		/*
		 * Set fd_syscon to -1 so that we ignore any deletes from it
		 * in the future as far as changing /dev/console to /dev/syscon.
		 */
		fd_syscon = -1;
	}
	action.sa_handler = (void (*) (int)) switchcon;

	sigaction(sig, &action, (struct sigaction *) NULL);
}

struct termios  curterm;	/* current terminal state */

/**************************/
/****    openconsole    ****/
/**************************/

/*
 * "openconsole" opens stdin, stdout, and stderr, making sure
 * that their file descriptors are 0, 1, and 2, respectively.
 */

openconsole()
{
	int             fd;

	close(0);
	close(1);
	close(2);

	fd = open(CONSOLE, O_RDWR);
	dup(fd);
	dup(fd);
	setbuf(stdin, (char *) NULL);
	setbuf(stdout, (char *) NULL);
	setbuf(stderr, (char *) NULL);

	/*
	 * Make sure the hangup on last close is off.  Then restore
	 * the modes that were on console when the signal was sent.
	 */
	tcgetattr(fd, &curterm);
	termios.c_cflag &= ~HUPCL;
	termios.c_cflag |= CLOCAL; /* make sure modem control is turned off */
	termios.c_cc[VERASE] = 0177;
	tcsetattr(fd, TCSADRAIN, &termios);
	return;
}


/********************************/
/****    get_ioctl_console    ****/
/********************************/

/* "get_ioctl_console" initializes the /dev/console settings */

get_ioctl_console()
{
	/* get the default internal states */
	memcpy(&termios, &dflt_termios, sizeof(dflt_termios));
	memcpy(termios.c_cc, ttydefchars, sizeof(dflt_termios.c_cc));
}

/****************************/
/****    reset_console    ****/
/****************************/

/*
 * "reset_console" changes /dev/console to /dev/syscon and puts
 * the default ioctl setting back into /etc/ioctl.syscon and
 * the incore arrays.
 */

reset_console()
{
}

/***********************/
/****    console    ****/
/***********************/
/*VARARGS1*/

/*
 * "console" forks a child if it finds that it is the main "init"
 * and outputs the requested message to the system console.
 * Note that the number of arguments passed to "console" is
 * determined by the print format.
 */
#ifndef	_NO_PROTO
console(char *format, ...)
#else
console(format, va_alist)
const char *format;
va_dcl
#endif
{
	extern struct proc *efork();
	struct proc *p;
	extern pid_t    own_pid;/* changed by staffan@osf.de */
	char            outbuf[BUFSIZ];
	extern int      childeath(void);
#ifdef	UDEBUG
	extern int      abort();
	struct sigaction action;
#endif
	va_list		ap;

#ifdef	XDEBUG
	vdebug("We have entered console().\n");
#endif
	va_start(ap, format);
	/*
	 * Are we the original "init"?  If so, fork a child to do the
	 * printing for us.
	 */
	if (own_pid == SPECIALPID) {
		while ((p = efork(NULLPROC, NOCLEANUP)) == NO_ROOM)
			timer(5);
		if (p == NULLPROC) {
#ifdef	UDEBUG
			action.sa_flags = 0;
			action.sa_handler = (void (*) (int)) abort;
			sigaction(SIGUSR1, &action, (struct sigaction *) NULL);
			sigaction(SIGUSR2, &action, (struct sigaction *) NULL);
#endif
			/*
			 * Close the standard descriptors and open the system
			 * console. 
			 */
			openconsole();	/* open proper file descriptors for
					 * console */
			setbuf(stdout, outbuf);

			/* Output the message to the console. */
			NLfprintf(stdout, "\nINIT: ");
			NLvfprintf(stdout, format, ap);
			fflush(stdout);
			tcsetattr(1, TCSADRAIN, &curterm);
			exit(0);

			/* The parent waits for the message to complete. */
		}
		else {
			while (waitproc(p) == FAILURE);
			freeproc(p);
		}

		/*
		 * If we are some other "init", then just print directly to
		 * the standard output.
		 */
	}
	else {
		openconsole();
		setbuf(stdout, outbuf);
		NLfprintf(stdout, "\nINIT: ");
		NLvfprintf(stdout, format, ap);
		fflush(stdout);
		tcsetattr(1, TCSADRAIN, &curterm);
	}
#ifdef	ACCTDEBUG
	vdebug(format, ap);
#endif
	va_end(ap);
}

#ifdef DEBUGGER

/*********************/
/****    debug    ****/
/*********************/

debug(format, arg1, arg2, arg3, arg4, arg5, arg6)
char	*format;
caddr_t	arg1, arg2, arg3, arg4, arg5, arg6;
{
	FILE           *fp;
	int             errnum;
	extern int      errno;

	if ((fp = fopen(DBG_FILE, "a+")) == NULL) {
		errnum = errno;
		console("Can't open \"%s\".  errno: %d\n", DBG_FILE, errnum);
		return;
	}
	fprintf(fp, format, arg1, arg2, arg3, arg4, arg5, arg6);
	fclose(fp);
}

vdebug(format, args)
char	*format;
va_list	args;
{
	FILE           *fp;

	if ((fp = fopen(DBG_FILE, "a+")) != NULL) {
		vfprintf(fp, format, args);
		fclose(fp);
	}
}

/*****************/
/**** vetchar ****/
/*****************/
/*
 * This little routine takes a char pointer input and makes
 * sure that each character is printable (non-control).
 */
char *
vetchar(id)
	register char  *id;
{
	static char     answer[12];
	register char  *ptr;
	register int    i;

#ifdef	XDEBUG
	debug("We have entered vetchar().\n");
#endif
	for (i = 6, ptr = answer; --i >= 0; id++) {
		if (isprint(*id) == 0) {
			*ptr++ = '^';
			*ptr++ = *id + 0100;
		}
		else
			*ptr++ = *id;
	}
	*ptr++ = '\0';
	return (answer);
}
#endif				/* DEBUGGER */
