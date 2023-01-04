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
static char	*sccsid = "@(#)$RCSfile: vfs_cache.c,v $ $Revision: 4.2.12.2 $ (DEC) $Date: 1993/08/02 14:55:56 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
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
 *	@(#)vfs_cache.c	7.4 (Berkeley) 8/25/89
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/namei.h>
#include <sys/errno.h>

/*
 * The name cache table and its size are initialized in
 * machine-dependent code.
 */
struct	namecache *namecache;
int	nchsize;
u_long	nextvnodeid;

/*
 * Name caching works as follows:
 *
 * Names found by directory scans are retained in a cache
 * for future reference.  It is managed LRU, so frequently
 * used names will hang around.  Cache is indexed by hash value
 * obtained from (vp, name) where vp refers to the directory
 * containing name.
 *
 * For simplicity (and economy of storage), names longer than
 * a maximum length of NCHNAMLEN are not cached; they occur
 * infrequently in any case, and are almost never of interest.
 *
 * Upon reaching the last segment of a path, if the reference
 * is for DELETE, or NOCACHE is set (rewrite), and the
 * name is located in the cache, it will be dropped.
 */

/*
 * Structures associated with name cacheing.  Initialized
 * in machine-dependent code.
 */
struct nchash	*nchash;		/* base of namei cache hash table */

/*
 * This macro depends on nchsz being a power of two.
 */
#define NHASH(vp, h)    ((((unsigned)(h) >> 6) + (h)) & ((nchsz)-1))

/*
 * Define the locks needed.  The hash chain lock has precedence over
 * the freelist lock.  
 *
 * There is no lock on each namecache entry.  It is not needed as there are
 * only two ways a namecache entry can be used:
 *
 *	o	while holding some other apppropriate lock, like the hash
 *		chain lock.
 *
 *	o	while it is impossible that another processor can use this
 *		entry at the same time (i.e. we are not on any hash chain nor
 *		the freelist).
 */
#define NCH_LOCK(nchp)		usimple_lock(&(nchp)->nch_lock)
#define NCH_LOCK_TRY(nchp)	usimple_lock_try(&(nchp)->nch_lock)
#define NCH_UNLOCK(nchp)	usimple_unlock(&(nchp)->nch_lock)
#define NCH_LOCK_INIT(nchp)	usimple_lock_init(&(nchp)->nch_lock)

udecl_simple_lock_data(, nc_freelist_lock)
#define	NC_FREE_LOCK()		usimple_lock(&nc_freelist_lock)
#define NC_FREE_UNLOCK()	usimple_unlock(&nc_freelist_lock)
#define NC_FREE_LOCK_INIT()	usimple_lock_init(&nc_freelist_lock)

udecl_simple_lock_data(, nc_purge_lock)
#define	NC_PURGE_LOCK()		usimple_lock(&nc_purge_lock)
#define NC_PURGE_UNLOCK()	usimple_unlock(&nc_purge_lock)
#define NC_PURGE_LOCK_INIT()	usimple_lock_init(&nc_purge_lock)
#if	UNIX_LOCKS
int	nc_purging;
#endif

struct	namecache *nchhead, **nchtail;	/* LRU chain pointers */
struct	nchstats nchstats;		/* cache effectiveness statistics */

#define NC_STATS_LOCK_INIT()    usimple_lock_init(&nchstats.ncs_lock)

int nch_check(struct nchash *nhp, struct namecache *ncp, struct nameidata *ndp); 
int doingcache = 1;			/* 1 => enable the cache */

/*
 * Look for the name in the cache. We don't do this
 * if the segment name is long, simply so the cache can avoid
 * holding long names (which would either waste space, or
 * add greatly to the complexity).
 *
 * Lookup is called with ni_dvp pointing to the directory to search,
 * ni_ptr pointing to the name of the entry being sought, ni_namelen
 * tells the length of the name, and ni_hash contains a hash of
 * the name. If the lookup succeeds, the vnode is returned in ni_vp
 * and a status of -1 is returned. If the lookup determines that
 * the name does not exist (negative cacheing), a status of ENOENT
 * is returned. If the lookup fails, a status of zero is returned.
 */
cache_lookup(ndp)
	register struct nameidata *ndp;
{
	register struct vnode *dvp;
	register struct namecache *ncp;
	struct nchash *nhp;
	int flag, error;

	if (!doingcache)
		return (0);
	if (ndp->ni_namelen > NCHNAMLEN) {
		NC_STATS(nchstats.ncs_long++);
		ndp->ni_makeentry = 0;
		return (0);
	}
	dvp = ndp->ni_dvp;
	nhp = &nchash[NHASH(dvp, ndp->ni_hash)];
	NCH_LOCK(nhp);
	for (ncp = nhp->nch_forw; ncp != (struct namecache *)nhp;
	    ncp = ncp->nc_forw) {
		if (ncp->nc_dvp == dvp &&
		    ncp->nc_dvpid == dvp->v_id &&
		    ncp->nc_nlen == ndp->ni_namelen &&
		    !bcmp(ncp->nc_name, ndp->ni_ptr, (unsigned)ncp->nc_nlen))
			break;
	}
	ndp->ni_nchtimestamp = nhp->nch_timestamp;
	if (ncp == (struct namecache *)nhp) {
		NCH_UNLOCK(nhp);
		NC_STATS(nchstats.ncs_miss++);
		return (0);
	}
	/*
	 * We got a cache hit, but there's more work to do.
	 * Special cases:
	 * If renaming (RENAME), we want to force a real lookup for the
	 * target, regardless of a positive or negative cache hit.  If
	 * creating (CREATE), we only do this for negative hits, since
	 * positive hits don't result in any changes to the directory.
	 * If the lookup were not forced, filesystems like UFS could 
	 * assume incorrectly that directory state has been saved when 
	 * it tries to write the new directory entry.
	 */
	flag = (ndp->ni_nameiop & OPFLAG);
	if ((!ndp->ni_makeentry) || (flag == RENAME)) {
		NC_STATS(nchstats.ncs_badhits++);
	} else {
		/*
		 * Got a good hit.  Check for negative, positive, and
		 * false (i.e. recycled vnodes with old or bad v_id).
		 */
		if (ncp->nc_vp == NULL) {
			NC_STATS(nchstats.ncs_neghits++);
			if (flag == CREATE)
				goto remove_entry;
			error = ENOENT;
		} else if (ncp->nc_vpid != ncp->nc_vp->v_id) {
			/* old vnode; get rid of entry */
			NC_STATS(nchstats.ncs_falsehits++);
			goto remove_entry;
		} else {
			NC_STATS(nchstats.ncs_goodhits++);
			ndp->ni_vp = ncp->nc_vp;
			error = -1;
#if	UNIX_LOCKS
			/*
			 * Save the capability id while holding the 
			 * hash chain lock
			 */
			ndp->ni_vpid = ncp->nc_vpid;
#endif
		}
		/*
		 * move this slot to end of LRU chain, if not already there
		 */
		NC_FREE_LOCK();
		NCH_UNLOCK(nhp);
		if (ncp->nc_nxt) {
			/* remove from LRU chain */
			*ncp->nc_prev = ncp->nc_nxt;
			ncp->nc_nxt->nc_prev = ncp->nc_prev;
			/* and replace at end of it */
			ncp->nc_nxt = NULL;
			ncp->nc_prev = nchtail;
			*nchtail = ncp;
			nchtail = &ncp->nc_nxt;
		}
		NC_FREE_UNLOCK();
		return (error);
	}

remove_entry:
	/*
	 * Last component and we are renaming or deleting,
	 * the cache entry is invalid, or otherwise don't
	 * want cache entry to exist.
	 */
	/* remove from LRU chain */
	NC_FREE_LOCK();
	*ncp->nc_prev = ncp->nc_nxt;
	if (ncp->nc_nxt)
		ncp->nc_nxt->nc_prev = ncp->nc_prev;
	else
		nchtail = ncp->nc_prev;
	/* remove from hash chain */
	remque(ncp);
#if	UNIX_LOCKS
	ncp->nc_hash_chain = NCH_NULL;
#endif
	/* and make a dummy hash chain */
	ncp->nc_forw = ncp;
	ncp->nc_back = ncp;
	NCH_UNLOCK(nhp);
	/* insert at head of LRU list (first to grab) */
	ncp->nc_nxt = nchhead;
	ncp->nc_prev = &nchhead;
	nchhead->nc_prev = &ncp->nc_nxt;
	nchhead = ncp;
	NC_FREE_UNLOCK();
	return (0);
}

/*
 * Add an entry to the cache
 */
/*
 * There is an issue here.  It is possible that the cache purge counter
 * could have rolled over between the time cache_lookup found the vnode and
 * its capability, and the time we get into cache_enter.  We will then have
 * stale data in the cache.  This might also be a problem in the uniprocessor
 * case.
 *
 * Synchronization:  We may not be able to just
 * take the first entry off the freelist.  When we are trying to grab a free
 * entry, it must first be removed from the hash chain it might be on.  If it
 * is on a hash chain (ncp->nc_hash_chain != NULL), that hash chain lock must
 * be acquired before removing it.  However, the hash chain lock has precedence
 * over the freelist lock, so we must do a lock_try.  If we cannot get it,
 * we have to continue down the freelist until we find an entry that is either
 * not on a hash chain, or is on a hash chain we can acquire the lock for.
 *
 * After we have the entry off the freelist and the hash chain, it is ours to
 * set up and no one else can find it on any list.  We set it up with no locks
 * held.  After setting it up, we have to check the hash chain again, to make
 * sure we aren't racing another thread to add the same entry.  If we don't
 * find a duplicate entry on the hash chain, we put ourselves on it, and 
 * at the end of the freelist.  If we DO find a duplicate entry, ours is now
 * bogus.  We put ours back on the freelist, at the head, with a null hash
 * chain.
 */
cache_enter(ndp)
	register struct nameidata *ndp;
{
	register struct namecache *ncp, *ncp2;
	register struct vnode *dvp;
	struct nchash *nhp;

	/* the namelen check is a bit of paranoia, but it's cheap */
	if ((!doingcache)|| (ndp->ni_namelen > NCHNAMLEN))
		return;
	/*
	 * Find a free cache entry
	 */
#if	UNIX_LOCKS
	NC_FREE_LOCK();
	for (ncp = nchhead; ncp != NULL; ncp = ncp->nc_nxt) {
		if (ncp->nc_hash_chain != NCH_NULL) {
			if (!NCH_LOCK_TRY(ncp->nc_hash_chain))
				continue;
			/*
			 * Remove from old hash chain
			 */
			remque(ncp);
			NCH_UNLOCK(ncp->nc_hash_chain);
		}
		/*
		 * Remove from freelist
		 */
		*ncp->nc_prev = ncp->nc_nxt;
		if (ncp->nc_nxt)
			ncp->nc_nxt->nc_prev = ncp->nc_prev;
		else
			nchtail = ncp->nc_prev;
		break;
	}
	NC_FREE_UNLOCK();
	if (ncp == NULL)
		return;
#else	/* UNIX_LOCKS */
	if (ncp = nchhead) {
		/*
		 * Remove from lru chain
		 */
		*ncp->nc_prev = ncp->nc_nxt;
		if (ncp->nc_nxt)
			ncp->nc_nxt->nc_prev = ncp->nc_prev;
		else
			nchtail = ncp->nc_prev;
		/* remove from old hash chain */
		remque(ncp);
	} else
		return;
#endif	/* UNIX_LOCKS */

	/* grab the inode we just found */
	ncp->nc_vp = ndp->ni_vp;
	if (ndp->ni_vp)
		ncp->nc_vpid = ndp->ni_vp->v_id;
	else
		ncp->nc_vpid = 0;
	/*
	 * Fill in cache info
	 */
	ncp->nc_dvp = ndp->ni_dvp;
	ncp->nc_dvpid = ndp->ni_dvp->v_id;
	ncp->nc_nlen = ndp->ni_namelen;
	bcopy(ndp->ni_ptr, ncp->nc_name, (unsigned) ncp->nc_nlen);
	nhp = &nchash[NHASH(ndp->ni_vp, ndp->ni_hash)];
#if	UNIX_LOCKS
	ncp->nc_hash_chain = nhp;
#endif
	NCH_LOCK(nhp);
	if (nch_check(nhp, ncp, ndp)) {
		NCH_UNLOCK(nhp);
		return;
	}
	/*
	 * Insert on new hash chain
	 */
	insque(ncp, nhp);
	nhp->nch_timestamp++;
	/*
	 * Insert at back of LRU free list
	 */
	NC_FREE_LOCK();
	ncp->nc_nxt = NULL;
	ncp->nc_prev = nchtail;
	*nchtail = ncp;
	nchtail = &ncp->nc_nxt;
	NC_FREE_UNLOCK();
	NCH_UNLOCK(nhp);
}

int
nch_check(struct nchash *nhp, struct namecache *ncp, struct nameidata *ndp) 
{
	register struct namecache *ncp2;
	int ret = 0;
	/*
	 * Deal with races and duplicate entries.
	 * If an entry matches ours, use it; otherwise, use ours.
	 */
	if (ndp->ni_nchtimestamp != nhp->nch_timestamp) {
		for (ncp2 = nhp->nch_forw; ncp2 != (struct namecache *)nhp;
		     ncp2 = ncp2->nc_forw) {
			if (ncp2->nc_dvp == ndp->ni_dvp &&
			    ncp2->nc_dvpid == ndp->ni_dvp->v_id &&
			    ncp2->nc_nlen == ndp->ni_namelen &&
			    !bcmp(ncp2->nc_name,ndp->ni_ptr,
					(unsigned)ncp2->nc_nlen)) {
				/*
				 * a hit.  If it's a dup positive or
				 * negative, release our entry.  If it's 
				 * different from ours, use ours.
				 */
				if (ncp2->nc_vp == ndp->ni_vp) {
					/*
					 * Duplicate; put us back on free list
					 */
					ret = 1;
					ncp2 = ncp;
					NC_FREE_LOCK();
				} else {
					/*
					 * Unequal dup exists; remove it.
					 * This code is copied from the
					 * end of cache_lookup().
					 */
					NC_FREE_LOCK();
					*ncp2->nc_prev = ncp2->nc_nxt;
					if (ncp2->nc_nxt)
						ncp2->nc_nxt->nc_prev = 
								ncp2->nc_prev;
					else
						nchtail = ncp2->nc_prev;
					/* remove from hash chain */
					remque(ncp2);
				}
#if	UNIX_LOCKS
				ncp2->nc_hash_chain = NCH_NULL;
#endif
				/* make a dummy hash chain */
				ncp2->nc_forw = ncp2->nc_back = ncp2;
				ncp2->nc_nxt = nchhead;
				ncp2->nc_prev = &nchhead;
				nchhead->nc_prev = &ncp2->nc_nxt;
				nchhead = ncp2;
				NC_FREE_UNLOCK();
				break;
			}
		}
	}
	return (ret);
}

/*
 * Name cache initialization, from main() when we are booting
 */
nchinit()
{
	register struct nchash *nchp;
	register struct namecache *ncp;

	nchhead = 0;
	nchtail = &nchhead;
	ASSERT((nchsz & nchsz-1) == 0);
	for (ncp = namecache; ncp < &namecache[nchsize]; ncp++) {
		ncp->nc_forw = ncp;			/* hash chain */
		ncp->nc_back = ncp;
		ncp->nc_nxt = NULL;			/* lru chain */
		*nchtail = ncp;
		ncp->nc_prev = nchtail;
		nchtail = &ncp->nc_nxt;
#if	UNIX_LOCKS
		ncp->nc_hash_chain = NCH_NULL;
#endif
		/* all else is zero already */
	}
	for (nchp = nchash; nchp < &nchash[nchsz]; nchp++) {
		nchp->nch_head[0] = nchp;
		nchp->nch_head[1] = nchp;
		NCH_LOCK_INIT(nchp);
	}
	NC_FREE_LOCK_INIT();
	NC_PURGE_LOCK_INIT();
	NC_STATS_LOCK_INIT();
}

/*
 * Cache flush, a particular vnode; called when a vnode is renamed to
 * hide entries that would now be invalid
 */
#if	UNIX_LOCKS
/*
 * Currently, thread_wakeup will just return if there are no threads waiting
 * (on nc_purging, in this case).  If we wanted to save the overhead of the
 * call we could set a bit (like the high bit) in nc_purging to
 * indicate that there is a thread actually blocking.  We don't bother with
 * this since the counter won't roll over all that often.
 */
#endif
cache_purge(vp)
	struct vnode *vp;
{
	struct namecache *ncp;

	NC_PURGE_LOCK();
#if	UNIX_LOCKS
	while (nc_purging) {
		assert_wait((vm_offset_t) &nc_purging, FALSE);
		NC_PURGE_UNLOCK();
		thread_block();
		NC_PURGE_LOCK();
	}
#endif
	vp->v_id = ++nextvnodeid;
	if (nextvnodeid != 0) {
		NC_PURGE_UNLOCK();
		return;
	}
#if	UNIX_LOCKS
	nc_purging = 1;
#endif
	NC_PURGE_UNLOCK();
	for (ncp = namecache; ncp < &namecache[nchsize]; ncp++) {
		ncp->nc_vpid = 0;
		ncp->nc_dvpid = 0;
	}
	NC_PURGE_LOCK();
	vp->v_id = ++nextvnodeid;
#if	UNIX_LOCKS
	nc_purging = 0;
#endif
	NC_PURGE_UNLOCK();
#if	UNIX_LOCKS
	thread_wakeup((vm_offset_t) &nc_purging);
#endif
}

/*
 * Cache flush, a whole filesystem; called when filesys is umounted to
 * remove entries that would now be invalid
 *
 * In the multiprocessor case, we have to traverse down the hash chains
 * rather than the freelist.  We will also do this for the uniprocessor
 * case because the performance difference doesn't seem to warrant
 * supporting the two separate cases.
 *
 * It is possible here that we will hold the hash chain lock for a fairly
 * long period of time.  However, we only do this on unmounts, so it shouldn't
 * be a large performance hit during normal operation of the system.
 */
cache_purgevfs(mp)
	register struct mount *mp;
{
	register struct namecache *ncp, *nextncp;
	struct nchash *nhp;
	int i;

	for (i = 0; i < nchsz; i++) {
		nhp = &nchash[i];
		NCH_LOCK(nhp);
		for (ncp = nhp->nch_forw; ncp != (struct namecache *)nhp;
		     ncp = nextncp) {
			nextncp = ncp->nc_forw;
			if (ncp->nc_dvp == NULL || ncp->nc_dvp->v_mount != mp)
				continue;
			/*
			 * Free the resources we had
			 */
			remque(ncp);
			ncp->nc_vp = NULL;
			ncp->nc_dvp = NULL;
			ncp->nc_forw = ncp;
			ncp->nc_back = ncp;
#if	UNIX_LOCKS
			ncp->nc_hash_chain = NCH_NULL;
#endif
			NC_FREE_LOCK();
			/*
			 * Delete from free list
			 */
			*ncp->nc_prev = ncp->nc_nxt;
			if (ncp->nc_nxt)
				ncp->nc_nxt->nc_prev = ncp->nc_prev;
			else
				nchtail = ncp->nc_prev;
			/*
			 * Put the now-free entry at head of LRU
			 */
			ncp->nc_nxt = nchhead;
			ncp->nc_prev = &nchhead;
			nchhead->nc_prev = &ncp->nc_nxt;
			nchhead = ncp;
			NC_FREE_UNLOCK();
		}
		NCH_UNLOCK(nhp);
	}
}
