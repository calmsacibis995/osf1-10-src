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
static char	*sccsid = "@(#)$RCSfile: ungetwc.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:49:48 $";
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
 * FUNCTIONS: ungetwc 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.10  com/lib/c/io/ungetwc.c, libcio, bos320, 9130320 7/16/91 11:40:06
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak ungetwc = __ungetwc
#endif
#endif
#include <stdio.h>
#include <wchar.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef  _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */


wint_t	
ungetwc(wint_t c, FILE *stream)
{
	int bytesinchar;
	unsigned char *new_base;
	char mbstr[MB_LEN_MAX];
	TS_FDECLARELOCK(filelock)

	if (c == WEOF)
		return (WEOF);
	TS_FLOCK(filelock, stream);
	if (stream->_base == NULL && _findbuf(stream)) {
		TS_FUNLOCK(filelock);
		return (WEOF);
	}

	if (((bytesinchar = wctomb(mbstr,c)) == -1)
	    || ((stream->_base - 2 * MB_LEN_MAX)
		> (new_base = stream->_ptr - bytesinchar))
	    || (stream->_flag & _IOWRT)) {
		TS_FUNLOCK(filelock);
		return (WEOF);
	}

	/* If the stream is really a string (possibly in read-only storage),
	   can't write to it. */
	if (stream->_flag & _IONOFD) {
		if (new_base < stream->_base) {
			TS_FUNLOCK(filelock);
			return (WEOF);
		}
		stream->_ptr = new_base;
	}
	else {
		/* mark stream as having ungetc() chars on it.  See fseek() */
		if (memcmp(stream->_ptr = new_base, mbstr, bytesinchar)) {
			stream->_flag |= _IOUNGETC;
			memcpy(new_base, mbstr, bytesinchar);
		}
		if (stream->_ptr < (unsigned char *)stream->__newbase)
			stream->__newbase = (char *)stream->_ptr;
	}
	stream->_flag &= ~_IOEOF ;
	stream->_cnt += bytesinchar;
	TS_FUNLOCK(filelock);
	return (c);
}
