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
static char	*sccsid = "@(#)$RCSfile: putwc.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/05 21:03:08 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * FUNCTIONS: putwc 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * putwc.c	1.5  com/lib/c/io,3.1,8943 9/9/89 13:30:54
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak putwc = __putwc
#pragma weak fputwc = __putwc
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
#include <limits.h>
#include "stdiom.h"
#include <errno.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#undef putwc
#ifdef _NAME_SPACE_WEAK_STRONG
#define putwc __putwc
#endif


/*
 * FUNCTION:    A subroutine version of the macro putc.  This function is
 *              created to meet ANSI C standards.  The putc function writes
 *              the character specified by c (converted to an unsigned char)
 *              to the output stream pointed to by stream, at the position
 *              indicated by the assoicated file poistion indicator for the
 *              stream, and advances the indicator appropriately.
 *
 * RETURN VALUE DESCRIPTION:	
 *              The putwc function returns the character written.  If a write
 *              error occurs, the error indicator for the stream is set and
 *              putc returns WEOF.
 *
 */
wint_t
putwc(wint_t c, FILE *stream)
{
	int	i, rc, ch;
        char	mbstr[MB_LEN_MAX], *mb;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

        if(MB_CUR_MAX == 1) {
		ch = putc(c, stream);
		TS_FUNLOCK(filelock);
		return (ch == EOF ? WEOF : (wint_t) ch);
	}

        if ((rc = wctomb(mbstr, c)) <= 0) {
                stream->_flag |= _IOERR;
                _Seterrno(EILSEQ);
                TS_FUNLOCK(filelock);
                return (WEOF);
        }

        if (stream->_cnt - rc < 0) {
                mb = mbstr;
                for(i = 0; i < rc; i++) {
                        if (putc(*mb++, stream) == EOF) {
				TS_FUNLOCK(filelock);
                                return (WEOF);
			}
		}
		TS_FUNLOCK(filelock);
                return (c);
        }
	memcpy(stream->_ptr,mbstr,rc);
	stream->_ptr += rc;
	stream->_cnt -= rc;
	_BUFSYNC(stream);
	TS_FUNLOCK(filelock);
	return (c);
}
