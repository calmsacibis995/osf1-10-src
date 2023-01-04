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
static char	*sccsid = "@(#)$RCSfile: dosys.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/04/29 19:22:24 $";
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
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 * dosys.c	4.11 (Berkeley) 11/15/87"; 
 */

#include "defs.h"
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#ifndef sigmask
#define sigmask(m)	(1 << ((m)-1))
#endif

#ifndef _BLD
#include "make_msg.h"
extern nl_catd  catd;
#define MSGSTR(Num, Str) catgets(catd, MS_MAKE, Num, Str)
#include <NLctype.h>
#else
#define MSGSTR(Num, Str) Str
#include <ctype.h>
#endif

FSTATIC int wpid = 0;
FSTATIC int waitmsk = 0;
FSTATIC doshell(), doexec(), shexec();
FSTATIC int await();


int
dosys(comm, nohalt, exok)
	register char *comm;
	int nohalt, exok;
{
	register int status;
	int onoexflag;

	if (metas(comm))
		doshell(comm, nohalt);
	else
		doexec(comm);
	waitmsk = sigblock(sigmask(SIGHUP)|sigmask(SIGINT)|sigmask(SIGQUIT));
	status = await(nohalt);
	if (exok) {
		onoexflag = noexflag;
		noexflag = YES;
	}
	(void) sigsetmask(waitmsk);
	if (exok)
		noexflag = onoexflag;
	return status;
}


/*
 * Are there are any Shell meta-characters?
 */
int
metas(s)
	register char *s;
{
	register char c;

	while ((funny[c = *s++] & META) == 0)
		;
	return c;
}


FSTATIC
doshell(comm, nohalt)
	char *comm;
	int nohalt;
{
	(void) fflush(stdout);
	(void) fflush(stderr);
	switch (wpid = vfork()) {
	case -1:
		fatal(MSGSTR(CANTFORK,"Cannot fork"));
	case 0:
		enbint(SIG_DFL);
		shexec(comm, nohalt);
	}
}




FSTATIC
doexec(comm)
	register char *comm;
{
	register char *t;
	register char **p;
	char *argv[MAXARGV];

	while (isspace(*comm))
		++comm;
	if (*comm == 0)
		fatal(MSGSTR(NOCMD,"no command"));

	p = argv;
	for (t = comm; *t; ) {
		if (p >= argv + MAXARGV)
			fatal(MSGSTR(MAXARGS,"%s: Too many arguments"), comm);
		*p++ = t;
		while (*t && !isspace(*t))
			++t;
		if (*t)
			for (*t++ = 0; isspace(*t); ++t)
				;
	}
	*p = 0;
	(void) fflush(stdout);
	(void) fflush(stderr);
	switch (wpid = vfork()) {
	case -1:
		fatal(MSGSTR(CANTFORK,"Cannot fork"));
	case 0:
		enbint(SIG_DFL);
		execvp(*argv, argv);
		perror(*argv);
		_exit(1);
	}
}


FSTATIC
shexec(comm, nohalt)
	char *comm;
	int nohalt;
{
#ifdef SHELLENV
	char *shellcom, *shellstr;
	extern char *getenv();

	if ((shellcom = getenv("SHELL")) == 0)
		shellcom = SHELLCOM;
	if ((shellstr = rindex(shellcom, '/')) == 0)
		shellstr = shellcom;
	else
		shellstr += 1;
	execl(shellcom, shellstr, nohalt ? "-c" : "-ce", comm, (char *)0);
	perror(shellcom);
#else
	execl(SHELLCOM, "sh", nohalt ? "-c" : "-ce", comm, (char *)0);
	perror(SHELLCOM);
#endif
	_exit(1);
}

#undef WCOREDUMP
#undef WIFSTOPPED
#undef WIFEXITED
#undef WEXITSTATUS
#undef WIFSIGNALED
#undef WTERMSIG

#define WCOREDUMP(x)    ( (x) & 0x80 )
#define	WIFSTOPPED(x)	( ( (x) & 0x7f ) == 0x7f )
#define	WIFEXITED(x)	( !((x) & 0xff) )
#define	WEXITSTATUS(x)	(int)(WIFEXITED(x) ? (((x) >> 8) & 0xff) : -1)
#define	WIFSIGNALED(x)	(  !WIFEXITED(x) && !WIFSTOPPED(x) )
#define	WTERMSIG(x)	(int)(WIFSIGNALED(x) ? ((x) & 0x7f) : -1)

FSTATIC int
await(nohalt)
	int nohalt;
{
	register int pid;
#ifdef BSDCOM
	union wait status;
#else
	int status;
#endif
	unsigned int sig;
	extern char *sys_siglist[];

	while ((pid = wait(&status)) != wpid)
		if (pid == -1)
			fatal(MSGSTR(BADWAIT,"bad wait code"));
	wpid = 0;
#ifdef	BSDCOM
	if (status.w_status) {
#else
	if (status) {
#endif
		if (WIFSIGNALED(status)) {
			sig = WTERMSIG(status);
			if (sig < NSIG && sys_siglist[sig] && *sys_siglist[sig])
				printf("*** %s", sys_siglist[sig]);
			else
				printf(MSGSTR(SYSSIG,"*** Signal %d"), sig);
			if (WCOREDUMP(status))
				printf(MSGSTR(DUMPCORE," - core dumped"));
			printf("\n");
			(void) fflush(stdout);
			(void) sigsetmask(waitmsk);
			quit((int) sig);
		}
		printf(MSGSTR(EXITED,"*** Exit %d"),
		       WEXITSTATUS(status));
		if (nohalt)
			printf(MSGSTR(IGNORE," (ignored)\n"));
		else if (keepgoing)
			printf("\n");
		else
			fatal((char *) 0);
		(void) fflush(stdout);
	}
#ifdef	BSDCOM
	return status.w_status;
#else
	return status;
#endif
}


touch(force, name)
	int force;
	char *name;
{
	register int fd;
	struct stat stbuff;
	char junk[1];
	extern long lseek();

	if (stat(name, &stbuff) == -1) {
		if (force)
			goto create;
		perror(name);
		return;
	}
	if (stbuff.st_size == 0)
		goto create;

	if ((fd = open(name, 2)) == -1)
		goto bad;

	if (read(fd, junk, 1) != 1
	|| lseek(fd, 0L, 0) != 0L
	|| write(fd, junk, 1) != 1) {
		(void) close(fd);
		goto bad;
	}
	(void) close(fd);
	return;

bad:
	perror(name);
	return;

create:
	if ((fd = creat(name, 0666)) == -1)
		goto bad;
	(void) close(fd);
}


FILE *
pfopen(comm, nohalt)
	char *comm;
	int nohalt;
{
	FILE *f;
	int fds[2];
	if (pipe(fds) == -1)
		fatal(MSGSTR(CANTPIPE,"Cannot make pipe"));
	(void) fflush(stdout);
	(void) fflush(stderr);
	switch (wpid = vfork()) {
	case -1:
		fatal(MSGSTR(CANTFORK,"Cannot fork"));
	case 0:
		enbint(SIG_DFL);
		(void) close(fds[0]);
		(void) dup2(fds[1], 1);
		(void) close(fds[1]);
		shexec(comm, nohalt);
	}
	(void) close(fds[1]);
	if ((f = fdopen(fds[0], "r")) == NULL)
		fatal(MSGSTR(CANTGETFL,"Cannot allocate file structure"));
	waitmsk = sigblock(sigmask(SIGHUP)|sigmask(SIGINT)|sigmask(SIGQUIT));
	return f;
}


int
pfclose(f, nohalt)
	FILE *f;
	int nohalt;
{
	register int status;

	(void) fclose(f);
	status = await(nohalt);
	(void) sigsetmask(waitmsk);
	return status;
}
