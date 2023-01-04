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
static char *rcsid = "@(#)$RCSfile: cma_malloc.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/09/29 14:31:43 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * DECthreads version of OSF/1 libpthreads.a's thread-reentrant malloc
 * package. No algorithm changes have been made, but references to OSF/1
 * libpthreads.a internal functions have been changed to DECthreads
 * equivalents (e.g., "spin_lock" to "cma__spinlock").
 */

/*
 * Memory allocator for use with multiple threads.
 */

#include <pthread.h>
#include <cma_defs.h>
#include <mach.h>

/*
 * Structure of memory block header.
 * When free, next points to next block on free list.
 * When allocated, fl points to free list.
 * Size of header is 4 bytes, so minimum usable block size is 8 bytes.
 */
typedef union header {
	union header *next;
	struct free_list *fl;
} *header_t;

#define MIN_SIZE	8	/* minimum block size */

typedef struct free_list {
	cma__t_atomic_bit lock;	/* spin lock for mutual exclusion */
	header_t head;		/* head of free list for this size */
#ifdef	DEBUG
	int in_use;		/* # mallocs - # frees */
#endif	DEBUG
} *free_list_t;

/*
 * Free list with index i contains blocks of size 2^(i+3) including header.
 * Smallest block size is 8, with 4 bytes available to user.
 * The largest block size allowed is 2^31, with 2^31 - 4 bytes available to user.
 */
#define NBUCKETS		29
#define MAX_USER_BLOCK_SIZE						    \
	((unsigned int) ((1 << (NBUCKETS + 2)) - sizeof(union header)))

static	struct free_list malloc_free_list[NBUCKETS];

static	cma__t_atomic_bit malloc_lock = cma__c_tac_static_clear;

typedef struct pool {
	struct pool *next;
	vm_size_t size;
} *pool_t;

#define	POOL_NULL   ((pool_t) 0)
static pool_t pool = POOL_NULL;
static vm_size_t pool_size = 0;

#define DEFAULT_MALLOC_POOL_SIZE    32
unsigned int malloc_pool_size = DEFAULT_MALLOC_POOL_SIZE;

/*
 * Function:
 *	more_memory
 *
 * Parameters:
 *	size - The size of the memory chunks in the bucket
 *	fl - free list pointer of the empty bucket
 *
 * Return value:
 *	FALSE	No more memory could be allocated
 *	TRUE	the bucket was filled
 *
 * Description:
 *	This function maintains pools of memory from which the malloc buckets
 *      are filled. New blocks added to the free list are taken
 *	from the first pool in the list of pools that has enough memory.
 *	If the amount of memory for the new blocks is bigger that what
 *      is available from any pool or is bigger than a pools initial
 *	size (vm_page_size * malloc_pool_size) then more memory is allocated
 *	using vm_allocate. The free list is assumed to be locked by the caller.
 */
static boolean_t
more_memory(unsigned int size, register free_list_t fl)
{
	register vm_size_t	amount;
	register unsigned int	n;
	vm_address_t		where;
	register header_t	h;
	kern_return_t		r;

	/*
	 * Calculate how many new blocks we are going to chain onto the
	 * free list. If the bucket contains blocks of less than 1 page
	 * then we allocate a page anyway and divide that up into a number
	 * of blocks to put on the free list. If we are allocating blocks of
	 * more than 1 page then we just allocate one.
	 */
	if (size <= vm_page_size) {
		amount = vm_page_size;
		n = vm_page_size / size;
		/*
		 * We lose vm_page_size - n*size bytes here.
		 */
	} else {
		amount = (vm_size_t) size;
		n = 1;
	}

	cma__spinlock(&malloc_lock);
	/*
	 * if the current pool (ie the one on the head of the pool list)
	 * has enough memory in it we will take it from there
	 */
	if (amount <= pool_size) {
		pool_size -= amount;
		where = (vm_address_t) pool + pool_size;
		/*
		 * having removed the memory, check to see if it is depleted.
		 * If so we discard this pool and go to the next one in the
		 * chain and make it the current pool (if there is one).
		 */
		if (pool_size == 0) {
			pool = pool->next;
			if (pool != POOL_NULL) {
				pool_size = pool->size;
			}
		}
	} else {
		/*
		 * There is either no current pool or it does not have
		 * enough memory in it for this allocation. Search through
		 * the other pools in the list (if there are any) to see if
		 * they have enough memory for this request.
		 */
		where = (vm_address_t) 0;
		if (pool != POOL_NULL) {
			register pool_t prev;
			register pool_t cur;

			prev = pool;
			while ((cur = prev->next) != POOL_NULL) {
				if (amount <= cur->size) {
					/*
					 * A pool with enough memory has been
					 * found. Take the memory and discard
					 * the pool if it is now empty.
					 */
					cur->size -= amount;
					where = (vm_address_t) cur + cur->size;
					if (cur->size == 0) {
						prev->next = cur->next;
					}
					break;
				}
				prev = cur;
			}
		}
		/*
		 * we may have been able to allocate some memory from one of
		 * the pools in the pools list. If not we have to allocate
		 * more memory with vm_allocate.
		 */
		if (where == (vm_address_t) 0) {
			/*
			 * If the amount of memory being requested is bigger
			 * than a pools size then we just return the memory
			 * allocated. Otherwise we make a new pool with
			 * the amount remaining after we have removed the
			 * memory we need from the pools initial size
			 */
			if (amount >= (vm_page_size * malloc_pool_size)) {
				r = vm_allocate(task_self(), &where, amount, TRUE);
				if (r != KERN_SUCCESS) {
					mach_error("malloc.more_memory: vm_allocate failed", r);
					cma__spinunlock(&malloc_lock);
					return(FALSE);
				}
			} else {
				pool_t new_pool = POOL_NULL;

				r = vm_allocate(task_self(), (vm_address_t *) &new_pool, (vm_page_size * malloc_pool_size), TRUE);
				if (r != KERN_SUCCESS) {
					mach_error("malloc.more_memory: vm_allocate failed", r);
					cma__spinunlock(&malloc_lock);
					return(FALSE);
				}
				/*
				 * save the current pool size in the pool
				 * structure and make the current pool and
				 * pool_size refer to the newly allocated
				 * memory.
				 */
				if (pool != POOL_NULL) {
					pool->size = pool_size;
				}
				new_pool->next = pool;
				pool = new_pool;
				pool_size = vm_page_size * malloc_pool_size - amount;
				where = (vm_address_t) pool + pool_size;
			}
		}
	}
	cma__spinunlock(&malloc_lock);

	/*
	 * we have the memory now. The free list should be locked by the caller
	 * so we can chain the new blocks onto it.
	 */
	h = (header_t) where;
	do {
		h->next = fl->head;
		fl->head = h;
		h = (header_t) ((char *) h + size);
	} while (--n != 0);

	return(TRUE);
}

/*
 * Function:
 *	malloc
 *
 * Parameters:
 *	size - the number of bytes requested
 *
 * Return value:
 *	0	the allocation failed
 *	otherwise a pointer to the newly allocated memory of the requested size
 *
 * Description:
 *	Memory is allocated in sizes which are powers of 2. These are taken
 *	from buckets which have memory of the same size chained onto them.
 *	If the bucket is empty, the function more memory is called to put at
 *	least on more element on the chain.
 */
any_t
malloc(register size_t size)
{
	register unsigned int	i, n;
	register free_list_t	fl;
	register header_t	h;

	/*
	 * An overhead of sizeof(union header) is need for our housekeeping
	 * Check the resulting size is sensible and will be found in one
	 * of the buckets.
	 */
	size += sizeof(union header);
	if (size <= sizeof(union header)) {
		/*
		 * A size of <= 0 or so big that the size turned
		 * negative was requested
		 */
#ifdef	DEBUG
		fprintf(stderr, "malloc: bad requested block size (%u)\n",
			size - sizeof(union header));
#endif	DEBUG
		return(0);
	}

	/*
	 * Find smallest power-of-two block size
	 * big enough to hold requested size plus header.
	 */
	i = 0;
	n = MIN_SIZE;
	while (n < size && i < NBUCKETS) {
		i += 1;
		n <<= 1;
	}
	if (i == NBUCKETS) {
#ifdef	DEBUG
		fprintf(stderr, "malloc: requested block too large (%u)\n",
			size - sizeof(union header));
#endif	DEBUG
		return(0);
	}
	
	/*
	 * we now have the index of the bucket that should contain memory
	 * big enough to staisfy this request. Lock the free list and check
	 * that it is not empty. If it is more blocks must be allocated for it.
	 */
	fl = &malloc_free_list[i];
	cma__spinlock(&fl->lock);
	h = fl->head;
	if (h == 0) {
		/*
		 * Free list is empty; allocate more blocks.
		 */
		if (! more_memory(n, fl)) {
			cma__spinunlock(&fl->lock);
			return(0);
		}
		h = fl->head;
	}
	/*
	 * Pop block from free list.
	 */
	fl->head = h->next;
#ifdef	DEBUG
	fl->in_use += 1;
#endif	DEBUG
	cma__spinunlock(&fl->lock);
	/*
	 * Store free list pointer in block header
	 * so we can figure out where it goes
	 * at free() time.
	 */
	h->fl = fl;
	/*
	 * Return pointer past the block header.
	 */
	return(((char *) h) + sizeof(union header));
}

/*
 * Function:
 *	free
 *
 * Parameters:
 *	base - pointer to the memory no longer needed.
 *
 * Description:
 *	Free up memory that was previously allocated with malloc. This is
 *	done by chaining the memory back onto the free list of the bucket
 *	of the correct size. Actually we don't know how much memory the
 *	pointer points to but hidden just before the memory is a pointer
 *	to the free list that the memory should be freed onto. Rudimentary
 *	sanity checks are made to ensure the pointer is not complete garbage.
 */
void
free(any_t base)
{
	register header_t	h;
	register free_list_t	fl;
	register int		i;

	if (base == 0) {
#ifdef	DEBUG
		fprintf(stderr, "free: freeing zero length block\n");
#endif	DEBUG
		return;
	}

	/*
	 * Find free list for block.
	 */
	h = (header_t) ((vm_address_t) base - sizeof(union header));
	/*
	 * If base is garbage this may seg fault, but such is life ...
	 */
	fl = h->fl;
	i = fl - malloc_free_list;
	/*
	 * Check that the free list pointer we have found is legitimate by
	 * first checking the index into the buckets is valid and then by
	 * checking the pointer is the same as the address of the free list
	 * indicated by the index.
	 */
	if (i < 0 || i >= NBUCKETS) {
#ifdef	DEBUG
		fprintf(stderr, 
			"free: object has bad free list pointer (0x%x)\n",
			fl);
#endif	DEBUG
		return;
	}
	if (fl != &malloc_free_list[i]) {
#ifdef	DEBUG
		fprintf(stderr, 
	"free: object's free list ptr != bucket free list ptr (0x%x != 0x%x)\n",
			fl, &malloc_free_list[i]);
#endif	DEBUG
		return;
	}
	/*
	 * Push block on free list.
	 */
	cma__spinlock(&fl->lock);
	h->next = fl->head;
	fl->head = h;
#ifdef	DEBUG
	fl->in_use -= 1;
#endif	DEBUG
	cma__spinunlock(&fl->lock);
	return;
}

/*
 * Function:
 *	realloc
 *
 * Parameters:
 *	old_base - the pointer to the memory to be copied and freed.
 *	new_size - size of the enlarged memory area
 *
 * Return value:
 *	0	eitheer the allocation or the free failed
 *	otherwise a pointer to the newly allocated memory of the requested size
 *
 * Description:
 *	This function performs the equivalent of
 *		new = malloc(new_size);
 *		bcopy(old,new);
 *		free(old);
 *		return(new);
 */
any_t
realloc(any_t old_base, size_t new_size)
{
	register header_t	h;
	register free_list_t	fl;
	register int		i;
	unsigned int		old_size;
	char			*new_base;

	/*
	 * If there is nothing to copy, just malloc the space and return it.
	 */
	if (old_base == 0)
		return(malloc(new_size));
	/*
	 * Find free list for the old block.
	 */
	h = (header_t) ((vm_address_t) old_base - sizeof(union header));
	/*
	 * If old_base is garbage this may seg fault, but such is life ...
	 */
	fl = h->fl;
	i = fl - malloc_free_list;
	/*
	 * Check that the free list pointer we have found is legitimate by
	 * first checking the index into the buckets is valid and then by
	 * checking the pointer is the same as the address of the free list
	 * indicated by the index.
	 */
	if (i < 0 || i >= NBUCKETS) {
#ifdef	DEBUG
		fprintf(stderr, 
			"realloc: object has bad free list pointer (0x%x)\n",
			fl);
#endif	DEBUG
		return(0);
	}
	if (fl != &malloc_free_list[i]) {
#ifdef	DEBUG
		fprintf(stderr, 
	"realloc: object's free list ptr != bucket free list ptr (0x%x != 0x%x)\n",
			fl, &malloc_free_list[i]);
#endif	DEBUG
		return(0);
	}
	/*
	 * Free list with index i contains blocks of size 2^(i+3)
	 * including header.
	 */
	old_size = (1 << (i+3)) - sizeof(union header);

	/*
	 * If new_size is in the same bucket, then don't do anything.
	 */
	if ((old_size >= new_size) &&
	    ((new_size + sizeof(union header)) > (1 << (i+2)))) {
		return(old_base);
	 }

	/*
	 * Allocate new block, copy old bytes, and free old block.
	 */
	new_base = malloc(new_size);
	if (new_base != 0)
		bcopy(old_base, new_base, (int) (old_size < new_size ? old_size : new_size));
	free(old_base);
	return(new_base);
}

#ifdef	DEBUG
void
print_malloc_free_list()
{
  	register unsigned int	i, size;
	register free_list_t	fl;
	register unsigned int	n;
  	register header_t	h;
	int			total_used = 0;
	int			total_free = 0;

	fprintf(stderr, "      Size     In Use       Free      Total\n");
  	for (i = 0, size = MIN_SIZE, fl = malloc_free_list;
	     i < NBUCKETS;
	     i += 1, size <<= 1, fl += 1) {
		cma__spinlock(&fl->lock);
		if (fl->in_use != 0 || fl->head != 0) {
			total_used += fl->in_use * size;
			for (n = 0, h = fl->head; h != 0; h = h->next, n += 1)
				;
			total_free += n * size;
			fprintf(stderr, "%10d %10d %10d %10d\n",
				size, fl->in_use, n, fl->in_use + n);
		}
		cma__spinunlock(&fl->lock);
  	}
  	fprintf(stderr, " all sizes %10d %10d %10d\n",
		total_used, total_free, total_used + total_free);
}
#endif	DEBUG

/*
 * Function:
 *	malloc_fork_prepare
 *
 * Description:
 *	Prepare the malloc module for a fork by insuring that no thread is in a
 *	malloc critical section.
 */
void
cma__reinit_malloc
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
{
	register int	i;

	if (flag == cma__c_reinit_prefork_lock) {
	    for (i = 0; i < NBUCKETS; i++)
		cma__spinlock(&malloc_free_list[i].lock);
	    }
	else if (flag == cma__c_reinit_postfork_unlock) {
	    for (i = NBUCKETS-1; i >= 0; i--)
		cma__spinunlock(&malloc_free_list[i].lock);
	    }

}
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_MALLOC.C */
/*  *1A2  21-JAN-1992 11:53:13 BUTENHOF "Add a cast" */
/*   2    21-JAN-1992 11:50:57 BUTENHOF "Add a cast" */
/*  *1A1  20-JAN-1992 18:16:50 SCALES "Integrate for Tin" */
/*  *1     7-JAN-1992 17:29:16 BUTENHOF "DEC OSF/1 malloc()" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_MALLOC.C */
