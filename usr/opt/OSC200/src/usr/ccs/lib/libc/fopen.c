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
static char	*sccsid = "@(#)$RCSfile: fopen.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/08/18 21:36:03 $";
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
 * FUNCTIONS: fopen, freopen 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fopen.c	1.15  com/lib/c/io,3.1,9013 2/14/90 19:09:08
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include <errno.h>
#include "glue.h"
#include "stdiom.h"
#include <fcntl.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#include "rec_mutex.h"

extern struct rec_mutex	_iobptr_rmutex;
#endif	/* _THREAD_SAFE */

extern FILE *_findiop();
static FILE *_endopen();

/*
 * FUNCTION: The fopen subroutine opens the file named by the path
 *           parameter and associates a stream with it. 
 *
 * PARAMETERS: 
 *	     char *filename  - points to a character string that contains
 * 	                   the name of the file to be opened.
 *	     char *mode  - points to a character string that has one
 *	                   of the following values:
 *   			   "r", "w", "a", "r+", "w+", "a+", "rb", "wb",
 *			   "ab", "r+b", "w+b", "a+b"
 *
 *                         Note: In this implementation, there is not any 
 *                               difference between text and binary files.
 *                               Therefore, the b type will be ignored.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns a pointer to the FILE structure of this stream.
 *
 */
FILE *
fopen(const char *filename, const char *mode)
{
	return (_endopen(filename, mode, _findiop()));
}


/*
 *
 * FUNCTION: The  freopen subroutine  substitutes  the  named file  in
 *           place of the open stream.   The original stream is closed
 *           whether  or not  the  open succeeds.   
 *
 * PARAMETERS: 
 *	     char *filename  - points to a character string that contains
 * 	                   the name of the file to be opened.
 *	     char *mode  - points to a character string that has one
 *	                   of the following values:
 *   			   "r", "w", "a", "r+", "w+", "a+", "rb", "wb",
 *			   "ab", "r+b", "w+b", "a+b"
 *
 *                         Note: In this implementation, there is not any 
 *                               difference between text and binary files.
 *                               Therefore, the b type will be ignored.
 *	     FILE *stream   - points to an open stream
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns a pointer to the FILE structure associated with stream.
 *
 */
FILE *
freopen(const char *filename, const char *mode, FILE *stream)
{
	TS_FDECLARELOCK(filelock)

	if (stream == NULL) {
		seterrno(ENOENT);
		return (NULL);
	}
#ifdef _THREAD_SAFE
	TS_FLOCK(filelock, stream);
	(void) fclose_unlocked(stream);	/* doesn't matter if this fails */
	stream->_flag |= _IOINUSE;
	TS_FUNLOCK(filelock);
#else
	(void) fclose(stream);		/* doesn't matter if this fails */
#endif	/* _THREAD_SAFE */

	return (_endopen(filename, mode, stream));
}


#ifdef _THREAD_SAFE
#define	FREEIOB(fp)	\
		TS_LOCK(&_iobptr_rmutex), \
		TS_FLOCK(filelock, stream), \
		fp->_flag = 0, \
		TS_FUNLOCK(filelock), \
		_glued.freefile--, \
		TS_UNLOCK(&_iobptr_rmutex)
#else
#define	FREEIOB(fp)	\
		_glued.freefile--

#endif	/* _THREAD_SAFE */


static FILE *
_endopen(char *filename, char *mode, FILE *stream)
{
	int	plus, oflag, fd;
	TS_FDECLARELOCK(filelock)

	if (stream == NULL) {
		seterrno(ENOENT);
		return (NULL);
	}
	if (filename == NULL || filename[0] == '\0') {
		seterrno(ENOENT);
		FREEIOB(stream);
		return (NULL);
	}

	/*
	 * Validate the file modes
	 */
	if (mode == NULL || mode[0] == '\0') {
		seterrno(EINVAL);
		FREEIOB(stream);
		return (NULL);
	}

	plus = (mode[1] == '+') || (mode[1] && mode[2] == '+');

	switch (mode[0]) {
	case 'w':
		oflag = O_TRUNC | O_CREAT | (plus ? O_RDWR : O_WRONLY);
		break;
	case 'a':
		oflag = O_APPEND | O_CREAT | (plus ? O_RDWR : O_WRONLY);
		break;
	case 'r':
		oflag = plus ? O_RDWR : O_RDONLY;
		break;
	default:
		seterrno(EINVAL);
		FREEIOB(stream);
		return (NULL);
	}

/* replace open call with call to _open for name collison problem */

/* 5/27/93: tap: put original open call back for name space pollution
 * work.
 */
#ifndef _NAME_SPACE_WEAK_STRONG
	if ((fd = _open(filename, oflag, 0666)) < 0) {
#else
	if ((fd = open(filename, oflag, 0666)) < 0) {
#endif
		FREEIOB(stream);
		return (NULL);
	}
	if (mode[0] == 'a' && !plus) {
		/* Shouldn't change errno on successful operation */
		int saved_errno = geterrno();
		(void)lseek(fd, (off_t)0, SEEK_END);
		seterrno(saved_errno);
	}

	TS_FLOCK(filelock, stream);

	stream->_cnt = 0;
	stream->_file = fd;
        stream->_flag &= ~_IONONSTD;
	stream->_flag = plus ? _IORW : (mode[0] == 'r') ? _IOREAD : _IOWRT;
        stream->__newbase = NULL;
	stream->_bufendp = stream->_base = stream->_ptr = NULL;

	TS_FUNLOCK(filelock);
	return (stream);
}
