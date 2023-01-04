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
static char	*sccsid = "@(#)$RCSfile: tmpnam.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:25:26 $";
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

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include <errno.h>
#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_tmpnam_rmutex;
#endif	/* _THREAD_SAFE */

extern char *mktemp(), *strcpy(), *strcat();
static char str[L_tmpnam], seed[] = { 'a', 'a', 'a', '\0' };

char *
tmpnam(char *s)
{
	register char *p, *q;

#ifdef _THREAD_SAFE
	if (s == NULL) {
		_Seterrno(EINVAL);
		return(NULL);
	}

	p = s;
#else
	p = (s == NULL)? str: s;
#endif	/* _THREAD_SAFE */
	(void) strcpy(p, P_tmpdir);
	TS_LOCK(&_tmpnam_rmutex);
	(void) strcat(p, seed);
	(void) strcat(p, "XXXXXX");

	q = seed;
	while(*q == 'z')
		*q++ = 'a';
	if(*q != '\0')  {
		++*q;
	}
	TS_UNLOCK(&_tmpnam_rmutex);

	(void) mktemp(p);
	return(p);
}
