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
static char	*sccsid = "@(#)$RCSfile: fputws.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:54:33 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: fputws 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * com/lib/c/io/fputws.c, bos320 2/26/91 17:54:12
 */

/*LINTLIBRARY*/

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak fputws = __fputws
#endif
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <errno.h>
#include <wchar.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return(TS_FUNLOCK(filelock), (val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */

extern char *memccpy();


/*
 * FUNCTION: The  fputws subroutine  writes the  null-terminated string
 *           pointed to by the s parameter to the stream.
 *
 *           This version writes directly to the buffer rather than looping
 *           on putwc.  Ptr args aren't checked for NULL because the program
 *           would be a catastrophic mess anyway.  Better to abort than just
 *           to return NULL.
 *
 * PARAMETERS: wchar_t *s      - NULL-terminated string to be written to
 *             FILE *stream	 the stream
 * RETURN VALUE DESCRIPTIONS:
 *           Upon successful completion, the putws subroutine returns
 *	     the number of characters written.  putws returns EOF on an
 *	     error.  This happens if the routines try to write on a file
 *	     that has not been opened for writing.
 */
int
fputws(const wchar_t *s, FILE *stream)
{
	int		err = 0, ndone = 0; 
	int		n, rc, wccnt, bufsize, i;
	unsigned char	*bufend, *retptr, mbstr[MB_LEN_MAX], *mb;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (_WRTCHK(stream)) {
		RETURN(-1);
	}

	bufend = _bufend(stream);
	bufsize = _bufsiz(stream);

	for (wccnt = wcslen(s) ;wccnt > 0 ; ) {

		if (err > bufsize) {	/* the buffer does have space */
			rc = wctomb(mb = mbstr, s[ndone]);
			if (rc == -1) {			/* Conversion error */
				stream->_flag |= _IOERR;
				_Seterrno(EILSEQ);
				RETURN(-1);
			}

			for(i = 0; i < rc; i++) {
				if (putc(*mb++, stream) == EOF) {
					RETURN(-1);
				}
			}
			err = 0; wccnt--; ndone++;
			continue;
		}
		/* check for full buf */
		while ((n = bufend - stream->_ptr) < err || n == 0) {
			if (_xflsbuf(stream) == EOF) {
				RETURN(-1);
			}
		}

		err = 0;	/* this is not reqd if __pcstombs sets it */

		/* Try to convert the process code string into byte string
		 * and place the string into the stdio buffer.
		 * __pcstombs(
		 *    wchar_t *  - start point of process code string.
		 *    size_t     - stop converting after these many wchars.
		 *    char *     - start placing the converted bytes here.
		 *    size_t     - amount of bytes available in the byte string.
		 *    char **    - returns the pointer to a position in byte
		 *   		   string where the next converted character
		 *  		   would have been placed.
		 *    int  *     - The return error indicator.
		 *                 0 => success.
		 *		  -1 => illegal wchar in wchar string.
		 *		  >0 => There was not enough space in the byte
		 *                      string after the last character to 
		 *			place the next character.The return
		 *			value indicates the number of bytes
		 *			reqd. to store the next character.
		 *           )
		 *
		 *   __pcstombs() returns number of wide characters converted
		 *   or -1 if it is not supported for this code set.
		 */
		if ((rc = __pcstombs(s + ndone, wccnt, stream->_ptr,
				     n, &retptr, &err)) > 0) {
			if (err == -1) {
				if (ndone >0 && 
				    stream->_flag & (_IONBF | _IOLBF))
					_xflsbuf(stream);
				stream->_flag |= _IOERR;
				_Seterrno(EILSEQ);
				RETURN(-1);
			}
			stream->_cnt -= (retptr - stream->_ptr);
			stream->_ptr = retptr;
			wccnt -= rc;
			ndone += rc;
			_BUFSYNC(stream);
			continue;	
		}
		else {
			if (rc == 0)  {
				/* err>0 We failed as there was not 
				 * enough space in the buffer
				 * to accomodate even one character.
				 */
				if (err > 0) 
					continue;

				/* err == 0, shouldn' happen ? */
				if (err == 0)
					break;

				/* else err == -1, illegal sequnce 
				 * flush buffer if reqd and exit *
				 */
				if (ndone > 0
				    && stream->_flag & (_IONBF | _IOLBF))
					_xflsbuf(stream);
				stream->_flag |= _IOERR;
				_Seterrno(EILSEQ);
				RETURN(-1);
			}
			else {
				/* rc == -1 => __pcstombs not supported *
				 * For now we return -1 later we can substitute
			  	 * code for implementing putws() using putwc()
				 * in here.
				 */
				wchar_t c, *start = (wchar_t *)s;

				while (c = *s++)
					if (putwc(c, stream)==WEOF) {
						RETURN(WEOF);
					}
				RETURN(s - start -1);
			}
		}
	}
	_BUFSYNC(stream);
	if (stream->_flag & (_IONBF | _IOLBF))
		if (_xflsbuf(stream) == EOF) {
			RETURN(-1);
		}
	RETURN(ndone);
}
