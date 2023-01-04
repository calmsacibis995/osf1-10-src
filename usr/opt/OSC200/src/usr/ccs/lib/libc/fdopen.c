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
static char	*sccsid = "@(#)$RCSfile: fdopen.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:49:18 $";
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
 * FUNCTIONS: fdopen 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fdopen.c	1.12  com/lib/c/io,3.1,9013 2/27/90 17:20:00
 */

/*LINTLIBRARY*/
/*
 * Unix routine to do an "fopen" on file descriptor
 * The mode has to be repeated because you can't query its
 * status
 */
/* The fdopen() function includes all the POSIX requirements */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak fdopen = __fdopen
#endif
#include <stdio.h>
#include "stdiom.h"
#include <fcntl.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern long lseek();
extern FILE *_findiop();

/*
 * FUNCTION: routine to do an "fopen" on file descriptor
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	     - FILE * on success
 *	     - NULL on failure
 */
FILE *
fdopen(int fd, const char *mode)
{
	register FILE	*iop;
	int		status_file;
	TS_FDECLARELOCK(filelock)

	if ((status_file = fcntl(fd, F_GETFL)) == -1)
		return (NULL);
	status_file &= O_ACCMODE;

	if ((iop = _findiop()) == NULL)
		return (NULL);

	TS_FLOCK(filelock, iop);
	iop->_cnt = 0;
	iop->_flag = 0;
	iop->_file = fd;
	iop->__newbase = NULL;
	iop->_base = iop->_ptr = NULL;

	if (mode[1] == '+' || (mode[1] && mode[2] == '+')) {
		if (status_file == O_RDWR) {
			iop->_flag &= ~(_IOREAD | _IOWRT);
			iop->_flag |= _IORW;
		} else {
			seterrno(EINVAL);
			iop = NULL;
		}
		TS_FUNLOCK(filelock);
		if (*mode == 'a') {
			(void) lseek(fd, 0L, SEEK_END);
		}

		return (iop);
	}

	switch (*mode) {

		case 'r':
			if (!(status_file == O_RDONLY
			      || status_file == O_RDWR)) {
				iop->_flag = 0;
				seterrno(EINVAL);
				TS_FUNLOCK(filelock);
				return (NULL);
			}
			if (!(iop->_flag & _IORW))
				iop->_flag |= _IOREAD;
			break;
		case 'a':
			if (!(status_file == O_WRONLY
			      || status_file == O_RDWR)) {
				iop->_flag = 0;
				seterrno(EINVAL);
				TS_FUNLOCK(filelock);
				return (NULL);
			}
			(void) lseek(fd, 0L, SEEK_END);
			if (!(iop->_flag & _IORW))
				iop->_flag |= _IOWRT;
			break;
		case 'w':
			if (!(status_file == O_WRONLY
			      || status_file == O_RDWR)){
				iop->_flag = 0;
				seterrno(EINVAL);
				TS_FUNLOCK(filelock);
				return (NULL);
			}
			if (!(iop->_flag & _IORW))
				iop->_flag |= _IOWRT;
			break;
		default:
			iop->_flag = 0;
			seterrno(EINVAL);
			TS_FUNLOCK(filelock);
			return (NULL);
	}

	TS_FUNLOCK(filelock);
	return (iop);
}
