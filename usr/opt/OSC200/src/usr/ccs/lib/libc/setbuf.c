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
static char	*sccsid = "@(#)$RCSfile: setbuf.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:36:52 $";
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
 * FUNCTIONS: setbuf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * setbuf.c	1.9  com/lib/c/io,3.1,8943 10/19/89 17:18:48
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <stdlib.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern int isatty();
extern unsigned char *_stdbuf[];


void 	
setbuf(FILE *stream, char *buf)
{
	int fno;  /* file number */
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	fno = fileno(stream);

	if (stream->_base != NULL && stream->_flag & _IOMYBUF) {
		free((void *)(stream->_base - 2 * MB_LEN_MAX));
		stream->_base = NULL;
	}
	stream->_flag &= ~(_IOMYBUF | _IONBF | _IOLBF);

	if ((stream->_base = (unsigned char*)buf) == NULL) {
		stream->_flag |= _IONBF; /* file unbuffered except in fastio */

		if (fno < 2)  /* for stdin, stdout, use the existing bufs */
			_bufend(stream) = (stream->_base =
					  (_stdbuf[fno] + 2 * MB_LEN_MAX))
					  + BUFSIZ;

		else {	/* otherwise, use small buffer */
			stream->_base = (unsigned char *)
					malloc((size_t)_SBFSIZ + 8
						+ 2 * MB_LEN_MAX);

			if (stream->_base != NULL)
				stream->_base += 2 * MB_LEN_MAX;
			stream->_flag |= _IOMYBUF;
			_bufend(stream) = stream->_base + _SBFSIZ;
		}
	}
	else {  /* regular buffered I/O, ASSUMES standard buffer size */
		_bufend(stream) = stream->_base
				  + (BUFSIZ - (8 + 2 * MB_LEN_MAX));
		if (isatty(fno))
			stream->_flag |= _IOLBF;
	}
	stream->__newbase = (char *)(stream->_ptr = stream->_base);
	stream->_cnt = 0;
	TS_FUNLOCK(filelock);
}
