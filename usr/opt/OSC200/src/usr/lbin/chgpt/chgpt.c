
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
 * @(#)$RCSfile: chgpt.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 16:21:55 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/*
 * chgpt - exec'ed by grantpt()
 */

#include <sys/secdefines.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <grp.h>


extern char *ptsname();
extern char *strrchr();


main(ac,av)
	int ac;
	char **av;
{
	char *slave_name;

	if (ac != 1)
		exit(1);

	if ((slave_name = ptsname(0)) == NULL)
		exit(1);


	if (get_pty(slave_name, 0) < 0)
		exit(1);

	exit(0);
}

/*
 * get_pty: Create the pty device named by sname, if it doesn't exist, 
 * and set the owner and privileges.   
 */
get_pty(sname, master)
char *sname;
int master;
{

	struct stat statbuf;
	dev_t devno;
	char *minor;
	struct group *gr;
	register ttygid;
	dev_t sdevno;
	if (stat(sname, &statbuf) < 0) {

		/* find minor number in "/dev/pts/number" */
		if (((minor = strrchr(sname, '/')) == NULL)
		    || (!isdigit(*++minor)))
			return (-1);


		/* get the slave pty device for the major number */
		sdevno = ioctl(master, ISPTS, 0);

		devno = makedev(major(sdevno), atoi(minor));

		/*
		 * To avoid letting anyone have premature access to this
		 * pty, create it with mode 0. Since we're either setuid
		 * or privileged, that shouldn't be a problem for us,
		 * and fixes potential races with opens between the
		 * mknod and the time we're done changing attributes.
		 */

		if (mknod(sname, S_IFCHR | 0000, devno) < 0) 
			return (-1);
	}

	if ((gr = getgrnam("tty")) != NULL)
		ttygid = gr->gr_gid;
	else
		ttygid = -1;

	if (chown(sname, getuid(), ttygid) < 0) {
		(void) chmod(sname, statbuf.st_mode & ~S_IFMT);
		return (-1);
	}

	/*
	 * Now it's safe to let the world have access to it
	 */

	if (chmod(sname, S_IRUSR | S_IWUSR | S_IWGRP) < 0)
		return (-1);

	return (0);
}

