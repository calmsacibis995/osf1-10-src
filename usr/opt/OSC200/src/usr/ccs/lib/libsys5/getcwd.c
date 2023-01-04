
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
static char *rcsid = "@(#)$RCSfile: getcwd.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/07/15 15:02:01 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * FUNCTIONS: getcwd
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * getcwd.c	1.12  com/lib/c/gen,3.1,8943 9/8/89 08:48:21
 */

#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

/*
 * NAME:	getcwd
 *
 * FUNCTION:	getcwd - get current working directory
 *
 * NOTES:	Getcwd gets the current working directory.
 *
 *		`buf' is a pointer to a character buffer into which the
 *		path name of the current directory is placed by the
 *		subroutine.  `buf' may be NULL, in which case the 
 *		subroutine will call malloc to get the required space.
 *		`size 'is the length of the buffer space for the path-name.
 *		If the actual path-name is longer than (size-2), or if
 *		the value of size is not at least 3, the subroutine will
 *		return a value of NULL, with errno set as appropriate.
 *
 * RETURN VALUE DESCRIPTION:
 *		NULL if anything fails, else
 *		`buf' if it is non-null, else
 *		a pointer to the malloc'ed memory containing the path
 */

/* The function getcwd() contains all the POSIX requirements */

#include <nl_types.h>
#define GETWDERR(s,n)

static int realloc_buffer(char **, char **, char **);

#define INIT_SIZE	(PATH_MAX+1)

#pragma weak getcwd = __getcwd
char *
__getcwd(char *pathname, int size)
{
	char *pnptr;			/* intermediate pathname pointer */
	char cdir[INIT_SIZE];		/* initial current directory buffer */
	char *curdir;			/* current directory buffer pointer */
	char *dptr;			/* directory pointer */
	char *eptr;			/* end of directory pointer */
	long cdev, rdev;		/* current & root vfs id */
	ino_t cino, rino;		/* current & root inode number */
	DIR *dirp;			/* directory stream */
	struct dirent *dir;		/* directory entry struct */
	struct stat d, dd;		/* file status struct */
	int malloced;			/* allocated pathname buffer */
	char *dent;			/* directory entry name */
	size_t len;			/* directory entry length */
	char *tp;			/* temporary copy pointer */
	struct statfs statfs_buf;	/* statfs for fs of curdir */
	int have_hint = 0;		/* have a hint available */
	char *hint = NULL;		/* pointer to hint component */

	/*
	 * check lowest size possible 
	 * POSIX states that if the size argument is less than or equal to 0,
	 * then EINVAL is returned.
	 */
	if (size <= 0) {
		if (pathname != NULL)
			GETWDERR("getwd: buffer size too small", M_BADSIZE);
		_Seterrno(EINVAL);
		return(NULL);
	}

	/* null buffer? */
	if (malloced = (pathname == NULL)) {
		if ((pathname = (char *) malloc((size_t) size)) == NULL) {
			_Seterrno(ENOMEM);
			return(NULL);
		}
	}

	if (stat("/", &d) < 0) {
		if (malloced) free(pathname);
		else GETWDERR("getwd: can't stat /", M_RSTAT);
		return (NULL);
	}
	rdev = d.st_dev;
	rino = d.st_ino;

	curdir = cdir;
	dptr = cdir;
	eptr = &cdir[sizeof(cdir)-1];
	*dptr++ = '.'; *dptr++ = '/'; *dptr = '\0';
	if (stat(curdir, &d) < 0) {
		if (malloced) free(pathname);
		else GETWDERR("getwd: can't stat .", M_HSTAT);
		return (NULL);
	}

	/*
	 * If we're on the root, no need for this call.
	 */
	if (d.st_dev != rdev) {
		if (statfs(curdir, &statfs_buf) < 0) {
			if (malloced) free(pathname);
			else GETWDERR("getwd: can't statfs .", M_STATFS);
			return (NULL);
		}
		if (statfs_buf.f_mntonname[0] == '/')
			have_hint = 1;
	}

	pnptr = &pathname[size-1];
	*pnptr = '\0';
	for (;;) {
		if (d.st_ino == rino && d.st_dev == rdev)
			break;		/* reached root directory */
		cino = d.st_ino;
		cdev = d.st_dev;
		if (dptr + 3 > eptr &&
		    realloc_buffer(&curdir, &dptr, &eptr)) {
			if (malloced) free(pathname);
			else GETWDERR("getwd: out of memory", M_NOMEM);
			_Seterrno(ENOMEM);
			return (NULL);
		}
		*dptr++ = '.'; *dptr++ = '.'; *dptr++ = '/'; *dptr = '\0';
		if (lstat(curdir, &d) < 0) {
			if (malloced) free(pathname);
			else GETWDERR("getwd: can't lstat ..", M_LSTAT);
			return (NULL);
		}
		if (have_hint && cdev != d.st_dev) {
			for (tp = statfs_buf.f_mntonname; *tp; tp++)
				if (*tp == '/')
					hint = tp;
			hint++;
			if (*hint == '\0') {
				tp = statfs_buf.f_mntonname;
				while (hint > tp && *hint == '/')
					*hint-- = '\0';
				while (hint > tp && *hint != '/')
					hint--;
				hint++;
			}
			have_hint = 0;
		}
		if (hint) {
			dent = hint;
			len = (size_t) strlen(dent);
			if (dptr + len > eptr &&
			    realloc_buffer(&curdir, &dptr, &eptr)) {
				if (malloced) free(pathname);
				else GETWDERR("getwd: out of memory", M_NOMEM);
				_Seterrno(ENOMEM);
				return (NULL);
			}
			memcpy(dptr, dent, len + 1);
			if (lstat(curdir, &dd) < 0) {
				if (malloced) free(pathname);
				else GETWDERR("getwd: can't lstat ..", M_LSTAT);
				return (NULL);
			}
			if (dd.st_ino != cino || dd.st_dev != cdev) {
				*dptr = '\0';
				hint = NULL;
			} else {
				hint--;
				tp = statfs_buf.f_mntonname;
				while (hint > tp && *hint == '/')
					*hint-- = '\0';
				while (hint > tp && *hint != '/')
					hint--;
				hint++;
			}
		}
		if (hint == NULL) {
			if ((dirp = opendir(curdir)) == NULL) {
				if (malloced) free(pathname);
				else GETWDERR("getwd: can't open ..", M_OPENPAR);
				return (NULL);
			}
		} else
			dirp = NULL;
		if (hint == NULL && cdev == d.st_dev) {
			if (cino == d.st_ino) {
				/* reached root directory */
				closedir(dirp);
				break;
			}
			do {
				if ((dir = readdir(dirp)) == NULL) {
					closedir(dirp);
					if (malloced) free(pathname);
					else GETWDERR("getwd: read error in ..", M_READPAR);
					/* readdir() needn't set errno on EOF*/
					if (!_Geterrno())
						_Seterrno(ENOENT);
					return (NULL);
				}
			} while (dir->d_fileno != cino);
			dent = dir->d_name;
			len = (size_t) strlen(dent);
		} else if (hint == NULL) {
			do {
				if ((dir = readdir(dirp)) == NULL) {
					closedir(dirp);
					if (malloced) free(pathname);
					else GETWDERR("getwd: read error in ..", M_READPAR);
					/* readdir() may not set errno */
					if (!_Geterrno())
						_Seterrno(ENOENT);
					return (NULL);
				}
				dent = dir->d_name;
				len = (size_t) strlen(dent);
				if (dptr + len > eptr &&
				    realloc_buffer(&curdir, &dptr, &eptr)) {
					closedir(dirp);
					if (malloced) free(pathname);
					else GETWDERR("getwd: out of memory", M_NOMEM);
					_Seterrno(ENOMEM);
					return (NULL);
				}
				memcpy(dptr, dent, len + 1);
				lstat(curdir, &dd);
			} while(dd.st_ino != cino || dd.st_dev != cdev);
		}
		pnptr -= len + 1;
		if (pnptr < pathname) {
			if (dirp) closedir(dirp);
			if (malloced) free(pathname);
			else GETWDERR("getwd: path too long", M_PATHTOOLONG);
			_Seterrno(ERANGE);
			return (NULL);
		}
		tp = pnptr;
		*tp++ = '/';
		while (len-- > 0)
			*tp++ = *dent++;
		if (dirp) closedir(dirp);
	}
	if (*pnptr == '\0')		/* current dir == root dir */
		strcpy(pathname, "/");
	else
		memmove(pathname, pnptr, size-(pnptr-pathname));
	return (pathname);
}

static int
realloc_buffer(char **curdirp, char **dptrp, char **eptrp)
{
	size_t nsize = (size_t) ((*eptrp+1-*curdirp)+INIT_SIZE);
	int used = *dptrp-*curdirp;

	if (nsize == 2*INIT_SIZE)
		*curdirp = (char *) malloc(nsize);
	else
		*curdirp = (char *) realloc((void *)*curdirp, nsize);
	if (*curdirp == NULL)
		return (1);
	*dptrp = *curdirp+used;
	*eptrp = *curdirp+nsize-1;
	return (0);
}




