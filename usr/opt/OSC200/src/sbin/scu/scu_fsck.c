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
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	scu_fsck.c
 * Author:	Robin T. Miller
 * Date:	September 25, 1991
 *
 * Description:
 *	This file contains various file system utility functions.
 */

#include <sys/param.h>
#ifndef OSF
#include <sys/inode.h>
#endif
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <io/common/devio.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <fstab.h>
#include <strings.h>
#include "scu.h"
#include "scu_device.h"

/*
 * External Declarations:
 */
extern int getuid(), getgroups(), stat();
extern void Fprintf(), Perror();

/*
 * Forward References:
 */
static char *rawname(), *unrawname();

/*
 * Local Declarations:
 */
#ifdef OSF
#define MSIZE (long)(NMOUNT * sizeof(struct statfs))
#define GFMT S_IFMT
struct statfs *fs_data;
#else
#define MSIZE (long)(NMOUNT * sizeof(struct fs_data))
struct fs_data *fs_data;
#endif
int nmountedfs;

/************************************************************************
 *									*
 * IS_Mounted() - Determine if Disk has Mounted File Systems.		*
 *									*
 * Description:								*
 *	This function checks the disk device for any mounted file	*
 * systems.  This code was taken from 'fsck' and works for UFS fs.	*
 *									*
 * Inputs:	scu = The device unit structure.			*
 *									*
 * Return Value:							*
 *		Returns TRUE if device has mounted file sytems.		*
 *		   "    FALSE if device does not have mounted fs.	*
 *		   "    FAILURE if errors were detected.		*
 *									*
 ************************************************************************/
int
IS_Mounted (scu)
struct scu_device *scu;
{
	char *device;
	int start = 0;
	int status;

	device = scu->scu_device_entry;
	if ( BypassFlag == TRUE ) return (FALSE);
	if ( ( EQ (DEV_UAGT, scu->scu_device_name) ) ||
	     ( EQ (DEV_UNKNOWN, scu->scu_device_name) ) ||
	     ( scu->scu_unit == NEXUS_NOT_SET ) ) {
	    Fprintf (
	"Mounted file system check cannot be performed using '%s' (%s).",
					device, scu->scu_device_name);
	    return (FAILURE);
	}
#ifdef OSF
	fs_data = (struct statfs *) malloc (MSIZE);
#else
	fs_data = (struct fs_data *) malloc (MSIZE);
#endif
	if (fs_data == NULL) {
	    Perror ("Cannot allocate %d bytes for file system data.", MSIZE);
	    return (FAILURE);
	}
#ifdef OSF
	if ((nmountedfs = getfsstat(fs_data, MSIZE, MNT_NOWAIT)) < 0) {
	    Perror ("Get all mounted file systems 'getfsstat()' failed");
	    return (FAILURE);
	}
#else
	if ((nmountedfs = getmountent (&start, fs_data, NMOUNT)) < 0) {
	    Perror ("Get mount entrys 'getmountent()' failed");
	    return (FAILURE);
	}
#endif
	status = CheckFileSystem (device);
	free ((char *) fs_data);
	return (status);
}

/************************************************************************
 *									*
 * CheckFileSystem() - Determine if Disk has Mounted File Systems.	*
 *									*
 * Description:								*
 *	This function checks the disk device for any mounted file	*
 * systems.  This code was taken from 'fsck' and works for UFS fs.	*
 *									*
 * Inputs:	filesys = The file system pointer.			*
 *									*
 * Return Value:							*
 *		Returns TRUE if device has mounted file sytems.		*
 *		   "    FALSE if device does not have mounted fs.	*
 *		   "    FAILURE if errors were detected.		*
 *									*
 ************************************************************************/
static int
CheckFileSystem (filesys)
char *filesys;
{
	struct stat sb, stat_raw;
#ifdef OSF
	register struct statfs *fs_dp = fs_data;
#else
	register struct fs_data *fs_dp = fs_data;
#endif
	char *raw;
	int gids[NGROUPS];		/* Groups user in.		*/
	int ngids = NGROUPS;		/* Number of groups user is in.	*/
	int *gidp = gids;
	int valid_user;			/* Valid for user access.	*/
	char block_device[MAXPATHLEN];	/* Array for block device name.	*/
	char *bdev = block_device;

	if (stat (filesys, &sb)) {
	    Perror ("Get statistics 'stat()' failed on '%s'\n", filesys);
	    return (FAILURE);
	}
	/*
	 * If we are not root, make sure device has write permission set.
	 */
	if (getuid()) {
	    valid_user = FALSE;
	    if (sb.st_mode & S_IWOTH) {
		valid_user = TRUE;
	    } else if (sb.st_mode & S_IWGRP) {
		ngids = getgroups (ngids, gidp);
		for (; gidp < &gids[ngids]; gidp++) {
		    if (sb.st_gid == *gidp) {
			valid_user = TRUE;
		    }
		}
	    }
	    if (valid_user == FALSE) {
		errno = EACCES;
		Perror ("No write permission on '%s'", filesys);
		return (FAILURE);
	    }
	}
	if ((sb.st_mode & GFMT) == S_IFBLK) {
	    raw = rawname (filesys);
	} else {
	    /*
	     * We must stat the corresponding block device here.  Further
	     * down in this routine, we check the mount table against the
	     * rdev field of the stat buffer (sb), to determine if the
	     * device is mounted.  If we don't stat the block device here,
	     * then this check will not work when the major numbers of the
	     * block and character devices do not match.
	     */
	    (void) bzero (bdev, MAXPATHLEN);
	    (void) strcpy (bdev, filesys);
	    bdev = unrawname (bdev);
	    if (stat (bdev, &sb)) {
		Perror ("Get statistics 'stat()' failed on '%s'\n", bdev);
		return (FAILURE);
	    }
	    raw = filesys;
	}

	if ((sb.st_mode & GFMT) != S_IFBLK) {
	    errno = EINVAL;
	    Perror ("'%s' is not a block device", bdev);
	    return (FAILURE);
	}

	if (stat (raw, &stat_raw) < 0) {
	    Perror ("Get statistics 'stat()' failed on '%s'\n", raw);
	    return (FAILURE);
        }

	if ((stat_raw.st_mode & GFMT) != S_IFCHR) {
	    errno = EINVAL;
	    Perror ("'%s' is not a character device.", raw);
	    return (FAILURE);
	}

#ifdef OSF
	for (; fs_dp < &fs_data[nmountedfs]; fs_dp++) {
	    if ( devices_match (bdev, fs_dp->f_mntfromname) ) {
		errno = EBUSY;
		Perror ("File system '%s' is mounted on '%s'",
				fs_dp->f_mntfromname, fs_dp->f_mntonname);
		return (TRUE);
	    }
	}
#else
	for (; fs_dp < &fs_data[nmountedfs]; fs_dp++) {
	    if ( (sb.st_rdev == fs_dp->fd_dev) ||
		 (devices_match (bdev, fs_dp->fd_devname)) ) {
		errno = EBUSY;
		Perror ("File system '%s' is mounted on '%s'",
				fs_dp->fd_path, fs_dp->fd_devname);
		return (TRUE);
	    }
	}
#endif
	return (FALSE);
}

static int
devices_match (bdev, fsdev)
char *bdev, *fsdev;
{
	register int blen;
	int status = FALSE;

	if ((blen = strlen(bdev)) == strlen(fsdev)) {
	    if (strncmp (bdev, fsdev, (blen - 1)) == 0) {
		status = TRUE;
	    }
	}
	return (status);
}

static char *
unrawname (cp)
char *cp;
{
	char *dp = rindex(cp, '/');
	struct stat stb;

	if (dp == 0) {
		return (cp);
	}
	if (stat (cp, &stb) < 0) {
		return (cp);
	}
	if ((stb.st_mode & S_IFMT) != S_IFCHR) {
		return (cp);
	}
	if (*(dp+1) != 'r') {
		return (cp);
	}
	(void) strcpy (dp+1, dp+2);
	return (cp);
}

static char *
rawname (cp)
char *cp;
{
	static char rawbuf[32];
	char *dp = rindex(cp, '/');

	if (dp == 0) {
		return (0);
	}
	*dp = 0;
	(void) strcpy (rawbuf, cp);
	*dp = '/';
	(void) strcat (rawbuf, "/r");
	(void) strcat (rawbuf, dp+1);
	return (rawbuf);
}
