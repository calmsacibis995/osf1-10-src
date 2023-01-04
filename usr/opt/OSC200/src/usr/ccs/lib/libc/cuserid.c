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
static char	*sccsid = "@(#)$RCSfile: cuserid.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 22:44:20 $";
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
 * FUNCTIONS: cuserid 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * cuserid.c	1.11  com/lib/c/io,3.1,8943 10/26/89 12:48:23
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak cuserid = __cuserid
#endif
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <string.h>

#ifndef _THREAD_SAFE
static char res[L_cuserid];
#endif

char *
cuserid(char *s)
{
	register struct passwd *pw;
	register char *p;
#ifdef _THREAD_SAFE
	char line[BUFSIZ];
	struct passwd password;
#endif	/* _THREAD_SAFE */

#ifdef _THREAD_SAFE
	if (s == NULL)
		return(NULL);
	password.pw_name = line;
	if (getpwuid_r(geteuid(), &password, line, BUFSIZ) < 0)
		pw = NULL;
	else
		pw = &password;
#else
	pw = getpwuid(geteuid());
#endif /* _THREAD_SAFE */

	/* If successful, then copy the name into the string and return it. */
	if (pw != NULL) {
#ifndef _THREAD_SAFE
		if (s == NULL)
			s = res;
#endif
		strcpy(s, pw->pw_name);
		endpwent();
		return s;
	}
	endpwent();

#ifndef _THREAD_SAFE
	/* If failure and a NULL was passed in, return the NULL. */
	if (s == NULL)
		return(NULL);
#endif

	/*
	 * If failure and a string was passed in, zero the first element of
	 * the string and return the orginal string.
	 */
	*s = '\0';
	return (s);
}
