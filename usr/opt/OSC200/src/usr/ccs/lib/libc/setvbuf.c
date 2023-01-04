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
static char	*sccsid = "@(#)$RCSfile: setvbuf.c,v $ $Revision: 4.3.8.4 $ (DEC) $Date: 1993/06/29 18:33:25 $";
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
 * FUNCTIONS: setvbuf
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * setvbuf.c	1.11  com/lib/c/io,3.1,8943 9/9/89 13:43:56
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak setvbuf_unlocked = __setvbuf_unlocked
#endif
#endif
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <malloc.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern void free();
extern int isatty();

/*
 * FUNCTION: Assigns buffering to a stream.
 */

int 	
#ifdef	_THREAD_SAFE
setvbuf_unlocked(FILE *stream, char *buf, int mode, size_t size)
#else
setvbuf(FILE *stream, char *buf, int mode, size_t size)
#endif	/* _THREAD_SAFE */
{
	/* Validate the stream and mode arguments */
	if (stream == NULL) { 
		TS_SETERR(EFAULT);
		return -1;
	}
	else if (fileno(stream) < 0 || fileno(stream) > getdtablesize()) {
		TS_SETERR(EBADF);
		return -1;
	}
	else if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF) {
		TS_SETERR(0);
		return -1;
	}
	if(stream->_base != NULL && stream->_flag & _IOMYBUF) {
		free((char*)(stream->_base-(2 * MB_LEN_MAX)));
		stream->_base = NULL;
	}
	stream->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);

	switch (mode) {
	/*note that the flags are the same as the possible values for mode*/
	case _IONBF:
		/* file is unbuffered */
		stream->_flag |= _IONBF;
		stream->_base = malloc((unsigned)(_SBFSIZ + 8 + 2*MB_LEN_MAX));
		if(stream->_base == NULL)
			return (-1);
		stream->_base += 2 * MB_LEN_MAX;
		stream->_flag |= _IOMYBUF;
		_bufend(stream) = stream->_base + _SBFSIZ;
		break;
	case _IOLBF:
	case _IOFBF:
		stream->_flag |= mode;
		/* If user's buffer is unreasonably small, ignore it. */
		if (buf && size >= 128) {
			size -= (8 + 2 * MB_LEN_MAX);
			stream->_base = (unsigned char *)buf;
		} else {
			size = (size == 0) ? BUFSIZ : size;
			stream->_base =
				malloc((unsigned)(size + 8 + 2 * MB_LEN_MAX));
			if(stream->_base == NULL)
				return (-1);
			stream->_flag |= _IOMYBUF;
		}
		stream->_base += 2 * MB_LEN_MAX;
		_bufend(stream) = stream->_base + size;
		break;
	default:
		return (-1);
	}
	stream->__newbase = (char *)(stream->_ptr = stream->_base);
	stream->_cnt = 0;
	return (0);
}

#ifdef	_THREAD_SAFE
int 	
setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
	register int rc;
	TS_FDECLARELOCK(filelock)

	/* Check stream argument before attempting to use it */
	if (stream == NULL) {
		TS_SETERR(EFAULT);
		return -1;
	}
	TS_FLOCK(filelock, stream);
	rc = setvbuf_unlocked(stream, buf, mode, size);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
