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
static char	sccsid[] = "@(#)$RCSfile: cdfs_vfsops.c,v $ $Revision: 4.3.10.6 $ (DEC) $Date: 1993/09/07 16:42:33 $";
#endif 
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
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * cdfs_vfsops.c
 *
 *      Revision History:
 *
 * 17-Jun-91    Mitch Condylis
 *      Prevent non-root mounts in cdfs_mount.
 *
 */

#if	MACH
#include <quota.h>
#include <bufcache_stats.h>
#ifdef	i386
#include <cputypes.h>
#endif
#endif

#include <rt_preempt.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/biostats.h>
#include <sys/ucred.h>
#include <sys/file.h>
#include <sys/disklabel.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/mode.h>
#if	MACH
#include <kern/zalloc.h>
#include <kern/kalloc.h>
#else
#include <sys/malloc.h>
#endif
#include <cdfs/cdfs.h>
#include <cdfs/cdfsmount.h>
#include <cdfs/cdfsnode.h>
#include <sys/lock_types.h>
#if	MACH
#include <sys/user.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <kern/mfs.h>
#endif
#if	EXL
#include <machine/open.h>
#endif
#if 	SEC_BASE
#include <sys/security.h>
#endif
#include <sys/open.h>

#if	MACH
zone_t	cdfspvd_zone;
zone_t	cdfsmount_zone;
zone_t	cdfsreaddir_zone;
#endif

/*
 * cdfs vfs operations.
 */
int cdfs_mount();
int cdfs_start();
int cdfs_unmount();
int cdfs_root();
int cdfs_quotactl();
int cdfs_statfs();
int cdfs_sync();
int cdfs_fhtovp();
int cdfs_vptofh();
int cdfs_init();

struct vfsops cdfs_vfsops = {
	cdfs_mount,
	cdfs_start,
	cdfs_unmount,
	cdfs_root,
	cdfs_quotactl,
	cdfs_statfs,
	cdfs_sync,
	cdfs_fhtovp,
	cdfs_vptofh,
	cdfs_init
};

#ifdef CDFSDEBUG
int cdfs_isodebug = 0;			/* see cdfs.h for bit values
					   to put here */
#endif /* CDFSDEBUG */

/*
 * VFS Operations.
 *
 * mount system call
 *
 * Synchronization assumptions:
 *	-- Mount structure could be on global list (if M_UPDATE), so
 *	   we need to lock (sigh).
 *	-- Other attempted mounts on this directory are locked out by
 *	   mount().
 * We are responsible for detecting races on mounting the device passed in.
 * N.B.  Expect ndp to contain a pointer to the vnode to be covered in ni_vp.
 */

cdfs_mount(mp, path, data, ndp)
	register struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	struct vnode *devvp;
	struct cdfs_args args;
	struct cdfsmount *ump = VFSTOCDFS(mp);
	register struct fs *fs;
	u_int size;
	int error;

	/* Disable user mounts for cdfs */
        if (mp->m_uid)
                return (EPERM);

	CDDEBUG1(MNTDEBUG, printf("In cdfs_mount routine\n"));

	if (error = copyin(data, (caddr_t)&args, sizeof (struct cdfs_args)))
		return (error);
	/*
	 * Process export requests.
	 */
	MOUNT_LOCK(mp);
	if ((args.exflags & M_EXPORTED) || (mp->m_flag & M_EXPORTED)) {
		if (args.exflags & M_EXPORTED)
			mp->m_flag |= M_EXPORTED;
		else
			mp->m_flag &= ~M_EXPORTED;
		if (args.exflags & M_EXRDONLY)
			mp->m_flag |= M_EXRDONLY;
		else
			mp->m_flag &= ~M_EXRDONLY;
		mp->m_exroot = args.exroot;
	}
	mp->m_flag |= M_RDONLY;
	if ((mp->m_flag & M_UPDATE) == 0) {
#if	SER_COMPAT || RT_PREEMPT
		mp->m_funnel = FUNNEL_NULL;
#endif
		MOUNT_UNLOCK(mp);
		/*
		 * getmdev expects the vnode to be covered in ni_bypassvp
		 * to handle certain pathname combinations that could
		 * otherwise deadlock.  ni_bypassvp is an alias for
		 * ni_vp, so there's no extra work to be done here.
		 */
		if ((error = getmdev(&devvp, args.fspec, ndp)) != 0)
			return (error);
		CDDEBUG1(MNTDEBUG,
			 printf("cdfs_mount: calling mountcdfs, args = 0x%x\n",
			       args.flags));
		if ((error = mountcdfs(devvp, mp, &args, NULL)) != 0) {
			vrele(devvp);
			return(error);
		}
		fs = (VFSTOCDFS(mp))->um_fs;
	} else {
		MOUNT_UNLOCK(mp);
		return(0);
	}
	CDDEBUG1(MNTDEBUG, printf("cdfs_mount: cdfsmount returned\n"));
	/*
	 * fs is set correctly
	 */
	(void) copyinstr(path, (caddr_t)mp->m_stat.f_mntonname, MNAMELEN - 1, 
			 &size);
	(void) copyinstr(args.fspec, mp->m_stat.f_mntfromname, MNAMELEN - 1, 
		&size);
	bzero(mp->m_stat.f_mntfromname + size, MNAMELEN - size);
	CDDEBUG1(MNTDEBUG, printf("cdfs_mount: Calling cdfs_statfs\n"));
	(void) cdfs_statfs(mp);
	CDDEBUG1(MNTDEBUG, printf("cdfs_mount: Returning sucess\n"));
	return (0);
}

/*
 * Common code for mount and mountroot
 *
 * Synchronization assumptions:
 *	-- mp is NOT on global list (mount update doesn't get here), so
 *	   no mount structure locking required.
 *	-- Other attempted mounts, including forcible ones on this 
 *	   directory are locked out by mount().
 *	-- No lock needed on fs->*, since it's a new mount, except for
 *	   update case.
 */
mountcdfs(devvp, mp, mount_args, update)
	struct vnode *devvp;
	struct mount *mp;
	struct cdfs_args *mount_args;
	struct cdfsmount *update;
{
	int mount_flags = mount_args->flags;
	register struct cdfsmount *ump;
	struct cdnode *np;
	struct buf *bp = NULL;
	register struct fs *fs;
	struct vnode *vp;
	struct partinfo dpart;
	caddr_t base, space;
	int havepart = 0, blks;
	int error, i, size;
	int needclose = 0;
	int setmounted = 0;
	int ronly = (mp->m_flag & M_RDONLY) != 0;
	int loc, primary_loc, supplementary_loc;
	register struct iso_fs *iso_fs = 0;
	register struct hsg_fs *hsg_fs = 0;
	struct iso_dir *iso_tdir;
	struct hsg_dir *hsg_tdir;
	unsigned char iso_dir_bytes[ISO_MAXDIRENTLEN];
	union {
		unsigned char incoming[4];
		unsigned int outgoing;
	} convert_extent;

	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of an open block device.
	 * Allow mounting of an open character device.
	 * Flush out any old buffers remaining from previous use.
	 */
	vinvalbuf(devvp, 1);
	/* TODO:  validate locking, are there races on mount-update? */
	if (update) {
		mount_flags &= ~M_RRIP;	/* can't turn it on mid-stream */
		ump = update;
		goto updateskip;
	}
	if ((ump = (struct cdfsmount *)zalloc(cdfsmount_zone)) == 
	    (struct cdfsmount *) 0) {
	    CDDEBUG1(MNTDEBUG,
		     printf("cdfsmount: Couldn't zalloc cdfsmount_zone\n"));
		return (ENOMEM);
	}
	bzero((caddr_t)ump, sizeof *ump);
	mp->m_data = (qaddr_t)ump;

/* In SVR4, driver open and close routines expect to get the OTYP_MNT flag
 * if the open/close was done as the result of a mount/unmount.  While OSF/1 
 * can pass a flag parameter to device open/close routines, it is only 
 * supported in spec_open() and spec_close(). These are the functions invoked
 * via VOP_OPEN and VOP_CLOSE for device special files. Therefore, we need 
 * to inform spec_open()/spec_close() that we are doing a mount/unmount.
 */
	VOP_OPEN(&devvp, FREAD|OTYP_MNT, NOCRED, error);
	if (error) {
		ump->um_fs = NULL;
		CDDEBUG1(MNTDEBUG,
			 printf("cdfsmount: Driver open call failed\n"));
		goto out1;
	}
	needclose = 1;
	/*
	 * To determine if the device is already mounted or open, we call
	 * setmount(), which knows the magic.  We set the mounted flag
	 * at the same time to prevent races on mounting this device.
	 */
	CDDEBUG1(MNTDEBUG, printf("cdfsmount: Calling setmount\n"));
	if (setmount(devvp, SM_OPEN|SM_MOUNTED|SM_SETMOUNT)) {
		error = EBUSY;
		ump->um_fs = NULL;
		goto out;
	}
	setmounted = 1;
	CDDEBUG1(MNTDEBUG, printf("cdfsmount: Calling driver ioctl\n"));
	VOP_IOCTL(devvp, DIOCGPART, (caddr_t)&dpart, FREAD, NOCRED, i, &size);
	if (i != 0)
		size = DEV_BSIZE;
	else {
		havepart = 1;
		size = dpart.disklab->d_secsize;
	}
	primary_loc = supplementary_loc = 0;
	loc = PVD_BLOCK;
	for (;;) {
		/*
		 * Read Primary Volume Descriptor
		 */
		if (error = bread(devvp, loc, ISO_SECSIZE, NOCRED, &bp)) {
			ump->um_fs = NULL;
			CDDEBUG1(MNTDEBUG,
				 printf("cdfsmount: read failed at loc %d\n",
					loc));
			goto out;
		}

		iso_fs = (struct iso_fs *)bp->b_un.b_fs;
		/*
		 *	Check the magic number and see 
		 *	if we have a valid filesystem.
		 */
		if(!strncmp(iso_fs->iso_std_id,"CD001", 5)) {
			switch(iso_fs->iso_vol_desc_type) {
			      case TERMINATING_VOL_DESC:
				      brelse(bp);
				      bp = NULL;
				      hsg_fs = (struct hsg_fs *)NULL;
				      CDDEBUG1(MNTDEBUG,
					       printf("cdfsmount: ISO TERM\n"));
				      goto found;
			      case PRIMARY_VOL_DESC:
				      primary_loc = loc;
				      CDDEBUG1(MNTDEBUG,
					       printf("cdfsmount: ISO PVD\n"));
				      break;
			      case SUPPLEMENTARY_VOL_DESC:
				      if (!strncmp(iso_fs->iso_system_id, 
						   "DEC_ULTRIX", 10)) {
					      supplementary_loc = loc;
				      }
				      CDDEBUG1(MNTDEBUG,
					       printf("cdfsmount: ISO SUPP\n"));
			}
			loc += (ISO_SECSIZE / DEV_BSIZE);
			brelse(bp);
			bp = NULL;
			continue;
		}
		hsg_fs = (struct hsg_fs *)bp->b_un.b_fs;
		/*
		 * Check the magic number and see
		 * if formatted in HSG.
		 */
		if(!strncmp(hsg_fs->iso_std_id,"CDROM", 5)) {
			switch(hsg_fs->iso_vol_desc_type) {
			      case TERMINATING_VOL_DESC:
				      brelse(bp);
				      bp = NULL;
				      iso_fs = (struct iso_fs *)NULL;
				      CDDEBUG1(MNTDEBUG,
					       printf("cdfsmount: HSG TERM\n"));
				      goto found;
			      case PRIMARY_VOL_DESC:
				      primary_loc = loc;
				      CDDEBUG1(MNTDEBUG,
					       printf("cdfsmount: HSG PVD\n"));
				      break;
			      case SUPPLEMENTARY_VOL_DESC:
				      if (!strncmp(hsg_fs->iso_system_id, 
						   "DEC_ULTRIX", 10)) {
					      supplementary_loc = loc;
				      }
				      CDDEBUG1(MNTDEBUG,
					       printf("cdfsmount: HSG SUPP\n"));
			}
			loc += (ISO_SECSIZE / DEV_BSIZE);
			brelse(bp);
			bp = NULL;
			continue;
		}
		uprintf("cdfs_mount: Unknown descriptor type\n");
		brelse(bp);
		bp = NULL;
		ump->um_fs = NULL;
		goto out;
	}
	
found:
	CDDEBUG1(MNTDEBUG,
		 printf("cdfsmount: primary_loc = %d supp_loc = %d\n", 
			primary_loc, supplementary_loc));
	if (primary_loc == 0) {
		uprintf("cdfs_mount: Volume Descriptor Terminator found before Primary volume descriptor\n");
		ump->um_fs = NULL;
		goto out;
	}
	if (supplementary_loc == 0 || mount_flags & M_PRIMARY)
		loc = primary_loc;
	else {
		loc = supplementary_loc;

#if	SEC_BASE
		/*
		 * Must have mount privilege.
		 */
		if (!privileged(SEC_MOUNT, 0))
#else
		if (suser(u.u_cred, &u.u_acflag) != 0)
#endif 
			ump->um_flag |= M_NODEFPERM;
	}
	if (error = bread(devvp, loc, ISO_SECSIZE, NOCRED, &bp)) {
		ump->um_fs = NULL;
		CDDEBUG1(MNTDEBUG,
			 printf("cdfsmount: read failed at loc %d\n", loc));
		goto out;
	}
	if (iso_fs)
		iso_fs = (struct iso_fs *)bp->b_un.b_fs;
	else
		hsg_fs = (struct hsg_fs *)bp->b_un.b_fs;
	/*
 	 * pvd zone has memory of size, ISO_SECSIZE
 	 */
	if ((ump->um_fs = (struct fs *)zalloc(cdfspvd_zone)) == 
	    (struct fs *) 0) {
	    CDDEBUG1(MNTDEBUG,
		     printf("cdfsmount: Couldn't zalloc cdfspvd_zone\n"));
		error = ENOMEM;
		goto out;
	}
	fs = ump->um_fs;
	    CDDEBUG1(MNTDEBUG,
		     printf("cdfsmount: Copying PVD to fs_block\n"));
	bcopy((caddr_t)bp->b_un.b_addr, (caddr_t)&(fs->fs_block), ISO_SECSIZE);
	brelse(bp);
	bp = NULL;

	CDDEBUG1(MNTDEBUG, printf("cdfsmount: Initializing fs fields\n"));
	FS_LOCK_INIT(fs);
	fs->fs_ronly = ronly;
	fs->fs_ibsize = MAXBSIZE;
	fs->rrip_numdevs = 0;
	fs->rrip_devmap = 0;
	fs->rrip_susp_offset = 0;
	/*
	 * Since ISO9660 files do not have unique numbers on disk,
	 * we must make our own. We will use the disk address of
	 * the first byte of a file's directory entry.
	 */
	CDDEBUG1(MNTDEBUG, printf("cdfsmount: Initializing iso_rootino\n"));
	if (iso_fs) {

		fs->fs_format = ISO_9660; /* maybe RRIP?  see below */
		iso_fs = (struct iso_fs *) &fs->fs_block.isofs;
		iso_tdir = &iso_fs->iso_root_dir;
		CDFS_COPYINT(iso_tdir->dir_extent_lsb,
			     convert_extent.incoming);

		fs->iso_rootino = 
			(convert_extent.outgoing + iso_tdir->dir_xar) * 
			(int)ISOFS_LBS(fs);

		if (mount_flags & M_RRIP) {
		    /* get the root directory entry, look for RRIP
		       telltales */
		    error = cdfs_readisodir_int(fs,
						fs->iso_rootino,
						devvp,
						devvp->v_rdev,
						0,
						(struct iso_dir *)iso_dir_bytes);
		    if (error) {
			CDDEBUG1(RRIPDEBUG,
				 printf("cdfsmount: readisodir %d\n", error));
		    } else {
			struct cd_suf_sp *sufp;
			int namelen, left;
			/* look for mandatory SP SUF in SUA */

			CDDEBUG1(RRIPDEBUG, printf("cdfsmount: try SUSP\n"));
			iso_tdir = (struct iso_dir *) &iso_dir_bytes[0];
			namelen = iso_tdir->dir_namelen;
			if (!(namelen & 0x1))
			    namelen++;	/* round up if even. */
			/* leftover is total less fixed less name length: */
			left = iso_tdir->dir_len
			    - (sizeof(*iso_tdir) - 1)
				- namelen;
			if (left >= SUF_SP_LEN ) {
			    static struct cd_suf_sp cd_suf_sp_template =
			    { {SUF_SUSP_INDICATOR,SUF_SP_LEN,SUF_SP_VERS},
			      {SUF_SP_CHECK1, SUF_SP_CHECK2}};
			    sufp = (struct cd_suf_sp *)&iso_tdir->dir_name[namelen];
			    CDDEBUG1(RRIPDEBUG,
				     printf("cdfsmount: room for SUSP\n"));
			    if (!bcmp((char *)sufp,
				      (char *)&cd_suf_sp_template, SUF_SP_LEN)) {
				/* this disk adheres to SUSP.  Look for RRIP. */
				struct cd_suf_er *suf_er;
				struct buf *nbp;
				CDDEBUG1(RRIPDEBUG,
					 printf("cdfsmount: looks like SUSP\n"));
				fs->rrip_susp_offset = sufp->sp_len_skp;

				suf_er = (struct cd_suf_er *)
				    susp_search_dir(fs, devvp,
						    iso_tdir, 0, /* no skip on this one... */
						    TRUE,
						    SUF_EXTENSIONS_REF, &nbp, 0);
				if (suf_er &&
				    suf_er->er_len_id == RRIP_ER_IDENT_LEN &&
				    !bcmp(suf_er->er_data, RRIP_ER_IDENT,
					  RRIP_ER_IDENT_LEN)) {
				    /* got it! */
				    /* fs->fs_format = ISO_RRIP; */
				    CDDEBUG1(RRIPDEBUG,
					     printf("found RRIP identifier\n"));
				    fs->fs_format = ISO_RRIP;
				} 
#ifdef CDFSDEBUG
				else if (RRIPDEBUG)
				    printf("no RRIP identifier\n");
#endif /* CDFSDEBUG */
				if (nbp)
				    brelse(nbp);
			    }
			}
		    
		    }
		    if (fs->fs_format != ISO_RRIP) {
			/* asked for RRIP, but it's not on the disk. XXX ? */
			uprintf("cdfs_mount: no RRIP on disk, defaulting to ISO-9660\n");
			mount_flags &= ~M_RRIP;
			/* continue on, though */	
		    }
		}
	} else {
		fs->fs_format = ISO_HSG;
		hsg_fs = (struct hsg_fs *) &fs->fs_block.hsgfs;
		hsg_tdir = &hsg_fs->iso_root_dir;
		CDFS_COPYINT(hsg_tdir->dir_extent_lsb,
			     convert_extent.incoming);
		fs->iso_rootino = 
			(convert_extent.outgoing + hsg_tdir->dir_xar) * 
			(int)ISOFS_LBS(fs);
	}
	if (fs->iso_rootino % ISO_SECSIZE) {
		printf("cdfs_mount: root directory record does not begin on a logical sector boundary\n");
		error = EINVAL;
		goto out;
	}
	CDDEBUG1(MNTDEBUG,
		 printf("cdfsmount: iso_rootino = %d\n", fs->iso_rootino));

	mp->m_data = (qaddr_t)ump;
	mp->m_stat.f_fsid.val[0] = devvp->v_rdev;
	mp->m_stat.f_fsid.val[1] = MOUNT_CDFS;
	mp->m_flag |= M_LOCAL;
	ump->um_mountp = mp;
	ump->um_dev = devvp->v_rdev;
	ump->um_devvp = devvp;

    updateskip:
	CDDEBUG1(MNTDEBUG,
		 printf("cdfsmount: mount_flags = 0x%x\n", mount_flags));
	/*
	 *	M_NOVERSION is backward compatibility; it means
	 *	M_LOWERCASE|M_DROPVERS, no M_XCDR, and weird relaxed
	 *	name matching 
	 *
	 *	M_DROPVERS and M_LOWERCASE (cdmntsuppl -l -m):
	 *	   drop version, map to lowercase, drop null "." extension.
	 *	M_DROPVERS and !M_LOWERCASE (cdmntsuppl -m):
	 *	   drop version, keep uppercase, keep null "." extension.
	 *	!M_DROPVERS and M_LOWERCASE (cdmntsuppl -l):
	 *	   keep version suffix, map to lowercase, drop null "." extens.
	 *	!M_DROPVERS and !M_LOWERCASE (no mapping at all):
	 *	   keep version suffix, keep uppercase, keep null "." extens.
	 */
	if (mount_flags & M_NOVERSION)
	    /* backward compat */
	    mount_flags |= M_DROPVERS|M_LOWERCASE;

	ump->um_flag |= (mount_flags & (/*M_DEFPERM |*/
			M_NODEFPERM | M_DROPVERS | M_PRIMARY |
			M_LOWERCASE | M_RRIP));

	if (update)
	    cache_purgevfs(mp);		/* name mappings may have changed. */

	CDDEBUG1(MNTDEBUG, printf("cdfsmount: returning sucess\n"));
	return (0);
out:
	CDDEBUG1(MNTDEBUG, printf("cdfsmount: In out\n"));
	if (bp)
		brelse(bp);
	if (needclose) {
		if (setmounted)
			(void) setmount(devvp, SM_CLEARMOUNT);
		VOP_CLOSE(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED, i);
	}
	if (ump->um_fs) {
#if	MACH
		ZFREE(cdfspvd_zone, ump->um_fs);
#else
		free((caddr_t)ump->um_fs, M_SUPERBLK);
#endif
		ump->um_fs = NULL;
	}
out1:
	CDDEBUG1(MNTDEBUG, printf("cdfsmount: In out1\n"));
#if	MACH
	ZFREE(cdfsmount_zone, ump);
#else
	free((caddr_t)ump, M_MOUNT);
#endif
	if (!error)
		error = EINVAL;
	return (error);
}

/*
 * Make a filesystem operational.
 * Nothing to do at the moment.
 */
/* ARGSUSED */
cdfs_start(mp, flags)
	struct mount *mp;
	int flags;
{

	return (0);
}

/*
 * unmount system call
 *
 * Synchronization assumptions:
 *	-- other unmounts are locked out.
 *	-- mp is still on global list, and is accessible.
 */
cdfs_unmount(mp, mntflags)
	struct mount *mp;
	int mntflags;
{
	register struct cdfsmount *ump;
	register struct fs *fs;
	int error, i, flags = 0;
	struct buf *bp;

	mntflushbuf(mp, 0);
	if (mntinvalbuf(mp))
		return (EBUSY);
	BM(MOUNT_LOCK(mp));
	ump = VFSTOCDFS(mp);
	BM(MOUNT_UNLOCK(mp));
	if (!ump)
		return(ENODEV);
	if (error = vflush(mp, NULLVP, flags))
		return (error);
#if	MACH_NBC
	mfs_cache_clear();		/* remove cached mapped files */
#endif
#if	MACH
	/*
	 * We need to deal with uncaching mapped files in a 
	 * synchronous manner
	 */
#endif
	fs = ump->um_fs;

	/* clear device mappings */
	if (fs->rrip_devmap)
	    kfree(fs->rrip_devmap, sizeof(struct rrip_devmap) *  CD_MAXDMAP);

#if	MACH
	/*
	 * use kfree for the first one
	 */
	ZFREE(cdfspvd_zone, fs);
#else
	free((caddr_t)fs, M_SUPERBLK);
#endif
	MOUNT_LOCK(mp);
	ump->um_fs = NULL;
	ump->um_dev = NODEV;
	MOUNT_UNLOCK(mp);
	/*
	 * Clear mounted flag from this vnode
	 */
	(void) setmount(ump->um_devvp, SM_CLEARMOUNT);
/* In SVR4, driver open and close routines expect to get the OTYP_MNT flag
 * if the open/close was done as the result of a mount/unmount.  While OSF/1 
 * can pass a flag parameter to device open/close routines, it is only 
 * supported in spec_open() and spec_close(). These are the functions invoked
 * via VOP_OPEN and VOP_CLOSE for device special files. Therefore, we need 
 * to inform spec_open()/spec_close() that we are doing a mount/unmount.
 */
	VOP_CLOSE(ump->um_devvp, FREAD|OTYP_MNT, NOCRED, error);
	vrele(ump->um_devvp);
	ump->um_devvp = (struct vnode *)0;
	MOUNT_LOCK(mp);
	mp->m_flag &= ~M_LOCAL;
	MOUNT_UNLOCK(mp);
#if	MACH
	ZFREE(cdfsmount_zone, ump);
#else
	free((caddr_t)ump, M_MOUNT);
#endif
	return (error);
}

/*
 * Return root of a filesystem
 * Synchronization assumptions:
 *	-- it's safe, and file system isn't going anywhere.
 */
cdfs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	register struct vnode *vp;
	struct cdfsmount *nmp;
	struct cdnode *np;
	struct fs *fs;
	int error;

	CDDEBUG1(MNTDEBUG, printf("cdfs_root: Inside\n"));
	fs = VFSTOCDFS(mp)->um_fs;
	if (error = cdnodeget(mp, fs->iso_rootino, &np)) {
	    CDDEBUG1(MNTDEBUG,
		     printf("cdfs_root: cdnodeget returned error %d\n",
			    error));
		return (error);
	}
	vp = CDTOV(np);
	VN_LOCK(vp);
	vp->v_type = VDIR;
	vp->v_flag |= VROOT;
	VN_UNLOCK(vp);
	*vpp = vp;
	CDDEBUG1(MNTDEBUG, printf("cdfs_root: Leaving\n"));
	return (0);
}

/*
 * Do operations associated with quotas
 */
cdfs_quotactl(mp, cmds, uid, arg)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
{
	return(0);
}

/*
 * Get file system statistics.
 *
 * Synchronization assumptions:
 *	-- File system isn't going anywhere.
 *	-- Lock order: mount structure, then struct fs.
 */
cdfs_statfs(mp)
	struct mount *mp;
{
	register struct statfs *sbp;
	register struct fs *fs;
	struct cdfsmount *cdfsmp;
	struct cdfs_args *cdfsargsp;

	CDDEBUG1(ISODEBUG, printf("cdfs_statfs: In cdfs_statfs\n"));
	MOUNT_LOCK(mp);
	sbp = &mp->m_stat;
	fs = VFSTOCDFS(mp)->um_fs;

	sbp->f_type = MOUNT_CDFS;
	/* f_fsize is the fundamental block size. */
	/* f_bsize is the optimal transfer size */
	sbp->f_fsize = ISOFS_LBS(fs);
	sbp->f_bsize = CDFS_IOBLOCKSIZE;
	sbp->f_bfree = 0;
	sbp->f_bavail = 0;

	/* we count blocks in f_fsize chunks.  So we adjust it as below */
	if (fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) {
		sbp->f_files = (int)fs->fs_block.isofs.iso_path_tbl_size_lsb;
		sbp->f_blocks = (int)fs->fs_block.isofs.iso_vol_space_size_lsb;
	} else {
		sbp->f_files = (int)fs->fs_block.hsgfs.iso_path_tbl_size_lsb;
		sbp->f_blocks = (int)fs->fs_block.hsgfs.iso_vol_space_size_lsb;
	}
	if (((VFSTOCDFS(mp))->um_flag & (M_LOWERCASE|M_DROPVERS)) ==
	    (M_LOWERCASE|M_DROPVERS)) {
	    /* compatibility mode: xlate back to M_NOVERSION */
	    sbp->mount_info.cdfs_args.flags =
		((VFSTOCDFS(mp))->um_flag & ~(M_LOWERCASE|M_DROPVERS)) |
		    M_NOVERSION;
	} else
	    sbp->mount_info.cdfs_args.flags = (VFSTOCDFS(mp))->um_flag;
	MOUNT_UNLOCK(mp);
	return (0);
}

cdfs_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{
	return(0);
}

/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is in range
 * - call iget() to get the locked inode
 * - check for an unallocated inode (cd_mode == 0)
 * - check that the generation number matches
 *
 * Synchronization assumptions:
 *	-- File system isn't going anywhere.
 */
cdfs_fhtovp(mp, fhp, vpp)
	register struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{
	register struct cdfid *ufhp;
	register struct fs *fs;
	register struct cdnode *cdp;
	struct cdnode *ncdp;
	int error;

	CDDEBUG1(ISODEBUG, printf("cdfs_fhtovp: Inside\n"));
	ufhp = (struct cdfid *)fhp;
	fs = VFSTOCDFS(mp)->um_fs;
	if (error = cdnodeget(mp, ufhp->cdfid_ino, &ncdp)) {
		*vpp = (struct vnode *)0;
		return (error);
	}
	cdp = ncdp;
	IN_LOCK(cdp);
	/*
	 * If the file has been unlinked or the inode has been reclaimed,
	 * we return EINVAL.
	 */
	if ((cdp->cd_mode == 0) ||
	    (cdp->cd_gen != ufhp->cdfid_gen)) {
		IN_UNLOCK(cdp);
		cdnodeput(cdp);
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	*vpp = CDTOV(cdp);
	IN_UNLOCK(cdp);
	CDDEBUG1(ISODEBUG, printf("cdfs_fhtovp: Leaving\n"));
	return (0);
}

/*
 * Vnode pointer to File handle
 * Synchronization assumptions:
 *	-- cd_gen and cd_number are read-only
 */
/* ARGSUSED */
cdfs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
	register struct cdnode *cdp = VTOCD(vp);
	register struct cdfid *ufhp;

	CDDEBUG1(ISODEBUG, printf("cdfs_vptofh: Inside\n"));
	ufhp = (struct cdfid *)fhp;
	ufhp->cdfid_len = sizeof(struct cdfid);
	ufhp->cdfid_ino = cdp->cd_number;
	ufhp->cdfid_gen = cdp->cd_gen;
	CDDEBUG1(ISODEBUG, printf("cdfs_vptofh: Leaving\n"));
	return (0);
}
