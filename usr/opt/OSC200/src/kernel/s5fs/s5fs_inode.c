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
static char	*sccsid = "@(#)$RCSfile: s5fs_inode.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/05/27 19:26:46 $";
#endif 
/*
 */
/*
   * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * History:
 *
 *	August 20, 1991: vsp
 *		OSF/1 Release 1.0.1 bug fixes.
 *		Addition of inode and disk inode locking.
 *		s5iupdat(): Reset inode flag before buffer block 
 *			update starts.
 *		s5fs_inactive(): inode locking bug fix.
 *
 * 	ADK:
 *		original OSF/1 Release 1.0
 *
 *
 */


#include <sysv_fs.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <s5fs/s5param.h>
#include <s5fs/filsys.h>
#include <s5fs/s5inode.h>
#include <s5fs/s5ino.h>
#include <s5fs/s5mount.h>
#include <s5fs/s5dir.h>
#include <sys/stat.h>
#include <sys/kernel.h>
#include <mach/vm_param.h>
#include <sys/dk.h>	/* for SAR counters */

/*
 *	inode hashing
 */

#define	S5INOHSZ	512
#if	((S5INOHSZ&(S5INOHSZ-1)) == 0)
#define	S5INOHASH(dev,ino)	(((dev)+(ino))&(S5INOHSZ-1))
#else
#define	S5INOHASH(dev,ino)	(((unsigned)((dev)+(ino)))%S5INOHSZ)
#endif

union s5ihead {				/* inode LRU cache */
	union  s5ihead *ih_head[2];
	struct s5inode *ih_chain[2];
} s5ihead[S5INOHSZ];

extern  zone_t	s5_superblock_zone;

int prtactive;	/* 1 => print out reclaim of active vnodes */

/*
 * These definitions need to move.
 * Keep them small at first to detect memory leaks faster.
 */
#define S5MOUNT_MAX	(64+32)
#define S5MOUNT_SIZE	(sizeof(struct filsys))

/*
 * Initialize s5fs file system specific items.
 */
s5fs_init()
{
	register int i;
	register union  s5ihead *s5 = s5ihead;
	

#ifndef lint
	if (vn_maxprivate < sizeof(struct s5inode))
		panic("s5_init: too small");
#endif
	for (i = S5INOHSZ; --i >= 0; s5++) {
		s5->ih_head[0] = s5;
		s5->ih_head[1] = s5;
	}
/*
 * S5mount structures.  One per mounted S5 file system.
 * Not too heavily used, and tend to be long-lived.
 */
	if (s5_superblock_zone == 0)
	        s5_superblock_zone = zinit(S5MOUNT_SIZE,
			      S5MOUNT_MAX * S5MOUNT_SIZE,
			      PAGE_SIZE, "s5mount");
	if (s5_superblock_zone == (zone_t) NULL)
		panic("s5_init: no zones");
/*
 * Need 2 buffers to transform directory entries in s5fs_readdir.
 */
	lock_init(&readbuflock, TRUE);
	readdirbuf = (struct s5direct *)kalloc(MAX_S5BSIZE);
	gpdirect = (struct gpdirect *)kalloc(MAX_S5BSIZE);
	return (0);
}

s5fs_uninit()
{
	kfree(readdirbuf);
	kfree(gpdirect);
	return (0);
}

/*
 * Look up an inode by device,inumber.
 * If it is in core (in the inode structure), honor the locking protocol.
 * If it is not in core, read it in from the specified device.
 * If the inode is mounted on, perform the indicated indirection.
 * In all cases, a pointer to an unlocked inode structure is returned.
 *
 * printf warning: inode table overflow -- if the inode structure is full
 * panic: no imt -- if the mounted filesystem is not in the mount table.
 *	"cannot happen"
 */
s5iget(dp, ino, ipp)
	struct s5inode *dp;
	s5ino_t ino;
	struct s5inode **ipp;
{
	dev_t dev = dp->i_dev;
	struct mount *mntp;

	extern struct vnodeops s5fs_vnodeops, spec_s5inodeops, s5fifo_inodeops;
	struct vnode *vp;
	struct s5inode *ip;
	struct vnode *nvp;
	union  s5ihead *ih;
	int error = 0;

	mntp = S5ITOV(dp)->v_mount;

        tf_iget++; /* global table() system call counter (see table.h) */

loop:

	ih = &s5ihead[S5INOHASH(dev, ino)];

	for (ip = ih->ih_chain[0]; ip != (struct s5inode *)ih; ip = ip->i_forw) {
		if (ino != ip->i_number || dev != ip->i_dev)
			continue;
		/*
		 *  We found the in-core inode
		 */
		if (!lock_try_write(&ip->i_lock)) {
			s5ILOCK(ip);
			s5IUNLOCK(ip);
			goto loop;
		}

		if (vget(S5ITOV(ip))) {
		        s5IUNLOCK(ip);
			goto loop;
		}

		s5IUNLOCK(ip);
        	/* global table() system call counter (see table.h) */
		pg_v_s5ifp++;
		*ipp = ip;
		return(0);
	}
	/*
	 * Allocate a new inode.
	 */
	if (error = getnewvnode(VT_S5FS, &s5fs_vnodeops, &nvp)) {
		*ipp = 0;
		return (error);
	}
	ip = S5VTOI(nvp);
	ip->i_vnode = nvp;
	ip->i_flag = 0;
	ip->i_devvp = 0;
	ip->i_dirstamp = 0;
	ip->i_mode = 0;
	ip->i_s5fs = 0;
	s5IN_LOCK_INIT(ip);

	/*
	 * Put it onto its hash chain and lock it so that other requests for
	 * this inode will block if they arrive while we are sleeping waiting
	 * for old data structures to be purged or for the contents of the
	 * disk portion of this inode to be read.
	 */
	ip->i_dev = dev;
	ip->i_number = ino;
	insque(ip, ih);
	s5ILOCK(ip);
	/*
	 * Read in the disk contents for the inode.
	 */

	/*
	if(((long)mntp % 4) != 0)
		 panic("s5iget: we are going to die in the call");
	*/

	error = s5iread(ip, mntp, &ip);
	if (error != 0) {
		s5IUNLOCK(ip);
		s5iput(ip);
		return(error);
	}

	/*
	 * Initialize the associated vnode
	 */
	vp = S5ITOV(ip);
	vp->v_type = S5IFTOVT(ip->i_mode);
	if (vp->v_type == VCHR || vp->v_type == VBLK) {
		if (error = specalloc(vp, ip->i_rdev)) {
			/*
			 * Clean up
			 */
			vp->v_type = VNON;
			ip->i_mode = 0;
			s5IUNLOCK(ip);
			s5iput(ip);
			return(error);
		}
		vp->v_op = &spec_s5inodeops;
	} else if (vp->v_type == VFIFO)
	        vp->v_op = &s5fifo_inodeops;
	if (ino == s5ROOTINO)
		vp->v_flag |= VROOT;
	/*
	 * Finish inode initialization.
	 */
	ip->i_s5fs = VFSTOS5FS(mntp)->um_fs;
	ip->i_devvp = VFSTOS5FS(mntp)->um_devvp;
	VREF(ip->i_devvp);

	/*
	 * Set up a generation number for this inode.  This must be always
         * be done for system V filesystems because there is no room for
         * the generation number on the disk inode.
	 */
	ip->i_gen = get_nextgen();
	insmntque(vp, mntp);
	MOUNT_LOCK(mntp);
	if ((vp->v_mount->m_flag & M_RDONLY) == 0)
		ip->i_flag |= S5ICHG;
	MOUNT_UNLOCK(mntp);
	*ipp = ip;
	s5IUNLOCK(ip);
	return (0);
}

s5iread(ip, mntp, ipp)
register struct s5inode *ip;
struct  mount *mntp;
struct s5inode **ipp;
{
	register char *p1, *p2;
	register struct s5dinode *dp;
	struct buf *bp;
	unsigned i;
	struct vnode *devvp = VFSTOS5FS(mntp)->um_devvp;
	struct filsys *fs = VFSTOS5FS(mntp)->um_fs;
	int bsize, error = 0;

	*ipp = 0;
	bsize = FsBSIZE(fs);

	error = bread(devvp, FsLTOP(bsize, FsITOD(bsize, ip->i_number)), bsize,
				NOCRED, &bp);
	if (error) {
		brelse(bp);
		return(error);
	}
	dp = bp->b_un.b_s5dino;
	dp += FsITOO(bsize, ip->i_number);
	ip->i_mode = dp->di_mode;
	ip->i_nlink = dp->di_nlink;
	ip->i_uid = dp->di_uid;
	ip->i_gid = dp->di_gid;
	ip->i_size = dp->di_size;
	p1 = (char *)ip->i_addr;
	p2 = (char *)dp->di_addr;
	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = 0;
	}
	brelse(bp);
	*ipp = ip;
	return(0);
}

/*
 * Decrement reference count of an inode structure.
 * On the last reference, write the inode out and if necessary,
 * truncate and deallocate the file.
 */
s5iput(ip)
register struct s5inode *ip;

{
	vrele(S5ITOV(ip));
}

/*
 * Update the inode with the current time.
 * No need to lock anything here.  The buffer cache locking deals
 * with it.
 */
s5iupdat(ip, ta, tm)
register struct s5inode *ip;
struct timeval *ta, *tm;
{
	struct buf *bp;
	struct s5dinode *dp;
	register char *p1;
	char *p2;
	unsigned i;
	int bsize, syn, error = 0;


  	/*
  	 * Don't do anything if inode not touched.
  	 */
  	if (!(ip->i_flag&(S5IUPD|S5ICHG|S5IACC)))
  		return(0);
  
	if (S5ITOV(ip)->v_mount->m_flag & M_RDONLY) {
		if(ip->i_flag&(S5IUPD|S5ICHG))
			ip->i_flag &= ~(S5IACC|S5IUPD|S5ICHG|S5ISYN);
		return(EROFS);
	}
	bsize = FsBSIZE(ip->i_s5fs);
	error = bread(ip->i_devvp, FsLTOP(bsize, FsITOD(bsize, ip->i_number)),
				bsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return(error);
	}
	dp = bp->b_un.b_s5dino;
	dp +=  FsITOO(bsize, ip->i_number);
	dp->di_mode = ip->i_mode;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	p1 = (char *)dp->di_addr;
	p2 = (char *)ip->i_addr;

	for(i=0; i<NADDR; i++) {
		*p1++ = *p2++;
		*p1++ = *p2++;
		*p1++ = *p2++;
		if(*p2++ != 0)
			printf("iaddress > 2^24\n");
	}
	if(ip->i_flag&S5IACC)
		dp->di_atime = ta->tv_sec;
	if(ip->i_flag&S5IUPD)
		dp->di_mtime = tm->tv_sec;
	if(ip->i_flag&S5ICHG)
		dp->di_ctime = time.tv_sec;
	syn = ip->i_flag&S5ISYN;
	ip->i_flag &= ~(S5IACC|S5IUPD|S5ICHG|S5ISYN);
	if(syn)
		bwrite(bp);
	else
		bdwrite(bp, bp->b_vp);
	return(0);

}

/*
 * Free all the disk blocks associated with the specified inode structure.
 * The blocks of the file are removed in reverse order. This FILO
 * algorithm will tend to maintain
 * a contiguous free list much longer than FIFO.
 */
s5itrunc(ip)
register struct s5inode *ip;
{
	register i;
	daddr_t bn;

	i = ip->i_mode & S5IFMT;
	if (i != S5IFREG && i != S5IFDIR)
		return(0);

	ASSERT(s5ILOCK_HOLDER(ip));
	for(i=NADDR-1; i>=0; i--) {
		bn = ip->i_addr[i];
		if(bn == (daddr_t)0)
			continue;
		ip->i_addr[i] = (daddr_t)0;
		switch(i) {

		default:
			s5free(ip, bn);
			break;

		case NADDR-3:
			tloop(ip, bn, 0, 0);
			break;

		case NADDR-2:
			tloop(ip, bn, 1, 0);
			break;

		case NADDR-1:
			tloop(ip, bn, 1, 1);
		}
	}
/**
	if (ip->i_map)
		freeblklst(ip);
**/
	ip->i_size = 0;
	ip->i_flag |= S5IUPD|S5ICHG;
	return(0);
}

tloop(ip, bn, f1, f2)
struct s5inode *ip;
daddr_t bn;
{
	register i;
	register daddr_t *bap;
	struct buf *bp;
	daddr_t nb;
	int error = 0;
	int bsize = FsBSIZE(ip->i_s5fs);

	bp = NULL;
	for(i=FsNINDIR(bsize)-1; i>=0; i--) {
		if(bp == NULL) {
			error = bread(ip->i_devvp,  FsLTOP(bsize, bn), 
				      bsize,  NOCRED, &bp);
			if (error) {
				brelse(bp);
				return;
			}
			bap = bp->b_un.b_daddr;
		}
		nb = bap[i];
		if(nb == (daddr_t)0)
			continue;
		if(f1) {
			brelse(bp);
			bp = NULL;
			tloop(ip, nb, f2, 0);
		} else
			s5free(ip, nb);
	}
	if(bp != NULL)
		brelse(bp);
	s5free(ip, bn);
}

/*
 * Check mode permission on inode pointer. Mode is READ, WRITE or EXEC.
 * The mode is shifted to select the owner/group/other fields. The
 * super user is granted all permissions.
 * Note: Checking  mounted file system being RDONLY or WTONLY is done at
 *       VFS level.
 */
s5iaccess(ip, mode, cred)
	register struct s5inode *ip;
	register int mode;
	struct ucred *cred;
{
	register gid_t *gp;
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
	if (cred->cr_uid != ip->i_uid) {
		mode >>= 3;
		if (ip->i_gid == cred->cr_gid)
			goto found;
		gp = cred->cr_groups;
		for (i = 0; i < cred->cr_ngroups; i++, gp++)
			if (ip->i_gid == *gp)
				goto found;
		mode >>= 3;
found:
		;
	}
	if ((ip->i_mode & mode) != 0)
		return (0);
	return (EACCES);
}

/*
 * Last reference to an inode, write the inode out and if necessary,
 * truncate and deallocate the file.
 */

s5fs_inactive(vp)
	struct vnode *vp;
{
	register struct s5inode *ip = S5VTOI(vp);
	int mode, error = 0;
	int mflag;
	struct mount *mp;

	if (prtactive && vp->v_usecount != 1)
		vprint("s5fs_inactive: pushing active", vp);
	mp = vp->v_mount;
	/*
	 * mp will be null only if there was an error in s5iget prior
	 * to the insmntque call.
	 */
	if (mp != DEADMOUNT) {
		BM(MOUNT_LOCK(mp));
		mflag = mp->m_flag;
		BM(MOUNT_UNLOCK(mp));
	}

	/*
	 * Get rid of inodes related to stale file handles.
	 */
	if ((ip->i_mode == 0 || mp == DEADMOUNT)) {
  		VN_LOCK(vp);
  		vgone(vp, VX_NOSLEEP, 0);
  		VN_UNLOCK(vp);
		return (0);
	}

	if (ip->i_nlink <= 0 && (mflag & M_RDONLY) == 0) {
		s5ILOCK(ip);
		error = s5itrunc(ip);
		s5IUNLOCK(ip);
		mode = ip->i_mode;
		ip->i_mode = 0;
		ip->i_rdev = 0;
		ip->i_flag |= S5IUPD|S5ICHG;
		s5ifree(ip, ip->i_number);

	}
	s5iupdat(ip, &time, &time);
	ip->i_flag = 0;
	/*
	 * If we are done with the inode, reclaim it
	 * so that it can be reused immediately.
	 */
        if (vp->v_usecount == 1 && ip->i_mode == 0) {
  		VN_LOCK(vp);
  		vgone(vp, VX_NOSLEEP, 0);
  		VN_UNLOCK(vp);
  	}
	return (error);
}



/*
 * Reclaim a device inode so that it can be used for other purposes.
 */

s5fsspec_reclaim(vp)
register struct vnode *vp;
{
	int error;
	if ((error = spec_reclaim(vp)) == 0)
		error = s5fs_reclaim(vp);
	return(error);
}

/*
 * Reclaim an inode so that it can be used for other purposes.
 */
s5fs_reclaim(vp)
	struct vnode *vp;
{
	struct s5inode *ip = S5VTOI(vp);

	if (prtactive && vp->v_usecount != 0)
		vprint("s5fs_reclaim: pushing active", vp);
	/*
	 * Remove the inode from its hash chain.
	 */
	remque(ip);
	ip->i_forw = ip;
	ip->i_back = ip;
	/*
	 * Purge old data structures associated with the inode.
	 */
	cache_purge(vp);
	if (ip->i_devvp) {
		vrele(ip->i_devvp);
		ip->i_devvp = 0;
	}
	ip->i_flag = 0;
	return (0);
}
