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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: pathname.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 20:13:41 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * pathname.c - directory/pathname support functions 
 *
 * DESCRIPTION
 *
 *	These functions provide directory/pathname support for PAX
 *
 * AUTHOR
 *
 *	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Revision 1.2  89/02/12  10:05:13  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:21  mark
 * Initial revision
 * 
 */

/* Headers */

#include "pax.h"


/* dirneed  - checks for the existance of directories and possibly create
 *
 * DESCRIPTION
 *
 *	Dirneed checks to see if a directory of the name pointed to by name
 *	exists.  If the directory does exist, then dirneed returns 0.  If
 *	the directory does not exist and the f_dir_create flag is set,
 *	then dirneed will create the needed directory, recursively creating
 *	any needed intermediate directory.
 *
 *	If f_dir_create is not set, then no directories will be created
 *	and a value of -1 will be returned if the directory does not
 *	exist.
 *
 * PARAMETERS
 *
 *	name		- name of the directory to create
 *
 * RETURNS
 *
 *	Returns a 0 if the creation of the directory succeeded or if the
 *	directory already existed.  If the f_dir_create flag was not set
 *	and the named directory does not exist, or the directory creation 
 *	failed, a -1 will be returned to the calling routine.
 */


int dirneed(char *name)

{
    char           *cp;
    char           *last;
    int             ok;
    static Stat     sb;

    last = (char *)NULL;
    for (cp = name; *cp;) {
	if (*cp++ == '/') {
	    last = cp;
	}
    }
    if (last == (char *)NULL) {
	return (STAT(".", &sb));
    }
    *--last = '\0';
    ok = STAT(*name ? name : ".", &sb) == 0
	? ((sb.sb_mode & S_IFMT) == S_IFDIR)
	: (f_dir_create && dirneed(name) == 0 && dirmake(name, &sb) == 0);
    *last = '/';
    return (ok ? 0 : -1);
}


/* nameopt - optimize a pathname
 *
 * DESCRIPTION
 *
 * 	Confused by "<symlink>/.." twistiness. Returns the number of final 
 * 	pathname elements (zero for "/" or ".") or -1 if unsuccessful. 
 *
 * PARAMETERS
 *
 *	char	*begin	- name of the path to optimize
 *
 * RETURNS
 *
 *	Returns 0 if successful, non-zero otherwise.
 *
 */


int nameopt(char *begin)

{
    char           *name;
    char           *item;
    int             idx;
    int             absolute;
    char           *element[PATHELEM];

    absolute = (*(name = begin) == '/');
    idx = 0;
    for (;;) {
	if (idx == PATHELEM) {
	    warn(begin, MSGSTR(PN_TOOMANY, "Too many elements"));
	    return (-1);
	}
	while (*name == '/') {
	    ++name;
	}
	if (*name == '\0') {
	    break;
	}
	element[idx] = item = name;
	while (*name && *name != '/') {
	    ++name;
	}
	if (*name) {
	    *name++ = '\0';
	}
	if (strcmp(item, "..") == 0) {
	    if (idx == 0) {
		if (!absolute) {
		    ++idx;
		}
	    } else if (strcmp(element[idx - 1], "..") == 0) {
		++idx;
	    } else {
		--idx;
	    }
	} else if (strcmp(item, ".") != 0) {
	    ++idx;
	}
    }
    if (idx == 0) {
	element[idx++] = absolute ? "" : "."; 
    }
    element[idx] = (char *)NULL;
    name = begin;
    if (absolute) {
	*name++ = '/';
    }
    for (idx = 0; item = element[idx]; ++idx, *name++ = '/') {
	while (*item) {
	    *name++ = *item++;
	}
    }
    *--name = '\0';
    return (idx);
}


/* dirmake - make a directory  
 *
 * DESCRIPTION
 *
 *	Dirmake makes a directory with the appropritate permissions.
 *
 * PARAMETERS
 *
 *	char 	*name	- Name of directory make
 *	Stat	*asb	- Stat structure of directory to make
 *
 * RETURNS
 *
 * 	Returns zero if successful, -1 otherwise. 
 *
 */


int dirmake(char *name, Stat *asb)

{
    if (mkdir(name, (int) (asb->sb_mode & S_IPOPN)) < 0) {
	return (-1);
    }
    chmod(name, ((asb->sb_mode & S_IPERM) | S_IWUSR));
    return (0);
}
