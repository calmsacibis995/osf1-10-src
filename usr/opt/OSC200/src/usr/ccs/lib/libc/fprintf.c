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
static char	*sccsid = "@(#)$RCSfile: fprintf.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/11/18 15:39:26 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * fprintf.c
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
 * FUNCTIONS: fprintf 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fprintf.c	1.13  com/lib/c/prnt,3.1,8943 9/7/89 10:15:30
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif
#include <stdarg.h>
#include <errno.h>
#include "ts_supp.h"

extern int _doprnt();
/*VARARGS2*/

/*                                                                    
 * FUNCTION: Writes output to to the stream pointed to by stream, under
 *           control of the string pointed to by format, that specifies
 *           how subsequent arguments are converted for output.
 *
 * PARAMETERS: stream - stream to be printed to
 *             format - format used to print arguments
 *	       ...    -   arguments to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      If successful, returns number of characters printed
 *	      Otherwise returns negative value
 */

#pragma weak NLfprintf = fprintf

int	
#ifndef	_NO_PROTO
fprintf(FILE *stream, const char *format, ...) 
#else
fprintf(stream, format, va_alist)
FILE *stream;
const char *format;
va_dcl
#endif
{
	int count;
	va_list ap;
	register int err;
        TS_FDECLARELOCK(filelock)

	va_start(ap,format);
	TS_FLOCK(filelock, stream);
	if (!(stream->_flag & _IOWRT)) {
		/* if no write flag */
		if (stream->_flag & _IORW) {
			/* if ok, cause read-write */
			stream->_flag |= _IOWRT;
		} else {
			/* else error */
			seterrno(EBADF);
                        TS_FUNLOCK(filelock);
			return EOF;
		}
	}
	count = _doprnt(format, ap, stream);
#ifdef	_THREAD_SAFE
	err = ferror_unlocked(stream)? EOF: count;
#else
	err = ferror(stream)? EOF: count;
#endif	/* _THREAD_SAFE */
        TS_FUNLOCK(filelock);
	va_end(ap);
	return err;
}
