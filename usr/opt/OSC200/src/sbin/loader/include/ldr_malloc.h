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
 *	@(#)$RCSfile: ldr_malloc.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:36:29 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_malloc.h
 * declarations for loader memory allocator
 * Depends on "ldr_types.h" and "standards.h"
 *
 * OSF/1 Release 1.0.1
 */

#ifndef	_H_LDR_MALLOC
#define	_H_LDR_MALLOC


typedef	short	ldr_malloc_t;

#define	OPEN_HASH_TABLE_T	((ldr_malloc_t)1)
#define	CHAIN_HASH_TABLE_T	((ldr_malloc_t)2)
#define LDR_MAPPING_T		((ldr_malloc_t)3)
#define LDR_WINDOW_T		((ldr_malloc_t)4)
#define LDR_MODULE_REC_T	((ldr_malloc_t)5)
#define LDR_REGION_REC_T	((ldr_malloc_t)6)
#define LDR_CONTEXT_T		((ldr_malloc_t)7)
#define LDR_STRING_T		((ldr_malloc_t)8)
#define LDR_MACHO_T		((ldr_malloc_t)9)
#define LDR_COFF_T		((ldr_malloc_t)10)
#define LDR_AOUT_T		((ldr_malloc_t)11)
#define MALLOC_T		((ldr_malloc_t)12)
#define LDR_KPT_REC_T		((ldr_malloc_t)13)
#define LDR_PACKAGE_REC_T	((ldr_malloc_t)14)
#define LDR_SYMBOL_REC_T	((ldr_malloc_t)15)
#define LDR_SWITCH_LINKS_T	((ldr_malloc_t)16)
#define LDR_RCOFF_T		((ldr_malloc_t)17)

/* These macros are the standard allocation and free procedures, from
 * the process heap.
 *
 * extern int ldr_malloc(size_t nbytes, ldr_malloc_t type, univ_t *ptr);
 */

#define	ldr_malloc(nb, t, ptr)	(ldr_heap_malloc(ldr_process_heap, (nb), (t), (ptr)))

/* extern int ldr_free(univ_t cp);
 */

#define ldr_free(cp)	(ldr_heap_free(ldr_process_heap, (cp)))

/* extern int ldr_realloc(univ_t *ptr, size_t nbytes);
 */

#define ldr_realloc(p, n) (ldr_heap_realloc(ldr_process_heap, (p), (n)))


/* These are the general-purpose heap allocation and free routines.
 * First argument is a heap, which must have been initialized by
 * calling ldr_heap_init()
 */

extern int
ldr_heap_malloc __((ldr_heap_t heap, size_t nbytes, ldr_malloc_t type,
		    univ_t *ptr));

extern int
ldr_heap_free __((ldr_heap_t heap, univ_t cp));


extern int
ldr_heap_realloc __((ldr_heap_t heap, univ_t *ptr, size_t nbytes));


/* Initialize a heap for future mallocs.  Arguments are:
 * - starting address for heap base (may be NULL for "anywhere")
 * - a file descriptor to map heap space from (may be LDR_NO_FILE to get
 *   anonymous space)
 * - flags to pass to ldr_mmap() when growing the heap.  Useful flags are:
 *   LDR_MAP_FILE		to map space from a file
 *   LDR_MAP_ANON		to map anonymous space
 *   LDR_MAP_FIXED		to make heap contiguous in virtual space
 *   LDR_MAP_INHERIT		to make heap keep-on-exec
 *   LDR_MAP_SHARED		to make heap shared with child (note: caller
 *				is responsible for locking!)
 *   LDR_MAP_PRIVATE		to make heap private
 * - starting offset in specified file to map space for heap (must be
 *   page-aligned; should be 0 if anon)
 * Returns newly-initialized heap in *heap.  Returns LDR_SUCCESS on success
 * or negative error status on error.
 */

extern int
ldr_heap_create __((univ_t addr, ldr_file_t fd, int flags, int offset,
		    ldr_heap_t *heap));


/* Initialize the standard process heap */

extern int
ldr_heap_init __((void));

/* Try to inherit a heap, located at the specified address, from a parent
 * process via file or keep-on-exec mapping.  Validate the heap header, and
 * the heap size.  Currently assumes that the heap must be contiguous in
 * VA space (ie. mapped MAP_FIXED), and so all addresses from the
 * heap start to the next address must be valid.  Prot is the protection
 * with which the heap must be mapped.
 * Returns LDR_SUCCESS on success, negative error status on failure.
 */

extern int
ldr_heap_inherit __((ldr_heap_t heap, int prot));

/* Return the total size in bytes occupied by the specified heap.
 * This routine can only be applied to a contiguous heap (ie a
 * heap mapped with MAP_FIXED), as it simply returns the difference
 * between the next heap address and the base of the heap.
 * Returns 0 on error (eg. heap not contiguous).
 */

extern size_t
ldr_heap_size __((ldr_heap_t heap));

#endif	/* _H_LDR_MALLOC */
