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
static char	*sccsid = "@(#)$RCSfile: prntdsply.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:22 $";
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
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 */



/*
 * DisplayFile()	display files through a pager
 * DisplayCommand()	send the output of a command through a pager
 * PrintFile()		print a file through lp
 * PrintCommand()	print the output of a command through lp
 */

#include <stdio.h>
#include <setjmp.h>
#include <sys/signal.h>
#include <string.h>
#include "userif.h"
#include "curs_supp.h"
#include "key_map.h"
#include "kitch_sink.h"
#include "logging.h"

#define PRINTFILECMD	"/usr/bin/lp"
#define DEFAULT_PAGER	"/usr/bin/pg"

static int do_the_work();
static int ps_child();

int
DisplayFile(filename)
	char *filename;
{
	return do_the_work(NULL, NULL, filename, 0);
}

int
DisplayCommand(program, argv)
	char *program;
	char *argv[];
{
	return do_the_work(program, argv, NULL, 0);
}

PrintFile(filename)
	char *filename;
{
	return do_the_work(NULL, NULL, filename, 1);
}

PrintCommand(program, argv)
	char *program;
	char *argv[];
{
	return do_the_work(program, argv, NULL, 1);
}

/*
 * send the output of a program or a file through a pager or to the printer.
 * If a program, program and argv are non-null.
 * If a file, filename is non-null.
 * If printing is desired, printit is non-zero.
 * If printit is zero, display the file/program output through the pager.
 */

static int
do_the_work(program, argv, filename, printit)
	char *program;
	char *argv[];
	char *filename;
	int printit;
{
	int	pid;
	int	wait_stat;
	char	buf[80];
	void (*oldsighup)();
	void (*oldsigint)();
	void (*oldsigquit)();
	extern void hup_catch();

	/* anytime hitting the screen, need to allow hangup to stop us.
	 */
	(void) signal (SIGHUP, SIG_DFL);
	clear();
	move (0, 0);
	refresh();
	reset_shell_mode();

	oldsighup = signal(SIGHUP, hup_catch);
	oldsigint = signal(SIGINT, SIG_IGN);
	oldsigquit = signal(SIGQUIT, SIG_IGN);
	switch (pid = fork()) {

	default:	/* see parent code below */
		break;

	case	0:	/* child */

		/* allow all the usual signals to kill the children
		 * forked underneath the parent (screen) process.
		 */

		(void) signal(SIGHUP, SIG_DFL);
		(void) signal(SIGINT, SIG_DFL);
		(void) signal(SIGQUIT, SIG_DFL);

		ps_child(program, argv, filename, printit);
		/* never returns */

	case 	-1:
		reset_prog_mode();
		pop_msg ("Cannot fork process to run program.",
		"Please check conditions and re-run.");
		(void) signal(SIGHUP, oldsighup);
		(void) signal(SIGINT, oldsigint);
		(void) signal(SIGQUIT, oldsigquit);
		return (1);
	}

	/* parent waits for child and restores shell mode */

	while ((pid = wait(&wait_stat)) == -1)
		;

	(void) signal (SIGHUP, SIG_DFL);
	printf("\nPress <RETURN> to continue: ");
	fflush(stdout);
	fgets(buf, sizeof(buf), stdin);
	reset_prog_mode();
	clearok(stdscr, TRUE);

	/* restore signal environment */

	(void) signal(SIGHUP, oldsighup);
	(void) signal(SIGINT, oldsigint);
	(void) signal(SIGQUIT, oldsigquit);

	return (1);
}

static int
ps_child(program, argv, filename, printit)
	char *program;
	char *argv[];
	char *filename;
	int printit;
{
	char	*cp, *pager;
	int	pipefd[2];
	int	pid, wait_stat;

	/*
	 * if sending output from a program
	 * create pipe for communication between program
	 * and pager/print program
	 */

	if (program) {
		if (pipe(pipefd) < 0) {
			printf("Cannot create pipe.\n");
			exit(1);
		}

		/*
		 * turn off signals while waiting for child
		 */

		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGQUIT, SIG_IGN);

		/*
		 * fork a child if running a program
		 */

		switch (pid = fork()) {

		case	0:  /* child needs to run the program */

			/* turn on signals */

			(void) signal(SIGINT, SIG_DFL);
			(void) signal(SIGQUIT, SIG_DFL);

			/* send stdout of child to pipe (close read side) */

			close(pipefd[0]);
			close(1);
			close(2);
			dup(pipefd[1]);
			dup(1);
			close (pipefd[1]);

			/* execute program */

			execv(program, argv);
			printf("Error: could not execute \'%s\'.\n", program);
			exit(1);

		case	-1:	/* fork failure */

	printf("Error: could not fork.  Check conditions and try again.\n");
			exit (1);

		default:	/* parent re-directs input from pipe */

			close (pipefd[1]);
			close (0);
			dup (pipefd[0]);
			close (pipefd[0]);
			break;
		}

	} else {

		/*
		 * input is coming from a file; re-direct stdin
		 */

		FILE *fp;

		fp = freopen(filename, "r", stdin);
		if (fp == (FILE *) 0) {
			printf("Cannot open file '%s'\n", filename);
			exit(1);
		}
	}

	if (!printit) { /* output to a terminal */

		/*
		 * determine the user's favorite pager (environment or default)
		 * and run that program
		 */

		pager = getenv("PAGER");
		if (pager == NULL)
			pager = DEFAULT_PAGER;
		cp = strrchr(pager, '/');
		if (cp)
			cp++;
		else
			cp = pager;

		execl(pager, cp, NULL);
		perror("Cannot exec pager");

		/*
		 * if running a program, kill the child
		 */

		if (program) {
			kill(pid, 9);
			while (wait(&wait_stat) < 0)
				;
		}

		exit(1);

	} else {  /* print output */

		cp = strrchr(PRINTFILECMD, '/');
		if (cp)
			cp++;
		else
			cp = PRINTFILECMD;

		execl(PRINTFILECMD, cp, NULL);
		printf("Could not execute program '%s'\n", PRINTFILECMD);
		exit(1);
	}
}
