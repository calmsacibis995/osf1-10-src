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
static char	*sccsid = "@(#)$RCSfile: fseek.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:54:55 $";
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
 * FUNCTIONS: fseek 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fseek.c	1.11  com/lib/c/io,3.1,9013 2/14/90 10:41:13
 */

/*LINTLIBRARY*/
/*
 * Seek for standard library.  Coordinates with buffering.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak fseek_unlocked = __fseek_unlocked
#endif
#endif
#include <stdio.h>
#include <unistd.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#	define	FFLUSH	fflush_unlocked
#	define	FILENO	fileno_unlocked
#else
#	define	FFLUSH	fflush
#	define	FILENO	fileno
#endif /* _THREAD_SAFE */


int	
#ifdef _THREAD_SAFE
fseek_unlocked(FILE *stream, long int offset, int whence)
#else
fseek(FILE *stream, long int offset, int whence)
#endif	/* _THREAD_SAFE */
{
	long	cnt, adj;
	off_t	errstat;

	stream->_flag &= ~_IOEOF;
	if (stream->_flag & _IOREAD) {

		if (whence < SEEK_END && stream->_base
		    && !(stream->_flag & (_IONBF | _IOUNGETC)) ) {
			/*
			 * Doing SEEK_SET or SEEK_CUR and we've got a buffer,
			 * and it's not a non-buffered stream or a stream that
			 * we've pushed back characters into.
			 *
			 * Idea here is to avoid doing actual i/o if the fseek()
			 * would be in the buffer that's presently available.
			 */
			cnt = stream->_cnt;
			adj = offset;

			/*
			 * adjust relative to the current file posn
			 */
			if (whence == SEEK_SET) {
			    	off_t curpos = lseek(FILENO(stream), (off_t)0L, SEEK_CUR);
				if (curpos == -1)
				    return(-1);

				adj += cnt - curpos;
			} else
				offset -= cnt;

			if (!(stream->_flag & _IORW) && cnt > 0 && adj <= cnt
			    && adj >= (unsigned char *)stream->__newbase
				    - stream->_ptr) {
				/*
				 * Not writable stream, the buffer still has
				 * chars, and the new position is within
				 * the buffer.
				 * Adjust ptr and cnt by relative motion.
				 */
				stream->_ptr += (int)adj;
				stream->_cnt -= (int)adj;
				return (0);
			}
		} else if (whence == SEEK_CUR)
			offset -= stream->_cnt;

		stream->_flag &= ~_IOUNGETC;

		if (stream->_flag & _IORW) {
			stream->__newbase =
				(char *)(stream->_ptr = stream->_base);
			stream->_flag &= ~_IOREAD;
		}
		errstat = lseek(FILENO(stream), (off_t)offset, whence);
		stream->_cnt = 0;

	} else if (stream->_flag & (_IOWRT | _IORW)) {

		errstat = FFLUSH(stream);

		if (stream->_flag & _IORW) {
			stream->_cnt = 0;
			stream->_flag &= ~_IOWRT;
			stream->__newbase =
				(char *)(stream->_ptr = stream->_base);
		}

		if (errstat)
			(void)lseek(FILENO(stream), (off_t)offset, whence);
		else
			errstat = lseek(FILENO(stream), (off_t)offset, whence);
	} else
		errstat = -1;		/* invalid or closed file block ptr */

	return ((errstat == -1) ? -1 : 0);
}


#ifdef	_THREAD_SAFE
int	
fseek(FILE *stream, long int offset, int whence)
{
	register int	rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = fseek_unlocked(stream, offset, whence);
	TS_FUNLOCK(filelock);
	return (rc);
}


#endif	/* _THREAD_SAFE */
