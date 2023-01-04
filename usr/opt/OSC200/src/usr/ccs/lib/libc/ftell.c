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
static char	*sccsid = "@(#)$RCSfile: ftell.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:55:51 $";
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
 * FUNCTIONS: ftell 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * static char sccsid[] = "@(#)ftell.c	3.1  @(#)ftell.c	3.1 2/26/91 10:13:22";
 */

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
#include <errno.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern long lseek();

/*                                                                    
 * FUNCTION: Returns the number of bytes from the beginning of the file
 *           for the current position in the stream.
 *
 * PARAMETERS: FILE *stream      - stream to be searched
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      zero if successful
 *	      -1 if not successful
 *	      errno is set on error to indicate the error
 */

long int
ftell(FILE *stream)
{
	long	tres;
	int	adjust;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if (stream->_cnt < 0)
		stream->_cnt = 0;
	if (stream->_flag & _IOREAD)
		adjust = - stream->_cnt;
	else if (stream->_flag & (_IOWRT | _IORW)) {
		adjust = 0;
#ifdef	_THREAD_SAFE
		(void) fflush_unlocked(stream);
#else
		(void) fflush(stream);
#endif	/* _THREAD_SAFE */
		if (stream->_flag & _IORW) {
			stream->_cnt = 0;
			stream->_flag &= ~_IOWRT;
			stream->_ptr = stream->_base;
		}
	} else {
		TS_FUNLOCK(filelock);
		seterrno(EBADF);
		return (-1);
	}
	tres = lseek(fileno(stream), 0L, SEEK_CUR);
	if (tres >= 0)
		tres += (long)adjust;

	TS_FUNLOCK(filelock);
	return (tres);
}
