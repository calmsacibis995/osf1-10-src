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
static char	sccsid[] = "@(#)$RCSfile: dbg.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:45:29 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <signal.h>
#include <sys/ptrace.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <loader.h>

char *program;

void parent(pid_t);
void wait_on_child(pid_t);
void child(char *);

main(argc, argv)
	char *argv[];
{
	pid_t pid;

	program = argv[0];

	if (argc < 2) {
		fprintf(stderr, "usage: %s <program-filename>\n", program);
		exit(1);
	}
	
	if (!(pid = fork()))
		child(argv[1]);

	if (pid == -1) {
		fprintf(stderr, "main: fork() failed: %s\n", strerror(errno));
		exit(1);
	}

	parent(pid);

	exit(0);
}

void
child(program)
	char *program;
{
	if (ptrace(PT_TRACE_ME) == -1) {
		fprintf(stderr, "child: ptrace(PT_TRACE_ME) failed: %s\n",
			strerror(errno));
		exit(1);
	}
	execl(program, program, (char *)0);
	fprintf(stderr, "child: execl(\"%s\", \"%s\", (char *)0) failed: %s\n",
		program, program, strerror(errno));
	exit(1);
}

void
parent(pid)
	pid_t pid;
{
	ldr_process_t child;

	child = (ldr_process_t)pid;
	wait_on_child(pid);
	cmd_info(child);

	exit(0);
}

void
wait_on_child(pid)
	pid_t pid;
{
	int status;

	if (wait(&status) == -1) {
		fprintf(stderr, "wait_on_child: wait() failed: %s\n",
			strerror(errno));
		exit(1);
	}

	if (!(WIFSTOPPED(status) && (WSTOPSIG(status) == SIGTRAP))) {
		fprintf(stderr, "wait_on_child: child not stopped with SIGTRAP: status = %#x\n",
			status);
		exit(1);
	}

#ifndef	STOP_ONCE_ONLY
	if (ptrace(PT_CONTINUE, pid, 1, 0) == -1) {
		fprintf(stderr, "wait_on_child: ptrace(PT_CONTINUE) failed: %s\n",
			strerror(errno));
		exit(1);
	}

	if (wait(&status) == -1) {
		fprintf(stderr, "wait_on_child: wait() failed: %s\n",
			strerror(errno));
		exit(1);
	}

	if (!(WIFSTOPPED(status) && (WSTOPSIG(status) == SIGTRAP))) {
		fprintf(stderr, "wait_on_child: child not stopped with SIGTRAP: status = %#x\n",
			status);
		exit(1);
	}
#endif	/* STOP_ONCE_ONLY */
}
