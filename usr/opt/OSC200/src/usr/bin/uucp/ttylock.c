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
static char rcsid[] = "@(#)$RCSfile: ttylock.c,v $ $Revision: 4.3.6.2 $ (DEC) $Date: 1993/09/07 16:08:22 $";
#endif
/* 
 * COMPONENT_NAME: LIBCTTY Standard C Library I/O Functions
 * 
 * FUNCTIONS: BASENAME, DEBUG, EQUALS, clrlock, mklock, putlock, 
 *            tlockf, ttylock, ttylocked, ttytouchlock, ttyunlock, 
 *            ttywait 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*  Routines modified from Basic Networking Utilities (BNU) 1.0 */
/*  ttylock.c	1.1  com/lib/c/tty,3.1,8943 10/28/89 14:48:14"; */

#include "uucp.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define MAXLOCKS 10    /* maximum number of lock files */
char *lckarray[MAXLOCKS];
int numlocks = 0;

int ttylockDebug;
static mklock();

char *Bnptr;		/* used when BASENAME macro is expanded */
static int mklock();


/*
 * lock device.
 *
 * return:
 *	0	-> success
 *	-1	-> failure
 */
ttylock(device)
char *device;
{
	char lname[MAXNAMESIZE];

	(void) sprintf(lname, "%s.%s", LOCKPRE, device);
	BASENAME(lname, '/')[MAXBASENAME] = '\0';
	return(tlockf(device, lname) < 0 ? -1 : 0);
}

/*
 *
 * lock filename.
 *
 * Return:-
 *	 0 on successful completion. 
 * 	-1 if "device" is locked by another process.
 *
 */
tlockf(device, file)
register char *device, *file;
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
		(void) sprintf(tempfile, "%s/LTMP.%d", DEVICE_LOCKPRE, getpid());
	}

	if (mklock(pid, tempfile, file) == -1) {
		(void) unlink(tempfile);
		if (ttylocked(device))
			return(-1);
		else {
		    if (mklock(pid, tempfile, file)) {
			(void) unlink(tempfile);
			DEBUG(4,"ttylock failed in mklock()\n","");
			return(-1);
		    }
		}
	}

	putlock(file);
	return(0);
}

/*
 * check to see if the lock file exists and is still active
 *
 * return:
 *	0	-> success (lock file removed - no longer active)
 *	-1	-> lock file still active
 */
ttylocked(device)
register char *device;
{
	register int ret;
	int lpid = -1;
#ifdef	ASCIILOCKS
	char alpid[SIZEOFPID+2];	/* +2 for '\n' and NULL */
#endif
	int fd;
	extern int errno;
	char file[MAXNAMESIZE];

	sprintf (file, "%s.%s", LOCKPRE, device);
	fd = open(file, O_RDONLY);
	DEBUG(4, "ttylock file %s\n", file);
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
	    return(-1);
	}
	else { /* process no longer active */
	    DEBUG(4, "kill pid (%d), ", lpid);
	    DEBUG(4, "returned %d", ret);
	    DEBUG(4, "--ok to remove lock file (%s)\n", file);
	}
unlk:
	
	if (unlink(file) != 0) {
		DEBUG(4,"ttylock failed in unlink()\n","");
		return(-1);
	}
	return(0);
}


/*
 * put name in list of lock files
 * return:
 *	none
 */
putlock(name)
char *name;
{
	register int i;
	char *p;

	for (i = 0; i < numlocks; i++) {
		if (lckarray[i] == NULL)
			break;
	}
	if (i >= MAXLOCKS) {
		DEBUG (4, "TOO MANY LOCKS - EXITING", "");
		clrlock(NULL);
		exit(-1);
	}
	if (i >= numlocks)
		i = numlocks++;
	p = (char *) calloc((unsigned) strlen(name) + 1, sizeof (char));
	if (p == NULL) {
		DEBUG (4, "CAN NOT ALLOCATE FOR %s\n", name);
		clrlock(NULL);
		exit (-1);
	}
	(void) strcpy(p, name);
	lckarray[i] = p;
	return(0);
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
static int
mklock(pid,tempfile,name)
#ifdef	ASCIILOCKS
char *pid;
#else
int pid;
#endif 
char *tempfile, *name;
{	
	register int fd;
	char	cb[100];

	fd=creat(tempfile, (mode_t)0444);
	if(fd < 0){
		(void) sprintf(cb, "%s %s %d",tempfile, name, errno);
		DEBUG (4, "ULOCKC %s\n", cb);
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
	/* (void) chown(tempfile, UUCPUID, UUCPGID); */
	(void) close(fd);
	if(link(tempfile,name)<0){
		DEBUG(4, "errno: %d ", errno);
		DEBUG(4, "link(%s, ", tempfile);
		DEBUG(4, "%s)\n", name);
		if(unlink(tempfile)< 0){
			(void) sprintf(cb, "ULK err %s %d", tempfile,  errno);
			DEBUG (4, "ULOCKLNK %s\n", cb);
		}
		return(-1);
	}
	if(unlink(tempfile)<0){
		(void) sprintf(cb, "%s %d",tempfile,errno);
		DEBUG (4, "ULOCKF %s\n", cb);
	}
	return(0);
}


/*
 * remove a lock file
 * return:
 *  none
 */
ttyunlock(s)
char *s;
{
	char ln[MAXNAMESIZE];

	(void) sprintf(ln, "%s.%s", LOCKPRE, s);
	BASENAME(ln, '/')[MAXBASENAME] = '\0';
	clrlock(ln);
	return;
}


/*
 * remove lock file associated with name
 * return:
 *	none
 */
clrlock(name)
register char *name;
{
	register int i;

	for (i = 0; i < numlocks; i++) {
		if (lckarray[i] == NULL)
			continue;
		if (name == NULL || EQUALS(name, lckarray[i])) {
			(void) unlink(lckarray[i]);
			(void) free(lckarray[i]);
			lckarray[i] = NULL;
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
ttytouchlock()
{
	register int i;

	struct {
		time_t actime;
		time_t modtime;
	} ut;

	ut.actime = time(&ut.modtime);
	for (i = 0; i < numlocks; i++) {
		if (lckarray[i] == NULL)
			continue;
		utime(lckarray[i], &ut);
	}
	return;
}

/*
 * Wait for device lock to go away
 *
 * return:
 *	none
 */
ttywait(device)
char *device;
{
	    for (;;) {	/* busy wait for LCK..line to go away */
		sleep(60);
		if (ttylocked(device) == 0) /* LCK..line gone */
		    break;
	    }
}
