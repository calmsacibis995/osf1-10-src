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
static char	*sccsid = "@(#)$RCSfile: gets.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/07 23:02:34 $";
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
 * FUNCTIONS: gets 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * gets.c	1.8  com/lib/c/io,3.1,8943 9/12/89 18:32:40
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
#include "stdiom.h"
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */

extern int __filbuf();
extern char *memccpy();

char *
gets(char *s)
{
	char	*p, *s0 = s;
	int	n;
	TS_FDECLARELOCK(filelock)

	TS_FLOCK(filelock, stdin);
	if ((stdin->_flag&(_IOREAD|_IORW))==0) {  /* not open for reading */
                TS_SETERR(EBADF);
	        TS_FUNLOCK(filelock);
                return(NULL);
	}

	for ( ; ; ) {
		if (stdin->_cnt <= 0) { /* empty buffer */
			if (__filbuf(stdin) == EOF) {
				if (s0 == s) {
	                                TS_FUNLOCK(filelock);
					return (NULL);
                                }
				break; /* no more data */
			}
			stdin->_ptr--;
			stdin->_cnt++;
		}
		n = stdin->_cnt;
		if ((p = memccpy(s, (char *) stdin->_ptr, '\n', n)) != NULL)
			n = p - s;
		s += n;
		stdin->_cnt -= n;
		stdin->_ptr += n;
		_BUFSYNC(stdin);
		if (p != NULL) { /* found '\n' in buffer */
			s--; /* step back over '\n' */
			break;
		}
	}
	*s = '\0';
        TS_FUNLOCK(filelock);
	return (s0);
}
