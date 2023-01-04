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
 * @(#)$RCSfile: heap_kmem.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/05/05 13:13:26 $
 */
#ifndef	__HEAP_KMEM__
#define	__HEAP_KMEM__ 1
#ifdef	KERNEL

extern struct h_kmem_info *
	h_kmem_alloc_init(/* register vm_size_t space, boolean_t canwait */);

/*
 * Default heap allocator
 */

extern struct h_kmem_info *h_kmem;
extern struct h_kmem_info *h_kmem_alloc_init();

#define h_kmem_alloc(N) 					\
		h_kmem_alloc_memory_(h_kmem, (N), TRUE)

#define h_kmem_alloc_memory(N, WAIT)				\
		h_kmem_alloc_memory_(h_kmem, (N), (WAIT))

#define	h_kmem_free(PTR, PTRSIZE) 				\
		h_kmem_free_memory_(h_kmem, (PTR), 		\
			(PTRSIZE), TRUE)

#define	h_kmem_free_memory(PTR, PTRSIZE, WAIT)			\
		h_kmem_free_memory_(h_kmem, (PTR), 		\
			(PTRSIZE), (WAIT))

#define	h_kmem_zalloc(N)					\
		h_kmem_zalloc_memory_(h_kmem, (N), TRUE)

#define	h_kmem_zalloc_memory(N, WAIT)				\
		h_kmem_zalloc_memory_(h_kmem, (N), (WAIT))

#define h_kmem_fast_alloc(PTR, PTRSIZE, CHUNKS)			\
		h_kmem_fast_alloc_memory_(h_kmem,(PTR), 	\
			(PTRSIZE), (CHUNKS), TRUE)	

#define	h_kmem_fast_alloc_memory(PTR, PTRSIZE, CHUNKS, WAIT)	\
		h_kmem_fast_alloc_memory_(h_kmem, (PTR),	\
			(PTRSIZE), (CHUNKS), (WAIT))

#define h_kmem_fast_zalloc_(PTR, PTRSIZE, CHUNKS)		\
		h_kmem_fast_zalloc_memory_(h_kmem, (PTR), 	\
			(PTRSIZE), (CHUNKS), TRUE)	

#define	h_kmem_fast_zalloc_memory(PTR, PTRSIZE, CHUNKS, WAIT)	\
		h_kmem_fast_zalloc_memory_(h_kmem, (PTR),	\
			(PTRSIZE), (CHUNKS), (WAIT))

#endif	/* KERNEL */
#endif	/* !__HEAP_KMEM__ */
