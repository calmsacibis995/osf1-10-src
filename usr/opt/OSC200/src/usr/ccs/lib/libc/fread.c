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
static char	*sccsid = "@(#)$RCSfile: fread.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/07 22:54:46 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * fread.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: fread 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fread.c	1.12  com/lib/c/io,3.1,8943 10/18/89 11:00:04
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak fread_unlocked = __fread_unlocked
#endif
#endif
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

#define MIN(x, y)	(x < y ? x : y)

extern int __filbuf();

/*
 * FUNCTION:	The fread function reads, into the array pointed to by
 *		ptr, up to nmemb members whose size is specified by size, 
 *		from the stream pointed to be stream.
 *
 * 		This version reads directly from the buffer rather than
 *		looping on getc.  Ptr args aren't checked for NULL because
 *		the program would be a catastrophic mess anyway.  Better
 *		to abort than just to return NULL.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The fread function returns the number of members successfully
 *		read, which may be less than nmemb if a read error or end-of-
 *		file is encountered.  If size or nmemb is zero, fread returns
 *		zero and the contents of the array and the state of the
 *		stream remains unchanged.
 *
 */  


size_t
#ifdef _THREAD_SAFE
fread_unlocked(void *ptr, size_t size, size_t nmemb, FILE *stream)
#else
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
#endif	/* _THREAD_SAFE */
{
	unsigned nleft;
	int n;
	if (size <= 0 || nmemb <= 0)
		return (0);

        /* Check if stream was opened for reading */
        if (!(stream->_flag & _IOREAD)) {
                if (stream->_flag & _IORW) {
                        stream->_flag |= _IOREAD;
                }
                else {
                        seterrno(EBADF);
                        return (0);
                }
        }

	for (nleft = nmemb * size; ; ) {
		if (stream->_cnt <= 0) { /* empty buffer */
			if (__filbuf(stream) == EOF) {
				return (nmemb - (nleft + size - 1)/size);
			}
			stream->_ptr--;
			stream->_cnt++;
		}
		n = MIN(nleft, stream->_cnt);
		ptr = (char *)memcpy(ptr, (void *) stream->_ptr, (size_t)n) + n;
		stream->_cnt -= n;
		stream->_ptr += n;
		_BUFSYNC(stream);
		if ((nleft -= n) == 0) {
			return (nmemb);
		}
	}
}

#ifdef	_THREAD_SAFE
size_t 	
fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	register size_t rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = fread_unlocked(ptr, size, nmemb, stream);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
