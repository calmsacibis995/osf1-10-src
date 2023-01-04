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
static char	sccsid[] = "@(#)$RCSfile: cdfs_node.c,v $ $Revision: 4.3.14.3 $ (DEC) $Date: 1993/07/27 20:23:35 $";
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
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
/*****************************************************************
 *			Modification History
 *
 * 23-May-91 -- prs
 *	Initial Creation
 *
 *****************************************************************/
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/mode.h>
#include <ufs/dir.h>
#include <sys/namei.h>
#include <sys/errno.h>
#include <cdfs/cdfs.h>
#include <cdfs/cdfsnode.h>
#include <cdfs/cdfsmount.h>
#include <sys/kernel.h>
#include <sys/lock_types.h>
#if	MACH
#include <kern/zalloc.h>
#include <kern/mfs.h>
#include <mach/vm_param.h>
#else
#include <sys/malloc.h>
#endif
#include <kern/assert.h>
#include <kern/kalloc.h>

#define	CDFSHSZ	512

#define CDFSNOHASH(dev,ino)	(((dev)+(ino))&(CDFSHSZ-1))

struct cdhead {
	union {
		struct cdhead *cdhu_head[2];
		struct cdnode *cdhu_chain[2];
	} cdhu;
	u_int cdh_timestamp;
	udecl_simple_lock_data(,cdh_lock)
} cdhead[CDFSHSZ];

#define cdh_head	cdhu.cdhu_head
#define cdh_chain	cdhu.cdhu_chain

#define	CDFS_HASH_LOCK(cdh)	usimple_lock(&(cdh)->cdh_lock)
#define CDFS_HASH_UNLOCK(cdh)	usimple_unlock(&(cdh)->cdh_lock)
#define CDFS_HASH_LOCK_INIT(cdh)	usimple_lock_init(&(cdh)->cdh_lock)

#if	MACH
/*
 * These definitions need to move.
 * Keep them small at first to detect memory leaks faster.
 */
#define CDFSMOUNT_MAX	(512)
#define READDIR_MAX	16

/* About the same, and linked, so just use one zone. */
#define CDFSMOUNT_SIZE	(sizeof(struct cdfsmount))

/*
 * Called once to initialize data structures.
 * Memory zones allocated will initially be neither exhaustible nor pageable.
 * If we run out, we panic; this if fine for debugging, but not for production.
 * These attributes should should be examined further.  The sleepable attribute
 * is not used at this time, so is a noop.
 */

cdfs_zone_init()
{
	/*
	 * cdfsmount structures.  One per CDFS mounted file system.
	 * Not too heavily used, and tend to be long-lived.
	 */
	cdfsmount_zone = zinit(CDFSMOUNT_SIZE,
			      CDFSMOUNT_MAX * CDFSMOUNT_SIZE,
			      PAGE_SIZE, "cdfsmount");
	if (cdfsmount_zone == ZONE_NULL)
		panic("cdfs_zone_init: no zones");
	cdfsmount_zone->exhaustible = TRUE;

	/*
	 * Primary volume descriptor storage.
	 */

	cdfspvd_zone = zinit(MAXBSIZE,
			      CDFSMOUNT_MAX * MAXBSIZE,
			      MAXBSIZE, "cdfspvd");
	if (cdfspvd_zone == ZONE_NULL)
		panic("cdfs_zone_init: no zones");
	cdfspvd_zone->exhaustible = TRUE;

	/*
	 * cdfs_readdir() needs space to work.
	 */

	cdfsreaddir_zone = zinit(MAXBSIZE,
			      READDIR_MAX * MAXBSIZE,
			      MAXBSIZE, "cdfspvd");
	if (cdfsreaddir_zone == ZONE_NULL)
		panic("cdfs_zone_init: no zones");
	cdfsreaddir_zone->exhaustible = TRUE;
}

#endif

/*
 * Initialize hash links for nfsnodes
 * and build nfsnode free list.
 */
cdfs_init()
{
	register int i;
	register struct  cdhead *cdh = cdhead;

	cdfs_zone_init();
	for (i = CDFSHSZ; --i >= 0; cdh++) {
		cdh->cdh_head[0] = cdh;
		cdh->cdh_head[1] = cdh;
		CDFS_HASH_LOCK_INIT(cdh);
		cdh->cdh_timestamp = 0;
	}
}

extern struct vnodeops spec_cdnodeops, fifo_cdnodeops;

/*
 * Look up an vnode/cdnode by device,number.
 * If it is in core (in the cdnode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * Callers must check for mount points!!
 * Callers guarantee that the filesystem won't
 * become unmounted; typically, xp is a referenced
 * cdnode on the target filesystem.
 * In all cases, a pointer to a unlocked
 * cdnode structure (with an incremented vnode
 * reference count) is returned.
 *
 */
cdnodeget(mntp, ino, cdpp)
	struct mount *mntp;
	ino_t ino;
	struct cdnode **cdpp;
{
	struct cdfsmount *cdmntp;
	struct fs *fs;
	struct cdnode *cdp, *cdp2;
	struct vnode *vp, *nvp;
	struct buf *bp;
	struct	cdhead *cdh;
	u_int flag;
	int error, mflag, stamp;
	dev_t dev;
	extern struct vnodeops cdfs_vnodeops;
	time_t ret_time;
	u_char dirbytes[ISO_MAXDIRENTLEN];
	struct iso_dir	*isodir = (struct iso_dir *)dirbytes;
	union {
		unsigned char incoming[4];
		unsigned int  outgoing;
	} iso_convert_int;

	CDDEBUG1(ISODEBUG,
		 printf("cdnodeget: mntp = 0x%x ino = %d\n", mntp, ino));
	if (mntp == DEADMOUNT)
		return (ENODEV);
	cdmntp = VFSTOCDFS(mntp);
	fs = cdmntp->um_fs;
	dev = cdmntp->um_dev;
loop:
	cdh = &cdhead[CDFSNOHASH(dev, ino)];
	CDFS_HASH_LOCK(cdh);
	for (cdp = (struct cdnode *)cdh->cdh_chain[0]; 
	     cdp != (struct cdnode *)cdh; cdp = cdp->cd_forw) {
		if (ino != cdp->cd_number || dev != cdp->cd_dev)
			continue;
		vp = CDTOV(cdp);
		VN_LOCK(vp);
		error = vget_nowait(vp);
		VN_UNLOCK(vp);
		if (error) {
			CDFS_HASH_UNLOCK(cdh);
			/*
			 * The inode we seek is undergoing traumatic change.
			 * Wait for that change to complete before going on.
			 */
			if (vget(vp) == 0)
				vrele(vp);
			goto loop;
		}
		CDFS_HASH_UNLOCK(cdh);
		IN_LOCK(cdp);
		if (cdp->cd_flag & INACTIVATING) {
			cdp->cd_flag |= INACTWAIT;
			assert_wait((vm_offset_t)&cdp->cd_flag, FALSE);
			IN_UNLOCK(cdp);
			thread_block();
			cdnodeput(cdp);
			goto loop;
		}
		IN_UNLOCK(cdp);
		(void) event_wait(&cdp->cd_iodone, FALSE, 0);
		BM(IN_LOCK(cdp));
		flag = cdp->cd_flag;
		BM(IN_UNLOCK(cdp));
		if (flag & IREADERROR) {
			cdnodeput(cdp);
			return(EIO);
		}
		*cdpp = cdp;
		ASSERT(cdp->cd_nlink > 0 && cdp->cd_mode != 0);
		return(0);
	}
	stamp = cdh->cdh_timestamp;
	CDFS_HASH_UNLOCK(cdh);
	/*
	 * Allocate a new inode.
	 */
	if (error = getnewvnode(VT_CDFS, &cdfs_vnodeops, &nvp)) {
		*cdpp = 0;
		return (error);
	}
	cdp = VTOCD(nvp);
	cdp->cd_vnode = nvp;
	cdp->cd_flag = 0;
	cdp->cd_mode = 0;
	cdp->cd_fs = fs;
	/* since CDFS uses the devvp frequently below, we attach it and ref
	   it now, being careful to release it (usually indirectly) if
	   we fail to fully initialize here */
	cdp->cd_devvp = VFSTOCDFS(mntp)->um_devvp;
	VREF(cdp->cd_devvp);
	cdp->cd_forw = cdp->cd_back = cdp;  /* in case we have to drop inode */
	/*
	 * Force other threads that find this inode in the cache to
	 * wait until initialization completes.
	 */
	event_clear(&cdp->cd_iodone);
	cdp->cd_dev = dev;
	cdp->cd_number = ino;
	cdp->cd_rrip_fields = CDNODE_RRIP_RR_NOT;	/* none yet known */
	cdp->cd_linktarg = 0;
	bzero(&cdp->cd_rrip_fldoffset[0], sizeof(cdp->cd_rrip_fldoffset));

	IN_LOCK_INIT(cdp);
	IN_IO_LOCK_INIT(cdp);
	/*
	 * Put the inode onto its hash chain so that other threads will
	 * find it but only if there's not already an identical inode
	 * in the cache.  If the timestamp on the hash chain hasn't
	 * changed, we can skip re-scanning the chain.
	 */
	CDFS_HASH_LOCK(cdh);
	if (stamp != cdh->cdh_timestamp) {
		for (cdp2 = (struct cdnode *)cdh->cdh_chain[0]; 
		     cdp2 != (struct cdnode *)cdh;
		     cdp2 = cdp2->cd_forw) {
			if (ino == cdp2->cd_number && dev == cdp2->cd_dev) {
				CDFS_HASH_UNLOCK(cdh);
				/* cdnodeput calls vrele(), which eventually
				   gets back into cdfs_reclaim(), which
				   will dispose of the reference to the
				   device vnode */
				cdnodeput(cdp);
				goto loop;
			}
		}
	}
	insque(cdp, cdh);
	cdh->cdh_timestamp++;
	CDFS_HASH_UNLOCK(cdh);
	/*
	 * Initialize FS specific portion of cdnode with fields from
	 * directory record.
	 */
	if (error = cdfs_readisodir(cdp,
				    fs->fs_format == ISO_RRIP ? isodir : 0)) {
		/*
		 * Unlock and discard unneeded inode.
		 * cd_mode is set to 0, which will cause anyone
		 * waiting for this inode to realize the inode
		 * is damaged.
		 */
		cdnodedrop(cdp);	/* does vrele() of devvp indirectly */
		*cdpp = 0;
		return (error);
	}
	if (fs->fs_format == ISO_RRIP) {
	    struct cd_suf_header *suf;
	    cdfs_isodir_to_idir(fs, isodir, 0, &cdp->cd_cdin);
	    if (!(cdp->cd_rrip_fields & CDNODE_RRIP_RR_SEEN)) {
		bp = 0;
		suf = susp_search_dir(fs, cdp->cd_devvp, isodir,
				      fs->rrip_susp_offset, TRUE,
				      RRIP_SIG_RR, &bp, cdp);
		/* side effect: if RR is seen, susp_search() will
		   set cdp->cd_rrip_fields appropriately */
		if (bp)
		    brelse(bp);
		bp = 0;
		if (!(cdp->cd_rrip_fields & CDNODE_RRIP_RR_SEEN))
		    cdp->cd_rrip_fields |= CDNODE_RRIP_RR_NOT; /* XXX */
	    }
#ifdef CDFSDEBUG
	    if ((WEIRDDEBUG || RRIPDEBUG) &&
		(cdp->cd_rrip_fields & CDNODE_RRIP_RR_SEEN)
		&& !(cdp->cd_rrip_fields & RRIP_RR_PX_PRESENT))
		printf("cdfs_getnode: PX not present on node %d\n",
		       cdp->cd_number);
#endif /* CDFSDEBUG */
	    if (lookfor(cdp, PX)) {
		struct rrip_px *px;
		px = (struct rrip_px *)
		    susp_search_dir(fs, cdp->cd_devvp, isodir,
				    fs->rrip_susp_offset, TRUE,
				    RRIP_SIG_PX, &bp, cdp);
		if (px) {
		    /* all the RRIP defined modes come from POSIX, so
		       we just copy it. */
		    CDFS_COPYINT(px->px_mode_lsb, iso_convert_int.incoming);
		    cdp->cd_mode = (u_short) iso_convert_int.outgoing;
		    CDFS_COPYINT(px->px_nlink_lsb, iso_convert_int.incoming);
		    cdp->cd_nlink = (u_short) iso_convert_int.outgoing;

		    /* uid/gid mapping may still occur with XCDR below. */
		    CDFS_COPYINT(px->px_uid_lsb, iso_convert_int.incoming);
		    cdp->cd_uid = (uid_t) iso_convert_int.outgoing;
		    CDFS_COPYINT(px->px_gid_lsb, iso_convert_int.incoming);
		    cdp->cd_gid = (gid_t) iso_convert_int.outgoing;
		    if (bp)
			brelse(bp);
		    bp = 0;
		    setdone(cdp, PX);
		    if (CD_ISBLK(cdp->cd_mode) || CD_ISCHR(cdp->cd_mode)) {
			dev_t mappeddev;
			if (lookfor(cdp, PN)) {
			    struct rrip_pn *pn;
			    pn = (struct rrip_pn *)
				susp_search_dir(fs, cdp->cd_devvp, isodir,
						fs->rrip_susp_offset, TRUE,
						RRIP_SIG_PN, &bp, cdp);
			    if (pn) {
				nvp->v_type =
				    CD_ISBLK(cdp->cd_mode) ? VBLK : VCHR;
				/* DEC OSF/1 only uses 32-bit dev_t's,
				   so we should just use the low 32 bits,
				   per RRIP document */
				CDFS_COPYINT(pn->pn_dev_t_high_lsb,
					     iso_convert_int.incoming);
				if (iso_convert_int.outgoing) {
				    /* XXX However, other systems
				       (e.g. Young Minds authoring software)
				       put major in dev_t_high, minor in
				       dev_t_low. */
				    /* Strategy: if the high word is non-zero,
				       do a makedev() on the high/low parts.
				       makedev will end up truncating
				       any out-of-range bits, which would be
				       useless anyway without remapping */
				    int high = iso_convert_int.outgoing;

				    CDFS_COPYINT(pn->pn_dev_t_low_lsb,
						 iso_convert_int.incoming);
				    cdp->cd_ondiskrdev =
					makedev(high,
						iso_convert_int.outgoing);
				} else {
				    CDFS_COPYINT(pn->pn_dev_t_low_lsb,
						 iso_convert_int.incoming);
				    cdp->cd_ondiskrdev =
					iso_convert_int.outgoing;
				}
				setdone(cdp, PN);
			    }
			    if (bp)
				brelse(bp);
			} else {
			    CDDEBUG1(WEIRDDEBUG,
				     printf("cdfs_getnode: PN not present on dev node %d\n",
					    cdp->cd_number));
			    cdnodedrop(cdp); /* vrele() indirectly */
			    *cdpp = 0;
			    return EINVAL;
			}
			mappeddev = rrip_getnodedevmap(fs, cdp);
			if (mappeddev != NODEV)
			    /* it's a mapped device */
			    cdp->cd_rdev = mappeddev;
			else
			    cdp->cd_rdev = cdp->cd_ondiskrdev;
		    }
		}
	    }
	    if (lookfor(cdp, TF)) {
		struct rrip_tf *tf;
		int count;
		unsigned char *susp_bufp;
		int tfflags;
		struct cd_suf_ce cont;

		susp_compute_diroff(isodir, fs->rrip_susp_offset,
				    &count, &susp_bufp);
		/* there may be multiple TFs */
		bp = 0;
		bzero(&cont, sizeof(cont));
		if (cached(cdp, TF)) 
		    tf = (struct rrip_tf *)
			susp_search_cached(fs, cdp->cd_devvp, &susp_bufp,
					   &count, TRUE, RRIP_SIG_TF,
					   &bp, cdp, &cont);
		else
		    tf = (struct rrip_tf *)
			susp_search(fs, cdp->cd_devvp, &susp_bufp,
				    &count, TRUE, RRIP_SIG_TF, &bp, cdp,
				    &cont);
		tfflags = 0;
		while (tf) {
		    /* XXX susp_search only returns version 1 entries */
		    int tsnum = 1;

		    tfflags |= tf->tf_flags;

		    if (tf->tf_flags & RRIP_TF_CREATION)
			/* ignore it for now */
			tsnum++;
		    if (tf->tf_flags & RRIP_TF_MODIFY) {
			cdp->cd_mtime =
			    rrip_convert_tf_ts(tf->tf_flags&RRIP_TF_LONG_FORM,
					       RRIP_TF_TSPTR(tf,tsnum));
			tsnum++;
		    }
		    if (tf->tf_flags & RRIP_TF_ACCESS) {
			cdp->cd_atime =
			    rrip_convert_tf_ts(tf->tf_flags&RRIP_TF_LONG_FORM,
					       RRIP_TF_TSPTR(tf,tsnum));
			tsnum++;
		    }
		    if (tf->tf_flags & RRIP_TF_ATTRIBUTES) {
			cdp->cd_ctime =
			    rrip_convert_tf_ts(tf->tf_flags&RRIP_TF_LONG_FORM,
					       RRIP_TF_TSPTR(tf,tsnum));
			tsnum++;
		    }
		    /* ignore BACKUP, EXPIRATION, EFFECTIVE timestamps */
		    /* go get more, if there are any... */
		    /* don't pass a valid cdp, since we know this won't be
		       the first */
		    tf = (struct rrip_tf *) susp_search(fs, cdp->cd_devvp,
							&susp_bufp,
							&count, TRUE,
							RRIP_SIG_TF,
							&bp, 0, &cont);
		}
		if (bp)
		    brelse(bp);
		bp = 0;
		if (!(tfflags & RRIP_TF_MODIFY))
		    cdp->cd_mtime = (time_t) 0;	/* XXX */
		if (!(tfflags & RRIP_TF_ACCESS))
		    cdp->cd_atime = (time_t) 0;	/* XXX */
		if (!(tfflags & RRIP_TF_ATTRIBUTES))
		    cdp->cd_ctime = cdp->cd_mtime;
		setdone(cdp, TF);
	    }
	    /* if there's an SL field here, do a bit of legwork on it... */
	    if (lookfor(cdp, SL)) {
		struct rrip_sl *sl;
		sl = (struct rrip_sl *)
		    susp_search_dir(fs, cdp->cd_devvp, isodir,
				    fs->rrip_susp_offset, TRUE,
				    RRIP_SIG_SL, &bp, cdp);
		if (bp)
		    brelse(bp);
		if (sl) {
		    unsigned char *pullup;
		    pullup = rrip_pullup_symlink(fs, mntp, cdp, isodir);
		    if (pullup) {
			/* we recopy, since it's been assembled in a borrowed
			   pathname translation buffer, which is probably
			   much bigger than we really need, and which we
			   shouldn't keep too long. */
			cdp->cd_size = strlen(pullup);
			cdp->cd_linktarg = kalloc(cdp->cd_size);
			bcopy(pullup, cdp->cd_linktarg, cdp->cd_size);
			PN_DEALLOCATE(pullup);
			setdone(cdp,SL);
		    }
		}
	    }
	    /* mostly done with RRIP fields. */
	}
	/*
	 * If the volume sequence number located in the directory record
	 * does not match the primary volume descriptors volume sequence
	 * number, don't initialize cdnode.
	 *
	 * XXX - for HSG format. Do check only if primary volume descriptor
	 * set size is greater than one (1).
	 */
	if ((fs->fs_format == ISO_HSG && ISOFS_SETSIZE(fs) > 1 &&
	    cdp->iso_dir_vol_seq_no != ISOFS_VOLSEQNUM(fs)) ||
	    ((fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) &&
	     cdp->iso_dir_vol_seq_no != ISOFS_VOLSEQNUM(fs))) {
	    /* mark node as not present.  for XCDR sec 3.6 ENODEV support */
	    cdp->cd_flag |= CDNODE_BADVOL;
	    cdp->iso_dir_xar = 0;	/* nuke the XAR...it's not on
					  this volume */
	}
	/*
	 * Fill in default values, if not done by RRIP.
	 */
	if (!isdone(cdp, TF)) {
	    
	    /* XCDR sez:
	       use the RDT in the directory entry if there's no XAR, otherwise
	       use the XAR timestamps. */
	    if (!cdp->iso_dir_xar) {
		cdfs_tounixdate(&ret_time,
				cdp->iso_dir_dt[0], 
				cdp->iso_dir_dt[1],
				cdp->iso_dir_dt[2], 
				cdp->iso_dir_dt[3],
				cdp->iso_dir_dt[4], 
				cdp->iso_dir_dt[5],
				((fs->fs_format == ISO_9660 ||
				  fs->fs_format == ISO_RRIP) ? 
				 cdp->iso_dir_dt[6] : 0));
		cdp->cd_atime = cdp->cd_mtime = cdp->cd_ctime = ret_time;
	    } else {
		/* RRIP provided owner, group, mode bits.
		   get some timestamps from the XAR. */
		/* cdfs_readxar() checks isdone(cdp, PX) */
		cdfs_readxar(cdp, cdmntp, NULL);
	    }
	}
	if (!isdone(cdp, PX)) {
	    /* We need owner, group, mode bits, and possibly timestamps */
	    cdp->cd_uid = 0;
	    cdp->cd_gid = 1;

	    /*
	     *	iso9660: setup default cd_mode and cd_nlink;
	     *	note cd_mode must be set to all access
	     *	before using (if it exists) xar modifiers.
	     */
	    if (cdp->iso_dir_file_flags&ISO_FLG_DIR) {
	 	cdp->cd_mode = CDFDIR |
		    (S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	 	cdp->cd_nlink = 2;
	    } else {
		cdp->cd_mode = CDFREG |
		    (S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	 	cdp->cd_nlink = 1;
	    }
	    /*
	     * Call cdfs_readxar to further initialize the node.
	     * It won't disturb permission stuff if M_NODEFPERM or
	     * flags & ISO_FLG_PROTECT.
	     * However, it will change the default mode to that specified
	     * by cdmntsuppl -F/-D, if !M_NODEFPERM && ISO_FLG_PROTECT)
	     */
	    cdfs_readxar(cdp, cdmntp, NULL);
	}
	/*
	 * Initialize the associated vnode.  Vnode can't be found
	 * anywhere; if found in the cdnode cache, callers will wait.
	 */
	vp = CDTOV(cdp);
	vp->v_type = CDFTOVT(cdp->cd_mode);
	if (vp->v_type == VCHR || vp->v_type == VBLK) {
	    if (error = specalloc(vp, cdp->cd_rdev)) {
		/*
		 * Get rid of this bogus inode.  Anyone else
		 * finding this inode in the cache will be
		 * awoken, see that there was an error, and
		 * return EIO themselves.
		 */
		vp->v_type = VNON;
		cdnodedrop(cdp);	/* vrele() of devvp, indirectly */
		*cdpp = 0;
		return (error);
	    }
	    vp->v_op = &spec_cdnodeops;
	} else if (vp->v_type == VFIFO)
		vp->v_op = &fifo_cdnodeops;
	else if (isdone(cdp, PN))
	    printf("cdfs_getnode: device node not VCHR/VBLK? (%o/%d)",
		   cdp->cd_mode, vp->v_type);
	    
	if (ino == fs->iso_rootino)
		vp->v_flag |= VROOT;
	/*
	 * Finish cdnode initialization.
	 */
	/* VREF(cdp->cd_devvp); */	/* see above */
	/*
	 * Set up a generation number for this inode if it does not
	 * already have one.  This should only happen on old filesystems.
	 */
	cdp->cd_gen = cdp->cd_number;
#if	MACH_NBC
	vp->v_vm_info->vnode_size = cdp->cd_size;
#endif
	insmntque(vp, mntp);		/* make it publically available */
	event_post(&cdp->cd_iodone);
	ASSERT(cdp->cd_nlink > 0 || cdp->cd_mode == 0);
	*cdpp = cdp;
	return (0);
}

/*
 * Decrement the reference count of an inode structure.
 */
cdnodeput(cdp)
	register struct cdnode *cdp;
{
	vrele(CDTOV(cdp));
}


/*
 * iget left an inode in a funny state, either
 * because the disk read failed or because it
 * wasn't possible to finish initializing the inode.
 *
 * It is possible that other threads found this inode
 * in the cache so we must wake them up; setting
 * IREADERROR lets them know there was a problem.
 *
 * Eventually everyone will iput/vrele the inode out
 * of existence; ufs_inactive and ufs_reclaim check
 * for IREADERROR.
 *
 * We also remove the inode from its hash chain so that
 * subsequent attempts to find the inode will try
 * all over.
 */
cdnodedrop(cdp)
register struct cdnode *cdp;
{
	struct cdhead	*cdh;

	IN_LOCK(cdp);
	cdp->cd_mode = 0;
	cdp->cd_flag |= IREADERROR;
	IN_UNLOCK(cdp);

	cdh = &cdhead[CDFSNOHASH(cdp->cd_dev, cdp->cd_number)];
	CDFS_HASH_LOCK(cdh);
	remque(cdp);
	cdp->cd_forw = cdp;		/* make fake hash chain */
	cdp->cd_back = cdp;
	CDFS_HASH_UNLOCK(cdh);

	event_post(&cdp->cd_iodone);
	cdnodeput(cdp);
}


/*
 * Last reference to an inode, write the inode out and if necessary,
 * truncate and deallocate the file.
 *
 * This activity is racy:  this inode could be reactivated by vget
 * through the namei cache or through the vnode mount list.  However,
 * if the link count went to zero, the only way the vnode may be
 * reactivated is through the mount vnode list (sync).
 *
 * Note that the vnode layer guarantees that inactive requests will
 * be serialized but it is still possible (albeit unlikely) to receive
 * multiple inactive requests on an inode that has a 0 link count.
 *
 * Interesting conditions:
 *	IREADERROR
 *	cd_nlink <= 0
 *
 * Can't have simultaneous inactives because the count properties
 * guarantee that only one will get through at a time.  However, could
 * have sequential inactives.  (1 would go to 0, calls inactive; vget
 * sends count to 2; first guy finishes inactivating, decrements count
 * to one; second guy would send count to 0, calls inactive.)
 *
 * However, we could be racing vclean.
 *
 * Three possible ways to re-activate:
 *	1.  namei cache -- not an issue for linkcount 0.
 *	2.  file handle, goes through iget
 *	3.  mount vnode list, goes through vget
 */

cdfs_inactive(vp)
	struct vnode *vp;
{
	register struct cdnode *cdp = VTOCD(vp);
	int mode, error = 0;
	int imode, mflag; 
	struct mount *mp;

	LASSERT(!IN_WRITE_HOLDER(cdp));
	mp = vp->v_mount;
	/*
	 * mp will be null only if there was an error in iget prior
	 * to the insmntque call.
	 */
	if (mp != DEADMOUNT) {
		BM(MOUNT_LOCK(mp));
		mflag = mp->m_flag;
		BM(MOUNT_UNLOCK(mp));
	}

	IN_LOCK(cdp);
	/*
	 * Uncommon cases:  inode is being inactivated already
	 * or the inode failed to be read correctly from the disk.
	 */
	if (cdp->cd_flag & (INACTIVATING|IREADERROR)) {
		error = cdp->cd_flag & IREADERROR;
		IN_UNLOCK(cdp);
		if (error) {
			VN_LOCK(vp);
			(void) vgone(vp, VX_NOSLEEP, 0);
			VN_UNLOCK(vp);
		}
		return(0);
	}
	cdp->cd_flag |= INACTIVATING;
	/*
	 * Get rid of inodes related to stale file handles
	 * or that are not entirely valid (see iget).
	 * Calling vgone will result in calling back into this function.
	 * We avoid recursion with the above check on INACTIVATING.
	 */
	if (cdp->cd_mode == 0 || mp == DEADMOUNT) {
		IN_UNLOCK(cdp);
		VN_LOCK(vp);
		(void) vgone(vp, VX_NOSLEEP, 0);
		VN_UNLOCK(vp);
		IN_LOCK(cdp);
		if (cdp->cd_flag & INACTWAIT)
			thread_wakeup((vm_offset_t)&cdp->cd_flag);
		cdp->cd_flag &= ~(INACTIVATING|INACTWAIT);
		IN_UNLOCK(cdp);
		return(0);
	}
	imode = cdp->cd_mode;
	VN_LOCK(vp);
	if (cdp->cd_flag & INACTWAIT)
		thread_wakeup((vm_offset_t)&cdp->cd_flag);
	cdp->cd_flag &= ~(INACTIVATING|INACTWAIT);
	IN_UNLOCK(cdp);
	if (vp->v_usecount == 1 && imode == 0) 
		(void) vgone(vp, VX_NOSLEEP, 0);
	VN_UNLOCK(vp);
	return(error);
}

/*
 * Reclaim a cdnode so that it can be used for other purposes.
 *
 * There must not be anyone else who knows about this cdnode.
 */
cdfs_reclaim(vp)
	register struct vnode *vp;
{
	register struct cdnode *cdp = VTOCD(vp);
	struct vnode *devvp;
	dev_t dev;
	ino_t ino;
	struct cdhead *cdh;

	dev = cdp->cd_dev;
	ino = cdp->cd_number;
	cdh = &cdhead[CDFSNOHASH(dev, ino)];
	CDFS_HASH_LOCK(cdh);
	/*
	 * Remove the inode from its hash chain.
	 * On occasion, the inode may be on a bogus hash-
	 * chain consisting only of itself; e.g., after
	 * an error reading an inode from disk.
	 */
	remque(cdp);
	cdp->cd_forw = cdp;
	cdp->cd_back = cdp;
	CDFS_HASH_UNLOCK(cdh);
	/*
	 * Purge old data structures associated with the cdnode.
	 */
	cache_purge(vp);
	IN_LOCK(cdp);
	if (cdp->cd_devvp) {
		devvp = cdp->cd_devvp;
		cdp->cd_devvp = 0;
		IN_UNLOCK(cdp);
		vrele(devvp);
	} else
		IN_UNLOCK(cdp);
	if (cdp->cd_linktarg)
	    kfree(cdp->cd_linktarg, cdp->cd_size);
	cdp->cd_flag = 0;
	return (0);
}

/*
 * Reclaim a device inode so that it can be used for other purposes.
 *
 * There must not be anyone else who knows about this inode.
 */
cdfsspec_reclaim(vp)
	register struct vnode *vp;
{
	int error;
	if (!(error = spec_reclaim(vp)))
		error = cdfs_reclaim(vp);
	return(error);
}

/*
 * Check accessed and update flags on an inode structure.
 * If any is on, update the inode with the current time.
 * If waitfor is given, then must ensure I/O order,
 * so wait for write to complete.
 */
cdnodeupdat(cdp, ta, tm, waitfor)
	register struct cdnode *cdp;
	struct timeval *ta, *tm;
	int waitfor;
{
	return (0);
}


/*
 * We cheat on the time.  In theory, the passed
 * in pointers could refer to any timeval.  In practice,
 * the vast majority of itimes call pass pointers to
 * the system time variable.  So we assume that we are
 * using the system time and once in a while pay the
 * price of doing irrelevant locking when the timeval
 * pointers refer to something else.
 */
cdnodetimes(cdp, t1, t2)
struct cdnode *cdp;
struct timeval *t1, *t2;
{
	/*
	 * Don't modify times on read-only file system.
	 */
	return(0);
}


/*
 * Check mode permission on inode pointer. Mode is READ, WRITE or EXEC.
 * The mode is shifted to select the owner/group/other fields. The
 * super user is granted all permissions.
 *
 * NB: Called from vnode op table. It seems this could all be done
 * using vattr's but...
 */
cdnodeaccess(cdp, mode, cred)
	register struct cdnode *cdp;
	register int mode;
	struct ucred *cred;
{
	register gid_t *gp;
	uid_t iuid;
	gid_t igid;
	u_short imode;
	int i;

	/*
	 * If you're the super-user, you always get access.
	 */
	if (cred->cr_uid == 0)
		return (0);
	/*
	 * Access check is based on only one of owner, group, public.
	 * If not owner, then check group. If not a member of the
	 * group, then check public access.
	 */
	IN_LOCK(cdp);
	iuid = cdp->cd_uid;
	igid = cdp->cd_gid;
	imode = cdp->cd_mode;
	IN_UNLOCK(cdp);
	if (cred->cr_uid != iuid) {
		mode >>= 3;
		if (igid == cred->cr_gid)
			goto found;
		gp = cred->cr_groups;
		for (i = 0; i < cred->cr_ngroups; i++, gp++)
			if (igid == *gp)
				goto found;
		mode >>= 3;
found:
		;
	}
	if ((imode & mode) != 0)
		return (0);
	return (EACCES);
}
