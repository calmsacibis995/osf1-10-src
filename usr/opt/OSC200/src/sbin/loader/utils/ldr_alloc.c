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
static char	sccsid[] = "@(#)$RCSfile: ldr_alloc.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/12/07 16:20:04 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *	loader memory allocater hacked from 4.3 BSD malloc.c
 *
 *	NOTE: The principal change to the BSD code is that anonymous
 *	ldr_mmap is used in place of sbrk() to get storage from the system.
 *
 * OSF/1 Release 1.0
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-4 (or 2^n-10) bytes long.
 * This is designed for use in a virtual memory environment.
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <loader.h>

#include "ldr_types.h"		/* added for loader */
#include "ldr_malloc.h"
#include "ldr_errno.h"
#include "ldr_sys_int.h"

#define DEBUG
/*
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled then a second word holds the size of the
 * requested block, less 1, rounded up to a multiple of sizeof(RMAGIC).
 * The order of elements is critical: ov_magic must overlay the low order
 * bits of ov_next, and ov_magic can not be a valid ov_next bit pattern.
 */
union	overhead {
	union	overhead *ov_next;	/* when free */
	struct {
		u_char	ovu_magic;	/* magic number */
		u_char	ovu_index;	/* bucket # */
		ldr_malloc_t ovu_type;	/* type of ldr blk allocated */
#ifdef RCHECK
		u_short	ovu_rmagic;	/* range magic number */
		u_int	ovu_size;	/* actual block size */
#endif
	} ovu;
#define	ov_magic	ovu.ovu_magic
#define	ov_index	ovu.ovu_index
#define	ov_type		ovu.ovu_type
#define	ov_rmagic	ovu.ovu_rmagic
#define	ov_size		ovu.ovu_size
};

#define	MAGIC		0xef		/* magic # on accounting info */
#define RMAGIC		0x5555		/* magic # on range info */
#define HEAP_MAGIC	0x5a5a5a5a	/* magic # on heap proper */

#ifdef RCHECK
#define	RSLOP		sizeof (u_short)
#else
#define	RSLOP		0
#endif

/* This is the internal definition of a heap.  A heap is a self-contained,
 * growable storage pool.  The heap is grown by using ldr_mmap(), using
 * the file descriptor and map flags specified at the time the heap is
 * created.  There is a single distinguished heap, the process heap,
 * which is initialized during process bootstrap.
 *
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define	NBUCKETS 30

typedef	struct int_heap {
	unsigned	magic;		/* magic number for heap */
	univ_t		next_addr;	/* next address for heap growth */
	ldr_file_t	fd;		/* file for heap growth */
	int		flags;		/* flags for heap growth */
	off_t		next_offset;	/* next offset in file */
	union overhead *nextf[NBUCKETS]; /* free space pointers */
} int_heap;

ldr_heap_t	ldr_process_heap;	/* initialized by ldr_bootstrap() */

static	int pagesz;			/* page size */
static	int pagebucket;			/* page size bucket */

static void ldr_morecore(int_heap *, int);


#ifdef MSTATS
/*
 * nmalloc[i] is the number of mallocs for a given blcok size.
 * nfree[i] is the number of frees for a given block size.
 * nmorecore is the number of times ldr_morecore() is called.
 */
static	u_int nmalloc[NBUCKETS];
static  u_int nfree[NBUCKETS];
static  u_int nmorecore;
#include <stdio.h>
#endif

#if defined(DEBUG) || defined(RCHECK)
#define	ASSERT(p)   if (!(p)) botch(#p)
static
botch(char *s)
{
	ldr_puts("\nassertion botched:");
	ldr_puts(s);
	ldr_puts("\n");
	ldr_abort();
}
#else
#define	ASSERT(p)
#endif


int
ldr_heap_malloc(ldr_heap_t heap, size_t nbytes, ldr_malloc_t type, univ_t *ptr)
{
	int_heap *hp = (int_heap *)heap;
  	register union overhead *op;
  	register int bucket;
        register int f_bucket;
	register unsigned amt, n;
	int	rc;

	/*
	 * heap must be initialized before use
	 */
	ASSERT((hp != NULL) && (hp->magic == HEAP_MAGIC));

	/*
	 * Convert amount of memory requested into closest block size
	 * stored in hash buckets which satisfies request.
	 * Account for space used per block for accounting.
	 */
	n = -(sizeof (*op) + RSLOP);
	if (nbytes <= (pagesz + n)) {
#ifndef RCHECK
		amt = 8;	/* size of first bucket */
		bucket = 0;
#else
		amt = 16;	/* size of first bucket */
		bucket = 1;
#endif
	} else {
		amt = pagesz;
		bucket = pagebucket;
	}
	while (nbytes > amt + n) {
		amt <<= 1;
		if (amt == 0)
			return (LDR_EINVAL);
		bucket++;
	}
	/*
	 * If nothing in hash bucket right now,
	 * look for a larger bucket or request more memory from the system.
	 */
  	if ((op = hp->nextf[bucket]) == NULL) 
		{			/* bucket is empty */
                       /* search for a non empty bucket */
                for (f_bucket = bucket+1; f_bucket < NBUCKETS; f_bucket++)
			if ((op = hp->nextf[f_bucket]) != NULL) 
				break;	/* found one! */
                if (op == NULL) 
			{		/* need to allocate space */
  			if (bucket < pagebucket)
				f_bucket = pagebucket;
			else
				f_bucket = bucket;
			ldr_morecore(hp, f_bucket);
  			if ((op = hp->nextf[f_bucket]) == NULL) 
				{
				ldr_log("ldr_heap_malloc: error growing loader heap: %E\n",
					ldr_errno_to_status(errno));
  				return (ldr_errno_to_status(errno));
				}
			}		/* need to allocate space */
			/* remove bucket from linked list */
		hp->nextf[f_bucket] = op->ov_next;
			/* divide up larger bucket into smaller ones */
		for (f_bucket--; f_bucket >= bucket; f_bucket--) 
			{
			op->ov_next = hp->nextf[f_bucket];
			hp->nextf[f_bucket] = op;
			op = (union overhead *) ((char *)op + (1 << f_bucket + 3));
			}
		}			/* bucket is empty */
	else		/* found one exact size */
		hp->nextf[bucket] = op->ov_next; 
		/* set header info */
	op->ov_magic = MAGIC;
	op->ov_index = bucket;
#ifdef MSTATS
  	nmalloc[bucket]++;
#endif
#ifdef RCHECK
	/*
	 * Record allocated size of block and
	 * bound space with magic numbers.
	 */
	op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
	op->ov_rmagic = RMAGIC;
  	*(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
	op->ov_type = type;
	*ptr = ((univ_t)(op + 1));
  	return (LDR_SUCCESS);
}

/*
 * Allocate more memory to the indicated bucket.
 */

static void
ldr_morecore(int_heap *hp, int bucket)
{
  	union overhead *op;
	register int sz;		/* size of desired block */
	char *tp;			/* temp pointer for blk alignment */
	off_t		new_size;
	int rc;

	/*
	 * sbrk_size <= 0 only for big, FLUFFY, requests (about
	 * 2^30 bytes on a VAX, I think) or for a negative arg.
	 */
	sz = 1 << (bucket + 3);
#ifdef DEBUG
	ASSERT(sz > 0);
#else
	if (sz <= 0) {
		errno = EINVAL;
		return;
	}
#endif
#ifdef DEBUG
        ASSERT(sz >= pagesz);
#endif

	/* Try to allocate from the file descriptor specified in the
	 * heap, beginning at the current end address of the heap.
	 * Flags will be LDR_MAP_FIXED to indicate that the heap must
	 * be contiguous.
	 * Note we allocate an extra page so that user data will be page aligned.
	 */

	new_size = 0;
	if (!(hp->flags & LDR_MAP_ANON)) {

		/* Need to check file size; we may need to grow it */

		new_size = hp->next_offset + sz + pagesz;
		if ((rc = ldr_grow_file(hp->fd, new_size)) != LDR_SUCCESS)
			return;
	}

	rc = ldr_mmap(hp->next_addr, sz + pagesz, LDR_PROT_READ | LDR_PROT_WRITE,
		      hp->flags, hp->fd, hp->next_offset, (univ_t *)&op);

	/* error - perhaps no more room! */
  	if (rc != LDR_SUCCESS) {
#ifdef	DEBUG
		ldr_msg("ldr_morecore: ldr_mmap error errno = %d\n", rc);
#endif
  		return;
	}

	/* Account for new allocation in heap */
	hp->next_addr = (univ_t)((char *)(hp->next_addr) + sz + pagesz);
	hp->next_offset = new_size;

	/* to keep user block page aligned put overhead at end of first page */
	tp = (char*) op;
	tp += pagesz - sizeof(*op);
	op = (union overhead *) tp;

	/*
	 * Add new memory allocated to that on
	 * free list for this hash bucket.
	 */
  	hp->nextf[bucket] = op;
#ifdef MSTATS
        nmorecore++;
#endif
}


int
ldr_heap_free(ldr_heap_t heap, univ_t cp)
{   
	int_heap *hp = (int_heap *)heap;
  	register int size;
	register union overhead *op;

  	if (cp == NULL)
  		return(LDR_SUCCESS);
	ASSERT((hp != NULL) && (hp->magic == HEAP_MAGIC));
	op = (union overhead *)((caddr_t)cp - sizeof (union overhead));
#ifdef DEBUG
  	ASSERT(op->ov_magic == MAGIC);		/* make sure it was in use */
#else
	if (op->ov_magic != MAGIC)
		return(LDR_EINVAL);	/* sanity */
#endif
#ifdef RCHECK
  	ASSERT(op->ov_rmagic == RMAGIC);
	ASSERT(*(u_short *)((caddr_t)(op + 1) + op->ov_size) == RMAGIC);
#endif
  	size = op->ov_index;
  	ASSERT(size < NBUCKETS);
	op->ov_next = hp->nextf[size];	/* also clobbers ov_magic */
  	hp->nextf[size] = op;
#ifdef MSTATS
  	nfree[size]++;
#endif
	return(LDR_SUCCESS);
}

/*
 * This version of realloc does not support reallocing an already
 * freed block. The old malloc man page mentioned that a program
 * may attempt "storage compaction" by doing this behavior. Again,
 * it is not supported. If you decrease the size of allocated memory 
 * significantly so you can fit in a smaller block, you will not get
 * the same pointer back that you call realloc with. This is different
 * from some implementations of realloc.
 */

int
ldr_heap_realloc(ldr_heap_t heap, univ_t *ptr, size_t nbytes)
{   
	int_heap *hp = (int_heap *)heap;
  	register u_int max_alloc;	/* max allocation for current block */
  	register u_int min_alloc;	/* min allocation for current block */
  	register u_int i;
	union overhead *op;
  	univ_t res;
	int error;

  	if (*ptr == NULL)
		return (ldr_heap_malloc(heap, nbytes, MALLOC_T, ptr));
	if (nbytes == 0)
		return (ldr_heap_free(heap, *ptr));

	op = (union overhead *)((caddr_t)*ptr - sizeof (union overhead));
	if (op->ov_magic != MAGIC) 
		{		/* block is not allocated */
		return (LDR_EINVAL);    /* can't do anything */
		}		/* block is not allocated */
	
	i = op->ov_index;
	max_alloc = (1 << (i + 3)) - sizeof(*op) - RSLOP;
	if (i) 			/* not the smallest block */
		min_alloc = (1 << (i + 2)) - sizeof(*op) - RSLOP;
	else
		min_alloc = 0;
	
	if (nbytes <= max_alloc && nbytes > min_alloc) 
		{		/* new size in current block */
			/* just change size of area */
#ifdef RCHECK
		op->ov_size = (nbytes + RSLOP - 1) & ~(RSLOP - 1);
		*(u_short *)((caddr_t)(op + 1) + op->ov_size) = RMAGIC;
#endif
		return(LDR_SUCCESS);
		}		/* new size in current block */ 

	if ((error = ldr_heap_malloc(heap, nbytes, MALLOC_T, &res)) != LDR_SUCCESS)
		return(error);
	if (*ptr != res)	/* common optimization if "compacting" */
		bcopy(*ptr, res, (nbytes < max_alloc) ? nbytes : max_alloc);
	if ((error = ldr_heap_free(heap, *ptr)) != LDR_SUCCESS)
		return(error);
	*ptr = res;
	return(LDR_SUCCESS);
}

int
ldr_heap_create(univ_t addr, ldr_file_t fd, int flags, int offset,
		ldr_heap_t *heap)

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
{
	int_heap	*hp;
	int		mapflags;
	int		bucket;
	unsigned	amt;
	size_t		map_size;
	off_t		new_size;
	struct addressconf *addr_conf;
	int		rc;

	/* If address is NULL, try appropriate default address.
	 * Don't map fixed even though LDR_MAP_FIXED
	 * is on.
	 */

	mapflags = flags;
	if (addr == NULL) {
		if ((rc = ldr_getaddressconf(&addr_conf) != LDR_SUCCESS)) { /* shouldn't fail */
#ifdef DEBUG
			ASSERT(ldr_getaddressconf(&addr_conf) != LDR_SUCCESS);
#else
			return(rc);
#endif
		}
		addr = addr_conf[AC_LDR_BSS].ac_base;
		mapflags &= ~LDR_MAP_FIXED;
	}

	/* Initial allocation size will be one page (we assume that heap
	 * structure is less than one page in size!)  If this is the first
	 * time we're called, get the page size.
	 */

	if (pagesz == 0) {
		pagesz = ldr_getpagesize();
		bucket = 0;
		amt = 8;
		while (pagesz > amt) {
			amt <<= 1;
			bucket++;
		}
		pagebucket = bucket;
	}

	map_size = pagesz;

	/* Map the initial region */

	new_size = offset;
	if (!(flags & LDR_MAP_ANON)) {
			     
		/* Need to check file size; we may need to grow it */

		new_size = offset + map_size;
		if ((rc = ldr_grow_file(fd, new_size)) != LDR_SUCCESS)
			return;
	}

	if ((rc = ldr_mmap(addr, map_size, LDR_PROT_READ|LDR_PROT_WRITE,
			   mapflags, fd, offset, (univ_t *)&hp)) != LDR_SUCCESS)
		return(rc);

	/* Initialize the heap */

	hp->magic = HEAP_MAGIC;
	hp->next_addr = (univ_t)((char *)hp + map_size);
	hp->fd = fd;
	hp->flags = flags;
	hp->next_offset = new_size;
	for (bucket = 0; bucket < NBUCKETS; bucket++)
		hp->nextf[bucket] = NULL;

	*heap = (univ_t)hp;
	return(LDR_SUCCESS);
}


int
ldr_heap_init(void)

/* Initialize the standard process heap */
{
	return(ldr_heap_create(NULL, LDR_FILE_NONE, LDR_MAP_ANON|LDR_MAP_PRIVATE,
			       0, &ldr_process_heap));
}


int
ldr_heap_inherit(ldr_heap_t heap, int prot)

/* Try to inherit a heap, located at the specified address, from a parent
 * process via file or keep-on-exec mapping.  Validate the heap header, and
 * the heap size.  Currently assumes that the heap must be contiguous in
 * VA space (ie. mapped MAP_FIXED), and so all addresses from the
 * heap start to the next address must be valid.  Prot is the protection
 * with which the heap must be mapped.
 * Returns LDR_SUCCESS on success, negative error status on failure.
 */
{
	int_heap	*hp = heap;
	size_t			size;
	int			rc;

	if ((rc = ldr_mvalid((univ_t)hp, sizeof(int_heap), prot)) != LDR_SUCCESS)
		return(rc);

	if (hp->magic != HEAP_MAGIC)
		return(LDR_EVERSION);

	size = (char *)(hp->next_addr) - (char *)hp;
	if ((rc = ldr_mvalid((univ_t)hp, size, prot)) != LDR_SUCCESS)
		return(rc);

	return(LDR_SUCCESS);
}


size_t
ldr_heap_size(ldr_heap_t heap)

/* Return the total size in bytes occupied by the specified heap.
 * This routine can only be applied to a contiguous heap (ie a
 * heap mapped with MAP_FIXED), as it simply returns the difference
 * between the next heap address and the base of the heap.
 * Returns 0 on error (eg. heap not contiguous).
 */
{
	int_heap	*hp = heap;
	size_t			size;

	if (!(hp->flags & LDR_MAP_FIXED))
		return((size_t)0);
	return((char *)(hp->next_addr) - (char *)(heap));
}

/*
 * Dump out data structure for a heap.
 */
#ifdef MSTATS
void
ldr_heap_dump (int_heap *hp)
{
	union overhead *ptr;
	int bucket;
	int count;

	printf("Heap dump of %x\n", hp);
	printf("  magic: %x\n", hp->magic);
	printf("  next_addr: %x\n", hp->next_addr);
	printf("  fd: %x\n", hp->fd);
	printf("  flags: %x\n", hp->flags);
	printf("  next_offset: %x\n", hp->next_offset);
	for (bucket = 0; bucket < NBUCKETS; bucket++) 
		printf("%3d ", bucket);
	printf("\n");
	for (bucket = 0; bucket < NBUCKETS; bucket++) {
		count = 0;
		ptr = hp->nextf[bucket];
		while (ptr != NULL) {
			count++;
			ptr = ptr->ov_next;
			}
		printf("%3d ", count);
		}
	printf("\n\n");
}
#endif
