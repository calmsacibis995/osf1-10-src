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
 *	@(#)$RCSfile: open_hash.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:37:29 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* open_hash.h
 * external declarations for symbol table hashing functions
 * Depends on <standards.h>, "ldr_hash.h"
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_OPEN_HASH
#define _H_OPEN_HASH

typedef	univ_t	open_hashtab_t;		/* opaque def for hash table (should be void *) */

typedef	int	open_hash_element_index; /* index of element in hash table */

/* Hash table flags */

typedef	int	open_hash_flags_t;	/* flags for tables, from: */

#define	OPEN_HASH_REBALANCE	((open_hash_flags_t)1) /* rebalance table on insert */


/* Insert the specified (key, value) pair into the specified hash table.
 * No error if the key already exists in the table (this may result in
 * duplicate keys; use open_hash_search(LDR_HASH_INSERT|LDR_HASH_LOOKUP)
 * if duplicate keys are possible).  Return negative error status on error.
 * extern int open_hash_insert(open_hashtab_t table, const univ_t key, univ_t *value);
 */

#define	open_hash_insert(table, key, value) \
	open_hash_search((table), (key), (value), LDR_HASH_INSERT)

/* Lookup the specified key in the specified hash table, returning the 
 * value in *value.  Returns negative error status if the key is not
 * in the table.
 * extern int open_hash_lookup(open_hashtab_t table, const univ_t key, univ_t *value);
 */

#define	open_hash_lookup(table, key, value) \
	open_hash_search((table), (key), (value), LDR_HASH_LOOKUP)


/* Create a hash table large enough to hold at least nelem elements.
 * Returns zero on sucess or a negative error status on error.
 */

#define open_hash_create(n, h, c, f, t) \
	open_hash_create_heap(ldr_process_heap, (n), (h), (c), (f), (t))

/* Destroy a hash table.  Returns zero on success or a negative error
 * status on error.  Free the storage to the specified heap.
 * Note that this procedure does not do anything about freeing the
 * key/value pairs; it assumes that the table is empty or that this
 * has been otherwise taken care of.
 */

#define open_hash_destroy(o) \
	open_hash_destroy_heap(ldr_process_heap, (o))

/* Grow the specified hash table to be big enough to handle at least
 * nelem elements, and return the pointer to the new table in *table.
 * Deallocates the old table.  Returns 0 on success or negative error
 * status on error.
 */

#define open_hash_resize(h, n) \
	open_hash_resize_heap(ldr_process_heap, (h), (n))


/* Create a hash table large enough to hold at least nelem elements.  Allocate
 * the table from the specified heap.
 * Return a negative error status on error.
 */

extern int
open_hash_create_heap __((ldr_heap_t heap, int nelem, ldr_hash_p hasher,
			  ldr_hash_compare_p comper, open_hash_flags_t flags,
			  open_hashtab_t *table));

/* Search a hash table for a specified key.  If HASH_INSERT
 * is specified and the key is not present, the (key, value) pair is
 * inserted into the table.  If HASH_LOOKUP is specified and the key is
 * present, the value is returned.  (See the descriptions of ldr_hash_action
 * in "hash.h" for a detailed description of the behavior).  Returns zero on
 * success or a negative error status on error.
 */

extern int
open_hash_search __((open_hashtab_t table, const univ_t key, univ_t *value,
		     ldr_hash_action action));

/* Iterate through the elements of a hash table in an unspecified order.
 * Set *index to 0 to initialize the iteration; hash_elements() will modify
 * it to identify the next element to be returned on each call.  On success,
 * *str is set to the key of the next element, *value is set to its value.
 * Returns 0 on success, LDR_EAGAIN when there are no more elements, or other
 * negative error status on error.
 */

extern int
open_hash_elements __((open_hashtab_t table, open_hash_element_index *ix,
		       univ_t *key, univ_t *value));

/* Destroy a hash table.  Returns zero on success or a negative error
 * status on error.  Free the storage to the specified heap.
 * Note that this procedure does not do anything about freeing the
 * key/value pairs; it assumes that the table is empty or that this
 * has been otherwise taken care of.
 */

extern int
open_hash_destroy_heap __((ldr_heap_t heap, open_hashtab_t table));

/* Grow the specified hash table to be big enough to handle at least
 * nelem elements, and return the pointer to the new table in *table.
 * Use the specified heap for allocations and deallocations.
 * Deallocates the old table.  Returns 0 on success or negative error
 * status on error.
 */

extern int
open_hash_resize_heap __((ldr_heap_t heap, open_hashtab_t *table, int nelem));

/* Try to inherit the specified open hash table, presumably from a
 * keep-on-exec region or mapped file.  Only thing to do is to verify
 * the addresses of the procedure pointers in the table.
 * Returns LDR_SUCCESS on success or negative error status on error.
 */

extern int
open_hash_inherit __((open_hashtab_t table, ldr_hash_p hasher,
		      ldr_hash_compare_p comper));

#endif /* _H_OPEN_HASH */
