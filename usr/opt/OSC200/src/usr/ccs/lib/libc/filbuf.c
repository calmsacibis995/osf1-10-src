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
static char	*sccsid = "@(#)$RCSfile: filbuf.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/10/05 21:00:53 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 */ 
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: __filbuf 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.12  com/lib/c/io/filbuf.c, libcio, bos320, 9125320 6/8/91 18:41:48
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
#include <unistd.h>
#include <errno.h>

#include "glue.h"
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

extern struct rec_mutex _iobptr_rmutex;
#endif /* _THREAD_SAFE */

extern _findbuf();
extern struct glued _glued;

#pragma weak _filbuf = __filbuf

/*
int
_filbuf(FILE *iop)
{
  return(__filbuf(iop)) ;
}
*/

int
__filbuf(iop)
register FILE *iop;
{
	register FILE *diop;
	register int i;
	TS_FDECLARELOCK(filelock)

	/* get buffer if we don't have one */
	if (iop->_base == NULL && _findbuf(iop))
		return (EOF);

	if ( !(iop->_flag & _IOREAD) )
		if (iop->_flag & _IORW)
			iop->_flag |= _IOREAD;
		else {
			iop->_flag |= _IOERR;
			seterrno(EBADF);
			return (EOF);
		}

	/* if this device is a terminal (line-buffered) or unbuffered, then
	 * flush buffers of all line-buffered devices currently writing
	 */
	if (iop->_flag & (_IOLBF | _IONBF)) {

		/* TS: Grab iobptr_rmutex for the duration of the flush to save
		 * repeated relocking to find the next iob (tradeoff).
		 */
		TS_LOCK(&_iobptr_rmutex);
		for (i = 0; i < _glued.lastfile; i++) {
			diop = &_glued.iobptr[ i >> 4][i & ~0xfff0];
			/* do not flush the current stream */
			if (diop != iop && diop->_flag & _IOLBF) {
#ifdef  _THREAD_SAFE
				/* Avoid deadlock by only flushing file
				 * streams iff we can lock them, otherwise
				 * assume they're busy and leave alone.
				 */
				if (!TS_FTRYLOCK(filelock, diop))
					continue;
				(void) fflush_unlocked(diop);
				TS_FUNLOCK(filelock);
#else
				(void) fflush(diop);
#endif /* _THREAD_SAFE */
			}
		}
		TS_UNLOCK(&_iobptr_rmutex);
	}

	iop->_flag &= ~_IOUNGETC;
	iop->_ptr = iop->_base;
	iop->_cnt = read((int)fileno(iop), (char *)iop->_base,
			(unsigned)((iop->_flag & _IONBF) ? 1 : _bufsiz(iop) ));
	if (--iop->_cnt >= 0)		/* success */
		return (*iop->_ptr++);
	if (iop->_cnt != -1)		/* error */
		iop->_flag |= _IOERR;
	else {				/* end-of-file */
		iop->_flag |= _IOEOF;
		if (iop->_flag & _IORW)
			iop->_flag &= ~_IOREAD;
	}
	iop->_cnt = 0;
	return (EOF);
}

/*
 *	_wcfilbuf() is essentially the same as __filbuf except that it is
 *	only called by wide character read routines when they encounter a
 *	wide character straddling a buffer boundary.
 *	_wcfilbuf() places the existing contents of the buffer in the area
 *	before _b ase and then tries to populate the buffer from the underlying
 *	file._wcfilbuf() loops around the read till it has atleast charbytes
 *	number of charcters in the buffer or it encounters end-of-file or read
 *	error.
 *	Returns EOF on failure.
 */

int
_wcfilbuf(iop,charbytes)
register FILE *iop;	/* stream to read from */
int charbytes;		/* need to have these many chars in buffer to form a
			 * valid character.
			 */
{
	register FILE *diop;
	register int i,savecount,rc;
	register char *nextstart;
	TS_FDECLARELOCK(filelock)

	/* get buffer if we don't have one */
	if (iop->_base == NULL && _findbuf(iop))
		return (EOF);

	if ( !(iop->_flag & _IOREAD) )
		if (iop->_flag & _IORW)
			iop->_flag |= _IOREAD;
		else {
			iop->_flag |= _IOERR;
			seterrno(EBADF);
			return(EOF);
		}

	nextstart= (char *)iop->_base;

	/* if this device is a terminal (line-buffered) or unbuffered, then
	 * flush buffers of all line-buffered devices currently writing
	 */
	if (iop->_flag & (_IOLBF | _IONBF)) {

		/* TS: Grab iobptr_rmutex for the duration of the flush to save
		 * repeated relocking to find the next iob (tradeoff).
		 */
		TS_LOCK(&_iobptr_rmutex);
		for (i = 0; i < _glued.lastfile; i++) {
			diop = &_glued.iobptr[ i >> 4][i & ~0xfff0];
			/* do not flush the current stream */
			if (diop != iop && diop->_flag & _IOLBF) {
#ifdef  _THREAD_SAFE
				/* Avoid deadlock by only flushing file
				 * streams iff we can lock them, otherwise
				 * assume they're busy and leave alone.
				 */
				if (!TS_FTRYLOCK(filelock, diop))
					continue;
				(void) fflush_unlocked(diop);
				TS_FUNLOCK(filelock);
#else
				(void) fflush(diop);
#endif /* _THREAD_SAFE */
			}
		}
		TS_UNLOCK(&_iobptr_rmutex);
	}

	iop->_flag &= ~_IOUNGETC;
	if (iop->_cnt > 0) {
		savecount=iop->_cnt;
		memcpy(iop->__newbase = (char *)iop->_base - savecount,
			iop->_ptr,savecount);
		iop->_ptr= (unsigned char *)iop->__newbase;

	} else {
		iop->__newbase= (char *)(iop->_ptr=iop->_base);
		iop->_cnt=0;
	}
	for(;;) {
		if ((rc = read((int)fileno(iop), (char *) nextstart,
			       (unsigned)((iop->_flag & _IONBF) ? 1 :
			       _bufend(iop)-(unsigned char *)nextstart))) > 0) {

			/* success */
			if ((iop->_cnt += rc) >= charbytes)
				return (0);
			nextstart += rc;
			continue;

		}
		if (rc == -1) {		/* error */
			iop->_flag |= _IOERR;
		}
		else {			/* end-of-file */
			if (!iop->_cnt) {
				iop->_flag |= _IOEOF;
				if (iop->_flag & _IORW)
					iop->_flag &= ~_IOREAD;
			}
			iop->_flag |= _IOERR;
			seterrno(EILSEQ);
		}
		return (EOF);
	}
}
