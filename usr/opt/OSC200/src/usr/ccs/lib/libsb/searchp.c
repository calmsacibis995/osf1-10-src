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
static char	*sccsid = "@(#)$RCSfile: searchp.c,v $ $Revision: 4.3 $ (DEC) $Date: 1991/11/26 11:45:41 $";
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
/*  searchp  --  search through pathlist for file
 *
 *  Usage:  p = searchp (path,file,fullname,func);
 *	char *p, *path, *file, *fullname;
 *	int (*func)();
 *
 *  Searchp will parse "path", a list of pathnames separated
 *  by colons, prepending each pathname to "file".  The resulting
 *  filename will be passed to "func", a function provided by the
 *  user.  This function must return zero if the search is
 *  successful (i.e. ended), and non-zero if the search must
 *  continue.  If the function returns zero (success), then
 *  searching stops, the full filename is placed into "fullname",
 *  and searchp returns 0.  If the pathnames are all unsuccessfully
 *  examined, then searchp returns -1.
 *  If "file" begins with a slash, it is assumed to be an
 *  absolute pathname and the "path" list is not used.  Note
 *  that this rule is used by Bell's cc also; whereas Bell's
 *  sh uses the rule that any filename which CONTAINS a slash
 *  is assumed to be absolute.  The execlp and execvp procedures
 *  also use this latter rule.  In my opinion, this is bogosity.
 */

int searchp (path,file,fullname,func)
char *path,*file,*fullname;
int (*func)();
{
	register char *nextpath,*nextchar,*fname,*lastchar;
	int failure;

	nextpath = ((*file == '/') ? "" : path);
	do {
		fname = fullname;
		nextchar = nextpath;
		while (*nextchar && (*nextchar != ':'))
			*fname++ = *nextchar++;
		if (nextchar != nextpath && *file) *fname++ = '/';
		lastchar = nextchar;
		nextpath = ((*nextchar) ? nextchar + 1 : nextchar);
		nextchar = file;	/* append file */
		while (*nextchar)  *fname++ = *nextchar++;
		*fname = '\0';
		failure = (*func) (fullname);
	} 
	while (failure && (*lastchar));
	return (failure ? -1 : 0);
}
