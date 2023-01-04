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
static char	*sccsid = "@(#)$RCSfile: puts.c,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/06/07 21:19:41 $";
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
 * FUNCTIONS: puts 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * puts.c	1.10  com/lib/c/io,3.1,8943 9/9/89 13:30:28
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include "stdiom.h"
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern char *memccpy();

/*                                                                   
 * FUNCTION:	The puts function writes the string pointed to by s to the
 *		stream pointed to by stdout, and appends a new-line character
 *		to the output.  The terminating character is not written.
 *
 * 		This version reads directly from the buffer rather than
 *		looping on putc.  Ptr args aren't checked for NULL because
 *		the program would be a catastrophic mess anyway.  Better
 *		to abort than just to return NULL.
 *
 * RETURN VALUE DESCRIPTION:	
 *		The puts function returns EOF if a write error occurs;
 *		otherwise it returns a nonnegative value
 *
 */  

int 	
puts(const char *s)
{
	char	*p;
	int	ndone = 0, n;
	unsigned char	*cs, *bufend;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdout);

	if (_WRTCHK(stdout)) {
		TS_FUNLOCK(filelock);
		return (EOF);
	}

	bufend = _bufend(stdout);

	for ( ; ; s += n) {
		while ((n = bufend - (cs = stdout->_ptr)) <= 0) /* full buf */
			if (_xflsbuf(stdout) == EOF) {
				TS_FUNLOCK(filelock);
				return(EOF);
			}
		if ((p = memccpy((char *) cs, s, '\0', n)) != NULL)
			n = p - (char *) cs;
		stdout->_cnt -= n;
		stdout->_ptr += n;
		_BUFSYNC(stdout);
		ndone += n;
		if (p != NULL) {
			stdout->_ptr[-1] = '\n'; /* overwrite '\0' with '\n' */
			if (stdout->_flag & (_IONBF | _IOLBF)) /* flush line */
				if (_xflsbuf(stdout) == EOF) {
					TS_FUNLOCK(filelock);
					return(EOF);
				}
			TS_FUNLOCK(filelock);
			return(ndone);
		}
	}
}
