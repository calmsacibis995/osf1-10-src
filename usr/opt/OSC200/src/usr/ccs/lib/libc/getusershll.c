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
static char	*sccsid = "@(#)$RCSfile: getusershll.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/04 20:24:10 $";
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
 * Copyright (c) 1985 Regents of the University of California.
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
 *
 * getusershell.c	5.5 (Berkeley) 7/21/88
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endusershell_r = __endusershell_r
#pragma weak getusershell_r = __getusershell_r
#pragma weak setusershell_r = __setusershell_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endusershell = __endusershell
#pragma weak getusershell = __getusershell
#pragma weak setusershell = __setusershell
#endif
#endif
#if defined(_NAME_SPACE_WEAK_STRONG) && !defined(_THREAD_SAFE)
#ifdef endusershell_r
#undef endusershell_r
#endif
#ifdef setusershell_r
#undef setusershell_r
#endif
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _getusershell_rmutex;
#endif	/* _THREAD_SAFE */

/*
 * This is the default list of shells, to be used in the event that
 * SHELLS cannot be opened.  Do not add local shells here; they
 * should be added in SHELLS (/etc/shells).
 */
static char *okshells[] = {
	"/usr/bin/sh",
	"/usr/bin/csh",
	"/usr/bin/ksh",
	"/sbin/sh",
	0
};
#define SHELLS "/etc/shells"	/* File holding list of approved shells	*/

/*
 *	The state of a list of shells is kept in a "shellrec".
 *	There is one default shellrec, plus one for each 
 *	invocation of setusershell_r().  These are assigned
 *	a "cookie" that is used to implement reentrancy.
 *	The root_shell_rec is statically declared & initialized so that
 *	the non-reentrant functions are immune to malloc failure.
 */
typedef	struct shellrec {
	struct shellrec *link;		/* Linked list of shellrecs	*/
	unsigned int cookie;		/* token for reentrancy		*/
	char *strings;			/* contents of SHELLS		*/
	char **shells;			/* array of pointers into above	*/
	char **curshell;		/* pointer into above array	*/
} shell_t;

static unsigned int next_cookie = 1;	/* Next cookie to use		*/
#define NO_COOKIE	(0)		/* Need a new cookie		*/
#define ROOT_COOKIE (~NO_COOKIE)	/* Special cookie for default	*/
static shell_t root_shell_rec = {	/* The default shellrec		*/
	(shell_t *)0,
	ROOT_COOKIE,
	(char *)0,
	(char **)0,
	(char **)0
};


/*
 * Given a cookie, look up the corresponding shell_t.
 * If the cookie is ROOT_COOKIE, use the root_shell_rec;
 * otherwise, find the indicated shell_t, getting a new one if needed.
 *
 * NB: may return NULL; must be called with lock held if reentrant.
 */
static shell_t *
find_cookie(unsigned int *cookie, int allocate)
{
	shell_t *shp = &root_shell_rec;
	if (*cookie == ROOT_COOKIE)
		return( shp );
	while (shp->link){
		shp = shp->link;
		if (shp->cookie == *cookie)
			return(shp);
	}
	if (!allocate)
		return((shell_t *)0);
	/*
	 * shp now points to the last shell_t, with null link,
	 * so allocate a new one.
	 */
	shp->link = (shell_t *)calloc(1, sizeof(shell_t));
	if (shp = shp->link){
		shp->cookie = *cookie = next_cookie;
		if (++next_cookie == ROOT_COOKIE)
			next_cookie = (NO_COOKIE+1);
		/*
		 * NB: No check for cookie being unique.
		 * Other fields are zero thanks to calloc.
		 */
	}
	return(shp);
}


/*
 * Given a pointer to a shell_t, free its contents.
 */
static void
free_contents(shell_t *shp)
{
	if (shp){
		if (shp->strings)
			free((void *)shp->strings);
		if (shp->shells && shp->shells != okshells)
			free((void *)shp->shells);
		shp->strings = NULL;
		shp->shells = NULL;
		shp->curshell = NULL;
	}
}


/*
 * Given a cookie, look up the shell_t (possibly a new one)
 * and set it up to begin interating getusershell over it.
 */
static shell_t *
initshells(unsigned int *cookie)
{
	register char **sp, *cp;
	register FILE *fp;
	struct stat statb;
	shell_t *shp;

	shp = find_cookie(cookie, 1);
	if (!shp)
		return(shp);
	free_contents(shp);

	if ((fp = fopen(SHELLS, "r")) == (FILE *)0) {
		shp->curshell = shp->shells = okshells;
		return(shp);
	}
	if (fstat(fileno(fp), &statb) == -1 || NULL ==
	    (shp->strings = (char *)malloc((size_t)statb.st_size + 1))) {
		(void)fclose(fp);
		shp->curshell = shp->shells = okshells;
		return(shp);
	}
	/*
	 * allocate size/3 pointers since the minimum shell path is "/a\n"
	 */
	shp->shells = (char **)calloc((unsigned)statb.st_size / 3,
							sizeof (char *));
	if (shp->shells == NULL) {
		(void)fclose(fp);
		free((void *)shp->strings);
		shp->strings = NULL;
		shp->curshell = shp->shells = okshells;
		return(shp);
	}
	sp = shp->shells;
	cp = shp->strings;
	while (fgets(cp, MAXPATHLEN + 1, fp) != NULL) {
		while (*cp != '#' && *cp != '/' && *cp != '\0')
			cp++;
		if (*cp == '#' || *cp == '\0')
			continue;
		*sp++ = cp;
		while (!isspace(*cp) && *cp != '#' && *cp != '\0')
			cp++;
		*cp++ = '\0';
	}
	*sp = (char *)0;
	(void)fclose(fp);
	shp->curshell = shp->shells;
	return(shp);
}


/*
 * Workhorse routine for getusershell*().
 */
static char *
getashell(unsigned int *cookie)
{
	shell_t *shp;
	char *ret;

	TS_LOCK(&_getusershell_rmutex);
	shp = find_cookie(cookie, 0);
	if (!shp || !shp->curshell)
		shp = initshells(cookie);
	if (!shp)
		ret = NULL;
	else {
		if ((ret = *shp->curshell) != NULL)
			shp->curshell++;
	}
	TS_UNLOCK(&_getusershell_rmutex);
	return (ret);
}


/*
 * This code is structured so that the non-reentrant functions are built
 * on top of the more general reentrant functions, and neither is visible
 * in the "wrong" library.
 */
#ifdef _THREAD_SAFE
#	define STATIC
#else
#	define STATIC static
#endif

STATIC int
endusershell_r(unsigned int *cookie)
{
	shell_t *shp;

	TS_EINVAL(cookie == NULL);
	TS_LOCK(&_getusershell_rmutex);
	free_contents( shp = find_cookie(cookie, 0) );
	*cookie = 0;
	if (shp && shp != &root_shell_rec){
		shell_t *shp2 = &root_shell_rec;
		while (shp2->link && shp2->link != shp)
			shp2 = shp2->link;
		shp2->link = shp2->link->link;
		free((void *)shp);
	}
	TS_UNLOCK(&_getusershell_rmutex);
	return(0);
}


STATIC int
setusershell_r(unsigned int *cookie)
{
	shell_t *shp;

	TS_EINVAL(cookie == NULL);
	TS_LOCK(&_getusershell_rmutex);
	shp = initshells(cookie);
	TS_UNLOCK(&_getusershell_rmutex);
	if (shp)
		return(0);
	/* Cannot happen unless thread_safe */
	_Seterrno(ENOMEM);
	return(-1);
}


#ifndef _THREAD_SAFE

int
endusershell(void)
{
	unsigned int cookie = ROOT_COOKIE;
	return( endusershell_r(&cookie) );
}

int
setusershell(void)
{
	unsigned int cookie = ROOT_COOKIE;
	return( setusershell_r(&cookie) );
}

char *
getusershell(void)
{
	unsigned int cookie = ROOT_COOKIE;
	return( getashell(&cookie) );
}

#else	/* _THREAD_SAFE */

int
getusershell_r(char *shell, int len, unsigned int *cookie)
{
	char *ret;

	TS_EINVAL((shell == NULL) || (len <= 0) || (cookie == NULL));
	ret = getashell(cookie);

	if (ret == NULL) {
		_Seterrno(ESRCH);
		return (-1);
	}
	if (strlen(ret) >= len) {
		_Seterrno(EINVAL);
		return (-1);
	}
	strcpy(shell, ret);
	return (0);
}

#endif	/* _THREAD_SAFE */
