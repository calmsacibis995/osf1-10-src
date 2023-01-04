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
static char	*sccsid = "@(#)$RCSfile: umount.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/07/15 15:02:19 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * FUNCTIONS: umount
 *
 * DESCRIPTION:
 *	System V compatible umount.
 */

#include <sys/syscall.h>
#include <sys/mount.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/habitat.h>

#pragma weak umount = __umount
int
__umount(char *spec)
{
struct	statfs	*mntbuf;
	int	mntsize;
	int	type;
	char	*dirname;
	int	i;

	/*
	 * SVVS fix: 'argument has a component that is not a directory'
	 * expects ENOTDIR, but gets EINVAL from getmntinfo/strcmp()
	 * below since it can't be mounted if it doesn't exist.
	 * Otherwise the unmount syscall would do the correct thing.
	 */
	{	struct stat sb;
		if (stat(spec,&sb) == -1 &&
				(_Geterrno() == ENOTDIR || _Geterrno() == ENOENT ||
				 _Geterrno() == ENAMETOOLONG || _Geterrno() == ELOOP))
			return(-1);
	}
	if ((mntsize = getmntinfo(&mntbuf, MNT_NOWAIT)) == 0) {
		_Seterrno(ENXIO);
		return(-1);
	}

	for (i = 0; i < mntsize; i++) {
		if (!strcmp(mntbuf[i].f_mntfromname, spec)) {
			type = mntbuf[i].f_type;
			dirname = mntbuf[i].f_mntonname;
			break;
		}
	}
	if (i == mntsize) {
		_Seterrno(EINVAL);
		return(-1);
	}

	return(syscall(SYS_unmount, dirname, 0));
}
