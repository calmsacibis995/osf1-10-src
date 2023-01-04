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
static char	*sccsid = "@(#)$RCSfile: getenv.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/10/05 21:01:15 $";
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
 * FUNCTIONS: getenv
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * getenv.c	1.10  com/lib/c/gen,3.1,8943 9/8/89 08:48:33
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>			/* for NULL */

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_environ_rmutex;
#endif	/* _THREAD_SAFE */

/* this is extern here because the space is actually allocated in
 * envirin.o, this was done to support both environ and __environ 
 * as evironment pointers */
extern char **environ;		/* environment list */

static char *nvmatch();	/* find an environment match */

/*
 * NAME:	getenv
 *
 * FUNCTION:	return the value of an environment variable
 *
 * NOTES:	Getenv searches the environment list for 'name', which
 *		is a string of the form "name=value".
 *
 * RETURN VALUE DESCRIPTION:	NULL if the enviroment variable is not
 *		found, else a pointer to the value.
 */  
/* The getenv() function includes all the POSIX requirements */
char *
getenv(const char *name)
{
	char **p, *v;

	TS_LOCK(&_environ_rmutex);

	if ((p = environ) != NULL)
		while (*p != NULL)
			if ((v = nvmatch(name, *p++)) != NULL) {
				TS_UNLOCK(&_environ_rmutex);
				return (v);
			}

	TS_UNLOCK(&_environ_rmutex);
	return (NULL);
}


/*
 * NAME:	nvmatch
 *
 * FUNCTION:	see if 's1' matches 's2'.  's1' is either "name" or
 *		"name=value" and 's2' is "name=value".
 *
 * RETURN VALUE DESCRIPTION:	s2 if they match, else NULL
 */  
static char *
nvmatch(char *s1, char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '=')
			return (s2);

	if (*s1 == '\0' && *(s2-1) == '=')
		return (s2);

	return (NULL);
}


#pragma weak _getenv = getenv
/*
char *
_getenv(const char *name)
{
	return(getenv(name));
}
*/
