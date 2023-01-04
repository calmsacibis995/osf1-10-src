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
static char	*sccsid = "@(#)$RCSfile: start.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:57:41 $";
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
 * start.c	4.5	(Berkeley)	5/15/86
 */

#include "stdio.h"
#include "lrnref.h"
#include <sys/types.h>
#ifndef DIR
#include <sys/dir.h>
#endif

start(lesson)
char *lesson;
{
	struct direct dbuf;
	register struct direct *ep = &dbuf;	/* directory entry pointer */
	int c, n;
	char where [100];

#ifdef BSD4_2
	DIR *dp;
#define OPENDIR(s)	((dp = opendir(s)) != NULL)
#define DIRLOOP(s)	for (s = readdir(dp); s != NULL; s = readdir(dp))
#define EPSTRLEN	ep->d_namlen
#define CLOSEDIR	closedir(dp)
#else
	int f;
#define OPENDIR(s)	((f = open(s, 0)) >= 0)
#define DIRLOOP(s)	while (read(f, s, sizeof *s) == sizeof *s)
#define EPSTRLEN	strlen(ep->d_name)
#define CLOSEDIR	close(f)
#endif

	if (!OPENDIR(".")) {		/* clean up play directory */
		perror("Start:  play directory");
		wrapup(1);
	}
	DIRLOOP(ep) {
		if (ep->d_ino == 0)
			continue;
		n = EPSTRLEN;
		if (ep->d_name[n-2] == '.' && ep->d_name[n-1] == 'c')
			continue;
		c = ep->d_name[0];
		if (c>='a' && c<= 'z')
			unlink(ep->d_name);
	}
	CLOSEDIR;
	if (ask)
		return;
	sprintf(where, "%s/%s/L%s", dname, sname, lesson);
	if (access(where, 04)==0)	/* there is a file */
		return;
	perror(where);
	fprintf(stderr, "Start:  no lesson %s\n",lesson);
	wrapup(1);
}

fcopy(new,old)
char *new, *old;
{
	char b[BUFSIZ];
	int n, fn, fo;
	fn = creat(new, 0666);
	fo = open(old,0);
	if (fo<0) return;
	if (fn<0) return;
	while ( (n=read(fo, b, BUFSIZ)) > 0)
		write(fn, b, n);
	close(fn);
	close(fo);
}
