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
static char	*sccsid = "@(#)$RCSfile: environment.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:09:45 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1987 Regents of the University of California.
 * This file may be freely redistributed provided that this
 * notice remains attached.
 *
 * environment.c	1.1 (OSF) 9/12/89";
 * getenv.c	5.4 (Berkeley) 3/13/87";
 * setenv.c	1.2 (Berkeley) 3/13/87";
 */

#include <sys/types.h>
#include <stdio.h>

extern char **environ;
extern char *malloc(), *realloc();
static char *_findenv();

/*
 * setenv(name,value,rewrite)
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rewrite is set, replace any current value.
 */
setenv(name,value,rewrite)
	register char	*name,
			*value;
	int	rewrite;
{
	static int	alloced;		/* if allocated space before */
	register char	*C;
	int	l_value,
		offset;

	if (*value == '=')			/* no `=' in value */
		++value;
	l_value = strlen(value);
	if ((C = _findenv(name,&offset))) {	/* find if already exists */
		if (!rewrite)
			return(0);
		if (strlen(C) >= l_value) {	/* old larger; copy over */
			while (*C++ = *value++);
			return(0);
		}
	}
	else {					/* create new slot */
		register int	cnt;
		register char	**P;

		for (P = environ,cnt = 0;*P;++P,++cnt);
		if (alloced) {			/* just increase size */
			environ = (char **)realloc((char *)environ,
			    (u_int)(sizeof(char *) * (cnt + 2)));
			if (!environ)
				return(-1);
		}
		else {				/* get new space */
			alloced = 1;		/* copy old entries into it */
			P = (char **)malloc((u_int)(sizeof(char *) *
			    (cnt + 2)));
			if (!P)
				return(-1);
			bcopy(environ,P,cnt * sizeof(char *));
			environ = P;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	for (C = name;*C && *C != '=';++C);	/* no `=' in name */
	if (!(environ[offset] =			/* name + `=' + value */
	    malloc((u_int)((int)(C - name) + l_value + 2))))
		return(-1);
	for (C = environ[offset];(*C = *name++) && *C != '=';++C);
	for (*C++ = '=';*C++ = *value++;);
	return(0);
}

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
void
unsetenv(name)
	char	*name;
{
	register char	**P;
	int	offset;

	while (_findenv(name,&offset))		/* if set multiple times */
		for (P = &environ[offset];;++P)
			if (!(*P = *(P + 1)))
				break;
}

/*
 * getenv(name) --
 *	Returns ptr to value associated with name, if any, else NULL.
 */
char *
getenv(name)
	char *name;
{
	int	offset;

	return(_findenv(name,&offset));
}

/*
 * _findenv(name,offset) --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
static
char *
_findenv(name,offset)
	register char *name;
	int	*offset;
{
	register int	len;
	register char	**P,
			*C;

	for (C = name,len = 0;*C && *C != '=';++C,++len);
	for (P = environ;*P;++P)
		if (!strncmp(*P,name,len))
			if (*(C = *P + len) == '=') {
				*offset = P - environ;
				return(++C);
			}
	return(NULL);
}
