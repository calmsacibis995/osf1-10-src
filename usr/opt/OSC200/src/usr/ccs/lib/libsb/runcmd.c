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
static char	*sccsid = "@(#)$RCSfile: runcmd.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:45:32 $";
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
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <varargs.h>

fd_runcmdv(cmd, dir, withpath, closefd, outfd, av)
char *cmd, *dir;
int withpath;
int closefd, outfd;
char **av;
{
    int child;
    int fd;
    int i;

    (void) fflush(stderr);
    if ((child = vfork()) == -1) {
	fprintf(stderr, "%s vfork failed: %s\n", av[0], errmsg(-1));
	return(-1);
    }
    if (child == 0) {
	(void) setgid(getgid());
	(void) setuid(getuid());
	if (dir != NULL && chdir(dir) < 0) {
	    fprintf(stderr, "%s chdir failed: %s\n", dir, errmsg(-1));
	    (void) fflush(stderr);
	    _exit(0377);
	}
	if (closefd >= 0)
	    (void) close(closefd);
	if (outfd >= 0 && outfd != 1) {
	    (void) dup2(outfd, 1);
	    (void) close(outfd);
	}
	if ((fd = open("/dev/null", O_RDONLY, 0)) > 0) {
	    (void) dup2(fd, 0);
	    (void) close(fd);
	}
	if (withpath)
	    execvp(cmd, av);
	else
	    execv(cmd, av);
	_exit(0377);
    }
    return(child);
}

/* VARARGS6 */
fd_runcmd(cmd, dir, withpath, closefd, outfd, va_alist)
char *cmd, *dir;
int withpath;
int closefd, outfd;
va_dcl
{
    int status;
    va_list ap;

    va_start(ap);
    status = fd_runcmdv(cmd, dir, withpath, closefd, outfd, ap);
    va_end(ap);
    return(status);
}

runcmdv(cmd, dir, av)
char *cmd, *dir;
char **av;
{
    return(fd_runcmdv(cmd, dir, 0, -1, -1, av));
}

/* VARARGS3 */
runcmd(cmd, dir, va_alist)
char *cmd, *dir;
va_dcl
{
    int status;
    va_list ap;

    va_start(ap);
    status = fd_runcmdv(cmd, dir, 0, -1, -1, ap);
    va_end(ap);
    return(status);
}

endcmd(child)
int child;
{
    int pid, omask;
    union wait w;
    int status;

    omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
    do {
	pid = wait3(&w, WUNTRACED, (struct rusage *)NULL);
	status = w.w_status;
	if (WIFSTOPPED(w)) {
	    (void) kill(0, SIGTSTP);
	    pid = 0;
	}
    } while (pid != child && pid != -1);
    (void) sigsetmask(omask);
    if (pid == -1) {
	fprintf(stderr, "wait error: %s\n", errmsg(-1));
	return(-1);
    }
    if (WIFSIGNALED(w) || w.w_retcode == 0377)
	return(-1);
    return(w.w_retcode);
}
