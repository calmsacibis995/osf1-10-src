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
static char *rcsid = "@(#)$RCSfile: heap_kmem.c,v $ $Revision: 1.1.15.3 $ (DEC) $Date: 1993/11/02 21:01:17 $";
#endif
/*	@(#)heap_kmem.c	2.4 89/03/17 4.0NFSSRC SMI;	from SMI 2.27 86/12/29	*/

/* define DEBUG ON */


/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

/*
 * Conditions on use:
 * kmem_alloc and kmem_free must not be called from interrupt level
 *
 * kmem_alloc will only return failure if called from interrupt level
 * and no storage is immediately available.  If called from non-interrupt
 * level it will sleep waiting for any required resources.
 *
 * Also, these routines are not that fast, so they should not be used
 * in very frequent operations (e.g. operations that happen more often
 * than, say, once every few seconds).
 *
 * Consider in the future of having different kernel heap spaces managed.
 *
 */

/*
 * description:
 *	Yet another memory allocator, this one based on a method
 *	described in C.J. Stephenson, "Fast Fits", IBM Sys. Journal
 *
 *	The basic data structure is a "Cartesian" binary tree, in which
 *	nodes are ordered by ascending addresses (thus minimizing free
 *	list insertion time) and block sizes decrease with depth in the
 *	tree (thus minimizing search time for a block of a given size).
 *
 *	In other words, for any node s, letting D(s) denote
 *	the set of descendents of s, we have:
 *
 *	a. addr(D(left(s))) <  addr(s) <  addr(D(right(s)))
 *	b. len(D(left(s)))  <= len(s)  >= len(D(right(s)))
 */

#include <mach/vm_param.h>
#include <mach/machine/vm_param.h>
#include <vm/vm_kern.h>
#include <sys/unix_defs.h>
#include <kern/zalloc.h>
#include <vm/vm_tune.h>
#include <mach/kern_return.h>
#include <vm/vm_debug.h>
#include <vm/heap_kmem.h>
#include <sys/kernel.h>
#ifdef MEMLOG
#include <sys/memlog.h>
#endif

#ifdef MEMLOG
#define  VM_HEAP_STATS 1
#endif

/*
 * The node header structure.
 * 
 * To reduce storage consumption, a header block is associated with
 * free blocks only, not allocated blocks.
 * When a free block is allocated, its header block is put on 
 * a free header block list.
 *
 * This creates a header space and a free block space.
 * The left pointer of a header blocks is used to chain free header
 * blocks together.
 */

typedef enum {false,true} bool;
typedef struct	freehdr	*Freehdr;
typedef struct	dblk	*Dblk;

/*
 * Description of a header for a free block
 * Only free blocks have such headers.
 */

struct 	freehdr	{
	Freehdr	left;			/* Left tree pointer */
	Freehdr	right;			/* Right tree pointer */
	Dblk	block;			/* Ptr to the data block */
	u_int	size;			/* Size of the data block */
};

#define NIL		((Freehdr) 0)
#define WORDSIZE	sizeof (long)
#define	SMALLEST_BLK	1	 	/* Size of smallest block */

/*
 * Description of a data block.  
 */
struct	dblk	{
	char	data[1];		/* Addr returned to the caller */
};

/*
 * weight(x) is the size of a block, in bytes; or 0 if and only if x
 *	is a null pointer. It is the responsibility of h_kmem_alloc_memory() and
 *	h_kmem_free_memory() to keep zero-length blocks out of the arena.
 */

#define	weight(x)	((x) == NIL? 0: (x->size))
#define	nextblk(p, size) ((Dblk) ((char *) (p) + (size)))
#define	max(a, b)	((a) < (b)? (b): (a))

Freehdr	h_getfreehdr();
bool	h_morecore();
caddr_t	h_getpages();
caddr_t	h_kmem_alloc_memory_();
caddr_t h_kmem_zalloc_memory_();


/*
 * Structure containing various info about allocated memory.
 */

#define	NEED_TO_FREE_SIZE	10
struct h_kmem_info {
	Freehdr	free_root;
	Freehdr	free_hdr_list;
	vm_map_t submap;
	vm_size_t submap_size;
	vm_offset_t submap_max, submap_min;
	vm_object_t submap_object;
	int more_core_failed;
	char want;
	struct h_kmem_info *next, *last;
#if	VM_HEAP_STATS
	vm_size_t pages_allocated;
	vm_size_t bytes_free;
#endif	/* VM_HEAP_STATS */
	struct need_to_free {
		caddr_t addr;
		u_int	nbytes;
	} need_to_free_list, need_to_free[NEED_TO_FREE_SIZE];
} h_kmem_info;

/*
 * The default heap allocator size at boot time
 */

struct h_kmem_info *h_kmem = &h_kmem_info;
vm_map_t heap_submap;
udecl_simple_lock_data(, h_kmem_list_lock)
extern int vm_managed_pages;

#define	HEAP_DEFAULT_PERCENT 	5			/* 5 percent */
#define HEAP_MAXKVA_PERCENT	20			/* 20 percent */
int heap_maxkva_percent = 0;

/*
 * Initialize kernel memory allocator
 */

h_kmem_init()
{
	int reduced = 0;
	register int i;
	register struct need_to_free *ntf;
	register int percent;
	vm_offset_t min, max;
	register vm_size_t size, default_size, maxkva;

	usimple_lock_init(&h_kmem_list_lock);
	
	h_kmem_info.free_root = NIL;
	h_kmem_info.free_hdr_list = NULL;
	h_kmem_info.need_to_free_list.addr = 0;

	/*
	 * Size the heap
	 */

	if (!heap_maxkva_percent) heap_maxkva_percent = HEAP_MAXKVA_PERCENT;

	maxkva = ((VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS) *
		heap_maxkva_percent) / 100;

	default_size = ptoa((HEAP_DEFAULT_PERCENT * vm_managed_pages) / 100);
	if (default_size > maxkva) default_size = maxkva;

	percent = vm_tune_value(heappercent);
	if (percent && (size = (vm_managed_pages * percent) / 100)) {
		size = ptoa(size);
		if (size > maxkva) size = maxkva;
	}
	else size = default_size;


	while ((heap_submap = 
		kmem_suballoc(kernel_map, &min, &max, size, FALSE)) 
						== VM_MAP_NULL) 
		if (size <= default_size) 
			panic("h_kmem_init: unable to initialize");
		else {
			size = default_size;
			continue;
			reduced = 1;
		}
	if (reduced) printf("h_kmem_init: heap size reduced");

	h_kmem_info.submap = heap_submap;
	h_kmem_info.submap_size = size;
	h_kmem_info.submap_max = max;
	h_kmem_info.submap_min = min;
	if (k_map_object_fast(&h_kmem_info.submap_object, size) !=
		KERN_SUCCESS) panic("h_kmem_init: no wired object");

	ntf = h_kmem_info.need_to_free;
	for (i = 0; i < NEED_TO_FREE_SIZE; i++) {
		ntf[i].addr = 0;
	}
}

struct h_kmem_info *
h_kmem_alloc_init(register vm_size_t space, 
	boolean_t canwait)
{
	vm_offset_t min, max;
	register vm_map_t submap;
	register struct h_kmem_info *hkmp;
	vm_object_t object;
	register struct need_to_free *ntf;
	register int i;

	hkmp = (struct h_kmem_info *) h_kmem_zalloc_memory_(h_kmem, 
			sizeof (struct h_kmem_info), canwait);
	if (!hkmp) return hkmp;

	space = round_page(space);
	if (k_map_object_fast(&object, space) != KERN_SUCCESS) {
		h_kmem_free_memory_(h_kmem, hkmp, sizeof (struct h_kmem_info),
			canwait);
		return (struct h_kmem_info *) 0;
	}

	if ((submap = kmem_suballoc(kernel_map, &min, &max, space, FALSE))
		== VM_MAP_NULL) { 
		h_kmem_free_memory_(h_kmem, hkmp, sizeof (struct h_kmem_info),
			canwait);
		vm_object_free(object);
		return (struct h_kmem_info *) 0;
	}

	hkmp->submap = submap;
	hkmp->submap_size = space;
	hkmp->submap_max = max;
	hkmp->submap_min = min;
	hkmp->submap_object = object;

	ntf = hkmp->need_to_free;
	for (i = 0; i < NEED_TO_FREE_SIZE; i++) {
		ntf[i].addr = 0;
	}

	usimple_lock(&h_kmem_list_lock);
	if (h_kmem->next) {
		h_kmem->last->next = hkmp;
		h_kmem->last = hkmp;
	}
	else h_kmem->next = h_kmem->last = hkmp;
	usimple_unlock(&h_kmem_list_lock);
	return hkmp;
}

int
h_get_kmem_info(struct h_kmem_info *hp,
	vm_offset_t *first,
	vm_size_t *size)
{
	*first = hp->submap_min;
	*size = (hp->submap_max - hp->submap_min);
}


/*
 * Insert a new node in a cartesian tree or subtree, placing it
 *	in the correct position with respect to the existing nodes.
 *
 * algorithm:
 *	Starting from the root, a binary search is made for the new
 *	node. If this search were allowed to continue, it would
 *	eventually fail (since there cannot already be a node at the
 *	given address); but in fact it stops when it reaches a node in
 *	the tree which has a length less than that of the new node (or
 *	when it reaches a null tree pointer).  The new node is then
 *	inserted at the root of the subtree for which the shorter node
 *	forms the old root (or in place of the null pointer).
 */


h_insert(p, len, tree, newhdr)
register Dblk p;		/* Ptr to the block to insert */
register u_int len;		/* Length of new node */
register Freehdr *tree;		/* Address of ptr to root */
Freehdr newhdr;
{
	register Freehdr x;
	register Freehdr *left_hook;	/* Temp for insertion */
	register Freehdr *right_hook;	/* Temp for insertion */

	x = *tree;
	/*
	 * Search for the first node which has a weight less
	 *	than that of the new node; this will be the
	 *	point at which we insert the new node.
	 */

	while (weight(x) >= len) {	
		if (p < x->block)
			tree = &x->left;
		else
			tree = &x->right;
		x = *tree;
	}

	/*
	 * Perform root insertion. The variable x traces a path through
	 *	the tree, and with the help of left_hook and right_hook,
	 *	rewrites all links that cross the territory occupied
	 *	by p.  Note that this improves performance under
	 *	paging.
	 */ 

	*tree = newhdr;
	left_hook = &newhdr->left;
	right_hook = &newhdr->right;

	newhdr->left = NIL;
	newhdr->right = NIL;
	newhdr->block = p;
	newhdr->size = len;

	while (x != NIL) {
		/*
		 * Remark:
		 *	The name 'left_hook' is somewhat confusing, since
		 *	it is always set to the address of a .right link
		 *	field.  However, its value is always an address
		 *	below (i.e., to the left of) p. Similarly
		 *	for right_hook. The values of left_hook and
		 *	right_hook converge toward the value of p,
		 *	as in a classical binary search.
		 */
		if (x->block < p) {
			/*
			 * rewrite link crossing from the left
			 */
			*left_hook = x;
			left_hook = &x->right;
			x = x->right;
		} else {
			/*
			 * rewrite link crossing from the right
			 */
			*right_hook = x;
			right_hook = &x->left;
			x = x->left;
		} /*else*/
	} /*while*/

	*left_hook = *right_hook = NIL;		/* clear remaining hooks */

} /*insert*/


/*
 * Delete a node from a cartesian tree. p is the address of
 *	a pointer to the node which is to be deleted.
 *
 * algorithm:
 *	The left and right sons of the node to be deleted define two
 *	subtrees which are to be merged and attached in place of the
 *	deleted node.  Each node on the inside edges of these two
 *	subtrees is examined and longer nodes are placed above the
 *	shorter ones.
 *
 * On entry:
 *	*p is assumed to be non-null.
 */

h_delete(struct h_kmem_info *hp,
	register Freehdr *p)
{
	register Freehdr x;
	register Freehdr left_branch;	/* left subtree of deleted node */
	register Freehdr right_branch;	/* right subtree of deleted node */

	x = *p;
	left_branch = x->left;
	right_branch = x->right;

	while (left_branch != right_branch) {	
		/*
		 * iterate until left branch and right branch are
		 * both NIL.
		 */
		if (weight(left_branch) >= weight(right_branch)) {
			/*
			 * promote the left branch
			 */
			*p = left_branch;
			p = &left_branch->right;
			left_branch = left_branch->right;
		} else {
			/*
			 * promote the right branch
			 */
			*p = right_branch;
			p = &right_branch->left;
			right_branch = right_branch->left;
		}/*else*/
	}/*while*/
	*p = NIL;
	h_freehdr(hp, x);
} /*delete*/


/*
 * Demote a node in a cartesian tree, if necessary, to establish
 *	the required vertical ordering.
 *
 * algorithm:
 *	The left and right subtrees of the node to be demoted are to
 *	be partially merged and attached in place of the demoted node.
 *	The nodes on the inside edges of these two subtrees are
 *	examined and the longer nodes are placed above the shorter
 *	ones, until a node is reached which has a length no greater
 *	than that of the node being demoted (or until a null pointer
 *	is reached).  The node is then attached at this point, and
 *	the remaining subtrees (if any) become its descendants.
 *
 * on entry:
 *   a. All the nodes in the tree, including the one to be demoted,
 *	must be correctly ordered horizontally;
 *   b. All the nodes except the one to be demoted must also be
 *	correctly positioned vertically.  The node to be demoted
 *	may be already correctly positioned vertically, or it may
 *	have a length which is less than that of one or both of
 *	its progeny.
 *   c. *p is non-null
 */


demote(p)
register Freehdr *p;
{
	register Freehdr x;		/* addr of node to be demoted */
	register Freehdr left_branch;
	register Freehdr right_branch;
	register u_int    wx;

	x = *p;
	left_branch = x->left;
	right_branch = x->right;
	wx = weight(x);

	while (weight(left_branch) > wx || weight(right_branch) > wx) {
		/*
		 * select a descendant branch for promotion
		 */
		if (weight(left_branch) >= weight(right_branch)) {
			/*
			 * promote the left branch
			 */
			*p = left_branch;
			p = &left_branch->right;
			left_branch = *p;
		} else {
			/*
			 * promote the right branch
			 */
			*p = right_branch;
			p = &right_branch->left;
			right_branch = *p;
		} /*else*/
	} /*while*/

	*p = x;				/* attach demoted node here */
	x->left = left_branch;
	x->right = right_branch;
} /*demote*/

/*
 * Allocate a block of storage
 *
 * algorithm:
 *	The freelist is searched by descending the tree from the root
 *	so that at each decision point the "better fitting" child node
 *	is chosen (i.e., the shorter one, if it is long enough, or
 *	the longer one, otherwise).  The descent stops when both
 *	child nodes are too short.
 *
 * function result:
 *	kmem_alloc returns a pointer to the allocated block; a null
 *	pointer indicates storage could not be allocated.
 */
/*
 * We need to return blocks that are on word boundaries so that callers
 * that are putting int's into the area will work.  Since we allow
 * arbitrary free'ing, we need a weight function that considers
 * free blocks starting on an odd boundary special.  Allocation is
 * aligned to 4 byte boundaries (ALIGN).
 */
#define	ALIGN		sizeof(long)
#define	ALIGNMASK	(ALIGN-1)
#define	ALIGNMORE(addr)	(ALIGN - ((vm_offset_t)(addr) & ALIGNMASK))

#define	mweight(x) ((x) == NIL ? 0 : \
    ((((int)(x)->block) & ALIGNMASK) && ((x)->size > ALIGNMORE((x)->block)))\
    ? (x)->size - ALIGNMORE((x)->block) : (x)->size)

caddr_t
h_kmem_alloc_memory_(struct h_kmem_info *hp,
	register u_int nbytes, 
	boolean_t wait)
{
	register Freehdr a;		/* ptr to node to be allocated */
	register Freehdr *p;		/* address of ptr to node */
	register u_int	 left_weight;
	register u_int	 right_weight;
	register Freehdr left_son;
	register Freehdr right_son;
	register char	 *retblock;	/* Address returned to the user */
	int s;
	long failed = 0;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(H_KMEM_ALLOC_LOG, caller, nbytes);
	}
#endif

	if (nbytes == 0) {
		return(NULL);
	}
	s = splnet();

	if (nbytes < SMALLEST_BLK) {
		printf("illegal h_kmem_alloc call for %d bytes\n", nbytes);
		panic("h_kmem_alloc_memory_");
	}

	if (wait) h_check_need_to_free(hp);

	/*
	 * ensure that at least one block is big enough to satisfy
	 *	the request.
	 */

	while (mweight(hp->free_root) < nbytes) {

		/*
		 * the largest block is not enough. 
		 */

		if (!h_morecore(hp, nbytes, wait)) {
			if (hp->more_core_failed && 
				((hp->more_core_failed % 100) == 0))
				printf("h_kmem_alloc_memory_: allocation failures = %d\n",
					hp->more_core_failed);
			if (wait == TRUE) {
				hp->want = TRUE;
				assert_wait((vm_offset_t) &hp->want, FALSE);
				(void) splx(s);
				if (failed) {
					if ((time.tv_sec - failed) > 5) {
						printf("h_kmem_alloc_memory_: request is stalled\n");
						failed = time.tv_sec;
					}
				}
				else {
					failed = time.tv_sec;
					hp->more_core_failed++;
					if (nbytes > 
					(hp->submap_max - hp->submap_min))
						printf("h_kmem_alloc_memory_: request will never succeed\n");
				}
				thread_block();
				splnet();
				continue;
			}
			else {
				hp->more_core_failed++;
				(void) splx(s);
				return (caddr_t) 0;
			}
		}
		break;
	}

	/*
	 * search down through the tree until a suitable block is
	 *	found.  At each decision point, select the better
	 *	fitting node.
	 */

	p = (Freehdr *) &hp->free_root;
	a = *p;
	left_son = a->left;
	right_son = a->right;
	left_weight = mweight(left_son);
	right_weight = mweight(right_son);

	while (left_weight >= nbytes || right_weight >= nbytes) {
		if (left_weight <= right_weight) {
			if (left_weight >= nbytes) {
				p = &a->left;
				a = left_son;
			} else {
				p = &a->right;
				a = right_son;
			}
		} else {
			if (right_weight >= nbytes) {
				p = &a->right;
				a = right_son;
			} else {
				p = &a->left;
				a = left_son;
			}
		}
		left_son = a->left;
		right_son = a->right;
		left_weight = mweight(left_son);
		right_weight = mweight(right_son);
	} /*while*/	

	/*
	 * allocate storage from the selected node.
	 */
	
	if (a->size - nbytes < SMALLEST_BLK) {
		/*
		 * not big enough to split; must leave at least
		 * a dblk's worth of space.
		 */
		retblock = a->block->data;
		h_delete(hp, p);
	} else {

		/*
		 * split the node, allocating nbytes from the top.
		 *	Remember we've already accounted for the
		 *	allocated node's header space.
		 */
		if ((int) a->block->data & ALIGNMASK &&
		    a->size > ALIGNMORE(a->block->data)) {
			int alm;		/* ALIGNMORE factor */
			u_int alsize;		/* aligned size */

			alm = ALIGNMORE(a->block->data);
			retblock = a->block->data + alm;

			/*
			 * Re-use this header
			 */
			alsize = a->size - alm;
			a->size = alm;
			/*
			 * the node pointed to by *p has become smaller;
			 *	move it down to its appropriate place in
			 *	the tree.
			 */
			demote(p);
			/*
			rmlog(0, a->block, 1, caller());
			rmlog(1, a->block, 1, caller());
			*/

			if (alsize > nbytes) {
				/*
				 * place trailing bytes back into the heap.
				 */
				h_kmem_free_memory_(hp,
					(caddr_t)(retblock + nbytes),
				    (u_int)alsize - nbytes, wait);
			}
		} else {
			retblock = a->block->data;
			a->block = nextblk(a->block, nbytes);
			a->size -= nbytes;
			/*
			 * the node pointed to by *p has become smaller;
			 *	move it down to its appropriate place in
			 *	the tree.
			 */
			demote(p);
		}
	}
	/*
	rmlog(0, retblock, nbytes, caller());
	*/
	(void) splx(s);

#if	VM_HEAP_STATS
	hp->bytes_free -= nbytes;
#endif	/* VM_HEAP_STATS */
	return (retblock);

} 

/*
 * Return nbytes worth of zeroed out memory.
 */

caddr_t
h_kmem_zalloc_memory_(struct h_kmem_info *hp,
	u_int nbytes,
	boolean_t wait)
{
	char *res;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(H_KMEM_ZALLOC_LOG, caller, nbytes);
	}
#endif
	res = h_kmem_alloc_memory_(hp, nbytes, wait);
	if (res != NULL) bzero(res, nbytes);
	return (res);
}

/*
 * Return a block to the free space tree.
 * 
 * algorithm:
 *	Starting at the root, search for and coalesce free blocks
 *	adjacent to one given.  When the appropriate place in the
 *	tree is found, insert the given block.
 *
 * Do some sanity checks to avoid total confusion in the tree.
 * If the block has already been freed, panic.
 * If the ptr is not from the arena, panic.
 */
#if	VM_HEAP_TRACE_FREE
extern vm_offset_t get_caller_ra();
struct h_trace_list {
	vm_offset_t	tl_pc;
	caddr_t		tl_ptr;
	u_int		tl_size;
} h_trace_list[200];
static h_trace_index = 0;

h_trace_free(vm_offset_t pc, caddr_t ptr, u_int nbytes)
{
	h_trace_list[h_trace_index].tl_pc = pc;
	h_trace_list[h_trace_index].tl_ptr = ptr;
	h_trace_list[h_trace_index++].tl_size = nbytes;
	if (h_trace_index == 200) h_trace_index = 0;
}
#endif /* VM_HEAP_TRACE_FREE */

h_kmem_free_memory_(struct h_kmem_info *hp,
	caddr_t ptr,
	register u_int nbytes,
	boolean_t wait)
{
	register Freehdr *np;		/* For deletion from free list */
	register Freehdr neighbor;	/* Node to be coalesced */
	register char	 *neigh_block;	/* Ptr to potential neighbor */
	register u_int	 neigh_size;	/* Size of potential neighbor */
	Freehdr newhdr;
	int s;
#if	VM_HEAP_TRACE_FREE
	h_trace_free(get_caller_ra(), ptr, nbytes);
#endif	/* VM_HEAP_TRACE_FREE */	

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(H_KMEM_FREE_LOG, caller, nbytes);
	}
#endif

	if (nbytes == 0) {
		return;
	}

	/*
	 * check bounds of pointer.
	 */
	if ((vm_offset_t) ptr < hp->submap_min ||
	    (vm_offset_t) ptr >= hp->submap_max) {
		printf("h_kmem_free_memory_: illegal pointer %x\n",ptr);
		panic("h_kmem_free_memory_");
		return;
	}

	s = splnet();

	newhdr = h_getfreehdr(hp, wait);

	/*
	 * This can only happen for the no wait case
	 */

	if (newhdr == (Freehdr) 0) {
		(void) splx(s);
		h_kmem_free_intr(hp, ptr, nbytes);
		return;
	}

#if	VM_HEAP_STATS
	hp->bytes_free += nbytes;
#endif	/* VM_HEAP_STATS */

	/*
	 * Search the tree for the correct insertion point for this
	 *	node, coalescing adjacent free blocks along the way.
	 */
	np = &hp->free_root;
	neighbor = *np;
	while (neighbor != NIL) {
		neigh_block = (char *) neighbor->block;
		neigh_size = neighbor->size;
		if (ptr < neigh_block) {
			if (ptr + nbytes == neigh_block) {
				/*
				 * Absorb and delete right neighbor
				 */
				nbytes += neigh_size;
				h_delete(hp, np);
			} else if (ptr + nbytes > neigh_block) {
				/*
				 * The block being freed overlaps
				 * another block in the tree.  This
				 * is bad news.
				 */
				 printf("h_kmem_free_memory_: free block overlap %x+%d over %x\n", ptr, nbytes, neigh_block);
				 panic("h_kmem_free_memory_: free block overlap");
			} else {
				/*
				 * Search to the left
			 	*/
				np = &neighbor->left;
			}
		} else if (ptr > neigh_block) {
			if (neigh_block + neigh_size == ptr) {
				/*
				 * Absorb and delete left neighbor
				 */
				ptr = neigh_block;
				nbytes += neigh_size;
				h_delete(hp, np);
			} else if (neigh_block + neigh_size > ptr) {
				/*
				 * This block has already been freed
				 */
				panic("h_kmem_free_memory_ block already free");
			} else {
				/*
				 * search to the right
				 */
				np = &neighbor->right;
			}
		} else {
			/*
			 * This block has already been freed
			 * as "ptr == neigh_block"
			 */
			panic("h_kmem_free_memory_: block already free as neighbor");
			return;
		} /*else*/
		neighbor = *np;
	} /*while*/

	/*
	 * Insert the new node into the free space tree
	 */
	h_insert((Dblk) ptr, nbytes, &hp->free_root, newhdr);
	if (hp->want) {
		hp->want = FALSE;
		thread_wakeup((vm_offset_t) &hp->want);
	}
	(void) splx(s);
} 

/*
 * We maintain a list of blocks that need to be freed.
 * This is because we don't want to spl the relatively long
 * routines malloc and free, but we need to be able to free
 * space at interrupt level.
 */

h_kmem_free_intr(struct h_kmem_info *hp,
	caddr_t ptr,
	register u_int nbytes)
{
	register u_int i;
	register struct need_to_free *ntf;
	int s;

	s = splimp();
	if (nbytes >= sizeof (struct need_to_free)) {
		if ((vm_offset_t)ptr & ALIGNMASK) {
			i = ALIGNMORE(ptr);
			h_kmem_free_intr(hp, ptr, i);
			h_kmem_free_intr(hp, ptr + i, nbytes - i);
			(void) splx(s);
			return;
		}
		ntf = &hp->need_to_free_list;
		*(struct need_to_free *)ptr = *ntf;
		ntf->addr = ptr;
		ntf->nbytes = nbytes;
		(void) splx(s);
		return;
	}
	ntf = hp->need_to_free;
	for (i = 0; i < NEED_TO_FREE_SIZE; i++) {
		if (ntf[i].addr == 0) {
			ntf[i].addr = ptr;
			ntf[i].nbytes = nbytes;
			(void) splx(s);
			return;
		}
	}
	panic("h_kmem_free_intr:");
}

static
h_check_need_to_free(struct h_kmem_info *hp)
{
	register int i;
	register struct need_to_free *ntf;
	caddr_t addr;
	u_int nbytes;
	int s;

again:
	s = splimp();
	ntf = &hp->need_to_free_list;
	if (ntf->addr) {
		addr = ntf->addr;
		nbytes = ntf->nbytes;
		*ntf = *(struct need_to_free *) ntf->addr;
		(void) splx(s);
		h_kmem_free_memory_(hp, addr, nbytes, TRUE);
		goto again;
	}
	ntf = hp->need_to_free;
	for (i = 0; i < NEED_TO_FREE_SIZE; i++) {
		if (ntf[i].addr) {
			addr = ntf[i].addr;
			nbytes = ntf[i].nbytes;
			ntf[i].addr = 0;
			(void) splx(s);
			h_kmem_free_memory_(hp, addr, nbytes, TRUE);
			goto again;
		}
	}
	(void) splx(s);
}

/*
 * Add a block of at least nbytes to the free space tree.
 *
 * return value:
 *	true	if at least nbytes can be allocated
 *	false	otherwise
 *
 * remark:
 *	free space (delimited by the static variable ubound) is 
 *	extended by an amount determined by rounding nbytes up to
 *	a multiple of the system page size.
 */

bool
h_morecore(struct h_kmem_info *hp,
	u_int nbytes,
	boolean_t wait)
{
	Dblk p;

	nbytes = round_page(nbytes);
	p = (Dblk) h_getpages(hp, nbytes / PAGE_SIZE, wait);
	if (p == 0) return (false);
	h_kmem_free_memory_(hp, (caddr_t) p, nbytes, TRUE);
	return (true);

} 

/*
 * get npages pages from the system
 */

caddr_t
h_getpages(struct h_kmem_info *hp,
	u_int npages,
	boolean_t wait)
{
	caddr_t va;

	if (k_map_allocate_fast( hp->submap, hp->submap_object, 
		&va, ptoa(npages), wait) != KERN_SUCCESS) va = (caddr_t) 0;
#if	VM_HEAP_STATS
	hp->pages_allocated += npages;
#endif	/* VM_HEAP_STATS */
	return va;
}


/*
 * Get a free block header
 * There is a list of available free block headers.
 * When the list is empty, allocate another pagefull.
 * Note free header page allocation always comes from the
 * master (default) allocator.
 */

Freehdr
h_getfreehdr(struct h_kmem_info *hp,
	boolean_t wait)
{
	Freehdr	r;
	int	n;

	if (hp->free_hdr_list != NIL) {
		r = hp->free_hdr_list;
		hp->free_hdr_list = hp->free_hdr_list->left;
	} else {
		r = (Freehdr) h_getpages(h_kmem, 1, wait);
		if (r == 0) {
			if (wait) panic("h_getfreehdr");
			else return r;
		}
		for (n = 1; n < CLBYTES / sizeof (*r); n++) {
			h_freehdr(hp, &r[n]);
		}
	}
	return (r);
}

/*
 * Free a free block header
 * Add it to the list of available headers.
 */

h_freehdr(struct h_kmem_info *hp,
	Freehdr	p)
{
	p->left = hp->free_hdr_list;
	p->right = NIL;
	p->block = NULL;
	hp->free_hdr_list = p;
}

#ifdef DEBUG
/*
 * Diagnostic routines
 */
static depth = 0;

h_prtree(p, cp)
h_Freehdr p;
char *cp;
{
	int n;
	if (depth == 0) {
		printf("h_prtree. p %x cp %s\n", p, cp);
/*
	} else {
		printf("h_prtree. p %x depth %d\n", p, depth);
*/
	}
	if (p != NIL){
		depth++;
		h_prtree(p->left, (char *)NULL);
		depth--;

		for (n = 0; n < depth; n++) {
			printf("   ");
		}
		printf(
		     "(%x): (left = %x, right = %x, block = %x, size = %d)\n",
			p, p->left, p->right, p->block, p->size);

		depth++;
		h_prtree(p->right, (char *)NULL);
		depth--;
	}
}
#endif /* DEBUG */

#ifdef MEMLOG
/* keep track of fast alloc sizes */
struct fast_bucket {
	caddr_t			base_addr;
	long			size;
};

#define NFAST 32
struct fast_bucket fast_buckets[NFAST];
int nfast = 0;
#endif /* MEMLOG */

/*
 * These routines are for quickly allocating and freeing memory in
 * some commonly used size.  The chunks argument is used to reduce the
 * number of calls to kmem_alloc, and to reduce memory fragmentation.
 * The base argument is a caller allocated caddr_t * which is the base
 * of the free list of pieces of memory.  None of this memory is ever
 * freed, so these routines should be used only for structures that
 * will be reused often.  These routines can only be called at process
 * level.
 */

caddr_t
h_kmem_fast_alloc_memory_(struct h_kmem_info *hp,
	register caddr_t *base,	
	register int size,
	register int chunks,
	boolean_t wait)
{
	register caddr_t p;

#ifdef MEMLOG
	if(memlog) {
		int i;
		GET_CALLER(caller);
		memory_log(H_KMEM_FAST_ALLOC_LOG, caller, size*chunks);

		for(i=0; i<nfast; i++) {
			if(fast_buckets[i].base_addr == *base)
				break;
		}
		if(i == nfast) {
			fast_buckets[nfast].base_addr = *base;
			fast_buckets[nfast].size = size;
			if(nfast < NFAST-1) nfast++;
		}
	}
#endif
	if (*base == 0) {	/* no free chunks */
		p = (caddr_t) h_kmem_alloc_memory_(hp,
			(u_int)(size * chunks), wait);
		if (p) for (;chunks--; p += size) h_kmem_fast_free(base, p);
		else return p;
	}
	p = *base;
	*base = *(caddr_t *)p;
	return (p);
}


h_kmem_fast_free(base, p)
	caddr_t *base, p;
{
#ifdef MEMLOG
	if(memlog) {
		int i, size;
		GET_CALLER(caller);

		for(i=0; i<nfast; i++) {
			if(fast_buckets[i].base_addr == *base)
				break;
		}
		if(i == nfast) 
			size = 0;
		else
			size = fast_buckets[i].size;

		memory_log(H_KMEM_FAST_FREE_LOG, caller, size);
	}
#endif

	*(caddr_t *)p = *base;
	*base = p;
}

/*
 * Like kmem_fast_alloc, but use kmem_zalloc instead of
 * kmem_alloc to bzero the memory upon allocation.
 */

caddr_t
h_kmem_fast_zalloc_memory_(struct h_kmem_info *hp,
	register caddr_t *base,	
	register int	size,
	register int	chunks,
	boolean_t wait)
{
	register caddr_t p;

#ifdef MEMLOG
	if(memlog) {
		GET_CALLER(caller);
		memory_log(H_KMEM_FAST_ZALLOC_LOG, caller, size*chunks);
	}
#endif
	if (*base == 0) {	/* no free chunks */
		p = h_kmem_zalloc_memory_(hp, (u_int)(size * chunks), wait);
		if (p) for (p += ((chunks - 1) * size); 
				chunks > 1; p -= size, chunks--) 
				h_kmem_fast_free(base, p);
	}
	else {
		p = *base;
		*base = *(caddr_t *)p;
		bzero(p, (u_int) size);
	}
	return p;
}
