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
 * Xau - X Authorization Database Library
 *
 * $XConsortium: AuLock.c,v 1.8 91/12/16 19:56:07 gildea Exp $
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

#include <X11/Xauth.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

extern int errno;

#if NeedFunctionPrototypes
int
XauLockAuth (
_Xconst char *file_name,
int	retries,
int	timeout,
long	dead)
#else
int
XauLockAuth (file_name, retries, timeout, dead)
char	*file_name;
int	retries;
int	timeout;
long	dead;
#endif
{
    char	creat_name[1025], link_name[1025];
    char	*strcpy (), *strcat ();
    long	time ();
    unsigned	sleep ();
    struct stat	statb;
    long	now;
    int		creat_fd = -1;

    if (strlen (file_name) > 1022)
	return LOCK_ERROR;
    (void) strcpy (creat_name, file_name);
    (void) strcat (creat_name, "-c");
    (void) strcpy (link_name, file_name);
    (void) strcat (link_name, "-l");
    if (stat (creat_name, &statb) != -1) {
	now = time ((long *) 0);
	/*
	 * NFS may cause ctime to be before now, special
	 * case a 0 deadtime to force lock removal
	 */
	if (dead == 0 || now - statb.st_ctime > dead) {
	    (void) unlink (creat_name);
	    (void) unlink (link_name);
	}
    }
    
    while (retries > 0) {
	if (creat_fd == -1) {
	    creat_fd = creat (creat_name, 0666);
	    if (creat_fd == -1) {
		if (errno != EACCES)
		    return LOCK_ERROR;
	    } else
		(void) close (creat_fd);
	}
	if (creat_fd != -1) {
	    if (link (creat_name, link_name) != -1)
		return LOCK_SUCCESS;
	    if (errno == ENOENT) {
		creat_fd = -1;	/* force re-creat next time around */
		continue;
	    }
	    if (errno != EEXIST)
		return LOCK_ERROR;
	}
	(void) sleep ((unsigned) timeout);
	--retries;
    }
    return LOCK_TIMEOUT;
}
