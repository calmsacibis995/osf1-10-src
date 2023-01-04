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
static char	*sccsid = "@(#)$RCSfile: lockit.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:42:35 $";
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

/*
 * COMPONENT_NAME: (LIBPW) Programmers Workbench Library
 *
 * FUNCTIONS: lockit, mylock, unlockit
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
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
 */

# include	"sys/types.h"
# include       "fcntl.h"
# include	"macros.h"
# include	"errno.h"
# include	<time.h>

/*
 * RETURN VALUE DESCRIPTIONS:
 *		0 if successful
 *		-1 on ERROR
 */
/*
	Process semaphore.
	Try repeatedly (`count' times) to create `lockfile' mode 444.
	Sleep 10 seconds between tries.
	If `tempfile' is successfully created, write the process ID
	`pid' in `tempfile' (in binary), move `tempfile' to `lockfile',
	and return 0.
	If `lockfile' is unreadable or does not contain the process ID
	of any existing process,
	`lockfile' is removed and it tries again to make `lockfile'.
	If the create fails for any unrecoverable reason, return xmsg().
	After `count' tries, return -1.
 
	Unlockit will return 0 if the named lock exists, contains
	the given pid, and is successfully removed; -1 otherwise.
*/

#ifndef	NLS
#define NLsprintf sprintf
#endif


lockit(lockfile,count,pid)
register char *lockfile;
register int count;
{
	register int fd;
	register int ret;
	long omtime;
	int opid[2];
	char tempfile[512];
	char dir_name[512];
	extern char *dname();

	copy(lockfile,dir_name);
	NLsprintf(tempfile,"%s/%u",dname(dir_name),pid);

	for (count *= 2; --count >= 0; sleep(5)) {
again:
		/* try to create the tempfile */
		if ((fd = open(tempfile,O_CREAT|O_EXCL|O_WRONLY, 0444)) < 0) {
			if (errno == EEXIST) {
				/* tempfile already exists; get rid of it */
				if (unlink(tempfile) == 0 || errno == ENOENT)
					goto again;
			}
			else if (errno == ENFILE) {
				/* no file structures; wait a bit */
				continue;
			}
			/* some problem */
			  break;
		}
		/* write our pid into it, guarantee it is readable even if
		   umask is set, and link it to the lockfile */
		if (write(fd,(char *)&pid,sizeof(pid)) == sizeof(pid) &&
		    chmod(tempfile, 0444) == 0 &&
		    link(tempfile,lockfile) == 0)
			ret = -1;
		else
			ret = errno;
		close(fd);
		unlink(tempfile);
		if ((errno = ret) < 0)
			return(0);
		if (errno != EEXIST)
			/* some problem */
			break;

		if (stat(lockfile,&Statbuf) != 0 ||
		    (fd = open(lockfile,O_RDONLY)) < 0) {
			if (errno == ENOENT)
				/* it went away; try again */
				goto again;
			if (errno != EACCES)
				/* some problem */
				break;
			/* unreadable; get rid of it */
		}
		else {
			if (time((long *)0) - Statbuf.st_atime < 10)
				/* someone else read it recently;
				 * they may be trying to delete it;
				 * wait a bit to avoid race
				 */
				continue;
			ret = read(fd,(char *)opid,sizeof(opid));
			close(fd);
			if (ret == sizeof(pid) &&
			    (kill(opid[0],0) != -1 || errno != ESRCH))
				/* someone else is using it; wait a bit */
				continue;
		}
		/* remove the lockfile if it hasn't been written since
		 * we checked it
		 */
		omtime = Statbuf.st_mtime;
		if (stat(lockfile,&Statbuf) == 0 &&
		    omtime == Statbuf.st_mtime)
			if (unlink(lockfile) == 0)
				goto again;
	}
	if (count >= 0)
		return(xmsg(lockfile,"lockit"));
	else
		return(-1);
}


unlockit(lockfile,pid)
register char *lockfile;
unsigned pid;
{
	register int fd, n;
	unsigned opid;

	if ((fd = open(lockfile,0)) < 0)
		return(-1);
	n = read(fd,(char *)&opid,sizeof(opid));
	close(fd);
	if (n == sizeof(opid) && opid == pid)
		return(unlink(lockfile));
	else
		return(-1);
}


mylock(lockfile,pid)
register char *lockfile;
unsigned pid;
{
	register int fd, n;
	unsigned opid;

	if ((fd = open(lockfile,0)) < 0)
		return(0);
	n = read(fd,(char *)&opid,sizeof(opid));
	close(fd);
	if (n == sizeof(opid) && opid == pid)
		return(1);
	else
		return(0);
}
