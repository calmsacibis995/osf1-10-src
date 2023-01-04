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
static char *rcsid = "@(#)$RCSfile: ul.c,v $ $Revision: 1.2.2.6 $ (DEC) $Date: 1992/12/18 15:31:02 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)ul.c	9.2	(ULTRIX/OSF)	10/24/91";
#endif 
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from ul.c	2.1	(ULTRIX/OSF)	12/3/90";
 * Note: this should go in dec/machine/common/
 */

#include <sys/user.h>
#include <sys/mount.h>
#include <sys/vnode.h>

smsys(p, args, retval) /* 4 */			/* 180 = shared memory */
	struct proc *p;
	void *args;
	long *retval;
{
	uprintf("smsys:\n");
}

lockf(p, args, retval) /* 0 */			/* 182 = lockf (future) */
	struct proc *p;
	void *args;
	long *retval;
{
	uprintf("lockf:\n");
}

/*
 *	getmnt() syscall mode argument values
 */
#define NOSTAT_MANY	1
#define STAT_MANY	2
#define STAT_ONE	3
#define NOSTAT_ONE	4
#define STAT_FD		5
#define NOSTAT_FD	6

static char *tbl[] = {
"Invalid Argument",
"NOSTAT_MANY",
"STAT_MANY",
"STAT_ONE",
"NOSTAT_ONE",
"STAT_FD",
"NOSTAT_FD",
};

ultrix_umount()
{
	/* takes a device number. Map it to a path and call unmount */
}


/*
 * NEED to emulate getmnt on Ultrix.
 * Since I am at it, why not fixup unmount as well ?
 * Ok, I'll doit..
 */
struct fs_data_req {    /* required part for all file systems */
          u_int     flags;    /* how mounted */
          u_int     mtsize;   /* max transfer size in bytes */
          u_int     otsize;   /* optimal transfer size in bytes */
          u_int     bsize;    /* fs block size in bytes for vm code */
          u_int     fstype;   /* see ../h/fs_types.h  */
          u_int     gtot;     /* total number of gnodes */
          u_int     gfree;    /* # of free gnodes */
          u_int     btot;     /* total number of 1K blocks */
          u_int     bfree;    /* # of free 1K blocks */
          u_int     bfreen;   /* user consumable 1K blocks */
          u_int     pgthresh; /* min size in bytes before paging*/
          int  uid;      /* uid that mounted me */
          short     dev;      /* Ultrix dev_t is short. major/minor of fs */
          short     pad;      /* Ultrix dev_t is short. alignment: */
          char devname[MAXPATHLEN + 4];  /* name of dev */
          char path[MAXPATHLEN + 4];     /* name of mount point */
};

struct fs_data {
          struct    fs_data_req    fd_req;   /* required data */
          u_int     fd_spare[113];      /* spare */
};   /* 2560 bytes */

getmnt(p, args, retval) /* 0 */			/* 182 = lockf (future) */
	struct proc *p;
	void *args;
	long *retval;
{
    struct args {
	int	*start;
	struct	fs_data *buffer;
	u_long	nbytes,
		mode;
	char	*path;
    } *uap = (struct args *) args;
    register struct mount *mp;
    register struct nameidata *ndp = &u.u_nd;
    register struct statfs *sp;
    struct fs_data *fake_fs;
    int error = 0, flag, index;
    int count, maxcount;

    /* Note: umount needs names (both), fd_dev and fd_fstype */

    switch (uap->mode) {
    case STAT_ONE:
    case NOSTAT_ONE:	/* single request */
		ndp->ni_nameiop = LOOKUP | FOLLOW;
		ndp->ni_segflg = UIO_USERSPACE;
		ndp->ni_dirp = uap->path;
		if (error = namei(ndp))
			return (error);
	
		mp = ndp->ni_vp->v_mount;
		sp = &mp->m_stat;
		flag = ndp->ni_vp->v_flag;
		vrele(ndp->ni_vp);

		/* Determine if this is really the mount point */
		if ((flag & VROOT) == 0) {
			return (EINVAL);
		}

		if (uap->mode == STAT_ONE) {
			VFS_STATFS(mp, error);
			if (error)
				return (error);
		}
		sp->f_flags = mp->m_flag & M_VISFLAGMASK;

		/* really root. Cobble up the data and return it */
		if ((fake_fs = (struct fs_data *)
				kalloc(sizeof(struct fs_data))) == NULL)
			return (ENOMEM);

		stat2fs_data(&mp->m_stat, fake_fs);

		error = copyout(fake_fs, uap->buffer, sizeof(struct fs_data));
		
		kfree(fake_fs, sizeof(struct fs_data));

		if (!error) 
			*retval = 1;
		break;
    case STAT_MANY:
    case NOSTAT_MANY:
		/* multiple requests */
		if (uap->nbytes < sizeof(struct fs_data)) break;

		/* index: number of filesystems to skip */
		if ((error = copyin(uap->start, &index, sizeof(int)))
			|| (index < 0)) break;

		if ((fake_fs = (struct fs_data *)
				kalloc(sizeof(struct fs_data))) == NULL)
			return (ENOMEM);

		count = 0;
		/* maxcount: number of struct fs_data's to fetch */
		maxcount = uap->nbytes / sizeof(struct fs_data);
		mp = rootfs;
		do {
			if (count < index) continue;

			sp = &mp->m_stat;

			if (uap->mode == STAT_MANY) {
				VFS_STATFS(mp, error);
				if (error)
					continue;
			}
			sp->f_flags = mp->m_flag & M_VISFLAGMASK;

			stat2fs_data(sp, fake_fs);

			if (error = copyout(fake_fs, uap->buffer,
					sizeof(struct fs_data)))
						break;
			uap->buffer++;
		} while ( (++count < (maxcount + index))
			&& ((mp = mp->m_prev) != rootfs) );

		if (!error)
			error = copyout(&count, uap->start, sizeof(int));

		if (!error)
			*retval = count - index ;

		kfree(fake_fs, sizeof(struct fs_data));
		break;
    default:	error = EINVAL;
    }
    return (error);
}

#define ULTRIX_M_RONLY         0x0001
#define ULTRIX_M_MOD           0x0002
#define ULTRIX_M_QUOTA         0x0004
#define ULTRIX_M_LOCAL         0x0008
#define ULTRIX_M_NOEXEC        0x0010
#define ULTRIX_M_NOSUID        0x0020
#define ULTRIX_M_NODEV         0x0040
#define ULTRIX_M_FORCE         0x0080
#define ULTRIX_M_SYNC          0x0100
#define ULTRIX_M_DONE          0x0200
#define ULTRIX_M_NOCACHE       0x0400
#define ULTRIX_M_EXPORTED      0x0800          /* export flag */
#define ULTRIX_M_NOFH          0x1000          /* no fhandle flag */
#define ULTRIX_M_EXRONLY       0x2000          /* export read-only */

stat2fs_data(sp, fs)
struct statfs *sp;
struct fs_data *fs;
{
int type, flags;

	bzero(fs, sizeof(struct fs_data));

	switch (sp->f_type) {
		case MOUNT_UFS: type = 1; break;
		case MOUNT_NFS: type = 5; break;
		default:	type = 0; break;
	}
	flags = 0;
	if (sp->f_flags&M_RDONLY)	flags |= ULTRIX_M_RONLY;
	if (sp->f_flags&M_SYNCHRONOUS)	flags |= ULTRIX_M_SYNC;
	if (sp->f_flags&M_NOEXEC)	flags |= ULTRIX_M_NOEXEC;
	if (sp->f_flags&M_NOSUID)	flags |= ULTRIX_M_NOSUID;
	if (sp->f_flags&M_NODEV)	flags |= ULTRIX_M_NODEV;
	if (sp->f_flags&M_EXPORTED)	flags |= ULTRIX_M_EXPORTED;
	if (sp->f_flags&M_EXRDONLY)	flags |= ULTRIX_M_EXRONLY;

	fs->fd_req.flags = flags;
	fs->fd_req.mtsize = sp->f_bsize;
	fs->fd_req.otsize = sp->f_bsize;
	fs->fd_req.bsize = sp->f_bsize;
	fs->fd_req.fstype = type;
	fs->fd_req.gtot = sp->f_files;
	fs->fd_req.gfree = sp->f_ffree;
	fs->fd_req.btot = sp->f_blocks;
	fs->fd_req.bfree = sp->f_bfree;
	fs->fd_req.bfreen = sp->f_bavail;
	fs->fd_req.pgthresh = sp->f_bsize;
	fs->fd_req.uid = 0;
	fs->fd_req.dev = sp->f_fsid.val[0];
	bcopy(sp->f_mntonname, fs->fd_req.path, MNAMELEN);
	bzero(fs->fd_req.path+MNAMELEN, (MAXPATHLEN+4)-MNAMELEN);
	bcopy(sp->f_mntfromname, fs->fd_req.devname, MNAMELEN);
	bzero(fs->fd_req.devname+MNAMELEN, (MAXPATHLEN+4)-MNAMELEN);

	return;
}

gets(cp)
	char *cp;
{
	register char *lp;
	register c;

	lp = cp;
	for (;;) {
		c = getchar() & 0177;
		switch (c) {
		case '\n':
		case '\r':
			*lp++ = '\0';
			return;
		case '\b':
		case '#':
		case '\177':
			lp--;
			if (lp < cp)
				lp = cp;
			continue;
		case '@':
		case 'u'&037:
			lp = cp;
#ifdef SABLE
			cnputc_flush('\n');
#else
			cnputc('\n');
#endif
			continue;
		default:
			*lp++ = c;
		}
	}
}

getchar()
{
	register c;

	c = cngetc();
	if (c == '\r')
		c = '\n';
	if (c != -1)
#ifdef SABLE
		cnputc_flush(c);
#else
		cnputc(c);
#endif
	return (c);
}
