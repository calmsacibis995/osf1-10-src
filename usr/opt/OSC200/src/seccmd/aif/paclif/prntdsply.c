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
static char *rcsid = "@(#)$RCSfile: prntdsply.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:13:22 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	prntdsply.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:52:36  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:35  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:20  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:29  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 *	(#)prntdsply.c	1.5 16:31:14 5/17/91 SecureWare
 */

/* #ident "@(#)prntdsply.c	1.1 11:18:07 11/8/91 SecureWare" */

/*
 * DisplayFile()	display files through a pager
 * DisplayCommand()	send the output of a command through a pager
 * PrintFile()		print a file through the line printer program
 * PrintCommand()	print the output of a command through the line
 *			printer program
 */

#include <sys/secdefines.h>
#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

#define PRINTFILECMD	"/usr/bin/lp"
#define DEFAULT_PAGER	"/usr/bin/pg"

#ifdef DEBUG
#define DEBUG_MSG(a,b)	\
	{	FILE *fp = fopen("/dev/tty", "w");	\
		if (fp != (FILE *) 0) {			\
			fprintf(fp, a, b);		\
			fclose(fp);			\
		}					\
	}
#endif /* DEBUG */

/* static routine definitions */

static void leave_curses();
static void enter_curses();
static void ignore_signals();
static void obey_signals();
static pid_t fork_child();
static int child_status();
static void kill_process();
static void redirect_from_fd();
static void redirect_to_fd();
static void pager();
static void do_command();
static void print_cmd();

extern char *getenv();

/*
 * The plumbing for this module is as follows:
 *
 * DisplayFile:
 *	input is from file, output is to screen, return exit status of pager
 *
 *               -----------
 *    file ----> |  pager  |------> stdout
 *               -----------
 *
 * DisplayCommand:
 *	input to program is from the pipe specified if pfd is >= 0
 *	Otherwise the input to the program is from stdin
 *	output from program is to new pipe
 *	input to pager is from pipe
 *	output from program is to screen
 *	return exit status of program
 *
 *                    ----------- pipe  ---------
 *   stdin/pipe ----> | program | ----> | pager | ----> stdout
 *                    -----------       ---------
 *
 * PrintFile:
 *	Input to print command is from filename specified
 *	Output of print command is to stdout
 *	Return exit status of print command
 *
 *                -----------------
 *     file ----> | print command | ----> stdout
 *                -----------------
 *
 * PrintCommand:
 *      Input to program is from stdin
 *      Output of the program is to a new pipe
 *      Input of the print command is from the pipe
 *      Output of the print command is to stdout
 *      Return exit status of the program
 *
 *                 ----------- pipe  -----------------
 *     stdin ----> | program | ----> | print command | ----> stdout
 *                 -----------       -----------------
 */

/*
 * redirect input to a pager from the specified file
 */

int
DisplayFile(filename)
	char *filename;
{
	pid_t pid;
	int file_fd;
	int ret;

	leave_curses();

	switch (pid = fork_child()) {
	case -1:
		printf(MSGSTR(PRNTDSPLY_1, "Unable to fork pager process, errno %d\n"), errno);
		enter_curses();
		return -1;
	case 0:
		file_fd = open(filename, O_RDONLY);
		if (file_fd == -1) {
			FILE *fp;

			fp = fopen("/dev/tty", "w");
			if (fp != (FILE *) 0) {
				fprintf(fp,
				  MSGSTR(PRNTDSPLY_2, "File %s could not be opened for reading.\n"),
				  filename);
				fclose(fp);
			}
			exit(1);
		}

		redirect_from_fd(file_fd);
		pager();		/* never returns */
	default:
		ret = child_status(pid);
		enter_curses();
		return ret;
	}
}

/*
 * redirect output from the command to the pager
 */

int
DisplayCommand(program, argv, input_pipe)
	char *program;
	char *argv[];
	int input_pipe;
{
	int pfd[2];	/* pipe from program to pager */
	pid_t pager_pid;
	pid_t program_pid;
	int wait_stat;
	int save_errno;
	int ret;
	
	leave_curses();

	if (pipe(pfd) < 0) {
		printf(MSGSTR(PRNTDSPLY_3, "Unable to create pipe between program and pager\n"));
		enter_curses();
		return -1;
	}

	/* the first child executes the pager */

	switch (pager_pid = fork_child()) {
	case -1:
		printf(MSGSTR(PRNTDSPLY_4, "Unable to create pager process\n"));
		close(pfd[0]);
		close(pfd[1]);
		enter_curses();
		return -1;
	case 0:
		close(pfd[1]);
		redirect_from_fd(pfd[0]);
		pager();		/* never returns */
	default:
		break;
	}

	/*
	 * the next child executes the program.
	 * if necessary, it redirects its input from the input pipe
	 */

	switch (program_pid = fork_child()) {
	case -1:
		printf(MSGSTR(PRNTDSPLY_5, "Unable to create program process\n"));
		close(pfd[0]);
		close(pfd[1]);
		kill_process(pager_pid);
		child_status(pager_pid);
		enter_curses();
		return -1;
	case 0:
		close(pfd[0]);
		if (input_pipe >= 0)
			redirect_from_fd(input_pipe);
		redirect_to_fd(pfd[1]);
		do_command(program, argv);	/* never returns */
	default:
		break;
	}

	/* close pipe and wait for completion of sub-processes */
	close(pfd[0]);
	close(pfd[1]);

	/* wait for termination of processes and return status of program */

	for (save_errno = 0; save_errno != ECHILD; ) {
		pid_t wait_pid;

		wait_pid = wait(&wait_stat);

		if (wait_pid == -1)
			save_errno = errno;

		if (wait_pid == program_pid)
			ret = wait_stat;

#ifdef DEBUG
		if (wait_pid == program_pid)
			DEBUG_MSG("Program returned wait status %d\n",
			  wait_stat);
		else if (wait_pid == pager_pid)
			DEBUG_MSG("Pager returned wait status %d\n", wait_stat);
		else if (wait_pid > 0)
			DEBUG_MSG("Other program returned wait status %d\n",
			  wait_stat);
		else
			DEBUG_MSG("Wait returned -1, errno %d\n", errno);
#endif
	}

	enter_curses();

	return wait_stat;
}

/*
 * redirect input from the specified file, fork print command
 */

PrintFile(filename)
	char *filename;
{
	pid_t print_pid;
	int file_fd;
	int wait_stat;
	
	leave_curses();

	file_fd = open(filename, O_RDONLY);
	if (file_fd == -1) {
		printf(MSGSTR(PRNTDSPLY_6, "Unable to open file %s\n"), filename);
		leave_curses();
		return -1;
	}

	/* fork a child process to run the print command */

	switch (print_pid = fork_child()) {
	case -1:
		printf(MSGSTR(PRNTDSPLY_7, "Unable to create process for print command\n"));
		leave_curses();
		close(file_fd);
		return -1;
	case 0:
		redirect_from_fd(file_fd);
		print_cmd();		/* never returns */
	default:
		close(file_fd);
		break;
	}

	wait_stat = child_status(print_pid);
	enter_curses();
	return wait_stat;
}

/*
 * create a pipe betweeen the program and the print command.
 * return the status of the program
 */

PrintCommand(program, argv)
	char *program;
	char *argv[];
{
	int pfd[2];	/* pipe from program to print command */
	pid_t print_pid;
	pid_t program_pid;
	int wait_stat;
	int save_errno;
	int ret;
	
	leave_curses();

	if (pipe(pfd) < 0) {
		printf(
		  MSGSTR(PRNTDSPLY_8, "Unable to create pipe between program and print command\n"));
		enter_curses();
		return -1;
	}

	/* the first child executes the print command */

	switch (print_pid = fork_child()) {
	case -1:
		printf(MSGSTR(PRNTDSPLY_4, "Unable to create pager process\n"));
		close(pfd[0]);
		close(pfd[1]);
		enter_curses();
		return -1;
	case 0:
		close(pfd[1]);
		redirect_from_fd(pfd[0]);
		print_cmd();		/* never returns */
	default:
		break;
	}

	/*
	 * the next child executes the program.
	 * if necessary, it redirects its input from the input pipe
	 */

	switch (program_pid = fork_child()) {
	case -1:
		printf(MSGSTR(PRNTDSPLY_5, "Unable to create program process\n"));
		close(pfd[0]);
		close(pfd[1]);
		kill_process(print_pid);
		child_status(print_pid);
		enter_curses();
		return -1;
	case 0:
		close(pfd[0]);
		redirect_to_fd(pfd[1]);
		do_command(program, argv);	/* never returns */
	default:
		break;
	}

	/* close pipe and wait for completion of sub-processes */
	close(pfd[0]);
	close(pfd[1]);

	/* account for termination of processes and return status of program */

	for (save_errno = 0; save_errno != ECHILD; ) {
		pid_t wait_pid;

		wait_pid = wait(&wait_stat);

		if (wait_pid == -1)
			save_errno = errno;

		if (wait_pid == program_pid)
			ret = wait_stat;

#ifdef DEBUG
		if (wait_pid == program_pid)
			DEBUG_MSG("Program returned wait status %d\n",
			  wait_stat);
		else if (wait_pid == print_pid)
			DEBUG_MSG("Print command returned wait status %d\n",
			  wait_stat);
		else
			DEBUG_MSG("Wait returned -1, errno %d\n", errno);
#endif
	}

	enter_curses();

	return wait_stat;
}

/*
 * The remainder of this file includes support routines called from
 * the previous four main routines
 */

/*
 * Initial signal state upon entry to these routines.
 * These variables are set by leave_curses() and restored by enter_curses()
 */

void (*oldsighup)();
void (*oldsigint)();
void (*oldsigquit)();
void (*oldsigalrm)();

/*
 * Enter the "shell mode" for output of line-oriented programs to the screen
 */

static void
leave_curses()
{
	/*
	 * clear the screen, put the cursor on the top left,
	 * and exit curses mode.
	 */

	clear();
	move (0, 0);
	refresh();
	reset_shell_mode();

	/* save signal state and ignore signals */

	oldsighup = signal(SIGHUP, SIG_IGN);
	oldsigint = signal(SIGINT, SIG_IGN);
	oldsigquit = signal(SIGQUIT, SIG_IGN);
	oldsigalrm = signal(SIGALRM, SIG_IGN);
}

/*
 * Re-enter curses mode
 */

static void
enter_curses()
{
	FILE *fp;
	char buf[10];

	fp = fopen("/dev/tty", "r+");
	if (fp != (FILE *) 0) {
		fprintf(fp, MSGSTR(PRNTDSPLY_9, "Press <RETURN> to continue: "));
		fflush(fp);
		fgets(buf, sizeof(buf), fp);
		fclose(fp);
	}

	reset_prog_mode();
	clearok(stdscr, TRUE);

	(void) signal(SIGHUP, oldsighup);
	(void) signal(SIGINT, oldsigint);
	(void) signal(SIGQUIT, oldsigquit);

	/* reset alarm catcher to update the time on the screen */

	(void) signal(SIGALRM, oldsigalrm);

	alarm((unsigned int) UPD_SECS);
}

/*
 * Ignore signals while waiting for the children to complete
 */

static void
ignore_signals()
{
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
}

/*
 * React normally to signals
 */

static void
obey_signals()
{
	(void) signal(SIGHUP, SIG_DFL);
	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
}

/*
 * Fork a child process and return the exit status.
 * The routine raises the LIMIT authorization to override per-user limit.
 */

static pid_t
fork_child()
{
	privvec_t saveprivs;
	pid_t pid;

	forceprivs(privvec(SEC_LIMIT, -1), saveprivs);
	pid = fork();
	seteffprivs(saveprivs, NULL);
	return pid;
}

/*
 * wait for the specified child to complete and return its exit status
 */

static int
child_status(pid)
	pid_t pid;
{
	int wait_status;
	int ret;

	while ((ret = wait(&wait_status)) != pid) {
#ifdef DEBUG
		DEBUG_MSG("Unexpected return in child_status, errno %d\n",
		  ret);
#endif
		;
	}
	return wait_status;
}

/*
 * Send a kill signal to the process specified -- process may have
 * changed ID or sensitivity label, so privileges must be raised
 */

static void
kill_process(pid)
	pid_t pid;
{
	privvec_t saveprivs;

	forceprivs(privvec(SEC_KILL,
#if SEC_MAC
			SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
			SEC_ALLOWNCAVACCESS,
#endif
			-1), saveprivs);
	kill(pid, SIGKILL);
	seteffprivs(saveprivs, NULL);
}

/*
 * redirect the program's standard input from the specified file descriptor
 */

static void
redirect_from_fd(fd)
	int fd;
{
	close(0);
	dup(fd);
	close(fd);
}

/*
 * redirect the program's standard output and error output to the
 * specified file descriptor
 */

static void
redirect_to_fd(fd)
	int fd;
{
	close(1);
	dup(fd);
	close(2);
	dup(fd);
	close(fd);
}

/*
 * execute the pager, specified from the environment or from a default value
 */

static void
pager()
{
	char *pager_program;
	char *cp;
	FILE *fp;

	/*
	 * determine the user's favorite pager (environment or default)
	 * and run that program
	 */

	pager_program = getenv("PAGER");
	if (pager_program == NULL)
		pager_program = DEFAULT_PAGER;
	cp = strrchr(pager_program, '/');
	if (cp)
		cp++;
	else
		cp = pager_program;

#ifdef DEBUG
	fp = fopen("/dev/tty", "r+");
	if (fp != (FILE *) 0) {
		fprintf(fp, MSGSTR(PRNTDSPLY_10, "ps_child parent: executing pager %s\n"),
		  pager_program);
		fclose(fp);
	}
#endif
	execl(pager_program, cp, NULL);
	fp = fopen("/dev/tty", "r+");
	if (fp != (FILE *) 0) {
		fprintf(fp, MSGSTR(PRNTDSPLY_11, "Cannot execute pager %s\n"), cp);
		fclose(fp);
	}
	exit(1);
}

/*
 * Execute the command specified by the program and its arguments
 */

static void
do_command(program, argv)
	char *program;
	char *argv[];
{
	FILE *fp;

#ifdef DEBUG
	int i;

	fp = fopen("/dev/tty", "r+");
	if (fp != (FILE *) 0) {
		fprintf(fp, MSGSTR(PRNTDSPLY_12, "do_command, executing %s\n"), program);
		for (i = 0; argv[i]; i++)
			fprintf(fp, MSGSTR(PRNTDSPLY_13, "\targument %d: %s\n"), i, argv[i]);
		fclose(fp);
	}
#endif
	execv(program, argv);
	fp = fopen("/dev/tty", "r+");
	if (fp != (FILE *) 0) {
		fprintf(fp, MSGSTR(PRNTDSPLY_14, "do_command: could not execute \'%s\'.\n"), program);
		fclose(fp);
	}
	exit(1);
}

/*
 * Execute the print command (system-specific) to print the output
 */

static void
print_cmd()
{
	char *cp;
	FILE *fp;

	cp = strrchr(PRINTFILECMD, '/');
	if (cp)
		cp++;
	else
		cp = PRINTFILECMD;

	execl(PRINTFILECMD, cp, NULL);

	fp = fopen("/dev/tty", "r+");
	if (fp != (FILE *) 0) {
		fprintf(fp, MSGSTR(PRNTDSPLY_14, "do_command: could not execute \'%s\'.\n"), cp);
		fclose(fp);
	}

	exit(1);
}
