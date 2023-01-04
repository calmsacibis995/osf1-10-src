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
static char *rcsid = "@(#)$RCSfile: pty.c,v $ $Revision: 1.1.12.4 $ (DEC) $Date: 1993/06/07 18:05:51 $";
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
/*-
 * Copyright (c) 1990 The Regents of the University of California.
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

#if defined(LIBC_SCCS) && !defined(lint)

#endif /* LIBC_SCCS and not lint */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak fix_errno = __fix_errno
#pragma weak fork_slvmod = __fork_slvmod
#pragma weak forkpty = __forkpty
#pragma weak openpty = __openpty
#endif
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>
#include <grp.h>
#include <errno.h>
#ifdef _THREAD_SAFE
#include <sia.h>
#endif

/* constant for max advertised or documented pty supported */
/* (versus number of ptys supported internally - 3162)     */
#define MAX_PTY_ADV 816

int
openpty(amaster, aslave, name, termp, winp)
	int *amaster, *aslave;
	char *name;
	struct termios *termp;
	struct winsize *winp;
{
	register char *clone = "/dev/ptmx_bsd";
	char *template = "/dev/ttyXX";
	char line [10];
	char *cp1 = "pqrstuvwxyzabcefghijklmnoABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *cp2 = "0123456789abcdef";
	char *cp3 = "ghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char *grp_nam = "tty";
#ifdef _THREAD_SAFE
	char buffer[SIABUFSIZ+1];
	int len = SIABUFSIZ;
	struct group _group;
#endif
	char char_8, char_9;
	register int master, slave, euid, ruid, ttygid;
	dev_t  dev;
	struct group *gr;
	void fix_errno();
	
	strcpy(line,template);
#ifdef _THREAD_SAFE
	gr = &_group;
	if (getgrnam_r(grp_nam,gr,buffer,len) == 0)
#else
	if ((gr = getgrnam("tty")) != NULL)
#endif
		ttygid = gr->gr_gid;
	else
		ttygid = -1;
	ruid = getuid();
	euid = geteuid();

	if (( master = open(clone,O_RDWR, 0)) == -1) {
		fix_errno();
		return (-1);	/* out of ptys */
	}
	dev = (dev_t)ioctl(master,ISPTM,0);
	if (dev < 0) {
		(void) close(master);
		fix_errno();
		return (-1);
	}
	if (minor(dev) < MAX_PTY_ADV) {
		char_8 = cp1[(minor(dev))/16];	
		char_9 = cp2[(minor(dev))%16];
	}
	else {
		char_8 = cp1[(minor(dev) - MAX_PTY_ADV)/46];	
		char_9 = cp3[(minor(dev) - MAX_PTY_ADV)%46];
	}
	line[8] = char_8;
	line[9] = char_9;
	if (!euid) {  
		(void) chown(line, ruid, ttygid);
		(void) chmod(line, 0620);
		(void) revoke(line);
	} else {
		if (fork_slvmod(master) < 0) {
			(void) close(master);
			fix_errno();
			return(-1);
		}
	}
	if ((slave = open(line, O_RDWR, 0)) != -1) {
		*amaster = master;
		*aslave = slave;
		if (name)
			strcpy(name, line);
		if (termp)
			(void) tcsetattr(slave, 
				TCSAFLUSH, termp);
		if (winp)
			(void) ioctl(slave, TIOCSWINSZ, 
				(char *)winp);
		return (0);
	}
	(void) close(master);
	fix_errno();
	return (-1);
}

int
fork_slvmod(mast)
register int mast;
{
	int	cstat;
	pid_t	cpid;
	int ret_val;
#ifndef _THREAD_SAFE
	sigset_t newset,oldset;
#endif

	/*
         *  block the SIGCHLD signal
	 */
#ifndef _THREAD_SAFE
	sigemptyset(&newset);
	sigaddset(&newset,SIGCHLD);
	if (sigprocmask(SIG_BLOCK,&newset,&oldset) != 0)
		return(-1);
#endif

	if ((cpid = fork()) < 0)
		return(-1);
	if (cpid) {
		if (waitpid(cpid,&cstat,0) == -1)
			return(-1);
		if (WIFEXITED(cstat))  
			ret_val = 0;
		else
			ret_val = -1;
	/*
	 * restore original signal mask
	 */
#ifndef _THREAD_SAFE
		sigprocmask(SIG_SETMASK,&oldset,(sigset_t *)NULL);
#endif
		return(ret_val);
	}
	else  {
		if (dup2(mast,1) == -1)
			exit(-1);
		execl("/usr/lbin/slvmod","slvmod",0);
		exit(-1); /* shouldn't get here!!!! */
	}
}
void
fix_errno()
{
	int l_errno;
#ifdef _THREAD_SAFE
	l_errno = geterrno(); 
#else
	l_errno = errno;
#endif
	
	switch(l_errno){
		case ENXIO:
		case ENOENT:
		case EMFILE:
		case ENFILE:
		case ENOMEM:
		case EAGAIN:
			break;
		default:
#ifdef _THREAD_SAFE
			seterrno(ENXIO);
#else
			errno = ENXIO;
#endif
	}
}

pid_t
forkpty(amaster, name, termp, winp)
	int *amaster;
	char *name;
	struct termios *termp;
	struct winsize *winp;
{
	int	master, slave;
	pid_t	pid;

	if (openpty(&master, &slave, name, termp, winp) == -1)
		return (-1);
	switch (pid = fork()) {
	case -1:
		fix_errno();
		return (-1);
	case 0:
		/* 
		 * child
		 */
		(void) close(master);
		login_tty(slave);
		return (0);
	}
	/*
	 * parent
	 */
	*amaster = master;
	(void) close(slave);
	return (pid);
}
