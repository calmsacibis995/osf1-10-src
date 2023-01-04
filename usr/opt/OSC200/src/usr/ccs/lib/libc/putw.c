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
static char	*sccsid = "@(#)$RCSfile: putw.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 21:20:09 $";
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
 * FUNCTIONS: putw 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * putw.c	1.7  com/lib/c/io,3.1,8943 10/6/89 14:06:44
 */

/*LINTLIBRARY*/
/*
 * The intent here is to provide a means to make the order of
 * bytes in an io-stream correspond to the order of the bytes
 * in the memory while doing the io a `word' at a time.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak putw = __putw
#endif
#ifdef _THREAD_SAFE

/* Assume streams in this module are already locked.
 */
#define	_STDIO_UNLOCK_CHAR_IO
#endif	/* _THREAD_SAFE */

#include <stdio.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return(TS_FUNLOCK(filelock), (val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */

int
putw(int w, register FILE *stream)
{
	register char	*s = (char *)&w;
	register int	i = sizeof(int);
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);

	while (--i >= 0) {
		(void) putc(*s++, stream);
		if (ferror(stream)) {
			RETURN(EOF);
		}
	}
	RETURN(0);
}
