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
static char	*sccsid = "@(#)$RCSfile: ustat.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/07 23:50:13 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif

/*
 * ustat (from SVID) as a library routine in OSF/1.
 * Algorithm:
 *	Collect statfs information on all mounted filesystems.
 *	Find the one corresponding to the device number passed in, and
 *	if there's a match, fill in the buffer supplied.
 * Return values:
 *	 0 upon success
 *	-1 upon failure (errno set to EINVAL)
 * Deficiencies:
 *	Could put information in f_fname and f_fpack fields.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ustat = __ustat
#endif
#include <sys/param.h>
#include <sys/mount.h>
#include <ustat.h>
#include <errno.h>

ustat(dev, buf)
	dev_t dev;
	struct ustat *buf;
{
	int mntsize, i, ret = -1;
	struct statfs *mntbuf;
#ifdef _THREAD_SAFE
	int bufsize;

	mntsize = 0; bufsize = 0;
	getmntinfo_r(&mntbuf, MNT_NOWAIT, &mntsize, &bufsize);
#else
	mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
#endif
	for (i = 0; i < mntsize; i++) {
		register struct statfs *stp = &mntbuf[i];
		if (stp->f_fsid.val[0] == dev) {
			buf->f_tfree = stp->f_bavail;
			buf->f_tinode = stp->f_ffree;
			/*
			 * XXX
			 * Since the information in the f_fname and
			 * f_fpack are optional, we'll leave them
			 * blank.  We could put something useful in
			 * the f_fname field, but...
			 */
			buf->f_fname[0] = '\0';
			buf->f_fpack[0] = '\0';
			ret = 0;
			break;
		}
	}
#ifdef _THREAD_SAFE
	if (mntbuf) free(mntbuf); /* ...malloc'ed by getmntinfo_r() */
#endif
	if (ret)
		_Seterrno(EINVAL);
	return(ret);
}
