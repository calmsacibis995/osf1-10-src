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
static char	*sccsid = "@(#)$RCSfile: vm_resident.c,v $ $Revision: 4.3.12.9 $ (DEC) $Date: 1994/01/14 18:50:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	vm/vm_page.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Resident memory management module.
 */
#include <cpus.h>

#include <kern/task.h>
#include <sys/types.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <mach/vm_prot.h>
#include <mach/vm_statistics.h>
#include <kern/zalloc.h>
#include <machine/machparam.h>		/* for spl's */
#include <vm/pmap.h>
#include <kern/xpr.h>
#include <kern/thread.h>
#include <vm/vm_debug.h>
#include <vm/vm_perf.h>
#include <sys/dk.h>	/* for SAR counters */

extern vm_offset_t pmap_mips_k0();          
extern boolean_t pmap_find_holes();
extern boolean_t ubc_page_stealer();
extern boolean_t vm_page_stealer();
extern vm_map_t	kernel_map;

/*
 *	Page coloring constants.  These will never change therefore, here is addequate.
 */

int 	vm_page_color_bucket_count = 0; /* number of page colors */
#define COLOR_BUCKET			vm_page_color_bucket_count-1	
#define	map_shift			3	/* insignificant bits of the map address */
#define log2default_secondary_cache_pages 7	/* log 2 of the default B-cache pages */
#define max_secondary_cache_pages	1024	/* largest possible number of pages in B-cache */
extern long rpcc();
int	log2secondary_cache_pages;		/* log 2 of the B-cache pages */

/*
 *	This module maps and initializes memory for several
 *	other modules at bootstrap time.  The following
 *	addresses and sizes describe these boot-time mappings.
 */

vm_offset_t	map_data;
vm_size_t	map_data_size;
vm_offset_t	kentry_data;
vm_size_t	kentry_data_size;
int		kentry_count = 256;		/* To init kentry_data_size */
vm_offset_t	zdata;
vm_size_t	zdata_size;

/*
 *	The vm_page_lookup() routine, which provides for fast
 *	(virtual memory object, offset) to page lookup, employs
 *	the following hash table.  The vm_page_{insert,remove}
 *	routines install and remove associations in the table.
 *	[This table is often called the virtual-to-physical,
 *	or VP, table.]
 */
typedef struct {
	vm_page_t	bk_pages;
	int		bk_count;
	decl_simple_lock_data(,bk_lock)
} vm_page_bucket_t;

vm_page_bucket_t *vm_page_buckets;		/* Array of buckets */
int		vm_page_bucket_count = 0;	/* How big is array? */
int		vm_page_hash_mask;		/* Mask for hash function */

vm_page_bucket_t *vm_page_color_buckets;        /* Array of color buckets */

/*
 * Verify that the page is not known to the pmap subsystem
 */

boolean_t vm_page_alloc_verify = FALSE;

/*
 * Reserved memory for privileged subsystems.
 */

extern int vm_page_free_reserved;
extern int vm_page_inactive_count;
extern int ubc_lru_page_count;
extern int vm_page_free_target;

/*
 *	The virtual page size is currently implemented as a runtime
 *	variable, but is constant once initialized using vm_set_page_size.
 *	This initialization must be done in the machine-dependent
 *	bootstrap sequence, before calling other machine-independent
 *	initializations.
 *
 *	All references to the virtual page size outside this
 *	module must use the PAGE_SIZE constant.
 */
vm_size_t	page_size  = 4096;
vm_size_t	page_mask  = 4095;
int		page_shift = 12;

/*
 *	Resident page structures are initialized from
 *	a template (see vm_page_alloc).
 *
 *	When adding a new field to the virtual memory
 *	object structure, be sure to add initialization
 *	(see vm_page_startup).
 */

struct vm_page	vm_page_template;

/*
 *	Resident pages that represent real memory
 *	are allocated from a free list.
 */

vm_page_t vm_page_queue_free;
decl_simple_lock_data(,vm_page_free_lock)
boolean_t vm_page_free_wanted;
int vm_page_free_count;
int vm_page_zeroed_count;
int vm_managed_pages;
int vm_page_kluster = 0;

/*
 *	The actual memory for these resident page structures
 *	is stolen at bootstrap time.  The vm_page_array_next
 *	and vm_page_array_end variables denote the range of
 *	stolen memory that has not yet been used.
 *
 *	Historically, the resident page structures formed
 *	an array, directly corresponding to physical addresses.
 *	The vm_page_array, first_page and last_page variables
 *	are preserved for debugging.  The first_phys_addr
 *	and last_phys_addr variables are preserved for debugging,
 *	and also for older machine-dependent modules that
 *	may still depend on them.
 */
vm_page_t	vm_page_array;
vm_offset_t	vm_page_array_next;
vm_offset_t	vm_page_array_end;
vm_size_t	vm_page_array_size = 0;

struct vm_holes {
  vm_offset_t phys_start;
  vm_offset_t phys_end;
  vm_offset_t page_start;
  vm_offset_t page_end;
};
typedef struct vm_holes  *vm_holes_t;
vm_holes_t vm_holes_start;
vm_holes_t vm_holes_end;
vm_offset_t vm_holes_size;

long		first_page;
long		last_page;
vm_offset_t	first_phys_addr;
vm_offset_t	last_phys_addr;




/*
 *	vm_set_page_size:
 *
 *	Sets the page size, perhaps based upon the memory
 *	size.  Must be called before any use of page-size
 *	dependent functions.
 *
 *	Sets page_shift and page_mask from page_size.
 */

void 
vm_set_page_size()
{
	page_mask = page_size - 1;

	if ((page_mask & page_size) != 0)
		panic("vm_set_page_size: page size not a power of two");

	for (page_shift = 0; ; page_shift++)
		if ((1 << page_shift) == page_size)
			break;
}

/*
 *	vm_page_startup:
 *
 *	Initializes the resident memory module.
 *
 *	Allocates memory for the page cells, and
 *	for the object/offset-to-page hash table headers.
 *	Each page cell is initialized and placed on the free list.
 */

vm_offset_t 
vm_page_startup(vm_offset_t vaddr)
{
  register vm_offset_t start;
  vm_offset_t end;
  vm_holes_t h;
  vm_offset_t phys_start;
  vm_offset_t phys_end;
  vm_offset_t phys;
  vm_offset_t span_size;
  vm_size_t phys_size;
  vm_offset_t alloc_size;
  vm_size_t tot_size = 0;
  vm_size_t holes_page;
  vm_offset_t array_start;
  vm_offset_t vm_page_buckets_size;
  vm_offset_t vm_page_color_buckets_size;
  int tmp;
  register vm_page_t	m;
  register vm_page_bucket_t *bucket;
  int i;
  extern vm_offset_t avail_start, avail_end;

  vm_offset_t vm_check_page();

  m = &vm_page_template;
  m->pg_object = VM_OBJECT_NULL;
  m->pg_offset = 0;
  m->pg_wire_count = 0;
  m->pg_iocnt = 0;
  m->pg_reserved = 0;
  m->pg_free = 0;
  m->pg_busy = 1;
  m->pg_wait = 0;
  m->pg_error = 0;
  m->pg_dirty = 0;
  m->pg_hold = 0;
  m->pg_phys_addr = 0;
  for (i = 0; i < VM_PAGE_PRIVATE; i++) 
    m->pg_private[i] = (vm_offset_t) 0;
  
  /*
   *	Initialize the free page queues.
   */
  
  simple_lock_init(&vm_page_free_lock);
  vm_page_queue_free = VM_PAGE_NULL;
  
  vm_page_free_wanted = FALSE;
  
  /*
   *	Normalize the physical address range
   */
  
  start = round_page(avail_start);
  end = trunc_page(avail_end);
  
  /*
   *	Allocate (and initialize) the virtual-to-physical
   *	table hash buckets.
   *
   *	The number of buckets should be a power of two to
   *	get a good hash function.  The following computation
   *	chooses the first power of two that is greater
   *	than the number of physical pages in the system.
   */
  
  if (vm_page_bucket_count == 0) {
    vm_page_bucket_count = 1;
    while (vm_page_bucket_count < atop(end - start))
      vm_page_bucket_count <<= 1;
  }
  
  vm_page_hash_mask = vm_page_bucket_count - 1;
  vm_page_buckets_size = vm_page_bucket_count * sizeof(vm_page_bucket_t);
  
  if (vm_page_hash_mask & vm_page_bucket_count)
    printf("vm_page_startup: WARNING -- strange page hash\n");
  
  /*
   *	Get physically contiguous addresses.
   */
  

#define PMAP_MAP(size)							\
	MACRO_BEGIN							\
	vm_size_t	o;						\
	vm_size_t	amount_to_map;					\
									\
	amount_to_map = round_page(size);				\
	for (o = amount_to_map; o != 0; o -= PAGE_SIZE) {		\
		while (!pmap_valid_page(start))				\
			start += PAGE_SIZE;				\
		pmap_enter(kernel_pmap, vaddr, start,			\
			VM_PROT_READ|VM_PROT_WRITE, FALSE,		\
			VM_PROT_READ|VM_PROT_WRITE);			\
		vaddr += PAGE_SIZE;					\
		start += PAGE_SIZE;					\
	}								\
	blkclr((caddr_t) (vaddr - amount_to_map), amount_to_map);	\
	MACRO_END

        start = vm_check_page(start,round_page(vm_page_buckets_size),
                              start,end);

        vm_page_buckets = (vm_page_bucket_t *)pmap_mips_k0(&vaddr, start,
                            round_page(vm_page_buckets_size));

        start += round_page(vm_page_buckets_size);

	for (bucket = vm_page_buckets, i = vm_page_bucket_count; i--; 
		bucket++) {
		bucket->bk_pages = VM_PAGE_NULL;
		bucket->bk_count = 0;
		simple_lock_init(&bucket->bk_lock);
	}

	/*
	 *	Allocate and initialize the page color buckets
	 */

	vm_page_color_bucket_count = secondary_cache_size(start);
	vm_page_color_buckets_size = vm_page_color_bucket_count *
					sizeof(vm_page_bucket_t); 

        start = vm_check_page(start,round_page(vm_page_color_buckets_size),
                              start,end);

	vm_page_color_buckets = (vm_page_bucket_t *)pmap_mips_k0(&vaddr, start,
                            round_page(vm_page_color_buckets_size));

	start += round_page(vm_page_color_buckets_size);
	
        for (bucket = vm_page_color_buckets, i = vm_page_color_bucket_count; i--;
                bucket++) {
                bucket->bk_pages = VM_PAGE_NULL;
                bucket->bk_count = 0;
                simple_lock_init(&bucket->bk_lock);
        }

	/*
	 *	Steal pages for some zones that cannot be
	 *	dynamically allocated.
	 */

	/**/ {
	vm_size_t	size;

	zdata_size = round_page(128 * sizeof(struct zone));
	zdata = (vm_offset_t) vaddr;
	size = zdata_size;

	map_data_size = round_page(10 * sizeof(struct vm_map));
	map_data = (vm_offset_t) vaddr + size;
	size += map_data_size;

	kentry_data_size = round_page(kentry_count*sizeof(struct vm_map_entry));
	kentry_data = (vm_offset_t) vaddr + size;
	size += kentry_data_size;

	PMAP_MAP(size);
	/**/ }

	first_page = atop(first_phys_addr = start);
	last_page = atop(last_phys_addr = end - 1);

       /* Call a pmap routine to lay out where the memory/holes
        * are found.  This routine will be called more than once for
        * architectures supporting holes
        */

        phys = start;

Retry:
        phys = vm_check_page(phys,(vm_offset_t)PAGE_SIZE,start,end);
        vm_holes_start = (h = (vm_holes_t)pmap_mips_k0(&vaddr, phys, PAGE_SIZE));
        phys += (vm_offset_t)PAGE_SIZE;
        start = phys;
        holes_page = (vm_size_t)PAGE_SIZE;

        while (phys < end) {
           if (pmap_find_holes(&phys, &phys_size, &span_size, start, end) == KERN_SUCCESS) {
              h->phys_start = phys;
              h->phys_end = phys + phys_size - PAGE_SIZE;
              phys = phys + phys_size + span_size;
              tot_size += phys_size;
              h++; holes_page -= sizeof(struct vm_holes);
              if (holes_page < sizeof(struct vm_holes)) {
                 if (!pmap_valid_page(vm_holes_start->phys_start)){
		   phys = vm_holes_start->phys_start + (vm_offset_t)PAGE_SIZE;
		   goto Retry;
		 }
                 vm_holes_start->phys_start += (vm_offset_t)PAGE_SIZE;
                 holes_page += (vm_size_t)PAGE_SIZE;
	       }
	    }
           else 
              panic("vm_page_startup: unable to configure memory holes");
	 }
        vm_holes_end = h;
        vaddr = round_page(vaddr);

        vm_page_array_size = atop(tot_size) * sizeof(struct vm_page);


       /* Verify that the physical pages slated for vm_page_array 
        * will be physically contiguous (no intervening bad pages)
        */

        array_start = vm_holes_start->phys_start;
        array_start = (vm_holes_start->phys_start = vm_check_page(array_start,
                        round_page(vm_page_array_size),start,end)); 

        vm_page_array = (vm_page_t)pmap_mips_k0(&vaddr, array_start, 
                         round_page(vm_page_array_size));
        vm_page_array_end = (vm_offset_t)vm_page_array + (vm_offset_t)vm_page_array_size;

	vm_holes_start->phys_start += round_page((vm_offset_t)vm_page_array_size);
	/* assign physical address of 1st available page after vm_page_array */
	avail_start = vm_holes_start->phys_start;

        vm_page_create();

	return(vaddr);
}


/*
 *	Routine:	vm_check_page
 *	Purpose:
 *              Allocate physically congigous memory
 *              jumping around holes if necessary.
 */

vm_offset_t
vm_check_page(vm_offset_t check_phys_start, vm_offset_t check_alloc_size, 
              vm_offset_t check_start, vm_offset_t check_end)

{
  vm_offset_t _check_size;
  vm_offset_t _phys_size;
  vm_offset_t _span_size;
  vm_offset_t _phys;
  vm_offset_t _phys_check;
  int tmp;

  _phys_size = 0;
  _phys = check_phys_start;
  while (_phys < check_end){
    _check_size = check_alloc_size;
    if (_phys_size <= 0)
      tmp = pmap_find_holes(&_phys, &_phys_size, &_span_size, 
                            check_start, check_end);
    while ((_check_size > 0) && (_phys_size > 0)){
      _phys_check = _phys;
      while (!pmap_valid_page(_phys) && (_phys_size > 0)){
	_phys += (vm_offset_t)PAGE_SIZE;
	_phys_size -= (vm_offset_t)PAGE_SIZE;
      }
      if (_phys != _phys_check)
	break;
      _phys += (vm_offset_t)PAGE_SIZE;
      _phys_size -= (vm_offset_t)PAGE_SIZE;
      _check_size -= (vm_offset_t)PAGE_SIZE;
    }
    if (!_check_size)
      break;
  }
  if (_check_size > 0){
    printf("check_start %x\n",check_start);
    printf("check_end %x\n",check_end);
    printf("_phys %x\n",_phys);
    printf("check_phys_start %x\n", check_phys_start);
    printf("check_alloc_size %x\n", check_alloc_size);
    printf("_check_size %x\n", _check_size);
    printf("_phys_check %x\n", _phys_check);
    panic("vm_resident: cannot allocate contiguous memory");
    }                                                                       
  return (_phys - check_alloc_size);
}


/*
 *	Routine:	vm_page_module_init
 *	Purpose:
 *		Second initialization pass, to be done after
 *		the basic VM system is ready.
 */

void
vm_page_module_init()
{
	vm_pageout_init();				
}

/*
 * Enter the page in the objects lists of pages
 */

void
vm_page_insert_object(register vm_page_t *plist,
	register vm_page_t pp)
{

#define	vm_page_insert_object(PLIST, PG) pgl_insert_tail(*(PLIST),PG,o)
	vm_page_insert_object(plist, pp);
}

/*
 * Remove page from object list of pages.
 */

void
vm_page_remove_object(register vm_page_t *plist,
	register vm_page_t pp)
{

#define	vm_page_remove_object(PLIST, PG) pgl_remove(*(PLIST),PG,o)	
	vm_page_remove_object(plist, pp);
}

/*
 *	vm_page_hash:
 *
 *	Distributes the object/offset key pair among hash buckets.
 *
 *	NOTE:	To get a good hash function, the bucket count should
 *		be a power of two.
 */

#define vm_page_hash(object, offset) \
	(((unsigned)object+(unsigned)atop(offset))&vm_page_hash_mask)

/*
 *
 *	The object and page must be locked.
 */


void 
vm_page_insert_bucket(register vm_page_t mem, 
	register vm_object_t object, 
	register vm_offset_t offset)
{

#define	vm_page_insert_bucket(PG, OBJ, OFF) {				\
	register vm_page_t PP;						\
	register vm_page_bucket_t *BK;					\
	register int S;							\
	BK = &vm_page_buckets[vm_page_hash(OBJ, OFF)];			\
	S = splimp();							\
	simple_lock(&BK->bk_lock);					\
	if ((PP = BK->bk_pages) == VM_PAGE_NULL) {			\
		(PG)->pg_hnext = (PG)->pg_hprev = (PG);			\
		BK->bk_pages = (PG);					\
	}								\
	else {								\
		(PG)->pg_hnext = PP;					\
		(PG)->pg_hprev = PP->pg_hprev;				\
		PP->pg_hprev->pg_hnext = (PG);				\
		PP->pg_hprev = (PG);					\
	}								\
	BK->bk_count++;							\
	simple_unlock(&BK->bk_lock);					\
	(void) splx(S);							\
}
	vm_page_insert_bucket(mem, object, offset);
}

void
vm_page_remove_bucket(register vm_page_t pp)
{

#define	vm_page_remove_bucket(PG) {					\
	register vm_page_bucket_t *BK;					\
	register int S;							\
	BK = &vm_page_buckets[						\
		vm_page_hash((PG)->pg_object, (PG)->pg_offset)];	\
	S = splimp();							\
	simple_lock(&BK->bk_lock);					\
	if ((PG)->pg_hnext == (PG)) 					\
		(BK)->bk_pages = VM_PAGE_NULL;				\
	else {								\
		if ((PG) == (BK)->bk_pages)				\
			(BK)->bk_pages  = (PG)->pg_hnext;		\
		(PG)->pg_hprev->pg_hnext = (PG)->pg_hnext;		\
		(PG)->pg_hnext->pg_hprev = (PG)->pg_hprev;		\
	}								\
	BK->bk_count--;							\
	simple_unlock(&BK->bk_lock);					\
	(void) splx(S);							\
}
	vm_page_remove_bucket(pp);
}

/*
 *	vm_page_init:
 *
 *	Initialize the given vm_page, entering it into
 *	the VP table at the given (object, offset),
 *	and noting its physical address.
 *
 *	Implemented using a template set up in vm_page_startup.
 *	All fields except those passed as arguments are static.
 */

void
vm_page_init(register vm_page_t pp, 
	register vm_object_t object, 
	register vm_offset_t offset)
{

#define vm_page_clear(PG) { 						\
	register vm_offset_t PHYS;					\
	PHYS = (PG)->pg_phys_addr;					\
	*(PG) = vm_page_template; 					\
	(PG)->pg_phys_addr = PHYS;					\
}

#define	vm_page_insert(PG, OBJ, OFFSET) {				\
	(PG)->pg_offset = OFFSET;					\
	(PG)->pg_object = OBJ;						\
	vm_page_insert_object(&(OBJ)->ob_memq, (PG));			\
	(OBJ)->ob_resident_pages++;					\
	vm_page_insert_bucket(PG, OBJ, OFFSET);				\
}

	vm_page_clear(pp);
	if (object) vm_page_insert(pp, object, offset);
}

#if	VM_FREE_CHECK
#define	pgl_allocate_page_stub(Q,P,F)	pgl_allocate_page(P)
#define	pgl_free_remove 		pgl_allocate_page_stub
#define	pgl_free_page_stub(Q,P,F)	pgl_free_page(P)
#define	pgl_free_insert_tail		pgl_free_page_stub
#else
#define	pgl_free_remove 		pgl_remove
#define	pgl_free_insert_tail		pgl_insert_tail
#define	pgl_free_insert			pgl_insert
#endif	/* VM_FREE_CHECK */

void
pgl_insert_head_color_bucket(register vm_page_t pg)
{

        register vm_page_t pp;
        register vm_page_bucket_t *bk;
        register int s;
        bk = &vm_page_color_buckets[((pg->pg_phys_addr >> page_shift)&COLOR_BUCKET)];
        s = splimp();
        simple_lock(&bk->bk_lock);
        if ((pp = bk->bk_pages) == VM_PAGE_NULL) {
                pg->pg_hnext = pg->pg_hprev = pg;
                bk->bk_pages = pg;
        }
        else {
                pg->pg_hnext = pp;
                pg->pg_hprev = pp->pg_hprev;
                pp->pg_hprev->pg_hnext = pg;
                pp->pg_hprev = pg;
                bk->bk_pages = pg;
        }
        bk->bk_count++;
        simple_unlock(&bk->bk_lock);
        (void) splx(s);
}

void
pgl_insert_tail_color_bucket(register vm_page_t pg)
{

        register vm_page_t pp;
        register vm_page_bucket_t *bk;
        register int s;
        bk = &vm_page_color_buckets[((pg->pg_phys_addr >> page_shift)&COLOR_BUCKET)];
        s = splimp();
        simple_lock(&bk->bk_lock);
        if ((pp = bk->bk_pages) == VM_PAGE_NULL) {
                pg->pg_hnext = pg->pg_hprev = pg;
                bk->bk_pages = pg;
        }
        else {
                pp = pp->pg_hprev;
                pg->pg_hprev = pp;
                pg->pg_hnext = pp->pg_hnext;
                pp->pg_hnext->pg_hprev = pg;
                pp->pg_hnext = pg;
        }
        bk->bk_count++;
        simple_unlock(&bk->bk_lock);
        (void) splx(s);
}

void
pgl_remove_color_bucket(register vm_page_t pg)
{

        register vm_page_t pp;
        register vm_page_bucket_t *bk;
        register int s,i;
        bk = &vm_page_color_buckets[((pg->pg_phys_addr >> page_shift)&COLOR_BUCKET)];
        s = splimp();
        simple_lock(&bk->bk_lock);
        if ((bk->bk_pages == VM_PAGE_NULL) || (bk->bk_count < 1)) 
                panic("vm_page_color_bucket is empty");
        if (bk->bk_pages == pg) {
                if(bk->bk_count == 1)
                        bk->bk_pages = VM_PAGE_NULL;
                else {
                        pg->pg_hnext->pg_hprev = pg->pg_hprev;
                        pg->pg_hprev->pg_hnext = pg->pg_hnext;
                        bk->bk_pages = pg->pg_hnext;
                }
        } else {
                pg->pg_hprev->pg_hnext = pg->pg_hnext;
                pg->pg_hnext->pg_hprev = pg->pg_hprev;
        }
        bk->bk_count--;
        simple_unlock(&bk->bk_lock);
        (void) splx(s);
}

/*
 *	Get an initial page color bucket for a given map 
 */

int
vm_initial_color_bucket(vm_offset_t map)
{
	return(((vm_offset_t)map >> map_shift) & COLOR_BUCKET);
}

/*
 *	Get a page out of this bucket if there is one. If there is none, look in 
 *	the next sequential bucket up to log2 number of  buckets.
 *	If all those buckets were empty, grab one off the free list.
 */

int
get_color_bucket(int bucket, boolean_t zeroed_page)
{
	register vm_page_bucket_t *bk;
	register int i;

	for (i=0; i<log2secondary_cache_pages; i++) {
		bucket++;
		bucket &= COLOR_BUCKET;
		bk = &vm_page_color_buckets[bucket];
		if (bk->bk_pages != VM_PAGE_NULL)
			return bucket;
	}
        return (zeroed_page ?
                ((vm_page_queue_free->pg_pprev->pg_phys_addr >> page_shift)&COLOR_BUCKET) :
                ((vm_page_queue_free->pg_phys_addr >> page_shift)&COLOR_BUCKET));
}

int vm_page_stealer_enabled = 1;
	
/*
 * Allocate a page
 */

void
vm_pg_alloc(vm_page_t *ppp) 
{
	register vm_page_t pp;

#define vm_pg_alloc(PG) {						\
	register int BUCKET;						\
	register vm_page_bucket_t *BK;					\
	register vm_map_t MAP;						\
	register thread_t THREAD;					\
	register int S;							\
	boolean_t retry_alloc = TRUE;					\
	S = splimp();							\
	simple_lock(&vm_page_free_lock);				\
retry:									\
        if ((!vm_page_free_count) ||                                    \
            ((vm_page_free_count < vm_page_free_reserved) &&            \
             (!current_thread()->vm_privilege))) {                      \
                if ((vm_page_stealer_enabled) && (retry_alloc)) {       \
                    (void) splx(S);                                     \
                    if (ubc_lru_page_count > vm_page_inactive_count)    \
                        ubc_page_stealer();                             \
                    else                                                \
                        vm_page_stealer();                              \
                    S = splimp();                                       \
                    retry_alloc = FALSE;                                \
                    goto retry;                                         \
                } else {                                                \
                        simple_unlock(&vm_page_free_lock);              \
                        (void) splx(S);                                 \
                        (PG) = VM_PAGE_NULL;                            \
                }                                                       \
	} else {								\
		if (THREAD = current_thread()) 				\
			MAP = THREAD->task->map;			\
		else 							\
			MAP = kernel_map;				\
		BUCKET = get_color_bucket(MAP->vm_color_bucket,FALSE); 	\
		MAP->vm_color_bucket = BUCKET;				\
		BK = &vm_page_color_buckets[BUCKET]; 			\
		PG = (BK)->bk_pages;			  		\
		pgl_free_remove(vm_page_queue_free,(PG),p);		\
		pgl_remove_color_bucket((PG));				\
		vm_page_free_count--;					\
		if ((PG)->pg_zeroed) 					\
			vm_page_zeroed_count--;				\
		pmap_clear_modify(page_to_phys(PG));			\
		vpf_store(freepages, vm_page_free_count);		\
		vpf_store(allocatedpages, 				\
			vm_managed_pages - vm_page_free_count);		\
		(PG)->pg_free = 0;					\
		simple_unlock(&vm_page_free_lock);			\
		splx(S);						\
	}								\
}
	
	vm_pg_alloc(pp);
	if ((*ppp = pp) != VM_PAGE_NULL) {
		vm_page_clear(pp);
		vm_page_sched();
	}
	return;
}

int	vm_zeroed_target_shift = 3;
int	vm_zeroed_min_shift = 4;
int	vm_zeroed_pages_wanted;

#define	vm_zeroed_page_sched() {					\
	if ((vm_page_zeroed_count < (vm_page_free_count >> vm_zeroed_min_shift)) && \
	    (vm_page_free_count >= vm_page_free_target))		\
		vm_zeroed_pages_wanted = 1;				\
}

/*
 * Allocate a zeroed page
 */

void
vm_zeroed_pg_alloc(vm_page_t *ppp) 
{
	register vm_page_t pp;

#define vm_zeroed_pg_alloc(PG) {					\
	register int BUCKET;						\
        register vm_page_bucket_t *BK;                                  \
	register thread_t THREAD;					\
	register vm_map_t MAP;						\
	register int S;							\
	boolean_t retry_alloc = TRUE;					\
	S = splimp();							\
	simple_lock(&vm_page_free_lock);				\
retry:                                                                  \
        if ((!vm_page_free_count) ||                                    \
            ((vm_page_free_count < vm_page_free_reserved) &&            \
             (!current_thread()->vm_privilege))) {                      \
                if ((vm_page_stealer_enabled) && (retry_alloc)) {       \
                    (void) splx(S);                                     \
                    if (ubc_lru_page_count > vm_page_inactive_count)    \
                        ubc_page_stealer();                             \
                    else                                                \
                        vm_page_stealer();                              \
                    S = splimp();                                       \
                    retry_alloc = FALSE;                                \
                    goto retry;                                         \
                } else {                                                \
                        simple_unlock(&vm_page_free_lock);              \
                        (void) splx(S);                                 \
                        (PG) = VM_PAGE_NULL;                            \
                }                                                       \
	} else {							\
                if (THREAD = current_thread()) 	                        \
                        MAP = THREAD->task->map;                        \
                else                                                    \
                        MAP = kernel_map;                               \
                BUCKET = get_color_bucket(MAP->vm_color_bucket,TRUE); 	\
                MAP->vm_color_bucket = BUCKET;                  	\
                BK = &vm_page_color_buckets[BUCKET]; 			\
		PG = (BK)->bk_pages->pg_hprev; 	           		\
		pgl_free_remove(vm_page_queue_free,(PG),p);		\
                pgl_remove_color_bucket((PG));                          \
		vm_page_free_count--;					\
		if ((PG)->pg_zeroed)					\
			vm_page_zeroed_count--;				\
		vpf_store(freepages, vm_page_free_count);		\
		vpf_store(allocatedpages, 				\
			vm_managed_pages - vm_page_free_count);		\
		(PG)->pg_free = 0;					\
		simple_unlock(&vm_page_free_lock);			\
		splx(S);						\
		if (!(PG)->pg_zeroed)					\
			pmap_zero_page(page_to_phys(PG));		\
	}								\
}
	
	vm_zeroed_pg_alloc(pp);
	if ((*ppp = pp) != VM_PAGE_NULL) {
		vm_page_clear(pp);
		vm_page_sched();
		vm_zeroed_page_sched();
	}
	return;
}

#if	VM_FREE_TRACE

#define	VM_FREE_TRACE_SIZE	3000
long	vm_free_trace_index = 0;
struct vm_free_trace_data {
	long		vft_pc;
	vm_page_t	vft_pp;
} vm_free_trace_data[VM_FREE_TRACE_SIZE];

#define	vm_free_trace(PP) {						\
	vm_free_trace_data[vm_free_trace_index].vft_pc =		\
		get_caller_ra();					\
	vm_free_trace_data[vm_free_trace_index].vft_pp = (PP);		\
	if (++vm_free_trace_index >= VM_FREE_TRACE_SIZE)		\
		vm_free_trace_index = 0;				\
}

#else

#define	vm_free_trace(PP)

#endif	/* VM_FREE_TRACE */

void
vm_pg_free(register vm_page_t pp)
{

/*
 * Deallocate a page
 */

#define vm_pg_free(PG) {						\
	register int S;							\
	S = splimp();							\
	simple_lock(&vm_page_free_lock);				\
	vm_free_trace(PG);						\
	pgl_free_insert(vm_page_queue_free,PG,p);			\
	pgl_insert_head_color_bucket(PG);				\
	(PG)->pg_free = 1;						\
	vm_page_free_count++;						\
	vpf_store(freepages, vm_page_free_count);			\
	vpf_store(allocatedpages, 					\
		vm_managed_pages - vm_page_free_count);			\
	if (vm_page_free_wanted) {					\
		thread_wakeup((vm_offset_t) &vm_page_free_count);	\
		vm_page_free_wanted = FALSE;				\
	}								\
	simple_unlock(&vm_page_free_lock);				\
	splx(S);							\
}

	/* global table() system call counter (see table.h) */
	pg_v_dfree++;

	vm_pg_free(pp);
}

void
vm_zeroed_pg_free(register vm_page_t pp)
{

/*
 * Deallocate a page that has been zeroed.
 */

#define vm_zeroed_pg_free(PG) {						\
	register int S;							\
	S = splimp();							\
	simple_lock(&vm_page_free_lock);				\
	vm_free_trace(PG);						\
	pgl_free_insert_tail(vm_page_queue_free,PG,p);			\
	pgl_insert_tail_color_bucket(PG);				\
	(PG)->pg_zeroed = 1;						\
	vm_page_free_count++;						\
	vm_page_zeroed_count++;						\
	vpf_store(freepages, vm_page_free_count);			\
	vpf_store(allocatedpages, 					\
		vm_managed_pages - vm_page_free_count);			\
	if (vm_page_free_wanted) {					\
		thread_wakeup((vm_offset_t) &vm_page_free_count);	\
		vm_page_free_wanted = FALSE;				\
	}								\
	simple_unlock(&vm_page_free_lock);				\
	splx(S);							\
}

	vm_zeroed_pg_free(pp);
}

void 
vm_page_create(void)
{
  vm_holes_t      h;
  static
    vm_page_t       p;
  vm_offset_t     phys;
  vm_offset_t     phys_start;
  vm_offset_t     phys_end;       
  
  p = vm_page_array;
  
  for (h = vm_holes_start; h < vm_holes_end; h++) {
    h->page_start = (vm_offset_t)p;
    phys_start = h->phys_start;
    phys_end = h->phys_end;
    
    /* Note that bad pages will still have to have a page structure
     * assigned to them.  Otherwise we can't calculate page structure
     * given a physical page.
     */
    
    for (phys = phys_start; phys <= phys_end; phys += PAGE_SIZE) {
      vm_page_clear(p);
      p->pg_phys_addr = phys;
      if (pmap_valid_page(phys)) {
	      vm_pg_free(p);
	      vm_managed_pages++;
      }
      p++;
    }

    /*
     * Global table() system call counter (see table.h).
     * Start with 0 pages freed -- these calls to vm_pg_free() were
     * just to set up the pages initially.
     */
    pg_v_dfree = 0;

    h->page_end = (vm_offset_t)p - sizeof(struct vm_page);
    
  }
}

/*
 *	vm_page_remove:		[ internal use only ]
 *
 *	Removes the given mem entry from the object/offset-page
 *	table and the object page list.
 *
 *	The object and page must be locked.
 */

void 
vm_page_remove(register vm_page_t pp)
{
	VM_PAGE_CHECK(pp);

	vm_page_remove_bucket(pp);
	pp->pg_object->ob_resident_pages--;
	vm_page_remove_object(&pp->pg_object->ob_memq,pp);
	
}

void
vm_page_rename(register vm_page_t pp,
	register vm_object_t obj,
	register vm_offset_t offset)
{
	VM_PAGE_CHECK(pp);
	pp->pg_object = obj;
	vm_page_insert_bucket(pp,obj,offset);
	vm_page_insert_object(&obj->ob_memq,pp);
	obj->ob_resident_pages++;
}

/*
 *	vm_page_lookup:
 *
 *	Returns the page associated with the object/offset
 *	pair specified; if none is found, VM_PAGE_NULL is returned.
 *
 *	The object must be locked.  No side effects.
 */

vm_page_t 
vm_page_lookup(register vm_object_t object, 
	register vm_offset_t offset)
{
	register vm_page_t pp;
	register vm_page_bucket_t *bp;
	register int s;

	/*
	 *	Search the hash table for this object/offset pair
	 */

	bp = &vm_page_buckets[vm_page_hash(object, offset)];

	s = splimp();
	simple_lock(&bp->bk_lock);
	pp = bp->bk_pages;
	if (pp != VM_PAGE_NULL) do {
		if ((pp->pg_object == object) && (pp->pg_offset == offset)) {
			simple_unlock(&bp->bk_lock);
			splx(s);
			return pp;
		}
		pp = pp->pg_hnext;
	} while (pp != bp->bk_pages);
	simple_unlock(&bp->bk_lock);
	splx(s);
	return VM_PAGE_NULL;
}



/*
 *	vm_page_alloc:
 *
 *	Allocate and return a memory cell associated
 *	with this VM object/offset pair.
 *
 *	Object must be locked.
 */

vm_page_t 
vm_page_alloc(register vm_object_t object, 
	vm_offset_t offset)
{
	register vm_page_t pp;

	vm_pg_alloc(pp);
	if (pp == VM_PAGE_NULL) return pp;

	if (vm_page_alloc_verify && !pmap_verify_free(page_to_phys(pp)))
		Debugger("vm_page_alloc: page is in use!\n");

	vm_page_clear(pp);
	vm_page_insert(pp, object, offset);

	return pp;
}

/*
 *	vm_zeroed_page_alloc:
 *
 *	Allocate and return a memory cell associated
 *	with this VM object/offset pair.
 *
 *	Object must be locked.
 */

vm_page_t 
vm_zeroed_page_alloc(register vm_object_t object, 
	vm_offset_t offset)
{
	register vm_page_t pp;

	vm_zeroed_pg_alloc(pp);
	
	if (pp == VM_PAGE_NULL) return pp;

	if (vm_page_alloc_verify && !pmap_verify_free(page_to_phys(pp)))
		Debugger("vm_page_alloc: page is in use!\n");

	vm_page_clear(pp);
	vm_page_sched();
	vm_zeroed_page_sched();
	vm_page_insert(pp, object, offset);

	return pp;
}

/*
 * Allocate N pages starting at this object's
 * offset. The pages are wired down.
 * These pages aren't in the bucket list.
 */
 
vm_page_t
vm_pages_alloc_private(register vm_object_t object,
	register vm_offset_t offset,
	vm_size_t size)
{
	register int s, pages, i;
	register vm_page_t first, last, next;

	pages = atop(size);
	if (pages < 1) panic("vm_pages_alloc_private: bad count");
	s = splimp();
	simple_lock(&vm_page_free_lock);

	if ((pages > vm_page_free_count) ||
		((vm_page_free_count < vm_page_free_reserved) && 
		!current_thread()->vm_privilege)) {
		simple_unlock(&vm_page_free_lock);
		(void) splx(s);
		vm_page_sched();
		return VM_PAGE_NULL;
	}

	for (last = first = vm_page_queue_free, i = 1; i  < pages; i++ ) 
		last = last->pg_pnext;

	/*
	 * Update the free list so that we can play with the pages.
	 */

	vm_page_free_count -= pages;
	if (vm_page_free_count) {
		vm_page_queue_free = last->pg_pnext;
		vm_page_queue_free->pg_pprev = first->pg_pprev;
		first->pg_pprev->pg_pnext = vm_page_queue_free;
	}
	else vm_page_queue_free = VM_PAGE_NULL;
	vpf_store(freepages, vm_page_free_count);
	vpf_store(allocatedpages, vm_managed_pages - vm_page_free_count);	
	simple_unlock(&vm_page_free_lock);
	(void) splx(s);

	
	for (i = 0, last->pg_pnext = VM_PAGE_NULL, last = first; i < pages; 
		i++, offset += PAGE_SIZE) {

		next = last->pg_pnext;
		pgl_remove_color_bucket(last);
		vm_page_clear(last);
		vm_page_insert_object(&object->ob_memq, last);
		object->ob_resident_pages++;
		last->pg_offset = offset;
		last->pg_object = object;
		vm_page_wire(last, FALSE);
		last->pg_pnext = next;
		last = next;
	}
	return first;
}
	
 


void
vm_wait()
{
	int wakeup, s;

	s = splimp();
	simple_lock(&vm_page_free_lock);
	wakeup = !vm_page_free_wanted;
	vm_page_free_wanted = TRUE;
	assert_wait((vm_offset_t)&vm_page_free_count, FALSE);
	simple_unlock(&vm_page_free_lock);
	splx(s);
	if (wakeup)
		thread_wakeup((vm_offset_t)&vm_page_free_wanted);
	thread_block();
}


/*
 *	vm_page_zero_fill:
 *
 *	Zero-fill the specified page.
 */

void 
vm_page_zero_fill(vm_page_t m)
{
	VM_PAGE_CHECK(m);

	pmap_zero_page(page_to_phys(m));
}

/*
 *	vm_page_copy:
 *
 *	Copy one page to another
 */

void 
vm_page_copy(vm_page_t src_m, vm_page_t dest_m)
{
	VM_PAGE_CHECK(src_m);
	VM_PAGE_CHECK(dest_m);


	pmap_copy_page(page_to_phys(src_m), page_to_phys(dest_m));
}

/*
 * Currently only a single lock supports changes
 * in state to a pages hold field and busy field.
 */

udecl_simple_lock_data(,vm_page_lock_data)


vm_page_lock_init()
{
	simple_lock_init(&vm_page_lock_data);
}

/*
 * Hold the page prevent pageout from manipulating it.
 * We can also recover the page in some circumstances
 * while its being pushed.
 */

#ifdef	vm_page_hold
#undef	vm_page_hold
#endif	/* vm_page_hold */

boolean_t
vm_page_hold(register vm_page_t pp)
{
	boolean_t busy;

	vm_page_lock(pp);
	pp->pg_hold++;
	busy = pp->pg_busy == 1;
	vm_page_unlock(pp);
	return busy;
}

/*
 * Release this page to the paging subsystem.
 */

#ifdef	vm_page_release
#undef	vm_page_release
#endif	/* vm_page_release */

vm_page_release(register vm_page_t pp)
{
	vm_page_lock(pp);
	pp->pg_hold--;
	vm_page_unlock(pp);
}

#if	VM_FREE_CHECK
pgl_allocate_page(register vm_page_t pp)
{
	if (pp->pg_pnext == pp)  vm_page_queue_free = VM_PAGE_NULL;	
	else {								
		if (vm_page_queue_free == pp) 
			vm_page_queue_free = pp->pg_pnext;	
		pp->pg_pnext->pg_pprev = pp->pg_pprev;			
		pp->pg_pprev->pg_pnext = pp->pg_pnext;			
		pgl_remove_color_bucket(pp);
	}								
}
pgl_free_page(register vm_page_t pp)
{
	register vm_page_t pg;					

	vm_free_check_pp(pp);
	if (vm_page_queue_free == VM_PAGE_NULL) {
		vm_page_queue_free = pp;
		pp->pg_pnext = pp->pg_pprev = pp;
	}								
	else {								
		pg = vm_page_queue_free;
		pg->pg_pprev->pg_pnext = pp;
		pp->pg_pprev = pg->pg_pprev;
		pg->pg_pprev = pp;
		pp->pg_pnext = pg;
	}								
	pgl_insert_head_color_bucket(pp);
}
#endif	/* VM_FREE_CHECK */

void
vm_page_zeroer()
{
	vm_page_t pp;

	vm_pg_alloc(pp);	
				
	if (pp == VM_PAGE_NULL) 
		goto exit;

	if (!pp->pg_zeroed) 
		pmap_zero_page(page_to_phys(pp));
	else
		pmap_set_modify(page_to_phys(pp));
	
	vm_zeroed_pg_free(pp);
		
exit:
	if ((vm_page_zeroed_count >= (vm_page_free_count >> vm_zeroed_target_shift)) ||
	    (vm_page_free_count < vm_page_free_target))
		vm_zeroed_pages_wanted = 0;

	return;
}

/*
 * 	This routine auto-sizes the secondary cache by measuring the number of cycles it
 *	takes to perform a sequence of reads and writes to the same offsets in two pages.
 *	We take a measurement and double the distance between the two pages.  Once the cycle
 *	time goes non-linear we assume that we have walked off the end of the secondary
 *	cache and therefore hace its size.  This routine assumes that the size of all 
 *	secondary caches is always a power of 2.  If this in ever not true, simply change 
 *	this routine increment the distance between two pages instead of doubling it,
 *	every thing else will work properly.
 */

int
secondary_cache_size(caddr_t start)
{
        {
                int 		i,j,temp,shift;
                unsigned long   cycles1, cycles2, cycles;
                unsigned long   base_cycles = 0;
                caddr_t 	first_page;
		vm_offset_t	vaddr;

                first_page = (caddr_t)pmap_mips_k0(&vaddr,start,(max_secondary_cache_pages));

                for(i=1,log2secondary_cache_pages=0;
	       	    i<max_secondary_cache_pages;
		    i=i<<1,log2secondary_cache_pages++) {

                        caddr_t last_page;
                        int     temp;

                        last_page = first_page + trunc_page(i*page_size);
                        cycles1 = (unsigned long)rpcc();
                        for (j=0;j<1000;j++) {
                                temp = *first_page;
                                *(int *)first_page = temp;
                                temp = *last_page;
                                *(int *)last_page = temp;
                        }
                        cycles2 = (unsigned long)rpcc();
                        cycles = ((cycles2>cycles1) ? (cycles2-cycles1) : ((2^64-cycles1)+cycles2));
                        if (!base_cycles) 
                                base_cycles = cycles;
                        if ((unsigned long)(cycles) > (unsigned long)(2*base_cycles))
				return i;
                }
		log2secondary_cache_pages = log2default_secondary_cache_pages;
		return (1<<log2default_secondary_cache_pages);
        }


}
