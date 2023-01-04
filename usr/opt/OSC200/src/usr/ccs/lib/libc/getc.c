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
static char	*sccsid = "@(#)$RCSfile: getc.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/07 22:57:13 $";
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
 * FUNCTIONS: getc 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * getc.c	1.8  com/lib/c/io,3.1,8943 9/12/89 18:21:51
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak getc_unlocked = __getc_unlocked
#endif
#endif
#include <errno.h>
#include <stdio.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#undef getc_unlocked
#ifdef _NAME_SPACE_WEAK_STRONG
#define getc_unlocked __getc_unlocked
#endif
#endif	/* _THREAD_SAFE */
#undef getc

/*
 * FUNCTION:	A subroutine version of the macro getc.  This function was
 *		created to meet ANSI C standards.  The getc function obtains
 *		the next character (if present) as an unsigned char
 *		converted to an int, from the file pointed to by stream, and
 *		advances the associated file position indicator for the stream.
 *		POSIX 1003.4a defines this to be locked by default so the
 *		single thread version becomes getc_unlocked() and a new
 *		thread safe version is defined with the same interface.
 *
 * RETURN VALUE DESCRIPTION:	
 *		Returns the next character from the input stream pointed to
 *		by stream.  If the stream is at end-of-file, the end-of-file
 *		indicator for the stream is set and getc returns EOF.  If a 
 *		read error occurs, the error indicator for the stream is set 
 *		and getc returns EOF.
 *
 */  

#ifdef _THREAD_SAFE
int	
getc_unlocked(FILE *stream)
#else
int	
getc(FILE *stream)
#endif	/* _THREAD_SAFE */
{
	if ((stream->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
		TS_SETERR(EBADF);
		return(EOF);
	}
	if (--(stream)->_cnt < 0)
		return(__filbuf(stream));
	else
		return((int) *(stream)->_ptr++);
}

#ifdef _THREAD_SAFE
int	
getc(FILE *stream)
{
	register int        rc;
	TS_FDECLARELOCK(filelock)

        TS_FLOCK(filelock, stream);
	if ((stream->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
		TS_SETERR(EBADF);
                TS_FUNLOCK(filelock);
		return(EOF);
	}
	if (--(stream)->_cnt < 0)
		rc = __filbuf(stream);
	else
		rc = (int) *(stream)->_ptr++;
	TS_FUNLOCK(filelock);
	return(rc);
}
#endif	/* _THREAD_SAFE */
