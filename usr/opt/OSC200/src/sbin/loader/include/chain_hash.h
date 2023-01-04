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
/*	
 *	@(#)$RCSfile: chain_hash.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:36:37 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* chain_hash.h
 * external declarations for symbol table hashing functions
 * depends on <standards.h>, "ldr_hash.h" 
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_CHAIN_HASH
#define _H_CHAIN_HASH

typedef	univ_t	chain_hashtab_t;	/* opaque def for hash table */

/* The fundamental structure of a chained hash-table element.
 * Each element inserted into a chained hash table MUST begin
 * with a link field followed by the key.  The search operation
 * takes a pointer to this type.
 */

typedef struct chain_hash_elem {
	struct chain_hash_elem	*che_next; /* next element in chain */
	univ_t			che_key; /* key of this element */
} chain_hash_elem;

/* Insert the specified element into the specified hash table.
 * No error if the key already exists in the table (this may result in
 * duplicate keys; use open_hash_search(LDR_HASH_INSERT|LDR_HASH_LOOKUP)
 * if duplicate keys are possible).  Return negative status on error.
 * extern int chain_hash_insert(chain_hashtab_t table, const univ_t key,
 *				chain_hash_elem **elem);
 */

#define	chain_hash_insert(table, key, elem) \
	chain_hash_search((table), (key), (elem), LDR_HASH_INSERT)

/* Lookup the specified key in the specified hash table, returning the 
 * element in *elem.  Returns negative error status if the key is not
 * in the table.
 * extern int chain_hash_lookup(chain_hashtab_t table, const univ_t key,
 *				chain_hash_elem **elem);
 */

#define	chain_hash_lookup(table, key, elem) \
	chain_hash_search((table), (key), (elem), LDR_HASH_LOOKUP)

/* Delete the element with the specified key from the specified hash table.
 * Return a negative error status if the key does not exist in the
 * table.
 * extern int chain_hash_delete(chain_hashtab_t table, const univ_t key);
 */

#define	chain_hash_delete(table, key) \
	chain_hash_search((table), (key), NULL, LDR_HASH_DELETE)

/* Create a hash table with nelem hash slots.
 * Returns zero on sucess or a negative error status on error.
 */

#define chain_hash_create(n, h, c, t) \
	chain_hash_create_heap(ldr_process_heap, (n), (h), (c), (t))

/* Create a hash table with nelem hash slots, using the specified functions
 * for hashing a key and for comparing two keys.  Allocate the table from
 * the specified heap.
 * Returns zero on sucess or a negative error status on error.
 */

extern int
chain_hash_create_heap __((ldr_heap_t heap, int nelem, ldr_hash_p hasher,
			   ldr_hash_compare_p comper, chain_hashtab_t *table));

/* Search a hash table for a specified key.  If HASH_INSERT
 * is specified and the key is not present, the element is
 * inserted into the table.  If HASH_LOOKUP is specified and the key is
 * present, the element address is returned.  If HASH_DELETE 
 * is specified and the key is present, the element is deleted.
 * (See the descriptions of ldr_hash_action in "hash.h" for a detailed
 * description of the behavior).  Returns zero on success or a negative
 * error status on error.
 */

extern int
chain_hash_search __((chain_hashtab_t table, const univ_t key,
		      chain_hash_elem **elem, ldr_hash_action action));

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

extern int
chain_hash_elements __((chain_hashtab_t table, chain_hash_elem *prev,
			chain_hash_elem **elem));

/* Destroy a hash table.  Returns zero on success or a negative error
 * status on error.
 * Note that this procedure does not do anything about freeing the
 * key/value pairs; it assumes that the table is empty or that this
 * has been otherwise taken care of.
 */

#define chain_hash_destroy(t)	chain_hash_destroy_heap(ldr_process_heap, (t))

extern int
chain_hash_destroy_heap __((ldr_heap_t heap, chain_hashtab_t table));

/* Try to inherit the specified chained hash table, presumably from a
 * keep-on-exec region or mapped file.  Only thing to do is to verify
 * the addresses of the procedure pointers in the table.
 * Returns LDR_SUCCESS on success or negative error status on error.
 */

extern int
chain_hash_inherit __((chain_hashtab_t table, ldr_hash_p hasher,
		       ldr_hash_compare_p comper));

#endif /* _H_CHAIN_HASH */
