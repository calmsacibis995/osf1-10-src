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
static char	*sccsid = "@(#)$RCSfile: ufs_alloc.c,v $ $Revision: 4.3.13.2 $ (DEC) $Date: 1993/05/12 12:58:56 $";
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
 * OSF/1 Release 1.0.3
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
 *	@(#)ufs_alloc.c	7.15 (Berkeley) 12/30/89
 */
#if	MACH
#include <quota.h>
#endif

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#if	QUOTA
#include <ufs/quota.h>
#endif
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/fs_proto.h>
#include <ufs/ufs_ubc.h>

extern u_int		hashalloc();
extern ino_t		ialloccg();
extern daddr_t		alloccg();
extern daddr_t		alloccgblk();
extern daddr_t		fragextend();
extern daddr_t		blkpref();
extern daddr_t		mapsearch();
extern int		inside[], around[];
extern unsigned char	*fragtbl[];

#if	MACH_ASSERT
extern struct vnodeops ufs_vnodeops;
#endif

/*
 * Allocate a block in the file system.
 *
 * The size of the requested block is given, which must be some
 * multiple of fs_fsize and <= fs_bsize.
 * A preference may be optionally specified. If a preference is given
 * the following hierarchy is used to allocate a block:
 *   1) allocate the requested block.
 *   2) allocate a rotationally optimal block in the same cylinder.
 *   3) allocate a block in the same cylinder group.
 *   4) quadradically rehash into other cylinder groups, until an
 *      available block is located.
 * If no block preference is given the following heirarchy is used
 * to allocate a block:
 *   1) allocate a block in the cylinder group that contains the
 *      inode for the file.
 *   2) quadradically rehash into other cylinder groups, until an
 *      available block is located.
 *
 * Caller holds inode I/O lock for writing.
 */
alloc(ip, lbn, bpref, size, bnp)
	register struct inode *ip;
	daddr_t lbn, bpref;
	int size;
	daddr_t *bnp;
{
	daddr_t bno;
	register struct fs *fs;
	register struct buf *bp;
	int cg, error;
	struct ucred *cred = u.u_cred;		/* XXX */
	
	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	/*
	 * It is entirely possible that we will "see" free space
	 * when we enter this routine but that the subsequent
	 * allocation will fail.  Rather than try to lock the
	 * filesystem during this entire process, we simply let
	 * nature take its course -- as the original code did.
	 */
	*bnp = 0;
	fs = ip->i_fs;
	if ((unsigned)size > fs->fs_bsize || fragoff(fs, size) != 0) {
		printf("dev = 0x%x, bsize = %d, size = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, size, fs->fs_fsmnt);
		panic("alloc: bad size");
	}
	FS_LOCK(fs);
	if (size == fs->fs_bsize && fs->fs_cstotal.cs_nbfree == 0)
		goto nospace;
#if	SEC_BASE
	if (freespace(fs, fs->fs_minfree) <= 0 && !privileged(SEC_LIMIT, 0))
#else
	if (cred->cr_uid != 0 && freespace(fs, fs->fs_minfree) <= 0)
#endif
		goto nospace;
	FS_UNLOCK(fs);
#if	QUOTA
	if (error = chkdq(ip, (long)btodb(size), cred, 0))
		return (error);
#endif
	if (bpref >= fs->fs_size)
		bpref = 0;
	if (bpref == 0)
		cg = itog(fs, ip->i_number);
	else
		cg = dtog(fs, bpref);
	bno = (daddr_t)hashalloc(ip, cg, (int)bpref, size,
		(u_int (*)())alloccg);
	if (bno > 0) {
		ip->i_blocks += btodb(size);
		IN_LOCK(ip);
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		*bnp = bno;
		return (0);
	}
#if	QUOTA
	/*
	 * Restore user's disk quota because allocation failed
	 * after we put it on his tab.
	 */
	(void) chkdq(ip, (long)-btodb(size), cred, 0);
#endif
	goto out;
nospace:
	FS_UNLOCK(fs);
out:
	fserr(fs, "file system full");
	uprintf("\n%s: write failed, file system is full\n", fs->fs_fsmnt);
	return (ENOSPC);
}


/*
 * Reallocate a fragment to a bigger size
 *
 * The number and size of the old block is given, and a preference
 * and new size is also specified. The allocator attempts to extend
 * the original block. Failing that, the regular block allocator is
 * invoked to get an appropriate block.
 *
 * Caller holds inode I/O lock for writing.
 */
realloccg(ip, lbprev, bpref, osize, nsize, rep)
	register struct inode *ip;
	off_t lbprev;
	daddr_t bpref;
	int osize, nsize;
	struct ufs_realloc *rep;
{
	register struct fs *fs;
	struct buf *bp, *obp;
	int cg, request;
	daddr_t bprev, bno, bn;
	int i, error;
	int freesp;
	struct ucred *cred = u.u_cred;		/* XXX */
#if	QUOTA
	int quota_updated;
#endif
	
	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	if (rep->ur_flags ^ B_UBC)
		*(rep->ur_bpp) = 0;
	fs = ip->i_fs;
	if ((unsigned)osize > fs->fs_bsize || fragoff(fs, osize) != 0 ||
	    (unsigned)nsize > fs->fs_bsize || fragoff(fs, nsize) != 0) {
		printf("dev = 0x%x,bsize = %d,osize = %d,nsize = %d,fs = %s\n",
		    ip->i_dev, fs->fs_bsize, osize, nsize, fs->fs_fsmnt);
		panic("realloccg: bad size");
	}
#if	QUOTA
	quota_updated = 0;
#endif
#if	SEC_BASE
	BM(FS_LOCK(fs));
	freesp = freespace(fs, fs->fs_minfree);
	BM(FS_UNLOCK(fs));
	if (freesp <= 0 && !privileged(SEC_LIMIT, 0))
		goto nospace;
#else
	if (cred->cr_uid != 0) {
		BM(FS_LOCK(fs));
		freesp = freespace(fs, fs->fs_minfree);
		BM(FS_UNLOCK(fs));
		if (freesp <= 0)
			goto nospace;
	}
#endif
	if ((bprev = ip->i_db[lbprev]) == 0) {
		printf("dev = 0x%x, bsize = %d, bprev = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, bprev, fs->fs_fsmnt);
		panic("realloccg: bad bprev");
	}
#if	QUOTA
	if (error = chkdq(ip, (long)btodb(nsize - osize), cred, 0))
		return (error);
	quota_updated++;
#endif
	/*
	 * Allocate the extra space in the buffer.
	 */
	if (rep->ur_flags ^ B_UBC) {
		if (error = bread(ITOV(ip), lbprev, osize, NOCRED, &bp)) {
			brelse(bp);
			goto realloccg_error;
		}
		allocbuf(bp, nsize);	/* XXX brealloc, event_post XXX */
		bzero(bp->b_un.b_addr + osize, (unsigned)nsize - osize);
		LASSERT(BUF_LOCK_HOLDER(bp));
	
	}
	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(fs));
	/*
	 * Check for extension in the existing location.
	 */
	cg = dtog(fs, bprev);
	if (bno = fragextend(ip, cg, (int)bprev, osize, nsize)) {
		if (rep->ur_flags ^ B_UBC && bp->b_blkno != fsbtodb(fs, bno))
			panic("bad blockno");
		ip->i_blocks += btodb(nsize - osize);
		IN_LOCK(ip);
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		if (rep->ur_flags ^ B_UBC)
			*(rep->ur_bpp) = bp;
		else
			rep->ur_blkno = fsbtodb(fs, bno);
		return (0);
	}
	/*
	 * Allocate a new disk location.
	 */
	if (bpref >= fs->fs_size)
		bpref = 0;
	FS_LOCK(fs);
	switch ((int)fs->fs_optim) {
	      case FS_OPTSPACE:
		/*
		 * Allocate an exact sized fragment. Although this makes
		 * best use of space, we will waste time relocating it if
		 * the file continues to grow. If the fragmentation is
		 * less than half of the minimum free reserve, we choose
		 * to begin optimizing for time.
		 */
		request = nsize;
		if (fs->fs_minfree < 5 ||
		    fs->fs_cstotal.cs_nffree >
		    (long)fs->fs_dsize * fs->fs_minfree / (2 * 100)) {
			FS_UNLOCK(fs);
			break;
		}
		fs->fs_optim = FS_OPTTIME;
		FS_UNLOCK(fs);
		log(LOG_NOTICE,
		    "%s: optimization changed from SPACE to TIME\n",
		    fs->fs_fsmnt);
		break;
	      case FS_OPTTIME:
		/*
		 * At this point we have discovered a file that is trying
		 * to grow a small fragment to a larger fragment. To save
		 * time, we allocate a full sized block, then free the
		 * unused portion. If the file continues to grow, the
		 * `fragextend' call above will be able to grow it in place
		 * without further copying. If aberrant programs cause
		 * disk fragmentation to grow within 2% of the free reserve,
		 * we choose to begin optimizing for space.
		 */
		request = fs->fs_bsize;
		if (fs->fs_cstotal.cs_nffree <
		    (long)fs->fs_dsize * (fs->fs_minfree - 2) / 100) {
			FS_UNLOCK(fs);
			break;
		}
		fs->fs_optim = FS_OPTSPACE;
		FS_UNLOCK(fs);
		log(LOG_NOTICE,
		    "%s: optimization changed from TIME to SPACE\n",
		    fs->fs_fsmnt);
		break;
	      default:
		/*
		 *	File system has some bogus value in this field.
		 *	Fix it.
		 */
		fs->fs_optim = FS_OPTSPACE;
		FS_UNLOCK(fs);
		request = nsize;
		break;
	}
	LASSERT(!FS_LOCK_HOLDER(fs));
	bno = (daddr_t)hashalloc(ip, cg, (int)bpref, request,
		(u_int (*)())alloccg);
	LASSERT(IN_WRITE_HOLDER(ip));
	if (bno > 0) {
		bn = fsbtodb(fs, bno);
		blkfree(ip, bprev, (off_t)osize);
		if (rep->ur_flags ^ B_UBC) {
			bp->b_blkno = bn;
			*(rep->ur_bpp) = bp;
		}
		else
			rep->ur_blkno = bn;
		if (nsize < request)
			blkfree(ip, bno + numfrags(fs, nsize),
				(off_t)(request - nsize));
		ip->i_blocks += btodb(nsize - osize);
		IN_LOCK(ip);
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		LASSERT((rep->ur_flags & B_UBC) || BUF_LOCK_HOLDER(bp));
		LASSERT(IN_WRITE_HOLDER(ip));
		LASSERT(!FS_LOCK_HOLDER(fs));
		return (0);
	}
	if (rep->ur_flags ^ B_UBC) {
		if ((osize != nsize) && (bp->b_bcount == nsize))
			allocbuf(bp, osize); /* XXX brealloc, event_post XXX */
		brelse(bp);
	}
nospace:
	/*
	 * no space available
	 */
	fserr(fs, "file system full");
	uprintf("\n%s: write failed, file system is full\n", fs->fs_fsmnt);
	error = ENOSPC;
realloccg_error:
#if	QUOTA
	if (quota_updated)
		(void) chkdq(ip, (long)-btodb(nsize - osize), cred, 0);
#endif
	return (error);
}

/*
 * Allocate an inode in the file system.
 *
 * A preference may be optionally specified. If a preference is given
 * the following hierarchy is used to allocate an inode:
 *   1) allocate the requested inode.
 *   2) allocate an inode in the same cylinder group.
 *   3) quadradically rehash into other cylinder groups, until an
 *      available inode is located.
 * If no inode preference is given the following heirarchy is used
 * to allocate an inode:
 *   1) allocate an inode in cylinder group 0.
 *   2) quadradically rehash into other cylinder groups, until an
 *      available inode is located.
 *
 * Caller usually holds no locks on pip, although this is not a requirement.
 */
ialloc(pip, ipref, mode, ipp)
	register struct inode *pip;
	ino_t ipref;
	int mode;
	struct inode **ipp;
{
	ino_t ino;
	register struct fs *fs;
	register struct inode *ip;
	int cg, error;
	int csnifree;

	LASSERT(!FS_LOCK_HOLDER(pip->i_fs));
	*ipp = 0;
	fs = pip->i_fs;
	BM(FS_LOCK(fs));
	csnifree = fs->fs_cstotal.cs_nifree;
	BM(FS_UNLOCK(fs));
	if (csnifree == 0)
		goto noinodes;
	if (ipref >= fs->fs_ncg * fs->fs_ipg)
		ipref = 0;
	cg = itog(fs, ipref);
	ino = (ino_t)hashalloc(pip, cg, (int)ipref, mode, ialloccg);
	if (ino == 0)
		goto noinodes;
	error = iget(pip, ino, ipp, 0);
	if (error) {
		ifree(pip, ino, 0);
		return (error);
	}
	ip = *ipp;
	ASSERT(ITOV(ip)->v_op == &ufs_vnodeops);
	/*
	 * No one else knows about this inode yet so there's no need
	 * to take any inode locks.
	 */
	if (ip->i_mode) {
		printf("mode = 0%o, inum = %d, pref = %d fs = %s\n",
		    ip->i_mode, ip->i_number, ipref, fs->fs_fsmnt);
		panic("ialloc: dup alloc");
	}
	if (ip->i_blocks) {				/* XXX */
		printf("free inode %s/%d had %d blocks\n",
		    fs->fs_fsmnt, ino, ip->i_blocks);
		ip->i_blocks = 0;
	}
	/*
	 *	i_flags holds fast link bit.
	 */
	ip->i_flags = 0;
#if	SEC_FSCHANGE
	sec_ialloc(ip, pip);
#endif
	/*
	 * Set up a new generation number for this inode.
	 */
	ip->i_gen = get_nextgen();
	return (0);
noinodes:
	fserr(fs, "out of inodes");
	uprintf("\n%s: create/symlink failed, no inodes free\n", fs->fs_fsmnt);
	return (ENOSPC);
}

/*
 * Find a cylinder to place a directory.
 *
 * The policy implemented by this algorithm is to select from
 * among those cylinder groups with above the average number of
 * free inodes, the one with the smallest number of directories.
 */
ino_t
dirpref(fs)
	register struct fs *fs;
{
	int cg, minndir, mincg, avgifree;

	BM(FS_LOCK(fs));
	avgifree = fs->fs_cstotal.cs_nifree / fs->fs_ncg;
	BM(FS_UNLOCK(fs));
	minndir = fs->fs_ipg;
	mincg = 0;
	for (cg = 0; cg < fs->fs_ncg; cg++) {
		/*
		 * If there's a contention problem on the fs_lock,
		 * recoding the lock as a non-blocking read/write
		 * lock might help for loops like this.
		 */
		BM(FS_LOCK(fs));
		if (fs->fs_cs(fs, cg).cs_ndir < minndir &&
		    fs->fs_cs(fs, cg).cs_nifree >= avgifree) {
			mincg = cg;
			minndir = fs->fs_cs(fs, cg).cs_ndir;
		}
		BM(FS_UNLOCK(fs));
	}
	return ((ino_t)(fs->fs_ipg * mincg));
}

/*
 * Select the desired position for the next block in a file.  The file is
 * logically divided into sections. The first section is composed of the
 * direct blocks. Each additional section contains fs_maxbpg blocks.
 *
 * If no blocks have been allocated in the first section, the policy is to
 * request a block in the same cylinder group as the inode that describes
 * the file. If no blocks have been allocated in any other section, the
 * policy is to place the section in a cylinder group with a greater than
 * average number of free blocks.  An appropriate cylinder group is found
 * by using a rotor that sweeps the cylinder groups. When a new group of
 * blocks is needed, the sweep begins in the cylinder group following the
 * cylinder group from which the previous allocation was made. The sweep
 * continues until a cylinder group with greater than the average number
 * of free blocks is found. If the allocation is for the first block in an
 * indirect block, the information on the previous allocation is unavailable;
 * here a best guess is made based upon the logical block number being
 * allocated.
 *
 * If a section is already partially allocated, the policy is to
 * contiguously allocate fs_maxcontig blocks.  The end of one of these
 * contiguous blocks and the beginning of the next is physically separated
 * so that the disk head will be in transit between them for at least
 * fs_rotdelay milliseconds.  This is to allow time for the processor to
 * schedule another I/O transfer.
 *
 * Caller holds the inode I/O lock for writing.  Bap is presumed to point
 * to an array of daddr_t's that is either NULL; the inode's direct blocks;
 * or one of the inode's indirect blocks, all of which are protected by the
 * inode I/O lock.
 */

int	blkpref_fails = 0;
vdecl_simple_lock_data(,blkpref_handy_lock)

daddr_t
blkpref(ip, lbn, indx, bap)
	struct inode *ip;
	daddr_t lbn;
	int indx;
	daddr_t *bap;
{
	register struct fs *fs;
	register int cg;
	int avgbfree, startcg;
	daddr_t nextblk;
	extern int ufs_blkpref_lookbehind;
	int lookbehind = 1;

	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	fs = ip->i_fs;
	/*
	 * If we are writing at least the second block of an indirect block,
	 * try to locate the closest block, within ufs_blkpref_lookbehind
	 * blocks.
	 */
	if (ufs_blkpref_lookbehind > 1 && lbn >= NDADDR && bap &&
	    indx > 0) {
		int floor;
		int ind;

		if (indx <= ufs_blkpref_lookbehind)
			floor = 0;
		else
			floor = indx - ufs_blkpref_lookbehind;
		/*
		 * Start at the block immediatly preceeding the block
		 * we want to allocate, and search until we find an
		 * allocated block, or until ufs_blkpref_lookbehind
		 * blocks have been searched.
		 */
		for (ind = indx - 1, lookbehind = 1; ind >= floor; 
		     ind--, lookbehind++)
			if (bap[ind] != 0)
				break;
		if (ind < floor)
			lookbehind = 1;
	}
	/*
	 * At this point, lookbehind is either set to one, meaning that
	 * no blocks within ufs_blkpref_lookbehind blocks preceeding
	 * block we want to allcate are currently allocated. Or if
	 * lookbehind > 1, bap[indx - lookbehind] is allocated, and
	 * we should determine our preference from that block. The former
	 * case will drop into the following if, the latter will jump
	 * around the if.
	 */
	if (indx % fs->fs_maxbpg == 0 || bap[indx - lookbehind] == 0) {
		if (lbn < NDADDR) {
			cg = itog(fs, ip->i_number);
			return (fs->fs_fpg * cg + fs->fs_frag);
		}
		/*
		 * Find a cylinder group with greater than average number of
		 * unused data blocks.
		 */
		if (indx == 0 || bap[indx - lookbehind] == 0)
			startcg = itog(fs, ip->i_number) + lbn / fs->fs_maxbpg;
		else
			startcg = dtog(fs, bap[indx - lookbehind]) + 1;
		startcg %= fs->fs_ncg;
		BM(FS_LOCK(fs));
		avgbfree = fs->fs_cstotal.cs_nbfree / fs->fs_ncg;
		BM(FS_UNLOCK(fs));
		for (cg = startcg; cg < fs->fs_ncg; cg++) {
			BM(FS_LOCK(fs));
			if (fs->fs_cs(fs, cg).cs_nbfree >= avgbfree) {
				NM(FS_LOCK(fs));
				fs->fs_cgrotor = cg;
				FS_UNLOCK(fs);
				return (fs->fs_fpg * cg + fs->fs_frag);
			}
			BM(FS_UNLOCK(fs));
		}
		for (cg = 0; cg <= startcg; cg++) {
			BM(FS_LOCK(fs));
			if (fs->fs_cs(fs, cg).cs_nbfree >= avgbfree) {
				NM(FS_LOCK(fs));
				fs->fs_cgrotor = cg;
				FS_UNLOCK(fs);
				return (fs->fs_fpg * cg + fs->fs_frag);
			}
			BM(FS_UNLOCK(fs));
		}
		/*
		 * We could have done the above search under lock,
		 * but generally speaking we should be able to find
		 * a cylinder with avgbfree even while other allocations
		 * are happening.  If the search fails, we could repeat
		 * it, holding the filesystem lock the second time around;
		 * but it's not unreasonable to just return "no preference".
		 * Nevertheless, we're interested in how many times we do
		 * just that
		 */
		STATS_ACTION(&blkpref_handy_lock, ++blkpref_fails);
		LASSERT(!FS_LOCK_HOLDER(fs));
		LASSERT(IN_WRITE_HOLDER(ip));
		return (NULL);
	}
	/*
	 * One or more previous blocks have been laid out. If less
	 * than fs_maxcontig previous blocks are contiguous, the
	 * next block is requested contiguously, otherwise it is
	 * requested rotationally delayed by fs_rotdelay milliseconds.
	 *
	 * lookbehind is set to the offset from indx in bap where a
	 * block is already allocated. Simply add (lookbehind * number
	 * of frags per block) to the block number of previosly allocated
	 * block, and we will have our preference.
	 */
	nextblk = bap[indx - lookbehind] + (lookbehind * fs->fs_frag);
	if (fs->fs_rotdelay != 0 && indx != 0 &&
	    indx % fs->fs_maxcontig == 0) {
		/*
		 * fs->fs_rotdelay != 0 && new block is on 
		 * fs_maxcontig boundry.
	 	 *
		 * Here we convert ms of delay to frags as:
		 * (frags) = (ms) * (rev/sec) * (sect/rev) /
		 *	((sect/frag) * (ms/sec))
		 * then round up to the next block.
		 */
		nextblk +=
		   roundup(fs->fs_rotdelay * fs->fs_rps * fs->fs_nsect /
		    (NSPF(fs) * 1000), fs->fs_frag);
	}
	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(fs));
	return (nextblk);
}

/*
 * Implement the cylinder overflow algorithm.
 *
 * The policy implemented by this algorithm is:
 *   1) allocate the block in its requested cylinder group.
 *   2) quadradically rehash on the cylinder group number.
 *   3) brute force search for a free block.
 *
 * Caller may or may not hold the inode's I/O lock for writing.
 * The cylinder group is always protected by its buffer's lock.
 */
/*VARARGS5*/
u_int
hashalloc(ip, cg, pref, size, allocator)
	struct inode *ip;
	int cg;
	int pref;
	int size;	/* size for data blocks, mode for inodes */
	u_int (*allocator)();
{
	register struct fs *fs;
	int result;
	int i, icg = cg;

	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	fs = ip->i_fs;
	/*
	 * 1: preferred cylinder group
	 */
	result = (*allocator)(ip, cg, pref, size);
	if (result)
		return (result);
	/*
	 * 2: quadratic rehash
	 */
	for (i = 1; i < fs->fs_ncg; i *= 2) {
		cg += i;
		if (cg >= fs->fs_ncg)
			cg -= fs->fs_ncg;
		result = (*allocator)(ip, cg, 0, size);
		if (result)
			return (result);
	}
	/*
	 * 3: brute force search
	 * Note that we start at i == 2, since 0 was checked initially,
	 * and 1 is always checked in the quadratic rehash.
	 */
	cg = (icg + 2) % fs->fs_ncg;
	for (i = 2; i < fs->fs_ncg; i++) {
		result = (*allocator)(ip, cg, 0, size);
		if (result)
			return (result);
		cg++;
		if (cg == fs->fs_ncg)
			cg = 0;
	}
	return (NULL);
}

/*
 * Determine whether a fragment can be extended.
 *
 * Check to see if the necessary fragments are available, and
 * if they are, allocate them.
 *
 * Caller holds inode I/O lock for writing.  Cylinder group is
 * protected by its buffer's lock.
 */
daddr_t
fragextend(ip, cg, bprev, osize, nsize)
	struct inode *ip;
	int cg;
	int bprev;
	int osize, nsize;
{
	register struct fs *fs;
	register struct cg *cgp;
	struct buf *bp;
	int bno;
	int frags, bbase, csnffree, ofrags;
	int i, error, s;

	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	ASSERT(nsize > osize);

	fs = ip->i_fs;
	BM(FS_LOCK(fs));
	csnffree = fs->fs_cs(fs, cg).cs_nffree;
	BM(FS_UNLOCK(fs));
	if (csnffree < numfrags(fs, nsize - osize))
		return (NULL);
	frags = numfrags(fs, nsize);
	bbase = fragnum(fs, bprev);
	if (bbase > fragnum(fs, (bprev + frags - 1))) {
		/* cannot extend across a block boundary */
		return (NULL);
	}
	error = bread(ip->i_devvp, fsbtodb(fs, cgtod(fs, cg)),
		(int)fs->fs_cgsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return (NULL);
	}
#if	UNIX_LOCKS
	/*
	 * Recheck whether there are really enough fragments
	 * available.  Now that we hold the cylinder group's
	 * buffer, this count will not change unless we change it.
	 * Why wasn't this done for the uniprocessor case?  XXX
	 */
	BM(FS_LOCK(fs));
	csnffree = fs->fs_cs(fs, cg).cs_nffree;
	BM(FS_UNLOCK(fs));
	if (csnffree < numfrags(fs, nsize - osize)) {
		brelse(bp);
		return (NULL);
	}
#endif
	cgp = bp->b_un.b_cg;
	if (!cg_chkmagic(cgp)) {
		brelse(bp);
		return (NULL);
	}
	s = splhigh();
	TIME_READ_LOCK();
	cgp->cg_time = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	bno = dtogd(fs, bprev);
	LASSERT(BUF_LOCK_HOLDER(bp));
	ofrags = numfrags(fs, osize);
	for (i = ofrags; i < frags; i++)
		if (isclr(cg_blksfree(cgp), bno + i)) {
			brelse(bp);
			return (NULL);
		}
	/*
	 * the current fragment can be extended
	 * deduct the count on fragment being extended into
	 * increase the count on the remaining fragment (if any)
	 * allocate the extended piece
	 */
	LASSERT(BUF_LOCK_HOLDER(bp));
	for (i = frags; i < fs->fs_frag - bbase; i++)
		if (isclr(cg_blksfree(cgp), bno + i))
			break;
	cgp->cg_frsum[i - ofrags]--;
	if (i != frags)
		cgp->cg_frsum[i - frags]++;
	for (i = ofrags; i < frags; i++) {
		clrbit(cg_blksfree(cgp), bno + i);
		cgp->cg_cs.cs_nffree--;
		FS_LOCK(fs);
		fs->fs_cstotal.cs_nffree--;
		fs->fs_cs(fs, cg).cs_nffree--;
		fs->fs_fmod = 1;
		FS_UNLOCK(fs);
	}
	bdwrite(bp, bp->b_vp);
	return (bprev);
}

/*
 * Determine whether a block can be allocated.
 *
 * Check to see if a block of the apprpriate size is available,
 * and if it is, allocate it.
 *
 * Caller holds inode I/O lock for writing.  Cylinder group protected
 * by its buffer's lock.
 */
daddr_t
alloccg(ip, cg, bpref, size)
	struct inode *ip;
	int cg;
	daddr_t bpref;
	int size;
{
	register struct fs *fs;
	register struct cg *cgp;
	struct buf *bp;
	register int i;
	int error, bno, frags, allocsiz, s;
	int csnbfree;

	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	fs = ip->i_fs;
	BM(FS_LOCK(fs));
	csnbfree = fs->fs_cs(fs, cg).cs_nbfree;
	BM(FS_UNLOCK(fs));
	if (csnbfree == 0 && size == fs->fs_bsize)
		return (NULL);
	error = bread(ip->i_devvp, fsbtodb(fs, cgtod(fs, cg)),
		(int)fs->fs_cgsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return (NULL);
	}
	cgp = bp->b_un.b_cg;
	if (!cg_chkmagic(cgp) ||
	    (cgp->cg_cs.cs_nbfree == 0 && size == fs->fs_bsize)) {
		brelse(bp);
		return (NULL);
	}
	s = splhigh();
	TIME_READ_LOCK();
	cgp->cg_time = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	if (size == fs->fs_bsize) {
		bno = alloccgblk(fs, cgp, bpref);
		bdwrite(bp, bp->b_vp);
		return (bno);
	}
	/*
	 * check to see if any fragments are already available
	 * allocsiz is the size which will be allocated, hacking
	 * it down to a smaller size if necessary
	 */
	frags = numfrags(fs, size);
	for (allocsiz = frags; allocsiz < fs->fs_frag; allocsiz++)
		if (cgp->cg_frsum[allocsiz] != 0)
			break;
	if (allocsiz == fs->fs_frag) {
		/*
		 * no fragments were available, so a block will be
		 * allocated, and hacked up
		 */
		if (cgp->cg_cs.cs_nbfree == 0) {
			brelse(bp);
			return (NULL);
		}
		bno = alloccgblk(fs, cgp, bpref);
		bpref = dtogd(fs, bno);
		for (i = frags; i < fs->fs_frag; i++)
			setbit(cg_blksfree(cgp), bpref + i);
		i = fs->fs_frag - frags;
		cgp->cg_cs.cs_nffree += i;
		FS_LOCK(fs);
		fs->fs_cstotal.cs_nffree += i;
		fs->fs_cs(fs, cg).cs_nffree += i;
		fs->fs_fmod = 1;
		FS_UNLOCK(fs);
		cgp->cg_frsum[i]++;
		bdwrite(bp, bp->b_vp);
		return (bno);
	}
	bno = mapsearch(fs, cgp, bpref, allocsiz);
	if (bno < 0) {
		brelse(bp);
		return (NULL);
	}
	for (i = 0; i < frags; i++)
		clrbit(cg_blksfree(cgp), bno + i);
	cgp->cg_cs.cs_nffree -= frags;
	FS_LOCK(fs);
	fs->fs_cstotal.cs_nffree -= frags;
	fs->fs_cs(fs, cg).cs_nffree -= frags;
	fs->fs_fmod = 1;
	FS_UNLOCK(fs);
	cgp->cg_frsum[allocsiz]--;
	if (frags != allocsiz)
		cgp->cg_frsum[allocsiz - frags]++;
	bdwrite(bp, bp->b_vp);
	return (cg * fs->fs_fpg + bno);
}

/*
 * Allocate a block in a cylinder group.
 *
 * This algorithm implements the following policy:
 *   1) allocate the requested block.
 *   2) allocate a rotationally optimal block in the same cylinder.
 *   3) allocate the next available block on the block rotor for the
 *      specified cylinder group.
 * Note that this routine only allocates fs_bsize blocks; these
 * blocks may be fragmented by the routine that allocates them.
 *
 * Cylinder group protected by its buffer's lock.
 */
daddr_t
alloccgblk(fs, cgp, bpref)
	register struct fs *fs;
	register struct cg *cgp;
	daddr_t bpref;
{
	daddr_t bno;
	int cylno, pos, delta;
	short *cylbp;
	register int i;

	/*
	 * cgp's buffer should be locked, no easy way to check.  XXX
	 */
	LASSERT(!FS_LOCK_HOLDER(fs));
	if (bpref == 0) {
		bpref = cgp->cg_rotor;
		goto norot;
	}
	bpref = blknum(fs, bpref);
	bpref = dtogd(fs, bpref);
	/*
	 * if the requested block is available, use it
	 */
	if (isblock(fs, cg_blksfree(cgp), fragstoblks(fs, bpref))) {
		bno = bpref;
		goto gotit;
	}
	/*
	 * check for a block available on the same cylinder
	 */
	cylno = cbtocylno(fs, bpref);
	if (cg_blktot(cgp)[cylno] == 0)
		goto norot;
#ifdef	multimax
	/*
	 * block layout info is not available, so just have
	 * to take any block in this cylinder.
	 */
	bpref = howmany(fs->fs_spc * cylno, NSPF(fs));
#else	/* multimax */
	if (fs->fs_cpc == 0) {
		/*
		 * block layout info is not available, so just have
		 * to take any block in this cylinder.
		 */
		bpref = howmany(fs->fs_spc * cylno, NSPF(fs));
		goto norot;
	}
	/*
	 * check the summary information to see if a block is
	 * available in the requested cylinder starting at the
	 * requested rotational position and proceeding around.
	 */
	cylbp = cg_blks(fs, cgp, cylno);
	pos = cbtorpos(fs, bpref);
	for (i = pos; i < fs->fs_nrpos; i++)
		if (cylbp[i] > 0)
			break;
	if (i == fs->fs_nrpos)
		for (i = 0; i < pos; i++)
			if (cylbp[i] > 0)
				break;
	if (cylbp[i] > 0) {
		/*
		 * found a rotational position, now find the actual
		 * block. A panic if none is actually there.
		 */
		pos = cylno % fs->fs_cpc;
		bno = (cylno - pos) * fs->fs_spc / NSPB(fs);
		if (fs_postbl(fs, pos)[i] == -1) {
			printf("pos = %d, i = %d, fs = %s\n",
			    pos, i, fs->fs_fsmnt);
			panic("alloccgblk: cyl groups corrupted");
		}
		for (i = fs_postbl(fs, pos)[i];; ) {
			if (isblock(fs, cg_blksfree(cgp), bno + i)) {
				bno = blkstofrags(fs, (bno + i));
				goto gotit;
			}
			delta = fs_rotbl(fs)[i];
			if (delta <= 0 ||
			    delta + i > fragstoblks(fs, fs->fs_fpg))
				break;
			i += delta;
		}
		printf("pos = %d, i = %d, fs = %s\n", pos, i, fs->fs_fsmnt);
		panic("alloccgblk: can't find blk in cyl");
	}
#endif	/* multimax */
norot:
	/*
	 * no blocks in the requested cylinder, so take next
	 * available one in this cylinder group.
	 */
	bno = mapsearch(fs, cgp, bpref, (int)fs->fs_frag);
	if (bno < 0)
		return (NULL);
	cgp->cg_rotor = bno;
gotit:
	clrblock(fs, cg_blksfree(cgp), (int)fragstoblks(fs, bno));
	cgp->cg_cs.cs_nbfree--;
	FS_LOCK(fs);
	fs->fs_cstotal.cs_nbfree--;
	fs->fs_cs(fs, cgp->cg_cgx).cs_nbfree--;
	fs->fs_fmod = 1;
	FS_UNLOCK(fs);
	cylno = cbtocylno(fs, bno);
	cg_blks(fs, cgp, cylno)[cbtorpos(fs, bno)]--;
	cg_blktot(cgp)[cylno]--;
	return (cgp->cg_cgx * fs->fs_fpg + bno);
}

/*
 * Determine whether an inode can be allocated.
 *
 * Check to see if an inode is available, and if it is,
 * allocate it using the following policy:
 *   1) allocate the requested inode.
 *   2) allocate the next available inode after the requested
 *      inode in the specified cylinder group.
 *
 * Caller may or may not hold inode I/O lock for writing.  Cylinder
 * group protected by its buffer's lock.
 */
ino_t
ialloccg(ip, cg, ipref, mode)
	struct inode *ip;
	int cg;
	daddr_t ipref;
	int mode;
{
	register struct fs *fs;
	register struct cg *cgp;
	struct buf *bp;
	int error, start, len, loc, map, i, s;
	int csnifree;

	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	fs = ip->i_fs;
	BM(FS_LOCK(fs));
	csnifree = fs->fs_cs(fs, cg).cs_nifree;
	BM(FS_UNLOCK(fs));
	if (csnifree == 0)
		return (NULL);
	error = bread(ip->i_devvp, fsbtodb(fs, cgtod(fs, cg)),
		(int)fs->fs_cgsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return (NULL);
	}
	cgp = bp->b_un.b_cg;
	if (!cg_chkmagic(cgp) || cgp->cg_cs.cs_nifree == 0) {
		brelse(bp);
		return (NULL);
	}
	s = splhigh();
	TIME_READ_LOCK();
	cgp->cg_time = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	if (ipref) {
		ipref %= fs->fs_ipg;
		if (isclr(cg_inosused(cgp), ipref))
			goto gotit;
	}
	start = cgp->cg_irotor / NBBY;
	len = howmany(fs->fs_ipg - cgp->cg_irotor, NBBY);
	loc = skpc(0xff, len, &cg_inosused(cgp)[start]);
	if (loc == 0) {
		len = start + 1;
		start = 0;
		loc = skpc(0xff, len, &cg_inosused(cgp)[0]);
		if (loc == 0) {
#if	SEC_FSCHANGE
			/*
			 * number of inodes in a cyl group may not fit
			 * exactly into the byte array.  Check last byte.
			 */
			len = fs->fs_ipg % NBBY;
			if(len) {
				loc = howmany(fs->fs_ipg, NBBY) - 1;
				map = cg_inosused(cgp)[loc];
				ipref = loc * NBBY;
				for(i=1; i < (1 << len); i <<= 1, ipref++) {
					if((map & i) == 0) {
						cgp->cg_irotor = ipref;
						goto gotit;
					}
				}
			}
#endif
			printf("cg = %s, irotor = %d, fs = %s\n",
			    cg, cgp->cg_irotor, fs->fs_fsmnt);
			panic("ialloccg: map corrupted");
			/* NOTREACHED */
		}
	}
	i = start + len - loc;
	map = cg_inosused(cgp)[i];
	ipref = i * NBBY;
	for (i = 1; i < (1 << NBBY); i <<= 1, ipref++) {
		if ((map & i) == 0) {
			cgp->cg_irotor = ipref;
			goto gotit;
		}
	}
	printf("fs = %s\n", fs->fs_fsmnt);
	panic("ialloccg: block not in map");
	/* NOTREACHED */
gotit:
	setbit(cg_inosused(cgp), ipref);
	cgp->cg_cs.cs_nifree--;
	FS_LOCK(fs);
	fs->fs_cstotal.cs_nifree--;
	fs->fs_cs(fs, cg).cs_nifree--;
	fs->fs_fmod = 1;
	if ((mode & IFMT) == IFDIR) {
		cgp->cg_cs.cs_ndir++;
		fs->fs_cstotal.cs_ndir++;
		fs->fs_cs(fs, cg).cs_ndir++;
	}
	FS_UNLOCK(fs);
	bdwrite(bp, bp->b_vp);
	return (cg * fs->fs_ipg + ipref);
}

/*
 * Free a block or fragment.
 *
 * The specified block or fragment is placed back in the
 * free map. If a fragment is deallocated, a possible
 * block reassembly is checked.
 *
 * Caller holds inode I/O lock for writing.
 */
void
blkfree(ip, bno, size)
	register struct inode *ip;
	daddr_t bno;
	off_t size;
{
	register struct fs *fs;
	register struct cg *cgp;
	struct buf *bp;
	int error, cg, blk, frags, bbase;
	register int i, s;

	LASSERT(IN_WRITE_HOLDER(ip));
	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	fs = ip->i_fs;
	if ((unsigned)size > fs->fs_bsize || fragoff(fs, size) != 0) {
		printf("dev = 0x%x, bsize = %d, size = %d, fs = %s\n",
		    ip->i_dev, fs->fs_bsize, size, fs->fs_fsmnt);
		panic("blkfree: bad size");
	}
	cg = dtog(fs, bno);
	if (badblock(fs, bno)) {
		printf("bad block %d, ino %d\n", bno, ip->i_number);
		return;
	}
	error = bread(ip->i_devvp, fsbtodb(fs, cgtod(fs, cg)),
		(int)fs->fs_cgsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return;
	}
	cgp = bp->b_un.b_cg;
	if (!cg_chkmagic(cgp)) {
		brelse(bp);
		return;
	}
	s = splhigh();
	TIME_READ_LOCK();
	cgp->cg_time = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	bno = dtogd(fs, bno);
	if (size == fs->fs_bsize) {
		if (isblock(fs, cg_blksfree(cgp), fragstoblks(fs, bno))) {
			printf("dev = 0x%x, block = %d, fs = %s\n",
			    ip->i_dev, bno, fs->fs_fsmnt);
			panic("blkfree: freeing free block");
		}
		setblock(fs, cg_blksfree(cgp), fragstoblks(fs, bno));
		cgp->cg_cs.cs_nbfree++;
		FS_LOCK(fs);
		fs->fs_cstotal.cs_nbfree++;
		fs->fs_cs(fs, cg).cs_nbfree++;
		FS_UNLOCK(fs);
		i = cbtocylno(fs, bno);
		cg_blks(fs, cgp, i)[cbtorpos(fs, bno)]++;
		cg_blktot(cgp)[i]++;
	} else {
		bbase = bno - fragnum(fs, bno);
		/*
		 * decrement the counts associated with the old frags
		 */
		blk = blkmap(fs, cg_blksfree(cgp), bbase);
		fragacct(fs, blk, cgp->cg_frsum, -1);
		/*
		 * deallocate the fragment
		 */
		frags = numfrags(fs, size);
		for (i = 0; i < frags; i++) {
			if (isset(cg_blksfree(cgp), bno + i)) {
				printf("dev = 0x%x, block = %d, fs = %s\n",
				    ip->i_dev, bno + i, fs->fs_fsmnt);
#if	MACH
				printf("size = 0x%x, frags = 0x%x, i = 0x%x\n",
					size, frags, i);
#endif
				panic("blkfree: freeing free frag");
			}
			setbit(cg_blksfree(cgp), bno + i);
		}
		cgp->cg_cs.cs_nffree += i;
		FS_LOCK(fs);
		fs->fs_cstotal.cs_nffree += i;
		fs->fs_cs(fs, cg).cs_nffree += i;
		FS_UNLOCK(fs);
		/*
		 * add back in counts associated with the new frags
		 */
		blk = blkmap(fs, cg_blksfree(cgp), bbase);
		fragacct(fs, blk, cgp->cg_frsum, 1);
		/*
		 * if a complete block has been reassembled, account for it
		 */
		if (isblock(fs, cg_blksfree(cgp),
		    (daddr_t)fragstoblks(fs, bbase))) {
			cgp->cg_cs.cs_nffree -= fs->fs_frag;
			FS_LOCK(fs);
			fs->fs_cstotal.cs_nffree -= fs->fs_frag;
			fs->fs_cs(fs, cg).cs_nffree -= fs->fs_frag;
			cgp->cg_cs.cs_nbfree++;
			fs->fs_cstotal.cs_nbfree++;
			fs->fs_cs(fs, cg).cs_nbfree++;
			FS_UNLOCK(fs);
			i = cbtocylno(fs, bbase);
			cg_blks(fs, cgp, i)[cbtorpos(fs, bbase)]++;
			cg_blktot(cgp)[i]++;
		}
	}
	FS_LOCK(fs);
	fs->fs_fmod = 1;
	FS_UNLOCK(fs);
	bdwrite(bp, bp->b_vp);
	return;
}

/*
 * Free an inode.
 *
 * The specified inode is placed back in the free map.
 *
 * Caller need not hold any lock on the inode but may need
 * other synchronization depending on how the inode has been used.
 */
void
ifree(ip, ino, mode)
	struct inode *ip;
	ino_t ino;
	int mode;
{
	register struct fs *fs;
	register struct cg *cgp;
	struct buf *bp;
	int error, cg, s;

	LASSERT(!FS_LOCK_HOLDER(ip->i_fs));
	fs = ip->i_fs;
	if ((unsigned)ino >= fs->fs_ipg*fs->fs_ncg) {
		printf("dev = 0x%x, ino = %d, fs = %s\n",
		    ip->i_dev, ino, fs->fs_fsmnt);
		panic("ifree: range");
	}
	cg = itog(fs, ino);
	error = bread(ip->i_devvp, fsbtodb(fs, cgtod(fs, cg)),
		(int)fs->fs_cgsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		return;
	}
	cgp = bp->b_un.b_cg;
	if (!cg_chkmagic(cgp)) {
		brelse(bp);
		return;
	}
	s = splhigh();
	TIME_READ_LOCK();
	cgp->cg_time = time.tv_sec;
	TIME_READ_UNLOCK();
	splx(s);
	ino %= fs->fs_ipg;
	if (isclr(cg_inosused(cgp), ino)) {
		printf("ifree: freeing free inode\n");
		printf("dev = 0x%x, ino = %d, fs = %s\n",
		    ip->i_dev, ino, fs->fs_fsmnt);
#ifndef	multimax
		panic("ifree: freeing free inode");
#endif
		brelse(bp);
		return;
	}
	clrbit(cg_inosused(cgp), ino);
	if (ino < cgp->cg_irotor)
		cgp->cg_irotor = ino;
	cgp->cg_cs.cs_nifree++;
	FS_LOCK(fs);
	fs->fs_cstotal.cs_nifree++;
	fs->fs_cs(fs, cg).cs_nifree++;
	if ((mode & IFMT) == IFDIR) {
		cgp->cg_cs.cs_ndir--;
		fs->fs_cstotal.cs_ndir--;
		fs->fs_cs(fs, cg).cs_ndir--;
	}
	fs->fs_fmod = 1;
	FS_UNLOCK(fs);
	bdwrite(bp, bp->b_vp);
}

/*
 * Find a block of the specified size in the specified cylinder group.
 *
 * It is a panic if a request is made to find a block if none are
 * available.
 *
 * Caller holds cylinder group's buffer's lock.
 */
daddr_t
mapsearch(fs, cgp, bpref, allocsiz)
	register struct fs *fs;
	register struct cg *cgp;
	daddr_t bpref;
	int allocsiz;
{
	daddr_t bno;
	int start, len, loc, i;
	int blk, field, subfield, pos;

	LASSERT(!FS_LOCK_HOLDER(fs));
	/*
	 * find the fragment by searching through the free block
	 * map for an appropriate bit pattern
	 */
	if (bpref)
		start = dtogd(fs, bpref) / NBBY;
	else
		start = cgp->cg_frotor / NBBY;
	len = howmany(fs->fs_fpg, NBBY) - start;
	loc = scanc((unsigned)len, (u_char *)&cg_blksfree(cgp)[start],
		(u_char *)fragtbl[fs->fs_frag],
		(u_char)(1 << (allocsiz - 1 + (fs->fs_frag % NBBY))));
	if (loc == 0) {
		len = start + 1;
		start = 0;
		loc = scanc((unsigned)len, (u_char *)&cg_blksfree(cgp)[0],
			(u_char *)fragtbl[fs->fs_frag],
			(u_char)(1 << (allocsiz - 1 + (fs->fs_frag % NBBY))));
		if (loc == 0) {
			printf("start = %d, len = %d, fs = %s\n",
			    start, len, fs->fs_fsmnt);
			panic("alloccg: map corrupted");
			/* NOTREACHED */
		}
	}
	bno = (start + len - loc) * NBBY;
	cgp->cg_frotor = bno;
	/*
	 * found the byte in the map
	 * sift through the bits to find the selected frag
	 */
	for (i = bno + NBBY; bno < i; bno += fs->fs_frag) {
		blk = blkmap(fs, cg_blksfree(cgp), bno);
		blk <<= 1;
		field = around[allocsiz];
		subfield = inside[allocsiz];
		for (pos = 0; pos <= fs->fs_frag - allocsiz; pos++) {
			if ((blk & field) == subfield)
				return (bno + pos);
			field <<= 1;
			subfield <<= 1;
		}
	}
	printf("bno = %d, fs = %s\n", bno, fs->fs_fsmnt);
	panic("alloccg: block not in map");
	return (-1);
}

/*
 * Check that a specified block number is in range.
 */
badblock(fs, bn)
	register struct fs *fs;
	daddr_t bn;
{

	if ((unsigned)bn >= fs->fs_size) {
		printf("bad block %d, ", bn);
		fserr(fs, "bad block");
		return (1);
	}
	return (0);
}

/*
 * Fserr prints the name of a file system with an error diagnostic.
 *
 * The form of the error message is:
 *	fs: error message
 */
fserr(fs, cp)
	struct fs *fs;
	char *cp;
{

	log(LOG_ERR, "%s: %s\n", fs->fs_fsmnt, cp);
}
