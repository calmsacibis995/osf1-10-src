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
static char	*sccsid = "@(#)$RCSfile: putenv.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:36:59 $";
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
 * FUNCTIONS: putenv, clearenv
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*-
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak clearenv = __clearenv
#pragma weak putenv = __putenv
#endif
#include <stdlib.h>
#include <stdio.h>	/* for NULL */
#include <string.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _environ_rmutex;
#endif	/* _THREAD_SAFE */

extern char *strdup();
static int match(const char *s1, const char *s2);
static int find(const char *str);

extern char **environ;	/* pointer to environment */
static reall = 0;	/* flag to reallocate space, if putenv is
			   called more than once */

/*
 * FUNCTION:    putenv - change environment variables
 *
 * NOTES:       This routine changes or adds values to the environment.
 *              The argument - char *change = a pointer to a string of
 *              the form "name=value"
 *
 * DATA STRUCTURES:     'Environ' gets modified.
 *
 * RETURN VALUE DESCRIPTION:    output - 0, if successful, otherwise -1
 */
int
putenv(const char *change)	/* "name=value" to add to environment */
{
	char **newenv;		/* points to new environment */
	int which;		/* index of variable to replace */

	TS_LOCK(&_environ_rmutex);

	if ((which = find(change)) < 0)  {
		/* if a new variable */
		/* which is negative of table size, so invert and
		   count new element */
		which = (-which) + 1;
		if (reall)  {
			/* we have expanded environ before */
			newenv = (char **)realloc((char *) environ,
				  which*sizeof(char *));
			if (newenv == NULL) {
				TS_UNLOCK(&_environ_rmutex);
				return(-1);
			}
			/* now that we have space, change environ */
			environ = newenv;
		} else {
			/* environ points to the original space */
			reall++;
			newenv = (char **)malloc(which*sizeof(char *));
			if (newenv == NULL) {
				TS_UNLOCK(&_environ_rmutex);
				return(-1);
			}
			memcpy((void *)newenv, (void *)environ,
				(size_t)(which*sizeof(char *)));
			environ = newenv;
		}
		environ[which-2] = (char *)change;
		environ[which-1] = NULL;
	}  else  {
		/* we are replacing an old variable */
		environ[which] = (char *)change;
	}

	TS_UNLOCK(&_environ_rmutex);
	return (0);
}


/*
 * NAME:        find
 *
 * FUNCTION:    find - find where 'str' is in environ
 *
 * NOTES:       Find looks thru the environment for the string
 *              matching 'str'.  'Str' is of the form "name=value".
 *
 * RETURN VALUE DESCRIPTION:    returns index of the pointer that
 *              matches.  if no match was found, the size of the
 *              table * -1 is returned.
 */
static int
find(const char *str)
{
	int ct = 0;	/* index into environ */

	while (environ[ct] != NULL) {
		if (match(environ[ct], str)  != 0)
			return (ct);
		ct++;
	}
	return (-(++ct));
}


/*
 * NAME:        match
 *
 * FUNCTION:    Match compares a string 's1', which is of the
 *              form "name" or "name=value" and compares it
 *              to 's2', which is of the form "name=value".
 *
 * RETURN VALUE DESCRIPTION:    1 if the names match, else 0
 */
static int
match(const char *s1, const char *s2)
{
	while (*s1 == *s2++)  {
		if (*s1 == '=')
			return (1);
		s1++;
	}
	return (0);
}


/*
 * FUNCTION:	clearenv - clear environment
 *
 * DATA STRUCTURES:	'Environ' gets modified.
 *
 * RETURN VALUE DESCRIPTION:	0 - always succeeds
 */  
int
clearenv(void)
{
	TS_LOCK(&_environ_rmutex);

	if (reall) {	/* we have expanded environ before */
		char **newenv;

		newenv = (char **)realloc((char *)environ, 1*sizeof(char *));
		if (newenv)
			environ = newenv;
	}
	environ[0] = NULL;

	TS_UNLOCK(&_environ_rmutex);
	return (0);
}
