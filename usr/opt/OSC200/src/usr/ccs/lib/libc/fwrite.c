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
static char	*sccsid = "@(#)$RCSfile: fwrite.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:56:50 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * fwrite.c
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
 * FUNCTIONS: fwrite 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.18  com/lib/c/io/fwrite.c, libcio, bos320, 9130320 7/16/91 11:03:16
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak fwrite_unlocked = __fwrite_unlocked
#endif
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <string.h>
#include "stdiom.h"
#include <errno.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


/*
 * FUNCTION:	The fwrite function writes, from the array pointed to by ptr,
 *		up to nmemb mebers whose size is specified by size, to the
 *		stream pointed to by stream.
 *
 * 		This version reads directly from the buffer rather than
 *		looping on putc.  Ptr args aren't checked for NULL because
 *		the program would be a catastrophic mess anyway.  Better
 *		to abort than just to return NULL.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The fwrite function returns the number of members successfully
 *		written, which will be less than nmemb only if a write error
 *	 	is encountered.
 *
 */                                                                   
size_t	
#ifdef	_THREAD_SAFE
fwrite_unlocked(const void *ptr, size_t size, size_t nmemb, FILE *stream)
#else
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
#endif	/* _THREAD_SAFE */
{
	unsigned nleft;
	int n;
	unsigned char *cptr, *bufend;
	int nwritten;

	/* return 0 when size or nmemb is 0 or the stream is not writable */

	if (size == 0 || nmemb == 0 || _WRTCHK(stream))
		return (0);

	bufend = _bufend(stream);
	nleft = nmemb*size;

	/* if the file is unbuffered, or if the buffer is empty and we are
	   writing more than a buffer full, do a direct write */
	if (stream->_base >= stream->_ptr)  {
		/* this covers the unbuffered case, too */
		if (stream->_flag & _IONBF
		    || nleft >= bufend - stream->_base)  {
			n = 0;
			cptr = (unsigned char *)ptr;
			while (nleft != 0) {

			  	nwritten = write(fileno(stream),
						 (cptr+n), nleft);

				if (nwritten > 0) {
					nleft -= nwritten;
					n += nwritten;
				} else {		/* write FAILED */
					stream->_flag |= _IOERR;
					break;		/* exit the WHILE */
				}
			}
			return (n/size);
		}
	}
	for (; ; ptr = (void *)((char *)ptr + n)) {
		while ((n = bufend - (cptr = stream->_ptr)) <= 0)  /* full buf */
			if (_xflsbuf(stream) == EOF) {
				return (nmemb - (nleft + size - 1)/size);
			}
		if (n > nleft) n = nleft;
		(void) memcpy((void *)cptr, (void *)ptr, (size_t)n);
		stream->_cnt -= n;
		stream->_ptr += n;
		_BUFSYNC(stream);
		if ((nleft -= n) == 0)
			break;
	}
	/* flush if linebuffered with a newline */
	if (stream->_flag & _IOLBF
	    && memchr((void *)cptr, (int)'\n', (size_t)n))
		(void) _xflsbuf(stream);
	return (nmemb);
}


#ifdef	_THREAD_SAFE
size_t	
fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	register size_t rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = fwrite_unlocked(ptr, size, nmemb, stream);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
