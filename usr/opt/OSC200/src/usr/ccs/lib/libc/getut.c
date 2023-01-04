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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getut.c,v $ $Revision: 4.2.8.3 $ (OSF) $Date: 1993/11/09 14:51:32 $";
#endif
/*
 * FUNCTIONS: getutent, getutid, getutline, pututline, setutent entutent, 
 *            utmpname, gdebug 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * getut.c	1.5  com/lib/c/adm,3.1,8943 9/13/89 10:28:56
 */

/*                                                                    
 * FUNCTION: Routines to read and write the /etc/utmp file.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - struct utmp on success
 *	     - NULL on failure or EOF
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak endutent_r = __endutent_r
#pragma weak getutent_r = __getutent_r
#pragma weak getutid_r = __getutid_r
#pragma weak getutline_r = __getutline_r
#pragma weak pututline_r = __pututline_r
#pragma weak setutent_r = __setutent_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak endutent = __endutent
#pragma weak getutent = __getutent
#pragma weak getutid = __getutid
#pragma weak getutline = __getutline
#pragma weak pututline = __pututline
#pragma weak setutent = __setutent
#endif
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <utmp.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "ts_supp.h"

#define	FAILURE	-1


#ifdef _THREAD_SAFE

#include "rec_mutex.h"

extern struct rec_mutex _utmp_rmutex;

#define	SETUTENT()	setutent_r(utmp_data)
#define	GETUTENT()	getutent_r(utmp, utmp_data)
#define	GETUTID(u)	getutid_r(u, utmp, utmp_data)
#define	UT_FOUND(u)	(*utmp = u, TS_SUCCESS)

#define	UT_FD		(utmp_data->ut_fd)
#define	LOC_UTMP	(utmp_data->loc_utmp)
#define	UBUF		(utmp_data->ubuf)

#else

#define	SETUTENT()	setutent()
#define	GETUTENT()	getutent()
#define	GETUTID(u)	getutid(u)
#define	UT_FOUND(u)	(u)

#define	UT_FD		ut_fd
#define	LOC_UTMP	loc_utmp
#define	UBUF		ubuf

static int		ut_fd = FAILURE;	/* fd for the utmp file. */
static long		loc_utmp;		/* offset in utmp of ubuf */
static struct utmp	ubuf;			/* last entry read in */

#endif	/* _THREAD_SAFE */


#define	MAXFILE	PATH_MAX + 1	/* max utmp filename length (inc. NULL) */

extern char __utmpfile[];


#ifdef _THREAD_SAFE
int
getutent_r(struct utmp **utmp, struct utmp_data *utmp_data)
#else
struct utmp *
getutent(void)
#endif	/* _THREAD_SAFE */
{
	int	err;

	/*
	 * If the "utmp" file is not open, attempt to open it for
	 * reading.  If there is no file, attempt to create one.  If
	 * both attempts fail, return NULL.  If the file exists, but
	 * isn't readable and writeable, do not attempt to create.
	 */
	TS_EINVAL((utmp == 0) || (utmp_data == 0));

	if (UT_FD == FAILURE) {
		TS_LOCK(&_utmp_rmutex);
		if ((UT_FD = open(__utmpfile, O_RDWR)) < 0) {
			/*
			 * If the open failed because the file didn't exist,
			 * then try to create one.
			 */
			if ((err = _Geterrno()) == ENOENT) {
				if ((UT_FD =
				    open(__utmpfile, O_RDWR|O_CREAT, 0644)) < 0) {
					TS_UNLOCK(&_utmp_rmutex);
					return (TS_FAILURE);
				}
			} else if (err == EACCES) {
				/*
				 * If the open failed for permissions, try
				 * opening it only for reading.  All
				 * "pututline()" later will fail the writes.
				 */
				if ((UT_FD = open(__utmpfile, O_RDONLY)) < 0) {
					TS_UNLOCK(&_utmp_rmutex);
					return (TS_FAILURE);
				}
			} else {
				TS_UNLOCK(&_utmp_rmutex);
				return (TS_FAILURE);
			}
		}
		TS_UNLOCK(&_utmp_rmutex);
	}

	/*
	 * Try to read in the next entry from the utmp file.  If the
	 * read fails, return NULL.
	 */

	if (read(UT_FD, (char *)&UBUF, sizeof(UBUF)) != sizeof(UBUF)) {
		/*
		 * Make sure ubuf is zeroed.
		 */
		LOC_UTMP = 0L;
		memset((void *)&UBUF, 0, sizeof(UBUF));
		UBUF.ut_type = EMPTY;
		return (TS_FAILURE);
	}

	/*
	 * Save the location in the file where this entry was found.
	 */

	LOC_UTMP = lseek(UT_FD, 0L, SEEK_CUR) - (long)(sizeof(struct utmp));

	return (UT_FOUND(&UBUF));
}


#ifdef _THREAD_SAFE
int
getutid_r(register struct utmp *utent,
	  struct utmp **utmp, struct utmp_data *utmp_data)
#else
struct utmp *
getutid(register struct utmp *utent)
#endif	/* _THREAD_SAFE */
{
	register short	type;

	TS_EINVAL((utent == 0) || (utmp == 0) || (utmp_data == 0));

	/*
	 * Start looking for entry.  Look in our current buffer before
	 * reading in new entries.
	 */
	do {
		/*
		 * If there is no entry in "ubuf", skip to the read.
		 */
		if (UBUF.ut_type != EMPTY) {
			switch(utent->ut_type) {
			case EMPTY :
				/*
				 * Do not look for an entry if the user sent
				 * us an EMPTY entry.
				 */
				TS_ERROR(EINVAL);
				return (TS_FAILURE);
			case RUN_LVL :
			case BOOT_TIME :
			case OLD_TIME :
			case NEW_TIME :
				/*
				 * For RUN_LVL, BOOT_TIME, OLD_TIME, and
				 * NEW_TIME entries, only the types have to
				 * match.  If they do, return the address of
				 * internal buffer.
				 */

				if (utent->ut_type == UBUF.ut_type) {
					return(UT_FOUND(&UBUF));
				}
				break;

			case INIT_PROCESS :
			case LOGIN_PROCESS :
			case USER_PROCESS :
			case DEAD_PROCESS :
				/*
				 * For INIT_PROCESS, LOGIN_PROCESS,
				 * USER_PROCESS, and DEAD_PROCESS the type
				 * of the entry in "ubuf", must be one of the
				 * above and id's must match.
				 */
				type = UBUF.ut_type;
				if ((type == INIT_PROCESS || 
				     type == LOGIN_PROCESS || 
				     type == USER_PROCESS  || 
				     type == DEAD_PROCESS) &&
				     (strcmp (UBUF.ut_id, utent->ut_id) == 0)) {
					return(UT_FOUND(&UBUF));
				}
				break;
			default :
				/*
				 * Do not search for illegal types of entry.
				 */
				TS_ERROR(EINVAL);
				return(TS_FAILURE);
			}
		}
	} while (GETUTENT() != TS_FAILURE);

	/*
	 * Return failure since the proper entry wasn't found.
	 */
	return (TS_NOTFOUND);
}


/*	"getutline" searches the "utmp" file for a LOGIN_PROCESS or	*/
/*	USER_PROCESS with the same "line" as the specified "entry".	*/

#ifdef _THREAD_SAFE
int
getutline_r(register struct utmp *utent,
	    struct utmp **utmp, struct utmp_data *utmp_data)
#else
struct utmp *
getutline(register struct utmp *utent)
#endif	/* _THREAD_SAFE */
{
	register struct utmp	*cur;

	TS_EINVAL((utent == 0) || (utmp == 0) || (utmp_data == 0));

	/*
	 * Start by using the entry currently incore.  This prevents
	 * doing reads that aren't necessary.
	 */
	cur = &UBUF;
	do {
		/*
		 * If the current entry is the one we are interested in, return
		 * a pointer to it.
		 */
		if (cur->ut_type != EMPTY && (cur->ut_type == LOGIN_PROCESS ||
		    cur->ut_type == USER_PROCESS) &&
		    strncmp(utent->ut_line, cur->ut_line,
			    sizeof(cur->ut_line)) == 0) {
			return(UT_FOUND(cur));
		}
	} while (GETUTENT() != TS_FAILURE);

	/*
	 * Since entry wasn't found, return NULL.
	 */
	return (TS_NOTFOUND);
}

/*
 * Write the structure sent into the utmp file
 * If there is already an entry with the same id, then it is
 * overwritten, otherwise a new entry is made at the end of the
 * utmp file.
 */
#ifdef _THREAD_SAFE
int
pututline_r(struct utmp *utent, struct utmp_data *utmp_data)
{
	struct utmp	*u;
	struct utmp	**utmp = &u;
#else
struct utmp *
pututline(struct utmp *utent)
{
#endif	/* _THREAD_SAFE */
	char		utmplock[MAXFILE + 4];
	struct stat	statbuf;
	int		err;
	struct utmp	tmpbuf;
#ifndef _THREAD_SAFE
	struct utmp	*answer;
#endif	/* _THREAD_SAFE */

	/*
	 * Copy the user supplied entry into our temporary buffer to
	 * avoid the possibility that the user is actually passing us
	 * the address of "ubuf".
	 */
	tmpbuf = *utent;
	utent = &tmpbuf;

	/*
	 * Create a lock link file before attempting to modify the utmp
	 * file.  Wait for up to sixty seconds, but then continue
	 * regardless of the lock file. If sixty seconds isn't enough time,
	 * the original creator of the lock probably has died.
	 */
	TS_LOCK(&_utmp_rmutex);
	sprintf(utmplock, "%s.lck", __utmpfile);

	while (link(__utmpfile, utmplock) == FAILURE) {
		if ((err = _Geterrno()) == EEXIST) {
			if (stat(__utmpfile, &statbuf) != FAILURE) {
				if (time((time_t *)0) - statbuf.st_ctime > 60)
					unlink(utmplock);
				else
					sleep((unsigned)3);
			}
		} else if (err == ENOENT) {
			/*
			 * If the utmp file doesn't exist, make one by trying
			 * to find the entry of interest.  "getutent()" will
			 * create the file.
			 */
			(void)GETUTENT();
			if (UT_FD == FAILURE) {
				TS_UNLOCK(&_utmp_rmutex);
				return (TS_FAILURE);
			}
		} else {
			TS_UNLOCK(&_utmp_rmutex);
			return (TS_FAILURE);
		}
	}
	TS_UNLOCK(&_utmp_rmutex);

	/*
	 * Find the proper entry in the utmp file.  Start at the current
	 * location.  If it isn't found from here to the end of the
	 * file, then reset to the beginning of the file and try again.
	 * If it still isn't found, then write a new entry at the end of
	 * the file.  (Making sure the location is an integral number of
	 * utmp structures into the file incase the file is scribbled.)
	 */

	if (GETUTID(&tmpbuf) == TS_FAILURE) {
		SETUTENT();
		if (GETUTID(&tmpbuf) == TS_FAILURE) {
			LOC_UTMP = lseek(UT_FD, 0L, SEEK_CUR);
			LOC_UTMP -= LOC_UTMP % sizeof(struct utmp);
	  	}
	}

	/*
	 * Seek to the proper place on the file descriptor for writing.
	 */
	(void)lseek(UT_FD, LOC_UTMP, SEEK_SET);

	/*
	 * Write out the user supplied structure.  If the write fails,
	 * then the user probably doesn't have permission to write the
	 * utmp file.
	 */
	if (write(UT_FD, (char *)utent, (unsigned)sizeof(struct utmp)) !=
			sizeof(struct utmp)) {
#ifdef _THREAD_SAFE
		err = -1;
#else
		answer = (struct utmp *)NULL;
#endif	/* _THREAD_SAFE */
	} else {
		/*
		 * Copy the user structure into ubuf so that it will be up to
		 * date in the future.
		 */
		UBUF = *utent;
#ifdef _THREAD_SAFE
		err = 0;
#else
		answer = &UBUF;
#endif	/* _THREAD_SAFE */

	}

	/*
	 * Remove the lock file (even if it wasn't your own creation).
	 */
	unlink(utmplock);

#ifdef _THREAD_SAFE
	return (err);
#else
	return (answer);
#endif	/* _THREAD_SAFE */
}

#ifdef _THREAD_SAFE
void
setutent_r(struct utmp_data *utmp_data)
#else
void
setutent(void)
#endif	/* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (utmp_data == 0) return;
#endif	/* _THREAD_SAFE */

	if (UT_FD != FAILURE)
		(void)lseek(UT_FD, 0L, SEEK_SET);

	LOC_UTMP = 0L;
	memset((void *)&UBUF, 0, sizeof(UBUF));
	UBUF.ut_type = EMPTY;
}
 

#ifdef _THREAD_SAFE
void
endutent_r(struct utmp_data *utmp_data)
#else
void
endutent(void)
#endif	/* _THREAD_SAFE */
{
#ifdef _THREAD_SAFE
	if (utmp_data == 0) return;
#endif	/* _THREAD_SAFE */

	if (UT_FD != FAILURE)
		close(UT_FD);
	UT_FD = FAILURE;

	LOC_UTMP = 0L;
	memset((void *)&UBUF, 0, sizeof(UBUF));
	UBUF.ut_type = EMPTY;
}
