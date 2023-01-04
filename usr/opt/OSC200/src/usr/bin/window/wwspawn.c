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
static char	*sccsid = "@(#)$RCSfile: wwspawn.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:14:44 $";
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * wwspawn.c	3.13 (Berkeley) 6/29/88
 */


#include "ww.h"
#include <sys/signal.h>

/*
 * There is a dead lock with vfork and closing of pseudo-ports.
 * So we have to be sneaky about error reporting.
 */
wwspawn(wp, file, argv)
register struct ww *wp;
char *file;
char **argv;
{
	int pid;
	int ret;
	char buf[] = "0";
	int s;
	int fildes[2], cc;

	s = sigblock(sigmask(SIGCHLD));
	/*
 	* Note:  OSF/1 does not support the native BSD vfork sematics.
 	* What should happen:
 	* 	vfork spawns the child, the parent is blocked
 	*	The child executes wwenviron() and then execs the shell
 	*	If the exec fails, erred is set and the child exits.
 	*	The parent is restarted after the child has execed or
 	*	exited.  It checks for an error, and continues on its way.
 	* What does happen:
 	*	vfork is treated as a fork, both the parent and 
	* 	child are running.
 	*	The child executes wwenviron(), the parent continues on 
	*	its merry way.
 	*	*Sometimes* the child gets killed (possibly a SIGHUP)
 	*	in wwenviron(), and never gets to exec.
 	*	After much trying, I was not able to track down the reason for
 	*	the child dieing.  Something the parent is doing is probably
 	*	the cause, but I can't seem to find it.
 	* My solution:
 	* 	Make the parent wait till the child says OK, via a pipe.
 	*	If the exec or wwenviron() fails, the child sends
 	*	a "0" down the line, otherwise it sends a "1".
 	* 	I had to pass the file descriptor which wwneviron shouldn't
 	* 	close and add a check for it in said function.
 	*/
	if(pipe(fildes) != 0) {
		wwerrno = WWE_SYS;
		ret = -1;
	} else {
		switch (pid = vfork()) {
		case -1:
			wwerrno = WWE_SYS;
			ret = -1;
			break;
		case 0:
			/* Child */
			close(fildes[0]);  /* close reading side */
			if (wwenviron(wp, fildes[1]) >= 0) {
				buf[0] = '1';
				(void) write(fildes[1], buf, 1);
				(void) close(fildes[1]);
				execvp(file, argv);
			}
			(void) write(fildes[1], buf, 1);
			(void) close(fildes[1]);
			_exit(1);
		default:
			/* Parent */
			close(fildes[1]); /* close writing side */
			cc = read(fildes[0], buf, 1);
			(void) close(fildes[0]);
			if ((cc != 1) || (buf[0] != '1')) {
				wwerrno = WWE_SYS;
				ret = -1;
			} else {
				wp->ww_pid = pid;
				wp->ww_state = WWS_HASPROC;
				ret = pid;
			}
		} 
	} /* if */
	(void) sigsetmask(s);
	if (wp->ww_socket >= 0) {
		(void) close(wp->ww_socket);
		wp->ww_socket = -1;
	}
	return ret;
}
