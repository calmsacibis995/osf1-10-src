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
static char *rcsid = "@(#)$RCSfile: spd_cache.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1992/06/03 14:58:37 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 *
 * Security Policy Decision Cache Manipulation Routines
 *
 * This module maintains the decision cache for each security policy.
 * There are only three externally callable routines.
 *
 * dcache_init()   boot-time initialization of a decision cache
 * dcache_lookup() attempts to retrieve a decision from the cache
 * dcache_insert() inserts a decision into the cache.
 *
 * Callers whose dcache_lookup() operations fail are expected to
 * ask the appropriate policy daemon to make the decision for them.
 * The caller must then call dcache_insert() to insert the decision
 * in the cache for the benefit of subsequent callers.
 */

/* #ident "@(#)spd_cache.c	2.1 16:07:35 4/20/90 SecureWare" */
/* #ident	"@(#)spd_cache.c	2.7 15:25:11 9/25/89 SecureWare"*/

#include <sec/include_sec>

#if SEC_ARCH /*{*/

/*	Cache initialization flag */

static int	spd_cache_init = 0;		/* initialization state */
static int	spd_mem_alloc_done = 0;		/* once only memory alloc */

/*	Array of Pointers to Security Policy Cache Blocks	*/

static dec_cache_t	**dcachep;

/*
 * Configuration information and stats for each decision cache...
 * Eventually, there should probably be an ioctl to fetch this information.
 */

static struct dcache_info {
	int	entries;	/* # of hash table slots */
/*
 * Stats of interest only to those doing performance studies...
 * These counters are 'vague stats' (not locked on MP systems)
 */
	ulong	hits;		/* cache lookup hits    */
	ulong	misses;		/* cache lookup misses  */
	ulong	inserts;	/* cache inserts	*/
} *dcache_info;


/*
 * Cache lookup and insert operations must be atomic.
 * Each hash slot contains a spin lock that is used to protect all
 * buckets within that slot.  Please observe the following rules:
 * 1) Don't hold more than one dcache slot lock at a time.
 * 2) Don't take any other locks (of any type) while holding a slot lock.
 * 3) Don't pend with a dcache slot lock held.
 */

static dcache_slot_lock(policy, slot)
register unsigned policy, slot;
{
	dec_cache_t *dcache = dcachep[policy];
	simple_lock(&dcache[slot].lock);
}

static dcache_slot_unlock(policy, slot)
register unsigned policy, slot;
{
	dec_cache_t *dcache = dcachep[policy];
	simple_unlock(&dcache[slot].lock);
}

/*
 * dcache_tag_match() - returns a Boolean to indicate whether the
 * cache entry pointed to by dcep contains a valid entry for
 * the tag pair <tag1,tag2>. The caller is assumed to have
 * locked the hash slot in which the bucket pointed to by
 * dcep resides.  dcep is a struct dcache_entry *.
 * tag1 and tag2 are of type tag_t.
 */

#define dcache_tag_match(dcep, tag1, tag2)		\
  (((dcep) -> dc_flags.valid) &&			\
   ((dcep) -> tag1 == (tag1)) &&			\
   ((dcep) -> tag2 == (tag2)))

/*
 * Return a Boolean to indicate whether the decision cache
 * for the specified policy has been initialized.  The order
 * of the tests is critical, since memory for the dcachep array
 * is allocated at runtime, and is only guaranteed to exist after
 * spd_cache_init is set.
 */

#define dcache_initialized(policy) (spd_cache_init && dcachep[(policy)])

/*
 * dcache_init()-create and initialize the decision cache for
 * a security policy. This requires allocating the memory for
 * control structures and the decisions and setting up the
 * control structures to indicate all entries are initially
 * invalid.  This is called only by the SPIOC_INIT ioctl code.
 * Use of spioc_init_lock (see SPIOC_INIT) prevents a number of
 * possible race conditions in both uniprocessor and multiprocessor
 * environments.
 */

dcache_init(policy, entries, width)
unsigned policy, entries, width;
{
	dec_cache_t *dcache;
	char *decision;
	register int bytes, i, j;

	/*
	 * Initialize cache control structures for all policies
	 * if no policy daemon has yet initialized them.
	 */

	if (spd_mem_alloc_done == 0) {

	   bytes = (spolicy_cnt * sizeof(struct dcache_info)) +
	      (spolicy_cnt * sizeof(dec_cache_t *));

	   dcachep = (dec_cache_t **) kalloc(bytes);
	   if (dcachep == (dec_cache_t **) 0) {
		printf("dcache_init: unable to allocate dcachep\n");
		return(ENOMEM);
	   }

	   dcache_info = (struct dcache_info *) ((ulong) dcachep +
		(spolicy_cnt * sizeof(dec_cache_t *)));
	   /*
	    * Initialize the dcachep[] array and each info structure
	    */

	   for(i=0; i < spolicy_cnt; i++) {
		dcachep[i] = (dec_cache_t *) 0;
		bzero(&dcache_info[i], sizeof(struct dcache_info));
	   }

	   spd_mem_alloc_done = 1; 
	}

	/*
	 * The cache for each policy daemon is only supposed to be
	 * initialized once each time the system is booted.  Refuse
	 * any duplicate requests.
	 */

	if (dcache_info[policy].entries)
	    return(EALREADY);

	bytes = (entries * DEC_CACHE_ENTRY_SIZE) +
		(entries * DCACHE_BUCKETS * width);

	/*
	 * Allocate system memory for the cache.
	 * If this fails, we don't want to free the previously
	 * allocated memory, because it may still be useful to
	 * a previously initialized policy.
	 */

	dcache = (dec_cache_t *)kalloc(bytes);
	if (dcache == (dec_cache_t *)0) {
		printf("dcache_init: unable to allocate cache space\n");
		return(ENOMEM);
	}

	/*
	 * Invalidate all entries, init locks, and link decision to bucket
	 * Remaining fields are a "don't care" as long as valid bit clear
	 */

	decision = (char *)((int) dcache + (entries * DEC_CACHE_ENTRY_SIZE));

	for(i=0; i < entries; i++) {
	   dec_cache_t *dp;
	   dp = &dcache[i];
	   for(j=0; j < DCACHE_BUCKETS; j++) {
		dp -> bucket[j].dc_flags.valid = 0;
		dp -> bucket[j].decision = decision;
		decision = (char *) ((int)decision + width);
	   }
	   simple_lock_init(&dp -> lock);
	}

	dcache_info[policy].entries = entries;	/* size of cache */

	/*
	 * Order of the following operations is important, so that the
	 * dcache_initialized() macro works properly, especially on
	 * MP systems.  Once dcachep is set, this macro assumes that
	 * the cache is ready for use.  spd_cache_init must not be set
	 * until the dcachep array is allocated and initialized.
	 */

	dcachep[policy] = dcache;		/* where cache is */

	/*
	 * This really only needs to be set by the first caller,
	 * but it's easier to prevent races by waiting until
	 * here to do it, rather than doing it in the once-only code.
	 */

	spd_cache_init = 1;
	return(0);
}


/*
 * dcache_hash()-hash two tags to a cache slot
 * This may only be called after dcache_init() has been
 * called for the specified policy.
 * PORT NOTE: Right shifts aren't guaranteed to zero fill!
 */

static dcache_hash(policy, tag1, tag2)
register unsigned policy;
register tag_t tag1, tag2;
{
	register unsigned long hash = ((tag1 ^ tag2) << 8) >> 16;
	return(hash % dcache_info[policy].entries);
}

/*
 * dcache_insert()-make a new decision cache entry for the two
 * tags and the accompanying decision. The tags are hashed to
 * identify the cache slot and a bucket is picked based on an
 * LRU entry. The decision is copied to the decision buffer for
 * that bucket.
 */

dcache_insert(policy, tag1, tag2, dec, dec_length)
register unsigned policy;
register tag_t tag1, tag2;
char *dec;
unsigned dec_length;
{
  	register struct dcache_entry *dcep;
	register dec_cache_t *dcache;
	time_t lru_time;
	int lru_index, free_index, i, slot, bucket;


	if (!dcache_initialized(policy)) /* if there is no cache yet */ 
	    return;			 /* forget about inserting   */

	dcache_info[policy].inserts++;	/* vague stats */

	/*
	 * Search to avoid replicating valid entries and also select
	 * the LRU bucket to steal if no free entries exist.
	 */

	lru_time = 0x7fffffff;
	lru_index = -1;
	free_index = -1;

	dcache = dcachep[policy]; 
	slot = dcache_hash(policy, tag1, tag2);
	dcache_slot_lock(policy, slot);

	for(i=0; i < DCACHE_BUCKETS; i++) {

		dcep = &dcache[slot].bucket[i];

		/*
		 * Due to possible races between dcache_lookup() and
		 * dcache_insert() operations, another thread may have
		 * inserted the entry that our caller just failed to
		 * find with dcache_lookup().  If the entry is already
		 * in the cache, just update its "last used" time and exit.
		 * This is NOT a design flaw.  Any attribute changes would
		 * have caused the tags to change.  As long as the cache
		 * contains an entry that matches both tags, we know that
		 * the cached decision for that entry is the same decision
		 * that we're trying to insert now, so the race is harmless.
		 */ 

		if (dcache_tag_match(dcep, tag1, tag2))
			goto insert_done;

		/*
		 * If this cache entry isn't valid, it's an ideal
		 * candidate to be replaced.
		 */

		if (dcep -> dc_flags.valid == 0) {
			free_index = i;
			break;
		}

		/*
		 * Keep a running record of the oldest entry in
		 * this bucket in case no entry in this slot is invalid.
		 */

		if (dcep -> time < lru_time) {
			lru_time = dcep -> time;
			lru_index = i;
		}
	}

	/* Choose a bucket in the slot to use for the new entry. */

	if (free_index == -1)
		bucket = lru_index;
	else bucket = free_index;

	if (bucket < 0)
		bucket = 0;
	dcep = &dcache[slot].bucket[bucket];

	/* Insert decision into the chosen cache bucket and mark entry valid */

	dcep -> tag1 = tag1;
	dcep -> tag2 = tag2;
	bcopy(dec, dcep->decision, dec_length);
	dcep -> dc_flags.valid = 1;	   /* mark it valid */

insert_done:

	/*
	 * The entry is in the cache, inserted either by us or by some
	 * previous caller of dcache_insert().  Either way, we need to
	 * update its "last use" time and  unlock the slot in which the
	 * inserted entry resides.
	 */

	dcep -> time = SYSTEM_TIME;	   /* for later LRU decisions */
	dcache_slot_unlock(policy, slot);  /* cache is sane again */
}

/*
 * dcache_lookup()-perform a decision cache lookup based on the two
 * tag values supplied. Return 0 if the decision is not in the cache
 * or 1 if the decision is.  Copy the decision into the caller-supplied
 * buffer.
 */

int
dcache_lookup(policy, tag1, tag2, dec, dec_length)
register unsigned policy;
register tag_t tag1, tag2;
char *dec;
int dec_length;
{
	dec_cache_t *dcache;
	struct dcache_entry *dcep;
	int slot, i;


	if (!dcache_initialized(policy)) /* if there is no cache there */
	    return(0);			 /* can't possibly  be a hit   */

	/*
	 * Search the buckets in the determined slot for a valid cache
	 * entry with matching tag values. If found then update the
	 * LRU time for the entry and give caller a copy of the decision
	 */

	dcache = dcachep[policy];
	slot = dcache_hash(policy, tag1, tag2);
	dcache_slot_lock(policy, slot);

	for(i=0; i < DCACHE_BUCKETS; i++) {

		dcep = &dcache[slot].bucket[i];
		if (dcache_tag_match(dcep, tag1, tag2)) { /* cache hit */
		    dcep -> time = SYSTEM_TIME; /* for LRU algorithm */
		    bcopy(dcep -> decision, dec, dec_length);
		    dcache_slot_unlock(policy, slot);
		    dcache_info[policy].hits++; /* vague stats */
		    return(1);
		}
	}

	dcache_slot_unlock(policy, slot);
	dcache_info[policy].misses++;	/* vague stats */
	return(0);			/* cache miss */
}

#endif /*}*/
