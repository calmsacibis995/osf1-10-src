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
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * FUNCTIONS: getwc 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * getwc.c	1.4  com/lib/c/io,3.1,8943 9/12/89 18:33:35
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak getwc = __getwc
#pragma weak fgetwc = __getwc
#if defined(_THREAD_SAFE)
#pragma weak getwc_unlocked = __getwc_unlocked
#endif
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


/*
 * FUNCTION:	
 * Get a multi-byte character and return it as a wide char.
 *
 * RETURN VALUE DESCRIPTION:	
 * Returns WEOF if getc returns EOF.
 *
 */  
wint_t
#ifdef	_THREAD_SAFE
getwc_unlocked(FILE *stream)
#else
getwc(FILE *stream)
#endif	/* _THREAD_SAFE */
{
	int	mbcurmax,charlen,ch;
	wchar_t	wc;
	int	err,err1;

	/* * * * * * * * * * * * *
	 * single byte code set	 *
 	* * * * * * * * * * * * */

	if (MB_CUR_MAX == 1)
		return(((ch = (getc(stream))) == EOF) ? WEOF : ch);

	/* * * * * * * * * * * * * * * * * *
 	 * multi byte code set		   *
	 * * * * * * * * * * * * * * * * * */

	if (stream->_cnt <= 0) {	/* If buffer empty get data from file */
		if (__filbuf(stream) == EOF)
			return (WEOF);
		else {
			stream->_ptr--;
			stream->_cnt++;
		}
	}

	if ((charlen = __mbtopc(&wc, stream->_ptr, stream->_cnt, &err)) > 0 ) {
		stream->_ptr += charlen;
		stream->_cnt -= charlen;
		return (wc);
	}
	else {
		if (charlen == -1) {
			/* __mbtopc() not supported for this code set.
			 * implement a version of getwc() using mbtowc() * here.
			 * For now this is a stub.
			 */
			stream->_flag |= _IOERR;
			_Seterrno(EILSEQ);
			return (WEOF);
		}

		/* Either we got an illegal sequence or the character straddles
		 * the buffer boundary */
		if (err > 0) {
			/* There were not enough bytes in the buffer to form
			 * the character.
			 * Could be a split char or an illegal sequence
			 * Try to read some more from the underlying file
			 * so that we have atleast err bytes in the buffer.
			 */

			if (_wcfilbuf(stream, err) == EOF) {
				/*
				 * Couldn't get enough bytes to complete the
				 * character. Could be due to a physical read
				 * error or we hit the end-of-file.
				 * errno and _IOERR flag are set in wcfilbuf().
				 */
				return (WEOF);
			}

			/* Try converting again */

			if ((charlen = __mbtopc(&wc, stream->_ptr,
						err, &err1)) > 0) {
				/* Got a valid character straddling the buffer
				 * boundary
				 */
				stream->_cnt -= charlen;
				stream->_ptr += charlen;
				return (wc);
			}

			/* Has to be an illegal sequence */

			stream->_flag |= _IOERR;
			_Seterrno(EILSEQ);
			return (WEOF);
		}
		else {	/* err == -1, an illegal sequence */
			stream->_flag |= _IOERR;
			_Seterrno(EILSEQ);
			return (WEOF);
		}
	}
}


#ifdef	_THREAD_SAFE
wint_t
getwc(FILE *stream)
{
	register wint_t rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	rc = getwc_unlocked(stream);
	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
