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
static char	sccsid[] = "@(#)$RCSfile: open_hash.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/12/07 16:21:06 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* open_hash.c
 * Symbol table hashing functions
 *
 * OSF/1 Release 1.0
 */

/* TODO:
 *  - write the rebalance routine
 */

#include <sys/types.h>
#ifndef _NO_PROTO
#include <loader.h>
#endif

#include <ldr_main_types.h>

#include "ldr_types.h"
#include "ldr_malloc.h"
#include "ldr_errno.h"
#include "ldr_hash.h"
#include "open_hash.h"
#include "open_hash_pvt.h"

#ifndef	ENOSYM
#undef LDR_ENOSYM
#define	LDR_ENOSYM	ldr_errno_to_status(ENOENT)
#endif /* ENOSYM */

#ifndef	EVERSION
#undef LDR_EVERSION
#define LDR_EVERSION	ldr_errno_to_status(EINVAL)
#endif /* EVERSION */

#if __STDC__
static unsigned hash_rebalance(int_hashtab_t *tab, unsigned hashval, int probe, int nprobe);
#else
static unsigned hash_rebalance();
#endif


int
#if __STDC__
open_hash_create_heap(ldr_heap_t heap, int nelem, ldr_hash_p hasher,
		      ldr_hash_compare_p comper, open_hash_flags_t flags,
		      open_hashtab_t *table)
#else
open_hash_create_heap(heap, nelem, hasher, comper, flags, table)
ldr_heap_t heap;
int nelem;
ldr_hash_p hasher;
ldr_hash_compare_p comper;
open_hash_flags_t flags;
open_hashtab_t *table;
#endif

/* Create a hash table large enough to hold at least nelem elements.  Allocate
 * the table from the specified heap.
 * Return a negative error status on error.
 */
{
	size_t		tsize;
	int_hashtab_t	*tab;
	int		rc;

	if ((nelem = open_hash_nelem(nelem)) <= 0)
		return(LDR_EINVAL);
	tsize = hash_table_size(nelem);

	if ((rc = ldr_heap_malloc(heap, tsize, OPEN_HASH_TABLE_T, (univ_t *)&tab)) != LDR_SUCCESS)
		return(rc);

	bzero(tab, tsize);
	tab->h_maxelem = nelem;
	tab->h_nelem = 0;
	tab->h_hasher = hasher;
	tab->h_comper = comper;
	tab->h_flags = flags;
	*table = (open_hashtab_t)tab;
	return(LDR_SUCCESS);
}


int
#if __STDC__
open_hash_destroy_heap(ldr_heap_t heap, open_hashtab_t table)
#else
open_hash_destroy_heap(heap, table)
ldr_heap_t heap;
open_hashtab_t table;
#endif

/* Destroy a hash table.  Returns zero on success or a negative error
 * status on error.  Free the storage to the specified heap.
 * Note that this procedure does not do anything about freeing the
 * key/value pairs; it assumes that the table is empty or that this
 * has been otherwise taken care of.
 */
{
	int_hashtab_t	*tab = (int_hashtab_t *)table;

	return(ldr_heap_free(heap, tab));
}


int
#ifdef __STDC__
open_hash_search (open_hashtab_t table, const univ_t key, univ_t *value,
		  ldr_hash_action action)
#else
open_hash_search (table, key, value, action)
open_hashtab_t table;
univ_t key;
univ_t *value;
ldr_hash_action action;
#endif

/* Search a hash table for a specified key.  If HASH_INSERT
 * is specified and the key is not present, the (key, value) pair is
 * inserted into the table.  If HASH_LOOKUP is specified and the key is
 * present, the value is returned.  (See the descriptions of ldr_hash_action
 * in "hash.h" for a detailed description of the behavior).  Returns zero on
 * success or a negative error status on error.
 *
 * The algorithm used here is open addressing, using the division method
 * for computing the original hash, and quadratic rehashing for resolving
 * collisions.  In addition, we use Brent's algorithm for improving lookup
 * performance at the cost of increased time to insert an element (see the
 * internal procedure hash_rebalance() below).  See Knuth Vol. 3, Section
 * 6.4 for details.
 */
{
	register int_hashtab_t	*tab = (int_hashtab_t *)table;
	register int		probe;	/* index of current probe into table */
	int			nprobe;	/* number of probes we've tried so far */
	unsigned		hsh;	/* hash encoding of this string */
	int			ins_dup_ok; /* no duplicate check on insert? */
	register hashtab_entry	*ent;

	ins_dup_ok = (action & (LDR_HASH_INSERT|LDR_HASH_LOOKUP)) == LDR_HASH_INSERT;
#ifdef DO_NOT_USE_PROC_PTRS
	hsh = hash_string(key, tab->h_maxelem);
#else
	hsh = (*tab->h_hasher)(key, tab->h_maxelem);
#endif
	if (hsh == 0)
		hsh = 1;

	for (probe = 0, nprobe = 0; nprobe < tab->h_maxelem; nprobe++) {

		/* Compute next probe, using quadratic rehash */

		if ((probe += hsh) >= tab->h_maxelem)
			probe -= tab->h_maxelem;

		if (tab->h_entries[probe].key == NULL) {

			/* Key was not found; see if we're allowed to insert */

			if (!(action & LDR_HASH_INSERT))
				return(LDR_ENOSYM);

			/* Now see if we can improve average lookup performance
			 * by rebalancing the tree, using Brent's algorithm.
			 */

			if (tab->h_flags & OPEN_HASH_REBALANCE)
				probe = hash_rebalance(tab, hsh, probe, nprobe);

			/* Insert it at specified location */

			tab->h_entries[probe].key = key;
			tab->h_entries[probe].value = *value;
			tab->h_nelem++;
			return(LDR_SUCCESS);

		} else if (ins_dup_ok) {

			/* Inserting without duplicate check; just probe again */

			continue;

#ifdef DO_NOT_USE_PROC_PTRS
		} else if (strcmp(key, tab->h_entries[probe].key) == 0) {
#else
		} else if ((*tab->h_comper)(key, tab->h_entries[probe].key) == 0) {
#endif
			/* Found the key; return value if not just inserting */

			if (!(action & LDR_HASH_LOOKUP)) {
				return(LDR_EEXIST);
			} else {
				*value = tab->h_entries[probe].value;
				return(LDR_SUCCESS);
			}
		}

	}

	/* If we get here, the table is full.  If inserting, show no space;
	 * otherwise it's just a failed lookup.
	 */

	if (!(action & LDR_HASH_INSERT))
		return(LDR_ENOSYM);
	else
		return(LDR_ENOMEM);
}



static unsigned
#if __STDC__
hash_rebalance(int_hashtab_t *tab, unsigned hashval, int probe, int nprobe)
#else
hash_rebalance(tab, hashval, probe, nprobe)
int_hashtab_t *tab;
unsigned hashval;
int probe;
int nprobe;
#endif

/* Use Brent's algorithm to attempt to rebalance the specified hash
 * table, to improve lookup performance (at the cost of increased
 * insert time).
 * NOT YET IMPLEMENTED.
 */
{
	return(probe);
}


int
#if __STDC__
open_hash_elements (open_hashtab_t table, open_hash_element_index *ix,
		    univ_t *key, univ_t *value)
#else
open_hash_elements (table, ix, key, value)
open_hashtab_t table;
open_hash_element_index *ix;
univ_t *key;
univ_t *value;
#endif

/* Iterate through the elements of a hash table in an unspecified order.
 * Set *index to 0 to initialize the iteration; hash_elements() will modify
 * it to identify the next element to be returned on each call.  On success,
 * *key is set to the key of the next element, *value is set to its value.
 * Returns 0 on success, LDR_EAGAIN when there are no more elements, or other
 * negative error status on error.
 */
{
	register int_hashtab_t	*tab = (int_hashtab_t *)table;
	register int	i;

	for (i = *ix; i < tab->h_maxelem; i++) {

		if (tab->h_entries[i].key != NULL) {

			*key = tab->h_entries[i].key;
			*value = tab->h_entries[i].value;
			*ix = i + 1;
			return(LDR_SUCCESS);
		}
	}

	*ix = 0;
	return(LDR_EAGAIN);
}


int
#if __STDC__
open_hash_resize_heap(ldr_heap_t heap, open_hashtab_t *table, int nelem)
#else
open_hash_resize_heap(heap, table, nelem)
ldr_heap_t heap;
open_hashtab_t *table;
int nelem;
#endif

/* Grow the specified hash table to be big enough to handle at least
 * nelem elements, and return the pointer to the new table in *table.
 * Use the specified heap for allocations and deallocations.
 * Deallocates the old table.  Returns 0 on success or negative error
 * status on error.
 */
{
	register int_hashtab_t	*tab = (int_hashtab_t *)*table;
	open_hashtab_t		new_table;
	register int_hashtab_t	*newtab;
	int			rc;
	open_hash_element_index	ix;
	univ_t			key;
	univ_t			value;

	if (nelem <= tab->h_maxelem)	/* if shrinking just return */
		return(0);

	if ((rc = open_hash_create_heap(heap, nelem, tab->h_hasher, tab->h_comper,
				   tab->h_flags, &new_table)) != LDR_SUCCESS)
		return(rc);

	newtab = (int_hashtab_t *)new_table;
	for (ix = 0; (rc = open_hash_elements(tab, &ix, &key, &value)) >= 0; ) {
		if ((rc = open_hash_search(newtab, key, &value, LDR_HASH_INSERT)) != LDR_SUCCESS) {
			open_hash_destroy_heap(heap, newtab);
			return(rc);
		}
	}

	open_hash_destroy_heap(heap, *table);
	*table = (open_hashtab_t)newtab;
	return(LDR_SUCCESS);
}


int
#if __STDC__
open_hash_inherit(open_hashtab_t table, ldr_hash_p hasher,
		  ldr_hash_compare_p comper)
#else
open_hash_inherit(table, hasher, comper)
open_hashtab_t table;
ldr_hash_p hasher;
ldr_hash_compare_p comper;
#endif

/* Try to inherit the specified open hash table, presumably from a
 * keep-on-exec region or mapped file.  Only thing to do is to verify
 * the addresses of the procedure pointers in the table.
 * Returns LDR_SUCCESS on success or negative error status on error.
 */
{
	register int_hashtab_t	*tab = (int_hashtab_t *)table;

#ifndef DO_NOT_USE_PROC_PTRS
	if ((tab->h_hasher != hasher) ||
	    (tab->h_comper != comper))
		return(LDR_EVERSION);
#endif
	return(LDR_SUCCESS);
}
