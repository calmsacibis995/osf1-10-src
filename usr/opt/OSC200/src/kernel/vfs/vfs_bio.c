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
static char	*sccsid = "@(#)$RCSfile: vfs_bio.c,v $ $Revision: 4.3.17.7 $ (DEC) $Date: 1994/01/27 14:18:22 $";
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
/*
 *	vfs/vfs_bio.c
 *
 *	Revision History:
 *
 * 6-Apr-91	Ron Widyono
 *	Enable BUF_GIVE_AWAY code in bwrite() for RT_PREEMPT case to keep
 *	lock counts consistent.
 *	
 *
 */
#include <bufcache_stats.h>
#include <rt_preempt.h>

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/biostats.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/trace.h>
#include <sys/ucred.h>
#include <kern/queue.h>
#include <kern/sched_prim.h>
#include <kern/assert.h>
#include <sys/lock_types.h>
#include <mach/vm_param.h>
#include <sys/dk.h>	/* for SAR counters */
#include <sys/vfs_proto.h>

#ifdef CJKTRACE
#include <sys/logsys.h>
extern long pr_cjtrace;
extern long pr_bptrace;
extern struct buf *prbufs;
extern int prnbufs;
#endif /* CJKTRACE */

int	BQ_AGE_DISABLE = 1;

struct	buf *buf;		/* the buffer pool itself */
char	*buffers;
long	nbuf,			/* number of buffer headers */
	bufpages;		/* number of memory pages in the buffer pool */
extern int	bufhsz;		/* size of buffer cache hash table */
struct	bufhd *bufhash;		/* base of buffer cache hash table */

struct	buf bfreelist[BQUEUES];	/* heads of available lists */
#if	BUFCACHE_STATS
struct	bio_stats	bio_stats = { 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
#if	MACH_LDEBUG
char 	biodone_ldebug;
#endif


/*
 * This lets sar know whether a biowait is outstanding.
 */
int sar_bio_state;

/*
 * Locking precedence:
 *	buffer lock (blocking lock)
 *	buffer cache free list lock (spin lock)
 *	buffer cache hash chain lock (spin lock)
 *
 * We often need to search the hash chain first, and then take a buffer
 * lock when we think we have found the buffer in the cache.  We must
 * drop the hash chain lock to take the blocking buffer lock and retake
 * the hash chain lock later, if needed.
 *
 * When we want to remove a buffer from the free list, we lock the free
 * list and conditionally lock the buffer.  If we can't lock the buffer,
 * we try the next one.  When we manage to lock a buffer, we remove it
 * from the free list, while holding the buffer lock and the free list
 * lock.
 *
 * Buffers are locked pretty much from the time we find an interesting one
 * in the buffer pool until we are done with it.  This means throughout any
 * I/O as well.
 *
 */

/*
 * The buffer freelist lock covers all of the freelist queues.
 * The lock only applies to UNIX_LOCKS kernels.
 */
udecl_simple_lock_data(,bfreelist_lock);
#define	BFREE_LOCK_INIT()	usimple_lock_init(&bfreelist_lock);
#define	BFREE_LOCK()		usimple_lock(&bfreelist_lock)
#define	BFREE_UNLOCK()		usimple_unlock(&bfreelist_lock)

#define BFREE_LOCK_HOLDER()	SLOCK_HOLDER(&bfreelist_lock)

/*
 * A buffer cache hash chain is protected by an individual lock that
 * prevents buffers from being added to or removed from the chain while
 * the lock is held.  This lock is needed only by UNIX_LOCKS kernels.
 *
 * Each buffer cache hash chain contains a timestamp that is updated
 * when buffers are added to the list.  The timestamp when is not
 * incremented when buffers are removed from the hash chain.
 */
#define	BHASH_LOCK_INIT(bp)	usimple_lock_init(&(bp)->bhd_lock)
#define	BHASH_LOCK(bp)		usimple_lock(&((bufhd_t *) bp)->bhd_lock)
#define	BHASH_UNLOCK(bp)	usimple_unlock(&((bufhd_t *) bp)->bhd_lock)
#define BHASH_STAMP(bp)		((struct bufhd *)(bp))->bhd_stamp

/*
 * Read in (if necessary) the block and return a buffer pointer.
 */
bread(vp, blkno, size, cred, bpp)
	struct vnode *vp;
	daddr_t blkno;
	int size;
	struct ucred *cred;
	struct buf **bpp;
{
	register struct buf *bp;
	int error;

	if (size == 0)
		panic("bread: size 0");

        ts_lread++; /* global table() system call counter (see table.h) */
	*bpp = bp = getblk(vp, blkno, size);
#ifdef	KTRACE
	kern_trace(800, bp->b_dev, blkno, size);
#endif  /* KTRACE */
        if (event_posted(&bp->b_iocomplete)) {
		trace(TR_BREADHIT, pack(vp, size), blkno);
		return (0);
	}
	bp->b_flags |= B_READ;
	if (bp->b_bcount > bp->b_bufsize)
		panic("bread");
	if (bp->b_rcred == NOCRED && cred != NOCRED) {
		crhold(cred);
		bp->b_rcred = cred;
	}
	event_clear(&bp->b_iocomplete);
	VOP_STRATEGY(bp, error);
	trace(TR_BREADMISS, pack(vp, size), blkno);
	u.u_ru.ru_inblock++;		/* pay for read */
        ts_bread++; /* global table() system call counter (see table.h) */
	error = biowait(bp);
	LASSERT(BUF_LOCK_HOLDER(bp));
	ASSERT(bp->b_bcount == size);
	return (error);
}

/*
 * Read in the block, like bread, but also start I/O on the
 * read-ahead block (which is not allocated to the caller)
 */
breada(vp, blkno, size, rablkno, rabsize, cred, bpp)
	struct vnode *vp;
	daddr_t blkno; int size;
	daddr_t rablkno; int rabsize;
	struct ucred *cred;
	struct buf **bpp;
{
	register struct buf *bp, *rabp;
	int error;

#ifdef	KTRACE
	kern_trace(803, blkno, size, cred);
#endif  /* KTRACE */
	bp = NULL;
	/*
	 * If the block isn't in core, then allocate
	 * a buffer and initiate i/o (getblk checks
	 * for a cache hit).  Note that incore is less
	 * certain on a multiprocessor.
	 */
	if (!incore(vp, blkno)) {
        	ts_lread++; /* global table() system call counter (table.h) */
		*bpp = bp = getblk(vp, blkno, size);
		/*
		 * We depend on the fact that getblk() won't
		 * return until the b_iocomplete status reflects
		 * whether an I/O has already happened.  getblk()
		 * makes this guarantee by first taking the buffer
		 * lock, which will remain held while an I/O is
		 * in progress.  If we move to a scheme where the
		 * buffer lock is not held over the I/O, getblk()
		 * will have to block on the iodone event should
		 * the buffer be found in the cache.
		 */
		if (!event_posted(&bp->b_iocomplete)) {
			bp->b_flags |= B_READ;
			if (bp->b_bcount > bp->b_bufsize)
				panic("breada");
			if (bp->b_rcred == NOCRED && cred != NOCRED) {
				crhold(cred);
				bp->b_rcred = cred;
			}
			event_clear(&bp->b_iocomplete);
			VOP_STRATEGY(bp, error);
			trace(TR_BREADMISS, pack(vp, size), blkno);
			u.u_ru.ru_inblock++;		/* pay for read */
			ts_bread++;
		} else
			trace(TR_BREADHIT, pack(vp, size), blkno);
		LASSERT(BUF_LOCK_HOLDER(bp));
		ASSERT(bp->b_bcount == size);
	}

	/*
	 * Start i/o on the read-ahead block.
	 */
	ASSERT(rablkno != 0 && rabsize > 0);
	if (!incore(vp, rablkno)) {
		rabp = getblk(vp, rablkno, rabsize);
		LASSERT(BUF_LOCK_HOLDER(rabp));
		ASSERT(rabp->b_bcount == rabsize);
		if (event_posted(&rabp->b_iocomplete)) {
			brelse(rabp);
			trace(TR_BREADHITRA, pack(vp, rabsize), rablkno);
		} else {
			rabp->b_flags |= B_READ|B_ASYNC;
			if (rabp->b_bcount > rabp->b_bufsize)
				panic("breadrabp");
			if (rabp->b_rcred == NOCRED && cred != NOCRED) {
				crhold(cred);
				rabp->b_rcred = cred;
			}
			event_clear(&rabp->b_iocomplete);

			BUF_GIVE_AWAY(rabp);

			VOP_STRATEGY(rabp, error);
			trace(TR_BREADMISSRA, pack(vp, rabsize), rablkno);
			u.u_ru.ru_inblock++;		/* pay in advance */
		}
	}

	/*
	 * If block was in core, let bread get it.
	 * If block wasn't in core, then the read was started
	 * above, and just wait for it.
	 */
	if (bp == NULL)
		return (bread(vp, blkno, size, cred, bpp));
	error = biowait(bp);
	LASSERT(BUF_LOCK_HOLDER(bp));
	ASSERT(bp->b_bcount == size);
	return (error);
}

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
 * Write the buffer, waiting for completion.
 * Then release the buffer.
 */
bwrite(bp)
	register struct buf *bp;
{
	register int flag;
	register struct vnode *vp;
	int s, error = 0;

	LASSERT(BUF_LOCK_HOLDER(bp));
        ts_lwrite++; /* global table() system call counter (see table.h) */
	flag = bp->b_flags;
	bp->b_flags &= ~(B_READ | B_ERROR | B_DELWRI);
	/*
	 * If the write was "delayed" and is being initiated
	 * asynchronously then put the buffer on the q of blocks
	 * awaiting i/o completion status.  Do this before starting
	 * the I/O because after it is started this thread doesn't
	 * own the buffer.
	 */
	if ((flag & (B_ASYNC|B_DELWRI)) == (B_ASYNC|B_DELWRI))
		bp->b_flags |= B_AGE;
	if ((flag&B_DELWRI) == 0)
		u.u_ru.ru_oublock++;		/* noone paid yet */
	else
		reassignbuf(bp, bp->b_vp);
#ifdef	KTRACE
	kern_trace(801, bp->b_vp, bp->b_bcount, bp->b_lblkno);
#endif  /* KTRACE */
	trace(TR_BWRITE, pack(bp->b_vp, bp->b_bcount), bp->b_lblkno);
	if (bp->b_bcount > bp->b_bufsize)
		panic("bwrite");
	vp = bp->b_vp;
	s = splbio();
	VN_OUTPUT_LOCK(vp);
	vp->v_numoutput++;
	VN_OUTPUT_UNLOCK(vp);
	splx(s);
	event_clear(&bp->b_iocomplete);
#if	(MACH_LDEBUG || RT_PREEMPT)
	if (flag&B_ASYNC) {
		BUF_GIVE_AWAY(bp);
	}
#endif
	VOP_STRATEGY(bp, error);
	if (bp->b_dev == ufs_writes.dev && (bp->b_flags & B_READ) == 0) {
		if (vp->v_type == VDIR) {
			if (flag & B_ASYNC)
				ufs_writes.async_dirs++;
			else
				ufs_writes.sync_dirs++;
		} else if (bp->b_bcount == 8192) {
			if (flag & B_ASYNC)
				ufs_writes.async_meta_blocks++;
			else
				ufs_writes.sync_meta_blocks++;
		} else if (bp->b_bcount == 2048) {
			if (flag & B_ASYNC)
				ufs_writes.async_meta_cgs++;
			else
				ufs_writes.sync_meta_cgs++;
		} else {
			if (flag & B_ASYNC)
				ufs_writes.async_meta_others++;
			else
				ufs_writes.sync_meta_others++;
		}
	}

        ts_bwrite++; /* global table() system call counter (see table.h) */

	/*
	 * If the write was synchronous, then await i/o completion.
	 * If the write was delayed, give away ownership of the buffer.
	 */
	if ((flag&B_ASYNC) == 0) {
		error = biowait(bp);
		brelse(bp);
	}
	return (error);
}

/*
 * Release the buffer, marking it so that if it is grabbed
 * for another purpose it will be written out before being
 * given up (e.g. when writing a partial block where it is
 * assumed that another write for the same block will soon follow).
 * This can't be done for magtape, since writes must be done
 * in the same order as requested.
 */
bdwrite(bp, vp)
	register struct buf *bp;
	register struct vnode *vp;
{
	int error;
	int retval;

	LASSERT(BUF_LOCK_HOLDER(bp));
        ts_lwrite++; /* global table() system call counter (see table.h) */
	if ((bp->b_flags & B_DELWRI) == 0) {
		bp->b_flags |= B_DELWRI;
		reassignbuf(bp, vp);
		u.u_ru.ru_oublock++;		/* noone paid yet */
	}
#ifdef	KTRACE
	kern_trace(804, bp->b_vp, bp->b_bcount, bp->b_lblkno);
#endif  /* KTRACE */
	/*
	 * If this is a tape drive, the write must be initiated.
	 */
	VOP_IOCTL(bp->b_vp, 0, B_TAPE, 0, NOCRED, error, &retval);
	if (error == 0)
		bawrite(bp);
	else {
		bp->b_flags |= B_DELWRI;
		event_post(&bp->b_iocomplete);
		brelse(bp);
	}
}

/*
 * Release the buffer, start I/O on it, but don't wait for completion.
 */
bawrite(bp)
	register struct buf *bp;
{

	LASSERT(BUF_LOCK_HOLDER(bp));
	bp->b_flags |= B_ASYNC;
	(void) bwrite(bp);
}

/*
 * Release the buffer, with no I/O implied.
 */
brelse(bp)
	register struct buf *bp;
{
	register struct buf *flist;
	register int awaken, inshead;
	long flags;
	int had_error, s;

	/*
	 * It is more than likely that the buffer was actually locked by some
	 * other thread and that we are picking up the pieces.  But we take
	 * care when we initiate an asynchronous I/O to give away ownership
	 * of the buffer.
     	 *
      	 * Furthermore, on a multiprocessor, it is not yet the time to wake up
       	 * someone waiting on this buffer, or waiting for a free buffer.
	 */
#ifdef	KTRACE
	kern_trace(805, bp->b_vp, bp->b_bcount, bp->b_lblkno);
#endif  /* KTRACE */
	LASSERT(BUF_LOCK_HOLDER(bp));
	trace(TR_BRELSE, pack(bp->b_vp, bp->b_bufsize), bp->b_lblkno);
	/*
	 * Retry I/O for locked buffers rather than invalidating them.
	 * We currently aren't locking any buffers. - XXX
	 */
	if ((bp->b_flags & B_ERROR) && (bp->b_flags & B_LOCKED))
		bp->b_flags &= ~B_ERROR;

	/*
	 * If this buffer had an async write error
	 * mark it dirty and retry the write later.
	 * Don't want to lose it's contents.
	 *
	 * If this buffer had an synchronous write error,
	 * mark the dev vnode with VIOERROR. We have to
	 * discard the contents because file systems
	 * like UFS depend on ordering for recoverability.
	 */
	had_error = 0;
	if ((bp->b_flags & B_ERROR) &&
	    (bp->b_flags & (B_READ|B_INVAL|B_NOCACHE)) == 0 &&
	    bp->b_bufsize > 0) {
		if ((bp->b_flags & B_ASYNC) == 0) {
			(bp->b_vp)->v_flag |= VIOERROR;
		} else {
			bp->b_flags &= ~B_ERROR;
			bp->b_error = 0;
			bp->b_flags |= B_DELWRI;
			reassignbuf(bp, bp->b_rvp); /* v_dirtyblkhd */
			had_error = 1;
		}
	}

	/*
	 * Disassociate buffers that are no longer valid.
	 */
	if (bp->b_flags & (B_NOCACHE|B_ERROR))
		bp->b_flags |= B_INVAL;

	if ((bp->b_bufsize <= 0) || (bp->b_flags & (B_ERROR|B_INVAL))) {
		if (bp->b_vp)
			brelvp(bp);
		bp->b_flags &= ~B_DELWRI;
	}
	/*
	 * Stick the buffer back on a free list.
	 */
	flags = bp->b_flags;
	bp->b_flags &= ~(B_ASYNC|B_AGE|B_NOCACHE|B_WANTED);
	awaken = inshead = 0;
	if (bp->b_bufsize <= 0) {
		/* block has no buffer... put at front of unused buffer list */
		flist = &bfreelist[BQ_EMPTY];
		inshead++;
	} else if (flags & (B_ERROR|B_INVAL)) {
		/* block has no info ... put at front of most free list */
		flist = &bfreelist[BQ_AGE];
		inshead++;
	} else {
		if (flags & (B_LOCKED|B_DELWRI))
			flist = &bfreelist[BQ_LOCKED];
 		else if ((flags & B_AGE) && (BQ_AGE_DISABLE == 0))
			flist = &bfreelist[BQ_AGE];
		else
			flist = &bfreelist[BQ_LRU];
	}
	if (had_error)
		bp->b_flags |= B_ERROR;
	s = splbio();
	BFREE_LOCK();
	if (inshead)
		binsheadfree(bp, flist);
	else
		binstailfree(bp, flist);
	/*
	 * Wakeup processes waiting for a free buffer.
	 */
	if (bfreelist[0].b_flags&B_WANTFREE) {
		bfreelist[0].b_flags &= ~B_WANTFREE;
		awaken++;
	}
	BFREE_UNLOCK();
	/*
	 * Unlocking buffer with interrupts disabled can deadlock
	 * with pmap system when another thread holds the buffer
	 * lock's interlock with interrupts enabled.
	 */
	BUF_UNLOCK(bp);
	splx(s);
	if (awaken)
		thread_wakeup((vm_offset_t)bfreelist);
}

/*
 * See if the block is associated with some buffer
 * (mainly to avoid getting hung up on a wait in breada)
 */
incore(vp, blkno)
	struct vnode *vp;
	daddr_t blkno;
{
	register struct buf *bp;
	register struct buf *dp;
	int s;

	dp = BUFHASH(vp, blkno);
	s = splbio();
	BHASH_LOCK(dp);
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw)
		/*
		 * There is an assumption that incore won't block.
		 * So, we can't lock the buffer.  Incore is advisory
		 * at best on a multiprocessor.  The block may be
		 * incore now, but not incore when you go to read it
		 * later.  The buffer may also be valid now, but
		 * invalid later.  So we don't lock it to check
		 * the flags.
		 */
		if (bp->b_lblkno == blkno && bp->b_vp == vp &&
		    (bp->b_flags & B_INVAL) == 0) {
			BHASH_UNLOCK(dp);
			splx(s);
			return (1);
		}
	BHASH_UNLOCK(dp);
	splx(s);
	return (0);
}

/*
 * Return a block if it is in memory.
 */
#if	!MACH
baddr(vp, blkno, size, cred, bpp)
	struct vnode *vp;
	daddr_t blkno;
	int size;
	struct ucred *cred;
	struct buf **bpp;
{

	if (incore(vp, blkno))
		return (bread(vp, blkno, size, cred, bpp));
	*bpp = 0;
	return (0);
}
#endif	



/*
 * Assign a buffer for the given block.  If the appropriate
 * block is already associated, return it; otherwise search
 * for the oldest non-busy buffer and reassign it.
 *
 * This routine is never called from interrupt level and
 * especialy not during a panic because it can block.  The
 * caller is expected to wait for any I/O on the buffer to complete.
 */
struct buf *
getblk(vp, blkno, size)
	register struct vnode *vp;
	daddr_t blkno;
	int size;
{
	register struct buf *bp, *dp, *bp2;
	register int s;
	register int stamp;
	int research = 0;

	if (size > MAXBSIZE)
		panic("getblk: size too big");
	/*
	 * Search the cache for the block.  If we hit, but
	 * the buffer is in use for i/o, then we wait until
	 * the i/o has completed.
	 */
	dp = BUFHASH(vp, blkno);
loop:
	s = splbio();
	BHASH_LOCK(dp);
	for (bp = dp->b_forw; bp != dp; bp = bp->b_forw) {
		/*
		 * We can check these fields without locking bp because
		 * we have the hash chain locked so these fields will
		 * not change on us.  We can't check for an invalid
		 * buffer without locking the buffer, so we must unlock
		 * the hash chain, lock the buffer, and then check the
		 * flags field.
		 */
		if (bp->b_lblkno != blkno || bp->b_vp != vp)
			continue;
		BHASH_UNLOCK(dp);
		BUF_LOCK(bp);
		/*
		 * Buffer could have changed hands while we slept
		 * or while the hash chain was unlocked.
		 */
		if (bp->b_lblkno != blkno || bp->b_vp != vp) {
			BUF_UNLOCK(bp);
			splx(s);
			goto loop;
		}
		/*
		 * If this buffer matches and is invalid, remove it
		 * from the hash chain, so that we don't find it again
		 * when re-searching the hash chain after calling getnewbuf.
		 */
		if (bp->b_flags & B_INVAL) {
			BHASH_LOCK(dp);
			bremhash(bp);
			BHASH_UNLOCK(dp);
			BUF_UNLOCK(bp);
			splx(s);
			goto loop;
		}
		BFREE_LOCK();
		bremfree(bp);
		BFREE_UNLOCK();
		splx(s);
		if (bp->b_bcount != size) {
			/* Stray b_bcount */
			bp->b_flags |= B_INVAL;
			/*
			 * How do we know this buffer is dirty?  XXX
			 * For that matter, how do we guarantee that
			 * there's no race with someone else re-creating
			 * an overlapping buffer and then issuing a read?  XXX
			 */
			bwrite(bp);
			goto loop;
		}
		bp->b_flags |= B_CACHE;
		BUF_STATS(bio_stats.getblk_hits++);
		LASSERT(BUF_LOCK_HOLDER(bp));
		ASSERT(bp->b_bcount == size);
		return (bp);
	}
	stamp = BHASH_STAMP(dp);
	BHASH_UNLOCK(dp);
	splx(s);
	BUF_STATS(bio_stats.getblk_misses++);
	bp = getnewbuf();
	bfree(bp);
	bgetvp(vp, bp);
	bp->b_lblkno = blkno;
	bp->b_blkno = blkno;
	bp->b_error = 0;
	bp->b_resid = 0;
	s = splbio();
	BHASH_LOCK(dp);
	if (stamp != BHASH_STAMP(dp)) {
		/*
		 * Someone else could have inserted an identical buffer
		 * in the hash chain while the hash chain was unlocked
		 * or while we slept in getnewbuf (if we did).
		 */
		research++;
		for (bp2 = dp->b_forw; bp2 != dp; bp2 = bp2->b_forw) {
			/*
			 * We can't check the B_INVAL flag here because
			 * we don't have bp2 locked.  So we go to the
			 * top and try again when we find a match.
			 * We mark bp invalid so that it will be added
			 * to the begining of the age list.
			 */
			if (bp2->b_lblkno == blkno && bp2->b_vp == vp) {
				BHASH_UNLOCK(dp);
				splx(s);
				bp->b_flags |= B_INVAL;
				brelse(bp);
				BUF_STATS(bio_stats.getblk_dupbuf++);
				goto loop;
			}
		}
	}
	binshash(bp, dp);
	LASSERT(valid_buf_on_chain(bp, dp));
	BHASH_UNLOCK(dp);
	splx(s);
	if (research)
		BUF_STATS(bio_stats.getblk_research += research);
	if (size != bp->b_bcount)
		allocbuf(bp, size);
	LASSERT(BUF_LOCK_HOLDER(bp));
	ASSERT(bp->b_bcount == size);
	return (bp);
}

/*
 * get an empty block,
 * not assigned to any particular device
 */
struct buf *
geteblk(size)
	int size;
{
	register struct buf *bp;

	if (size > MAXBSIZE)
		panic("geteblk: size too big");
	bp = getnewbuf();
	bp->b_flags |= B_INVAL;
	bp->b_error = 0;
	bp->b_resid = 0;
	bfree(bp);
	if (size != bp->b_bcount)
		allocbuf(bp, size);
	LASSERT(BUF_LOCK_HOLDER(bp));
	ASSERT(bp->b_bcount == size);
	return(bp);
}

/*
 * Find a buffer which is available for use.
 * Select something from a free list.
 * Preference is to AGE list, then LRU list.
 * Removes buffer from its hash chain.
 */
struct buf *
getnewbuf()
{
	register struct buf *bp, *dp;
	register struct ucred *cred;
	register struct buf *dp2;
	int s, locked;
	int lockedbufs = 0;

	BUF_STATS(bio_stats.getnewbuf_calls++);
loop:
	s = splbio();
	BFREE_LOCK();
	for (dp = &bfreelist[BQ_AGE]; dp >= bfreelist; dp--) {
		if (dp->av_forw == dp)
			continue;
		/*
		 * Walk freelist avoiding locked buffers and hash chains.
		 * On a uniprocessor, this loop will terminate on the first
		 * buffer on the freelist.
		 */
		for (bp = dp->av_forw; bp != dp; bp = bp->av_forw) {
			BUF_LOCK_TRY(bp, locked);
			if (!locked) {
				lockedbufs++;
				continue;
			}
			dp2 = bp->b_hash_chain;
			if (dp2 != BHASH_NULL)
				BHASH_LOCK(dp2);
			LASSERT(dp2==BHASH_NULL || valid_chain(dp2, 0));
			goto got_one;
		}
	}

                          /* no free blocks */
	dp = bfreelist;
	dp->b_flags |= B_WANTFREE;
	assert_wait((vm_offset_t)dp, FALSE);
	BFREE_UNLOCK();
	splx(s);
	thread_block();
	goto loop;
got_one:
	LASSERT(BFREE_LOCK_HOLDER());
	LASSERT(BUF_LOCK_HOLDER(bp));
	bremfree(bp);
	BFREE_UNLOCK();
	if (bp->b_flags & B_DELWRI) {
		if (dp2 != BHASH_NULL)
			BHASH_UNLOCK(dp2);
		splx(s);
		(void) bawrite(bp);
		goto loop;
	}
	if (dp2 != BHASH_NULL) {
		bremhash(bp);
		BHASH_UNLOCK(dp2);
	}
	splx(s);
	trace(TR_BRELSE, pack(bp->b_vp, bp->b_bufsize), bp->b_lblkno);
	if (lockedbufs)
		BUF_STATS(bio_stats.getnewbuf_buflocked += lockedbufs);
	if (bp->b_vp)
		brelvp(bp);
	if (bp->b_rcred != NOCRED) {
		cred = bp->b_rcred;
		bp->b_rcred = NOCRED;
		crfree(cred);
	}
	if (bp->b_wcred != NOCRED) {
		cred = bp->b_wcred;
		bp->b_wcred = NOCRED;
		crfree(cred);
	}
	bp->b_flags = B_BUSY;
	bp->b_iodone = NULL;
	event_clear(&bp->b_iocomplete);
	LASSERT(BUF_LOCK_HOLDER(bp));
	return (bp);
}

/*
 * Wait for I/O completion on the buffer; return errors
 * to the user.
 */
biowait(bp)
	register struct buf *bp;
{
	LASSERT(BUF_LOCK_HOLDER(bp));
	sar_bio_state++;
	(void) event_wait(&bp->b_iocomplete, FALSE, 0);
	sar_bio_state--;

	/*
	 * Pick up the device's error number and pass it to the user;
	 * if there is an error but the number is 0 set a generalized code.
	 */
	if ((bp->b_flags & B_ERROR) == 0)
		return (0);
	if (bp->b_error)
		return (bp->b_error);
	return (EIO);
}

/*
 * Mark I/O complete on a buffer.
 * If someone should be called, e.g. the pageout
 * daemon, do so.  Otherwise, wake up anyone
 * waiting for it.
 */
void
biodone(bp)
	register struct buf *bp;
{
	register struct vnode *vp;
	int s, wakeup = 0; 
	register int flags;

#ifdef CJKTRACE
/*	if (pr_bptrace) {
		if (bp >= prbufs && bp < &prbufs[prnbufs])
			LOG_SYS_PR(bp, PRESTO_BIODONE);
	}
*/
#endif /* CJKTRACE */

	ASSERT(bp != (struct buf *) NULL);
	if (event_posted(&bp->b_iocomplete))
		panic("dup biodone");

#ifdef	KTRACE
	kern_trace(802, bp->b_dev, bp->b_lblkno, bp->b_error); 
#endif  /* KTRACE */

	flags = bp->b_flags;

	if (flags & (B_SWAP|B_UBC)) {
		vm_iodone(bp);
		return;
	}

	if ((flags & B_READ) == 0) {
		bp->b_dirtyoff = bp->b_dirtyend = 0;
		if (vp = bp->b_vp) {
			s = splbio();
			VN_OUTPUT_LOCK(vp);
			ASSERT(vp->v_numoutput > 0);
			vp->v_numoutput--;
			if ((vp->v_outflag & VOUTWAIT) &&
			    vp->v_numoutput <= 0) {
				vp->v_outflag &= ~VOUTWAIT;
				wakeup++;
			}
			VN_OUTPUT_UNLOCK(vp);
			splx(s);
			if (wakeup)
				thread_wakeup((vm_offset_t)&vp->v_numoutput);
		}
	}

	/*
	 * NOTE:  Device drivers using b_iodone must funnel, 
	 * if necessary, in their iodone routine -- and must
	 * also provide themselves a thread context, if necessary.
	 */
	if (bp->b_iodone) {
		void (*f)() = bp->b_iodone;
		bp->b_iodone = NULL;
		(*f)(bp);
		return;
	}

#ifdef CJKTRACE
	if (pr_bptrace) {
		if (bp >= prbufs && bp < &prbufs[prnbufs])
			LOG_SYS_PR(bp, PRESTO_EPOST);
	}
#endif /* CJKTRACE */

	event_post(&bp->b_iocomplete);
	if (flags & B_ASYNC) {
		/*
		 * This buf must have been previously given away. We
		 * accept ownership here.
		 */
		BUF_ACCEPT(bp);
		brelse(bp);
	}
}

/*
 * Return number of busy buffers.
 */
mntbusybuf(mountp)
	struct mount *mountp;
{
	register struct vnode *vp;
	register struct vnode *nvp;
	register int nbusy;
	extern struct vnode *rootvp;

	nbusy = 0;
	MOUNT_VLIST_LOCK(mountp);
	for (vp = mountp->m_mounth; vp; vp = nvp) {
		/*
		 * Potentially replace vp with its shadow (VBLK).
		 * shadowvnode and vget_nowait expect to receive a
		 * locked vnode.
		 * nvp will hold the vnode we flush; vp will hold the
		 * one on the mount vnode list.
		 */
		VN_LOCK(vp);
		if (vp->v_type == VBLK) {
			if ((nvp = shadowvnode(vp)) == (struct vnode *) 0) {
				VN_UNLOCK(vp);
				nvp = vp->v_mountf;
				continue;
			}
			VN_UNLOCK(vp);
			VN_LOCK(nvp);
		} else
			nvp = vp;
		if (vget_nowait(nvp)) {
			VN_UNLOCK(nvp);
			nvp = vp->v_mountf;
			continue;
		}
		VN_UNLOCK(nvp);
		MOUNT_VLIST_UNLOCK(mountp);
		nbusy += nvp->v_numoutput;
		vrele(nvp);
		/*
		 * We must check to see if the vnode is still on the
		 * mount vnode list.  After the above, just about anything
		 * could have happened to the vnode.  It's okay if the
		 * vnode was removed from this mount vnode list and
		 * added back to it because we insert at the beginning
		 * of the list.
		 */
		MOUNT_VLIST_LOCK(mountp);
		if (vp->v_mount == mountp)
			nvp = vp->v_mountf;
		else  {
			/*
			 * We have to start all over again.
			 */
			nvp = mountp->m_mounth;
			nbusy = 0;
		}
	}
	/*
	 * Since ``rootvp'' is not on the root's vnode chain,
	 * account for outstanding write buffers by hand.
	 */
	if (mountp == rootfs)
		nbusy += rootvp->v_numoutput;
	MOUNT_VLIST_UNLOCK(mountp);
	return (nbusy);
}

/*
 * Make sure all write-behind blocks associated
 * with mount point are flushed out (from sync).
 */
mntflushbuf(mountp, flags)
	struct mount *mountp;
	int flags;
{
	register struct vnode *vp;
	register struct vnode *nvp;

	MOUNT_VLIST_LOCK(mountp);
	for (vp = mountp->m_mounth; vp; vp = nvp) {
		/* 
		 * Potentially replace vp with its shadow (VBLK).
		 * shadowvnode and vget_nowait expect to receive a 
		 * locked vnode.
		 * nvp will hold the vnode we flush; vp will hold the
		 * one on the mount vnode list.
		 */
		VN_LOCK(vp);
		if (vp->v_type == VBLK) {
			if ((nvp = shadowvnode(vp)) == (struct vnode *) 0) {
		 		VN_UNLOCK(vp);
		 		nvp = vp->v_mountf;
	   	 		continue;
			}
		 	VN_UNLOCK(vp);
			VN_LOCK(nvp);
		} else 
			nvp = vp;
		if (vget_nowait(nvp)) {
		 	VN_UNLOCK(nvp);
		 	nvp = vp->v_mountf;
	   	 	continue;
		}
		VN_UNLOCK(nvp);
		MOUNT_VLIST_UNLOCK(mountp);
		vflushbuf(nvp, flags);
		if (nvp->v_type == VREG) 
			ubc_flush_dirty(nvp, (flags & B_SYNC) ? 0 : B_ASYNC);
		vrele(nvp);
		/*
		 * We must check to see if the vnode is still on the
		 * mount vnode list.  After the above, just about anything
		 * could have happened to the vnode.  It's okay if the
		 * vnode was removed from this mount vnode list and
		 * added back to it because we insert at the beginning
		 * of the list.
		 */
		MOUNT_VLIST_LOCK(mountp);
		if (vp->v_mount == mountp)
			nvp = vp->v_mountf;
		else  {
			BUF_STATS(bio_stats.mntflushbuf_misses++);
			/*
			 * We have to start all over again.
			 */
			nvp = mountp->m_mounth;
		}
	}
	MOUNT_VLIST_UNLOCK(mountp);
}

/*
 * Flush all dirty buffers associated with a vnode.
 */
void
vflushbuf(vp, flags)
	register struct vnode *vp;
	int flags;
{
	register struct buf *bp;
	register struct buf *nbp;
	int locked, s;
	int lockskips = 0;
		
	s = splbio();
	VN_BUFLISTS_LOCK(vp);
	/*
	 * Clear out async write errors.
	 * If they still fail, brelse()
	 * will mark them again....
	 */
	for (bp = vp->v_dirtyblkhd; bp; bp = nbp) {
		BUF_LOCK_TRY(bp, locked);
		if (!locked) {
			nbp = bp->b_blockf;
			continue;
		}
		if (bp->b_flags & B_ERROR)
			bp->b_flags &= ~B_ERROR;
		nbp = bp->b_blockf;
		BUF_UNLOCK(bp);
	}
	goto loop_lock;
loop:
	s = splbio();
	VN_BUFLISTS_LOCK(vp);
loop_lock:
	for (bp = vp->v_dirtyblkhd; bp; bp = nbp) {
		BUF_LOCK_TRY(bp, locked);
		/*
		 * Skip buffers we could not lock, and
		 * those that just got async write
		 * failures.
		 */
		if (!locked || bp->b_flags & B_ERROR) {
			nbp = bp->b_blockf;
			if (!locked)
				lockskips++;
			else
				BUF_UNLOCK(bp);
			continue;
		}
		ASSERT((bp->b_flags & B_DELWRI) != 0);
		VN_BUFLISTS_UNLOCK(vp);
		BFREE_LOCK();
		bremfree(bp);
		BFREE_UNLOCK();
		splx(s);
		/*
		 * Wait for I/O associated with indirect blocks to complete,
		 * since there is no way to quickly wait for them below.
		 * NB - This is really specific to ufs, but is done here
		 * as it is easier and quicker.
		 */
		if (bp->b_vp == vp || (flags & B_SYNC) == 0)
			(void) bawrite(bp);
		else
			(void) bwrite(bp);
		goto loop;
	}
	VN_BUFLISTS_UNLOCK(vp);
	splx(s);
	if (lockskips)
		BUF_STATS(bio_stats.vflushbuf_lockskips += lockskips);
	if ((flags & B_SYNC) == 0)
		return;

	s = splbio();
	VN_OUTPUT_LOCK(vp);
	while (vp->v_numoutput) {
		vp->v_outflag |= VOUTWAIT;
		assert_wait((vm_offset_t)&vp->v_numoutput, FALSE);
		VN_OUTPUT_UNLOCK(vp);
		thread_block();
		VN_OUTPUT_LOCK(vp);
	}
	VN_OUTPUT_UNLOCK(vp);
	VN_BUFLISTS_LOCK(vp);
#ifdef notdef
	if (vp->v_dirtyblkhd) {
		VN_BUFLISTS_UNLOCK(vp);
		splx(s);
		/*
		 * Don't need this.  This can happen when sync'ing;
		 * especially on devvp.
		 * vprint("vflushbuf: dirty", vp);
		 */
		goto loop;
	}
#endif
	VN_BUFLISTS_UNLOCK(vp);
	splx(s);
}

/*
 * Invalidate in core blocks belonging to closed or umounted filesystem
 *
 * Go through the list of vnodes associated with the file system;
 * for each vnode invalidate any buffers that it holds. Normally
 * this routine is preceeded by a mntflushbuf call, so that on a quiescent
 * filesystem there will be no dirty buffers when we are done. Binval
 * returns the count of dirty buffers when it is finished.
 *
 * If the vnode represents a block special file, we use its shadow vnode
 * instead, since all buffered io will have been done using it, not the
 * "real" vnode.
 */
mntinvalbuf(mountp)
	struct mount *mountp;
{
	register struct vnode *vp;
	register struct vnode *nvp;
	int dirty = 0;

	MOUNT_VLIST_LOCK(mountp);
	for (vp = mountp->m_mounth; vp; vp = nvp) {
		/* 
		 * Potentially replace vp with its shadow (VBLK).
		 * shadowvnode and vget_nowait expect to receive a 
		 * locked vnode.
		 * nvp will hold the vnode we flush; vp will hold the
		 * one on the mount vnode list.
		 */
		VN_LOCK(vp);
		if (vp->v_type == VBLK) {
			if ((nvp = shadowvnode(vp)) == (struct vnode *) 0) {
		 		VN_UNLOCK(vp);
		 		nvp = vp->v_mountf;
	   	 		continue;
			}
		 	VN_UNLOCK(vp);
			VN_LOCK(nvp);
		} else 
			nvp = vp;
		if (vget_nowait(nvp)) {
		 	VN_UNLOCK(nvp);
		 	nvp = vp->v_mountf;
	   	 	continue;
		}
		VN_UNLOCK(nvp);
	   	MOUNT_VLIST_UNLOCK(mountp);
		dirty += vinvalbuf(nvp, 1);
		/*
		 * If this vnode had outstanding write errors,
		 * mark the mount structure. This was the last
		 * chance to flush them out.... The M_IOERROR
		 * flag needs to remain set even if another
		 * mntinvalbuf is attempted, because the buffers
		 * that were marked with write failures are now
		 * invalidated.
		 */
		if (nvp->v_flag & VIOERROR) {
			mountp->m_flag |= M_IOERROR;
			nvp->v_flag &= ~VIOERROR;
		}

		dirty += (nvp->v_type == VREG) ?
			ubc_flush_dirty(nvp, B_ASYNC) : 0;
		vrele(nvp);
		/*
		 * We must check to see if the vnode is still on the
		 * mount vnode list.  After the above, just about anything
		 * could have happened to the vnode.  It's okay if the
		 * vnode was removed from this mount vnode list and
		 * added back to it because we insert at the beginning
		 * of the list.
		 */
		MOUNT_VLIST_LOCK(mountp);
		if (vp->v_mount == mountp)
			nvp = vp->v_mountf;
		else  {
			BUF_STATS(bio_stats.mntinvalbuf_misses++);
			/*
			 * We have to start all over again.
			 */
			nvp = mountp->m_mounth;
		}
	}
	MOUNT_VLIST_UNLOCK(mountp);
	return (dirty);
}

/*
 * Flush out and invalidate all buffers associated with a vnode.
 * Called with the underlying object locked.
 */
vinvalbuf(vp, save)
	register struct vnode *vp;
	int save;
{
	register struct buf *bp, *nbp;
	register struct buf **blist;
	register s;
	int dirty = 0, misses = 0, locked;

	s = splbio();
	VN_BUFLISTS_LOCK(vp);
	/*
	 * Clear out all async write errors.
	 * This is the last chance to write
	 * them out.....
	 */
	for (bp = vp->v_dirtyblkhd; bp; bp = nbp) {
		BUF_LOCK_TRY(bp, locked);
		if (!locked) {
			nbp = bp->b_blockf;
			continue;
		}
		if (bp->b_flags & B_ERROR)
			bp->b_flags &= ~B_ERROR;
		nbp = bp->b_blockf;
		BUF_UNLOCK(bp);
	}
	for (;;) {
		if (vp->v_dirtyblkhd)
			blist = &vp->v_dirtyblkhd;
		else if (vp->v_cleanblkhd)
			blist = &vp->v_cleanblkhd;
		else 
			break;
		while (bp = *blist) {
			VN_BUFLISTS_UNLOCK(vp);
			BUF_LOCK(bp);
			/*
			 * See if this buffer is still at the head of this list.
			 */
			BM(VN_BUFLISTS_LOCK(vp));
			if (bp != *blist) {
				BM(VN_BUFLISTS_UNLOCK(vp));
				BUF_UNLOCK(bp);
				VN_BUFLISTS_LOCK(vp);
				misses++;
				continue;
			}
			BM(VN_BUFLISTS_UNLOCK(vp));
			BFREE_LOCK();
			bremfree(bp);
			BFREE_UNLOCK();
			splx(s);
			if (save && (bp->b_flags & B_DELWRI)) {
				if ((bp->b_flags & B_ERROR) == 0) {
					dirty++;
					bawrite(bp);
					s = splbio();
					VN_BUFLISTS_LOCK(vp);
					continue;
				} else {
					/*
					 * If B_ERROR is set, mark the vnode
					 * with VIOERROR. We won't attempt
					 * to write it out again.
					 *
					 * We will let the unmount continue
					 * by decrementing ``dirty'' and
					 * invalidating the offending
					 * buffer.
					 */
					vp->v_flag |= VIOERROR;
					dirty--;
				}
			}
				
                        if (bp->b_vp != vp) {
                                reassignbuf(bp, bp->b_vp);
			} else 
                                bp->b_flags |= B_INVAL;
			brelse(bp);
			s = splbio();
			VN_BUFLISTS_LOCK(vp);
		}
	}
	VN_BUFLISTS_UNLOCK(vp);
	if (dirty) {
		VN_OUTPUT_LOCK(vp);
		while (vp->v_numoutput) {
			vp->v_outflag |= VOUTWAIT;
			assert_wait((vm_offset_t)&vp->v_numoutput, FALSE);
			VN_OUTPUT_UNLOCK(vp);
			thread_block();
			VN_OUTPUT_LOCK(vp);
		}
		VN_OUTPUT_UNLOCK(vp);
	}
	splx(s);
	if (misses)
		BUF_STATS(bio_stats.vinvalbuf_misses += misses);
	return (dirty);
}

/*
 * Associate a buffer with a vnode.
 */
bgetvp(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	int s;

	LASSERT(BUF_LOCK_HOLDER(bp));
	if (bp->b_vp || bp->b_rvp)
		panic("bgetvp: not free");
	VHOLD(vp);
	bp->b_rvp = bp->b_vp = vp;
	VN_LOCK(vp);
	if (vp->v_type == VBLK || vp->v_type == VCHR)
		bp->b_dev = vp->v_rdev;
	else
		bp->b_dev = NODEV;
	VN_UNLOCK(vp);
	/*
	 * Insert onto list for new vnode.
	 */
	s = splbio();
	VN_BUFLISTS_LOCK(vp);
	if (vp->v_cleanblkhd) {
		bp->b_blockf = vp->v_cleanblkhd;
		bp->b_blockb = &vp->v_cleanblkhd;
		vp->v_cleanblkhd->b_blockb = &bp->b_blockf;
		vp->v_cleanblkhd = bp;
	} else {
		vp->v_cleanblkhd = bp;
		bp->b_blockb = &vp->v_cleanblkhd;
		bp->b_blockf = NULL;
	}
	VN_BUFLISTS_UNLOCK(vp);
	splx(s);
}

/*
 * Disassociate a buffer from a vnode.
 */
brelvp(bp)
	register struct buf *bp;
{
	register struct vnode	*vp;
	struct buf		*bq;
	int s;

	LASSERT(BUF_LOCK_HOLDER(bp));
	vp = bp->b_rvp;
	if (vp == (struct vnode *) 0)
		panic("brelvp: NULL");
	/*
	 * Delete from old vnode list, if on one.
	 */
	s = splbio();
	VN_BUFLISTS_LOCK(vp);
	if (bp->b_blockb) {
		if (bq = bp->b_blockf)
			bq->b_blockb = bp->b_blockb;
		*bp->b_blockb = bq;
		bp->b_blockf = NULL;
		bp->b_blockb = NULL;
	}
	VN_BUFLISTS_UNLOCK(vp);
	splx(s);
	bp->b_rvp = bp->b_vp = (struct vnode *) 0;
	HOLDRELE(vp);
}

/*
 * Reassign a buffer from one vnode to another.
 * Used to assign file specific control information
 * (indirect blocks) to the vnode to which they belong.
 */
reassignbuf(bp, newvp)
	register struct buf *bp;
	register struct vnode *newvp;
{
	register struct buf	*bq, **listheadp;
	register struct vnode	*vp;
	register		 s;

	LASSERT(BUF_LOCK_HOLDER(bp));
	if (newvp == NULL)
		panic("reassignbuf: NULL");
	vp = bp->b_rvp;
	/*
	 * Delete from old vnode list, if on one.
	 */
	if (vp != (struct vnode *) 0) {
		ASSERT(bp->b_blockb != 0);
		s = splbio();
		VN_BUFLISTS_LOCK(vp);
		if (bq = bp->b_blockf)
			bq->b_blockb = bp->b_blockb;
		*bp->b_blockb = bq;
		VN_BUFLISTS_UNLOCK(vp);
		splx(s);
	}
	/*
	 * If dirty, put on list of dirty buffers;
	 * otherwise insert onto list of clean buffers.
	 */
	if (bp->b_flags & B_DELWRI)
		listheadp = &newvp->v_dirtyblkhd;
	else
		listheadp = &newvp->v_cleanblkhd;
	s = splbio();
	VN_BUFLISTS_LOCK(newvp);
	if (*listheadp) {
		bp->b_blockf = *listheadp;
		bp->b_blockb = listheadp;
		bp->b_blockf->b_blockb = &bp->b_blockf;
		*listheadp = bp;
	} else {
		*listheadp = bp;
		bp->b_blockb = listheadp;
		bp->b_blockf = NULL;
	}
	VN_BUFLISTS_UNLOCK(newvp);
	splx(s);
	bp->b_rvp = newvp;
}


/*
 * Release space associated with a buffer.  Why isn't this a macro? - XXX
 */
bfree(bp)
	struct buf *bp;
{
	bp->b_bcount = 0;
}

/*
 * Expand or contract the actual memory allocated to a buffer.
 * If no memory is available, release buffer and take error exit
 */
void
allocbuf(tp, size)
	register struct buf *tp;
	int size;
{
	register struct buf *bp, *ep, *dp;
	int sizealloc, take, locked;
	int s, buflocked = 0;

	LASSERT(BUF_LOCK_HOLDER(tp));
	sizealloc = round_page(size);
	/*
	 * Buffer size is not changing
	 */
	if (sizealloc == tp->b_bufsize) {
		tp->b_bcount = size;
		return;
	}
	/*
	 * Buffer size is shrinking.  Place excess space in a
	 * buffer header taken from the BQ_EMPTY buffer list and
	 * placed on the "most free" list. If no extra buffer
	 * headers are available, leave the extra space in the
	 * present buffer.
	 */
	if (sizealloc < tp->b_bufsize) {
		s = splbio();
		BFREE_LOCK();
		/*
		 * Walk the empty list avoiding locked buffers. On a
		 * uniprocessor, this loop will terminate at the
		 * first buffer on the freelist.
		 */
		dp = &bfreelist[BQ_EMPTY];
		for (ep = dp->av_forw; ep != dp; ep = ep->av_forw) {
			BUF_LOCK_TRY(ep, locked);
			if (!locked) {
				buflocked++;
				continue;
			}
			goto got_one;
		}
		if (ep == dp) {
			BFREE_UNLOCK();
			splx(s);
			tp->b_bcount = size;
			return;
		}
got_one:
		bremfree(ep);
		BFREE_UNLOCK();
		splx(s);
		if (buflocked)
			BUF_STATS(bio_stats.allocbuf_buflocked += buflocked);
		pagemove(tp->b_un.b_addr + sizealloc, ep->b_un.b_addr,
		    (int)tp->b_bufsize - sizealloc);
		ep->b_bufsize = tp->b_bufsize - sizealloc;
		tp->b_bufsize = sizealloc;
		ep->b_flags |= B_INVAL;
		ep->b_bcount = 0;
		LASSERT(BUF_LOCK_HOLDER(ep));
		brelse(ep);
		LASSERT(BUF_LOCK_HOLDER(tp));
		tp->b_bcount = size;
		return;
	}
	/*
	 * More buffer space is needed. Get it out of buffers on
	 * the "most free" list, placing the empty headers on the
	 * BQ_EMPTY buffer header list.
	 */
	while (tp->b_bufsize < sizealloc) {
		take = sizealloc - tp->b_bufsize;
		bp = getnewbuf();
		LASSERT(BUF_LOCK_HOLDER(bp));
		if (take >= bp->b_bufsize)
			take = bp->b_bufsize;
		pagemove(&bp->b_un.b_addr[bp->b_bufsize - take],
		    &tp->b_un.b_addr[tp->b_bufsize], take);
		tp->b_bufsize += take;
		bp->b_bufsize = bp->b_bufsize - take;
		if (bp->b_bcount > bp->b_bufsize)
			bp->b_bcount = bp->b_bufsize;
		if (bp->b_bufsize <= 0) {
			bp->b_dev = (dev_t)NODEV;
			bp->b_error = 0;
			bp->b_flags |= B_INVAL;
		}
		brelse(bp);
	}
	LASSERT(BUF_LOCK_HOLDER(tp));
	tp->b_bcount = size;
}

/*
 * Initialize the buffer I/O system by initializing buffer cache
 * hash links, freeing all buffers and setting all device buffer
 * lists to empty.  Assumes uniprocessor mode (boot-time).
 */
void
bio_init()
{
	register struct buf *bp, *dp;
	register struct bufhd *bhp;
	register int i;
	int base, residual;
	extern task_t first_task;

	for (bhp = bufhash, i = 0; i < bufhsz; i++, bhp++) {
		BHASH_LOCK_INIT(bhp);
		bhp->b_forw = bhp->b_back = (struct buf *)bhp;
		BHASH_STAMP(bhp) = 0;
	}
	BFREE_LOCK_INIT();
	for (dp = bfreelist; dp < &bfreelist[BQUEUES]; dp++) {
		dp->b_forw = dp->b_back = dp->av_forw = dp->av_back = dp;
		dp->b_flags = B_HEAD;
	}
	base = bufpages / nbuf;
	residual = bufpages % nbuf;
	for (i = 0; i < nbuf; i++) {
		bp = &buf[i];
		event_init(&bp->b_iocomplete);
		bp->b_hash_chain = BHASH_NULL;
		bp->b_dev = NODEV;
		bp->b_bcount = 0;
		bp->b_rcred = NOCRED;
		bp->b_wcred = NOCRED;
		bp->b_dirtyoff = 0;
		bp->b_dirtyend = 0;
		bp->b_iodone = NULL;
#if	EXL
/* The actual allocation is one page per buffer.	--- csy	*/
		bp->b_un.b_addr = buffers + i * PAGE_SIZE;
#else
	        bp->b_un.b_addr = buffers + i * MAXBSIZE;
#endif
 		if (i < residual)
			bp->b_bufsize = (base + 1) * page_size;
		else
			bp->b_bufsize = base * page_size;
		bp->b_flags = B_INVAL;
		BUF_LOCKINIT(bp);
		BUF_LOCK(bp);
		brelse(bp);
	}
}

/*
 * Print out statistics on the current allocation of the buffer pool.
 * Can be enabled to print out on every ``sync'' by setting "syncprt"
 * above.
 */
void
bufstats()
{
	int i, j, count;
	register struct buf *bp, *dp;
#ifdef	__alpha
	int counts[MAXBSIZE/8192+1];
#else
	int counts[MAXBSIZE/CLBYTES+1];
#endif
	static char *bname[BQUEUES] = { "LOCKED", "LRU", "AGE", "EMPTY" };
	int s;

	for (bp = bfreelist, i = 0; bp < &bfreelist[BQUEUES]; bp++, i++) {
		count = 0;
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			counts[j] = 0;
		s = splbio();
		BFREE_LOCK();
		for (dp = bp->av_forw; dp != bp; dp = dp->av_forw) {
			counts[dp->b_bufsize/CLBYTES]++;
			count++;
		}
		BFREE_UNLOCK();
		splx(s);
		printf("%s: total-%d", bname[i], count);
		for (j = 0; j <= MAXBSIZE/CLBYTES; j++)
			if (counts[j] != 0)
				printf(", %d-%d", j * CLBYTES, counts[j]);
		printf("\n");
	}
}

#if	MACH_LDEBUG
valid_buf_on_chain(bp, dp)
struct buf *bp;
struct buf *dp;
{
	struct bufhd *dpchk;

	if (!BUF_LOCK_HOLDER(bp)) {
		printf("corruption:  unlocked bp 0x%x\n", bp);
		return 0;
	}
	dpchk = (struct bufhd *) dp;
	if (dpchk < bufhash || dpchk >= bufhash + bufhsz) {
		printf("corruption:  dp 0x%x out of range\n", dpchk);
		return 0;
	}
	if (((int)dpchk-(int)bufhash) % sizeof(struct bufhd) != 0) {
		printf("corruption:  dp 0x%x off-center\n", dpchk);
		return 0;
	}
	if (bp < buf || bp >= buf + nbuf) {
		printf("corruption:  bp 0x%x out of range\n", bp);
		return 0;
	}
	if (((int)bp - (int)buf) % sizeof(struct buf) != 0) {
		printf("corruption:  bp 0x%x off-center\n", bp);
		return 0;
	}
	return valid_chain(dp, bp);
}


valid_chain(dp, bp)
struct buf *dp, *bp;
{
	struct buf *bpchk;
	struct buf *last_forw, *last_bpchk;
	int found_bp;

	found_bp = 0;
	last_forw = dp->b_forw;
	last_bpchk = dp;

	for (bpchk = dp->b_forw; bpchk != dp; bpchk = bpchk->b_forw) {
		if (bpchk == bp)
			++found_bp;
		if (last_forw != bpchk) {
			printf("corruption:  bp 0x%x but last_forw 0x%x\n",
			       bpchk, last_forw);
			return 0;
		}
		if (bpchk->b_back != last_bpchk) {
			printf("corruption:  bp 0x%x, b_back 0x%x, last_bpchk 0x%x\n",
			       bpchk, bpchk->b_back, last_bpchk);
			return 0;
		}
		if (bpchk->b_forw->b_back != bpchk) {
			printf("corruption:  bp 0x%x b_forw 0x%x b_forw_back 0x%x\n",
			       bpchk, bpchk->b_forw, bpchk->b_forw->b_back);
			return 0;
		}
		if (bpchk->b_back->b_forw != bpchk) {
			printf("corruption:  bp 0x%x b_back 0x%x b_back_forw 0x%x\n",
			       bpchk, bpchk->b_back, bpchk->b_back->b_forw);
			return 0;
		}
		last_forw = bpchk->b_forw;
		last_bpchk = bpchk;
	}
	if ((bp && !found_bp) || (!bp && found_bp)) {
		printf("corruption:  bp 0x%x not on chain 0x%x\n", bp, dp);
		return 0;
	}
	return 1;
}
#endif	/* MACH_LDEBUG */
