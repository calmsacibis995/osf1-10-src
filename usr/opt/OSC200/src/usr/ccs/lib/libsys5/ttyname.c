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
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * ttyname.c	5.2 (Berkeley) 3/9/86
 */

/*
 * ttyname(f): return the absolute pathname of the tty special file
 *  NULL if it is not a tty
 */

#define	NULL	0
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pty.h>
#ifdef _THREAD_SAFE
#include <errno.h>
#endif

static	char	dev[]	= "/dev/";
char	*strcpy();
char	*strcat();                                                 

#ifdef _THREAD_SAFE
#define	FAIL	-1
#else
#define	FAIL	NULL
#endif

static char *
#ifdef _THREAD_SAFE
fastttyname(int fd, char *ttytemp, int len)
#else
fastttyname(int fd)
#endif
{
	struct stat stb ;
	struct stat fstb ;
	int i ;

#ifndef _THREAD_SAFE
	static char ttytemp[MAX_PTYNAME_LEN];
#endif
	i = fstat(fd, &fstb) ;	
	if ((i <0)  || (fstb.st_rdev < 0 ))
		return(NULL);
	sprintf(ttytemp,"%s%d",_PATH_PTY,minor(fstb.st_rdev));
	if (stat(ttytemp, &stb) >= 0 && stb.st_rdev == fstb.st_rdev )
		return(ttytemp);
	return(NULL);	
}

	

#ifdef _THREAD_SAFE
int
ttyname_r(int f, char *rbuf, int len)
#else
#pragma weak ttyname = __ttyname
char *
__ttyname(int f)
#endif
{
	struct stat fsb;
	struct stat tsb;
	register struct dirent *db;
	register DIR *df;
	char *p;
#ifndef _THREAD_SAFE
	static char rbuf[32];
#else

	if ((rbuf == NULL) || (len < 32)) {
		seterrno(EINVAL);
		return(FAIL);
	}
#endif

	if (isatty(f)==0)
		return(FAIL);
	if (fstat(f, &fsb) < 0)
		return(FAIL);
	if ((fsb.st_mode&S_IFMT) != S_IFCHR)
		return(FAIL);

#ifdef _THREAD_SAFE
	if ((p = fastttyname(f, rbuf, len)) != NULL)
		return(0);
#else
	if ((p = fastttyname(f)) != NULL)
		return(p);
#endif

	if ((df = opendir(dev)) == NULL)
		return(FAIL);

	while ((db = readdir(df)) != NULL) {
		if (db->d_fileno != fsb.st_ino)
			continue;
		strcpy(rbuf, dev);
		strcat(rbuf, db->d_name);
		if (stat(rbuf, &tsb) < 0)
			continue;
		if (tsb.st_dev == fsb.st_dev && tsb.st_ino == fsb.st_ino) {
			closedir(df);
#ifdef _THREAD_SAFE
			return(0);
#else
			return(rbuf);
#endif
		}
	}
	closedir(df);
#ifdef _THREAD_SAFE
	seterrno(ENOENT);
#endif
	return(FAIL);
}

