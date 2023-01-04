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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: expfile.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:05:10 $";
#endif
/* 
 * COMPONENT_NAME: UUCP expfile.c
 * 
 * FUNCTIONS: canPath, ckexpf, expfile, mkdirs, mkdirs2 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/
/*								*/
/*	 1.7  com/cmd/uucp/expfile.c, 9115320a, bos320 3/27/91 10:59:03"; */

#include "uucp.h"
#include "pathnames.h"
/* VERSION( expfile.c	5.2 -  -  ); */

/*
 * expand file name expansion is based on first characters
 *	/	-> fully qualified pathname. no
 *		   processing necessary
 *	~	-> prepended with login directory
 *	~/	-> prepended with Pubdir
 *	default	-> prepended with current directory
 *	file	-> filename to expand
 * returns: 
 *	0	-> ok
 *      FAIL	-> no Wrkdir name available
 */
expfile(file)
register char *file;
{
	register char *fpart, *up;
	int uid;
	char user[NAMESIZE], save[MAXFULLNAME];

	strcpy(save, file);
	if (*file == '/')
	    ;
	else if (*file ==  '~') {
		/* find / and copy user part */
		for (fpart = save + 1, up = user; *fpart != '\0'
			&& *fpart != '/'; fpart++)
				*up++ = *fpart;
		*up = '\0';
		if ((user[0]=='\0') || (gninfo(user, &uid, file) != 0)){
			(void) strcpy(file, Pubdir);
		}
		(void) strcat(file, fpart);
	}
	else {
		(void) sprintf(file, "%s/%s", Wrkdir, save);
		if (Wrkdir[0] == '\0')
			return(FAIL);
	}

	if (canPath(file) != 0) { /* I don't think this will ever fail */
		(void) strcpy(file, CORRUPTDIR);
		return(FAIL);
	}
	else
		return(0);
}


/*
 * make all necessary directories
 *	name	-> directory to make
 * return: 
 *	0	-> success
 * 	FAIL	-> failure
 */
mkdirs(name)
register char *name;
{
	register char *p;
	char dir[MAXFULLNAME];

	strcpy(dir, name);
	if (*LASTCHAR(dir) != '/')
	    	(void) strcat(dir, "/");
	p = dir + 1;
	while (1) {
	    if ((p = (char *)strchr(p, '/')) == NULL)
		return(0);
	    *p = '\0';
	    DEBUG(4, "mkdir - %s\n", dir);
	    if (mkdirs2(dir, 0) == FAIL)
		return (FAIL);
	    *p++ = '/';
	}
}


#ifdef ATTSV
/*
 * Make name a directory if it is not already a directory
 * ATTSV handling of setuid is less than convenient ...
 * return:
 *	0 -> ok
 *	FAIL -> failed
 */

mkdirs2(name, mask)
register char	*name;
register int	mask;
{
	int	ret, pid, status, sptr;
	char	*tail, nmbuf[MAXFULLNAME], *parent;
	struct stat	statbuf;

	if (DIRECTORY(name))
		return(0);		/* directory exists */

	/* in ATTSV the parent directory has to be writeable by real uid */
	(void) strcpy(nmbuf, name);

	/* get pathname of parent */
	tail = nmbuf + strlen(nmbuf) - 1;	/* last char in nmbuf */
	while (*tail == '/' && tail > nmbuf) 	/* kill trailing slashes */
		*tail-- = '\0';

	/* is parent "."? */
	if ((tail = (char *)strrchr(nmbuf, '/')) != NULL) {
		*tail = '\0';
		parent = nmbuf;
	} else
		parent = ".";


	/* save mode of parent */
	if (stat(parent, &statbuf) != 0) {
	    DEBUG(5, "CAN'T STAT PARENT DIRECTORY %s ", parent);
	    DEBUG(5, "errno %d\n", errno);
	    return(FAIL);
	}

	  /* make parent writable by all (thus by real uid) */
	    if (chmod(parent, 0777) != 0) {
		DEBUG(5, "CAN'T CHMOD PARENT DIRECTORY %s ", parent);
		DEBUG(5, "errno %d\n", errno);
		return(FAIL);
	    }


	switch (pid = fork()) {
	case 0:			/* child */
		umask(0);
		/* close stdout and stderr to fail quietly */
		close(1);
		close(2);
		execl(_PATH_MKDIR, "mkdir", name, 0);
		_exit(1);

	case -1:		/* fork failed */
		ASSERT(pid != -1, Ct_FORK, "mkdir", errno);

	default:		/* parent */
		break;
	}

	while ((ret = wait(&status)) != pid );

	if (status != 0) {
	    errent(Ct_CREATE, name, ret, rcsid, __FILE__, __LINE__);
	    return(FAIL);
	}

/*
** changed to ensure that the directly is always set to 0777 instead of
** "statbuf.st_mode & 0777"
*/

	if ((statbuf.st_mode & 0777) != 0777) {
	  /* restore mode of parent directory */
			if (chmod(parent, statbuf.st_mode & 0777) != 0) {
				errent(Ct_CHMOD, parent, errno, rcsid, __FILE__, __LINE__);
				DEBUG(5, "CAN'T CHMOD PARENT %s\n", parent);
	        return(FAIL);
	    }
	}

	if (Uid != 0) {
		if (setuid(Uid) != 0) {
		    DEBUG(5, "CAN'T SETUID %d\n", Uid);
		    return(FAIL);
		}
		if ( chmod(name, (mask ^ 0777) & 0777) != 0) {
		    errent(Ct_CHMOD, name, errno, rcsid, __FILE__, __LINE__);
		    return(FAIL);
		};
		if (chown(name, UUCPUID, getgid()) != 0) {
	            DEBUG(5, "CHOWN FAILED %s  ", name);
		    DEBUG(5, "errno %d\n", errno);
		    errent(Ct_CHOWN, name, errno, rcsid, __FILE__, __LINE__);
		    setuid(Euid);
		    return(FAIL);
		}
		setuid(Euid);
    	}
	return(0);
}
#endif

#ifdef V7
mkdirs2(name, mask)
register char	*name;
register int	mask;
{
	int	ret;

	if (DIRECTORY(name))
		return(0);		/* directory exists */

	if (vfork() == 0) {
		setuid(Euid);	/* this makes it trivial in V7 */
		umask(mask);
		/* close stdout and stderr to fail quietly */
		close(1);
		close(2);
		execl(_PATH_MKDIR, "mkdir", name, 0);
		_exit(1);
	}
	wait(&ret);

	if (ret != 0)		/* why should we abort if some remote tries */
	    return(FAIL);	/* to make a garbage directory */

	return(0);
}
#endif

/*
 * expand file name and check return
 * print error if it failed.
 *	file	-> file name to check
 * returns: 
 *      0	-> ok
 *      FAIL	-> if expfile failed
 */
ckexpf(file)
char *file;
{
	if (expfile(file) == 0)
		return(0);

	fprintf(stderr, MSGSTR(MSG_EXPFILE1,"Illegal filename (%s).\n"), 
		  file);
	return(FAIL);
}


/*
 * make canonical path out of path passed as argument.
 *
 * Eliminate redundant self-references like // or /./
 * (A single terminal / will be preserved, however.)
 * Dispose of references to .. in the path names.
 * In relative path names, this means that .. or a/../..
 * will be treated as an illegal reference.
 * In full paths, .. is always allowed, with /.. treated as /
 *
 * returns:
 *	0	-> path is now in canonical form
 *	FAIL	-> relative path contained illegal .. reference
 */

int
canPath(path)
register char *path;	/* path is modified in place */
{
    register char *to, *fr;

    to = fr = path;
    if (*fr == '/') *to++ = *fr++;
    for (;;) {
	/* skip past references to self and validate references to .. */
	for (;;) {
	    if (*fr == '/') {
		fr++;
		continue;
	    }
	    if ((strncmp(fr, "./", 2) == SAME) || EQUALS(fr, ".")) {
		fr++;
		continue;
	    }
	    if ((strncmp(fr, "../", 3) == SAME) || EQUALS(fr, "..")) {
		fr += 2;
		/*	/.. is /	*/
		if (((to - 1) == path) && (*path == '/')) continue;
		/* error if no previous component */
		if (to <= path) return (FAIL);
		/* back past previous component */
		while ((--to > path) && (to[-1] != '/'));
		continue;
	    }
	    break;
	}
	/*
	 * What follows is a legitimate component,
	 * terminated by a null or a /
	 */
	if (*fr == '\0') break;
	while ((*to++ = *fr) && (*fr++ != '/'));
    }
    /* null path is . */
    if (to == path) *to++ = '.';
    *to = '\0';
    return (0);
}
