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
static char rcsid[] = "@(#)$RCSfile: ulockf.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:08:28 $";
#endif
/* 
 * COMPONENT_NAME: UUCP ulockf.c
 * 
 * FUNCTIONS: checkLock, 
 *            onelock, rmlock, stlock, ulockf, ultouch 
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
/*
ulockf.c	1.5  com/cmd/uucp,3.1,9013 10/28/89 15:07:24";
*/
/*	/sccs/src/cmd/uucp/s.ulockf.c
	ulockf.c	1.4	7/29/85 16:33:30
*/
#include "uucp.h"
/* VERSION( ulockf.c	5.2 -  -  ); */

#ifdef	V7
#define O_RDONLY	0
#endif

static void	stlock();
static int	onelock();

#ifdef ATTSVKILL
/*
 * create a lock file (file).
 * If one already exists, send a signal 0 to the process--if
 * it fails, then unlink it and make a new one.
 *
 * input:
 *	file - name of the lock file
 *	atime - is unused, but we keep it for lint compatibility with non-ATTSVKILL
 *
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
ulockf(file, atime)
register char *file;
time_t atime;
{
#ifdef	ASCIILOCKS
	static	char pid[SIZEOFPID+2] = { '\0' }; /* +2 for '\n' and NULL */
#else
	static int pid = -1;
#endif

	static char tempfile[MAXNAMESIZE];

#ifdef	ASCIILOCKS
	if (pid[0] == '\0') {
		(void) sprintf(pid, "%*d\n", SIZEOFPID, getpid());
#else
	if (pid < 0) {
		pid = getpid();
#endif
		(void) sprintf(tempfile, "%s/LTMP.%d", X_LOCKDIR, getpid());
	}

	if (onelock(pid, tempfile, file) == -1) {
		(void) unlink(tempfile);
		if (checkLock(file))
			return(FAIL);
		else {
		    if (onelock(pid, tempfile, file)) {
			(void) unlink(tempfile);
			DEBUG(4,"ulockf failed in onelock()\n","");
			return(FAIL);
		    }
		}
	}

	stlock(file);
	return(0);
}

/*
 * check to see if the lock file exists and is still active
 * - use kill(pid,0) - (this only works on ATTSV and some hacked
 * BSD systems at this time)
 * return:
 *	0	-> success (lock file removed - no longer active
 *	FAIL	-> lock file still active
 */
checkLock(file)
register char *file;
{
	register int ret;
	int lpid = -1;
#ifdef	ASCIILOCKS
	char alpid[SIZEOFPID+2];	/* +2 for '\n' and NULL */
#endif
	int fd;
	extern int errno;

	fd = open(file, O_RDONLY);
	DEBUG(4, "ulockf file %s\n", file);
	if (fd == -1) {
	    if (errno == ENOENT)  /* file does not exist -- OK */
		return(0);
	    DEBUG(4,"Lock File--can't read (errno %d) --remove it!\n", errno);
	    goto unlk;
	}
#ifdef	ASCIILOCKS
	ret = read(fd, (char *) alpid, SIZEOFPID+1); /* +1 for '\n' */
	(void) close(fd);
	if (ret != (SIZEOFPID+1)) {
#else
	ret = read(fd, (char *) &lpid, sizeof(int));
	(void) close(fd);
	if (ret != sizeof(int)) {
#endif

	    DEBUG(4, "Lock File--bad format--remove it!\n", "");
	    goto unlk;
	}
#ifdef	ASCIILOCKS
	lpid = atoi(alpid);
#endif
	if ((ret=kill(lpid, 0)) == 0 || errno == EPERM) {
	    DEBUG(4, "Lock File--process still active--not removed\n","");
	    return(FAIL);
	}
	else { /* process no longer active */
	    DEBUG(4, "kill pid (%d), ", lpid);
	    DEBUG(4, "returned %d", ret);
	    DEBUG(4, "--ok to remove lock file (%s)\n", file);
	}
unlk:
	
	if (unlink(file) != 0) {
		DEBUG(4,"ulockf failed in unlink()\n","");
		return(FAIL);
	}
	return(0);
}
#else	/* !ATTSVKILL */

/*
 * check to see if the lock file exists and is still active
 * - consider the lock expired after SLCKTIME seconds.
 * return:
 *	0	-> success (lock file removed - no longer active
 *	FAIL	-> lock file still active
 */
checkLock(file)
register char *file;
{
	register int ret;
	int lpid = -1;
	int fd;
	extern int errno;
	struct stat stbuf;
	time_t ptime, time();

	fd = open(file, 0);
	DEBUG(4, "ulockf file %s\n", file);
	if (fd == -1) {
	    if (errno == ENOENT)  /* file does not exist -- OK */
		return(0);
	    DEBUG(4,"Lock File--can't read (errno %d) --remove it!\n", errno);
	    goto unlk;
	}
	ret = stat(file, &stbuf);
	if (ret != -1) {
		(void) time(&ptime);
		if ((ptime - stbuf.st_ctime) < SLCKTIME) {

			/*
			 * file not old enough to delete
			 */
			return(FAIL);
		}
	}
unlk:
	DEBUG(4, "--ok to remove lock file (%s)\n", file);
	
	if (unlink(file) != 0) {
		DEBUG(4,"ulockf failed in unlink()\n","");
		return(FAIL);
	}
	return(0);
}


/*
 * create a lock file (file).
 * If one already exists, the create time is checked for
 * older than the age time (atime).
 * If it is older, an attempt will be made to unlink it
 * and create a new one.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
ulockf(file, atime)
register char *file;
time_t atime;
{
	register int ret;
	struct stat stbuf;
#ifdef	ASCIILOCKS
	static	char pid[SIZEOFPID+2] = { '\0' }; /* +2 for '\n' and null */
#else
	static int pid = -1;
#endif
	static char tempfile[MAXNAMESIZE];
	time_t ptime, time();

#ifdef	ASCIILOCKS
	if (pid[0] == '\0') {
		(void) sprintf(pid, "%*d\n", SIZEOFPID, getpid());
#else
	if (pid < 0) {
		pid = getpid();
#endif
		(void) sprintf(tempfile, "%s/LTMP.%d", X_LOCKDIR, getpid());
	}
	if (onelock(pid, tempfile, file) == -1) {

		/*
		 * lock file exists
		 * get status to check age of the lock file
		 */
		(void) unlink(tempfile);
		ret = stat(file, &stbuf);
		if (ret != -1) {
			(void) time(&ptime);
			if ((ptime - stbuf.st_ctime) < atime) {

				/*
				 * file not old enough to delete
				 */
				return(FAIL);
			}
		}
		ret = unlink(file);
		ret = onelock(pid, tempfile, file);
		if (ret != 0)
			return(FAIL);
	}
	stlock(file);
	return(0);
}
#endif	/* ATTSVKILL */

#define MAXLOCKS 10	/* maximum number of lock files */
char *Lockfile[MAXLOCKS];
int Nlocks = 0;

/*
 * put name in list of lock files
 * return:
 *	none
 */
static
void
stlock(name)
char *name;
{
	register int i;
	char *p;

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			break;
	}
	ASSERT(i < MAXLOCKS, MSGSTR(MSG_ULOCKFA1,"TOO MANY LOCKS"), "", i);
	if (i >= Nlocks)
		i = Nlocks++;
	p = calloc((unsigned) strlen(name) + 1, sizeof (char));
	ASSERT(p != NULL, MSGSTR(MSG_ULOCKFA2,"CAN NOT ALLOCATE FOR"), name, 0);
	(void) strcpy(p, name);
	Lockfile[i] = p;
	return;
}

/*
 * remove all lock files in list
 * return:
 *	none
 */
void
rmlock(name)
register char *name;
{
	register int i;

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		if (name == NULL || EQUALS(name, Lockfile[i])) {
			(void) unlink(Lockfile[i]);
			(void) free(Lockfile[i]);
			Lockfile[i] = NULL;
		}
	}
	return;
}

/*
 * update access and modify times for lock files
 * return:
 *	none
 */
void
ultouch()
{
	register int i;
	time_t time();

	struct ut {
		time_t actime;
		time_t modtime;
	} ut;

	ut.actime = time(&ut.modtime);
	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		utime(Lockfile[i], &ut);
	}
	return;
}

/*
 * makes a lock on behalf of pid.
 * input:
 *	pid - process id
 *	tempfile - name of a temporary in the same file system
 *	name - lock file name (full path name)
 * return:
 *	-1 - failed
 *	0  - lock made successfully
 */
static
onelock(pid,tempfile,name)
#ifdef	ASCIILOCKS
char *pid;
#else
int pid;
#endif 
char *tempfile, *name;
{	
	register int fd;
	char	cb[100];

	fd=creat(tempfile, 0444);
	if(fd < 0){
		(void) sprintf(cb, "%s %s %d",tempfile, name, errno);
		logent(MSGSTR(MSG_ULOCKFL1,"ULOCKC"), cb);
		if((errno == EMFILE) || (errno == ENFILE))
			(void) unlink(tempfile);
		return(-1);
	}
#ifdef	ASCIILOCKS
	(void) write(fd, pid, SIZEOFPID+1);	/* +1 for '\n' */
#else
	(void) write(fd,(char *) &pid,sizeof(int));
#endif
	(void) chmod(tempfile,0444);
	(void) chown(tempfile, UUCPUID, UUCPGID);
	(void) close(fd);
	if(link(tempfile,name)<0){
		DEBUG(4, "%s: ", sys_errlist[errno]);
		DEBUG(4, "link(%s, ", tempfile);
		DEBUG(4, "%s)\n", name);
		if(unlink(tempfile)< 0){
			(void) sprintf(cb, "ULK err %s %d", tempfile,  errno);
			logent(MSGSTR(MSG_ULOCKFL2, "ULOCKLNK"), cb);
		}
		return(-1);
	}
	if(unlink(tempfile)<0){
		(void) sprintf(cb, "%s %d",tempfile,errno);
		logent(MSGSTR(MSG_ULOCKFL3, "ULOCKF"), cb);
	}
	return(0);
}
