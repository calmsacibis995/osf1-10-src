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
static char	*sccsid = "@(#)$RCSfile: tempnam.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/19 19:05:55 $";
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
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: tempnam 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.13  com/lib/c/io/tempnam.c, libcio, bos320, 9125320 6/11/91 11:55:38
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak tempnam = __tempnam
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/access.h>
#include <sys/types.h>
#include <errno.h>
#include "ts_supp.h"

#ifdef	_THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex _tempnam_rmutex;
#endif	/* _THREAD_SAFE */

extern char *mktemp();

static char *pcopy();
#define RANDOMLEN	9	/* 1 for us and 6 for mktemp() */

#define	SEEDLEN		3
static char seed[] = "AAA";

/*                                                                    
 * FUNCTION: Returns temporary file name.
 *                                                                    
 * RETURN VALUE DESCRIPTION: char * filename, NULL on error.
 */  

char *
tempnam(
const char *dir,		/* use this directory please (if non-NULL) */
const char *pfx)		/* use this (if non-NULL) as filename prefix */
{
	register char *p, *q, *tdir;
	int pfxmax, x = 0;
	char	*last, *letter, first;

	if ((tdir = getenv("TMPDIR")) != NULL)
		if ((x = strlen(tdir)) > 0 && 
				access(tdir, W_ACC | X_ACC) == 0)
			goto OK;
	if ((tdir = (char*)dir) != NULL)
		if ((x = strlen(tdir)) > 0 && 
				access(tdir, W_ACC | X_ACC) == 0)
			goto OK;
	if ((tdir = P_tmpdir) != NULL)
		if ((x = strlen(tdir)) > 0 && 
				access(tdir, W_ACC | X_ACC) == 0)
			goto OK;
	if (access(tdir = "/tmp", W_ACC | X_ACC) != 0)
		return (NULL);
	x = 4;	/* strlen("/tmp") */

OK:
	if ((pfxmax = pathconf(tdir, _PC_NAME_MAX)) < 0)
		pfxmax = 14;	/* System V NAME_MAX */
	pfxmax -= RANDOMLEN; /* max chars to use of pfx argument */
        if ((p = (char *)malloc((size_t)(x + pfxmax + RANDOMLEN + 2))) == NULL)
		return (NULL);
	pcopy(p, tdir);

	(void)strcat(p, "/");
	if (pfx) {
	  	int pfxlen, charlen = mblen(pfx, MB_CUR_MAX);
	  	char *pfxptr = (char*)pfx; 

                if (charlen < 1)
                        charlen = 1;

                /* determine how many bytes of prefix, up to pfxmax, to keep */
                /* (do not truncate prefix mid-character) */
                for (pfxlen=charlen; pfxlen+charlen<=pfxmax; pfxlen+=charlen) {
                        pfxptr+=charlen;
                        charlen = mblen(pfxptr, MB_CUR_MAX);
                        if (charlen < 1)
                                charlen = 1;
                }
                *(p+strlen(p)+pfxlen) = '\0';
                (void)strncat(p, pfx, pfxlen);
        }
	last = p + strlen(p);	/* find end of string */
	first = p[0];		/* mktemp() trashes the p[0] if it fails */

	/* loop around trying mktemp() with a differing lead */
	TS_LOCK(&_tempnam_rmutex);
	(void)strcpy(last, seed);
	(void)strcpy(last + SEEDLEN, "XXXXXX");	/* mktemp() template */
	q = seed;
	while(*q == 'Z') {
		*q++ = 'A';
	}
	if(*q != '\0' )
		++*q;
	TS_UNLOCK(&_tempnam_rmutex);
	if (mktemp(p) && p[0] != '\0')
		return (p);
	p[0] = first;
	free(p);
	return (NULL);
}

static char*
pcopy(space, arg)
char *space, *arg;
{
        char *p;

        if (arg) {
                (void)strcpy(space, arg);
                p = space-1+strlen(space);
                if (*p == '/')
                        *p = '\0';
        }
        return (space);
}
