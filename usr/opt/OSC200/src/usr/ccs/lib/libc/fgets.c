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
static char	*sccsid = "@(#)$RCSfile: fgets.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:50:47 $";
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
 * FUNCTIONS: fgets 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fgets.c	1.11  com/lib/c/io,3.1,8943 10/18/89 10:18:40
 */

/*LINTLIBRARY*/
/*
 * This version reads directly from the buffer rather than looping on getc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "stdiom.h"

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "stdio_lock.h"

#define	RETURN(val)	return(TS_FUNLOCK(filelock), (char *)(val))

#else
#define	RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */


#define MIN(x, y)	(x < y ? x : y)

extern int __filbuf();

char 	*
fgets(char *s, int n, FILE *stream)
{
	char *p, *save = s;
	int i;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stream);
	if ((stream->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
		_Seterrno(EBADF);
		RETURN(NULL);
	}
	for (n--; n > 0; n -= i) {
		if (stream->_cnt <= 0) { /* empty buffer */
			if (__filbuf(stream) == EOF) {
				if (save == s) {
					RETURN(NULL);
				}
				break; /* no more data */
			}
			stream->_ptr--;
			stream->_cnt++;
		}
		i = MIN(n, stream->_cnt);
		if ((p = memccpy((void *)s,(void *)stream->_ptr,(int)'\n',(size_t)i)) !=NULL)
			i = p - s;
		s += i;
		stream->_cnt -= i;
		stream->_ptr += i;
		_BUFSYNC(stream);
		if (p != NULL)
			break; /* found '\n' in buffer */
	}
	*s = '\0';
	RETURN(save);
}
