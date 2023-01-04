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
static char	*sccsid = "@(#)$RCSfile: setenv.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 22:37:40 $";
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
 *
 * CONTENTS: setenv, unsetenv
 *
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak setenv = __setenv
#pragma weak unsetenv = __unsetenv
#endif
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex	_environ_rmutex;
#endif	/* _THREAD_SAFE */

extern char *strdup();
extern char **environ;
static int alloced = 0;			/* if allocated space before */

#ifdef _THREAD_SAFE
#define RETURN(val)	return(TS_UNLOCK(&_environ_rmutex), val)
#else
#define RETURN(val)	return(val)
#endif	/* _THREAD_SAFE */

/*
 * _findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
char *
_findenv(register const char *name, int *offset)
{
	extern char **environ;
	register int len;
	register char **P, *C;
	{
		register const char *CC;

		for (CC = name, len = 0; *CC && *CC != '='; ++CC, ++len)
			;
	}
	TS_LOCK(&_environ_rmutex);

	for (P = environ; *P; ++P)
		if (!strncmp(*P, name, len))
			if (*(C = *P + len) == '=') {
				*offset = P - environ;
				RETURN(++C);
			}
	RETURN(NULL);
}

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
int
setenv(register const char *name, register const char *value, int rewrite)
{
	register char *C;
	int l_value, offset;

	if (*value == '=')			/* no `=' in value */
		++value;
	l_value = strlen(value);

	TS_LOCK(&_environ_rmutex);
	if ((C = _findenv(name, &offset))) {	/* find if already exists */
		if (!rewrite)
			RETURN(0);
		if (strlen(C) >= l_value) {	/* old larger; copy over */
			while (*C++ = *value++)
				;
			RETURN(0);
		}
	}
	else {					/* create new slot */
		register int	cnt;
		register char	**P;

		for (P = environ, cnt = 0; *P; ++P, ++cnt);
		if (alloced) {			/* just increase size */
			environ = (char **)realloc((char *)environ,
			    (u_int)(sizeof(char *) * (cnt + 2)));
			if (!environ)
				RETURN(-1);
		}
		else {				/* get new space */
			alloced = 1;		/* copy old entries into it */
			P = (char **)malloc((u_int)(sizeof(char *) *
			    (cnt + 2)));
			if (!P)
				RETURN(-1);
			memcpy(P, environ, cnt * sizeof(char *));
			environ = P;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	{
		register const char *CC;
		for (CC = name; *CC && *CC != '='; ++CC) /* no `=' in name */
			;
		if (!(environ[offset] =			/* name + `=' + value */
		    malloc((u_int)((int)(CC - name) + l_value + 2))))
			RETURN(-1);
	}
	for (C = environ[offset]; (*C = *name++) && *C != '='; ++C)
		;
	for (*C++ = '='; *C++ = *value++;)
		;
	RETURN(0);
}


/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
void
unsetenv(const char *name)
{
	extern	char	**environ;
	register char	**P;
	int	offset;

	TS_LOCK(&_environ_rmutex);

	while (_findenv(name, &offset))	/* if set multiple times */
		for (P = &environ[offset];; ++P)
			if (!(*P = *(P + 1)))
				break;

	TS_UNLOCK(&_environ_rmutex);
}
