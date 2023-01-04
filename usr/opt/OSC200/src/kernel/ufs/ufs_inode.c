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
static char	*sccsid = "@(#)$RCSfile: ufs_inode.c,v $ $Revision: 4.3.17.3 $ (DEC) $Date: 1993/06/03 15:41:15 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *    @(#)ufs_inode.c 7.28 (Berkeley) 2/8/90
 */
#if	MACH
#include <quota.h>
#include <mach_assert.h>
#include <xpr_debug.h>
#include <inocache_stats.h>
#include <rt_preempt.h>
#endif

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#include <sys/secpolicy.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#if	QUOTA
#include <ufs/quota.h>
#endif
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/fs_proto.h>
#include <ufs/ufsmount.h>
#include <ufs/icstats.h>
#include <kern/event.h>
#include <sys/lock_types.h>
#include <sys/kernel.h>

#include <mach/mach_types.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vfs_ubc.h>
#if	MACH
#include <kern/zalloc.h>
#else
#include <sys/malloc.h>
#endif
#include <vm/heap_kmem.h>
#include <vm/vm_tune.h>
#include <sys/dk.h>	/* for SAR counters */

#include <dcedfs.h>

int		prtactive;		/* 1 => print active vnode reclaim */
struct	ihead	*ihead;			/* inode hash chains */

#if	INOCACHE_STATS
struct icache_stats 	icache_stats;
#define	ICSTAT(clause)	STATS_ACTION(&icache_stats_lock, (clause))
vdecl_simple_lock_data(,icache_stats_lock)
#else
#define	ICSTAT(clause)
#endif

/*
 * Generation number manipulation
 */
u_int	nextgennumber;		/* next generation number to assign */

udecl_simple_lock_data(,gen_lock)

#define	GEN_LOCK()		usimple_lock(&gen_lock)
#define	GEN_UNLOCK()		usimple_unlock(&gen_lock)
#define	GEN_LOCK_INIT()		usimple_lock_init(&gen_lock)


/*
 * Conceal some Mach/Unix details.
 */
#if	MACH
#define	TEMP_FS_ALLOC(s,size)	ZALLOC(superblock_zone, (s), daddr_t *)
#define	TEMP_FS_FREE(s)		ZFREE(superblock_zone, (s))
#else
#define	TEMP_FS_ALLOC(s,size)	MALLOC((s), daddr_t *, (size), M_TEMP,M_WAITOK)
#define	TEMP_FS_FREE(s)		FREE((s), M_TEMP);
#endif


/*
 * Initialize hash links for inodes.
 */
ufs_init()
{
	register int i;
	register struct ihead *ih = ihead;
	register int num_pages;
	extern int nmount_max;
	extern int ufs_blkpref_lookbehind;
	vdecl_simple_lock_data(extern,blkpref_handy_lock)

	ASSERT((inohsz & inohsz-1) == 0);
	for (i = inohsz; --i >= 0; ih++) {
		IHASH_LOCK_INIT(ih);
		ih->ih_head[0] = ih;
		ih->ih_head[1] = ih;
		ih->ih_timestamp = 0;
	}
#if	MACH
	/*
	 * superblocks for mounted file systems.  These are used
	 * when mounting file systems and truncating files.
	 */
	superblock_zone = zinit(
#ifdef __alpha
			      MAXBSIZE+ALPHA_EXT,
			      nmount*(MAXBSIZE+ALPHA_EXT),
			      MAXBSIZE+ALPHA_EXT, 
#else
			      MAXBSIZE,
			      nmount*MAXBSIZE,
			      MAXBSIZE, 
#endif
			      "superblocks");
	if (superblock_zone == (zone_t) NULL)
		panic("ufs_init: no zones");
#endif
	GEN_LOCK_INIT();
	VSTATS_LOCK_INIT(&blkpref_handy_lock);
	MOUNTTAB_LOCK_INIT();
#if	QUOTA
	dqinit();
#endif
	/*
	 * If the number of blocks to search back is bogus,
	 * set to resonable value.
	 */
	if (ufs_blkpref_lookbehind < 1)
		ufs_blkpref_lookbehind = 1;
	else if (ufs_blkpref_lookbehind > 20)
		ufs_blkpref_lookbehind = 20;
	return (0);
}


extern struct vnodeops ufs_vnodeops, spec_inodeops, fifo_inodeops;
#if SEC_FSCHANGE
extern struct vnsecops ufs_vnsecops;
#endif

/*
 * ....................TEMPORARY CODE FOR BL6.............................
 * 
 * xlate_dev(rdev, format)
 *
 * Inputs:
 *	rdev   - the old, or new format dev_t to translate
 *	format - 1 translate old to new format
 *	         0 translate new to old format
 *
 * 	This routine will take an old-format dev_t entry and translate
 *	it into the new 32-bit dev_t format.  This routine is being
 *	provided so that BL6 kernels will run on BL5 systems (with old
 *	device special files).  Once a BL6 kit has been propagated to
 *	developers, this routine will be removed.
 */
xlate_dev(rdev,format)
	dev_t	rdev;
	int	format;
{
/*
 * These macros break down old-style dev_t's for major, minor,
 * unit, partition and density information.
 *
 *  OLDMAJ(x)        -- picks off old format major number
 *  OLDMIN(x)        -- picks off old format minor number
 *  OLDMAKEDEV(x,y)  -- creates old format dev_t
 *  OLDDU(dev)       -- gets old format disk unit number
 *  OLDDP(dev)       -- gets old format disk partition number
 *  OLDUNIT(dev)     -- gets old format tape unit number
 *  OLDSEL(dev)      -- gets old format tape density info
 *  OLDDSKMINOR(x,y) -- creates old format disk minor number given unit/part
 *  OLDTAPMINOR(x,y) -- creates old format tape minor number given unit/dens
 *  OLDUTOCAMU(x)    -- translates old disk unit to new CAM/LUN format
 *  CAMUTOOLDU(x)    -- translates CAM/LUN format to old disk unit 
 *  MAKECAMMIN(x,y)  -- translates old minor format to new CAM minor
 */
#define oldformat	1
#define OLDMAJ(x)	((int)(((unsigned)(x)>>8)&0377))
#define OLDMIN(x)	((int)((x)&0377))
#define OLDMAKEDEV(x,y)	((dev_t)(((x)<<8) | (y)))
#define OLDDU(dev)	((OLDMIN(dev) >> 3)&037)
#define OLDDP(dev)	(OLDMIN(dev)&07)
#define OLDUNIT(dev)	(((OLDMIN(dev)&0xe0)>>3)|(OLDMIN(dev)&0x03))
#define OLDSEL(dev)	((OLDMIN(dev)&0x01c) >> 2)
#define OLDDSKMINOR(x,y) ( (((x)&037)<<3)|((y)&07) )
#define OLDTAPMINOR(x,y) ( (((x)<<3)&0xe0)|((y << 2)&0x01c)|((x)&0x03) )
#define OLDUTOCAMU(x)   ( ((x&030) << 5)|((x&07) << 4) )
#define CAMUTOOLDU(x)	( ((GETUNIT(x) >> 5)&030)|((GETUNIT(x) >> 4)&07) )
#define MAKECAMMIN(x,y) (MAKEMINOR(OLDUTOCAMU(x),y)) 

    dev_t tmpmin, tmpmaj;

/*
 * Check if we are translating old to new or new to old and
 * annotate get major number.  We then check for devices which
 * specially mung the minor number field and make sure information
 * gets shifted to the right place in the old or new structure.
 */ 
    if (format == oldformat) 
        tmpmaj = OLDMAJ(rdev);
    else
        tmpmaj = major(rdev);
 
    switch(tmpmaj) {

        /*
         * SCSI disk device special file.
         */         
        case 8:
            if (format == oldformat) 
                tmpmin = MAKECAMMIN(OLDDU(rdev),OLDDP(rdev));
	    else
                tmpmin = OLDDSKMINOR(CAMUTOOLDU(rdev),GETDEVS(rdev));
            break;
        
        /*
         * SCSI tape device special file.
         */         
        case 9:
            if (format == oldformat) 
                tmpmin = MAKECAMMIN(OLDUNIT(rdev),OLDSEL(rdev));
	    else
                tmpmin = OLDTAPMINOR(CAMUTOOLDU(rdev),GETDEVS(rdev));
            break;

        /*
         * MSCP or MD disk device special file.
         */         
        case 23:
        case 28:
        case 17:
            if (format == oldformat) 
                tmpmin= MAKEMINOR(OLDDU(rdev),OLDDP(rdev));
	    else
                tmpmin = OLDDSKMINOR(GETUNIT(rdev),GETDEVS(rdev));
            break;

        /*
         * MSCP tape device special file.
         */         
        case 27:
            if (format == oldformat) 
                tmpmin = MAKEMINOR(OLDUNIT(rdev),OLDSEL(rdev));
	    else
                tmpmin = OLDTAPMINOR(GETUNIT(rdev),GETDEVS(rdev));
            break;

        /*
         * Run-of-the-mill device special file, just annotate
         * the minor number as is...
         */         
        default:
            if (format == oldformat) 
                tmpmin = OLDMIN(rdev);
	    else
                tmpmin = minor(rdev);
            break;
    }

/*
 * Return new format dev entry...
 */
    if (format == oldformat)
        rdev = makedev(tmpmaj,tmpmin);
    else
        rdev = OLDMAKEDEV(tmpmaj,tmpmin);

return(rdev);
}
/*
 * ...................END TEMPORARY CODE FOR BL6..........................
 */


/*
 * Look up an vnode/inode by device,inumber.
 * If it is in core (in the inode structure),
 * honor the locking protocol.
 * If it is not in core, read it in from the
 * specified device.
 * Callers must check for mount points!!
 * Callers guarantee that the filesystem won't
 * become unmounted; typically, xp is a referenced
 * inode on the target filesystem.
 * In all cases, a pointer to a unlocked
 * inode structure (with an incremented vnode
 * reference count) is returned.
 *
 * If update parameter set, read dinode data from disk unconditionally,
 * to refresh the information in the cache.  Leave non-dinode state
 * as it is; it won't have changed during a mount update.
 */
iget(xp, ino, ipp, update)
	struct inode *xp;
	ino_t ino;
	struct inode **ipp;
	int update;
{
	struct mount *mntp;
	struct fs *fs;
	struct inode *ip, *ip2, *iq;
	struct vnode *vp, *nvp;
	struct buf *bp;
	struct dinode *dp;
	struct	ihead *ih;
	u_int iflag;
	int error, mflag, stamp;
	dev_t dev;
	extern int cluster_consec_init;
	extern int cluster_lastr_init;
#if	QUOTA
	int i;
#endif

        tf_iget++; /* global table() system call counter (see table.h) */

	mntp = ITOV(xp)->v_mount;
	if (mntp == DEADMOUNT)
		return (ENODEV);
	fs = VFSTOUFS(mntp)->um_fs;
	dev = xp->i_dev;
	ICSTAT(icache_stats.ic_iget_call++);
loop:
	ICSTAT(icache_stats.ic_iget_loop++);
	ih = &ihead[INOHASH(dev, ino)];
	IHASH_LOCK(ih);
	for (ip = ih->ih_chain[0]; ip != (struct inode *)ih; ip = ip->i_forw) {
		if (ino != ip->i_number || dev != ip->i_dev)
			continue;
		vp = ITOV(ip);
		VN_LOCK(vp);
		error = vget_nowait(vp);
		VN_UNLOCK(vp);
		if (error) {
			IHASH_UNLOCK(ih);
			/*
			 * The inode we seek is undergoing traumatic change.
			 * Wait for that change to complete before going on.
			 */
			if (vget(vp) == 0)
				vrele(vp);
			ICSTAT(icache_stats.ic_iget_vget++);
			goto loop;
		}
		IHASH_UNLOCK(ih);
		IN_LOCK(ip);
		if (ip->i_flag & INACTIVATING) {
			ip->i_flag |= INACTWAIT;
			assert_wait((vm_offset_t)&ip->i_flag, FALSE);
			IN_UNLOCK(ip);
			thread_block();
			iput(ip);
			goto loop;
		}
		IN_UNLOCK(ip);
		(void) event_wait(&ip->i_iodone, FALSE, 0);
		BM(IN_LOCK(ip));
		iflag = ip->i_flag;
		BM(IN_UNLOCK(ip));
		if (iflag & IREADERROR) {
			iput(ip);
			return(EIO);
		}
		if (update)
			break;
		*ipp = ip;
		ICSTAT(icache_stats.ic_iget_hit++);
		ICSTAT(ip->i_mode == 0 ? icache_stats.ic_iget_reallocd++ : 0);
#ifdef	notyet
		/*
		 * We should really be checking this assertion.  However,
		 * it is currently possible for an NFS client to send us
		 * a file handle for a file that has been unlinked.  The
		 * inode for the file may not have been inactivated.  So
		 * we may find the inode in the cache with a link count
		 * of 0 and a non-zero mode.  The generation number will
		 * have changed. This guarantees that NFS servers will
		 * return an error if they find such inodes in the cache.
		 * No threads will do igets on this inode from UFS because
		 * the file has been unlinked and its vnode is no longer
		 * in the name cache.
		 */
		ASSERT(ip->i_nlink > 0 || ip->i_mode == 0);
#endif
		return(0);
	}
	if (update) {
		ASSERT(ip == xp);	/* inode to be updated is xp */
		nvp = ITOV(ip);
		event_clear(&ip->i_iodone);
		goto updateskip;
	}
	stamp = ih->ih_timestamp;
	IHASH_UNLOCK(ih);
	/*
	 * Allocate a new inode.
	 */
	if (error = getnewvnode(VT_UFS, &ufs_vnodeops, &nvp)) {
		*ipp = 0;
		return (error);
	}
	ip = VTOI(nvp);
	ip->i_vnode = nvp;
	ip->i_flag = 0;
	ip->i_devvp = 0;
	ip->i_mode = 0;
	ip->i_diroff = 0;
	ip->i_dirstamp = 0;
	ip->i_forw = ip->i_back = ip;	/* in case we have to drop inode */
	/*
	 * Force other threads that find this inode in the cache to
	 * wait until initialization completes.
	 */
	event_clear(&ip->i_iodone);
#if	QUOTA
	for (i = 0; i < MAXQUOTAS; i++)
		ip->i_dquot[i] = NODQUOT;
#endif
	ip->i_dev = dev;
	ip->i_number = ino;
	IN_LOCK_INIT(ip);
	IN_IO_LOCK_INIT(ip);
	/*
	 * Put the inode onto its hash chain so that other threads will
	 * find it but only if there's not already an identical inode
	 * in the cache.  If the timestamp on the hash chain hasn't
	 * changed, we can skip re-scanning the chain.
	 */
	IHASH_LOCK(ih);
	if (stamp != ih->ih_timestamp) {
		ICSTAT(icache_stats.ic_iget_research++);
		for (ip2 = ih->ih_chain[0]; ip2 != (struct inode *)ih;
		     ip2 = ip2->i_forw) {
			if (ino == ip2->i_number && dev == ip2->i_dev) {
				struct vnode *tvp = ITOV(ip);
				IHASH_UNLOCK(ih);
				ip->i_flag |= IREADERROR;
				VN_LOCK(tvp);
				vgone(tvp, VX_NOSLEEP, 0);
				VN_UNLOCK(tvp);
				vrele(tvp);
				goto loop;
			}
		}
	}
	insque(ip, ih);
	ih->ih_timestamp++;
	IHASH_UNLOCK(ih);
	ICSTAT(icache_stats.ic_iget_insert++);
updateskip:
	/*
	 * Read in the disk contents for the inode.
	 */
	if (error = bread(VFSTOUFS(mntp)->um_devvp, fsbtodb(fs, itod(fs, ino)),
	    (int)fs->fs_bsize, NOCRED, &bp)) {
		/*
		 * Unlock and discard unneeded inode.
		 * i_mode is set to 0, which will cause anyone
		 * waiting for this inode to realize the inode
		 * is damaged.
		 */
		idrop(ip);
		brelse(bp);
		*ipp = 0;
		ICSTAT(icache_stats.ic_iget_error++);
		ASSERT(!update);	/* let's hope not! */
		return (error);
	}
	/*
	 * We don't need to lock the inode across this initialization
	 * because any other threads finding the inode in the cache sleep
	 * until this thread completes the initialization.
	 */
#if	SEC_FSCHANGE
	nvp->v_secop = &ufs_vnsecops;
	if (FsSEC(fs)) {
		struct sec_dinode       *dp;

		dp = (struct sec_dinode *) bp->b_un.b_dino + itoo(fs, ino);
		ip->i_din = dp->di_node;
		ip->i_disec = dp->di_sec;
	} else {
		dp = bp->b_un.b_dino + itoo(fs, ino);
		if (update) {
			dp->di_gen = ip->i_gen;
			if (bcmp(&(ip->i_din), dp, sizeof(struct dinode)))
				printf("WARNING: iget, dinode no. %d changed\n",
					ip->i_number);
		}
		ip->i_din = *dp;
		bzero(&ip->i_disec, sizeof ip->i_disec);
		bcopy(mntp->m_tag, ip->i_tag, sizeof ip->i_tag);
	}
#if	SEC_ILB
	SEC_SETOBJNUM(ip->i_tag, nvp - vnode);
	sp_init_obj_bits(ip->i_tag);
#endif	/* SEC_ILB */
#else	/* !SEC_FSCHANGE */
	dp = bp->b_un.b_dino;
	dp += itoo(fs, ino);
	if (update) {
		dp->di_gen = ip->i_gen;
		if (bcmp(&(ip->i_din), dp, sizeof(struct dinode)))
			printf("WARNING: iget, dinode no. %d changed\n",
				ip->i_number);
	}
	ip->i_din = *dp;
#endif	/* !SEC_FSCHANGE */
	brelse(bp);

	/*
	 * For new dev_t support, we have to be able to translate old
	 * dev_t formats (16-bit) to new dev_t format (32-bit).  Here
	 * we perform translation from old to new, if required.
	 */
	vp = ITOV(ip);
	vp->v_type = IFTOVT(ip->i_mode);
	if (vp->v_type == VCHR || vp->v_type == VBLK) {
	    if (((ip->i_rdev >> 16)&0xffff) == 0) {
		/*
		 * We have an old-style dev_t or a new-style dev_t for
		 * major number zero.  We next check for the old-style
		 * major number field (bits 7->15).  If zero, we have
		 * major number zero, of either the old or new format.
		 * If non-zero, we have an old-style dev_t format.
		 */
		if (((ip->i_rdev >> 8)&0xff) == 0) {
			/*
			 * NOTE: Major number zero is restricted to an
			 * 8-bit minor so that translation from old to
			 * new format can be performed.
			 */
			ip->i_rdev = makedev( 0, ((ip->i_rdev)&0xff) );
		}else{
			/*
			 * TEMPORARY:
			 * We have an old format dev_t so call the 
			 * translation routine, which will translate
			 * old to new.  Also, mark the inode so we
			 * know to xlate back to old format before
			 * going back to disk.
			 * NOTE: xlate_dev goes away after BL6 has kits
			 * and here we will only translate old major and
			 * minor numbers directly to new format (i.e. no
			 * special translation of old minor number). See
			 * xlate_dev for details.
			 */
			ip->i_rdev = xlate_dev(ip->i_rdev, 1);
			ip->i_flag |= IOLDDEVT;
		}
	    }
	}

	/*
	 * If we're updating, we're done.  Finish up and return.
	 */
	if (update) {
		*ipp = ip;
		event_post(&ip->i_iodone);
		vrele(nvp);
		return (0);
	}
	/*
	 * Initialize the associated vnode.  Vnode can't be found
	 * anywhere; if found in the inode cache, callers will wait.
	 */
	vp = ITOV(ip);
	vp->v_type = IFTOVT(ip->i_mode);
	if (vp->v_type == VCHR || vp->v_type == VBLK) {
		if (error = specalloc(vp, ip->i_rdev)) {
			/*
			 * Get rid of this bogus inode.  Anyone else
			 * finding this inode in the cache will be
			 * awoken, see that there was an error, and
			 * return EIO themselves.
			 */
			vp->v_type = VNON;
			idrop(ip);
			*ipp = 0;
			return (error);
		}
		vp->v_op = &spec_inodeops;
	} else if (vp->v_type == VFIFO)
		vp->v_op = &fifo_inodeops;
	if (ino == ROOTINO)
		vp->v_flag |= VROOT;
#if	SEC_FSCHANGE
	if (ip->i_type_flags & SEC_I_MLD)
		vp->v_flag |= VMLD;
#endif
	vp->v_lastr = cluster_lastr_init;
	/*
	 * Finish inode initialization.
	 */
	ip->i_fs = fs;
	ip->i_devvp = VFSTOUFS(mntp)->um_devvp;
	VREF(ip->i_devvp);
	/*
	 * Initialize UFS cluster variables.
	 */
	ip->i_consecreads = cluster_consec_init;
	ip->i_delayoff = 0;
	ip->i_delaylen = 0;
	ip->i_clusterlbn = 0;
	ip->i_clustersz = 0;
	/*
	 * Set up a generation number for this inode if it does not
	 * already have one.  This should only happen on old filesystems.
	 */
	if (ip->i_gen == 0) {
		ip->i_gen = get_nextgen();
		BM(MOUNT_LOCK(mntp));
		mflag = mntp->m_flag;
		BM(MOUNT_UNLOCK(mntp));
		if ((mflag & M_RDONLY) == 0)
			ip->i_flag |= IMOD;
	}
	insmntque(vp, mntp);		/* make it publically available */
	event_post(&ip->i_iodone);
	ASSERT(ip->i_nlink > 0 || ip->i_mode == 0);
	*ipp = ip;
	return (0);
}

/*
 * Decrement the reference count of an inode structure.
 */
iput(ip)
	register struct inode *ip;
{
	vrele(ITOV(ip));
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
idrop(ip)
register struct inode *ip;
{
	struct ihead	*ih;

	IN_LOCK(ip);
	ip->i_mode = 0;
	ip->i_flag |= IREADERROR;
	IN_UNLOCK(ip);

	ih = &ihead[INOHASH(ip->i_dev, ip->i_number)];
	IHASH_LOCK(ih);
	remque(ip);
	ip->i_forw = ip;		/* make fake hash chain */
	ip->i_back = ip;
	IHASH_UNLOCK(ih);

	event_post(&ip->i_iodone);
	iput(ip);
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
 *	i_nlink <= 0
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

ufs_inactive(vp)
	struct vnode *vp;
{
	register struct inode *ip = VTOI(vp);
	int mode, error = 0;
	int imode, mflag; 
	struct mount *mp;

	LASSERT(!IN_WRITE_HOLDER(ip));
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

	IN_LOCK(ip);
	/*
	 * Uncommon cases:  inode is being inactivated already
	 * or the inode failed to be read correctly from the disk.
	 */
	if (ip->i_flag & (INACTIVATING|IREADERROR)) {
		error = ip->i_flag & IREADERROR;
		IN_UNLOCK(ip);
		if (error) {
			VN_LOCK(vp);
			(void) vgone(vp, VX_NOSLEEP, 0);
			VN_UNLOCK(vp);
		}
		return(0);
	}
	ip->i_flag |= INACTIVATING;
	/*
	 * Get rid of inodes related to stale file handles
	 * or that are not entirely valid (see iget).
	 * Calling vgone will result in calling back into this function.
	 * We avoid recursion with the above check on INACTIVATING.
	 */
	if (ip->i_mode == 0 || mp == DEADMOUNT) {
		IN_UNLOCK(ip);
		VN_LOCK(vp);
		(void) vgone(vp, VX_NOSLEEP, 0);
		VN_UNLOCK(vp);
		IN_LOCK(ip);
		if (ip->i_flag & INACTWAIT)
			thread_wakeup((vm_offset_t)&ip->i_flag);
		ip->i_flag &= ~(INACTIVATING|INACTWAIT);
		IN_UNLOCK(ip);
		return(0);
	}
	imode = ip->i_mode;
	if (ip->i_nlink <= 0 && (mflag & M_RDONLY) == 0) {
		IN_UNLOCK(ip);
		/*
		 * Note that this inode could still be
		 * reactivated from the mount vnode list.
		 * Assume such users are only going to do
		 * a sync and are going to "do the right thing".
		 *
		 * Inodes reactivated by file handle translation
		 * call iget, which sleeps while INACTIVATING is set.
		 * When iget finally returns, i_mode is zero, causing
		 * file handle translation to fail.
		 */
#if	QUOTA
		/*
		 * No need to hold inode I/O write lock across
		 * this chkiq call because no one else knows
		 * about this inode.
		 */
		if (!getinoquota(ip))
			(void) chkiq(ip, -1, NOCRED, 0);
#endif
		error = itrunc(ip, 0, 0);
		ASSERT(ip->i_blocks == 0);
		IN_LOCK(ip);
#if	SEC_FSCHANGE
		bzero((caddr_t) &ip->i_disec, sizeof ip->i_disec);
#endif
		mode = ip->i_mode;
		imode = ip->i_mode = 0;
		ip->i_rdev = 0;
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		ifree(ip, ip->i_number, mode);
	} else
		IN_UNLOCK(ip);
	IUPDAT(ip, &time, &time, 0);
	/*
	 * If we are done with the inode, reclaim it
	 * so that it can be reused immediately.
	 */
	IN_LOCK(ip);
	VN_LOCK(vp);
	if (ip->i_flag & INACTWAIT)
		thread_wakeup((vm_offset_t)&ip->i_flag);
	ip->i_flag &= ~(INACTIVATING|INACTWAIT);
	IN_UNLOCK(ip);
	if (vp->v_usecount == 1 && imode == 0) 
		(void) vgone(vp, VX_NOSLEEP, 0);
	VN_UNLOCK(vp);
	return(error);
}

int iupdnot = 0;	/* TEMP: suppress time updates if true */

/*
 * Reclaim a device inode so that it can be used for other purposes.
 *
 * There must not be anyone else who knows about this inode.
 */
ufsspec_reclaim(vp)
	register struct vnode *vp;
{
	int error;
	if (!(error = spec_reclaim(vp)))
		error = ufs_reclaim(vp);
	return(error);
}

/*
 * Reclaim an inode so that it can be used for other purposes.
 *
 * There must not be anyone else who knows about this inode.
 */
ufs_reclaim(vp)
	register struct vnode *vp;
{
	register struct inode *ip = VTOI(vp);
	struct vnode *devvp;
	dev_t dev;
	ino_t ino;
	struct ihead *ih;
#if	QUOTA
	struct ufsmount *ump;
	int i;
#endif

	dev = ip->i_dev;
	ino = ip->i_number;
	ih = &ihead[INOHASH(dev, ino)];
	IHASH_LOCK(ih);
	/*
	 * Remove the inode from its hash chain.
	 * On occasion, the inode may be on a bogus hash-
	 * chain consisting only of itself; e.g., after
	 * an error reading an inode from disk.
	 */
	remque(ip);
	ip->i_forw = ip;
	ip->i_back = ip;
	IHASH_UNLOCK(ih);
	/*
	 * Purge old data structures associated with the inode.
	 */
	cache_purge(vp);
	IN_LOCK(ip);
	if (ip->i_devvp) {
		devvp = ip->i_devvp;
		ip->i_devvp = 0;
		IN_UNLOCK(ip);
		vrele(devvp);
	} else
		IN_UNLOCK(ip);
#if	QUOTA
	/*
	 * No one else knows about this inode/vnode
	 * so there's no concern about clobbering
	 * the i_dquot array.  Note that the quota
	 * routines that walk the mount vnode list
	 * call vget, which synchronizes appropriately
	 * with reclaim.  We must hold the ufsmount
	 * quota lock for reading while disposing of the
	 * dquots to prevent races with quotaon/quotaoff.
	 */
	ump = (struct ufsmount *)0;
        if (ITOV(ip)->v_mount != DEADMOUNT) {
                if (ump = VFSTOUFS(ITOV(ip)->v_mount))
	UMPQ_READ_LOCK(ump);
        }
	for (i = 0; i < MAXQUOTAS; i++) {
		if (ip->i_dquot[i] != NODQUOT) {
			dqrele(vp, ip->i_dquot[i]);
			ip->i_dquot[i] = NODQUOT;
		}
	}
        if (ump)
	UMPQ_READ_UNLOCK(ump);
#endif
	ip->i_flag = 0;
	return (0);
}

struct timeval ufs_unique_time;

struct ufs_writes {
	dev_t dev;
	u_long	sync_notblockalligned;
	u_long	async_notblockalligned;
	u_long	sync_page_boundary;
	u_long	async_page_boundary;
	u_long	page_append_tries;
	u_long	page_append_success;
	u_long	sync_blocks;
	u_long	async_blocks;
	u_long	sync_multi_pages;
	u_long	async_multi_pages;
	u_long	sync_single_pages;
	u_long	async_single_pages;
	u_long	sync_frags;
	u_long	async_frags;
	u_long	sync_others;
	u_long	async_others;
	u_long	dir_iupdats;
	u_long	file_iupdats;
	u_long	sync_dirs;
	u_long	async_dirs;
	u_long 	sync_meta_blocks;
	u_long 	async_meta_blocks;
	u_long	sync_meta_cgs;
	u_long	async_meta_cgs;
	u_long	sync_meta_others;
	u_long	async_meta_others;
};

extern struct ufs_writes ufs_writes;

/*
 * Check accessed and update flags on an inode structure.
 * If any is on, update the inode with the current time.
 * If waitfor is given, then must ensure I/O order,
 * so wait for write to complete.
 */
iupdat(ip, ta, tm, waitfor)
	register struct inode *ip;
	struct timeval *ta, *tm;
	int waitfor;
{
	struct buf *bp;
	struct vnode *vp = ITOV(ip);
	struct dinode *dp;
	register struct fs *fs;
	int error, ronly;
 	dev_t saved_rdev;
	register int s, changed;

	fs = ip->i_fs;
	BM(IN_LOCK(ip));
	changed = ((ip->i_flag & (IUPD|IACC|ICHG|IMOD|ICHGMETA)) != 0);
	BM(IN_UNLOCK(ip));
	if (!changed)
		return (0);
	BM(FS_LOCK(fs));
	ronly = fs->fs_ronly;
	BM(FS_UNLOCK(fs));
	if (ronly)
		return (0);
	BM(IN_LOCK(ip));
	if ((ip->i_flag & (IUPD|IACC|ICHG|IMOD|ICHGMETA)) == ICHGMETA) {
		/*
		 * optimization (see note after bread):  if block not
		 * incore, buffer has been flushed, and we can remove
		 * ICHGMETA without ever having to read it back in.
		 */
		if (!incore(ip->i_devvp, fsbtodb(fs, itod(fs, ip->i_number)))) {
			ip->i_flag &= ~ICHGMETA;
			BM(IN_UNLOCK(ip));
			return (0);
		}
	}
	BM(IN_UNLOCK(ip));
	error = bread(ip->i_devvp, fsbtodb(fs, itod(fs, ip->i_number)),
		(int)fs->fs_bsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return (error);
	}
	/*
	 * the problem is that sync(2) always causes a (async)write:
	 * the sequence of events are:
	 *  1. ufs_sync calls iupdat because ICHGMETA, which will only
	 *     be cleared on a synchronous fs-write operation.
	 *  2. bdwrite occurs below, since waitfor==0.
	 *     bdwrite will place buf on v_dirtyblkhd list.
	 *  3. ufs_sync calls vflushbuf which causes bawrite/bwrite.
	 * so if only ICHGMETA, see if there really is any need to do
	 * an iupdat.
	 */
	BM(IN_LOCK(ip));
	if ((ip->i_flag & (IUPD|IACC|ICHG|IMOD|ICHGMETA)) == ICHGMETA) {
		/*
		 * if this buffer is clean, we must have written it
		 * out sucessfully, and not only can we skip the bdwrite
		 * of this inode, but we can now remove the ICHGMETA if
		 * we've got a 'clean' buffer, i.e. only B_CACHE|B_BUSY.
		 */
		if ((bp->b_flags & ~(B_CACHE|B_BUSY)) == 0) {
			ip->i_flag &= ~ICHGMETA;
			BM(IN_UNLOCK(ip));
			brelse(bp);
			return (0);
		}
	}
	BM(IN_UNLOCK(ip));
	/*
	 * To be perfectly honest, nothing says that the time
	 * being passed in is the "system" time; however, it
	 * usually is so we take the lock around the whole thing.
	 */
	IN_LOCK(ip);
	s = splhigh();
	TIME_READ_LOCK();
	if (time.tv_sec > ufs_unique_time.tv_sec ||
	    time.tv_usec > ufs_unique_time.tv_usec) {
		ufs_unique_time.tv_sec = time.tv_sec;
		ufs_unique_time.tv_usec = time.tv_usec;
	} else {
		ufs_unique_time.tv_usec++;
	}
	if (iupdnot)
		ip->i_flag &= ~(ICHG);
	if (ip->i_flag&IACC) {
		if (ta == &time) {
			ip->i_atime = ufs_unique_time.tv_sec;
			ip->i_uatime = ufs_unique_time.tv_usec;
		} else {
			ip->i_atime = ta->tv_sec;
			ip->i_uatime = ta->tv_usec;
		}
	}
	if (ip->i_flag&IUPD) {
		if (tm == &time) {
			ip->i_mtime = ufs_unique_time.tv_sec;
			ip->i_umtime = ufs_unique_time.tv_usec;
		} else {
			ip->i_mtime = tm->tv_sec;
			ip->i_umtime = tm->tv_usec;
		}
	}
	if (ip->i_flag&ICHG) {
		ip->i_ctime = ufs_unique_time.tv_sec;
		ip->i_uctime = ufs_unique_time.tv_usec;
	}
	TIME_READ_UNLOCK();
	splx(s);
	ip->i_flag &= ~(IUPD|IACC|ICHG|IMOD);

	/*
	 * TEMPORARY: we specially make translated old dev_t formats
	 * for device special files when the inode is read from disk.
	 * Here we check if we have a translated dev_t and if so,
	 * save the current value and translate to old format to write
	 * back to disk.  The saved value (saved_rdev) is restored below...
	 */
	if (ip->i_flag & IOLDDEVT) {
	    saved_rdev = ip->i_rdev;
	    ip->i_rdev = xlate_dev(ip->i_rdev, 0);
	}

#if	SEC_FSCHANGE
	if (FsSEC(fs)) {
		struct sec_dinode       *dp;

		dp = (struct sec_dinode *) bp->b_un.b_dino +
			itoo(fs, ip->i_number);
		dp->di_node = ip->i_din;
		dp->di_sec = ip->i_disec;
	} else {
		dp = bp->b_un.b_dino + itoo(fs, ip->i_number);
		*dp = ip->i_din;
	}
#else	/* !SEC_FSCHANGE */
	dp = bp->b_un.b_dino + itoo(fs, ip->i_number);
	*dp = ip->i_din;
#endif	/* !SEC_FSCHANGE */

	/*
	 * TEMPORARY: retore the saved rdev value from above.  This
	 * is the already translated (old to new) format dev_t. 
	 */
	if (ip->i_flag & IOLDDEVT)
	    ip->i_rdev = saved_rdev;

	if (waitfor) {
		ip->i_flag &= ~ICHGMETA;
		IN_UNLOCK(ip);
		if (ip->i_dev == ufs_writes.dev) {
			if ((ip->i_mode & IFMT) == IFDIR)
				ufs_writes.dir_iupdats++;
			else
				ufs_writes.file_iupdats++;
		}
		return (bwrite(bp));
	}
	IN_UNLOCK(ip);
	bdwrite(bp, bp->b_vp);
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
void
itimes(ip, t1, t2)
struct inode *ip;
struct timeval *t1, *t2;
{
	register int s;
	register struct fs *fsp;

	/*
	 * Don't modify times on read-only file system.
	 */
	fsp = ip->i_fs;
	FS_LOCK(fsp);
	if (fsp->fs_ronly) {
		FS_UNLOCK(fsp);
		return;
	}
	FS_UNLOCK(fsp);
	IN_LOCK(ip);
	s = splhigh();
	TIME_READ_LOCK();
	if (time.tv_sec > ufs_unique_time.tv_sec ||
	    time.tv_usec > ufs_unique_time.tv_usec) {
		ufs_unique_time.tv_sec = time.tv_sec;
		ufs_unique_time.tv_usec = time.tv_usec;
	} else {
		ufs_unique_time.tv_usec++;
	}
	if ((ip)->i_flag&(IUPD|IACC|ICHG)) {
		(ip)->i_flag |= IMOD;
		if ((ip)->i_flag&IACC) {
			if (t1 == &time) {
				(ip)->i_atime = ufs_unique_time.tv_sec;
				(ip)->i_uatime = ufs_unique_time.tv_usec;
			} else {
				(ip)->i_atime = (t1)->tv_sec;
				(ip)->i_uatime = (t1)->tv_usec;
			}
		}
		if ((ip)->i_flag&IUPD) {
			if (t2 == &time) {
				(ip)->i_mtime = ufs_unique_time.tv_sec;
				(ip)->i_umtime = ufs_unique_time.tv_usec;
			} else {
				(ip)->i_mtime = (t2)->tv_sec;
				(ip)->i_umtime = (t2)->tv_usec;
			}
		}
		if ((ip)->i_flag&ICHG) {
			(ip)->i_ctime = ufs_unique_time.tv_sec;
			(ip)->i_uctime = ufs_unique_time.tv_usec;
		}
		(ip)->i_flag &= ~(IACC|IUPD|ICHG);
	}
	TIME_READ_UNLOCK();
	splx(s);
	IN_UNLOCK(ip);
}


#define	SINGLE	0	/* index of single indirect block */
#define	DOUBLE	1	/* index of double indirect block */
#define	TRIPLE	2	/* index of triple indirect block */
/*
 * Truncate the inode ip to at most length size.  Free affected disk
 * blocks -- the blocks of the file are removed in reverse order.
 *
 * NB: triple indirect blocks are untested.
 */
itrunc(oip, length, flags)
	register struct inode *oip;
	u_long length;
	int flags;
{
	register daddr_t lastblock;
	daddr_t bn, lbn, lastiblock[NIADDR];
	register struct fs *fs;
	register struct inode *ip;
	struct buf *bp;
#if	MACH
	u_long osize, size, offset;
	int level;
#else
	u_long offset, osize, size;
	int level;
#endif
	int count, nblocks, blocksreleased = 0;
	register int i;
	int aflags, error, allerror;
	struct inode tip;

	/*
	 *	Fast symbolic links have no storage.  Can truncate in place.
	 */
	if (((oip->i_mode & IFMT) == IFLNK) &&
	    ((oip->i_flags & IC_FASTLINK) != 0) &&
	    (oip->i_size > length)) {
		IN_WRITE_LOCK(oip);
		bzero(&(oip->i_symlink[length]), oip->i_size - length);
		IN_LOCK(oip);
		oip->i_size = length;
		oip->i_flag |= ICHG|IUPD;
		IN_UNLOCK(oip);
		IN_WRITE_UNLOCK(oip);
		error = iupdat(oip, &time, &time, (flags & IO_SYNC));
		return (error);
	}
	IN_WRITE_LOCK(oip);
	if (oip->i_size <= length) {
#if	SEC_ILB
                /*
                 * Catch here to reset object ILs when the object
                 * is already empty
                 */
                if (oip->i_size == 0 && length == 0 && FsSEC(oip->i_fs))
                        SP_EMPTY_OBJECT(oip->i_tag);
#endif
		IN_LOCK(oip);
 		if (length == oip->i_size) { 
			/*
			 *	Not extending.  Don't update synchronously
			 *	unless caller asked for it explicitly.
			 */
			oip->i_flag |= ICHG|IUPD;
			IN_UNLOCK(oip);
			IN_WRITE_UNLOCK(oip);
			error = iupdat(oip, &time, &time, (flags & IO_SYNC));
			return (error);
		}
		else {
			struct vnode *vp;
			struct uio uio;
			struct iovec iov;
			int zero = 0;

			/*
			 *	Write a zero at the end of the file to extend
			 *	it.  The write logic does all the dirty work.
			 */
			vp = ITOV(oip);
			IN_UNLOCK(oip);
			IN_WRITE_UNLOCK(oip);
			iov.iov_base = (caddr_t) &zero;
			iov.iov_len = 1;
			uio.uio_iov = &iov;
			uio.uio_iovcnt = 1;
			uio.uio_offset = length - 1;
			uio.uio_segflg = UIO_SYSSPACE;
			uio.uio_rw = UIO_WRITE;
			uio.uio_resid = 1;
#if defined(DCEDFS) && DCEDFS
			error = dfs_writeop(vp, &uio, (flags & IO_SYNC),
					    u.u_cred);
#else
			VOP_WRITE(vp, &uio, (flags & IO_SYNC), u.u_cred,
				  error);
#endif /* DCEDFS */
			return(error);
		}
	}
	/*
	 * Calculate index into inode's block list of
	 * last direct and indirect blocks (if any)
	 * which we want to keep.  Lastblock is -1 when
	 * the file is truncated to 0.
	 */
	LASSERT(IN_WRITE_HOLDER(oip));
	fs = oip->i_fs;
	lastblock = lblkno(fs, length + fs->fs_bsize - 1) - 1;
	lastiblock[SINGLE] = lastblock - NDADDR;
	lastiblock[DOUBLE] = lastiblock[SINGLE] - NINDIR(fs);
	lastiblock[TRIPLE] = lastiblock[DOUBLE] - NINDIR(fs) * NINDIR(fs);
	nblocks = btodb(fs->fs_bsize);
	/*
	 * Update the size of the file. If the file is not being
	 * truncated to a block boundry, the contents of the
	 * partial block following the end of the file must be
	 * zero'ed in case it ever become accessable again because
	 * of subsequent file growth.
	 */
	osize = oip->i_size;
	offset = blkoff(fs, length);
	if (offset == 0) {
		if (ITOV(oip)->v_type == VREG) {
			/*
			 * Calling ubc_invalidate() with a length of zero,
			 * will invalidate all the inode pages.
			 */
			ubc_invalidate(ITOV(oip), length, 
				       (length ? osize - length : 0),
				       B_INVAL);
		}
		IN_LOCK(oip);
		oip->i_size = length;
		IN_UNLOCK(oip);
	} else {
		register struct vnode *vp;

		lbn = lblkno(fs, length);
#if	QUOTA
		if (error = getinoquota(oip)) {
			IN_WRITE_UNLOCK(oip);
			return (error);
		}
#endif
		vp = ITOV(oip);
		if (vp->v_type == VREG) {
			vm_offset_t addr, poff;
			vm_page_t plist[(MAXBSIZE/MINBSIZE) + 1], *pla;
			register int plsz;
			register vm_page_t pp, *pl;

			size = lbn >= NDADDR ? fs->fs_bsize : 
				fragroundup(fs, offset);
			plsz = atop(round_page(size)) - atop(offset);
			if (!plsz) plsz = 1;

			if (plsz > (MAXBSIZE/MINBSIZE)) {
				pla = (vm_page_t *) h_kmem_zalloc((plsz + 1) * 
					sizeof pla);
				pl = pla;
			}
			else {
				pla = (vm_page_t *) 0;
				pl = plist;
			}
			
			poff = offset & page_mask;
			error = ufs_getpage(vp, (length & fs->fs_bmask) +
					    (offset - poff), poff,
					    (vm_prot_t *) 0, pl, plsz, 
					    (struct vm_mape_entry *) 0,
					    (vm_offset_t) 0, B_WRITE,
					    u.u_cred);

			if (error) {
				IN_WRITE_UNLOCK(oip);
				if (pla)
					h_kmem_free(pla,
						    (plsz + 1) * sizeof pla);
				return error;
			}

			for (pp = *pl; pp != VM_PAGE_NULL; pl++, pp = *pl) {
				vm_page_wait(pp);
				addr = ubc_load(pp, poff, PAGE_SIZE - poff);
				ubc_page_zero(pp, poff, PAGE_SIZE - poff);
				ubc_unload(pp, poff, PAGE_SIZE - offset);
				if (flags & IO_SYNC) {
					ubc_page_busy(pp, 1);
					ufs_rwblk(vp, pp, 1, 0, (daddr_t) 0);
					ubc_page_release(pp, 0);
				}
				else
					ubc_page_release(pp, B_DIRTY);
				poff = 0;
			}
			/*
			 * Calling ubc_invalidate() with a length of zero,
			 * will invalidate all the inode pages.
			 */
			ubc_invalidate(vp, length,
				       (length ? osize - length : 0), B_INVAL);

			IN_LOCK(oip);
			oip->i_size = length;
			IN_UNLOCK(oip);

			if (pla)
				h_kmem_free(pla, (plsz + 1) * sizeof pla);
		}
		else {
			aflags = B_CLRBUF;
			if (flags & IO_SYNC) aflags |= B_SYNC;
			if (error = balloc(oip, lbn, offset, &bp, aflags)) {
				IN_WRITE_UNLOCK(oip);
				return (error);
			}
			bn = bp->b_blkno;
			IN_LOCK(oip);
			oip->i_size = length;
			IN_UNLOCK(oip);
			size = blksize(fs, oip, lbn);
			bzero(bp->b_un.b_addr + offset, 
			      (unsigned)(size - offset));
			if (size != bp->b_bcount)
				allocbuf(bp, size);
			if (flags & IO_SYNC)
				bwrite(bp);
			else
				bdwrite(bp, bp->b_vp);
		}
	}
	/*
	 * Update file and block pointers
	 * on disk before we start freeing blocks.
	 * If we crash before free'ing blocks below,
	 * the blocks will be returned to the free list.
	 * lastiblock values are also normalized to -1
	 * for calls to indirtrunc below.
	 */
	tip = *oip;
	tip.i_size = osize;
	for (level = TRIPLE; level >= SINGLE; level--)
		if (lastiblock[level] < 0) {
			oip->i_ib[level] = 0;
			lastiblock[level] = -1;
		}
	for (i = NDADDR - 1; i > lastblock; i--)
		oip->i_db[i] = 0;
	IN_LOCK(oip);
#if	SEC_ILB
/* XXX inode locked */
	if (oip->i_size == 0 && FsSEC(oip->i_fs))
		SP_EMPTY_OBJECT(oip->i_tag);
#endif	/* SEC_ILB */
	oip->i_flag |= ICHG|IUPD;
	IN_UNLOCK(oip);
        vinvalbuf(ITOV(oip), (length > 0));
	allerror = iupdat(oip, &time, &time, MNT_WAIT);

	/*
	 * Don't have to lock ip because it's totally private.  However,
	 * tip is a copy of oip and thus inherited a writelocked I/O lock.
	 *
	 * Indirect blocks first.
	 */
	ip = &tip;
	for (level = TRIPLE; level >= SINGLE; level--) {
		bn = ip->i_ib[level];
		if (bn != 0) {
			error = indirtrunc(ip, bn, lastiblock[level], level,
				&count);
			if (error)
				allerror = error;
			blocksreleased += count;
			if (lastiblock[level] < 0) {
				ip->i_ib[level] = 0;
				blkfree(ip, bn, (off_t)fs->fs_bsize);
				blocksreleased += nblocks;
			}
		}
		if (lastiblock[level] >= 0)
			goto done;
	}

	/*
	 * All whole direct blocks or frags.
	 */
	for (i = NDADDR - 1; i > lastblock; i--) {
		register off_t bsize;

		bn = ip->i_db[i];
		if (bn == 0)
			continue;
		ip->i_db[i] = 0;
		bsize = (off_t)blksize(fs, ip, i);
		blkfree(ip, bn, bsize);
		blocksreleased += btodb(bsize);
	}
	if (lastblock < 0)
		goto done;

	/*
	 * Finally, look for a change in size of the
	 * last direct block; release any frags.
	 */
	bn = ip->i_db[lastblock];
	if (bn != 0) {
		off_t oldspace, newspace;

		/*
		 * Calculate amount of space we're giving
		 * back as old block size minus new block size.
		 */
		oldspace = blksize(fs, ip, lastblock);
		ip->i_size = length;
		newspace = blksize(fs, ip, lastblock);
		if (newspace == 0)
			panic("itrunc: newspace");
		if (oldspace - newspace > 0) {
			/*
			 * Block number of space to be free'd is
			 * the old block # plus the number of frags
			 * required for the storage we're keeping.
			 */
			bn += numfrags(fs, newspace);
			blkfree(ip, bn, oldspace - newspace);
			blocksreleased += btodb(oldspace - newspace);
		}
	}
done:
/* BEGIN PARANOIA */
	for (level = SINGLE; level <= TRIPLE; level++)
		if (ip->i_ib[level] != oip->i_ib[level])
			panic("itrunc1");
	for (i = 0; i < NDADDR; i++)
		if (ip->i_db[i] != oip->i_db[i])
			panic("itrunc2");
/* END PARANOIA */
	oip->i_blocks -= blocksreleased;
/* this is also PARANOIA that needs to disappear after we find the bug */
	if (oip->i_blocks < 0)			/* sanity */
		oip->i_blocks = 0;
	IN_LOCK(oip);
	oip->i_flag |= ICHG;
	IN_UNLOCK(oip);
#if	QUOTA
	if (!getinoquota(oip))
		(void) chkdq(oip, -blocksreleased, NOCRED, 0);
#endif
	IN_WRITE_UNLOCK(oip);
	return (allerror);
}

/*
 * Release blocks associated with the inode ip and
 * stored in the indirect block bn.  Blocks are free'd
 * in LIFO order up to (but not including) lastbn.  If
 * level is greater than SINGLE, the block is an indirect
 * block and recursive calls to indirtrunc must be used to
 * cleanse other indirect blocks.
 *
 * NB: triple indirect blocks are untested.
 */
indirtrunc(ip, bn, lastbn, level, countp)
	register struct inode *ip;
	daddr_t bn, lastbn;
	int level;
	int *countp;
{
	register int i;
	struct buf *bp;
	register struct fs *fs = ip->i_fs;
	register daddr_t *bap;
	daddr_t *copy, nb, last;
	int blkcount, factor;
	int nblocks, blocksreleased = 0;
	int error, allerror = 0;

	LASSERT(IN_WRITE_HOLDER(ip));
	/*
	 * Calculate index in current block of last
	 * block to be kept.  -1 indicates the entire
	 * block so we need not calculate the index.
	 */
	factor = 1;
	for (i = SINGLE; i < level; i++)
		factor *= NINDIR(fs);
	last = lastbn;
	if (lastbn > 0)
		last /= factor;
	nblocks = btodb(fs->fs_bsize);
	/*
	 * Get buffer of block pointers, zero those
	 * entries corresponding to blocks to be free'd,
	 * and update on disk copy first.
	 */
	error = bread(ip->i_devvp, fsbtodb(fs, bn), (int)fs->fs_bsize,
		NOCRED, &bp);
	if (error) {
		brelse(bp);
		*countp = 0;
		return (error);
	}
	bap = bp->b_un.b_daddr;
	TEMP_FS_ALLOC(copy, fs->fs_bsize);
	bcopy((caddr_t)bap, (caddr_t)copy, (u_int)fs->fs_bsize);
	bzero((caddr_t)&bap[last + 1],
	  (u_int)(NINDIR(fs) - (last + 1)) * sizeof (daddr_t));
	if (last == -1)
		bp->b_flags |= B_INVAL;
	error = bwrite(bp);
	if (error)
		allerror = error;
	bap = copy;
	/*
	 * Recursively free totally unused blocks.
	 */
	for (i = NINDIR(fs) - 1; i > last; i--) {
		nb = bap[i];
		if (nb == 0)
			continue;
		if (level > SINGLE) {
			error = indirtrunc(ip, nb, (daddr_t)-1, level - 1,
				&blkcount);
			if (error)
				allerror = error;
			blocksreleased += blkcount;
		}
		blkfree(ip, nb, (off_t)fs->fs_bsize);
		blocksreleased += nblocks;
	}

	/*
	 * Recursively free last partial block.
	 */
	if (level > SINGLE && lastbn >= 0) {
		last = lastbn % factor;
		nb = bap[i];
		if (nb != 0) {
			error = indirtrunc(ip, nb, last, level - 1, &blkcount);
			if (error)
				allerror = error;
			blocksreleased += blkcount;
		}
	}
	ASSERT(blocksreleased > 0 || ip->i_size > 0);
	TEMP_FS_FREE(copy);
	*countp = blocksreleased;
	return (allerror);
}


/*
 * Check mode permission on inode pointer. Mode is READ, WRITE or EXEC.
 * The mode is shifted to select the owner/group/other fields. The
 * super user is granted all permissions.
 *
 * NB: Called from vnode op table. It seems this could all be done
 * using vattr's but...
 */
iaccess(ip, mode, cred)
	register struct inode *ip;
	register int mode;
	struct ucred *cred;
{
	register gid_t *gp;
	uid_t iuid;
	gid_t igid, *egp;
	u_short imode;

#if	SEC_BASE
	if (security_is_on) {
#if	SEC_ARCH
		{
		udac_t          udac;

		udac.uid = udac.cuid = ip->i_uid;
		udac.gid = udac.cgid = ip->i_gid;
		udac.mode = ip->i_mode & 07777;

		if (SP_ACCESS(SIP->si_tag, ip->i_tag, mode, &udac)) {
			return EACCES;
		}
		return 0;
		}
#endif
		if (privileged(SEC_ALLOWDACACCESS, 0))
			return (0);
	} else {
		if (cred->cr_uid == 0)
			return (0); /* super-user always gets access. */
	}

#if	SEC_ARCH
	/* ignore all modes except read, write and exec */
	if (! (mode & (SP_READACC|SP_WRITEACC|SP_EXECACC))) {
		return (0);
	}
#endif
#else
	if (cred->cr_uid == 0)
		return (0); /* super-user always gets access. */
#endif /* SEC_BASE */

	/*
	 * Access check is based on only one of owner, group, public.
	 * If not owner, then check group. If not a member of the
	 * group, then check public access.
	 */
	IN_LOCK(ip);
	iuid = ip->i_uid;
	igid = ip->i_gid;
	imode = ip->i_mode;
	IN_UNLOCK(ip);
	if (cred->cr_uid != iuid) {
		mode >>= 3;
		if (igid == cred->cr_gid)
			goto found;
		gp = cred->cr_groups;
		egp = &(cred->cr_groups[cred->cr_ngroups]);
		for ( ; gp < egp ; gp++)
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

u_int
get_nextgen()
{
	register u_int gen;
	int s;

	GEN_LOCK();
	s = splhigh();
	TIME_READ_LOCK();
	if (++nextgennumber < time.tv_sec)
		nextgennumber = time.tv_sec;
	gen = nextgennumber;
	TIME_READ_UNLOCK();
	splx(s);
	GEN_UNLOCK();
	return(gen);
}
