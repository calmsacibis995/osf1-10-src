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
static char	*sccsid = "@(#)$RCSfile: ufs_bmap.c,v $ $Revision: 4.3.4.11 $ (DEC) $Date: 1992/12/11 15:21:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
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
 * OSF/1 Release 1.0
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
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/ufs_ubc.h>
#include <mach/machine/vm_types.h>
#include <sys/vfs_ubc.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <vm/heap_kmem.h>

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device
 * given the inode and the logical block number in a file.
 *
 * Bmap also returns the number of logically consecutive disk
 * blocks from 'bn'.
 */
bmap(ip, bn, bnp, cluster_sz)
	register struct inode *ip;
	register daddr_t bn;
	daddr_t	*bnp;
	int *cluster_sz;
{
	register struct fs *fs;
	register daddr_t nb;
	struct buf *bp;
	daddr_t *bap;
	int i, j, sh;
	int error;
	/* PRS - Cluster stack variables begin */
	daddr_t lbn;
	int ind, contig, maxcontig, consecreads;
	extern long cluster_maxcontig;
	extern int cluster_consec_incr;
	/* PRS - Cluster stack variables end */

	if (bn < 0)
		return (EFBIG);
	fs = ip->i_fs;
	/* PRS - Cluster variable initialization begin */
	if (cluster_sz) {
		int sv_maxcontig;

		contig = *cluster_sz = 0;
		lbn = bn;
		BM(IN_LOCK(ip));
		consecreads = ip->i_consecreads;
		BM(IN_UNLOCK(ip));
		maxcontig = MIN(fs->fs_maxcontig, cluster_maxcontig);
		sv_maxcontig = maxcontig;
		maxcontig = MIN(consecreads, maxcontig);
		if (consecreads + cluster_consec_incr < sv_maxcontig) {
			/*
			 * Current number of consecutive reads
			 * is less than the maximum cluster
			 * size.
			 */
			consecreads += cluster_consec_incr;
		} else if (consecreads < sv_maxcontig) {
			/*
			 * The number of consecutive reads now
			 * equal the maximum cluster size.
			 */
			consecreads = sv_maxcontig;
		}
		BM(IN_LOCK(ip));
		ip->i_consecreads = consecreads;
		BM(IN_UNLOCK(ip));
	}
	/* PRS - Cluster variable initialization end */

	/*
	 * The first NDADDR blocks are direct blocks
	 */
	if (bn < NDADDR) {
		BM(IN_LOCK(ip));
		nb = ip->i_db[bn];
		BM(IN_UNLOCK(ip));
		if (nb == 0) {
			*bnp = (daddr_t)-1;
			return (0);
		}
		*bnp = fsbtodb(fs, nb);
		/* PRS - Cluster sizing code for direct blocks begin */
		if (cluster_sz) {
			int prev_bn;
			daddr_t tmp_bn;

			BM(IN_LOCK(ip));
			prev_bn = ip->i_db[lbn];
			BM(IN_UNLOCK(ip));
			if (lbn < NDADDR && prev_bn) {
				contig = 1;
				for (ind = lbn + 1;
				     ind < NDADDR && contig < maxcontig;
				     ind++) {
					BM(IN_LOCK(ip));
					tmp_bn = ip->i_db[ind];
					BM(IN_UNLOCK(ip));
					if (prev_bn + fs->fs_frag != tmp_bn ||
					    blksize(fs, ip, ind) !=
					    fs->fs_bsize)
						break;
					contig++;
					prev_bn = tmp_bn;
				}
				goto out;
			}
		}
		/* PRS - Cluster sizing code for direct blocks end */
		return (0);
	}
	/*
	 * Determine the number of levels of indirection.
	 */
	sh = 1;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0)
		return (EFBIG);
	/*
	 * Fetch through the indirect blocks.
	 */
	BM(IN_LOCK(ip));
	nb = ip->i_ib[NIADDR - j];
	BM(IN_UNLOCK(ip));
	if (nb == 0) {
		*bnp = (daddr_t)-1;
		return (0);
	}
	for (; j <= NIADDR; j++) {
		if (error = bread(ip->i_devvp, fsbtodb(fs, nb),
		    (int)fs->fs_bsize, NOCRED, &bp)) {
			brelse(bp);
			return (error);
		}
		bap = bp->b_un.b_daddr;
		sh /= NINDIR(fs);
		i = (bn / sh) % NINDIR(fs);
		nb = bap[i];
		if (nb == 0) {
			*bnp = (daddr_t)-1;
			brelse(bp);
			return (0);
		}
		/* PRS - Cluster sizing code for indirect blocks begin */
		if (j == NIADDR && cluster_sz) {
			int prev_bn = bap[i];
			daddr_t tmp_bn;

			if (prev_bn) {
				contig = 1;
				for (ind = i + 1;
				     ind < NINDIR(fs) && contig < maxcontig;
				     ind++) {
					tmp_bn = bap[ind];
					if (prev_bn + fs->fs_frag != tmp_bn)
						break;
					contig++;
					prev_bn = tmp_bn;
				}
			}
		}
		/* PRS - Cluster sizing code for indirect blocks end */
		brelse(bp);
	}
	*bnp = fsbtodb(fs, nb);
out:
	if (cluster_sz) {
		BM(IN_LOCK(ip));
		ip->i_clusterlbn = lbn;
		ip->i_clustersz = contig;
		BM(IN_UNLOCK(ip));
		*cluster_sz = contig;
	}
	return (0);
}

/*
 * Balloc defines the structure of file system storage
 * by allocating the physical blocks on a device given
 * the inode and the logical block number in a file.
 */
balloc(ip, bn, size, bpp, flags)
	register struct inode *ip;
	register daddr_t bn;
#if	MACH
	u_int size;
#else
	int size;
#endif
	struct buf **bpp;
	int flags;
{
	register struct fs *fs;
	register daddr_t nb;
	struct buf *bp, *nbp;
	struct vnode *vp = ITOV(ip);
	u_int osize, nsize;
	int i, j, sh, error;
	daddr_t newb, lbn, *bap, pref, blkpref();
	struct ufs_realloc ufs_realloc;

	ufs_realloc.ur_flags = 0;
	ufs_realloc.ur_bpp = &bp;

	LASSERT(IN_WRITE_HOLDER(ip));
	*bpp = (struct buf *)0;
	if (bn < 0)
		return (EFBIG);
	fs = ip->i_fs;

	/*
	 * Writing a hole in a file, and if the last direct block
	 * is a fragment, extend it to be a full block. - prs
	 *
	 * If the next write will extend the file into a new block,
	 * and the file is currently composed of a fragment
	 * this fragment has to be extended to be a full block.
	 */
	BM(IN_LOCK(ip));
	nb = lblkno(fs, ip->i_size);
	BM(IN_UNLOCK(ip));
	if (nb < NDADDR && nb < bn) {
		osize = blksize(fs, ip, nb);
		if (osize < fs->fs_bsize && osize > 0) {
			error = realloccg(ip, nb,
				blkpref(ip, nb, (int)nb, &ip->i_db[0]),
				osize, (int)fs->fs_bsize, &ufs_realloc);
			if (error)
				return (error);
			IN_LOCK(ip);
			ip->i_size = (nb + 1) * fs->fs_bsize;
			ip->i_db[nb] = dbtofsb(fs, bp->b_blkno);
			ip->i_flag |= IUPD|ICHG;
			IN_UNLOCK(ip);
			if (flags & B_SYNC)
				bwrite(bp);
			else
				bawrite(bp);
		}
	}
	/*
	 * The first NDADDR blocks are direct blocks
	 */
	if (bn < NDADDR) {
		BM(IN_LOCK(ip));
		nb = ip->i_db[bn];
		if (nb != 0 && ip->i_size >= (bn + 1) * fs->fs_bsize) {
			BM(IN_UNLOCK(ip));
			error = bread(vp, bn, fs->fs_bsize, NOCRED, &bp);
			if (error) {
				brelse(bp);
				return (error);
			}
			*bpp = bp;
			return (0);
		} else
			BM(IN_UNLOCK(ip));
		if (nb != 0) {
			/*
			 * Consider need to reallocate a fragment.
			 */
			BM(IN_LOCK(ip));
			osize = fragroundup(fs, blkoff(fs, ip->i_size));
			BM(IN_UNLOCK(ip));
			nsize = fragroundup(fs, size);
			if (nsize <= osize) {
				error = bread(vp, bn, osize, NOCRED, &bp);
				if (error) {
					brelse(bp);
					return (error);
				}
			} else {
				error = realloccg(ip, bn,
					blkpref(ip, bn, (int)bn, &ip->i_db[0]),
					osize, nsize, &ufs_realloc);
				if (error)
					return (error);
			}
		} else {
			BM(IN_LOCK(ip));
			if (ip->i_size < (bn + 1) * fs->fs_bsize)
				nsize = fragroundup(fs, size);
			else
				nsize = fs->fs_bsize;
			BM(IN_UNLOCK(ip));
			error = alloc(ip, bn,
				blkpref(ip, bn, (int)bn, &ip->i_db[0]),
				nsize, &newb);
			if (error)
				return (error);
			bp = getblk(vp, bn, nsize);
			bp->b_blkno = fsbtodb(fs, newb);
			if (flags & B_CLRBUF)
				clrbuf(bp);
		}
		IN_LOCK(ip);
		ip->i_db[bn] = dbtofsb(fs, bp->b_blkno);
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		*bpp = bp;
		return (0);
	}
	/*
	 * Determine the number of levels of indirection.
	 */
	pref = 0;
	sh = 1;
	lbn = bn;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0)
		return (EFBIG);
	/*
	 * Fetch the first indirect block allocating if necessary.
	 */
	BM(IN_LOCK(ip));
	nb = ip->i_ib[NIADDR - j];
	BM(IN_UNLOCK(ip));
	if (nb == 0) {
		pref = blkpref(ip, lbn, 0, (daddr_t *)0);
	        if (error = alloc(ip, lbn, pref, (int)fs->fs_bsize, &newb))
			return (error);
		nb = newb;
		bp = getblk(ip->i_devvp, fsbtodb(fs, nb), fs->fs_bsize);
		clrbuf(bp);
		/*
		 * Write synchronously so that indirect blocks
		 * never point at garbage.
		 */
		if (error = bwrite(bp)) {
			blkfree(ip, nb, fs->fs_bsize);
			return (error);
		}
		IN_LOCK(ip);
		ip->i_ib[NIADDR - j] = nb;
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
	}
	/*
	 * Fetch through the indirect blocks, allocating as necessary.
	 */
	for (; ; j++) {
		error = bread(ip->i_devvp, fsbtodb(fs, nb),
		    (int)fs->fs_bsize, NOCRED, &bp);
		if (error) {
			brelse(bp);
			return (error);
		}
		bap = bp->b_un.b_daddr;
		sh /= NINDIR(fs);
		i = (bn / sh) % NINDIR(fs);
		nb = bap[i];
		if (j == NIADDR)
			break;
		if (nb != 0) {
			brelse(bp);
			continue;
		}
		if (pref == 0)
			pref = blkpref(ip, lbn, 0, (daddr_t *)0);
		if (error = alloc(ip, lbn, pref, (int)fs->fs_bsize, &newb)) {
			brelse(bp);
			return (error);
		}
		nb = newb;
		nbp = getblk(ip->i_devvp, fsbtodb(fs, nb), fs->fs_bsize);
		clrbuf(nbp);
		/*
		 * Write synchronously so that indirect blocks
		 * never point at garbage.
		 */
		if (error = bwrite(nbp)) {
			blkfree(ip, nb, fs->fs_bsize);
			brelse(bp);
			return (error);
		}
		bap[i] = nb;
		/*
		 * If required, write synchronously, otherwise use
		 * delayed write. If this is the first instance of
		 * the delayed write, reassociate the buffer with the
		 * file so it will be written if the file is sync'ed.
		 */
		if (flags & B_SYNC)
			bwrite(bp);
		else
			bdwrite(bp, vp);
	}
	/*
	 * Get the data block, allocating if necessary.
	 */
	if (nb == 0) {
		pref = blkpref(ip, lbn, i, &bap[0]);
		if (error = alloc(ip, lbn, pref, (int)fs->fs_bsize, &newb)) {
			brelse(bp);
			return (error);
		}
		nb = newb;
		nbp = getblk(vp, lbn, fs->fs_bsize);
		nbp->b_blkno = fsbtodb(fs, nb);
		if (flags & B_CLRBUF)
			clrbuf(nbp);
		IN_LOCK(ip);
		bap[i] = nb;
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		/*
		 * If required, write synchronously, otherwise use
		 * delayed write. If this is the first instance of
		 * the delayed write, reassociate the buffer with the
		 * file so it will be written if the file is sync'ed.
		 */
		if (flags & B_SYNC)
			bwrite(bp);
		else
			bdwrite(bp, vp);
		*bpp = nbp;
		return (0);
	}
	brelse(bp);
	if (flags & B_CLRBUF) {
		error = bread(vp, lbn, (int)fs->fs_bsize, NOCRED, &nbp);
		if (error) {
			brelse(nbp);
			return (error);
		}
	} else {
		nbp = getblk(vp, lbn, fs->fs_bsize);
		nbp->b_blkno = fsbtodb(fs, nb);
	}
	*bpp = nbp;
	return (0);
}

ufs_ubcbmap(ip, bn, size, bpp, flags, allocated)
	register struct inode *ip;
	register daddr_t bn;
	u_int size;
	daddr_t *bpp;
	int flags;
	boolean_t *allocated;
{
	struct buf *bp;
	register struct fs *fs;
	register daddr_t nb;
	struct buf *nbp;
	struct vnode *vp = ITOV(ip);
	u_int osize, nsize;
	long i, j, sh;
	int error;
	daddr_t newb, lbn, *bap, pref, blkpref(), rbp;
	vm_page_t pl;

	if (bn < 0)
		return (EFBIG);
	fs = ip->i_fs;
	*allocated = FALSE;

	/*
	 * Writing a hole in a file, and if the last direct block
	 * is a fragment, extend it to be a full block. - prs
	 *
	 * If the next write will extend the file into a new block,
	 * and the file is currently composed of a fragment
	 * this fragment has to be extended to be a full block.
	 */
	BM(IN_LOCK(ip));
	nb = lblkno(fs, ip->i_size);
	BM(IN_UNLOCK(ip));
	if ((flags & B_READ) == 0 && nb < NDADDR && nb < bn) {
		osize = blksize(fs, ip, nb);
		if (osize < fs->fs_bsize && osize > 0) {

			error = ufs_ubcrealloc(ip, nb, osize, 
					fs->fs_bsize, &rbp, &pl); 
			if (error) return error;
			IN_LOCK(ip);
			ip->i_size = (nb + 1) * fs->fs_bsize;
			ip->i_db[nb] = dbtofsb(fs, rbp);
			ip->i_flag |= IUPD|ICHG|ICHGMETA;
			IN_UNLOCK(ip);
			/*
			 * if synchronous operation is specified, then
			 * write out the new block synchronously, then
			 * update the inode to make sure it points to it
			 */
			ufs_ubcrealloc_done(ip, pl, flags & B_SYNC);
		}
	}

	/*
	 * The first NDADDR blocks are direct blocks
	 */

	if (bn < NDADDR) {
		BM(IN_LOCK(ip));
		nb = ip->i_db[bn];
		if (nb != 0 && ip->i_size >= (bn + 1) * fs->fs_bsize) {
			BM(IN_UNLOCK(ip));
			*bpp = fsbtodb(fs, nb);
			return 0;
		} else
			BM(IN_UNLOCK(ip));

		if (flags & B_READ) {
			*bpp = (nb) ? fsbtodb(fs, nb) : (daddr_t) -1;
			return 0;
		}

		if (nb != 0) {
			/*
			 * Consider need to reallocate a fragment.
			 */
			BM(IN_LOCK(ip));
			osize = fragroundup(fs, blkoff(fs, ip->i_size));
			BM(IN_UNLOCK(ip));
			nsize = fragroundup(fs, size);
			if (nsize <= osize) {
				*bpp = fsbtodb(fs, nb);
				return 0;
			} else {
				error = ufs_ubcrealloc(ip, bn, osize, 
						nsize, bpp, &pl);
				if (error)
					return error;
				IN_LOCK(ip);
				ip->i_db[bn] = dbtofsb(fs, *bpp);
				ip->i_size += (nsize - osize);
				ip->i_flag |= IUPD|ICHG|ICHGMETA;
				IN_UNLOCK(ip);
				ufs_ubcrealloc_done(ip, pl, 0);
				return (0);
			}
		} else {
			BM(IN_LOCK(ip));
			if (ip->i_size < (bn + 1) * fs->fs_bsize)
				nsize = fragroundup(fs, size);
			else
				nsize = fs->fs_bsize;
			BM(IN_UNLOCK(ip));
			error = alloc(ip, bn,
				blkpref(ip, bn, (int)bn, &ip->i_db[0]),
				nsize, &newb);
			if (error)
				return (error);
			*bpp = fsbtodb(fs, newb);
			*allocated = TRUE;
			IN_LOCK(ip);
			ip->i_db[bn] = dbtofsb(fs, *bpp);
			ip->i_flag |= IUPD|ICHG|ICHGMETA;
			IN_UNLOCK(ip);
			return (0);
		}
	}
	/*
	 * Determine the number of levels of indirection.
	 */
	pref = 0;
	sh = 1;
	lbn = bn;
	bn -= NDADDR;
	for (j = NIADDR; j > 0; j--) {
		sh *= NINDIR(fs);
		if (bn < sh)
			break;
		bn -= sh;
	}
	if (j == 0)
		return (EFBIG);
	/*
	 * Fetch the first indirect block allocating if necessary.
	 */
	BM(IN_LOCK(ip));
	nb = ip->i_ib[NIADDR - j];
	BM(IN_UNLOCK(ip));
	if (nb == 0) {
		if (flags & B_READ) {
			*bpp = (daddr_t) -1;
			return 0;
		}

		pref = blkpref(ip, lbn, 0, (daddr_t *)0);
	        if (error = alloc(ip, lbn, pref, (int)fs->fs_bsize, &newb))
			return (error);
		nb = newb;
		bp = getblk(ip->i_devvp, fsbtodb(fs, nb), fs->fs_bsize);
		clrbuf(bp);
		/*
		 * Write synchronously so that indirect blocks
		 * never point at garbage.
		 */
		if (error = bwrite(bp)) {
			blkfree(ip, nb, fs->fs_bsize);
			return (error);
		}
		IN_LOCK(ip);
		ip->i_ib[NIADDR - j] = nb;
		ip->i_flag |= IUPD|ICHG|ICHGMETA;
		IN_UNLOCK(ip);
	}
	/*
	 * Fetch through the indirect blocks, allocating as necessary.
	 */
	for (; ; j++) {
		error = bread(ip->i_devvp, fsbtodb(fs, nb),
		    (int)fs->fs_bsize, NOCRED, &bp);
		if (error) {
			brelse(bp);
			return (error);
		}
		bap = bp->b_un.b_daddr;
		sh /= NINDIR(fs);
		i = (bn / sh) % NINDIR(fs);
		nb = bap[i];
		if (j == NIADDR)
			break;
		if (nb != 0) {
			brelse(bp);
			continue;
		}
		else if (flags & B_READ) {
			brelse(bp);
			*bpp = (daddr_t) -1;
			return 0;
		}

		if (pref == 0)
			pref = blkpref(ip, lbn, 0, (daddr_t *)0);
		if (error = alloc(ip, lbn, pref, (int)fs->fs_bsize, &newb)) {
			brelse(bp);
			return (error);
		}
		nb = newb;
		nbp = getblk(ip->i_devvp, fsbtodb(fs, nb), fs->fs_bsize);
		clrbuf(nbp);
		/*
		 * Write synchronously so that indirect blocks
		 * never point at garbage.
		 */
		if (error = bwrite(nbp)) {
			blkfree(ip, nb, fs->fs_bsize);
			brelse(bp);
			return (error);
		}
		bap[i] = nb;
		/*
		 * If required, write synchronously, otherwise use
		 * delayed write. If this is the first instance of
		 * the delayed write, reassociate the buffer with the
		 * file so it will be written if the file is sync'ed.
		 */
		if (flags & B_SYNC && !(flags & IO_DATAONLY))
			bwrite(bp);
		else
			bdwrite(bp, vp);
	}

	/*
	 * Get the data block, allocating if necessary.
	 */
	if (nb == 0) {
		if (flags & B_READ) {
			brelse(bp);
			*bpp = (daddr_t) -1;
			return 0;
		}
		pref = blkpref(ip, lbn, i, &bap[0]);
		if (error = alloc(ip, lbn, pref, (int)fs->fs_bsize, &newb)) {
			brelse(bp);
			return error;
		}
		nb = newb;
		*bpp = fsbtodb(fs, nb);
		bap[i] = nb;
		/*
		 * If required, write synchronously, otherwise use
		 * delayed write. If this is the first instance of
		 * the delayed write, reassociate the buffer with the
		 * file so it will be written if the file is sync'ed.
		 */
		if (flags & B_SYNC && !(flags & IO_DATAONLY))
			bwrite(bp);
		else
			bdwrite(bp, vp);
		IN_LOCK(ip);
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
		*allocated = TRUE;
		return 0;
	}
	else {
		brelse(bp);
		*bpp = fsbtodb(fs, nb);
		return 0;
	}
}

ufs_ubcrealloc(register struct inode *ip, 
	daddr_t lbn,
	u_int osize, 
	u_int newsize,
	daddr_t *dblock,
	vm_page_t *pl)
{
	daddr_t blkpref();
	struct vnode *vp;
	register struct fs *fs;
	struct ufs_realloc ufs_realloc;
	struct ufs_realloc *rep = &ufs_realloc;
	struct plookup {
		int pl_uflags;
		vm_page_t pl_page;
	} plookup[MAXBSIZE/MINBSIZE], *pla;
	register struct plookup *plp;
	register vm_offset_t offset;
	register vm_offset_t ooffset;
	register int np, plsz;
	register daddr_t pblk;
	vm_page_t pp;
	register vm_page_t rpage;
	int error, uflags;
	
	fs = ip->i_fs;
	vp = ITOV(ip);

	/*
	 * Do a lookup to prevent any I/O from 
	 * starting by asserting a hold on the pages.
	 * We're write locked at ip because flags & B_WRITE
	 */


	*pl = rpage = VM_PAGE_NULL;

	offset = trunc_page(lbn << fs->fs_bshift);
	ooffset = offset + round_page(osize);
	np = atop(osize + page_mask);
	if (np > (MAXBSIZE/MINBSIZE)) {
		pla = (struct plookup *) 
			h_kmem_alloc(np * sizeof (struct plookup));
		plp = pla;
	}
	else {
		pla = NULL;
		plp = plookup;
	}
	for (pblk = fsbtodb(fs, ip->i_db[lbn]), plsz = 0; offset < ooffset; 
		offset += PAGE_SIZE, plp++, plsz++, 
		pblk += btodb(PAGE_SIZE)) {
		plp->pl_uflags = 0;
		error = ubc_lookup(vp, offset, fs->fs_bsize, 0, 
			&plp->pl_page, &plp->pl_uflags);
		if (error) goto failed;
		if (plp->pl_uflags & B_NOCACHE) {
			pp = plp->pl_page;
			(daddr_t) (pp->pg_pfs) = pblk;
			error = ufs_rwblk(vp, pp, 1, B_READ, (daddr_t)0); 
			if (error) {
				plsz++;
				goto failed;
			}
		}
	}

	rep->ur_flags = B_UBC;
	error = realloccg(ip, lbn,
		blkpref(ip, lbn, (int)lbn, &ip->i_db[0]),
		osize, (int)newsize, rep);

	if (error) goto failed;

	*dblock = rep->ur_blkno;

	offset = trunc_page(lbn << fs->fs_bshift);
	pblk = rep->ur_blkno;
	for (plp = pla ? pla : plookup; offset < ooffset; 
		pblk += btodb(PAGE_SIZE), offset += PAGE_SIZE, plp++) {
		pp = plp->pl_page;
		(daddr_t) (pp->pg_pfs) = pblk;
		if (rpage) pp->pg_pnext = rpage;
		else pp->pg_pnext = VM_PAGE_NULL;
		rpage = pp;
	}

	if (pla) 
		h_kmem_free((caddr_t) pla, np * sizeof(struct plookup));

	for (ooffset = trunc_page(lbn << fs->fs_bshift) + newsize;
		offset < ooffset;
		offset += PAGE_SIZE, pblk += btodb(PAGE_SIZE)) {
		uflags = 0;
		error = ubc_page_alloc(vp, offset, fs->fs_bsize, 0, 
			&pp, &uflags);
		if (error || (uflags & B_NOCACHE) == 0)
			panic("ufs_ubcrealloc: page alloc failed");
		ubc_page_zero(pp, 0, PAGE_SIZE);
		(daddr_t) (pp->pg_pfs) = pblk;
		if (rpage) pp->pg_pnext = rpage;
		else pp->pg_pnext = VM_PAGE_NULL;
		rpage = pp;
	}
	*pl = rpage;
	return 0;
failed:
	if (plsz) {
		for (plp = pla ? pla : plookup; plsz--; plp++)
			ubc_page_release(plp->pl_page, 0);
		if (pla) 
			h_kmem_free((caddr_t) pla, np * sizeof(struct plookup));
	}
	return error;
}

ufs_ubcrealloc_done(register struct inode *ip,
		    register vm_page_t pp,
		    int sync)
{
	register vm_page_t pn, fp, lp;
	int error = 0;

	if (pp != VM_PAGE_NULL) {
		if (sync) {
			fp = lp = pp;
			do {
				pn = pp->pg_pnext;
				ubc_page_busy(pp, 1);
				lp->pg_pnext = pp;
				lp = pp;
				ubc_page_release(pp, 0);
			} while (pp = pn);
			fp->pg_pprev = lp;
			lp->pg_pnext = VM_PAGE_NULL;
			error = ufs_writepages(fp, ip, 1, 0, 0, 0);
		} else {
			do {
				pn = pp->pg_pnext;
				ubc_page_release(pp, B_DIRTY);
			} while (pp = pn);
		}
	}
	return(error);
}
