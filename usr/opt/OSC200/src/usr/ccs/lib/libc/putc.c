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
static char	*sccsid = "@(#)$RCSfile: putc.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/07 23:36:32 $";
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
 * FUNCTIONS: putc 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * putc.c	1.8  com/lib/c/io,3.1,8943 9/9/89 13:29:58
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak putc_unlocked = __putc_unlocked
#endif
#endif
#include <stdio.h>
#include "ts_supp.h"
#include "stdiom.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef putc_unlocked
#ifdef _NAME_SPACE_WEAK_STRONG
#define putc_unlocked __putc_unlocked
#endif
#endif	/* _THREAD_SAFE */
#undef putc

/*
 * FUNCTION:	A subroutine version of the macro putc.  This function is
 *		created to meet ANSI C standards.  The putc function writes
 *		the character specified by c (converted to an unsigned char)
 *		to the output stream pointed to by stream, at the position
 *		indicated by the assoicated file poistion indicator for the
 *		stream, and advances the indicator appropriately.
 *		POSIX 1003.4a requires that this function is locked by default
 *		so an unlocked version is also provided.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The putc function returns the character written.  If a write
 *		error occurs, the error indicator for the stream is set and
 * 		putc returns EOF.
 *
 */  

#ifdef _THREAD_SAFE
int 	
putc_unlocked(int c, FILE *stream)
#else
int 	
putc(int c, FILE *stream)
#endif	/* _THREAD_SAFE */
{
	if (_WRTCHK(stream))
		return (EOF);
	if (--(stream)->_cnt < 0)
		return (_flsbuf((unsigned char) (c), (stream)));
	else
		return ((int) (*(stream)->_ptr++ = (unsigned char) (c)));
}

#ifdef	_THREAD_SAFE
int 	
putc(int c, FILE *stream)
{
	register int        rc;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (_WRTCHK(stream)) {
                TS_FUNLOCK(filelock);
                return (EOF);
        }
	if (--(stream)->_cnt < 0)
		rc = _flsbuf((unsigned char) (c), (stream));
	else
		rc = ((int) (*(stream)->_ptr++ = (unsigned char) (c)));

	TS_FUNLOCK(filelock);
	return (rc);
}
#endif	/* _THREAD_SAFE */
