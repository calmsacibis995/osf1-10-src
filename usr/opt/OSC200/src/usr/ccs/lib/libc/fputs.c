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
static char	*sccsid = "@(#)$RCSfile: fputs.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:54:05 $";
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
 * FUNCTIONS: fputs 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fputs.c	1.9  com/lib/c/io,3.1,8943 9/12/89 17:56:55
 */

/*LINTLIBRARY*/
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
 * FUNCTION: The  fputs subroutine  writes the  null-terminated string
 *           pointed to by the s parameter to the output stream speci-
 *           fied by the stream  parameter.  The fputs subroutine does
 *           not append a new-line character.
 *
 *           This version writes directly to the buffer rather than looping
 *           on putc.  Ptr args aren't checked for NULL because the program
 *           would be a catastrophic mess anyway.  Better to abort than just
 *           to return NULL.
 *
 * PARAMETERS: char *s      - NULL-terminated string to be written to
 *             FILE *stream      - File to be written to
 *
 * RETURN VALUE DESCRIPTIONS:
 *           Upon successful completion, the fputs subroutine returns
 *	     the number of characters written.  fputs returns EOF on an
 *	     error.  This happens if the routines try to write on a file
 *	     that has not been opened for writing.
 */
int 	
fputs(const char *s, FILE *stream)
{
	int ndone = 0, n;
	unsigned char *cs, *bufend;
	char *p;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (_WRTCHK(stream)) {
		TS_FUNLOCK(filelock);
		return (EOF);
	}
	bufend = _bufend(stream);

	for ( ; ; s += n) {
		while ((n = bufend - (cs = stream->_ptr)) <= 0)  /* full buf */
			if (_xflsbuf(stream) == EOF) {
				TS_FUNLOCK(filelock);
				return(EOF);
			}
		if ((p = memccpy((char *) cs, s, '\0', n)) != NULL)
			n = (p - (char *) cs) - 1;
		stream->_cnt -= n;
		stream->_ptr += n;
		_BUFSYNC(stream);
		ndone += n;
		if (p != NULL)  { /* done; flush buffer if "unbuffered" or if
				     line-buffered */
			if (stream->_flag & (_IONBF | _IOLBF))
				if (_xflsbuf(stream) == EOF)
					ndone = EOF;
			TS_FUNLOCK(filelock);
			return(ndone);
		}
	}
}
