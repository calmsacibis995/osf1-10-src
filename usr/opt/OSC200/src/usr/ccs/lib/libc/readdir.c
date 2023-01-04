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
static char	*sccsid = "@(#)$RCSfile: readdir.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 21:37:55 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
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
 * readdir.c	5.4 (Berkeley) 6/18/88
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak readdir_r = __readdir_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak readdir = __readdir
#endif
#endif
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#include <string.h>
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE
# define SUCCESS 0
# define FAIL -1
#else
# define FAIL NULL
#endif   

/*
 * get next entry in a directory.
 */
#ifdef _THREAD_SAFE
int 
readdir_r(dirp, dirent)
	DIR *dirp;
	struct dirent *dirent;

#else

struct dirent *
readdir(dirp)
	DIR *dirp;

#endif /* _THREAD_SAFE */
{
	register struct dirent *dp;
	long offset;

	if (dirp == NULL || dirp->dd_buf == NULL) {
	  	_Seterrno(EBADF);
		return (FAIL);
	}
	TS_LOCK(dirp->dd_lock);
	for (;;) {

		/* Replenish buffer of entries from system.
		 */
		if (dirp->dd_loc == 0) {
			dirp->dd_size = getdirentries(dirp->dd_fd,
						      dirp->dd_buf, 
						      dirp->dd_bufsiz,
						      &offset);
			/* End of directory or error.
			 */
			if (dirp->dd_size <= 0) {
				TS_UNLOCK(dirp->dd_lock);
				return (FAIL);
			}
		}

		/* Need refill ?
		 */
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}

		dp = (struct dirent *)(dirp->dd_buf + dirp->dd_loc);

		if (dp->d_reclen <= 0 ||
		    dp->d_reclen > dirp->dd_bufsiz + 1 - dirp->dd_loc) {
			TS_UNLOCK(dirp->dd_lock);
			return (FAIL);
		}
		dirp->dd_loc += dp->d_reclen;

		/* Skip empty entries.
		 */
		if (dp->d_fileno == 0)
			continue;
#ifdef _THREAD_SAFE
		memcpy((void *)dirent, (void *)dp, sizeof(struct dirent));
		TS_UNLOCK(dirp->dd_lock);
		return (SUCCESS);
#else
		return (dp);
#endif	/* _THREAD_SAFE */
	}
}
