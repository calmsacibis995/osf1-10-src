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
static char	*sccsid = "@(#)$RCSfile: wwenviron.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/29 17:56:49 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Edward Wang at The University of California, Berkeley.
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
 * 
 * wwenviron.c	3.26 (Berkeley) 6/6/90";
 * taken from BSD 4.4 "reno" alpha release code.
 */


#include "ww.h"
#ifdef POSIX_TTY
#include <sys/ioctl.h>
#endif
#include <sys/signal.h>

/*#undef TIOCSCTTY	/* To use 4.3 Compat until kernel fixed */
			/* DEC OSF/1 kernel fixed -- Fred */
/*
 * Set up the environment of this process to run in window 'wp'.
 */
wwenviron(wp, fildes)
register struct ww *wp;
int fildes;
{
	register i;
#ifndef TIOCSCTTY
	int pgrp = getpid();
#endif
	char buf[1024];

#ifndef TIOCSCTTY
	if ((i = open("/dev/tty", 0)) < 0)
		goto bad;
	if (ioctl(i, TIOCNOTTY, (char *)0) < 0)
		goto bad;
	(void) close(i);
#endif
	if ((i = wp->ww_socket) < 0) {
		struct winsize winsize;

		if ((i = open(wp->ww_ttyname, 2)) < 0)
			goto bad;
		if (wwsettty(i, &wwwintty, (struct ww_tty *)0) < 0)
			goto bad;
		winsize.ws_row = wp->ww_w.nr;
		winsize.ws_col = wp->ww_w.nc;
		winsize.ws_xpixel = winsize.ws_ypixel = 0;
		if (ioctl(i, TIOCSWINSZ, (char *)&winsize) < 0)
			goto bad;
	}
	(void) dup2(i, 0);
	(void) dup2(i, 1);
	(void) dup2(i, 2);
	for (i = wwdtablesize - 1; i > 2; i--) {
		if (i == fildes) /* don't close our pipe */
			continue;
		(void) close(i); 
	}
#ifdef TIOCSCTTY
	(void) setsid(0);
	(void) ioctl(0, TIOCSCTTY, 0);
#else
	(void) ioctl(0, TIOCSPGRP, (char *)&pgrp);
	(void) setpgrp(pgrp, pgrp);
#endif
	/* SIGPIPE is the only one we ignore */
	(void) signal(SIGPIPE, SIG_DFL);
	(void) sigsetmask(0);
	/*
	 * Two conditions that make destructive setenv ok:
	 * 1. setenv() copies the string,
	 * 2. we've already called tgetent which copies the termcap entry.
	 */
	(void) sprintf(buf, "%sco#%d:li#%d:%s",
		WWT_TERMCAP, wp->ww_w.nc, wp->ww_w.nr, wwwintermcap);
	(void) setenv("TERMCAP", buf, 1);
	(void) sprintf(buf, "%d", wp->ww_id + 1);
	(void) setenv("WINDOW_ID", buf, 1);
	return(0);
bad:
	wwerrno = WWE_SYS;
	return -1;
}
