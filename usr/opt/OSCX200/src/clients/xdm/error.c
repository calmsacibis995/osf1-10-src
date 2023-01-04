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
 * $XConsortium: error.c,v 1.12 91/04/02 11:56:56 rws Exp $
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

/*
 * error.c
 *
 * Log display manager errors to a file as
 * we generally do not have a terminal to talk to
 */

# include "dm.h"
# include <stdio.h>

InitErrorLog ()
{
	int	i;
	if (errorLogFile[0]) {
		i = creat (errorLogFile, 0666);
		if (i != -1) {
			if (i != 2) {
				dup2 (i, 2);
				close (i);
			}
		} else
			LogError ("Cannot open errorLogFile %s\n", errorLogFile);
	}
}

/*VARARGS1*/
LogInfo (fmt, arg1, arg2, arg3, arg4, arg5, arg6)
char	*fmt;
char *	arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
{
    fprintf (stderr, "info (pid %d): ", getpid());
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
    fflush (stderr);
}

/*VARARGS1*/
LogError (fmt, arg1, arg2, arg3, arg4, arg5, arg6)
char	*fmt;
char *	arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
{
    fprintf (stderr, "error (pid %d): ", getpid());
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
    fflush (stderr);
}

/*VARARGS1*/
LogPanic (fmt, arg1, arg2, arg3, arg4, arg5, arg6)
char	*fmt;
char *	arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
{
    LogError ("panic (pid %d): ", getpid());
    LogError (fmt, arg1, arg2, arg3, arg4, arg5, arg6);
    exit (1);
}

/*VARARGS1*/
LogOutOfMem (fmt, arg1, arg2, arg3, arg4, arg5, arg6)
char	*fmt;
char *	arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
{
    fprintf (stderr, "xdm: out of memory in routine ");
    fprintf (stderr, fmt, arg1, arg2, arg3, arg4, arg5, arg6);
    fflush (stderr);
}

Panic (mesg)
char	*mesg;
{
    int	i;

    i = creat ("/dev/console", 0666);
    write (i, "panic: ", 7);
    write (i, mesg, strlen (mesg));
    exit (1);
}


/*VARARGS1*/
Debug (fmt, arg1, arg2, arg3, arg4, arg5, arg6)
char	*fmt;
char *	arg1, * arg2, * arg3, * arg4, * arg5, * arg6;
{
    if (debugLevel > 0)
    {
	printf (fmt, arg1, arg2, arg3, arg4, arg5, arg6);
	fflush (stdout);
    }
}
