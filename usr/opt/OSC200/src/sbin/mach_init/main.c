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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:49:50 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	Program:	Service server
 *
 *	Purpose:
 *		Create ports for globally interesting services,
 *		and hand the receive rights to those ports (i.e.
 *		the ability to serve them) to whoever asks.
 *
 *	Why we need it:
 *		We need to get the service ports into the
 *		very top of the task inheritance structure,
 *		but the currently available system startup
 *		mechanism doesn't allow the actual servers
 *		to be started up from within the initial task
 *		itself.  We start up as soon as we can, and
 *		force the service ports back up the task tree,
 *		and let servers come along later to handle them.
 *
 *		In the event that a server dies, a new instantiation
 *		can grab the same service port.
 *
 *	Can be run before /etc/init instead, if desired.
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <varargs.h>
#include <sys/signal.h>
#include <mach.h>

extern int errno;

extern void serv_init();
extern void serv_loop();

int sig_alarm();
int sig_term();

FILE *error_stream();

char *program_name;

boolean_t signalled = FALSE;

#ifdef	DEBUG
boolean_t debug = TRUE;
boolean_t verbose = TRUE;
#else	DEBUG
boolean_t debug = FALSE;
boolean_t verbose = FALSE;
#endif	DEBUG

boolean_t exec_init = TRUE;

int
main(argc, argv, envp)
	int argc;
	char *argv[];
	char *envp[];
{
	int i;
	kern_return_t kr;
	task_t parent_task;
	int parent_pid;
	int wakeup_pid;

	setbuf(stdout, NULL);

	program_name = rindex(argv[0], '/');
	if (program_name == NULL)
		program_name = argv[0];
	else
		program_name++;

	for (i = 1; i < argc; i++) {
		char *pp;

		for (pp = argv[i]; *pp != '\0'; pp++)
			switch (*pp) {
				case 'n':
				case 'N':
					exec_init = isupper(*pp);
					break;

				case 'd':
				case 'D':
					debug = islower(*pp);
					break;

				case 'v':
				case 'V':
					verbose = islower(*pp);
					break;
			}
	}

	wakeup_pid = getpid();
	parent_pid = (wakeup_pid == 1) ? 1 : getppid();

	signal(SIGTERM, sig_term);
	signal(SIGALRM, sig_alarm);

	if (exec_init && debug)
		(void) error_stream();

	/*
 	 *	Create a child process which will actually
	 *	do the servicing; the parent will exit (to
	 *	indicate that the registration is done), or
	 *	run /etc/init (if done as part of system startup).
	 *
	 *	The parent must be the task that exec's /etc/init,
	 *	because /etc/init must have pid 1.  We don't want
	 *	the parent to exec /etc/init until the ports are
	 *	registered.  It would be easiest to allocate and
	 *	register the ports before forking, but this would
	 *	leave the parent with receive rights and it's awkward
	 *	to get the receive rights to the child.  So:
	 *	the parent pauses, waiting (up to 60 secs) for a signal
	 *	from the child that it OK to go ahead.  The child
	 *	initializes and then signals the parent.
	 */

	if (fork()) {
		alarm(60);
		if (!signalled)
			pause();
		if (debug)
			printf("Parent terminating.\n");

		fflush(stdout);
		fflush(stderr);

		if (exec_init) {
			if (argv[0] != NULL)
				argv[0] = "init";
			execve("/sbin/init", argv, envp);

			if (verbose)
				fprintf(error_stream(), "%s: exec /etc/init failed\n");
			return(errno);
		}
		return(0);
	}

	/*
	 *	Make ourselves an "init" process, so we
	 *	won't be known to (or killed by) another init process.
	 */

	if (exec_init)
		(void) init_process();

	/*
	 *	Get our original parent's task port.  If we were spawned
	 *	directly as an init process, so we don't have an original
	 *	parent, this will be our current parent (init).
	 */

	kr = task_by_unix_pid(task_self(), parent_pid, &parent_task);
	if (kr != KERN_SUCCESS)
		fprintf(error_stream(), "%s: cannot get task for parent\n",
			program_name);

	serv_init(parent_task);

	/*
	 *	Wake up the parent.
	 */

	if (debug)
		printf("Registered\n");
	kill(wakeup_pid, SIGTERM);

	if (verbose)
		printf("%s: child alive, pid %d\n", program_name, getpid());

	serv_loop();
}

int
sig_alarm()
{
	signalled = TRUE;
}

int
sig_term()
{
	signalled = TRUE;
}

FILE *
error_stream()
{
	static boolean_t io_init = FALSE;

	/*
	 *	If we're the initial process, we must first open
	 *	somewhere to write error messages.
	 */

	if (exec_init && !io_init) {
	    	FILE *f;

		if ((f = fopen("/dev/console", "w")) != NULL)
			fclose(f);
		freopen("/dev/tty", "r", stdin);
		setbuf(stdin, NULL);
		freopen("/dev/tty", "w", stdout);
		setbuf(stdout, NULL);
		freopen("/dev/tty", "w", stderr);
		setbuf(stdout, NULL);
		io_init = TRUE;
	}
	return(stderr);
}

void
barf(va_alist)
	va_dcl
{
	char *fmt;
	va_list pvar;

	va_start(pvar);
	fmt = va_arg(pvar, char *);
	fprintf(error_stream(), "%s: ", program_name);
	vfprintf(error_stream(), fmt, pvar);
	fputc('\n', error_stream());
	va_end(pvar);

}
