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
/*
 * xdm - display manager daemon
 *
 * $XConsortium: daemon.c,v 1.8 91/05/11 15:37:38 gildea Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xos.h>

#ifdef SVR4
#include <termios.h>
#else
#include <sys/ioctl.h>
#endif

#ifdef hpux
#include <sys/ptyio.h>
#endif

#include <errno.h>

extern void exit ();

BecomeOrphan ()
{
pid_t	child_id;
    /*
     * fork so that the process goes into the background automatically. Also
     * has a nice side effect of having the child process get inherited by
     * init (pid 1).
     */
    /*
     * Separate the child into its own process group before the parent
     * exits.  This eliminates the possibility that the child might get
     * killed when the init script that's running xdm exits.
     */

    child_id = fork();
    if (child_id)
	{
	if (0 != setpgid(child_id, child_id))
	    printf("xdm: setpgid on child failed errno = %d \n", errno);
	exit (0);
	}
}

BecomeDaemon ()
{
    register int i;

    /*
     * Close standard file descriptors and get rid of controlling tty
     */

#if defined(SYSV) || defined(SVR4)
    setpgrp ();
#else
    setpgrp (0, getpid());
#endif

    close (0); 
    close (1);
    close (2);

#ifndef SYSV386
    if ((i = open ("/dev/tty", O_RDWR)) >= 0) {	/* did open succeed? */
#if (defined(SYSV) || defined(SVR4)) && defined(TIOCTTY)
	int zero = 0;
	(void) ioctl (i, TIOCTTY, &zero);
#else
	(void) ioctl (i, TIOCNOTTY, (char *) 0);    /* detach, BSD style */
#endif
	(void) close (i);
    }
#endif /* !SYSV386 */

    /*
     * Set up the standard file descriptors.
     */
    (void) open ("/", O_RDONLY);	/* root inode already in core */
    (void) dup2 (0, 1);
    (void) dup2 (0, 2);
}
