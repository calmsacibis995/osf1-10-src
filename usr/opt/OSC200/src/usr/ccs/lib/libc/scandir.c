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
static char	*sccsid = "@(#)$RCSfile: scandir.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:36:28 $";
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
 * Copyright (c) 1983 Regents of the University of California.
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
 * scandir.c	5.3 (Berkeley) 6/18/88
 */


/*
 * Scan the directory dirname calling select to make a list of selected
 * directory entries then sort using qsort and compare routine dcomp.
 * Returns the number of entries and a pointer to a list of pointers to
 * struct direct (through namelist). Returns -1 if there were any errors.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak alphasort = __alphasort
#pragma weak scandir = __scandir
#endif
#endif
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * define DIRSIZ macro to get a smaller than NAME_MAX sized entry.
 * Note: this is NOT the same as the DIRSIZ in <ufs/dir.h>.
 */
#define DIRSIZ(dp) \
    ((sizeof (struct dirent) - sizeof((dp)->d_name)) + (dp)->d_namlen + 1)

scandir(const char *dirname, struct dirent *(*namelist[]),
	int (*select)(struct dirent *), 
	int (*dcomp)(struct dirent **, struct dirent **))
{
	register struct dirent *d, *p, **names;
	register int nitems;
	register char *cp1, *cp2;
	long arraysz;
	DIR *dirp;

	if ((dirp = opendir((char *) dirname)) == NULL)
		return(-1);
	arraysz = BUFSIZ / sizeof(struct dirent *);
	names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
	if (names == NULL)
		return(-1);

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (select && !(*select)(d))
			continue;	/* just selected names */
		/*
		 * Make a minimum size copy of the data
		 */
		p = (struct dirent *)malloc(DIRSIZ(d));
		if (p == NULL)
			return(-1);
		p->d_fileno = d->d_fileno;
		p->d_reclen = d->d_reclen;
		p->d_namlen = d->d_namlen;
		for (cp1 = p->d_name, cp2 = d->d_name; *cp1++ = *cp2++; );
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems >= arraysz) {
			arraysz += BUFSIZ / sizeof(struct dirent *);
			names = (struct dirent **)realloc((char *)names,
				arraysz * sizeof(struct dirent *));
			if (names == NULL)
				return(-1);
		}
		names[nitems-1] = p;
	}
	closedir(dirp);
	if (nitems && dcomp)
		qsort(names, nitems, sizeof(struct dirent *), 
			(int (*)(const void *, const void *))dcomp);
	*namelist = names;
	return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
alphasort(struct dirent **d1, struct dirent **d2)
{
	return(strcmp((*d1)->d_name, (*d2)->d_name));
}
