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
static char	*sccsid = "@(#)$RCSfile: opendir.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/07 23:32:00 $";
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
 * opendir.c	5.3 (Berkeley) 6/18/88
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak opendir = __opendir
#endif
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#ifdef _THREAD_SAFE
#include "rec_mutex.h"
#endif	/* _THREAD_SAFE */


/*
 * open a directory.
 */
DIR *
opendir(const char *name)
{
	register DIR *dirp;
	register int fd;
	struct stat st;
	int pid;

	if ((fd = open((char *) name, O_RDONLY)) == -1)
		return(NULL);
	(void) fcntl(fd, F_SETFD, FD_CLOEXEC);

	if (fstat(fd, &st) < 0 || (st.st_mode&S_IFMT) != S_IFDIR) {
		close(fd);
		_Seterrno(ENOTDIR);
		return(NULL);
	}
	if ((dirp = malloc(sizeof(DIR))) == NULL) {
		close(fd);
		return(NULL);
	}
	dirp->dd_bufsiz = MAX(st.st_blksize, MAXBSIZE);
	if ((dirp->dd_buf = malloc(dirp->dd_bufsiz)) == NULL) {
		(void)free(dirp);
		close(fd);
		return(NULL);
	}
#ifdef _THREAD_SAFE
	if (_rec_mutex_alloc((rec_mutex_t *)&dirp->dd_lock) < 0) {
		(void)free(dirp->dd_buf);
		(void)free(dirp);
		close(fd);
		return(NULL);
	}
#endif	/* _THREAD_SAFE */
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return(dirp);
}
