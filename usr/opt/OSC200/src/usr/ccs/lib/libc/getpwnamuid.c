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
static char     *sccsid = "@(#)$RCSfile: getpwnamuid.c,v $ $Revision: 4.3.4.3 $ (DEC) $Date: 1992/03/26 12:06:44 $";
#endif 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *#if defined(LIBC_SCCS) && !defined(lint)

 *#endif LIBC_SCCS and not lint
 */

#include <stdio.h>
#include <pwd.h>
#include <ndbm.h>

#include <sys/file.h>
#include <sys/fcntl.h>

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
# include <errno.h>

extern struct rec_mutex	_passwd_rmutex;
#else
static char line[BUFSIZ+1];
static struct passwd passwd;
#endif

/*
 * The following are shared with getpwent.c
 */
extern	char *_pw_file;
DBM	*_pw_db;
int	_pw_stayopen;


/*
* Thread-safe getpwnam
*/
#ifdef _THREAD_SAFE
int 
getpwnam_r(char *nam, struct passwd *pw, char *line, int linelen)
#else 
struct passwd *
getpwnam(char *nam)
#endif
{
	datum key;
#ifdef _THREAD_SAFE
	int	retval;
#else
	register struct passwd *pw;
#endif

#ifdef _THREAD_SAFE
	if ((pw == NULL) || (line == NULL) || (linelen < 1)) {
		seterrno(EINVAL);
		return(-1);
	}
	rec_mutex_lock(&_passwd_rmutex);
#endif

	if (_pw_db == (DBM *)0 &&
	    (_pw_db = dbm_open(_pw_file, O_RDONLY, 0)) == (DBM *)0) {
oldcode:
		setpwent();
#ifdef 	_THREAD_SAFE
		while (getpwent_r(pw, line, linelen) == 0)
		       if (strcmp(nam, pw->pw_name) == 0) {
				if (!_pw_stayopen)
					endpwent();
				rec_mutex_unlock(&_passwd_rmutex);
				return(0);
			}
		if (!_pw_stayopen)
			endpwent();
		rec_mutex_unlock(&_passwd_rmutex);
		seterrno(ENOENT);
		return(-1);
#else
		while ((pw = getpwent()) && strcmp(nam, pw->pw_name))
			;
		if (!_pw_stayopen)
			endpwent();
		return (pw);
#endif /* _THREAD_SAFE */
	}
	if (flock(dbm_dirfno(_pw_db), LOCK_SH) < 0) {
		dbm_close(_pw_db);
		_pw_db = (DBM *)0;
		goto oldcode;
	}
	key.dptr = nam;
	key.dsize = strlen(nam);
#ifdef _THREAD_SAFE
	retval = fetchpw(key, pw, line, linelen);
#else
	pw = fetchpw(key);
#endif
	(void) flock(dbm_dirfno(_pw_db), LOCK_UN);
	if (!_pw_stayopen) {
		dbm_close(_pw_db);
		_pw_db = (DBM *)0;
	}
#ifdef _THREAD_SAFE
	rec_mutex_unlock(&_passwd_rmutex);
	return(retval);
#else	
	return (pw);
#endif /* _THREAD_SAFE */
}

#ifdef _THREAD_SAFE
int
getpwuid_r(uid_t uid, struct passwd *pw, char *line, int linelen)
#else
struct passwd *
getpwuid(uid_t uid)
#endif
{
	datum key;
#ifdef _THREAD_SAFE
	int	retval;
#else
	register struct passwd *pw;
#endif /* _THREAD_SAFE */

#ifdef _THREAD_SAFE 
	if ((pw == NULL) || (line == NULL) || (linelen < 1)) {
		seterrno(EINVAL);
		return(-1);
	}
	rec_mutex_lock(&_passwd_rmutex);
#endif

	if (_pw_db == (DBM *)0 &&
	    (_pw_db = dbm_open(_pw_file, O_RDONLY, 0)) == (DBM *)0) {
oldcode:
		setpwent();
#ifdef _THREAD_SAFE 
		while (getpwent_r(pw, line, linelen) == 0)
			if (pw->pw_uid == uid) {
				if (!_pw_stayopen)
					endpwent();
				rec_mutex_unlock(&_passwd_rmutex);
				return(0);
			}
		if (!_pw_stayopen)
			endpwent();
		rec_mutex_unlock(&_passwd_rmutex);
		seterrno(ENOENT);
		return(-1);
#else
		while ((pw = getpwent()) && pw->pw_uid != uid)
			;
		if (!_pw_stayopen)
			endpwent();
		return (pw);
#endif /* _THREAD_SAFE  */
	}
	if (flock(dbm_dirfno(_pw_db), LOCK_SH) < 0) {
		dbm_close(_pw_db);
		_pw_db = (DBM *)0;
		goto oldcode;
	}
	key.dptr = (char *) &uid;
	key.dsize = sizeof uid;
#ifdef _THREAD_SAFE
	retval = fetchpw(key, pw, line, linelen);
#else /* _THREAD_SAFE */
	pw = fetchpw(key);
#endif /* _THREAD_SAFE */
	(void) flock(dbm_dirfno(_pw_db), LOCK_UN);
	if (!_pw_stayopen) {
		dbm_close(_pw_db);
		_pw_db = (DBM *)0;
	}
#ifdef _THREAD_SAFE 
	rec_mutex_unlock(&_passwd_rmutex);
	return(retval);
#else
	return (pw);
#endif
}
