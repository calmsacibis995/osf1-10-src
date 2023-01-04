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
static char	sccsid[] = "@(#)$RCSfile: chain_hash.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/12/07 16:19:25 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* chain_hash.c
 * Hashing functions for managing chained hash tables
 *
 * OSF/1 Release 1.0
 */

#include <sys/types.h>
#include <loader.h>

#include "ldr_types.h"
#include "ldr_malloc.h"
#include "ldr_errno.h"
#include "squeue.h"

#include "ldr_hash.h"
#include "chain_hash.h"
#include "chain_hash_pvt.h"

int
chain_hash_create_heap(ldr_heap_t heap, int nelem, ldr_hash_p hasher,
		       ldr_hash_compare_p comper, chain_hashtab_t *table)

/* Create a hash table with nelem hash slots, using the specified functions
 * for hashing a key and for comparing two keys.  Allocate the table from
 * the specified heap.
 * Returns zero on sucess or a negative error status on error.
 */
{
	size_t		tsize;
	int_hashtab_t	*tab;
	int		rc;

	tsize = hash_table_size(nelem);

	if ((rc = ldr_heap_malloc(heap, tsize, CHAIN_HASH_TABLE_T,
				  (univ_t *)&tab)) != LDR_SUCCESS)
		return(rc);

	bzero(tab, tsize);
	tab->h_nelem = nelem;
	tab->h_hasher = hasher;
	tab->h_comper = comper;
	*table = (void *)tab;
	return(LDR_SUCCESS);
}


int
chain_hash_destroy_heap(ldr_heap_t heap, chain_hashtab_t table)

/* Destroy a hash table.  Returns zero on success or a negative error
 * status on error.  Free the storage into the specified heap.
 * Note that this procedure does not do anything about freeing the
 * elements; it assumes that the table is empty or that this
 * has been otherwise taken care of.
 */
{
	int_hashtab_t	*tab = table;

	return(ldr_heap_free(heap, tab));
}


int
chain_hash_search(chain_hashtab_t table, const univ_t key,
		  chain_hash_elem **elem, ldr_hash_action action)

/* Search a hash table for a specified key.  If HASH_INSERT
 * is specified and the key is not present, the element is
 * inserted into the table.  If HASH_LOOKUP is specified and the key is
 * present, the element address is returned.  If HASH_DELETE 
 * is specified and the key is present, the element is deleted.
 * (See the descriptions of ldr_hash_action in "hash.h" for a detailed
 * description of the behavior).  Returns zero on success or a negative
 * error status on error.
 *
 * The algorithm used here is chained hashing, using the division
 * method for computing the original hash and chaining through the
 * elements.  See Knuth Vol. 3, Section 6.4 for details.
 */
{
	register int_hashtab_t	*tab = table;
	unsigned		hsh;	/* hash encoding of this string */
	register chain_hash_elem *prev;
	register chain_hash_elem *cur;
	int			ins_dup_ok; /* no duplicate check on insert? */

	ins_dup_ok = (action & (LDR_HASH_INSERT|LDR_HASH_LOOKUP)) == LDR_HASH_INSERT;
#ifdef DO_NOT_USE_PROC_PTRS
	hsh = hash_string(key, tab->h_nelem);
#else
	hsh = (*tab->h_hasher)(key, tab->h_nelem);
#endif

	if (ins_dup_ok) {
		sq1_ins_head(&(tab->h_entries[hsh]), *elem);
		return(LDR_SUCCESS);
	}

	for (prev = (chain_hash_elem *)&(tab->h_entries[hsh]), cur = prev->che_next;
	     cur != NULL;
	     prev = cur, cur = cur->che_next) {
		
#ifdef DO_NOT_USE_PROC_PTRS
		if (strcmp(key, cur->che_key) == 0) {
#else
		if ((*tab->h_comper)(key, cur->che_key) == 0) {
#endif			
			/* Found element; error if not looking up or deleting */
			
			if ((action & (LDR_HASH_LOOKUP|LDR_HASH_DELETE)) == 0)
				return(LDR_EEXIST);
			if (action & LDR_HASH_LOOKUP)
				*elem = cur;
			if (action & LDR_HASH_DELETE)
				sq1_rem_after(cur, prev);
			return(LDR_SUCCESS);
		}
	}

	/* element not found; if inserting, put it in.  prev is pointer to
	 * last element in chain.  Note that we always insert at the end.
	 */

	if (!(action & LDR_HASH_INSERT)) {
		return(LDR_ENOSYM);
	} else {
		sq1_ins_after(*elem, prev);
		return(LDR_SUCCESS);
	}
}


int
chain_hash_elements(chain_hashtab_t table, chain_hash_elem *prev,
		    chain_hash_elem **elem)

/* Iterate through the elements of a hash table in an unspecified
 * order.  Set prev to NULL to initialize the iteration;
 * chain_hash_elements() will modify *elem to point to the next
 * element on each call.  Returns 0 on success, LDR_EAGAIN when there
 * are no more elements, or other negative error status on error.
 * So, to iterate through the entire table using this routine:
 * 	for (prev = NULL, rc = 0; rc == 0; prev = elem)
 *		rc = chain_hash_elements(table, prev, &elem)
 *
 * To use this routine for emptying a hash table (eg. to delete it),
 * do the following:
 *	while (chain_hash_elements(table, NULL, &elem) >= 0)
 *		chain_hash_delete(table, elem->che_key);
 */
{
	register int_hashtab_t	*tab = table;
	unsigned		hsh;	/* bucket being searched */

	if (prev != NULL) {

		/* Just try for next element in this bucket */

		*elem = prev->che_next;
		if (*elem != NULL)	/* success, just return */
			return(LDR_SUCCESS);

		/* No more in this bucket; figure out the next bucket */

#ifdef DO_NOT_USE_PROC_PTRS
		hsh = hash_string(prev->che_key, tab->h_nelem) + 1;
#else
		hsh = (*tab->h_hasher)(prev->che_key, tab->h_nelem) + 1;
#endif
	} else {

		/* Starting at the beginning; initial bucket is zero */

		hsh = 0;
	}

	for ( ; hsh < tab->h_nelem; hsh++) {

		if (tab->h_entries[hsh] != NULL) {

			/* Something in this bucket; return it */

			*elem = tab->h_entries[hsh];
			return(LDR_SUCCESS);
		}
	}

	/* No more elements */

	return (LDR_EAGAIN);
}


int
chain_hash_inherit(chain_hashtab_t table, ldr_hash_p hasher,
		   ldr_hash_compare_p comper)

/* Try to inherit the specified chained hash table, presumably from a
 * keep-on-exec region or mapped file.  Only thing to do is to verify
 * the addresses of the procedure pointers in the table.
 * Returns LDR_SUCCESS on success or negative error status on error.
 */
{
	register int_hashtab_t	*tab = table;

#ifndef DO_NOT_USE_PROC_PTRS
	if ((tab->h_hasher != hasher) ||
	    (tab->h_comper != comper))
		return(LDR_EVERSION);
#endif
	return(LDR_SUCCESS);
}
