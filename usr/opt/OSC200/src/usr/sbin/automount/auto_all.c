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
#ifdef ultrix
static char     *sccsid = "%W%  ULTRIX  %G%";
#else /* ultrix */
static char     *sccsid = "@(#)$RCSfile: auto_all.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1993/10/19 18:08:55 $";
#endif /* ultrix */
#endif /* lint */

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <netdb.h>
#include "nfs_prot.h"
#ifdef ultrix				/* JFS */
typedef quad fsid_t;                    /* file system id type */
#include "auto_ultrix.h"
#endif /* ultrix */
#define NFSCLIENT
#include "automount.h"
#include <rpcsvc/mount.h>
#include "mntent.h"

char *optlist[] = {
	MNTOPT_RO,	MNTOPT_RW,	MNTOPT_GRPID,	MNTOPT_SOFT,
	MNTOPT_HARD,	MNTOPT_NOSUID,	MNTOPT_INT,	MNTOPT_SECURE,
	MNTOPT_NOAC,	MNTOPT_NOCTO,	MNTOPT_PORT,	MNTOPT_RETRANS,
	MNTOPT_RSIZE,	MNTOPT_WSIZE,	MNTOPT_TIMEO,	MNTOPT_ACTIMEO,
	MNTOPT_ACREGMIN,MNTOPT_ACREGMAX,MNTOPT_ACDIRMIN,MNTOPT_ACDIRMAX,
	MNTOPT_QUOTA,	MNTOPT_NOQUOTA,
#ifdef  MNTOPT_POSIX
	MNTOPT_POSIX,
#endif
	"suid",
	NULL
};

/*
 * Routine to verify that a list of options is legal.  If all the options
 * in opts match a a keyword in optlist, then this returns NULL.  If a mismatch
 * occurs, then a pointer to the first unrecognized option is returned.
 */
char *
opt_check(opts)
	char *opts;
{
	static char buf [256];
	register char *p, *pe, *pb;
	register char **q;

	if (strlen(opts) > 254)
		return (NULL);
	(void) strcpy(buf, opts);
	pb = buf;

	while (p = strtok(pb, ",")) {
		pb = NULL;
		if (pe = strchr(p, '='))
			*pe = '\0';
		for (q = optlist ; *q ; q++) {
			if (strcmp(p, *q) == 0)
				break;
		}
		if (*q == NULL)
			return (p);
	}
	return (NULL);
}

/*
 * Routine to unmount a map.  We ping the NFS server before the umount
 * system calls because on OSF/1 systems umount(2) can hang in a lock wait
 * if some other process is stuck in a lookup due to a down server.
 * If the server doesn't respond, we treat it as a busy file system
 * until it comes back up.
 */
do_unmount(fsys)
	struct filsys *fsys;
{
	struct queue tmpq;
	struct filsys *fs, *nextfs;
	nfsstat remount();
	extern int trace;
	enum clnt_stat pingmount();

	tmpq.q_head = tmpq.q_tail = NULL;
	for (fs = HEAD(struct filsys, fs_q); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		if (fs->fs_rootfs == fsys) {
			REMQUE(fs_q, fs);
			INSQUE(tmpq, fs);
		}
	}
	/* walk backwards trying to unmount */
	for (fs = TAIL(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = PREV(struct filsys, fs);
		if (fs->fs_unmounted)
			continue;
		if (pingmount(fs->fs_addr.sin_addr) != RPC_SUCCESS)
			goto inuse;
		if (trace > 1)
			fprintf(stderr, "unmount %s\n", fs->fs_mntpnt);
#ifdef ultrix
		if (!pathok(tmpq, fs) || umount(fs->fs_mountdev) < 0)
#else
		if (!pathok(tmpq, fs) || umount(fs->fs_mntpnt, MNT_NOFORCE) < 0)
#endif /* ultrix */
		{
			if (trace > 1)
				perror(fs->fs_mntpnt);
			goto inuse;
		}
		fs->fs_unmounted = 1;
		if (trace > 1)
			fprintf(stderr, "unmount %s: OK\n", fs->fs_mntpnt);
	}
	/* all ok - walk backwards removing directories */

	/* clean_mtab(0, HEAD(struct filsys, tmpq)); */
	for (fs = TAIL(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = PREV(struct filsys, fs);
		nfsunmount(fs);
		safe_rmdir(fs->fs_mntpnt);
		REMQUE(tmpq, fs);
		INSQUE(fs_q, fs);
		free_filsys(fs);
	}
	/* success */
	return (1);

inuse:
	/* remount previous unmounted ones */
	for (fs = HEAD(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		if (fs->fs_unmounted && remount(fs) == NFS_OK)
			fs->fs_unmounted = 0;
	}

	/* put things back on the correct list */
	for (fs = HEAD(struct filsys, tmpq); fs; fs = nextfs) {
		nextfs = NEXT(struct filsys, fs);
		REMQUE(tmpq, fs);
		INSQUE(fs_q, fs);
	}

	return (0);
}

/*
 * Check a path prior to using it.  This avoids hangups in
 * the unmount system call from dead mount points in the
 * path to the mount point we're trying to unmount.
 * If all the mount points ping OK then return 1.
 */
pathok(tmpq, tfs)
	struct queue tmpq;
	struct filsys *tfs;
{
	struct filsys *fs, *nextfs;
	extern dev_t tmpdev;
	extern int trace;
	enum clnt_stat pingmount();


	while (tfs->fs_mntpntdev != tmpdev && tfs->fs_rootfs != tfs) {
		for (fs = HEAD(struct filsys, tmpq); fs; fs = nextfs) {
			nextfs = NEXT(struct filsys, fs);
			if (tfs->fs_mntpntdev == fs->fs_mountdev)
				break;
		}
		if (fs == NULL) {
			syslog(LOG_ERR,
				"pathok: couldn't find devid %04x(%04x) for %s",
				tfs->fs_mntpntdev & 0xFFFF,
				tmpdev & 0xFFFF,
				tfs->fs_mntpnt);
			return (1);
		}
		if (trace > 1)
			fprintf(stderr, "pathok: %s\n", fs->fs_mntpnt);
		if (pingmount(fs->fs_addr.sin_addr) != RPC_SUCCESS) {
			if (trace > 1)
				fprintf(stderr, "pathok: %s is dead\n",
					fs->fs_mntpnt);
			return (0);
		}
		if (trace > 1)
			fprintf(stderr, "pathok: %s is OK\n",
				fs->fs_mntpnt);
		tfs = fs;
	}
	return (1);
}

freeex(ex)
	struct exports *ex;
{
	struct groups *groups, *tmpgroups;
	struct exports *tmpex;

	while (ex) {
		FREE(ex->ex_name);
		groups = ex->ex_groups;
		while (groups) {
			tmpgroups = groups->g_next;
			free((char *)groups);
			groups = tmpgroups;
		}
		tmpex = ex->ex_next;
		free((char *)ex);
		ex = tmpex;
	}
}

mkdir_r(dir)
	char *dir;
{
	int err;
	char *slash;
	char *rindex();

	if (mkdir(dir, 0555) == 0 || errno == EEXIST)
		return (0);
	if (errno != ENOENT)
		return (-1);
	slash = rindex(dir, '/');
	if (slash == NULL)
		return (-1);
	*slash = '\0';
	err = mkdir_r(dir);
	*slash++ = '/';
	if (err || !*slash)
		return (err);
	return mkdir(dir, 0555);
}

safe_rmdir(dir)
	char *dir;
{
	extern dev_t tmpdev;
	struct stat stbuf;

	if (stat(dir, &stbuf))
		return;
	if (stbuf.st_dev == tmpdev)
		(void) rmdir(dir);
}
