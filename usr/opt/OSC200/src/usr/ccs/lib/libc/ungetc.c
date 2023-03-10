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
static char	*sccsid = "@(#)$RCSfile: ungetc.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:42:37 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: ungetc 
 *
 * ORIGINS: 3, 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *  1.15  com/lib/c/io/ungetc.c, , bos320, 9134320 8/16/91 15:28:02
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
#include "stdiom.h"
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return (TS_FUNLOCK(filelock), (val))

#else
#define	RETURN(val)	return (val)
#endif	/* _THREAD_SAFE */

int	
ungetc(int c, FILE *stream)
{
	TS_FDECLARELOCK(filelock)

	if (c == EOF)
		return (-1);
	TS_FLOCK(filelock, stream);

	if (stream->_base == NULL && _findbuf(stream))
		RETURN(-1);

	/* ungetc gets complicated because now a failed wchar read function
	 * can leave _ptr==_base with valid data.In such a case we should
	 * allow ungetc to overflow into the _base - 2 * MB_LEN_MAX area.
	 * We do not want to use the overflow area when a process is not
	 * using wide character functions on this stream, this is needed to
	 * avoid breaking existing applications for which _ptr normally is
	 * not less than _base.
	 * Also if the _ptr is already in the overflow space then we allow
	 * ungetc to succeed till the __newbase reaches _base - 2 * MB_LEN_MAX.
	 */

	if ((stream->_flag & _IOREAD) == 0
	    || stream->_ptr <= (unsigned char *)stream->__newbase) {
		if ((stream->_ptr==(unsigned char *)stream->__newbase)
		    && !(stream->_flag & _IOWRT)) {
			if (stream->_cnt == 0) {
				/* The buffer is actually empty and so the
				 * unget byte, can be placed at the base of
				 * the buffer.
				 */
				++stream->_ptr;
			} else {	/* _ptr == __newbase and _cnt != 0 */

				/* If _ptr == __newbase was reached by ungetc
				 * or ungetwc or __newbase cannot be moved any
				 * further return an error.
				 */
				if ((stream->_cnt > 0
				     && (stream->_flag & (_IOUNGETC|_IONOFD)))
				    || (unsigned char *)stream->__newbase ==
					stream->_base - 2 * MB_LEN_MAX) {

					RETURN(-1);
				}

				/* We are ready to move __newbase back one byte,
				 * to unget the current byte. The cal to
				 * __getmybuf would support delayed allocation
				 * of the stream buffer
				 */
				if (!stream->_flag & _IOMYBUF) {
					__getmybuf(stream);
				}
				--stream->__newbase;
			}
		} else {
			/* Either _ptr < __newbase (highly improbable),
			 * or _IOWRT is set .In  either case we are not
			 * supposed to unget the byte.
			 */
			RETURN(-1);
		}
	}


	/* If the stream is really a string (possibly in read-only storage),
	 * can't write to it.
	 * If the unget char is already there we needn't set _IOUNGETC.
	 */
	if (stream->_flag & _IONOFD)
		--stream->_ptr;
	else if (*--stream->_ptr != c) {
		/* mark stream as having ungetc() chars on it.  See fseek() */
		stream->_flag |= _IOUNGETC;
		*stream->_ptr = c;
	}
	stream->_flag &= ~_IOEOF ;
	++stream->_cnt;

	/* clear EOF */
	clearerr(stream);
	if (stream->_flag &  _IORW)
		stream->_flag |= _IOREAD;
	RETURN(c);
}


int
__getmybuf(FILE *stream)
{
	return (0);
}
