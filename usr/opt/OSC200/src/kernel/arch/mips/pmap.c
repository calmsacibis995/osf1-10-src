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
static char	*sccsid = "@(#)$RCSfile: pmap.c,v $ $Revision: 1.2.4.7 $ (DEC) $Date: 1992/09/29 09:18:04 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from pmap.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 *	File:	pmap.c
 *	Author: Alessandro Forin
 *
 *	Copyright (C) 1988, 1989 Alessandro Forin
 *
 *	MIPS Physical Map management code.
 *	The original pmap code was done for the Vax by
 *
 *		Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	which I used as a template.  Most of their comments are preserved.
 *
 */

/*
 *	Manages physical address maps.
 *
 *	In addition to hardware address maps, this
 *	module is called upon to provide software-use-only
 *	maps which may or may not be stored in the same
 *	form as hardware maps.  These pseudo-maps are
 *	used to store intermediate results from copy
 *	operations to and from address spaces.
 *
 *	Since the information managed by this module is
 *	also stored by the logical address mapping module,
 *	this module may throw away valid virtual-to-physical
 *	mappings at almost any time.  However, invalidations
 *	of virtual-to-physical mappings must be done as
 *	requested.
 *
 *	In order to cope with hardware architectures which
 *	make virtual-to-physical map invalidates expensive,
 *	this module may delay invalidate or reduced protection
 *	operations until such time as they are actually
 *	necessary.  This module is given full information as
 *	to which processors are currently using which maps,
 *	and to when physical maps must be made correct.
 */
/************************************************************************
 *
 *	Modification History: pmap.c
 * 10-Oct-91    Marian Macartney
 *   Clean up definitions from BL6.  Fix compiler errors
 *
 * 05-Sep-91    Marian Macartney
 *   Fix bug in pmap_collect which only evaluates the first pte in a page
 *   for wiring.  Revised version evaluates all ptes.
 *
 * 22-Jun-91    Marian Macartney
 *   Merged in OSF-1.0.1 changes.  Added reference bit implementation support.
 *   Note that this merge includes some definitions normally put in pmap.h - 
 *   which will have to go back there in the next BL.
 *
 *************************************************************************/

#ifdef notdef
#include <compacted.h>
#endif

#include <mach/vm_attributes.h>
#include <kern/macro_help.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <machine/cpu.h>
#include <machine/endian.h>

#include <machine/counters.h> 
#include <pmap_pcache.h>
#include <ref_bits.h>

extern vm_offset_t pmap_mips_k0();          
extern boolean_t pmap_find_holes();

#define	L1PT_SIZE	(MIPS_PGBYTES/2)

zone_t l1pt_zone;

vm_size_t pmap_seg_size = PTES_PER_PAGE * MIPS_PGBYTES;
vm_size_t pmap_seg_mask = (PTES_PER_PAGE * MIPS_PGBYTES) - 1;
extern struct pv_entry *pmap_seg_free_pagetable();
extern void pmap_seg_remove_all();


#define ALL_THRESHHOLD 64*1024
#define coprocessor_invalidate(pmap, va, size)			\
	if ((pmap)->coprocessor_vm_maint) {			\
		vm_size_t _SIZE = (size);			\
		if (_SIZE < ALL_THRESHHOLD) {			\
			while (_SIZE > 0) {			\
			(*(pmap)->coprocessor_vm_maint)		\
				(PDEVCMD_ONE, mips_btop(va));	\
			(va) += PAGE_SIZE;			\
			_SIZE -= PAGE_SIZE;			\
			}					\
		} else						\
			(*(pmap)->coprocessor_vm_maint)		\
				(PDEVCMD_ALL, (vm_offset_t)0);	\
	} else
			
#define TLBVASIZ  NTLBENTRIES*MIPS_PGBYTES

#define	lock_pmap(pmap)		simple_lock(&(pmap)->lock)
#define	unlock_pmap(pmap)	simple_unlock(&(pmap)->lock)

struct pmap	kernel_pmap_store;
pmap_t		kernel_pmap;	/* kernel pmap, statically alloced */
pmap_t		active_pmap;	/* pmap for the current_thread() */
vm_map_t	pmap_submap;	/* kernel submap where we get our pages */
vm_offset_t	pmap_vlow, pmap_vhigh;	/* (virtual) limits for our needs */




vm_offset_t	root_kptes;		/* Second level kernel ptes */

struct zone	*pmap_zone;		/* Zone of pmap structures  */
struct zone	*pmap_seg_zone;		/* Zone of segments */

vm_offset_t     pmap_resident_extract();

extern vm_offset_t pmap_resident_extract();
extern void pmap_release_page();
extern void pmap_clean_pcache();
extern void pmap_tbsync();
extern void pmap_cache_flush();
extern void pmap_pagemove();
extern void get_more_pte_pages();
extern void assign_tlbpid();
extern void destroy_tlbpid();

#if    COUNTERS
#define	DO_COUNT(x)	(x++);
#else	/* COUNTERS */
#define	DO_COUNT(x)
#endif	/* COUNTERS */

/*
 *	For each vm_page_t, there is a list of all currently
 *	valid virtual mappings of that page.  An entry is
 *	a pv_entry_t; the lists' heads are in the pv_table.
 *	There are analogous arrays for the modify (and reference) bits.
 */

struct pv_entry {
	struct pv_entry	*pv_next;		/* next pv_entry */
	union {
		pmap_t		_pv_pmap;	/* pmap where mapping lies */
		pmap_seg_t	_pv_seg;	/* segment */
		int		_pv_mappings;	/* valid mapping count */
	} _upv0;
	vm_offset_t	pv_va;			/* 
						 * virtual address 
						 * for mapping 
						 */
};

#define	pv_pmap	_upv0._pv_pmap
#define	pv_seg	_upv0._pv_seg
#define	pv_mappings	_upv0._pv_mappings

#define	pv_segtype	0x00000001		/* Ored in pv_va for seg id. */
#define	pv_isseg(PV)	((PV)->pv_va & pv_segtype)

#define	pte_to_pvh(PTE)							\
	pa_to_pvh(PTETOPHYS((PTE)))
#define	pmap_ptptetoseg(PTPTE)						\
	pa_to_pvh(PTETOPHYS((PTPTE)))->pv_seg
#define	pmap_segpt_alloc(SEG,PT)					\
	pa_to_pvh(K0_TO_PHYS(PT))->pv_seg = (SEG)
#define	pmap_segpt_free(PT)						\
	pa_to_pvh(K0_TO_PHYS(PT))->pv_pmap = kernel_pmap

#define	pmap_free_l2pte_list(PTE) do {					\
	register vm_offset_t NPTE;					\
	do {								\
		NPTE = * (vm_offset_t *) (PTE);				\
		put_free_ptepage((PTE));				\
		(vm_offset_t) (PTE) = NPTE;				\
	} while ((PTE) != (vm_offset_t) 0);				\
} while (1 == 0)

typedef struct pv_entry *pv_entry_t;
#define	PV_ENTRY_NULL	((pv_entry_t) 0)

pv_entry_t pmap_remove_range();	/* forward */
#define managed_phys(p)		((p) >= vm_first_phys && (p) < vm_last_phys)
/*
 * The pv_list proper, which is actually an array of lists
 */
pv_entry_t	pv_head_table;		/* array of entries, one per page */
zone_t		pv_list_zone;		/* zone of pv_entry structures */

#define	pa_index(PA)		(atop((PA) - vm_first_phys))
#define pa_to_pvh(PA)		(&pv_head_table[pa_index(PA)])
#define	pvh_to_pa(PH)		(vm_first_phys + (ptoa((PH) - pv_head_table)))

/* locking of the pv_list */
#define lock_pvh_pa(s,pa)	{ (s) = splvm();}
#define unlock_pvh_pa(s,pa)	{ splx((s));}

/*
 * The modify table is an array.  Could be compacted, but we might
 * need to put more info there sometimes.
 * Not all phys pages are accounted for.
 */
vm_offset_t	vm_first_phys;	/* range of phys pages which we can handle */
vm_offset_t	vm_last_phys;
vm_offset_t	vm_reserved_pte;/* boot strapped reserved pv */

char	*pmap_modify_bits;      /* dirty bits table */
char	*pmap_reference_bits;	/* reference bits array*/

#if	COMPACTED	/* needs adequate locking */
#define set_modify_bit(pa) 					\
	{ register int ix = pa_index(pa), s;			\
		s = splhigh();					\
		pmap_modify_bits[(ix >> 3)] |= (1 << (ix & 7));	\
		splx(s);					\
	}

#define clear_modify_bit(pa) 					\
	{ register int ix = pa_index(pa), s;			\
		s = splhigh();					\
		pmap_modify_bits[(ix >> 3)] &= ~(1 << (ix & 7));\
		splx(s);					\
	}

#define return_modify_bit(pa) 					\
	{ register int ix = pa_index(pa);			\
	return (pmap_modify_bits[(ix >> 3)] & (1 << (ix & 7))); \
	}

#else	/* COMPACTED */
#define set_modify_bit(pa) 					\
	{ register int ix = pa_index(pa);			\
		pmap_modify_bits[ix] = 1;			\
	}

#define clear_modify_bit(pa) 					\
	{ register int ix = pa_index(pa);			\
		pmap_modify_bits[ix] = 0;			\
	}

#define return_modify_bit(pa) 					\
	{ register int ix = pa_index(pa);			\
	return (pmap_modify_bits[ix]);                          \
	}

#endif	/* COMPACTED */

#define PMAP_SET_MODIFY(p)					\
	{							\
	    DO_COUNT(mod_count)				    	\
	    TRACE({printf("pmap_set_modify x%x\n", (p));})	\
	    if (managed_phys(p))				\
		set_modify_bit((p));				\
	}


#if	REF_BITS

#define PMAP_SET_REFERENCE(p)					\
	{							\
	    TRACE({printf("pmap_set_reference x%x\n", (p));})	\
	    if (managed_phys(p))				\
		set_reference_bit((p));				\
	}

#define set_reference_bit(pa) 					\
	{ register int ix = pa_index(pa);			\
		pmap_reference_bits[ix] = 1;			\
	}

#define set_refclu_bit(map,va)					\
      {                                                         \
       int __s;                                                 \
       vm_offset_t __tva = (va);                                \
       pte_template *__pte;                                     \
                                                                \
       __s = splvm();                                           \
       lock_pmap(map);                                          \
       __pte = (pte_template *) pmap_l1pte(map, va);		\
       __pte = (pte_template *) pmap_l2pte(__pte, va);		\
       __pte->raw = (__pte->raw & ~PG_V) | PG_REFCLU;		\
       unlock_pmap(map);                                        \
       splx(__s);                                               \
       }

#define return_reference_bit(pa) 				\
	{ register int ix = pa_index(pa);			\
	return (pmap_reference_bits[ix]);			\
	}

#define clear_reference_bit(pa)					\
	{ register int ix = pa_index(pa);			\
		pmap_reference_bits[ix] = 0;			\
	}
     
#else	/* REF_BITS */

#define PMAP_SET_REFERENCE(p)

#endif	 REF_BITS

/*
 *	Lock and unlock macros for the pmap system.  Interrupts must be locked
 *	out because memory might be allocated at interrupt level and cause
 *	a pmap_enter.
 */

#if MACH_SLOCKS
	/* This is a non-sleeping complex lock - only used for MP or DEBUG */
lock_data_t	pmap_lock;	/* General lock for the pmap system */

#define READ_LOCK(s)		s = splvm(); lock_read(&pmap_lock);
#define READ_UNLOCK(s)		lock_read_done(&pmap_lock); splx(s);
#define WRITE_LOCK(s)		s = splvm(); lock_write(&pmap_lock);
#define WRITE_UNLOCK(s)		lock_write_done(&pmap_lock); splx(s);
#else	/* MACH_SLOCKS */
#define READ_LOCK(s)		s = splvm();
#define READ_UNLOCK(s)		splx(s);
#define WRITE_LOCK(s)		s = splvm();
#define WRITE_UNLOCK(s)		splx(s);
#endif	/* MACH_SLOCKS */

static boolean_t pmap_module_is_initialized = FALSE;

int pmap_debug = 0;
#define TRACE(x) if (pmap_debug) x
/*#define TRACE(x) */

/*
 * Pool of physical pages for use as page tables.
 * The problem with allocating these pages is that
 * we may need pte pages to allocate pte pages.
 * The solution is to keep a reserved pool, which may
 * may only be used if a thread is trying to allocate pte pages,
 * and either it needs a pte page to do so, or a VM-privileged
 * thread needs pte pages (presumably to make more free pages).
 */

decl_simple_lock_data(,ptepages_lock) /* protects the following variables */
vm_offset_t	free_ptepages = 0;
int		free_ptepages_count = 0;
thread_t	ptepages_thread = THREAD_NULL;
boolean_t	ptepages_wanted = FALSE;

unsigned int	ptepages_freed = 0;
unsigned int	ptepages_taken = 0;

/* enough to cover emergency needs while in get_more_pte_pages */
int		min_pte_pages = 3;

/* leave enough for minimum and a couple processes */
#define	PMAP_MIN_PTEPAGES	5
int		max_pte_pages = 3 + 2 * PMAP_MIN_PTEPAGES;

/* should be less than max_pte_pages - min_pte_pages */
int		more_pte_pages = PMAP_MIN_PTEPAGES;

void
put_free_ptepage(page)
	vm_offset_t page;
{
	/*
	 *	Put the page back on the free list.
	 *	But don't let the free list get too big!
	 */

	simple_lock(&ptepages_lock);

	if (free_ptepages_count < max_pte_pages || page < vm_reserved_pte) {
		* (vm_offset_t *) page = free_ptepages;
		free_ptepages = page;
		free_ptepages_count++;
		page = 0;
	}

	if (ptepages_wanted) {
		ptepages_wanted = FALSE;
		thread_wakeup((int) &ptepages_wanted);
	}

	simple_unlock(&ptepages_lock);

	/*
	 *	Actually release the page if it
	 *	didn't go on the free list.
	 */

	if (page) {
		pa_to_pvh(K0_TO_PHYS(page))->pv_pmap = kernel_pmap;
		pmap_release_page(K0_TO_PHYS(page));
	}
}

/*
 * Allocate more pages to the pool when they are scarce
 * If we allocate too many, put_free_ptepage will just free them.
 */
void
get_more_pte_pages()
{
	vm_offset_t vp, pp;
	int	    i;
	static int ncalls = 0;

	ptepages_taken++;

	DO_COUNT(mpp_count)

	TRACE({dprintf("pmap: get_more_pte_pages %d\n", ++ncalls);})

	if((vp = kmem_alloc(pmap_submap, more_pte_pages * MIPS_PGBYTES)) == 0)
		panic("get_more_pte_pages");

	for (i = 0; i < more_pte_pages; i++) {
		/* we speak k0 here */
		pp = pmap_resident_extract(kernel_pmap, vp);
		pp = PHYS_TO_K0(pp);
		put_free_ptepage(pp);
		vp += MIPS_PGBYTES;
	}

	TRACE({printf("pmap: get_more_pte_pages done.\n");})
}

vm_offset_t
get_free_ptepage()
{
	vm_offset_t page;

	for (;;) {
		simple_lock(&ptepages_lock);
		if (free_ptepages_count > min_pte_pages)
			break;

		if (ptepages_thread == THREAD_NULL) {
			/* nobody else is allocating ptepages */

			ptepages_thread = current_thread();
			simple_unlock(&ptepages_lock);

			get_more_pte_pages();

			simple_lock(&ptepages_lock);
			ptepages_thread = THREAD_NULL;
			simple_unlock(&ptepages_lock);

			/* must check again */
			continue;
		}

		/* somebody is allocating; check for reentrancy/privilege */
		if ((ptepages_thread == current_thread()) ||
		    current_thread()->vm_privilege) {
			if (free_ptepages_count == 0)
				panic("get_free_ptepage");
			break;
		}

		/* block until pages are available */
		ptepages_wanted = TRUE;
		thread_sleep((int) &ptepages_wanted,
			     simple_lock_addr(ptepages_lock),
			     FALSE);
	}

	page = free_ptepages;
	free_ptepages = * (vm_offset_t *) page;
	* (vm_offset_t *) page = 0;
	free_ptepages_count--;
	
	simple_unlock(&ptepages_lock);
	pa_to_pvh(K0_TO_PHYS(page))->pv_mappings = 0;
	return page;
}

/*
 * Allocate a ptepage to a given pmap.
 * This function is called by the tlb_miss() handler when a ptepage
 * is needed to cover a fault.
 */
void 
pmap_pte_fault(map, ppte)
	pmap_t map;
	pte_template  *ppte;
{
	register vm_offset_t  vp, pp;

	DO_COUNT(ptf_count)

	TRACE({printf("pmap_pte_fault x%x x%x\n", map, ppte);})

	if (map == PMAP_NULL) 
		panic("pmap_pte_fault");

	vp = get_free_ptepage();

	lock_pmap(map);

	/*
	 *	get_free_ptepage() can block, during which time some other
	 *	thread might have covered this pte fault.  Check whether
	 *	we still want the page.
	 */
   
	if (ppte->raw & PG_V ){
		unlock_pmap(map);
		put_free_ptepage(vp);
		return;
	}

	TRACE({printf("ptepage at %x\n", vp);})

	pp = K0_TO_PHYS(vp);

	if (map == kernel_pmap) 
		ppte->raw = PHYSTOPTE(pp) | PG_V | PG_G | PG_M |
			((VM_PROT_READ|VM_PROT_WRITE) << PG_PROTOFF);
	else
		ppte->raw = PHYSTOPTE(pp) | PG_V | PG_M |
			((VM_PROT_READ|VM_PROT_WRITE) << PG_PROTOFF);
	
	map->ptepages_count++;

	unlock_pmap(map);

}

/*
 *	Pmap mgmt.
 *
 */
vm_offset_t	*free_pmaps = 0;

/*
 * Operations defined on the pmap free list:
 *	- test if empty
 *	- add to list
 *	- remove from list
 *	- walk through list
 */
#define	pmap_pool_is_empty()	(free_pmaps == 0)

#define	pmap_pool_add(pmap)						\
MACRO_BEGIN								\
	*((vm_offset_t**)(pmap)) = free_pmaps;				\
	free_pmaps = (vm_offset_t*)(pmap);				\
MACRO_END
/*
 * As an optimization, the k2 address of zalloced pmaps is converted to
 * a k0 address whenever the zalloced pmap fits entirely on one page.
 * This should save us a few TLBMISSes !
 */
#define	pmap_pool_remove(_pmap)						\
if (pmap_pool_is_empty()) {						\
	vm_offset_t pmap;						\
	int	    size = sizeof (struct pmap); 			\
	pmap =  zalloc(pmap_zone);					\
	if (pmap == 0) panic("out of pmaps");				\
	if ((pmap + size) < (mips_trunc_page(pmap) + MIPS_PGBYTES)) {	\
	   pmap = pmap_resident_extract(kernel_pmap, pmap); 		\
	   pmap = PHYS_TO_K0(pmap);					\
	}								\
	bzero(pmap, sizeof( struct pmap ));				\
	_pmap = (pmap_t) pmap;						\
	_pmap->ptebase = KPTEADDR - UPTESIZE;				\
} else {								\
	_pmap = (pmap_t) free_pmaps;					\
	free_pmaps = (vm_offset_t*)*free_pmaps;				\
}									

#define pmap_pool_walk(_p)						\
	for (_p = (pmap_t)free_pmaps; _p; _p = (pmap_t)*((vm_offset_t*)_p))

/*
 *	Machine-level page attributes
 *
 *	These are infrequently used features of the Mips MMU,
 *	basically cache control functions.  The cachability
 *	property of mappings must be remembered across paging
 *	operations, so that they can be restored on need.
 *
 *	Obviously these attributes will be used in a sparse
 *	fashion, so we use a simple list of attribute-value
 *	pairs.
 */
typedef struct pmap_attribute {
	struct pmap_attribute         *next;
	vm_offset_t		      start;
	vm_offset_t		      end;
	vm_machine_attribute_t	      attribute;
	vm_machine_attribute_val_t    value;
} *pmap_attribute_t;

zone_t	pmap_attribute_zone;

#define pmap_attribute_alloc()	(pmap_attribute_t)zalloc(pmap_attribute_zone)
#define pmap_attribute_free(a)	zfree(pmap_attribute_zone, (a))

#define pmap_has_attributes(pmap)	(pmap->attributes != 0)

#define pmap_destroy_attributes(pmap)			\
	{ register pmap_attribute_t a1,a2;		\
		a1 = (pmap_attribute_t)pmap->attributes;\
		while (a1) {				\
			a2 = a1->next;			\
			pmap_attribute_free(a1);	\
			a1 = a2;			\
		}					\
		pmap->attributes = 0;			\
	}
#if 1/* debug */
extern void print_attributes();
#endif
/*
 *	Lookup an attribute in a pmap.
 */
pmap_attribute_t
pmap_attribute_lookup(pmap, start, attr)
	pmap_t		pmap;
	vm_offset_t	start;
	vm_machine_attribute_t	attr;
{
	pmap_attribute_t att;

	for (att = (pmap_attribute_t)pmap->attributes;
	     att; att = att->next)
		if (att->start <= start && start < att->end)
			return att;
	return (pmap_attribute_t)0;
}

/*
 *	Update an attribute in a pmap.
 *	The pmap must be locked.
 */
void
pmap_attribute_update(pmap, start, end, attr, val)
	pmap_t				pmap;
	vm_offset_t			start, end;
	vm_machine_attribute_t		attr;
	vm_machine_attribute_val_t	val;
{
	register pmap_attribute_t 	att, prev;
	pmap_attribute_t	  	new;

	/*
	 *	Allocate
	 */
	new = pmap_attribute_alloc();
	new->start = start;
	new->end = end;
	new->attribute = attr;
	new->value = val;
	new->next = 0;

	prev = (pmap_attribute_t)&pmap->attributes;
	if (prev->next == 0){
		prev->next = new;
		return;
	}

	/*
	 *	Insert
	 */
	att = prev->next;
	if (att->start >= end) {
		new->next = att;
		prev->next = new;
		goto prune;
	}

	for (; att; prev = att, att = att->next)
		if (att->start <= start && start < att->end)
			break;

	if (att == 0) {
		prev->next = new;
		goto prune;
	}

	if (att->start == start) {
		if (att->end <= end) {
			/* XXX */
			att->value = val;
			att->end = end;
			pmap_attribute_free(new);
		} else {
			prev->next = new;
			new->next = att;
			att->start = end;
		}
	} else {
		new->next = att->next;
		att->next = new;
		if (att->end <= end) {
			att->end = start;
		} else {
			register pmap_attribute_t t;

			t = pmap_attribute_alloc();
			t->end = att->end;
			att->end = start;
			t->next = new->next;
			new->next = t;
			t->start = end;
			t->attribute = attr;
			t->value = att->value;
		}
	}


	/*
	 *	Prune
	 */
prune:
	for (att = (pmap_attribute_t)pmap->attributes; att; ) {
		if (((new = att->next) == 0) ||
		    (new->start > end))
			break;
		if (att->value == new->value) {
			att->end = new->end;
			att->next = new->next;
			pmap_attribute_free(new);
		} else if ((att->start == new->start) ||
			   (new->end <= att->end)) {
			att->next = new->next;
			pmap_attribute_free(new);
		} else {
			if (new->start < att->end)
				new->start = att->end;
			att = new;
		}
	}

	if (val != -1)
		return;

	/*
	 *	Deletion
	 */
	prev = (pmap_attribute_t)&pmap->attributes;
	att = prev->next;
	while (att) {
		if (att->value == -1) {
			prev->next = att->next;
			pmap_attribute_free(att);
		} else
			prev = att;
		att = prev->next;
	}
}

/*
 *	Initialize the kernel's physical map.
 *
 *	This routine could be called at boot.  It maps a range of
 *	physical addresses into the kernel's virtual address space.
 *
 */
vm_offset_t 
pmap_map(virt, start, end, prot)
	vm_offset_t	virt;		/* Starting virtual address.	*/
	vm_offset_t	start;		/* Starting physical address.	*/
	vm_offset_t	end;		/* Ending physical address.	*/
	int		prot;		/* Machine indep. protection.	*/
{
	pte_template    pte;
	pt_entry_t     *ppte;
	int             npg;
	
	DO_COUNT(bmp_count)
	
	pte.raw = PHYSTOPTE(start) | PG_V | PG_G |
		(mips_protection(kernel_pmap, prot) << PG_PROTOFF);

	if (prot & VM_PROT_WRITE) 	/* cannot fault yet */
		pte.raw |= PG_M;
	
	/* Map all pages, in Mips page size steps */
	for (npg = mips_btop(mips_round_page(end - start)); npg > 0; 
	     npg--, virt += MIPS_PGBYTES, pte.raw += MIPS_PGBYTES) {
		ppte = pmap_l1pte(kernel_pmap, virt);
		if (!ppte->pg_v) pmap_pte_fault(kernel_pmap, ppte);
		ppte = pmap_l2pte(ppte, virt);
		*ppte = pte.pte;
		tlb_map_random(0, virt, pte.raw);	/* eager */
		if (pte.raw & PG_M)
			PMAP_SET_MODIFY(PTETOPHYS((&pte)));
	}
	return virt;
}

/*
 *	Tell the VM system whether the given physical page actually exists.
 *	This is for machines/configurations where there actually are holes
 *	in the address range we said was available (e.g. Suns).
 */
#undef pmap_valid_page
boolean_t 
pmap_valid_page(p)
	vm_offset_t p;
{
	return TRUE;
}

/*
 *	Bootstrap the system enough to start using kernel virtual memory.
 *	Allocate the kernel pmap (system page table).
 *
 *	Parameters:
 *	avail_start	PA of first available physical page
 *	avail_end	PA of last available physical page
 *	virtual_avail	VA of first available page (after kernel bss)
 *	virtual_end	VA of last available page (end of kernel space)
 *
 *	&start_text	start of kernel text
 *	&etext		end of kernel text
 */
void 
pmap_bootstrap(avail_start, avail_end, virtual_avail, virtual_end)
	vm_offset_t	*avail_start;	/* IN/OUT */
	vm_offset_t	*avail_end;	/* IN/OUT */
	vm_offset_t	*virtual_avail;	/* OUT */
	vm_offset_t	*virtual_end;	/* OUT */
{
	pte_template 	pte;
	pt_entry_t	*ppte;
	extern int nsys_space_elems;
	vm_size_t	s;
	long		npages;

	/*
	 *	The kernel's pmap is statically allocated so we don't
	 *	have to use pmap_create, which is unlikely to work
	 *	correctly at this time.
	 */

	kernel_pmap = &kernel_pmap_store;
	simple_lock_init(&kernel_pmap->lock);
	kernel_pmap->ptebase = (vm_offset_t) KPTEADDR;
	/*
	 *	Allocate the distinguished tlbpid value of 0 to
	 *	the kernel pmap. The tlbpid allocator is statically
	 *	initialized and knows about this
	 */
	kernel_pmap->pid = 0;
	/*
	 *	Take a reference on the kernel pmap, even if it
	 *	wont go away.
	 */
	kernel_pmap->ref_count = 1;

	/*
	 *	Initialize our locks
	 */
#if MACH_SLOCKS
	lock_init(&pmap_lock, FALSE);		/* Complex spin lock */
#endif
	simple_lock_init(&ptepages_lock);

	/*
	 *	Allocate the kernel root page table entries from the front
	 *	of available physical memory. To avoid wiring one tlb entry
	 *	just for this table we keep the table in kseg0.
	 */

	root_kptes = (vm_offset_t) PHYS_TO_K0(*avail_start);
	kernel_pmap->l1ptepage = root_kptes;
	bzero(root_kptes, MIPS_PGBYTES);
	*avail_start += MIPS_PGBYTES;

	/*
	 *	Allocate the pv header array, the modify bits and the reference bits
	 *	right out of physical so we can take tb misses(which calls
	 *	pmap_pte_fault) before pmap init is called.
	 */

	npages = mips_btop(*avail_end-*avail_start);
	s = (vm_size_t) (sizeof(struct pv_entry) * npages);
#if	COMPACTED
	s += (npages >> 3) + 1;
#else	/* COMPACTED */
	s +=  2 * npages;
#endif	/* COMPACTED */

	pv_head_table = (pv_entry_t)PHYS_TO_K0(*avail_start);
	pmap_modify_bits = (char *)pv_head_table + (sizeof(struct pv_entry) * npages);
	pmap_reference_bits = pmap_modify_bits + npages;
	bzero(pv_head_table, s);
       	/*
         * Enable bookkeeping of the modify list
         */

	vm_first_phys = *avail_start;
	vm_last_phys = *avail_end;

	*avail_start += mips_round_page(s);
	
	/*
	 * 	Note in the table itself that we have the page, although we
	 *	will not map it virtually.  Note that in this way we
	 *	will be able to use less space for the root_ptes (1k
	 *	instead of a full 4k).  Eventually. Maybe.
	 */

	pte.raw = PHYSTOPTE(K0_TO_PHYS(root_kptes)) | PG_V | PG_M | PG_G |
		(mips_protection(kernel_pmap, (VM_PROT_READ | VM_PROT_WRITE)) << PG_PROTOFF);
	ppte = pmap_root_pte(-1);	/* e.g. the last virtual address */
	*ppte = pte.pte;

	/*
	 *	The kernel runs unmapped and cached (k0seg),
	 *	only dynamic data are mapped in k2seg.
	 *	==> No need to map it.
	 */

	/*
	 *	Reserve an initial amount of pages for the ptes.
	 */

	{	vm_offset_t	va, pa;
		int		num_pte_pages, i;

		num_pte_pages = min_pte_pages + 1;
		pa = *avail_start;
		*avail_start += num_pte_pages * MIPS_PGBYTES;
		vm_reserved_pte = PHYS_TO_K0(*avail_start);
		for (va = PHYS_TO_K0(pa), i = 0; i < num_pte_pages; 
		     i++, va += MIPS_PGBYTES) {
			bzero(va, MIPS_PGBYTES);
			put_free_ptepage(va);
		      }
	}
		
	/*
	 *	Assert the kernel virtual address limits.
	 */

	*virtual_avail = round_page(VM_MIN_KERNEL_ADDRESS);
	*virtual_end   = trunc_page(VM_MAX_KERNEL_ADDRESS);

	if(nsys_space_elems) {
		extern struct sys_space sys_space[];
		/*
		 * reserve kernel virtual space for drivers that need 
		 * to map things before the vm/pmap subsystems are up
		 * but make sure the kernel has at least 100mb
		 */
		int i;

		for(i = 0; i < nsys_space_elems; i++) {
			*sys_space[i].s_va = *virtual_avail;
			*virtual_avail = round_page(*virtual_avail + sys_space[i].s_size);
		}

		if((*virtual_avail >= *virtual_end) ||
		   (*virtual_end - *virtual_avail < 100*1024*1024))
			panic("pmap_bootstrap: no kernel virtual space\n");
	}

	printf("Kernel virtual space from 0x%x to 0x%x.\n",
	       *virtual_avail, *virtual_end);
}

/*
 *	Initialize the pmap module.
 *	Called by vm_init, to initialize any structures that the pmap
 *	system needs to map virtual memory.
 */
void 
pmap_init(phys_start, phys_end)
	vm_offset_t	phys_start, phys_end;
{
	long		npages, vsize;
	vm_offset_t	addr, phys, e;
	vm_size_t	s;
	extern int nproc;


	/*
	 * Notify upper level segment initialization code.
	 */

#ifdef	PMAP_SEGMENTATION
	u_seg_init(VM_MIN_ADDRESS, VM_MAX_ADDRESS, 
			pmap_seg_size, pmap_seg_mask);
#endif	PMAP_SEGMENTATION

	/*
	 *	How many phys pages will we be accounting for
	 */
	npages = atop(phys_end - phys_start);

	/*
	 *	Create our submap to allocate pages from
	 *
	 *	Take enough virtual to cover all phys mem if we
	 *	don't have much of it (pmaxen), but no more than
	 *	50 meg otherwise.  This should be plenty.
	 */
#define _PMAP_MAX_VIRT	0x3000000
	vsize = (npages * PAGE_SIZE);
	if (vsize > _PMAP_MAX_VIRT)
		vsize = _PMAP_MAX_VIRT;
	pmap_submap = kmem_suballoc(kernel_map, &pmap_vlow, &pmap_vhigh, vsize, FALSE);

	/*
	 * Create the segment zone
	 */

	pmap_seg_zone = zinit(sizeof (struct pmap_seg),
		nproc * sizeof (struct pmap_seg), 4096, "pmap segments");

	/*
	 * Create the l1pte zone
	 */

	l1pt_zone = zinit(L1PT_SIZE, nproc * L1PT_SIZE, MIPS_PGBYTES, "l1pts");
		 

	/*
	 *	Create the zones of physical maps
	 *	and physical_to_virtual entries
	 */
	s = (vm_size_t) sizeof(struct pmap);
	pmap_zone = zinit(s, 8196*s, 4096, "pmap");

	s = (vm_size_t) sizeof(struct pv_entry);
	pv_list_zone = zinit(s, 100000*s, 4096, "pv_list");

	/*
	 *	Create the zone for machine attributes
	 */
	s = (vm_size_t) sizeof(struct pmap_attribute);
	pmap_attribute_zone = zinit( s, 2048*s, 4096, "m_attributes");

	pmap_module_is_initialized = TRUE;	/* let's dance! */
}

/*
 *	Create and return a physical map.
 *
 *	If the size specified for the map is zero, the map is an actual 
 *	physical map, and may be referenced by the hardware.  In this
 *	case, space for the page table entries is allocated and cleared.
 *
 *	If the size specified is non-zero, the map will be used in software
 *	only, and is bounded by that size.
 */
pmap_t 
pmap_create(size)
	vm_size_t	size;
{
	register pmap_t		      map;
	register pmap_statistics_t    stats;
	register vm_offset_t l1ptepage, kva, phys;

	/* A software use-only map doesn't even need a map */
	if (size != 0)
		return PMAP_NULL;

	DO_COUNT(pcr_count)

	TRACE({printf("pmap_create x%x\n", size);})

	/*
	 *	Allocate a pmap structure.  Also allocate enough kernel
	 *	virtual addresses for the ptes to cover a USEG segment.
	 *	This assumes the map will not need to cover both user and
	 *	kernel virtual addresses (which are disjoint anyways).
	 */
	pmap_pool_remove(map);

	/* Initialize the various fields */
	map->ref_count = 1;
	simple_lock_init(&map->lock);
	map->pid = -1;

	/* Initialize statistics */
	stats = &map->stats;
	stats->resident_count = 0;
	stats->wired_count = 0;

	TRACE({ printf("pmap_create: created at x%x, ptebase x%x\n",
		(uint_t)map, map->ptebase);})

	ZALLOC(l1pt_zone, l1ptepage, vm_offset_t);
	kva = trunc_page(l1ptepage);
	phys = pmap_extract(kernel_pmap, kva);
	l1ptepage = PHYS_TO_K0(phys + (l1ptepage - kva));
	bzero(l1ptepage, L1PT_SIZE);
	map->l1ptepage = l1ptepage;

	return(map);
}

/*
 *	Retire the given physical map from service.  Should
 *	only be called if the map contains no valid mappings.
 */
void 
pmap_destroy(map)
	pmap_t	map;
{
	register int index;
	register pt_entry_t *pte;
	int c;
	unsigned s;
	register vm_offset_t l1ptepage, phys;
	register pv_entry_t pvhp;

	if (map == PMAP_NULL)
		return;

	TRACE({printf("pmap_destroy x%x\n", (uint_t)map);})

	DO_COUNT(pds_count)

	s = splvm();
	lock_pmap(map);
	c = --map->ref_count;
	unlock_pmap(map);
	splx(s);

	if (c != 0)
		return;

	s = splvm();
	destroy_tlbpid(map->pid, TRUE);
	splx(s);
	clear_coprocessor_vm_maint(map);

	/*
	 *	Release all pte pages, then the pmap itself 
	 *	No need to lock or protect here, no one else
	 *	can possibly need this pmap.
	 */

	/*	Scan the l1pte page and free any l2pte pages.
	 *  	Note that only the first half of the l1pte page 
	 *	(512 entries) has to be scanned since that is all
	 *  	that is actually used.
	 */

	pte = (pt_entry_t *) map->l1ptepage;

	for (index = 0; index < (PTES_PER_PAGE/2); index++, pte++){

		if (pte->pg_v){

#if 0 /* turn on for debug */

			register int l2_index;
			register pt_entry_t *l2pte;

			/*
     	 		 *  Scan l2pte page looking for valid l2ptes.  If any
	 		 *  valid l2ptes are found, panic ! 
	 		 */

			l2pte = (pt_entry_t *)PHYS_TO_K0(PTETOPHYS(pte));

			for (l2_index = 0; l2_index < PTES_PER_PAGE; 
				l2_index++, l2pte++){

				if (l2pte->pg_v || l2pte->pg_refclu){
					printf("map %x pte %x \n", map, l2pte);
					panic("pmap_destroy: valid l2pte !");
				}	
			}
#endif
			put_free_ptepage(PHYS_TO_K0(PTETOPHYS(pte)));	
		}
	}

	map->ptepages_count = 0;
	
	pmap_clean_pcache(map, FALSE);		/* cleanup pcache */

	l1ptepage = K0_TO_PHYS(map->l1ptepage);
	pvhp = pa_to_pvh(l1ptepage);
	phys = pvh_to_pa(pvhp);
	l1ptepage = pvhp->pv_va + (l1ptepage - phys);
	ZFREE(l1pt_zone, l1ptepage);
	map->l1ptepage = (vm_offset_t) 0;
	
	/* Destroy attributes, if any */
	if (pmap_has_attributes(map))
		pmap_destroy_attributes(map);

	/* Finally, put the pmap to rest */
	pmap_pool_add(map);
}

void
pmap_clean_pcache(map, flush)
	pmap_t 		map;
	boolean_t	flush;
{
	register int i;
	register vm_offset_t addr;
	struct pcache *cache = map->pcache.data;

	for (i = 0; i < NPCACHE; i++) {
		if (addr = cache[i].vaddr) {
			if (flush)
				tlb_unmap(map->pid, addr);
			cache[i].vaddr = 0;
		}
	}
	if (flush)
		pmap_tbsync(map, 0, TLBVASIZ);
}
#ifdef	PMAP_PCACHE_DEBUG
void
pmap_pcache_debug()
{
	register int i;
	register vm_offset_t addr;
	struct pcache *cache = active_pmap->pcache.data;

	for (i = 0; i < NPCACHE; i++) {
		if (addr = cache[i].vaddr) {
			tlb_zero(i+2);
			if (tlb_probe(active_pmap->pid, addr))
				panic("pcache is invalid !");
		}
	}
}
#endif	/* PMAP_PCACHE_DEBUG */
/*
 *	Add a reference to the specified pmap.
 */
void 
pmap_reference(map)
	pmap_t	map;
{
	unsigned s;

	TRACE({printf("pmap_reference x%x\n", (uint_t)map);})

	if (map != PMAP_NULL) {
		s = splvm();
		lock_pmap(map);
		map->ref_count++;
		unlock_pmap(map);
		splx(s);
	}
}

/*
 *	Remove a range of mappings.  The given
 *	addresses are the first (inclusive) and last (exclusive)
 *	addresses for the VM pages.
 *
 *	The pmap must be locked.
 */
pv_entry_t 
pmap_remove_range(pmap_t map, 
	vm_offset_t start, 
	vm_offset_t end, 
	vm_offset_t *l2ptelist)
{
	register pt_entry_t	*spte, *cpte, *spte1;
	pt_entry_t		*epte;
	register pv_entry_t	 pv_h, prev, cur, l2pv;
	register pv_entry_t	 free_pv = PV_ENTRY_NULL;
	vm_offset_t		 pa;
	int			 vtm;
	unsigned		 s;
	boolean_t 	pcache_purged = FALSE;

	TRACE({printf("pmap_remove_range x%x x%x x%x\n", (uint_t)map, start, end);})
	if (l2ptelist) *l2ptelist = (vm_offset_t) 0;
   	DO_COUNT(rrg_count)

	vtm = vm_to_mips(1);
	while (start < end) {
		
		coprocessor_invalidate(map, start, end-start);

		/*
		 * Since we handle "sparse address spaces", it might well
		 * be that a ptepage for this VM range has never been
		 * actually allocated.  Avoid doing it now.
		 *
		 * Note that this optimization depends on the fact that
		 * once a pte page is allocated to a pmap, it is never
		 * reclaimed until the pmap is destroyed.
		 */

		spte1 = pmap_l1pte(map, start);		/* get l1pte */

		if (!spte1->pg_v){			/* is it invalid ? */

			/*
			 * If not, bump start to the address
			 * covered by the next pte page.
			 */

			start += MIPS_PGBYTES * PTES_PER_PAGE;
			start &= ~(MIPS_PGBYTES * PTES_PER_PAGE - 1);
			continue;
		}

		spte = pmap_l2pte(spte1, start);		/* get l2pte */
		l2pv = pte_to_pvh(spte1);
		
		/*
		 * Find the last (non-inclusive) pte,
		 * or the last pte in the current pte
		 * page, which ever is less.
		 * Note that all ptes are contiguous.
		 */
		epte = (pt_entry_t *)mips_trunc_page(spte + PTES_PER_PAGE);
		if (epte > spte + mips_btop(end-start))
			epte = spte + mips_btop(end-start);
		
		/*
		 * Remove each mapping from the PV list
		 */
		for (cpte = spte; cpte < epte; 
		     cpte += vtm, start += PAGE_SIZE) {
			/*
			 * Get the physical address, skipping this
			 * page if there is no physical mapping.
			 *
			 * NOTE: this saves us from screwing up
			 * during initialization, too.
			 */
				
			if (!(cpte->pg_v || cpte->pg_refclu))
				continue;
			pa = PTETOPHYS(cpte);
			/* Selectively invalidate the tlb */
                        if (map->pid != -1) 
				pmap_tb(map, start, (vtm * MIPS_PGBYTES), TB_SYNC_ALL);

			map->stats.resident_count--;
		        if (cpte->pg_wired)
			    map->stats.wired_count--;
			
		        if (managed_phys(pa)) {
				
				lock_pvh_pa(s,pa);
				pv_h = pa_to_pvh(pa);
				if (pv_h->pv_pmap == PMAP_NULL)
					panic("pmap_remove_range: null pv_list!");
				
				if (pv_h->pv_va == start && 
					pv_h->pv_pmap == map) {
				/*
				 * Header is the pv_entry.  Copy the next one
				 * to header and free the next one (we can't
				 * free the header)
				 */
					cur = pv_h->pv_next;
					if (cur != PV_ENTRY_NULL) {
						*pv_h = *cur;
				                cur->pv_next = free_pv;
				                free_pv = cur;

					} else 
						pv_h->pv_pmap = PMAP_NULL;
				} else {
					prev = pv_h;
					while ((cur = prev->pv_next) 
					       != PV_ENTRY_NULL) {
						if (cur->pv_va == start && 
						    cur->pv_pmap == map)
							break;
						prev = cur;
					}
					if (cur == PV_ENTRY_NULL) {
						printf("va %x map %x pvh %x\n", start, (uint_t)map, pv_h);
						panic("pmap_remove_range: mapping not in pv_list!");
					}
					prev->pv_next = cur->pv_next;
			                cur->pv_next = free_pv;
			                free_pv = cur;
				}
				unlock_pvh_pa(s,pa);
			}
			/*
			 * 	Done with this page, zero pte(s)
			 */
			{ 
				register int i;

				l2pv->pv_mappings--;

				* (int *)cpte = 0;
				/* Unroll the loop manually */
				for (i = vtm - 1; i; i--) 
					*(int*)(cpte + i) = 0;
			}
		}

		/*
		 * If the l2pte page contains no more mappings
		 * get rid of it.
		 */

		if (l2ptelist && (l2pv->pv_mappings == 0)) {
			pa = PHYS_TO_K0(PTETOPHYS(spte1));
			if (map == kernel_pmap) {
				tlb_unmap(kernel_pmap->pid, 
					mips_ptob((spte1 - 
						(pt_entry_t *) 
						kernel_pmap->l1ptepage)) +
						KPTEADDR);
				*(int*)spte1 = 0;
			}
			else if (pcache_purged == FALSE) {
				*(int*)spte1 = 0;
				pmap_clean_pcache(map, TRUE);
				pcache_purged = TRUE;
			}
			if (*l2ptelist) 
				* (vm_offset_t *) pa = 
					*l2ptelist;
			else {
				* (vm_offset_t *) pa =
					(vm_offset_t) 0;
				*l2ptelist = (vm_offset_t) pa;
			}
		}
	}
        return(free_pv);
}

/*
 *	Remove the given range of addresses from the specified map.  It is 
 *	assumed that start and end are rounded to the VM page size.
 */
void 
pmap_remove(map, start, end)
	pmap_t		map;
	vm_offset_t	start, end;
{
        pv_entry_t      free_pv, cur;
	unsigned	s;
	vm_offset_t	l2ptelist;

	if (map == PMAP_NULL)
		return;

	READ_LOCK(s);
	lock_pmap(map);

	free_pv = pmap_remove_range(map, start, end, &l2ptelist);

	unlock_pmap(map);
	READ_UNLOCK(s);

	/*
	 *      Free any pv entries that pmap_remove_range returned.
	 */

	while (free_pv != PV_ENTRY_NULL) {
		cur = free_pv;
		free_pv = free_pv->pv_next;
		zfree(pv_list_zone, (vm_offset_t) cur);
	}
	if (l2ptelist) pmap_free_l2pte_list(l2ptelist);
}

/*
 *	Routine:	pmap_remove_all
 *	Function:
 *		Removes this physical page from
 *		all physical maps in which it resides.
 */
void 
pmap_remove_all(phys)
	vm_offset_t	phys;
{
	register pv_entry_t	pv_h, l2_pv;
	pv_entry_t		cur, free_pv;
	register pt_entry_t	*pte, *end_pte;
	vm_offset_t		va, pa;
	register pmap_t 	map;
	unsigned		s;
	register pmap_seg_t	segp;
	register vm_offset_t 	l2ptepage, nl2ptepage;

	TRACE({printf("pmap_remove_all x%x\n", phys);})

	DO_COUNT(ral_count)

	if (!managed_phys(phys))
		return;	/* can't handle */

	l2ptepage = (vm_offset_t) 0;
	free_pv = PV_ENTRY_NULL;

	/*
	 *	Lock the pmap system first, since we might be changing
	 *	several pmaps.
	 */

	WRITE_LOCK(s);

	/*
	 *	Walk down PV list, removing all mappings.
	 *	We have to do the same work as in pmap_remove_range
	 *	since that routine locks the pv_head.  We don't have
	 *	to lock the pv_head, since we have the entire pmap system.
	 */

	pv_h = pa_to_pvh(phys);
	while ((map = pv_h->pv_pmap) != PMAP_NULL) {


		if (pv_isseg(pv_h)) {
			va = pv_h->pv_va;
			segp = (pmap_seg_t) map;
			pmap_seg_lock(segp);
			va = pv_h->pv_va & ~pv_segtype;
			pte = pmap_segpte(segp, va);
			for (end_pte = pte + mips_btop(PAGE_SIZE); 
				pte < end_pte; pte++)  * (int *) pte = 0;
			segp->ps_loadedpte--;
			if (segp->ps_rescnt)
				pmap_seg_remove_all(segp, va);
			else if (!segp->ps_loadedpte) {
				nl2ptepage = segp->ps_pagetable;
				pmap_segpt_free(nl2ptepage);
				segp->ps_pagetable = (vm_offset_t) 0;
				if (l2ptepage) {
					* (vm_offset_t *) nl2ptepage = 
						l2ptepage;
					l2ptepage = nl2ptepage;
				}
				else {
					l2ptepage = nl2ptepage;
					* (vm_offset_t *) l2ptepage = 
						(vm_offset_t) 0;
				}
			}
		}
		else {
			segp = PMAP_SEG_NULL;
			va = pv_h->pv_va;

			lock_pmap(map);

			coprocessor_invalidate(map, va, MIPS_PGBYTES);

			pte = pmap_l1pte(map, va);	/* get l1pte */
			l2_pv = pte_to_pvh(pte);
			pte = pmap_l2pte(pte, va);	/* get l2pte */

			map->stats.resident_count--;
			if (pte->pg_wired) 
				panic("pmap_remove_all removing a wired page");  
			pmap_tb(map, va, PAGE_SIZE, TB_SYNC_ALL);
			l2_pv->pv_mappings--;
			for (end_pte = pte + mips_btop(PAGE_SIZE); 
				pte < end_pte; pte++, va += MIPS_PGBYTES) 
					*(int *)pte = 0;
		}

		if ((cur = pv_h->pv_next) != PV_ENTRY_NULL) {
			*pv_h = *cur;
			cur->pv_next = free_pv;
			free_pv = cur;
		} else
			pv_h->pv_pmap = PMAP_NULL;

		
		if (segp) pmap_seg_unlock(segp);
		else unlock_pmap(map);
	}
	WRITE_UNLOCK(s);

	/*
	 *  Really free free pv entries now.
	 */

	while (free_pv != PV_ENTRY_NULL) {
		cur = free_pv;
		free_pv = free_pv->pv_next;
		zfree(pv_list_zone, (vm_offset_t) cur);
	}

	if (l2ptepage) do {
		nl2ptepage = * (vm_offset_t *) l2ptepage; 
		put_free_ptepage(l2ptepage);	
		l2ptepage = nl2ptepage;
	} while  (l2ptepage != (vm_offset_t) 0);

}

/*
 *	Routine:	pmap_copy_on_write
 *	Function:
 *		Remove write privileges from all
 *		physical maps for this physical page.
 */
void 
pmap_copy_on_write(phys)
	vm_offset_t	phys;
{
	register pv_entry_t	 pv_e;
	register pmap_t		 map;
	register pt_entry_t	*pte, *end_pte;
	vm_offset_t		 va;
	unsigned 		 s;

	TRACE({printf("pmap_copy_on_write x%x\n", phys);})

	DO_COUNT(cow_count)

	if (!managed_phys(phys))
		return;	/* can't handle */

	/*
	 *	Lock the entire pmap system, since we might change several maps.
	 */
	WRITE_LOCK(s);

	/*
	 *	Run down the list of mappings to this physical page,
	 *	disabling write privileges on each one.
	 */
	pv_e = pa_to_pvh(phys);

	if (pv_e->pv_pmap != PMAP_NULL)
	do {

		/*
		 * Ignore pv backed by a segment.
		 * Segments are mapped VM_PROT_EXECUTE 
		 * or (VM_PROT_EXECUTE|VM_PROT_READ).
		 */

		if (pv_isseg(pv_e)) continue;

		/*
		 *	Lock this pmap.
		 */
		map = pv_e->pv_pmap;		
		lock_pmap(map);

		/*
		 *	If there is a mapping, change the protections and
		 *  	modify the tlb entry accordingly.
		 */
		va = pv_e->pv_va;

		pte = pmap_l1pte(map, va);	/* get l1pte */

		pte = pmap_l2pte(pte, va);	/* get l2pte */

		for (end_pte = pte + mips_btop(PAGE_SIZE);
		     pte < end_pte; pte++, va += MIPS_PGBYTES) {
			register unsigned e = *(unsigned*)pte;
                        if (e & (PG_V|PG_REFCLU|VM_PROT_WRITE)) {
				tlb_modify(map->pid, va, 0);
				*(unsigned*)pte = e & ~(PG_M|VM_PROT_WRITE);
			}
		}
		/*
		 *	Done with this one, so drop the lock, and get next pv entry.
		 */
		unlock_pmap(map);

	} while ((pv_e = pv_e->pv_next) != PV_ENTRY_NULL);

	WRITE_UNLOCK(s);
}

/*
 *	Set the physical protection on the
 *	specified range of this map as requested.
 */
void 
pmap_protect(map, start, end, prot)
	register pmap_t map;
	register vm_offset_t start;
	vm_offset_t	end;
	vm_prot_t	prot;
{
	register pt_entry_t *pte, *end_pte;
#ifdef	mips
	register boolean_t change_mappings = TRUE;
#endif	/* mips */
        vm_offset_t         pa;
	unsigned	    s;

	TRACE({printf("pmap_protect x%x x%x x%x x%x\n",
		      (uint_t)map, start, end, prot);})

	if (map == PMAP_NULL)
		return;

	/*
	 *	Protection combinations without read are no access
	 *	pmap_protect may not increase protection, so RW is a no-op
	 *	Must flush cache if execute permission was specified.
	 */
	if ((prot & VM_PROT_READ) == 0) {
		pmap_remove(map, start, end);
		return;
	}

	if (prot & VM_PROT_WRITE) {
#ifdef	mips
		if (prot & VM_PROT_EXECUTE) { 
			change_mappings = FALSE;
		}
		else
#endif	/* mips */
			return;
	}
        DO_COUNT(ppt_count)
	s = splvm();
	lock_pmap(map);

	while (start < end) {

		/*
		 *	Since we handle "sparse address spaces", it might well
		 *	be that a ptepage for this VM range has never been
		 *	actually allocated.  Avoid doing it now.
		 *
		 *	Note that this optimization depends on the fact that
		 *	once a pte page is allocated to a pmap, it is never
		 *	reclaimed until the pmap is destroyed.
		 */
		
		pte = pmap_l1pte(map, start);		/* get l1pte */

		if (!pte->pg_v){			/* is it invalid ? */
			/*
			 * No, bump start to the address covered
			 * by the next pte page.
			 */
			start += MIPS_PGBYTES * PTES_PER_PAGE;
			start &= ~(MIPS_PGBYTES * PTES_PER_PAGE - 1);
			continue;
		}

		pte = pmap_l2pte(pte, start);		/* get l2pte */

		/*
		 * Only change protection if there is a mapping.
		 * We are removing write protection.  If the entry
		 * was modifiable, we must modify the tlb.
		 */
                if (*(unsigned*)pte & (PG_V | PG_REFCLU)) {
                    pa = PTETOPHYS(pte);
#ifdef	mips
			if (change_mappings) {
#endif	/* mips */
				for (end_pte = pte + mips_btop(PAGE_SIZE);
				     pte < end_pte;
				     pte++, start += MIPS_PGBYTES) {
					vm_prot_t new_prot = pte->pg_prot&prot;
				
					if (*(unsigned*)pte & PG_M) {
						pte->pg_m = 0;
						tlb_modify(map->pid, start, 0);
					}
					pte->pg_prot = mips_protection(map, new_prot);
				}
#ifdef	mips
			}
			/*
			 * Now if this is executable code
			 * there might be inconsistencies among the two
			 * caches, so we flush the I-cache just in case.
			 *
			 * WARNING: This may be needed on boxes that don't
			 *           need the cache flush in pmap_enter.
			 */
		    if (prot & VM_PROT_EXECUTE) {
			register vm_offset_t addr = PHYS_TO_K0(pa);
 		        register int i = vm_to_mips(1);
			DO_COUNT(ifl_count)
			for (; i > 0; i--, addr += MIPS_PGBYTES)
				page_iflush(addr);
		    }
#endif	/* mips */
		}
		start += PAGE_SIZE;
	}
	unlock_pmap(map);
	splx(s);
}

/*
 *	Insert the given physical page (p) at
 *	the specified virtual address (v) in the
 *	target physical map with the protection requested.
 *
 *	If specified, the page will be wired down, meaning
 *	that the related pte can not be reclaimed.
 *
 *	NB:  This is the only routine which MAY NOT lazy-evaluate
 *	or lose information.  That is, this routine must actually
 *	insert this page into the given map NOW.
 */
void 
pmap_enter(map, v, p, prot, wired, access)
	register pmap_t map;
	vm_offset_t	v;
	vm_offset_t	p;
	vm_prot_t	prot;
	boolean_t	wired;
	vm_prot_t	access;
{
	register pt_entry_t	*pte, *end_pte;
	register pv_entry_t	pv_h, l2_pv;
	register pv_entry_t	pv_e, free_pv = PV_ENTRY_NULL;
	vm_offset_t		ptaddr;
	unsigned		s,s1;
	pte_template		tmpl;
	boolean_t 		flush_needed = FALSE;

	TRACE({printf("pmap_enter x%x x%x x%x x%x x%x ", 
		      (uint_t)map, v, p, prot, wired);})

	if (map == PMAP_NULL)
		return;

        DO_COUNT(ent_count)

	if (!pmap_module_is_initialized) {	
		/* pmap system up ? */
		pmap_map(v, p, p + PAGE_SIZE, prot);
		return;
	}

	TRACE({printf("(%x)\n", ptaddr);})

	pv_e = PV_ENTRY_NULL;

Retry:

	READ_LOCK(s);
	lock_pmap(map);

	pte = pmap_l1pte(map,v);		/* get l1pte */

	if  (!pte->pg_v){			/* is it valid ? */
		unlock_pmap(map);		/* if not, drop locks to */
		READ_UNLOCK(s);			/* prevent deadlock. */
		pmap_pte_fault(map,pte);	/* get a pte page */
		goto Retry;			/* and start all over */
	}

	l2_pv = pte_to_pvh(pte);
	pte = pmap_l2pte(pte,v);		/* get l2pte */	
	ptaddr = PTETOPHYS(pte);
	
	/*
	 *	Enter the mapping in the PV list for this physical page.
	 *	If there is already a mapping, remove the old one first.
	 *	(If it's the same physical page, it's already in the PV list.)
	 */

	if (ptaddr != p) {
		if (*(int*)pte != 0) {
			/*
			 *      Don't free the pte page if removing last
			 *      mapping - we will immediately replace it.
			 */
		    if (pv_e == PV_ENTRY_NULL)
			pv_e = pmap_remove_range(map, v, (v + PAGE_SIZE),
				(vm_offset_t *) 0);
		    else
			free_pv = pmap_remove_range(map, v, (v + PAGE_SIZE),
				(vm_offset_t *) 0);
		}

		/*
		 *	Only keep the pv_list info for the pages we handle,
		 *	e.g. do not mess around with I/O or other weirdos.
		 */
		if (managed_phys(p)) {

			lock_pvh_pa(s1,p);
			pv_h = pa_to_pvh(p);

			if (pv_h->pv_pmap == PMAP_NULL) {
				/*
				 *	No mappings yet
				 */
				flush_needed = TRUE;
				pv_h->pv_va = v;
				pv_h->pv_pmap = map;
				pv_h->pv_next = PV_ENTRY_NULL;
			} else {
				/*
				 *	Add new pv_entry after header.
				 */
				if (pv_e == PV_ENTRY_NULL) {
			 		unlock_pvh_pa(s1,p);
					unlock_pmap(map);
                                        READ_UNLOCK(s);

					pv_e = (pv_entry_t)zalloc(pv_list_zone);
	     				goto Retry;
				}
				pv_e->pv_va = v;
				pv_e->pv_pmap = map;
				pv_e->pv_next = pv_h->pv_next;
				pv_h->pv_next = pv_e;
				/*
				 *	Remember that we used the pv_list entry.
				 */
				pv_e = PV_ENTRY_NULL;
			}
			unlock_pvh_pa(s1,p);
		}
		map->stats.resident_count++;
	}

	/*
	 *	Enter the mapping in each Mips pte.
	 */
        if (pte->pg_v) {

		/*
		 *	Replacing a valid mapping - only worry about
		 *	write permissions, since we could only be changing
		 *	protection or wired bits here, not the phys page.
		 *
		 */

		if (pte->pg_prot != mips_protection(map, prot)) {
			/*
			 *	Changing protection and possibly wiring.
			 */
			for ( end_pte = pte + mips_btop(PAGE_SIZE);
			      pte < end_pte; pte++, v += MIPS_PGBYTES) {
				if ((prot & VM_PROT_WRITE) == 0) {
					tlb_modify(map->pid, v, 0);
					pte->pg_m = 0;
				}
				pte->pg_prot = mips_protection(map, prot);
				pte->pg_wired = wired;			
			      }
		} else if(pte->pg_wired != wired) {
			/*
			 * Changing only the wired bit.
			 */
			for ( end_pte = pte + mips_btop(PAGE_SIZE);
			      pte < end_pte; pte++) {
				pte->pg_wired = wired;
			}
		      }
	 }
            else {
		/*
		 *	Not a valid mapping -
		 *	Set up a template to use in making the pte's.
		 */

		tmpl.raw = PHYSTOPTE(p)| PG_V | (wired << PG_WIREDOFF) |
				(mips_protection(map, prot) << PG_PROTOFF);

		if (IS_KSEG2(v)) {
			/*
			 * Kernel virtual space is global
			 */
			tmpl.raw |= PG_G;
		} else {
			/*
			 * We are basically interested in I/O space here.
			 * Apparently, even a bitmap screen has no reasons
			 * to be cached [impeds performance of other programs
			 * by uselessly filling the cache], so we make
			 * all I/O space memory non-cachable.
			 */
			if (p > mem_size)
				tmpl.raw |= PG_N;
		}
		
		if (pmap_has_attributes(map)) {
			pmap_attribute_t	a;
			a = pmap_attribute_lookup(map, v, MATTR_CACHE);
			if (a && a->value == MATTR_VAL_OFF)
				tmpl.raw |= PG_N;
		}

		/*
		 *	Do the access checking and tlbmod optimizations
		 */

		if ((access & prot) != access)
			panic("pmap_enter: invalid access");
		if (managed_phys(p) & (access & VM_PROT_WRITE)) {
			tmpl.raw |= PG_M;
			PMAP_SET_MODIFY(p);	
		}
		
		/*
		 *	Now use the template to set up all the pte's.
		 */
		l2_pv->pv_mappings++;
		for (end_pte = pte + mips_btop(PAGE_SIZE); pte < end_pte; 
		     pte++, tmpl.raw += MIPS_PGBYTES, v += MIPS_PGBYTES) {
			*pte = tmpl.pte;
			if (map->pid >= 0) 
				tlb_map_random(map->pid, v, tmpl.raw);

		}
		/*
		 *	Now if this is executable code we are mapping
		 *	there might be inconsistencies among the two
		 *	caches, so we flush the I-cache just in case.
		 *	This only happens on boxes that do not do DMA,
		 *	otherwise bufflush() would do it.
		 */

		if (flush_needed) {
		    	DO_COUNT(ifn_count)
			if (prot & VM_PROT_EXECUTE) {
				register vm_offset_t addr = PHYS_TO_K0(p);
				register int i = vm_to_mips(1);
				DO_COUNT(ifl_count)
				for (; i > 0; i--, addr += MIPS_PGBYTES)
					page_iflush(addr);
			}
		}
	}

	unlock_pmap(map);
	READ_UNLOCK(s);

	if (pv_e != PV_ENTRY_NULL)
		zfree(pv_list_zone, (vm_offset_t) pv_e);

	if (free_pv != PV_ENTRY_NULL)
		zfree(pv_list_zone, (vm_offset_t) free_pv);

}

/*
 *	Routine:	pmap_change_wiring
 *	Function:	Change the wiring attribute for a map/virtual-address
 *			pair.
 *	In/out conditions:
 *			The mapping must already exist in the pmap.
 */
void 
pmap_change_wiring(map, v, wired)
	register pmap_t	map;
	vm_offset_t	v;
	boolean_t	wired;
{
	register pt_entry_t	*pte, *end_pte;
	unsigned		s;

	READ_LOCK(s);
	lock_pmap(map);

	pte = pmap_l1pte(map, v);		/* get l1pte */

	pte = pmap_l2pte(pte, v);		/* get l2pte */

	if (!pte->pg_v  && !pte->pg_refclu)
		panic("pmap_change_wiring: virtual address is not mapped");

	for (end_pte = pte + mips_btop(PAGE_SIZE); pte < end_pte; pte++) {
		pte->pg_v = 1;
		pte->pg_refclu = 0;
		pte->pg_wired = wired;
	}

	unlock_pmap(map);
	READ_UNLOCK(s);
}


/*
 *	Routine:	pmap_extract
 *	Function:
 *		Extract the physical page address associated with the given
 *		map/virtual_address pair.  The address includes the offset
 *		within a page.
 */
vm_offset_t 
pmap_extract(map, va)
	register pmap_t		map;
	vm_offset_t		va;
{
	pt_entry_t	 	*pte;
	unsigned long   	pa;
	unsigned		s;

	TRACE({printf("pmap_extract x%x x%x ..", (uint_t)map, va);})

        DO_COUNT(ext_count)

	s = splvm();
	lock_pmap(map);

	pte = pmap_l1pte(map, va);			/* get l1pte */

	if (!pte->pg_v)					/* is it invalid ? */
		pa = 0;					/* yes, set pa = 0 */
	else {
		pte = pmap_l2pte(pte, va);		/* get l2pte */
    		if ((!pte->pg_v) && (!pte->pg_refclu))	/* is it invalid ? */
			pa = 0;				/* yes, set pa = 0 */
		else
			pa = PTETOPHYS(pte) + (va & VA_OFFMASK);
	}
	
	unlock_pmap(map);
	splx(s);

	TRACE({printf(" %x.\n", (uint_t)pa);})
	return (pa);
}

/*
 *	Routine:	pmap_resident_extract
 *	Function:
 *		Extract the physical page address associated with the given
 *		virtual address in the kernel pmap.  The address includes
 *		the offset within a page.  This routine does not lock the
 *		pmap; it is intended to be called only by device drivers
 *		or copyin/copyout routines that know the buffer whose address
 *		is being translated is memory-resident.
 */
vm_offset_t 
pmap_resident_extract(map, va)
	pmap_t		map;
	vm_offset_t	va;
{
	pt_entry_t           *pte;
	register vm_offset_t pa;

	TRACE({printf("pmap_resident_extract x%x x%x\n", (uint_t)map, va);})

        DO_COUNT(ext_count)

	pte = pmap_l1pte(map, va);			/* get l1pte */

	if (!pte->pg_v)					/* is it invalid ? */
		pa = 0;					/* yes, set pa = 0 */
	else {
		pte = pmap_l2pte(pte, va);		/* get l2pte */
    		if ((!pte->pg_v) && (!pte->pg_refclu))	/* is it invalid ? */
			pa = 0;				/* yes, set pa = 0 */
		else
			pa = (PTETOPHYS(pte) + (va & VA_OFFMASK));
	}
	
	return pa;
}

/*
 *	Routine:	pmap_access
 *	Function:
 *		Returns whether there is a valid mapping for the
 *		given virtual address stored in the given physical map.
 */
boolean_t 
pmap_access(map, va)
	register pmap_t		map;
	vm_offset_t		va;
{
	register pt_entry_t	*pte;

	TRACE({printf("pmap_access x%x x%x\n", (uint_t)map, va);})

        DO_COUNT(acc_count)

	pte = pmap_l1pte(map, va);		/* get l1pte */

	if (!pte->pg_v)				/* is it invalid ? */
		return 0;			/* yes, return false */
	else {
		pte = pmap_l2pte(pte, va);	/* get l2pte */
		return ((* (int *) pte & (PG_V|PG_REFCLU)) ? TRUE : FALSE);
	}
}

/*
 *	Routine:	pmap_copy
 *	Function:
 *		Copy the range specified by src_addr/len
 *		from the source map to the range dst_addr/len
 *		in the destination map.
 *
 *	This routine is only advisory and need not do anything.
 */
#if	0
/* See pmap.h */
void 
pmap_copy(dst_pmap, src_pmap, dst_addr, len, src_addr)
	pmap_t		dst_pmap;
	pmap_t		src_pmap;
	vm_offset_t	dst_addr;
	vm_size_t	len;
	vm_offset_t	src_addr;
{
}
#endif	0

/*
 *	Require that all active physical maps contain no
 *	incorrect entries NOW.  [This update includes
 *	forcing updates of any address map caching.]
 *
 *	Generally used to insure that a thread about
 *	to run will see a semantically correct world.
 */
#if	0
/* See pmap.h */
void 
pmap_update()
{
	/*
	 *	Everything should be just fine
	 */
}
#endif	/* 0 */

/*
 *	Routine:	pmap_release_page
 *	Function:
 *		Deallocate a pte page that is no longer wanted.
 *		The caller supplies the physical address, and
 *		we convert this to the virtual address in pmap_submap.
 */

void 
pmap_release_page(pa)
	vm_offset_t pa;
{
	register pv_entry_t pv;

	ptepages_freed++;

	/*
	 *	Look for the mapping in the kernel pmap
	 *	in the right virtual range.  There should
	 *	only be one such.
	 */
	for(pv = pa_to_pvh(pa); ; pv = pv->pv_next) {

		if(pv->pv_pmap == PMAP_NULL)
			panic("pmap_release_page");

		if((pv->pv_pmap == kernel_pmap) && 
		   (pv->pv_va >= pmap_vlow) && (pv->pv_va < pmap_vhigh))
			break;
	}
	kmem_free(pmap_submap, pv->pv_va, MIPS_PGBYTES);
}

/*
 *	Routine:	pmap_collect
 *	Function:
 *		Garbage collects the physical map system for
 *		pages which are no longer used.
 *		Success need not be guaranteed -- that is, there
 *		may well be pages which are not referenced, but
 *		others may be collected.
 *	Usage:
 *		Called by the pageout daemon when pages are scarce.
 *
 */

void 
pmap_collect(map)
	register pmap_t		map;
{
	register int		l1_index, l2_index;
	register pt_entry_t	*l1pte, *l2pte;
	int			vtm;
	vm_offset_t		mapped, emapped, l2ptepage;
	pv_entry_t		free_pv = PV_ENTRY_NULL;
	pv_entry_t		cur;
	unsigned		s, spl;
	register pmap_seg_t	segp, seglp, segnp;

	if (map == PMAP_NULL)
		return;

	if (map == kernel_pmap)
		panic("pmap_collect on kernel_pmap");

	/*
	 *	Since we fragmented pages badly, can't do anything
	 *	if the pagesize does not match the natural pagesize.
	 */

	if (PAGE_SIZE != MIPS_PGBYTES)
		return;

	DO_COUNT(pgc_count)

	spl = splvm();
	READ_LOCK(s);
	lock_pmap(map);


	vtm = vm_to_mips(1);

   	/*	Scan the l1pte page and free any l2pte pages that 
    	 *  	don't contain any wired ptes. Note that only the 
    	 *   	first half of the l1pte page (512 entries) has 
    	 *	to be scanned since that is all that is actually 
    	 *  	used.
    	 */

	l1pte = (pt_entry_t *) map->l1ptepage;

	for (l1_index = 0; l1_index < (PTES_PER_PAGE/2); 
			l1_index++, l1pte++){

		if (!(* (int *)l1pte & PG_V))	/* is l1pte invalid ? */
			continue;		/* yes, go to next l1pte */
	

       		/* Locate the VA range mapped by the ptes
	   	 * on this page, and remove their mappings.
		 */

		mapped = (vm_offset_t)(l1_index << 22);
		emapped = mapped + (1 << 22);		

		l2ptepage = (vm_offset_t) 0;

		if (* (int *) l1pte & PG_SEGMENT) {
			segp = pmap_ptptetoseg(l1pte);
			pmap_segres_lock(segp);
			segp->ps_rescnt--;
			pmap_segres_unlock(segp);
			* (int *) l1pte ^= PG_V;
			if (map->pid >= 0) 
				tlb_flush_range(map->pid, mips_btop(mapped),
					mips_btop(emapped) - 1);
			free_pv = PV_ENTRY_NULL;
			if (pmap_seg_lock_try(segp)) {
				if (!segp->ps_rescnt && segp->ps_pagetable) {
					free_pv = pmap_seg_free_pagetable(segp, 
						&l2ptepage);
					pmap_segpt_free(l2ptepage);
					segp->ps_pagetable = (vm_offset_t) 0;
					* (vm_offset_t *) l2ptepage = 
						(vm_offset_t) 0;
				}
				pmap_seg_unlock(segp);
			}
		}
		else {

			/*
     		 	 *  Scan l2pte page looking for wired l2ptes.  If any
		 	 *  wired l2ptes are found, abort scan an go to next
       		  	 *  l1pte.
		 	 */

			l2pte = (pt_entry_t *)PHYS_TO_K0(PTETOPHYS(l1pte));

			for (l2_index = 0; l2_index < PTES_PER_PAGE; 
					l2_index++, l2pte++){

				if (l2pte->pg_wired)	break;	
			}

			if (l2_index != PTES_PER_PAGE) /* was scan aborted ? */
				continue;	/* yes, go to next l1pte */

			free_pv = pmap_remove_range(map, mapped, emapped,
					&l2ptepage);
			map->ptepages_count--;
			*(int *)l1pte = 0;		/* clear l1pte */
		}


		unlock_pmap(map);
		READ_UNLOCK(s);

		if (l2ptepage) pmap_free_l2pte_list(l2ptepage);

		while (free_pv != PV_ENTRY_NULL) {
			cur = free_pv;
			free_pv = free_pv->pv_next;
			zfree(pv_list_zone, (vm_offset_t)cur);
		}

		READ_LOCK(s);
		lock_pmap(map);

	}

	unlock_pmap(map);
	READ_UNLOCK(s);
	splx(spl);
	return;
}

/*
 *	Routine:	pmap_activate
 *	Function:
 *		Binds the given physical map to the given processor.
 */
void 
pmap_activate(map, th, cpu)
	pmap_t		map;
	thread_t	th;
	int		cpu;
{
	TRACE({printf("pmap_activate x%x x%x\n", (uint_t)map, (uint_t)th);})
	PMAP_ACTIVATE(map, th, cpu)
}

/*
 *	Routine:	pmap_deactivate
 *	Function:
 *		Indicates that the given physical map is no longer
 *		in use on the specified processor.
 */
void 
pmap_deactivate(map, th, cpu)
	pmap_t		map;
	thread_t	th;
	int		cpu;
{
	TRACE({printf("pmap_deactivate x%x x%x\n", map, th);})
	PMAP_DEACTIVATE(map, th, cpu);
}

/*
 *	Routine:	pmap_kernel
 *	Function:
 *		Returns the physical map handle for the kernel.
 */
#if	0
/* See pmap.h */
pmap_t 
pmap_kernel()
{
	return (kernel_pmap);
}
#endif	/* 0 */

/*
 *	pmap_zero_page zeros the specified (machine independent) page.
 *	pmap_copy_page copies the specified (machine independent) pages.
 *
 */
void
pmap_zero_page(phys)
	register vm_offset_t	phys;
{
	/*
	 *	Use cache.
	 */

	TRACE({printf("pmap_zero_page x%x\n", phys);})

        DO_COUNT(zer_count)

	PMAP_SET_MODIFY(phys);

	bzero(PHYS_TO_K0(phys), PAGE_SIZE);				

}

void
pmap_copy_page(src, dst)
	register int	*src, *dst;
{

        DO_COUNT(cop_count)

	PMAP_SET_MODIFY((unsigned)dst);

	bcopy(PHYS_TO_K0(src), PHYS_TO_K0(dst), PAGE_SIZE);

}

/*
 *	pmap_page_protect:
 *
 *	Lower the permission for all mappings to a given page.
 */
void
pmap_page_protect(phys, prot)
	vm_offset_t	phys;
	vm_prot_t	prot;
{
	TRACE({printf("pmap_page_protect x%x x%x\n", phys, (uint_t)prot);})

	DO_COUNT(prt_count)

	switch (prot) {
		case VM_PROT_READ:
		case VM_PROT_READ|VM_PROT_EXECUTE:
			pmap_copy_on_write(phys);
			break;
		case VM_PROT_ALL:
			break;
		default:
			pmap_remove_all(phys);
			break;
	}
}

/*
 *	Routine:	pmap_pageable
 *	Function:
 *		Make the specified pages (by pmap, offset)
 *		pageable (or not) as requested.
 *
 *		A page which is not pageable may not take
 *		a fault; therefore, its page table entry
 *		must remain valid for the duration.
 *
 *		This routine is merely advisory; pmap_enter
 *		will specify that these pages are to be wired
 *		down (or not) as appropriate.
 */

pmap_pageable(map, start, end, pageable)
	register pmap_t	map;
	register vm_offset_t start;
	register vm_offset_t end;
	boolean_t	pageable;
{
}

/*
 *	Set the reference bits on the specified physical page.
 */
void
pmap_set_reference(phys)
	vm_offset_t	phys;
{
	if (managed_phys(phys)) 
		PMAP_SET_REFERENCE(phys)
}

/*
 *	Set the modify bits on the specified physical page.
 */
void
pmap_set_modify(phys)
	vm_offset_t	phys;
{
        DO_COUNT(smd_count)

	if (managed_phys(phys)) 
		PMAP_SET_MODIFY(phys)
}

/*
 *	Clear the modify bits on the specified physical page.
 */
void 
pmap_clear_modify(phys)
	vm_offset_t	phys;
{
	TRACE({printf("pmap_clear_modify x%x\n", phys);})

        DO_COUNT(clr_count)

	if (managed_phys(phys))
		clear_modify_bit(phys);
}

/*
 *	pmap_is_modified:
 *
 *	Return whether or not the specified physical page is modified
 *	by any physical maps.
 *
 */
boolean_t 
pmap_is_modified(phys)
	vm_offset_t	phys;
{

	TRACE({printf("pmap_is_modified x%x\n", phys);})


	DO_COUNT(imd_count)

	if (managed_phys(phys)) {
		return_modify_bit(phys);
	} else
		return FALSE;
}
/*
 *	pmap_clear_reference:
 *
 *	Clear the reference bit on the specified physical page.
 *
 *	Since the Mips mmu does not have reference bits, we have to do it
 *	in software.  Place the reference bit code in TLBMISS to keep
 *      system overhead as low as possible. The REF_BITS option controls 
 *      the simulation of reference bits.
 */
void pmap_clear_reference(phys)
	vm_offset_t	phys;
{
#if	REF_BITS
	int i, s;
	register pv_entry_t 	cur;
	register pmap_t		map;
	register vm_offset_t	va;
	int			vtm;
	register pmap_seg_t	segp;
	register pt_entry_t	*l2pte;

	if (!managed_phys(phys)) return;
	vtm = vm_to_mips(1);
	WRITE_LOCK(s);
	cur = pa_to_pvh(phys);
	while (cur != PV_ENTRY_NULL) {
		if (map = cur->pv_pmap) {
			if (pv_isseg(cur)) {
				segp = (pmap_seg_t) map;
				pmap_seg_lock(segp);
				va = cur->pv_va & ~pv_segtype;
				l2pte = pmap_segpte(segp, va);
				for (i = vtm; i; i--, l2pte++) 
				((pte_template *) l2pte)->raw =
				(((pte_template *) l2pte)->raw &
					~PG_V) | PG_REFCLU;
				pmap_seg_remove_all(segp, va);
				pmap_seg_unlock(segp);
			}
			else if (map->pid >= 0) {
				va = cur->pv_va;
				set_refclu_bit(map, va);
				tlb_unmap(map->pid, va);
				for (i = vtm - 1; i; i--) {
					va += MIPS_PGBYTES;
					set_refclu_bit(map, va);
					tlb_unmap(map->pid, va);
				}
			}
       			clear_reference_bit(phys);
		}
		cur = cur->pv_next;
	}
	WRITE_UNLOCK(s);
#else	/* REF_BITS */
	pmap_remove_all(phys);
#endif	/* REF_BITS */
	DO_COUNT(crr_count)
}

/*
 *	pmap_is_referenced:
 *
 *	Return whether or not the specified physical page is referenced
 *	by any physical maps.
 *
 *	See comment above.
 */
boolean_t pmap_is_referenced(phys)
	vm_offset_t	phys;
{
	DO_COUNT(irr_count)
#if	REF_BITS
	if (managed_phys(phys))
		return_reference_bit(phys);
#endif	/* REF_BITS */
	return FALSE;
}

/*
 *	copy_from_phys:
 *
 *	Copy from a physically addressed page to a virtually
 *	addressed one.
 */
void 
copy_from_phys(src, dst, cnt)
	vm_offset_t	src, dst;
	unsigned int	cnt;
{
	TRACE({printf("pmap:copy_from_phys x%x x%x x%x\n", src, dst, cnt);})
	bcopy(PHYS_TO_K0(src), dst, cnt);
}

/*
 *	copy_to_phys:
 *
 *	Copy from a virtually addressed page to a physically
 *	addressed one.
 */
void 
copy_to_phys(src, dst, cnt)
	vm_offset_t	src, dst;
	unsigned int	cnt;
{
	TRACE({printf("pmap:copy_to_phys x%x x%x x%x\n", src, dst, cnt);})

        DO_COUNT(tph_count)

	bcopy(src, PHYS_TO_K0(dst), cnt);
}

/*
 *	verify_free:
 *
 *	Check whether the given page is really not mapped in any pmap
 */
boolean_t 
pmap_verify_free( phys )
	vm_offset_t phys;
{
	pv_entry_t	pv;

	if (!pmap_module_is_initialized)
		return(TRUE);

	if (!managed_phys(phys))
		return(FALSE);

	pv = pa_to_pvh(phys);
	return (pv->pv_pmap == PMAP_NULL);

}

/*
 *	phys_to_k0seg:
 *
 *	Translate from physical address to cached kernel address
 *	[For places where you cannot import the macro]
 */
vm_offset_t 
phys_to_k0seg(phys)
	vm_offset_t phys;
{
	return PHYS_TO_K0(phys);
}

/*
 *	pmap_attributes:
 *
 *	Set/Get special memory attributes
 *
 */
kern_return_t 
pmap_attribute(pmap, address, size, attribute, value)
	pmap_t		pmap;
	vm_offset_t	address;
	vm_size_t	size;
	vm_machine_attribute_t	attribute;
	vm_machine_attribute_val_t* value;		/* IN/OUT */
{
#ifdef notyet		/* when we do we must comply with pmap_remove_range */
	register vm_offset_t 	v;
	unsigned		s;
	kern_return_t		ret;
	pmap_attribute_t	a;

	if (attribute != MATTR_CACHE)
		return KERN_INVALID_ARGUMENT;

	if (pmap == PMAP_NULL)
		return KERN_SUCCESS;

	v = trunc_page(address);
	ret = KERN_SUCCESS;

	s = splvm();
	lock_pmap(pmap);

	switch (*value) {

	case MATTR_VAL_OFF:		/* (generic) turn attribute off */

		pmap_attribute_update(pmap, v, round_page(address + size), MATTR_CACHE, MATTR_VAL_OFF);
		pmap_remove_range(pmap, v, round_page(address + size),
			(vm_offset_t *) 0);
		/*print_attributes(pmap,"mattr_val_off->\n");*/
		break;

	case MATTR_VAL_ON:		/* (generic) turn attribute on */

		/* cache is enabled by default */
		if (pmap_has_attributes(pmap)) {
			pmap_attribute_update(pmap, v, round_page(address + size), MATTR_CACHE, MATTR_VAL_ON);
			pmap_remove_range(pmap, v, round_page(address + size),
				(vm_offset_t *) 0);
		}
		/*print_attributes(pmap,"mattr_val_on->\n");*/
		break;

	case MATTR_VAL_GET:		/* (generic) return current value */

		a = pmap_attribute_lookup(pmap, v, attribute);
		if (a)
			*value = a->value;
		else
			*value = MATTR_VAL_ON;
		/*printf(pmap,"mattr_val_get-> %d\n", *value);*/
		break;

	case MATTR_VAL_CACHE_FLUSH:	/* flush from all caches */
	case MATTR_VAL_DCACHE_FLUSH:	/* flush from data cache(s) */
	case MATTR_VAL_ICACHE_FLUSH:	/* flush from instruction cache(s) */

		v = pmap_resident_extract(pmap, address);
		if ((v + size) > mem_size) {
			ret = KERN_INVALID_ARGUMENT;
			break;
		}
		if (v) {
			int cache;
			if (*value == MATTR_VAL_CACHE_FLUSH)
				cache = BCACHE;
			else if(*value == MATTR_VAL_DCACHE_FLUSH)
				cache = DCACHE;
			else 
				cache = ICACHE;
			pmap_cache_flush(pmap, address, size, cache, NV_OK);
		}
		break;

	default:

		ret = KERN_INVALID_ARGUMENT;
		break;

	}

out:
	unlock_pmap(pmap);
	splx(s);

	return ret;
#else	/* notyet */
	return KERN_INVALID_ARGUMENT;
#endif /* notyet */
}

void
pmap_remove_attributes(pmap, start, end)
	pmap_t		pmap;
	vm_offset_t	start, end;
{
	unsigned s;

	if (pmap == PMAP_NULL ||
	    !pmap_has_attributes(pmap) ||
	    start == end)
		return;

	s = splvm();
	lock_pmap(pmap);

	pmap_attribute_update(pmap, start, end, -1, -1);

	unlock_pmap(pmap);
	splx(s);
	/*print_attributes(pmap,"deletion->\n");*/
}

#if	1/*debug*/
void print_attributes(pmap, s)
	pmap_t pmap;
{
	pmap_attribute_t att;

	if (s)
		printf(s);
	att = (pmap_attribute_t)pmap->attributes;
	if (att == 0) {
		printf("Empty\n");
		return;
	}
	while (att) {
		printf("\t@%8x: [%8x..%8x] -> %2d,%2d\n",
		       (uint_t)att, att->start, att->end, att->attribute, att->value);
		att = att->next;
	}
}
#endif

/* 
 * invalidate either 
 *        range of addresses from the TB
 *        or the whole TB
 * if requested 
 *        synchronize the local TB with 
 *        any other processors' TBs 
 */
void
pmap_mmu_tb(pmapp, va, sz, tbop)
	pmap_t         pmapp;
	vm_offset_t    va;
	vm_size_t      sz;
	vm_tbop_t      tbop;
{
	vm_offset_t sva = va;
	vm_size_t   ssz = sz;

	if(tbop == TB_SYNC_NONE || pmapp == PMAP_NULL || pmapp->pid < 0)
		return;

	while(ssz > 0) {
		tlb_unmap(pmapp->pid, sva);
		sva += MIPS_PGBYTES;
		ssz -= MIPS_PGBYTES;
	}

	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(pmapp, va, sz);
}

/*
 * load contiguous phys address into the kernel_pmap
 * map it in the TB(s) if requested
 */
kern_return_t
pmap_mmu_load(kva, phys, sz, prot, tbop)
	vm_offset_t    kva;
	vm_offset_t    phys;
	vm_size_t      sz;
	vm_prot_t      prot;
	vm_tbop_t      tbop;
{
	pte_template    pte;
	pt_entry_t      *kpte;
	int             npf;
	vm_offset_t     skva = kva;

	if (!pmap_module_is_initialized)
		return KERN_FAILURE;

	if(sz == 0 || sz > (VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS))
		KERN_INVALID_ARGUMENT;

	if(kva < VM_MIN_KERNEL_ADDRESS || kva > VM_MAX_KERNEL_ADDRESS)
		return KERN_INVALID_ADDRESS;

	if((npf = 
	    mips_btop(mips_round_page(phys + sz) - mips_trunc_page(phys))) < 1)
		npf = 1;

	pte.raw = PHYSTOPTE(phys) | PG_V | PG_G |
		(mips_protection(kernel_pmap, prot) << PG_PROTOFF);
	if (prot & VM_PROT_WRITE)
		pte.raw |= PG_M;

	while(npf--) {
		kpte = pmap_l1pte(kernel_pmap, kva);
		if (!kpte->pg_v) pmap_pte_fault(kernel_pmap, kpte);
		kpte = pmap_l2pte(kpte, kva);
		*kpte = pte.pte;
		if(tbop != TB_SYNC_NONE)
			tlb_map_random(0, kva, pte.raw);
		if (prot & VM_PROT_WRITE) 
			PMAP_SET_MODIFY(PTETOPHYS((&pte)));
		kva += MIPS_PGBYTES;
		pte.raw += MIPS_PGBYTES;
	}

	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(kernel_pmap, skva, sz);
	return KERN_SUCCESS;
}

/*
 * unload va from the kernel_pmap
 * unmap it from the TB(s) if requested
 */
void
pmap_mmu_unload(va, sz, tbop)
	vm_offset_t va;
	vm_size_t sz;
	vm_tbop_t tbop;
{
	register pt_entry_t    *kpte;
	register int           npf;
	vm_offset_t            sva = va;

	if((npf = 
	    mips_btop(mips_round_page(va + sz) - mips_trunc_page(va))) < 1)
		npf = 1;

	while(npf--) {
		kpte = pmap_l1pte(kernel_pmap, va);
		if (!kpte->pg_v) pmap_pte_fault(kernel_pmap, kpte);
		kpte = pmap_l2pte(kpte, va);
		if(tbop != TB_SYNC_NONE)
			tlb_unmap(0, va);
		*(int *)kpte = 0;
		va += MIPS_PGBYTES;
	}
	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(kernel_pmap, sva, sz);
}

/*
 * copy the user's virt address (uva) for 
 * sz bytes, to the kernel pmap (kva).  protection (kprot)
 * is to let pmap subsystem know that this may be written to.
 * caller is responsible for wiring uva...uva+sz.
 */
kern_return_t 
pmap_dup(upmap, uva, sz, kva, kprot, tbop)
	pmap_t         upmap;
	vm_offset_t    uva;
	vm_size_t      sz;
	vm_offset_t    kva;
	vm_prot_t      kprot;
	vm_tbop_t      tbop;
{
	pt_entry_t    *kpte, *upte;
	int           npf;
	vm_offset_t   skva = kva;

	if (!pmap_module_is_initialized)
		return KERN_FAILURE;

	if(upmap == PMAP_NULL || 
	   sz == 0 || sz > (VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS))
		KERN_INVALID_ARGUMENT;

	if(kva < VM_MIN_KERNEL_ADDRESS || kva > VM_MAX_KERNEL_ADDRESS)
		return KERN_INVALID_ADDRESS;

	if((npf = 
	    mips_btop(mips_round_page(uva + sz) - mips_trunc_page(uva))) < 1)
		npf = 1;

	while(npf--) {

		upte = pmap_l1pte(upmap, uva);		/* get l1pte */

		if (!upte->pg_v)			/* is it valid ? */
			pmap_pte_fault(upmap, upte);    /* no, get a pte page */

		upte = pmap_l2pte(upte, uva);		/* get l2pte */

		if (!(upte->pg_v | upte->pg_refclu))
			panic("pmap_dup: page not valid");
		if (kprot & VM_PROT_WRITE) 
			PMAP_SET_MODIFY(PTETOPHYS((upte)));
		kpte = pmap_l1pte(kernel_pmap, kva);
		if (!kpte->pg_v) pmap_pte_fault(kernel_pmap, kpte);
		kpte = pmap_l2pte(kpte, kva);
		*(int *)kpte = *(int *)upte | PG_G | 
			(mips_protection(kernel_pmap, kprot) << PG_PROTOFF);
		if(tbop != TB_SYNC_NONE)
			tlb_map_random(0, kva, *(int *)kpte);
		kva += MIPS_PGBYTES;  
		uva += MIPS_PGBYTES;
	}
	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(kernel_pmap, skva, sz);
	return KERN_SUCCESS;
}

kern_return_t
pmap_svatophys(kva, phys)
	vm_offset_t kva;
	vm_offset_t *phys;
{
	pt_entry_t     *pte;

	if(IS_KSEG0(kva)) {
		*phys = K0_TO_PHYS(kva);
		return KERN_SUCCESS;
	} 
	if(IS_KSEG1(kva)) {
		*phys = K1_TO_PHYS(kva);
		return KERN_SUCCESS;
	} 
	/* catch special kseg2 addresses first, like wired stack entries */
	if (IS_WIRED(kva)) {
		register vm_offset_t     k2va;
		register thread_t    	thread;

		thread = current_thread();
		k2va = thread->kernel_stack + (kva - 
			(VM_MIN_KERNEL_ADDRESS - KERNEL_STACK_SIZE) );
		kva = k2va;
	}
	/* IS_KSEG2 && !(IS_WIRED) */
	if(IS_KSEG2(kva)) {
		pte = pmap_l1pte(kernel_pmap, kva);
		if (!pte->pg_v) return KERN_INVALID_ADDRESS;
		else pte = pmap_l2pte(pte, kva);
		if (pte->pg_v | pte->pg_refclu) {
			*phys = PTETOPHYS(pte) + (kva & VA_OFFMASK);
			return KERN_SUCCESS;
		}
	}
	return KERN_INVALID_ADDRESS;
}

/*
 * map IO space 
 */
kern_return_t 
pmap_map_io(phys, sz, kva, kprot, tbop)
	vm_offset_t	phys;		/* Starting physical address.	*/
	vm_size_t       sz;		/* siz to be mapped in bytes    */
	vm_offset_t	kva;		/* Starting virtual address.	*/
	vm_prot_t       kprot;
	vm_tbop_t       tbop;
{
	pte_template    pte;
	pt_entry_t      *ppte;
	int             npf;
	vm_offset_t	skva = kva;

	if (!pmap_module_is_initialized)
		return KERN_FAILURE;

	if(kva < VM_MIN_KERNEL_ADDRESS || kva > VM_MAX_KERNEL_ADDRESS)
		return KERN_INVALID_ADDRESS;
	if(sz == 0 || sz > (VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS))
		return KERN_INVALID_ARGUMENT;

	pte.raw = PHYSTOPTE(phys) | PG_V | PG_G | PG_N | 
		(mips_protection(kernel_pmap, kprot) << PG_PROTOFF);

	if((npf = 
	    mips_btop(mips_round_page(kva + sz) - mips_trunc_page(kva))) < 1)
		npf = 1;

	while(npf --) {
		ppte = pmap_l1pte(kernel_pmap, kva);
		if (!ppte->pg_v) pmap_pte_fault(kernel_pmap, ppte);
		ppte = pmap_l2pte(ppte, kva);
		*ppte = pte.pte;
		if(tbop != TB_SYNC_NONE)
			tlb_map_random(0, kva, pte.raw);
		kva += MIPS_PGBYTES;
		pte.raw += MIPS_PGBYTES;
	}
	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(kernel_pmap, skva, sz);
	return KERN_SUCCESS;
}

void
pmap_tbsync(pmap, va, siz)
	pmap_t         pmap;
	vm_offset_t    va;
	vm_size_t      siz;
{
	/* perform tb shootdown... */
}


/***************************************************************************
 *
 *      Address conversion routine.  Used by the upper level
 *      vm_page_startup routine to return architecture independent
 *      addresses (kseg0 for mips, vaddr for others).
 */

vm_offset_t
pmap_mips_k0(vaddr, phys, size)
        vm_offset_t    *vaddr;
        vm_offset_t    phys;
	vm_size_t      size;
{
  return(PHYS_TO_K0(phys));             /* kseg0 on the mips */
}  


/***************************************************************************
 *
 *      Locates holes in memory for vm_page_startup.  The routine
 *      will be called successively by vm_page_startup until all
 *      holes are found.  
 */

boolean_t
pmap_find_holes(phys, size, span, start, end)
        vm_offset_t    *phys;         /* start looking here           */
        vm_size_t      *size;         /* memory size before next hole */
        vm_offset_t    *span;         /* size of hole span            */
        vm_offset_t    start;         /* start of physical memory     */
        vm_offset_t    end;           /* end of physical memory       */
{
   *span = 0;
   *size = end - *phys;               /* no holes on the mips         */
   return(KERN_SUCCESS);
}

/***************************************************************************
 *
 *	TLBPID Management
 *
 *	The MIPS tlb (coprocessor 0) uses the TLBPID register
 *	while searching for a valid mapping of a virtual address.
 *	The register has only a limited size, hence this module
 *	handles the hashing from pmaps to TLBPIDs
 *
 */


#define MAX_PID		TLBHI_NPID
#define	PID_MASK	(MAX_PID-1)

static struct pmap *pids_in_use[MAX_PID] = {0,};
static int next_pid = 1;
/*
 * NOTE: It might be possible to use the pid 0 too. Since it is
 *	 assigned to the kernel_pmap that only holds PG_G type
 *	 mappings the conflict would be unnoticeable.
 *	 But it tastes too awful.
 */

/*
 * Axioms:
 *	- next_pid always points to a free one, unless the table is full;
 *	  in that case it points to a likely candidate for recycling.
 *	- pmap.pid prevents from making duplicates: if -1 there is no
 *	  pid for it, otherwise there is one and only one entry at that index.
 *
 * assign_tlbpid	provides a tlbpid for the given pmap, creating
 *			a new one if necessary
 * destroy_tlbpid	returns a tlbpid to the pool of available ones
 */

void
assign_tlbpid(map)
	struct pmap *map;
{
	register int pid;

	TRACE({printf("pmap:assign_tlbpid x%x --> ", (uint_t)map);})

	DO_COUNT(tpa_count)

	if (map->pid < 0) {

		/* No locks: we know we are called at splhigh() */

		if (pids_in_use[next_pid]) {
			/* are we _really_ sure it's full ? */
			for (pid = 1; pid < MAX_PID; pid++)
				if (pids_in_use[pid] == PMAP_NULL) {
					/* aha! */
					next_pid = pid;
					goto got_a_free_one;
				}
			/* Table full */
			if (active_pmap->pid == next_pid) {
				if (++next_pid == MAX_PID)
					next_pid = 1;
			}
			destroy_tlbpid(next_pid, TRUE);
		}
got_a_free_one:
		pids_in_use[next_pid] = map;
		map->pid = next_pid;
		if (++next_pid == MAX_PID)
			next_pid = 1;
	}
	TRACE({printf(" %d \n", map->pid);})
}

int tlbpid_recycle_fifo = 0;
int tlbpid_flushes = 0;

void
destroy_tlbpid(pid, flush)
	int pid;
	boolean_t flush;
{
	struct pmap    *map;

	/*
	 * NOTE:  We must get here with interrupts off
	 */
	TRACE({printf("pmap:destroy_tlbpid %d\n", pid);})

        DO_COUNT(tpf_count)

	if (pid < 0)	/* not in use anymore */
		return;

	/*
	 * Flush the tlb if necessary.
	 */
	if (flush || tlbpid_flushes)
		tlb_flush_pid(pid,0,NTLBENTRIES);

	/*
	 * Make the pid available, and the map unassigned.
	 */
	map = pids_in_use[pid];
	pids_in_use[pid] = PMAP_NULL;
	if (tlbpid_recycle_fifo)
		next_pid = pid;
	map->pid = -1;
}


/*
 * flush size (in bytes) of K0 address
 * from the appropriate cache
 */
void
pmap_cache_flush(pmap, addr, size, cache, nv)
	pmap_t      pmap;
	vm_offset_t addr;
	vm_size_t   size;
	int         cache;
	int         nv;
{
	vm_offset_t k0addr;
	pt_entry_t  *pte;
	register unsigned rem;

	extern void clean_icache(), clean_dcache(), clean_cache();
	void (*flush_fn)();

	flush_fn = (cache == ICACHE ? clean_icache : 
		    (cache == DCACHE ? clean_dcache : clean_cache));

	if IS_KSEG0(addr){			
		(*flush_fn)(addr, size);
		return;		
		}

	for (; size > 0; size -= rem, addr += rem) {
		if((rem = MIPS_PGBYTES - (addr & VA_OFFMASK)) > size)
			rem = size;

		pte = pmap_l1pte(pmap, addr);		/* get l1pte */

		if (!pte->pg_v){			/* is it valid ? */
			if(nv == NV_NOTOK) 
				/* used by bufflush, page better be there */
				panic("pmap_cache_flush: page not valid");
			else 
				pmap_pte_fault(pmap, pte); /* no, fault it in */
		}

		pte = pmap_l2pte(pte, addr);		/* get l2pte */

    		if ((!pte->pg_v) && (!pte->pg_refclu)) {/* is it invalid ? */
			if(nv == NV_NOTOK) 
				/* used by bufflush, page better be there */
				panic("pmap_cache_flush: page not valid");
			else 
				continue;
		}

		if(pte->pg_n)
			continue;
		k0addr = PHYS_TO_K0(PTETOPHYS(pte)) | (addr & VA_OFFMASK);
		(*flush_fn)(k0addr, rem);
	}
}

void
pmap_pagemove(from, to, size)
	register vm_offset_t from, to;
	int size;
{
	register pt_entry_t *fpte, *tpte;

	if (size % MIPS_PGBYTES || from < K2BASE || to < K2BASE)
		panic("pmap_pagemove");
	fpte = pmap_l1pte(kernel_pmap, from);
	fpte = pmap_l2pte(fpte, from);
	tpte = pmap_l1pte(kernel_pmap, to);
	tpte = pmap_l2pte(tpte, to);
	while (1) {
		*tpte = *fpte;
		*(int *)fpte = 0;
		tlb_unmap(0, from);
		tlb_map_random(0, to, *(int *)tpte);
		size -= MIPS_PGBYTES;
		if (!size) break;
		else {
			from += MIPS_PGBYTES;
			to += MIPS_PGBYTES;
			fpte = pmap_l1pte(kernel_pmap, from);
			fpte = pmap_l2pte(fpte, from);
			tpte = pmap_l1pte(kernel_pmap, to);
			tpte = pmap_l2pte(tpte, to);
		}
	}
}

/*
 * Convert a virtual address to a physical address.
 * The virtual address is NOT guaranteed to be good, so we must
 * continually check values before referencing pointers.
 */
caddr_t
vatophys(va)
        register caddr_t va;    /* the virt.addr to convert to physical */
{
        caddr_t pa;                     /* the physical addr */

        if (IS_KSEG0(va))
                pa = (caddr_t)K0_TO_PHYS(va);
        else if (IS_KSEG1(va))
                pa = (caddr_t)K1_TO_PHYS(va);
	/* catch special kseg2 addresses first, like wired stack entries */
	else if (IS_WIRED(va)) {
		register caddr_t k2va;	/* alloc'd k2addr of wired addr */
		register thread_t    	thread;

		thread = current_thread();
		k2va = thread->kernel_stack + (va - 
			(VM_MIN_KERNEL_ADDRESS - KERNEL_STACK_SIZE) );
		pa = vatophys(k2va);	/* addr. that vatophs can xlate */
	}
	/* IS_KSEG2 && !(IS_WIRED) */
        else if ((vm_offset_t)va >= VM_MIN_KERNEL_ADDRESS) {
                pa = (caddr_t)pmap_resident_extract(kernel_pmap, va);
                if (pa == 0) pa = (caddr_t)-1;
        }
        else if (IS_KUSEG(va)) {
                pa = (caddr_t)pmap_resident_extract(active_pmap, va);
                if (pa == 0) pa = (caddr_t)-1;
        }
        else pa = (caddr_t)-1;
        return(pa);
}

kern_return_t
pmap_seg_alloc(pmap_seg_t *segpp)
{
	register pmap_seg_t segp;
	vm_offset_t addr;

	ZALLOC(pmap_seg_zone, segp, pmap_seg_t);
	simple_lock_init(&segp->ps_seglock);
	simple_lock_init(&segp->ps_reslock);
	segp->ps_refcnt = 1;
	segp->ps_rescnt = 0;
	segp->ps_loadedpte = 0;
	segp->ps_pvsegment = PV_ENTRY_NULL;
	segp->ps_pagetable = get_free_ptepage();
	pmap_segpt_alloc(segp,segp->ps_pagetable);
	*segpp = segp;
	return KERN_SUCCESS;
}

/*
 * Its assumed the seg isn't recoverable
 */

void
pmap_seg_destroy(register pmap_seg_t segp)
{
	vm_offset_t l2ptepage;
	register pv_entry_t pv, pvn;

	pmap_seg_lock(segp);
	if (segp->ps_pagetable) {
		pv = pmap_seg_free_pagetable(segp, &l2ptepage);
		pmap_seg_unlock(segp);
		pmap_segpt_free(l2ptepage);
		put_free_ptepage(l2ptepage);
		while (pv) {
			pvn = pv->pv_next;
			zfree(pv_list_zone, (vm_offset_t) pv);
			pv = pvn;
		}
	}
	else pmap_seg_unlock(segp);
	ZFREE(pmap_seg_zone, segp);
}

struct pv_entry *
pmap_seg_free_pagetable(pmap_seg_t segp, 
		vm_offset_t *l2ptepage)
{
	register pt_entry_t *pte, *lpte;
	register int i, j, s;
	register vm_offset_t pa;
	register pv_entry_t pvhp, pvc, pvp, freepv;
	
	freepv = PV_ENTRY_NULL;
	pte = (pt_entry_t *) (segp->ps_pagetable);
	lpte = pte + PTES_PER_PAGE;
	j = vm_to_mips(1);
	for (; (pte < lpte) && segp->ps_loadedpte;) 
		if (* (int *) pte & (PG_V|PG_REFCLU)) {
			segp->ps_loadedpte--;
			pa = PTETOPHYS(pte);
			lock_pvh_pa(s,pa);
			pvc = pvhp = pa_to_pvh(pa);
			pvp = PV_ENTRY_NULL;
			while (pvc) {
				if (pv_isseg(pvc) && pvc->pv_seg == segp) break;
				pvp = pvc;
				pvc = pvc->pv_next;
			}
			if (pvc == PV_ENTRY_NULL) 
				panic("pmap_seg_free_pageable: pv not found");
			if (pvc == pvhp) {
				if ((pvc = pvhp->pv_next) == PV_ENTRY_NULL) 
					pvhp->pv_pmap = PMAP_NULL;
				else *pvhp = *pvc;
			}
			else pvp->pv_next = pvc->pv_next;
			unlock_pvh_pa(s,pa);
			if (pvc) {
				pvc->pv_next = freepv;
				freepv = pvc;
			}
			for (i = j; i; i--, pte++) * (int *) pte = 0;
		}
		else pte += j;

	*l2ptepage = segp->ps_pagetable;
	return freepv;
}

/*
 * Enter a physical address into the segment.
 * Note it might already be loaded.
 */

void
pmap_seg_enter(pmap_t pmap,
	register pmap_seg_t segp,
	register vm_offset_t addr,
	register vm_offset_t phys,
	vm_prot_t prot)
{
	register pt_entry_t *pte, *l1pte;
	register pv_entry_t pvhp, pvep;
	register int s, i;
	vm_offset_t pttable;
	register boolean_t ptemodify;

	assert((prot == VM_PROT_EXECUTE) || 
		(prot == (VM_PROT_EXECUTE|VM_PROT_READ)));

	pvep = PV_ENTRY_NULL;
	pttable = (vm_offset_t) 0;

	l1pte = pmap_l1pte(pmap, addr);
retry:
	READ_LOCK(s);
	pmap_seg_lock(segp);
	lock_pmap(pmap);
	if (!(* (int *) l1pte & PG_V)) {
		if (segp->ps_pagetable) 
			((pte_template *) l1pte)->raw = 
				PG_V | PG_SEGMENT | PG_M |
				((VM_PROT_READ | VM_PROT_WRITE) << PG_PROTOFF) |
				PHYSTOPTE(K0_TO_PHYS(segp->ps_pagetable));
		else if (pttable != (vm_offset_t) 0) {
			segp->ps_pagetable = pttable;
			((pte_template *) l1pte)->raw = 
				PG_V | PG_SEGMENT | PG_M |
				((VM_PROT_READ | VM_PROT_WRITE) << PG_PROTOFF) |
				PHYSTOPTE(K0_TO_PHYS(pttable));
			pmap_segpt_alloc(segp,pttable);
			pttable = (vm_offset_t) 0;
		}
		else {
			unlock_pmap(pmap);
			pmap_seg_unlock(segp);
			READ_UNLOCK(s);
			pttable = get_free_ptepage();
			goto retry;
		}
		pmap_segres_lock(segp);
		segp->ps_rescnt++;
		pmap_segres_unlock(segp);
	}

	pte = pmap_segpte(segp, addr);

	if (* (int *) pte & (PG_V|PG_REFCLU)) {
		if (* (int *) pte & PG_REFCLU) {
			set_reference_bit(phys);
			ptemodify = TRUE;
		}
		else ptemodify = FALSE;
	}
	else {
		register vm_offset_t pa;
		register int p;

		lock_pvh_pa(p,phys);
		pvhp = pa_to_pvh(phys);
		if (pvhp->pv_pmap == PMAP_NULL) {
			pvhp->pv_seg = segp;
			pvhp->pv_va = addr | pv_segtype;
			pvhp->pv_next = PV_ENTRY_NULL;
		}
		else {
			if (pvep == PV_ENTRY_NULL) {
				unlock_pvh_pa(p, phys);
				unlock_pmap(pmap);
				pmap_seg_unlock(segp);
				READ_UNLOCK(s);
				pvep = (pv_entry_t) zalloc(pv_list_zone);
				goto retry;
			}
			pvep->pv_seg = segp;
			pvep->pv_va = addr | pv_segtype;
			pvep->pv_next = pvhp->pv_next;
			pvhp->pv_next = pvep;
			pvep = PV_ENTRY_NULL;
		}
		unlock_pvh_pa(p,phys);
		segp->ps_loadedpte++;
		for (pa = phys, i = vm_to_mips(1); i; i--, pa += MIPS_PGBYTES)
			page_iflush(PHYS_TO_K0(pa));
		ptemodify = TRUE;
	}

	for (i = vm_to_mips(1); i; 
		addr += MIPS_PGBYTES, phys += MIPS_PGBYTES, pte++, i--) {

		if (ptemodify == TRUE)
			((pte_template *)pte)->raw = PHYSTOPTE(phys) | PG_V | 
				(prot << PG_PROTOFF);
		if (pmap->pid >= 0) 
			tlb_map_random(pmap->pid, addr, * (int *) pte);
	}

	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	READ_UNLOCK(s);
	if (pvep) zfree(pv_list_zone, (vm_offset_t) pvep);
	if (pttable) put_free_ptepage(pttable);
	return;
}

/*
 * Load a segment into the addres space.
 */

kern_return_t
pmap_seg_load(register pmap_t pmap, 
	register pmap_seg_t segp,
	register vm_offset_t addr)
{
	register int s;
	register pt_entry_t *l1pte;
	register pv_entry_t spve;
	vm_offset_t ol2pttable;
	register pttmp;

	spve = PV_ENTRY_NULL;
	ol2pttable = (vm_offset_t) 0;
	addr &= ~pmap_seg_mask;
	READ_LOCK(s);
	pmap_seg_lock(segp);
	lock_pmap(pmap);
	l1pte = pmap_l1pte(pmap, addr);

	if ((* (int *) l1pte & (PG_V|PG_SEGMENT)) == PG_V) {
		spve = pmap_remove_range(pmap, addr, addr + pmap_seg_size,
			&ol2pttable);
		if (spve != PV_ENTRY_NULL) 
			panic("pmap_seg_load: old l2pte not empty");
		pmap_clean_pcache(pmap, TRUE);	
		* (int *) l1pte = 0;
	}
	else if (* (int *) l1pte & PG_SEGMENT) {
		if (segp != pmap_ptptetoseg(l1pte))
			panic("pmap_seg_load: old segment not unloaded");
		goto done;
	}

	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	READ_UNLOCK(s);
	spve = (pv_entry_t) zalloc(pv_list_zone);
	READ_LOCK(s);
	pmap_seg_lock(segp);
	lock_pmap(pmap);
	if (* (int *) l1pte & PG_SEGMENT) goto done;
	segp->ps_refcnt++;
	spve->pv_pmap = pmap;
	spve->pv_va = addr & ~pmap_seg_mask;
	spve->pv_next = segp->ps_pvsegment;
	segp->ps_pvsegment = spve;
	spve = (pv_entry_t) 0;
	if (segp->ps_pagetable) {
		pttmp = PHYSTOPTE(K0_TO_PHYS(segp->ps_pagetable)) | PG_V;
		pmap_segres_lock(segp);
		segp->ps_rescnt++;
		pmap_segres_unlock(segp);
	}
	else pttmp = 0;
	((pte_template *) l1pte)->raw = pttmp | PG_SEGMENT | PG_M | 
			((VM_PROT_READ | VM_PROT_WRITE) << PG_PROTOFF);
done:
	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	READ_UNLOCK(s);
	if (spve != PV_ENTRY_NULL) 
		zfree(pv_list_zone, (vm_offset_t) spve);
	if (ol2pttable) pmap_free_l2pte_list(ol2pttable);
	return KERN_SUCCESS;
}

/*
 * Unload this segment from this pmap at the
 * position segstart within the address space.
 */

void
pmap_seg_unload(register pmap_t pmap,
	register pmap_seg_t segp,
	register vm_offset_t addr)
{
	register pt_entry_t *l1pte;
	register pv_entry_t spv, pspv;
	register vm_offset_t l2ptepage;

	spv = (pv_entry_t) 0;
	l2ptepage = (vm_offset_t) 0;
	addr &= ~pmap_seg_mask;
	pmap_seg_lock(segp);
	lock_pmap(pmap);
	l1pte = pmap_l1pte(pmap, addr);
	if (!(* (int *)l1pte & PG_SEGMENT)) goto done;
	pspv = PV_ENTRY_NULL;
	spv = segp->ps_pvsegment;
	while (spv) {
		if (spv->pv_pmap == pmap && spv->pv_va == addr) break;
		pspv = spv;
		spv = spv->pv_next;
	}
	if (spv == PV_ENTRY_NULL) panic("pmap_seg_unload: null pv");
	if (pspv) pspv->pv_next = spv->pv_next;
	else segp->ps_pvsegment = spv->pv_next;

	if (* (int *) l1pte & PG_V) {

		/*
		 * Nuke the pcache
		 */

		pmap_clean_pcache(pmap, TRUE);	

		/*
		 * call tlb nuke range for pid.
		 */

		addr &= ~pmap_seg_mask;	
		if (pmap->pid >= 0) 
			tlb_flush_range(pmap->pid, mips_btop(addr),
			mips_btop(addr + pmap_seg_size) - 1);

		pmap_segres_lock(segp);
		segp->ps_rescnt--;
		pmap_segres_unlock(segp);
	}

	((pte_template *)l1pte)->raw  = 0;
	segp->ps_refcnt--;
	if (!segp->ps_rescnt && !segp->ps_loadedpte && segp->ps_pagetable) {
		l2ptepage = segp->ps_pagetable;
		pmap_segpt_free(l2ptepage);
		segp->ps_pagetable = (vm_offset_t) 0;
	}
	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	zfree(pv_list_zone, (vm_offset_t) spv);
	if (l2ptepage) put_free_ptepage(l2ptepage);
	return;
done:
	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	return;
}

void
pmap_seg_remove_all(register pmap_seg_t segp, 
	register vm_offset_t va)
{
	register int pteindex;
	register pmap_t map;
	register int i, vtm;
	register pv_entry_t spv;
	register pt_entry_t *l1pte;

	pteindex = mips_ptob(va_to_ptindex(va));
	vtm = vm_to_mips(1);
	for (spv = segp->ps_pvsegment; spv; spv = spv->pv_next) {
		map = spv->pv_pmap;
		lock_pmap(map);
		if (map->pid >= 0) {
			va = spv->pv_va + pteindex;
			l1pte = pmap_l1pte(map, va);
			if (* (int *)l1pte & PG_V) 
				for (i = vtm; i; i--, va += MIPS_PGBYTES) 
					tlb_unmap(map->pid, va);
		}
		unlock_pmap(map);
	}
}

extern vm_offset_t avail_start;
extern int etext[];
extern int maxmem, physmem;

int num_kernel_pages(void)
{
  register int index, count, data;
  register pt_entry_t *pte;

  pte = &((pt_entry_t *) root_kptes)[PTES_PER_PAGE/2];
  data = (int) K0_TO_PHYS(etext) & VA_PAGEMASK;
  count = (avail_start - data)/NBPG;
  count += physmem - maxmem;
  for (index = PTES_PER_PAGE/2; index < PTES_PER_PAGE; index++, pte++){
    if (pte->pg_v){

      register int l2_index;
      register pt_entry_t *l2pte;

      /*
       *  Scan l2pte page looking for valid l2ptes.
       */

      l2pte = (pt_entry_t *)PHYS_TO_K0(PTETOPHYS(pte));

      for (l2_index = 0; l2_index < PTES_PER_PAGE;
	   l2_index++, l2pte++){

	if (l2pte->pg_v || l2pte->pg_refclu){
	  if(PTETOPHYS(l2pte) >= avail_start) count++;
	}
      }
    }
  }
  return(count);
}

int get_next_page(vm_offset_t *blocks, int maxblocks)
{
  static vm_offset_t kernel_addr = -1;
  static int index = PTES_PER_PAGE/2;
  static int l2_index = 0;
  static int state = 0;  
  register pt_entry_t *pte;
  int count = 0;

  switch(state){
  case 0:			/* Low memory */
    if(kernel_addr == -1) kernel_addr = K0_TO_PHYS(etext) & VA_PAGEMASK;
    for(;kernel_addr < avail_start;kernel_addr+=NBPG){     
      blocks[count++] = PHYS_TO_K0(kernel_addr);
      if(count == maxblocks){
	kernel_addr += NBPG;
	return(maxblocks);
      }
    }
    kernel_addr = -1;
    state = 1;
    /* Fall through */
  case 1:			/* memory referenced by a pte */
    pte = &((pt_entry_t *) root_kptes)[index];
    for (; index < PTES_PER_PAGE; index++, pte++){
      if (pte->pg_v){
	register pt_entry_t *l2pte;
	
	/*
	 *  Scan l2pte page looking for valid l2ptes.
	 */

	l2pte = &((pt_entry_t *)PHYS_TO_K0(PTETOPHYS(pte)))[l2_index];
	for (; l2_index < PTES_PER_PAGE;l2pte++){
	  l2_index++;
	  if((l2pte->pg_v || l2pte->pg_refclu) &&
	     (PTETOPHYS(l2pte) >= avail_start)){
	    blocks[count++] = PHYS_TO_K0(PTETOPHYS(l2pte));
	    if(count == maxblocks){
	      return(maxblocks);
	    }
	  }
	}
	l2_index = 0;
      }
    }
    state = 2;
    /* Fall through */
  case 2:                      /* High memory */
    if(kernel_addr == -1) kernel_addr = mips_ptob(maxmem);
    for(;kernel_addr < mips_ptob(physmem);kernel_addr += NBPG){
      blocks[count++] = PHYS_TO_K0(kernel_addr);
      if(count == maxblocks){
	kernel_addr += NBPG;
	return(maxblocks);
      }
    }
  }
  return(count);
}

