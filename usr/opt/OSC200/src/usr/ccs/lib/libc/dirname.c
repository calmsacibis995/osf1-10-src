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
static char	*sccsid = "@(#)$RCSfile: dirname.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/18 12:27:31 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: dirname 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * dirname.c	1.7  com/lib/c/adm,3.1,8943 9/12/89 13:18:12
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak dirname_r = __dirname_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak dirname = __dirname
#endif
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_supp.h"
/*LINTLIBRARY*/

#ifdef _THREAD_SAFE
int
dirname_r(char *path, char *buf, int size)
#else
char *
dirname(char *path)
#endif	/* _THREAD_SAFE */
{
	register char	*cp;
	size_t		length;
#ifndef _THREAD_SAFE
	static char	*buf = NULL;
	static int	size = 0;
#endif	/* _THREAD_SAFE */

	TS_EINVAL((path == NULL || buf == NULL || size <= 0));

	TS_EINVAL(!*path);
	/*
	** find end of string
	*/
	for (cp = path; *cp; cp++)
		;
	cp--;
	/*
	** lop off trailing slashes
	*/
	while (cp > path && *cp == '/')
		cp--;
	/*
	** find the next last slash
	*/
	while (cp > path && *cp != '/')
		cp--;
	/*
	** check for no directory
	*/
	if (cp == path && *cp != '/') {
#ifdef	_THREAD_SAFE
		_Seterrno(EINVAL);
#endif	/* _THREAD_SAFE */
		return (TS_FAILURE);
	}
	/*
	** skip slash sequence
	*/
	while (cp > path && *cp == '/')
		cp--;
	cp++;

	if ((length = cp - path) >= size) {
#ifdef _THREAD_SAFE
		_Seterrno(EINVAL);
		return (TS_FAILURE);
#else
		free(buf);
		size = length + 1;
		if (!(buf = malloc(size)))
			return (TS_FAILURE);
#endif	/* _THREAD_SAFE */
	}

	buf[length] = '\0';
	strncpy(buf, path, length);
	return (TS_FOUND(buf));
}
