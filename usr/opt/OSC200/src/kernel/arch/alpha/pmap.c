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
static char *rcsid = "@(#)$RCSfile: pmap.c,v $ $Revision: 1.2.57.16 $ (DEC) $Date: 1994/01/11 20:50:40 $";
#endif

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	File:	pmap.c
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

#include <mach/boolean.h>
#include <mach/vm_attributes.h>
#include <mach/vm_param.h>
#include <kern/macro_help.h>
#include <kern/lock.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/sched_prim.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/table.h>
#include <sys/proc.h>
#include <machine/cpu.h>
#include <machine/rpb.h>
#include <machine/machparam.h>
#include <machine/pmap_lw.h>
#include <kdebug.h>


#define managed(PA)		((PA) >= vm_first_phys && (PA) < vm_last_phys)
#define IS_TEXT(PROT)		(((PROT) \
	& (VM_PROT_WRITE | VM_PROT_EXECUTE)) == VM_PROT_EXECUTE)
#define MAPS_TEXT(PTE)		(((PTE)->quadword \
	& (PTEQ_MASK_KWE | PTEQ_MASK_UWE | PTEQ_MASK_EXEC)) == PTEQ_MASK_EXEC) 
#define PAGE_PER_MB		(0x100000/ALPHA_PGBYTES)

vm_size_t pmap_seg_size = PTES_PER_PAGE * ALPHA_PGBYTES;
vm_size_t pmap_seg_mask = (PTES_PER_PAGE * ALPHA_PGBYTES) - 1;

struct pmap	kernel_pmap_store;
pmap_t		kernel_pmap;		/* kernel pmap, statically alloced */
pmap_t	/*cpu->*/active_pmap;		/* pmap for the current_thread() */
vm_map_t	pmap_submap;		/* kernel submap where we take
					   our pages from */
vm_offset_t	pmap_vlow, pmap_vhigh;	/* (virtual) limits for our needs */
vm_offset_t	pmap_physroot = 0L;	/* physical root of sys page table */
vm_offset_t	pmap_physhwrpb = 0L;	/* phys addr of hardware rpb struct */
vm_offset_t	pmap_physetext = 0L;	/* phys addr of end of kernel text */
int		pmap_maxasn = -1;	/* max asn from rpb unless patched */
struct zone	*pmap_zone;		/* Zone of pmap structures  */
struct zone	*pmap_seg_zone;		/* Zone of segments */

vm_offset_t	pmap_resident_extract();
static pt_entry_t * pmap_map_pt_pages();

struct scavenge_list scavenge_info; 	/* scavenge_list for post-boot memory
								 scavenging */

void		pmap_release_page(), 
		pmap_clean_pcache(),
		pmap_tbsync(),
		pmap_pagemove(),
                pmap_segpt_free(),
		pmap_enter_range(),
		pmap_zero_page();
extern void	init_zero_page();


/*
 *	The following global variables describe memory size, and
 *	are all initialized by pmap_bootstrap or setup_next_pfn.
 */

extern int maxmem;			/* number of physical pages for OS */
extern int physmem;			/* last physical page number + 1 */
extern vm_size_t mem_size;		/* size of OS memory in bytes */
extern vm_offset_t avail_start;		/* first available physical address */
extern vm_offset_t avail_end;		/* last available physical addr + 1 */
extern vm_offset_t virtual_avail;	/* lowest avail kernel virtual addr */
extern vm_offset_t virtual_end;		/* highest avail kernel virtual addr */
extern void dumpbm();

extern long lww_event[];
/*
 *	For each vm_page_t, there is a list of all currently
 *	valid virtual mappings of that page.  An entry is
 *	a pv_entry_t; the lists' heads are in the pv_table.
 *
 *	Rather than maintain separate arrays for lock, modify,
 *	and reference bits, we steal the three low-order bits
 *	of the virtual address field in each list header.  It
 *	works because operations on the virtual address are really
 *	operations on the virtual page number, and the low-order
 *	bits are "noise".
 */
struct pv_entry {
	vm_offset_t	pv_va;		/* overlaid with pv_bits (below) */
	union {
		pmap_t		_pv_pmap;	/* pmap where mapping lies */
		pmap_seg_t	_pv_seg;	/* segment */
		int		_pv_mappings;	/* valid mapping count */
	} _upv0;
	struct pv_entry	*pv_next;	/* next pv_entry */
};

#define	pv_pmap		_upv0._pv_pmap
#define	pv_seg		_upv0._pv_seg
#define	pv_mappings	_upv0._pv_mappings

/*
 * Steal bit 3 of the VA to identify the pv entry
 * as belonging to a segment.  We chose 3 because 
 * bits 0 thru 2 are being used in the pv head for other purposes.
 * Also bit 3 with be preserved because it will be treated a part of the VA.
 */

#define	PV_SEGFLAG	0x00000008L		/* Ored in pv_va for seg id. */
#define	pv_isseg(PV)	((PV)->pv_va & PV_SEGFLAG)

#define	pmap_segpte(SEGP,VA)						\
	(pt_entry_t *) ((SEGP)->ps_pagetable + (LEVEL3_PT_OFFSET(VA) << 3))
#define	pte_to_pvh(PTE)							\
	pa_to_pvh(PTETOPHYS((PTE)))

#define pmap_ptptetoseg(PTPTE,SEG) do {                                 \
        pv_entry_t      PVK;                                            \
        pvh_to_pvk(pa_to_pvh(PTETOPHYS((PTPTE))), PVK);                 \
        (SEG) = PVK ?  PVK->pv_seg : (pmap_seg_t)NULL;                  \
} while (1 == 0)

#define pmap_segpt_alloc(SEG,PT) do {                                   \
        pv_entry_t      PVK;                                            \
        pvh_to_pvk(pa_to_pvh(KSEG_TO_PHYS(PT)), PVK);                   \
        if(PVK)                                                         \
              PVK->pv_seg = (SEG);                                      \
      	else                                                            \
              panic("pmap_segpt_alloc: no kernel map for this page");   \
} while (1 == 0)

#define	pmap_free_pt_list(PTE) do {					\
	register vm_offset_t NPTE;					\
	do {								\
		NPTE = * (vm_offset_t *) (PTE);				\
		put_free_ptepage((PTE));				\
		(vm_offset_t) (PTE) = NPTE;				\
	} while ((PTE) != (vm_offset_t) 0);				\
} while (1 == 0)

#define pvh_to_pvk(PVH, PVK) do {                                       \
        pv_entry_t      cur;                                            \
        cur = (pv_entry_t)(PVH);                                        \
        (PVK) = PV_ENTRY_NULL;                                          \
        do {                                                            \
                if (IS_SYS_VA(cur->pv_va)) {                            \
                        (PVK) = cur;                                    \
                        break;                                          \
                }                                                       \
        } while (cur = cur->pv_next);                                   \
} while (1 == 0)

union pv_list_head {
	struct pv_bits {
		unsigned	unlock:1,
				modify:1,
				ref:1;
	} bit;
	struct {
		unsigned	keep:3,
				lose:29;
	} bits;
	struct pv_entry	entry;
};
#define PV_LIST_HEAD_NULL ((union pv_list_head *) 0)

typedef struct pv_entry *pv_entry_t;
#define	PV_ENTRY_NULL	((pv_entry_t) 0)

/*
 * The pv_list proper, which is actually an array of lists
 */
union pv_list_head	*pv_head_table;	/* array of entries, one per page */
zone_t			pv_list_zone;	/* zone of pv_entry structures */

vm_offset_t	vm_first_phys;	/* range of phys pages which we can handle */
vm_offset_t	vm_last_phys;

#define	pa_index(pa)		(alpha_btop((pa) - vm_first_phys))
#define pa_to_pvh(pa)		(&pv_head_table[pa_index(pa)])

/* locking of the pv_list */
#if NCPUS > 1
#define lock_pvh(h)	_lock_low_bit_down(h)
#define unlock_pvh(h)	_unlock_low_bit_up(h)
void _lock_low_bit_down(h) {}
void _unlock_low_bit_up(h) {}
#else
#define lock_pvh(h)	(h->bit.unlock = 0)
#define unlock_pvh(h)	(h->bit.unlock = 1)
#endif /* NCPUS */

#define BOOT_Selfmap	((pt_entry_t *)(SELF_PFN << (3 * pgshift - 6)))

#define In_kernel_text(v) \
			((v) >= (vm_offset_t)start && (v) < (vm_offset_t)etext)

#define vtopte(v)	(&Selfmap[((vm_offset_t)(v) & ADDRESS_MASK) >> PGSHIFT])
#define Root_pt		vtopte(vtopte(Selfmap))

pt_entry_t *
pmap_pte(map, addr)
	pmap_t map;
	vm_offset_t addr;
{
#define pmap_pte(MAP, ADDR) (vtopte(ADDR)				\
 + (((MAP) == kernel_pmap || (MAP) == vm_map_pmap(current_task()->map))	\
   ? 0									\
   : (Othermap - Selfmap)))

	return pmap_pte(map, addr);
}

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
vm_offset_t	nonseg_free_ptepages = 0;
thread_t	ptepages_thread = THREAD_NULL;
boolean_t	ptepages_wanted = FALSE;

unsigned int	ptepages_freed = 0;
unsigned int	ptepages_taken = 0;

#define MIN_PTE_PAGES 4
#define MAX_PTE_PAGES MIN(2*MAX(NCPUS, MIN_PTE_PAGES), 16)
int		min_pte_pages = MIN_PTE_PAGES;
int		max_pte_pages = MAX_PTE_PAGES;
int		more_pte_pages = (MAX_PTE_PAGES - MIN_PTE_PAGES)/2;

extern void _scb();
#define END_OF_BOOT_TEXT ((vm_offset_t)_scb)
vm_offset_t end_of_scavenge;
#define scavenged(KSEG_ADDR) ((KSEG_ADDR) < end_of_scavenge)

static void
put_free_ptepage(page)
	vm_offset_t		page;
{
	register vm_offset_t *	freelist;
	/*
	 *	Put the page back on the free list.
	 *	But don't let the free list get too big!
	 */

	freelist = (scavenged(page)) ? &nonseg_free_ptepages : &free_ptepages;

	simple_lock(&ptepages_lock);

	if (freelist == &nonseg_free_ptepages
	    || free_ptepages_count < max_pte_pages) {
		*(vm_offset_t *)page = *freelist;
		*freelist = page;
		free_ptepages_count += (freelist == &free_ptepages);
		page = 0;
	}

	if (ptepages_wanted) {
		ptepages_wanted = FALSE;
		thread_wakeup((vm_offset_t)&ptepages_wanted);
	}

	simple_unlock(&ptepages_lock);

	/*
	 *	Actually release the page if it
	 *	didn't go on the free list.
	 */

	if (page)
		pmap_release_page(KSEG_TO_PHYS(page));
}

void
pmap_scavenge_boot()
{
	extern struct scavenge_list	scavenge_info;
	register long			count = scavenge_info.count;
	register vm_offset_t		page_addr = scavenge_info.kseg_start;

	while (--count >= 0) {
		put_free_ptepage(page_addr);
		page_addr += NBPG;
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
	int	num_pte_pages;

	ptepages_taken++;

	INC_MPP_COUNT;

	/*
	 * Keep backing off the size of the allocation in the failure 	  
	 * case.  The pmap submap can be fragmented such that there arent 
	 * more_pte_pages of contiguous virtual space in the pmap submap. 
	 */

	for (num_pte_pages = more_pte_pages; num_pte_pages; num_pte_pages /= 2) 
		if(vp = kmem_alloc(pmap_submap, num_pte_pages*NBPG))
			break;
	
	if (num_pte_pages) {

		for (i = 0; i < num_pte_pages; i++) {
			/* we speak KSEG here */
			pp = pmap_resident_extract(kernel_pmap, vp);
			pp = PHYS_TO_KSEG(pp);
			put_free_ptepage(pp);
			vp += NBPG;
		}
	} else
		panic("get_more_pte_pages");
}

/*
 * Segmentation uses the PV entry of the segment PTE page to hold a
 * pointer to the segment structure.  Only managed pages have PV
 * entries.  Scavenged pages are not managed and therefore cannot
 * serve as segment PTE pages.  Therefore, getting a PTE page is a
 * little more complicated in a kernel with segmentation than in a
 * kernel without.
 */
static vm_offset_t
#if	PMAP_SEGMENTATION
get_ptepage(for_seg)
long for_seg;

#define get_seg_ptepage()	get_ptepage(1)
#define get_free_ptepage()	get_ptepage(0)

#else	/* PMAP_SEGMENTATION */
get_free_ptepage()
#define for_seg	0

#endif	/* PMAP_SEGMENTATION */
{
	register vm_offset_t	page, * freelist;

	for (;;) {
		freelist = &nonseg_free_ptepages;

		simple_lock(&ptepages_lock);
		if (*freelist && !for_seg)
			break;

		freelist = &free_ptepages;
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
		thread_sleep((vm_offset_t)&ptepages_wanted,
			     simple_lock_addr(ptepages_lock),
			     FALSE);
	}

	page = *freelist;
	*freelist = * (vm_offset_t *) page;
	pmap_zero_page(KSEG_TO_PHYS(page));
	free_ptepages_count -= (freelist == &free_ptepages);

	simple_unlock(&ptepages_lock);
	return page;
}

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
	register pv_entry_t	pv;
	vm_offset_t		alloc_va;
	union pv_list_head	*head;
	long			s;

	ptepages_freed++;

	/*
	 *	Look for the mapping in the kernel pmap
	 *	in the right virtual range.  There should
	 *	only be one such.
	 */
	s = splvm();
	head = pa_to_pvh(pa);
	lock_pvh(head);
	for (pv = &head->entry; ; pv = pv->pv_next) {

		if(pv->pv_pmap == PMAP_NULL)
			panic("pmap_release_page");

		if((pv->pv_pmap == kernel_pmap) && 
		   (pv->pv_va >= pmap_vlow) && (pv->pv_va < pmap_vhigh))
			break;
	}
	alloc_va = alpha_trunc_page(pv->pv_va);
	unlock_pvh(head);
	(void)splx(s);

	kmem_free(pmap_submap, alloc_va, NBPG);
}

pv_entry_t
grab_pv_head(pa)
	vm_offset_t	pa;
{
	union pv_list_head	*head;

	head = pa_to_pvh(pa);
	lock_pvh(head);
	return head->entry.pv_pmap
		? PV_ENTRY_NULL		/* head is in use */
		: &head->entry;		/* head is available */
}

pv_entry_t
insert_pv_entry(pa, zentry)
	vm_offset_t	pa;
	pv_entry_t	zentry;
{
	union pv_list_head	*head;

	head = pa_to_pvh(pa);
	zentry->pv_next = head->entry.pv_next;
	head->entry.pv_next = zentry;
	return zentry;
}

void
fill_pv_entry(pa, entry, map, va)
	vm_offset_t	pa;
	pv_entry_t	entry;
	pmap_t		map;
	vm_offset_t	va;
{
	union pv_list_head	*head;
	unsigned long		save;

	head = pa_to_pvh(pa);
	save = head->bits.keep;

	entry->pv_va = va;
	entry->pv_pmap = map;

	head->bits.keep = save;
	unlock_pvh(head);
}

void
delete_pv_entry(pa, map, va)
	vm_offset_t	pa;
	pmap_t		map;
	vm_offset_t	va;
{
	union pv_list_head	*head;
	pv_entry_t		prev, cur;
	unsigned long		save;

	head = pa_to_pvh(pa);
	lock_pvh(head);
	save = head->bits.keep;

	prev = PV_ENTRY_NULL;
	cur = &head->entry;
	while (cur) {
		if (map == cur->pv_pmap && (va ^ cur->pv_va) < NBPG)
			break;
		prev = cur;
		cur = cur->pv_next;
	}
	if (cur == PV_ENTRY_NULL) {
		printf("pa 0x%l012x   map 0x%l016x   va 0x%l016x\n",
		       pa, map, va);
		printf("prev->pv_va 0x%l016x   prev->pv_pmap 0x%l016x\n",
		       prev->pv_va, prev->pv_pmap);
		panic("delete_pv_entry: mapping not in pv_list");
	}
	if (prev == PV_ENTRY_NULL)
		if (cur->pv_next == PV_ENTRY_NULL)
			cur->pv_pmap = PMAP_NULL;
		else {
			pv_entry_t	temp = cur->pv_next;
			*cur = *temp;
			ZFREE(pv_list_zone, (vm_offset_t)temp);
		}
	else {
		prev->pv_next = cur->pv_next;
		ZFREE(pv_list_zone, (vm_offset_t)cur);
	}

	head->bits.keep = save;
	unlock_pvh(head);
}

#define clear_modify_bit(pa) if (1)			\
	{union pv_list_head *	_HEAD = pa_to_pvh(pa);	\
		lock_pvh(_HEAD);			\
		_HEAD->bit.modify = 0;			\
		unlock_pvh(_HEAD);			\
	} else

#define modify_bit(pa)	(pa_to_pvh(pa)->bit.modify)

#define PMAP_SET_MODIFY(p) if (1) {		\
	INC_MOD_COUNT;				\
	if (managed(p)) {			\
		union pv_list_head *	_HEAD;	\
		_HEAD = pa_to_pvh(p);		\
		lock_pvh(_HEAD);		\
		(_HEAD)->bit.modify = 1;	\
		unlock_pvh(_HEAD);		\
	}					\
} else

#ifdef	REF_BITS
					/* reference bits */
#define set_reference_bit(head) ((head)->bit.ref = 1)

#define reference_bit(pa)	(pa_to_pvh(pa)->bit.ref)
#define CLEAR_for(pte)		((pte).quadword &= ~0x2L)
#define SET_fo_any(pte)		((pte).quadword |= 0xeL)
#define IS_READ(ACCESS)		((ACCESS) == MMF_READ)

#else	/* REF_BITS */
					/* NO reference bits */
#define set_reference_bit(head)	1
#define clear_reference_bit(pa)
#define reference_bit(pa)	((boolean_t)FALSE)
#define CLEAR_for(pte)		0
#define SET_fo_any(pte)		((pte).quadword |= 0xcL)
#define IS_READ(ACCESS)		((boolean_t)FALSE)

#endif	/* REF_BITS */

/*
 *	Lock and unlock macros for the pmap system.  Interrupts must be
 *	locked out because many of the pmap functions can be invoked at
 *	interrupt level.  The locking hierarchy is (from top to bottom):
 *
 *	1) pmap_lock				using BEGIN_LOCKS_IN_ORDER()
 *	2) (struct pmap *)->lock		using lock_pmap(pmap)
 *	3) (union pv_list_head *)->bit.unlock	using lock_pvh(head)
 *
 *	In the case where a level 3 lock needs to be acquired before
 *	a level 2 lock, the level 1 lock is first acquired with the
 *	BEGIN_LOCKS_OUT_OF_ORDER macro.  This protocol eliminates any
 *	deadlock potential because when a "write lock" has been granted
 *	on the global pmap_lock, no other locks on levels 2 and 3 could
 *	be outstanding.
 */

lock_data_t	pmap_lock;	/* General lock for the pmap system */
#if NCPUS > 1
#define BEGIN_LOCKS_IN_ORDER()		lock_read(&pmap_lock)
#define END_LOCKS_IN_ORDER()		lock_read_done(&pmap_lock)
#define BEGIN_LOCKS_OUT_OF_ORDER()	lock_write(&pmap_lock)
#define END_LOCKS_OUT_OF_ORDER()	lock_write_done(&pmap_lock)
#define	lock_pmap(pmap)			simple_lock(&(pmap)->lock)
#define	unlock_pmap(pmap)		simple_unlock(&(pmap)->lock)
#else
#define BEGIN_LOCKS_IN_ORDER()
#define END_LOCKS_IN_ORDER()
#define BEGIN_LOCKS_OUT_OF_ORDER()
#define END_LOCKS_OUT_OF_ORDER()
#define	lock_pmap(pmap)
#define	unlock_pmap(pmap)
#endif /* NCPUS */

static boolean_t pmap_module_is_initialized = FALSE;

long pmap_debug = 0;
#define TRACE(x) if (pmap_debug) x
/*#define TRACE(x) */

/*
 * The following 2 tables will contain prototype PTEs indexed by the OSF
 * VM protection code.  Each prototype contains the appropriate settings
 * for the pg_prot, pg_exec, pg_foe, pg_asm, and pg_v fields.  All of the
 * remaining bits will be clear.
 *
 * The first table is for PTEs in the kernel pmap, the second one is for
 * PTEs in user pmap's.  Since indexing into these tables depends on the
 * VM protection codes being 0-7, a dummy structure declaration exists to
 * generate a compile error if the current VM_PROT_xxx flag settings change.
 *
 * These tables are initialized in pmap_bootstrap() because initialized
 * data can't be used with unions.  The patchable global variable named
 * alpha_independent_vm_exec_access controls whether the prototype PTEs
 * specify independent execute access from read access.  (Setting it to
 * 0 will grant execute permission for any memory having read access.)
 *
 * Use of the previous single prototype for system PTEs will be replaced
 * by an appropriate reference to the kernel prototype pte table.
 */
pt_entry_t proto_kern_ptetab[VM_PROT_ALL + 1];
pt_entry_t proto_user_ptetab[VM_PROT_ALL + 1];
pt_entry_t proto_user_segpte;
struct dependency_on_vm_prot_flag_values { char x[VM_PROT_ALL == 7]; };
int alpha_independent_vm_exec_access = 1;
#define proto_sys_pte proto_kern_ptetab[VM_PROT_READ | VM_PROT_WRITE]

/*
 * Some fields of the HWRPB contain virtual addresses in bootspace Region
 * 0.  Rather than change the HWRPB or copy it, add "hwrpb_offset" to
 * the field to adjust the address to its value in the running system.
 */
vm_offset_t hwrpb_offset = 0;

struct rpb_mdt *memdsc;		/* The memory descriptor */
long nfpfn;			/* Next available page */
struct rpb_cluster *memcl;	/* and the cluster in which it sits. */

long
fls(m)		/* Find the position of the most significant 1. */
	unsigned long m;
{
	unsigned long n;
	long shift, log;

	if (m == 0)
		return (-1);
	for (shift = NBBY*sizeof(unsigned long)/2, log = 0; shift; shift >>= 1)
		if (n = m>>shift) {
			log += shift;
			m = n;
		}
	return log;
}

/*
 *	Tell the VM system whether the given physical page actually
 *	exists.  This functions traverses the ALPHA memory descriptor
 *	backwards on the assumption that the console uses the first
 *	cluster, so there's no point in looking at the least
 *	interesting information until last.
 *
 *	The logic below uses the fact that memory descriptor clusters
 *	are ordered in ascending order of physical page number.
 */
boolean_t
pmap_valid_page(p)
	vm_offset_t p;
{
	extern struct rpb_mdt		*memdsc;
	register long			pfn = alpha_btop(p);
	register struct rpb_cluster	*cluster;
	register long			i, cloffset;

	for (i = memdsc->rpb_numcl - 1; i >= 0; --i) {
		cluster = &memdsc->rpb_cluster[i];
		if ((cloffset = pfn - cluster->rpb_pfn) < 0)
			continue;		       /* below this cluster */
		if (cloffset >= cluster->rpb_pfncount		/* not found */
		    || cloffset >= cluster->rpb_pfntested	/* untested  */
		    || cluster->rpb_usage == CLUSTER_USAGE_PAL)	/* reserved  */
			return FALSE;
		else						/* in bitmap */
			return (((long *)
				((vm_offset_t)cluster->rpb_va + hwrpb_offset))
				[cloffset >> 6] & (1L << (cloffset & 63))) != 0;
	}
	return FALSE;						/* not found */
}

int consmem; /* number of pages of console memory (rpb, PALcode, console sw) */

static void
setup_next_pfn(first_pfn, first_cluster, cluster_count)
	long first_pfn;
	struct rpb_cluster *first_cluster;
	long cluster_count;
{
	extern long		nfpfn;	  /* Initialized by this function. */
	extern struct rpb_cluster *memcl; /* Initialized by this function. */
	extern long             memlimit; /* set by user at boot */
	struct rpb_cluster 	*mcl;
	long i;

	mem_size = 0;
	nfpfn = first_pfn;
	for (i=0, mcl = first_cluster; i < cluster_count; ++i, ++mcl) {
		switch (mcl->rpb_usage) {
		case CLUSTER_USAGE_OS:
			/*
 			* If memlimit is non zero the user has specified
 			* it on the command line.
 			*
 			* we assume that there is only one CLUSTER_USAGE_OS
 			* so if the user wants to boot with less than the
 			* physical memory, only one mcl->rpb_pfncount gets
 			* updated.  If we ever have more then one, we'll
 			* need to re-think how memlimit gets used.
 			*/
			if(memlimit)
				mcl->rpb_pfncount = (memlimit-2)*PAGE_PER_MB;

			mem_size += alpha_ptob(mcl->rpb_pfncount);
			if (mcl->rpb_pfn <= nfpfn &&
			    mcl->rpb_pfn + mcl->rpb_pfncount > nfpfn)
				memcl = mcl;
			break;
		case CLUSTER_USAGE_PAL:
			consmem += (int)mcl->rpb_pfncount;
			break;
		}
	}
	avail_end = alpha_ptob(memcl->rpb_pfn + memcl->rpb_pfncount);
}

long	pgshift;	/* Set in pmap_bootstrap below. */
long	pgofset;	/* Set in pmap_bootstrap below. */
vm_offset_t unity_base; /* Set in pmap_bootstrap below. */

#define BOOT_vtopte(v)	((pt_entry_t *)(PGTBL_ADDR) + ((long)(v) >> pgshift))
#define level1_vpn(v)	(((vm_offset_t)(v) >> (3*pgshift - 6)) & (pgofset>>3))
#define level2_vpn(v)	(((vm_offset_t)(v) >> (2*pgshift - 3)) & (pgofset>>3))
#define level3_vpn(v)	(((vm_offset_t)(v) >> pgshift) & (pgofset>>3))

static unsigned int
next_pfn()
{
	extern long nfpfn;
	extern struct rpb_cluster *memcl;
	extern vm_offset_t hwrpb_addr;

	for (;; ++nfpfn) {
		if (nfpfn >= memcl->rpb_pfn + memcl->rpb_pfncount)
			do {
				struct rpb *rpb;
				struct rpb_mdt *memdsc;
			
				nfpfn = (++memcl)->rpb_pfn;
				rpb = (struct rpb *)hwrpb_addr;
				memdsc = (struct rpb_mdt *)((char *)rpb
							    + rpb->rpb_mdt_off);
				if (memcl >=
				    memdsc->rpb_cluster + memdsc->rpb_numcl)
					return NOTaPFN;
			} while (memcl->rpb_usage == CLUSTER_USAGE_PAL);
		if (pmap_valid_page(alpha_ptob(nfpfn)))
			break;
	}
	return nfpfn++;
}

/*
 * Add a page to the bootstrap map.
 */
vm_offset_t
map_bootspace(va, pfn, prot)
	vm_offset_t va;
	unsigned int pfn;
	long prot;
{
	pt_entry_t scratch;

	scratch = proto_sys_pte;
	scratch.pg_pfn = pfn;
	scratch.pg_prot = prot;
	BOOT_vtopte(va)->quadword = scratch.quadword;
	mtpr_tbis(va);
	return va;
}

/*
 * Add a page to the kernel map, while we are still in the boot
 * address space.
 *
 * This routine insures that all levels of page tables
 * are setup for the virtual address that is passed.
 *
 *	Inputs:
 *	    va		virtual address that needs to be mapped
 *	    out_1st	address of kernel 1st level page table
 *			in boot address space.
 *	    out_2nd	address to be used to access 2nd level page table.
 *	    out_3rd	address to be used to access 3rd level page table.
 *	    pte		copy of pte that will be put into kernel map.
 */
vm_offset_t
map_kernelspace(va, out_1st, out_2nd, out_3rd, pte)
	vm_offset_t va;
	pt_entry_t *out_1st, *out_2nd, *out_3rd;
	pt_entry_t pte;
{
        if (out_1st[level1_vpn(va)].quadword == 0) {
	    (void)map_bootspace(out_2nd, next_pfn(), PROT_KW);
	    bzero(out_2nd, page_size);
	    out_1st[level1_vpn(va)] = *BOOT_vtopte(out_2nd);
	} else {
	    (void)map_bootspace(out_2nd, out_1st[level1_vpn(va)].pg_pfn, PROT_KW);
	}
	
	if (out_2nd[level2_vpn(va)].quadword == 0) {
	    (void)map_bootspace(out_3rd, next_pfn(), PROT_KW);
	    bzero(out_3rd, page_size);
	    out_2nd[level2_vpn(va)] = *BOOT_vtopte(out_3rd);
	} else {
	    (void)map_bootspace(out_3rd, out_2nd[level2_vpn(va)].pg_pfn, PROT_KW);
	}
	out_3rd[level3_vpn(va)] = pte;
}


#ifndef PT_DEBUG
#define dump_pt_alpha(PFN)
#else /* PT_DEBUG */

#include <machine/xpr.h>
static struct reg_values pte_protection_codes[] = {
	{0x00, "NA"}, {0x01, "KR"}, {0x03, "UR"},
	{0x11, "KRW"}, {0x33, "URW"}, {0x13, "URKW"},
	{0, NULL}
};
static struct reg_desc pte_field_descriptors[] = {
	{0x0000000000000001L, 0, "valid", NULL, NULL},
	{0x0000000000000002L, 0, "for", NULL, NULL},
	{0x0000000000000004L, 0, "fow", NULL, NULL},
	{0x0000000000000008L, 0, "foe", NULL, NULL},
	{0x0000000000000010L, 0, "asm", NULL, NULL},
	{0x0000000000000060L, -5, "gh", "%d", NULL},
	{0x000000000000ff00L, -8, "prot", "%#02x", &pte_protection_codes[0]},
	{0x0000000000010000L, 0, "exec", NULL, NULL},
	{0x0000000000020000L, 0, "wire", NULL, NULL},
	{0x0000000000040000L, 0, "seg", NULL, NULL},
	{0xffffffff00000000L, -32, "pfn", "%#x", NULL},
	{0x0000000000000000L, 0, NULL, NULL, NULL}
};

static long
dump_pte_alpha(indent, pt, index)
	long		indent;
	pt_entry_t	*pt;
	long		index;
{
	pt_entry_t	*pte = pt + index;

	while (indent-- > 0)
		printf(">>");
	printf("%03x ", index);
	
	/*
	 * This is a sanity check.  If pte doesn't look real, print it
	 * as a hex number and return 1, indicating that it shouldn't
	 * be used to map the next level.
	 */
	if (pte->pg_soft || pte->pg_prot & ~PROT_UW || pte->pg_pfn > (1<<17)) {
		printf("0x%016lx\n", pte->quadword);
		return 1;
	} else {
#if 1
		printf("%r\n", pte->quadword, &pte_field_descriptors[0]);
#else
		printf(
	"v:%x for:%x fow:%x foe:%x asm:%x gh:%x prot:%02x exec:%x pfn:%08x\n",
		       pte->pg_v,
		       pte->pg_for,
		       pte->pg_fow,
		       pte->pg_foe,
		       pte->pg_asm,
		       pte->pg_gh,
		       pte->pg_prot,
		       pte->pg_exec,
		       pte->pg_pfn);
#endif
		return 0;
	}
}

static pt_entry_t *dpt_1st, *dpt_2nd, *dpt_3rd;

static void
dump_pt_alpha(pfn)
	unsigned int	pfn;
{
	register long	i, j, k;

	(void)map_bootspace(dpt_1st, pfn, PROT_KR);
	for (i = 0; i < PTES_PER_PAGE; ++i) {
		if (dpt_1st[i].quadword == 0L)
			continue;
		if (dump_pte_alpha(0, dpt_1st, i))
			continue;
		(void)map_bootspace(dpt_2nd, dpt_1st[i].pg_pfn, PROT_KR);
		for (j = 0; j < PTES_PER_PAGE; ++j) {
			if (dpt_2nd[j].quadword == 0L)
				continue;
			if (dump_pte_alpha(1, dpt_2nd, j))
				continue;
			(void)map_bootspace(dpt_3rd, dpt_2nd[j].pg_pfn,PROT_KR);
			for (k = 0; k < PTES_PER_PAGE; ++k) {
				if (dpt_3rd[k].quadword == 0L)
					continue;
				(void)dump_pte_alpha(2, dpt_3rd, k);
			}
		}
	}
}
#endif /* PT_DEBUG */

			
/*
 * Allocate a page for the boot PCB and the boot/restart kernel stack.  The
 * page is mapped at the virtual address of KERNELSTACK - page_size, which
 * is the page just below the top 32k of virtual space.  No mapping will
 * exist just below this page, so a stack overflow would not be able to
 * corrupt the rest of the system.  The top 32k is reserved because of its
 * easy accessibility with a negative offset off of the zero register (r31).
 * Eventually, the kernel_parameters data structure will be mapped there.
 */

vm_offset_t bootpcb_va;
vm_offset_t bootpcb_pa;

static void
setup_bootpcb(pt_1st, pt_2nd, pt_3rd)
	pt_entry_t	*pt_1st, *pt_2nd, *pt_3rd;
{
	unsigned int	pfn;
	struct pcb	*pcb;
	pt_entry_t	scratch;

	/* Allocate and initialize a physical page for boot pcb and stack. */
	pfn = next_pfn();
	bootpcb_va = (vm_offset_t)trunc_page(KERNELSTACK - sizeof(long));
	bootpcb_pa = (vm_offset_t)pfn << pgshift;
	pcb = (struct pcb *)PHYS_TO_KSEG(bootpcb_pa);
	bzero(pcb, page_size);
	pcb->pcb_ksp = KERNELSTACK;
	pcb->pcb_usp = NOTaVA;
	pcb->pcb_ptbr = BOOT_vtopte(pt_1st)->pg_pfn;

	/* Install the mapping for the above in the page tables being built. */
	scratch = proto_sys_pte;
	scratch.pg_asm = 0;
	scratch.pg_pfn = pfn;
	pt_3rd[level3_vpn(KERNELSTACK - sizeof(long))] = scratch;
}

static void
mark_for_scavenge(kseg_high)
	register vm_offset_t		kseg_high;
{
	extern struct scavenge_list	scavenge_info;
	extern vm_offset_t		unity_base, end_of_scavenge;
	vm_offset_t			kseg_low;

	/* Start at the beginning of the OS memory cluster. */
	kseg_low = (memcl->rpb_pfn << pgshift) + unity_base;
	scavenge_info.count = (kseg_high - kseg_low) >> pgshift;
	scavenge_info.kseg_start = kseg_low;
	end_of_scavenge = kseg_high;
}

void
remap_os(in_1st, in_2nd, in_3rd, out_1st, out_2nd, out_3rd, ptbr, rva, rpa_ptr)
	pt_entry_t *in_1st, *in_2nd, *in_3rd, *out_1st, *out_2nd, *out_3rd;
	long ptbr;
	vm_offset_t rva;	/* virtual address of rpb_software_data */
	long *rpa_ptr;		/* place to store physical address of rva */
{
	extern char start[], etext[];
	register long	i;
	vm_offset_t	run_va;

	/* Map the current (bootstrap) root page table. */
	(void)map_bootspace(in_1st, ptbr, PROT_KR);

	/* Map the kernel 2nd-level page table set up by bootstrap. */
	(void)map_bootspace(in_2nd, in_1st[level1_vpn(start)].pg_pfn, PROT_KR);

	/* Make a copy of the kernel 2nd-level page table and link to 1st. */
	(void)map_bootspace(out_2nd, next_pfn(), PROT_KW);
	bcopy(in_2nd, out_2nd, page_size);
	out_1st[level1_vpn(start)] = *BOOT_vtopte(out_2nd);

	/* Copy 3rd-level kernel page tables from the bootstrap map. */
	run_va = ((vm_offset_t)start >> (3*pgshift - 6) << (3*pgshift - 6))
		+ page_size - 1;
	for (i = 0; i < PTES_PER_PAGE; ++i) {
		register long j;

		if (in_2nd[i].quadword == 0L) {
			run_va += (1 << (2*pgshift - 3));
			continue;
		}
		(void)map_bootspace(in_3rd, in_2nd[i].pg_pfn, PROT_KR);
		(void)map_bootspace(out_3rd, next_pfn(), PROT_KW);
		for (j = 0; j < PTES_PER_PAGE; ++j, run_va += page_size) {
			register pt_entry_t	temp;
			
			temp = in_3rd[j];
			if (temp.quadword) {
				temp.pg_asm = 1;
#if KDEBUG
				temp.pg_prot = PROT_KW;
#else
				temp.pg_prot = In_kernel_text(run_va)
						? PROT_KR
						: PROT_KW;
#endif
				/*
				 * The test below checks to see if the pte
				 * we've come across will map the global
				 * kernel variable rpb_software_data.  If
				 * this is the one, we then calculate the
				 * corresponding physical address and store
				 * it in the rpb_software field of the rpb.
				 */
				if ((run_va ^ rva) < NBPG)
					*rpa_ptr = alpha_ptob(temp.pg_pfn) |
						(rva & PGOFSET);
				/*
				 * The test below checks to see if the pte
				 * we've come across will map the last page
				 * of kernel text.  If this is the one, we
				 * then calculate the corresponding physical
				 * address for "etext" and store it case we
				 * need to do a partial memory dump later.
				 */
				if ((run_va ^ (vm_offset_t)etext) < NBPG)
					pmap_physetext =
						alpha_ptob(temp.pg_pfn) |
						(((vm_offset_t)etext)&PGOFSET);

				/*
				 * Capture the (kseg) limit of boot memory
				 * scavenging.
				 */
				if ((run_va ^ END_OF_BOOT_TEXT) < page_size)
					mark_for_scavenge(PTETOPHYS(&temp)
							  + unity_base);
			}
			out_3rd[j] = temp;
		}
		out_2nd[i] = *BOOT_vtopte(out_3rd);
	}
}

vm_offset_t			/* physical address of first HWPCB */
pmap_bootstrap(ffpfn, ptbr, rpb_software_data_va, saved_physroot_ptr)
	long ffpfn, ptbr;		/* first available page frame */
	vm_offset_t rpb_software_data_va;
	vm_offset_t *saved_physroot_ptr;
{
	extern char start[], etext[];
	extern vm_offset_t hwrpb_addr;
	extern struct rpb_mdt *memdsc;
	extern struct rpb_cluster *memcl;
	extern int nsys_space_elems;
	extern vm_size_t page_size;
#ifndef VM_RESIDENT_SANE
typedef struct {
	queue_head_t	pages;
	int		count;
	decl_simple_lock_data(,lock)
} vm_page_bucket_t;
extern int kentry_count;
#endif /* VM_RESIDENT_SANE */
	
	struct rpb *rpb;
	struct rpb_crb *crb;
	pt_entry_t *out_1st;	/* 1st level page table on return */
	pt_entry_t *out_2nd, *out_3rd;
	pt_entry_t *pte;	/* general purpose pte pointer */
	char *next_addr;	/* an easily mapped virtual address */
	long i, j, sum, *lp1, *lp2;
	register vm_size_t nbpg;
	extern long pgofset;
	extern long pgshift;
	vm_offset_t high_water, boot_va, run_va;
	pt_entry_t scratch;
	unsigned int level1_pfn;

	/* Ritual opening */
	kernel_pmap = &kernel_pmap_store;

	/* Extract useful things from the HWRPB. */
	rpb = (struct rpb *)hwrpb_addr;
	crb = (struct rpb_crb *)((char *)rpb + rpb->rpb_crb_off);
	memdsc = (struct rpb_mdt *)((char *)rpb + rpb->rpb_mdt_off);
	nbpg = page_size = rpb->rpb_pagesize;
	vm_set_page_size();
	pgofset = nbpg - 1;
	pgshift = fls(nbpg);
	unity_base = ~0L << (4 * pgshift - 10);

	/* Allow other modules to allocate page-aligned space beyond bss. */
	i = log_bootstrap((ffpfn << pgshift) + unity_base);
	ffpfn += (i + pgofset) >> pgshift;
	
	/* Initialize the template used for making kernel PTEs. */
	scratch.quadword = (unsigned long)0;
	scratch.pg_v = 1;
	scratch.pg_asm = 1;
	if (alpha_independent_vm_exec_access)
		scratch.pg_foe = 1;
	else
		scratch.pg_exec = 1;
	scratch.pg_prot = PROT_NA;
	proto_kern_ptetab[VM_PROT_NONE] = scratch;
	scratch.pg_prot = PROT_KR;
	proto_kern_ptetab[VM_PROT_READ] = scratch;
	scratch.pg_prot = PROT_KW;
	proto_kern_ptetab[VM_PROT_WRITE] = scratch;
	proto_kern_ptetab[VM_PROT_READ | VM_PROT_WRITE] = scratch;
	scratch.pg_prot = PROT_KR;
	scratch.pg_exec = 1;
	scratch.pg_foe = 0;
	proto_kern_ptetab[VM_PROT_EXECUTE] = scratch;
	proto_kern_ptetab[VM_PROT_READ | VM_PROT_EXECUTE] = scratch;
	scratch.pg_prot = PROT_KW;
	proto_kern_ptetab[VM_PROT_WRITE | VM_PROT_EXECUTE] = scratch;
	proto_kern_ptetab[VM_PROT_ALL] = scratch;

	/* Initialize the templates used for making user PTEs. */
	scratch.quadword = (unsigned long)0;
	scratch.pg_v = 1;
	if (alpha_independent_vm_exec_access)
		scratch.pg_foe = 1;
	else
		scratch.pg_exec = 1;
	scratch.pg_prot = PROT_NA;
	proto_user_ptetab[VM_PROT_NONE] = scratch;
	scratch.pg_prot = PROT_UR;
	proto_user_ptetab[VM_PROT_READ] = scratch;
	scratch.pg_prot = PROT_UW;
	proto_user_ptetab[VM_PROT_WRITE] = scratch;
	proto_user_ptetab[VM_PROT_WRITE | VM_PROT_READ] = scratch;
	scratch.pg_prot = PROT_UR;
	scratch.pg_exec = 1;
	scratch.pg_foe = 0;
	proto_user_ptetab[VM_PROT_EXECUTE] = scratch;
	proto_user_ptetab[VM_PROT_EXECUTE | VM_PROT_READ] = scratch;
	scratch.pg_prot = PROT_UW;
	proto_user_ptetab[VM_PROT_EXECUTE | VM_PROT_WRITE] = scratch;
	proto_user_ptetab[VM_PROT_ALL] = scratch;

	/* Initialize the user segment pte proto. */
	proto_user_segpte.quadword = 0L;
	proto_user_segpte.pg_v = 1;
	proto_user_segpte.pg_seg = 1;
	proto_user_segpte.pg_prot = PROT_UR;
	proto_user_segpte.pg_exec = 1;
	/* Segment PTEs are assumed to map managed pages. */
	SET_fo_any(proto_user_segpte);

	/* Get scratch virtual addresses from the top of region 1 map. */
	next_addr = (char *)(BOOT_ADDR + (1L << (2*pgshift - 3)));
#define NEXT_VA() (next_addr -= nbpg)

#ifdef PT_DEBUG
	dpt_1st = (pt_entry_t *)NEXT_VA();
	dpt_2nd = (pt_entry_t *)NEXT_VA();
	dpt_3rd = (pt_entry_t *)NEXT_VA();
#endif /* PT_DEBUG */
	/* Dump the current map made by the console and primary bootstrap. */
	dump_pt_alpha(ptbr);

	/* Get a handle on physical memory resources. */
	setup_next_pfn(ffpfn, memdsc->rpb_cluster, memdsc->rpb_numcl);
	maxmem = alpha_btop(mem_size); /* number of pages of OS memory */
	physmem = alpha_btop(avail_end); /* end of physical memory (last+1) */
	pmap_physhwrpb = PTETOPHYS(BOOT_vtopte(HWRPB_ADDR));

	/* Allocate the first free physical page for 1st level PTEs. */
	level1_pfn = next_pfn();
	*saved_physroot_ptr = pmap_physroot = alpha_ptob(level1_pfn);

	/*
	 * Be prepared for the case where the kernel_pmap pointer is
	 * actually followed rather than simply compared to another pmap
	 * pointer.
	 */
	kernel_pmap->level1_pt = (pt_entry_t *)(pmap_physroot + unity_base);

	/* Create a self-mapping root page table for the next SWPCTX. */
	out_1st = (pt_entry_t *)map_bootspace(NEXT_VA(), level1_pfn, PROT_KW);
	bzero(out_1st, nbpg);
	out_1st[level1_vpn(BOOT_Selfmap)] = *BOOT_vtopte(out_1st);
	out_1st[level1_vpn(BOOT_Selfmap)].pg_asm = 0;

	/*
	 * Allocate some virtual addresses through which page tables can
	 * be written.  Then map all of physical memory 1-1 with low
	 * kernel address space.
	 */
	out_2nd = (pt_entry_t *)NEXT_VA();
	out_3rd = (pt_entry_t *)NEXT_VA();

	/*
	 * unity_base is the beginning of kseg.  Half of unity_base
	 * (as a signed quantity) is the beginning of seg1.  If the
	 * kernel starts in seg1, then make a map for it; otherwise,
	 * the kernel was linked into kseg and needs no page table.
	 *
	 * Load the physical address of the rpb_software_data structure
	 * into the reserved-for-software field of the rpb.  Recompute
	 * the rpb checksum as well.
	 */
	if ((vm_offset_t)start >= (vm_offset_t)((long)unity_base >> 1)) {
		remap_os(NEXT_VA(), NEXT_VA(), NEXT_VA(),
			 out_1st, out_2nd, out_3rd, ptbr,
			 rpb_software_data_va, &rpb->rpb_software);
	} else {
		rpb->rpb_software = rpb_software_data_va - unity_base;
		pmap_physetext = (vm_offset_t)etext - unity_base;
		mark_for_scavenge(END_OF_BOOT_TEXT & ~pgofset);
	}
	sum = 0;
	lp1 = (long *)rpb;
	lp2 = &rpb->rpb_checksum;
	while (lp1 < lp2)
		sum += *lp1++;
	*lp2 = sum;

	/*
	 * Assert the kernel virtual address limits.
	 */
	virtual_avail = round_page(VM_MIN_KERNEL_ADDRESS);
	virtual_end = trunc_page(VM_MAX_KERNEL_ADDRESS);

	/*
	 * We need to grab an additional two pages for the prom
	 * stack and the temp buffer virtual address
	 */
	virtual_avail += alpha_ptob(crb->rpb_mapped_pages + 2);

	/*
	 * reserve kernel virtual space for drivers that need 
	 * to map things before the vm/pmap subsystems are up
	 */
	if (nsys_space_elems) {
		extern struct sys_space sys_space[];
		for (i = 0; i < nsys_space_elems; i++) {
			*sys_space[i].s_va = virtual_avail;
			virtual_avail += sys_space[i].s_size;
			virtual_avail = round_page(virtual_avail);
		}
	}

	/*
	 * Allocate the second level page table for the kernel heap,
	 * etc. if it isn't already allocated.
	 */
	if (out_1st[level1_vpn(virtual_avail)].quadword == 0) {
		(void)map_bootspace(out_2nd, next_pfn(), PROT_KW);
		bzero(out_2nd, nbpg);
		out_1st[level1_vpn(virtual_avail)]
			= *BOOT_vtopte(out_2nd);
	}

	/* 
	 * Now allocate third level page tables to cover virtual heap
	 * allocation during startup.
	 */
	for (run_va = virtual_avail, pte = &out_2nd[level2_vpn(run_va)]
	     ;run_va < 
#ifndef jmfix
virtual_avail + (1<<26)	/* 64MB, 8 2nd-level PTEs.  Crude! Effective? */
#else /* jmfix */
virtual_avail
#ifndef VM_RESIDENT_SANE
/*
 * This is a crude and, of course, somewhat generous estimate of the
 * amount of virtual address space consumed in vm_startup_page.
 * vm_startup_page assumes flat page tables of a sort Alpha doesn't
 * have.  This preallocates the page table pages necessary to support
 * the assumption made in the "machine independent" code that it can
 * simply increment through pages from avail_start to avail_end.
 */
+ (sizeof(vm_page_bucket_t) << (fls(alpha_btop(avail_end) - nfpfn) + 1))
+ sizeof(struct vm_page) * (alpha_btop(avail_end) - nfpfn)
+ alpha_round_page(128 * sizeof(struct zone))
+ alpha_round_page(10 * sizeof(struct vm_map))
+ alpha_round_page(kentry_count*sizeof(struct vm_map_entry))
#endif /* VM_RESIDENT_SANE */
#endif /* jmfix */
	     ;++pte, run_va += (1L << (2*pgshift - 3))) {
		(void)map_bootspace(out_3rd, next_pfn(), PROT_KW);
		bzero(out_3rd, nbpg);
		*pte = *BOOT_vtopte(out_3rd);
	}

	/*
	 * Allocate and map a page table for the HWRPB as seen through
	 * the kernel address space.  If KERNEL_PARAMETERS are mapped
	 * (variable page size), they are mapped by the last entries
	 * in the page table.  This must be done before the call to
	 * setup_bootpcb() just below.
	 */
	(void)map_bootspace(out_3rd, next_pfn(), PROT_KW);
	bzero(out_3rd, nbpg);

	/*
	 * Make a boot PCB, allocate a kernel stack, and link them to the
	 * page table we are building.  The kernel virtual address space
	 * used is just below the top 32K, so the page table page just
	 * allocated above will cover this mapping.
	 */
	setup_bootpcb(out_1st, out_2nd, out_3rd);

	/*
	 * Compute how much of the HWRPB and related data to map.  In
	 * addition to the HWRPB proper this includes the memory bit
	 * maps, the CTB, CRB,
	 */
	high_water = HWRPB_ADDR;
	for (i = 0; i < memdsc->rpb_numcl; ++i) {
		struct rpb_cluster *cl = &memdsc->rpb_cluster[i];

		if (cl->rpb_va == 0 || cl->rpb_usage == CLUSTER_USAGE_PAL)
			continue;
		if (cl->rpb_va > high_water)	
			high_water = cl->rpb_va
					 + ((cl->rpb_pfncount+7) >> 3) - 1;
	}

	for (boot_va = HWRPB_ADDR, run_va = OSF_HWRPB_ADDR;
	     boot_va < high_water;
	     boot_va += nbpg, run_va += nbpg) {
		if ((long)run_va >= (long)KERNEL_PARAMETERS) {
			printf("HWRPB overran system space.\n");
			halt();
		}
		out_3rd[level3_vpn(run_va)] = *BOOT_vtopte(boot_va);
		out_3rd[level3_vpn(run_va)].pg_prot = PROT_KW;
		out_3rd[level3_vpn(run_va)].pg_asm = 1;
	}

	/* Initialize vm globals dependent on page size. */
	init_vm_parameters(pgshift, out_3rd);

	/*
	 * Assuming that KERNEL_PARAMETERS, and the OSF_HWRPB_ADDR are
	 * all within the last 8MB of system space, one second level pte
	 * does them all.
	 */
	out_2nd[level2_vpn(OSF_HWRPB_ADDR)] = *BOOT_vtopte(out_3rd);

	/* must do some console callbacks BEFORE proceeding past here... */
	perform_console_callbacks();

	/* Save two pages for the prom stack and temp buffer page */
	run_va = VM_MIN_KERNEL_ADDRESS + alpha_ptob(2);

	/* Invoke prom fixup callback to remap rpb data structures */
	prom_fixup(run_va); /* run_va is new stack top and 1st callback arg */

	/* Map the prom stack */
	scratch = proto_sys_pte;
	scratch.pg_pfn = next_pfn();
	scratch.pg_prot = PROT_KW;
	map_kernelspace(run_va - 1, out_1st, out_2nd, out_3rd, scratch);

	/* Finish remapping prom space */
	crb->rpb_va_disp += (long)run_va - crb->rpb_map[0].rpb_virt;
	crb->rpb_va_fixup = (long)run_va - crb->rpb_map[0].rpb_virt;
	prom_init(); /* reassign prom callback dispatch vector */
	for (i = 0; i < crb->rpb_num; i++) {
		boot_va = crb->rpb_map[i].rpb_virt;
		crb->rpb_map[i].rpb_virt = run_va;
		for (j = 0; j < crb->rpb_map[i].rpb_pgcount; j++) {
			map_kernelspace(run_va, out_1st, out_2nd, out_3rd,
				*BOOT_vtopte(boot_va));
			boot_va += alpha_ptob(1);
			run_va += alpha_ptob(1);
		}
	}

	/*
	 * next_pfn() uses the globals hwrpb_offset and hwrbp_addr.
	 * From here until after the next CALL_PAL SWPCTX no calls to
	 * the function next_pfn() can be made.
	 */
	hwrpb_offset = (vm_offset_t)OSF_HWRPB_ADDR - (vm_offset_t)HWRPB_ADDR;
	hwrpb_addr = OSF_HWRPB_ADDR;

	avail_start = alpha_ptob(nfpfn);

	/* Change globals to their new, soon-to-be-mapped addresses. */
	memdsc = (struct rpb_mdt *)((char *)memdsc + hwrpb_offset);
	memcl = (struct rpb_cluster *)((char *)memcl + hwrpb_offset);

	return bootpcb_pa;
}

void
pmap_asn_init()
{
	register long			i, j;
	register struct asn_state *	state;

	if (pmap_maxasn < 0)
		pmap_maxasn = ((struct rpb *)hwrpb_addr)->rpb_maxasn;
	if (pmap_maxasn > PMAP_MAXASN)
		pmap_maxasn = PMAP_MAXASN;
	else if (pmap_maxasn < 0)
		pmap_maxasn = 0;
	for (i = 0; i < NCPUS; ++i) {
		state = &asns_on[i];
		queue_init(&state->free_asns);
		queue_init(&state->active_asns);

		state->cpu_asn[0].owner = kernel_pmap;
		for (j = 1; j <= pmap_maxasn; ++j)
			enqueue(&state->free_asns, &state->cpu_asn[j]);
	}
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
	extern	int nproc;

	/*
	 * Notify upper level segment initialization code.
	 */

#ifdef	PMAP_SEGMENTATION
	u_seg_init(VM_MIN_ADDRESS, VM_MAX_ADDRESS, 
			pmap_seg_size, pmap_seg_mask);

	/*
	 * Create the segment zone
	 */

	pmap_seg_zone = zinit(sizeof (struct pmap_seg),
		nproc * sizeof (struct pmap_seg), 4096, "pmap segments");

#endif	PMAP_SEGMENTATION

	/*
	 *	How many phys pages will we be accounting for
	 */
	npages = alpha_btop(phys_end - phys_start);

	/*
	 *	Create our submap to allocate pages from
	 *
	 *	Take enough virtual to cover all phys mem if we
	 *	don't have much of it (pmaxen), but no more than
	 *	512 meg otherwise.  This should be plenty.  (This
	 *	limit must be less than 2 gig.  Specifically, the
	 *	delta between the constants VM_MIN_KERNEL_ADDRESS
	 *	and VM_MAX_KERNEL_ADDRESS imply an upper bound.)
	 */
#define _PMAP_MAX_VIRT 0x20000000
	if ((vsize = npages * PAGE_SIZE) > _PMAP_MAX_VIRT)
		vsize = _PMAP_MAX_VIRT;
	pmap_submap = kmem_suballoc(kernel_map, &pmap_vlow, &pmap_vhigh,
		vsize, FALSE);
	/*
	 *	Allocate memory for the pv_list headers.
	 */
	s = (vm_size_t) (sizeof(struct pv_entry) * npages);	/* pv_list */
	s = round_page(s);
	addr = (vm_offset_t) kmem_alloc(pmap_submap, s);
	if (addr == 0)
		panic("pmap_init: failed to allocate pv_head_table");

	pv_head_table = (union pv_list_head *)addr;

	/*
	 *	Create the zones of physical maps
	 *	and physical_to_virtual entries
	 */
	s = (vm_size_t) sizeof(struct pmap);
	pmap_zone = zinit(s, 400*s, 4096, "pmap");

	s = (vm_size_t) sizeof(struct pv_entry);
	pv_list_zone = zinit(s, 100000*s, 4096, "pv_list");

	/* Initialize the per-processor ASN state. */
	pmap_asn_init();

	/* Initialize /dev/zero_page. */
	init_zero_page();

	lw_init();
	/*
	 * Enable bookkeeping of the modify list
	 */
	vm_first_phys = phys_start;
	vm_last_phys =  phys_end;

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
	register pmap_t			map;
	register pmap_statistics_t	stats;
	pt_entry_t			scratch;

	/* A software use-only map doesn't even need a map */
	if (size != 0)
		return PMAP_NULL;

	INC_PCR_COUNT;

	ZALLOC(pmap_zone, map, pmap_t);
	if (map == PMAP_NULL)
		panic("pmap_create: cannot allocate a pmap");

	/* Initialize the various fields */
	map->ref_count = 1;
	simple_lock_init(&map->lock);
	map->coproc_tbi = (int (*)())0;
	map->map_asn[cpu_number()] = 0; /* defer assignment of a real ASN */
	/* Allocate a root page table. */
	map->level1_pt = (pt_entry_t *)get_free_ptepage();

	/* Map system space in the new page table. */
	bcopy(&Root_pt[PTES_PER_PAGE/2], &map->level1_pt[PTES_PER_PAGE/2],
	      NBPG/2);

	/* Have the page table map itself. */
	scratch = proto_sys_pte;
	scratch.pg_pfn = KSEG_TO_PHYS(map->level1_pt) >> PGSHIFT;
	scratch.pg_asm = 0;
	map->level1_pt[LEVEL1_PT_OFFSET(Selfmap)] = scratch;
		

	/*
	 *	Initialize statistics.
	 */

	stats = &map->stats;
	stats->resident_count = 0;
	stats->wired_count = 0;
	stats->resident_text = 0;

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
	register pt_entry_t	*pte1, *pte2;
	struct asn_data		*asn_d;
	long			s, c, i;

	if (map == PMAP_NULL)
		return;

	s = splvm();
	lock_pmap(map);
	c = --map->ref_count;
	unlock_pmap(map);
	splx(s);

	if (c != 0)
		return;

	/* Walk the page table freeing page table pages. */
	for (pte1 = map->level1_pt + PTES_PER_PAGE/2 - 1;
	     pte1 >= map->level1_pt; --pte1) {
		if (pte1->quadword == 0L)
			continue;
		for (pte2 = (pt_entry_t *)(PHYS_TO_KSEG(PTETOPHYS(pte1))),
		     i = 0; i < PTES_PER_PAGE; ++pte2, ++i) {
			if (pte2->quadword == 0L)
				continue;
			put_free_ptepage(PHYS_TO_KSEG(PTETOPHYS(pte2)));
		}
		put_free_ptepage(PHYS_TO_KSEG(PTETOPHYS(pte1)));
	}
	put_free_ptepage(map->level1_pt);

	/* Free address space numbers */
	if (pmap_maxasn > 0) {
#if NCPUS==1
		if (c = map->map_asn[cpu_number()]) {
			asn_d = &asns_on[cpu_number()].cpu_asn[c];
			remqueue(&asns_on[cpu_number()].active_asns, asn_d);
			enqueue(&asns_on[cpu_number()].free_asns, asn_d);
		}
#else
	    /* appropriate mp locking will need to be added around asn stuff */
	    for (i = 0; i < NCPUS; i++) {
		if (c = map->map_asn[i]) {
			asn_d = &asns_on[cpu_number()].cpu_asn[c];
			remqueue(&asns_on[cpu_number()].active_asns, asn_d);
			enqueue(&asns_on[cpu_number()].free_asns, asn_d);
		}
	    }
#endif /* NCPUS==1 */
	}

	ZFREE(pmap_zone, (vm_offset_t)map);
}

/*
 *	Add a reference to the specified pmap.
 */
void
pmap_reference(map)
	pmap_t	map;
{
	unsigned s;

	if (map != PMAP_NULL) {
		s = splvm();
		map->ref_count++;
		splx(s);
	}
}

	/* pmap_pt_access:
	 * is an elaborated pmap_pte for Alpha.  It returns (through
	 * pointer arguments) the first-, second-, and third-level PTEs
	 * corresponding to a pmap and virtual address pair.  It also
	 * indicates (through its return value) whether the PTEs are in
	 * the map for the current context (Selfmap) or they have been
	 * mapped to the window used to access other contexts
	 * (Othermap).
	 */
static boolean_t
pmap_pt_access(map, v, pte1p, pte2p, ptep)
	pmap_t		map;
	vm_offset_t	v;
	pt_entry_t	**pte1p, **pte2p, **ptep; /* return values */
{
	boolean_t	other;

	*ptep = pmap_pte(map, v);
	other = in_Othermap(*ptep);
	*pte2p = vtopte(*ptep + (other ? (Selfmap - Othermap) : 0))
			      + (other ? (Othermap - Selfmap) : 0);
	*pte1p = vtopte(*pte2p + (other ? (Selfmap - Othermap) : 0))
			       + (other ? (Othermap - Selfmap) : 0);

	/* If some other context, map it and flush non-system PTEs. */
	if (other) {
		register pt_entry_t	*window;

		window = &Root_pt[LEVEL1_PT_OFFSET(Othermap)];
		*window = map->level1_pt[LEVEL1_PT_OFFSET(Selfmap)];
		if (window->pg_v == 0 || window->pg_prot != PROT_KW) {
			printf("0x%l016x/2X  %l016x  %l016x\r\n", window,
			       window[0].quadword, window[1].quadword);
			panic("pmap_pt_access");
		}
		mtpr_tbiap();
	}
	return other;
}

/*
 *	Remove the given range of addresses from the specified map.  It is 
 *	assumed that start and end are rounded to the VM page size.
 */

boolean_t
pmap_remove_range(map, start, end, collect)
	register pmap_t		map;
	vm_offset_t		start;
	register vm_offset_t	end;
	boolean_t collect;		/* Stop if wired page is found */
{
	register vm_offset_t	va, pa;
	register pt_entry_t	*end2, *end3;
	pt_entry_t		*pte1, *pte2, *pte3;
	boolean_t		other;
	int			status;

	if (map == PMAP_NULL)
		return;

	status = 0;
	other = pmap_pt_access(map, start, &pte1, &pte2, &pte3);

	for (va = start; va < end; ++pte1) {
		end2 = (pt_entry_t *)(alpha_trunc_page(pte2) + NBPG);
		if (pte1->pg_v == 0) {
			va += (end2 - pte2) << (2 * PGSHIFT - 3);
			pte2 = end2;
			continue;
		}
		for (; va < end && pte2 < end2; ++pte2) {
			end3 = (pt_entry_t *)(alpha_trunc_page(pte3) + NBPG);
			if (pte2->pg_v == 0) {
				va += (end3 - pte3) << PGSHIFT;
				pte3 = end3;
				continue;
			}
			for (; va < end && pte3 < end3; ++pte3, va += NBPG) {
				if ((pa = PTETOPHYS(pte3)) == 0)
					continue;
				if(pte3->pg_wire ||
				   (map->coproc_tbi &&
					 pmap_coproc_invalidate(map,
						va, NBPG))) {
				     if (collect) {
					status = 1;
					goto collect_abort;
				     }
				     else
				        panic("pmap_remove_range, page wired");
				 }

				if (managed(pa)) {
					delete_pv_entry(pa, map, va);
					map->stats.resident_count--;
					if (MAPS_TEXT(pte3))
						map->stats.resident_text--;
				}
				pte3->quadword = 0L;
				if (!other)
					mtpr_tbis(va);
			}
		}
	}
collect_abort:
	pmap_tbsync(map, start, end-start);
	return status;
}

#define PMAP_REMOVE_INCR (1024*1024) 

void
pmap_remove(map, begin, end)
	register pmap_t		map;
	vm_offset_t		begin;
	vm_offset_t		end;
{
	unsigned		s;
	vm_offset_t		start, stop;

	start = trunc_page(begin);

	while (start < end) {
		stop = start + PMAP_REMOVE_INCR;
		if (stop > end)
			stop = end;
		s = splvm();
		BEGIN_LOCKS_IN_ORDER();
		lock_pmap(map);
		pmap_remove_range(map, start, stop, FALSE);
		unlock_pmap(map);
		END_LOCKS_IN_ORDER();
		splx(s);
		start = stop;
	}
}

/*
 *	Set the physical protection on the specified range of this map as
 *	requested.
 */
void
pmap_protect(map, start, end, prot)
	register pmap_t map;
	register vm_offset_t start;
	vm_offset_t	end;
	vm_prot_t	prot;
{
	long		s;	/* saved IPL */
	register pt_entry_t	scratch;
	register vm_offset_t	va;
	register pt_entry_t	*end2, *end3;
	pt_entry_t		*pte1, *pte2, *pte3;
	boolean_t		other;
	unsigned int		a_prot;
	boolean_t		a_exec;

	if (map == PMAP_NULL)
		return;

	if ((prot & VM_PROT_READ) == 0) {		/* No access */
		pmap_remove(map, start, end);
		return;
	}

	ASSERT((unsigned int)prot <= VM_PROT_ALL);
	scratch = (map == kernel_pmap) ?
		proto_kern_ptetab[prot] : proto_user_ptetab[prot];
	a_prot = scratch.pg_prot;
	a_exec = scratch.pg_exec;
	s = splvm();
	lock_pmap(map);
		
	other = pmap_pt_access(map, start, &pte1, &pte2, &pte3);

	for (va = start; va < end; ++pte1) {
		end2 = (pt_entry_t *)(alpha_trunc_page(pte2) + NBPG);
		if (pte1->pg_v == 0) {
			va += (end2 - pte2) << (2 * PGSHIFT - 3);
			pte2 = end2;
			continue;
		}
		for (; va < end && pte2 < end2; ++pte2) {
			end3 = (pt_entry_t *)(alpha_trunc_page(pte3) + NBPG);
			if (pte2->pg_v == 0) {
				va += (end3 - pte3) << PGSHIFT;
				pte3 = end3;
				continue;
			}
			for (; va < end && pte3 < end3; ++pte3, va += NBPG) {
				scratch.quadword = pte3->quadword;
				scratch.pg_prot = a_prot;
				scratch.pg_exec = a_exec;
				if (managed(PTETOPHYS(&scratch)))
					SET_fo_any(scratch);
				pte3->quadword = scratch.quadword;
				if (!other && scratch.pg_v)
					mtpr_tbis(va);
			}
		}
	}
	pmap_tbsync(map, start, end-start);
	unlock_pmap(map);
	splx(s);
	if (prot & VM_PROT_EXECUTE)
		imb();
}

/*
 * pmap_map maps kernel pages after pmap_bootstrap and before pmap_init.
 */
vm_offset_t
pmap_map(virt, start, end, prot)
	vm_offset_t	virt;		/* Starting virtual address.	*/
	vm_offset_t	start;		/* Starting physical address.	*/
	vm_offset_t	end;		/* Ending physical address.	*/
	int		prot;		/* Machine indep. protection.	*/
{
	pt_entry_t 		scratch;
	pt_entry_t     		*pte;

	ASSERT((unsigned int)prot <= VM_PROT_ALL);
	scratch = proto_kern_ptetab[prot];

	while (start < end) {
		scratch.pg_pfn = alpha_btop(start);
		pte = vtopte(virt);
		pte->quadword = scratch.quadword;
		mtpr_tbis(virt);
		virt += PAGE_SIZE;
		start += PAGE_SIZE;
	}

	return virt;
}

/*
 *	Routine:	pmap_remap_prom_addr
 *	Function:	If the address passed in is not in 32 bit
 *			address space, remap the physical page so that
 *			it is in 32 bit address space.
 */
vm_offset_t
pmap_remap_prom_addr(addr)
	vm_offset_t	addr;
{
	pt_entry_t		scratch;
	pt_entry_t		*pte, *npte;
        /* 
	 * If the address is already in 32 bit address
	 * space then it is OK for the prom, else we need
	 * to remap it.
	 */
	if ((addr <= 0x7fffffff) || (addr >= 0xffffffff80000000))
	    return (addr);

	pte = vtopte(addr);
	npte = vtopte(VM_MIN_KERNEL_ADDRESS);

	scratch = proto_sys_pte;
	scratch.pg_prot = PROT_KW;
	scratch.pg_pfn = pte->pg_pfn;
	
	npte->quadword = scratch.quadword;
	mtpr_tbis(VM_MIN_KERNEL_ADDRESS);

	return(VM_MIN_KERNEL_ADDRESS + (addr & pgofset));
}

/*
 *	Routine:	pmap_change_wiring
 *	Function:	Change the wiring attribute for a map/virtual-address
 *			pair.
 *	In/out conditions:
 *			The mapping must already exist in the pmap.
 */

void
pmap_change_wiring(register pmap_t map, 
	vm_offset_t v, 
	boolean_t wired)
{
	pt_entry_t *pte1, *pte2, *pte;
	register pt_entry_t *epte;
	register int s;
	boolean_t other;

	s = splvm();
	BEGIN_LOCKS_IN_ORDER();
	lock_pmap(map);
	

	other = pmap_pt_access(map, v, &pte1, &pte2, &pte);
	if (!pte1->pg_v || !pte2->pg_v || !pte->pg_v)
		panic("pmap_change_wiring: not valid va");

	/*
	 * Only changing software state.  Since MMU doesn't modify
	 * pte we're ok and don't concern ourselves with tbis. 
	 */

	for (epte = pte + alpha_btop(PAGE_SIZE); pte < epte; pte++) 
		pte->pg_wire = wired;

	unlock_pmap(map);
	END_LOCKS_IN_ORDER();
	(void) splx(s);
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
	register pmap_t		map;
	register vm_offset_t	v;
	register vm_offset_t	p;
	vm_prot_t		prot;
	boolean_t		wired;
	vm_prot_t		access;
{
	register pt_entry_t	scratch;
	pt_entry_t		*pte, *pte2, *pte1;
	vm_offset_t		physaddr;
	pv_entry_t		new_pv_entry, locked_pv_entry;
	vm_offset_t		free_pt_page2, free_pt_page3;
	vm_offset_t		pt_page2, pt_page3;
	int			s;	/* for saving and restoring IPL */
	boolean_t		other;
	boolean_t		same_phys = FALSE;
	union pv_list_head	*head;


	if (map == PMAP_NULL)
		return;
	if (pv_head_table == PV_LIST_HEAD_NULL) {	/* pmap system up ? */
		pmap_map(v, p, p + PAGE_SIZE, prot);
		return;
	}

	free_pt_page2 = free_pt_page3 = (vm_offset_t)0;
	new_pv_entry = PV_ENTRY_NULL;
	locked_pv_entry = PV_ENTRY_NULL;
	do {
		s = splvm();
		BEGIN_LOCKS_IN_ORDER();
		lock_pmap(map);
		other = pmap_pt_access(map, v, &pte1, &pte2, &pte);

		if (pte1->pg_v)
			pt_page2 = (vm_offset_t)0;
		else if (!(pt_page2 = free_pt_page2)) {
			/*
			 * If we get here, we need to allocate a page
			 * for 2nd-level ptes, and thus we might need
			 * to sleep.  So we must release our spin locks,
			 * call get_free_ptepage(), save the allocated
			 * page, and restart page table walk under the
			 * appropriate locks.  We'll check for getting
			 * another pte page for 3rd-level ptes as well.
			 */
			unlock_pmap(map);
			END_LOCKS_IN_ORDER();
			(void)splx(s);
			free_pt_page2 = get_free_ptepage();
			if (!free_pt_page3)
				free_pt_page3 = get_free_ptepage();
			continue;
		} else {
			free_pt_page2 = (vm_offset_t)0;
			scratch = proto_sys_pte;
			scratch.pg_pfn = KSEG_TO_PHYS(pt_page2) >> PGSHIFT;
			scratch.pg_asm = IS_SYS_VA(v);
			*pte1 = scratch;
			if (!other)
				mtpr_tbisd(pte2);
		}
		if (pte2->pg_v)
			pt_page3 = (vm_offset_t)0;
		else if (!(pt_page3 = free_pt_page3)) {
			/*
			 * If we get here, we need to allocate a page
			 * for 3rd-level ptes, and thus we might need
			 * to sleep.  So we must release our spin locks,
			 * call get_free_ptepage(), save the allocated
			 * page, and restart page table walk under the
			 * appropriate locks.
			 */
			unlock_pmap(map);
			END_LOCKS_IN_ORDER();
			(void)splx(s);
			free_pt_page3 = get_free_ptepage();
			continue;
		} else {
			free_pt_page3 = (vm_offset_t)0;
			scratch = proto_sys_pte;
			scratch.pg_pfn = KSEG_TO_PHYS(pt_page3)>>PGSHIFT;
			scratch.pg_asm = IS_SYS_VA(v);
			*pte2 = scratch;
			if (!other)
				mtpr_tbisd(pte);
		}

	/*
	 * Get a pv_entry, if needed.  zalloc may re-enter pmap_enter by
	 * way of a fault; it may sleep in the process of servicing the
	 * fault.  Therefore, calling zalloc must be preceded by giving
	 * up locks and followed retaking the locks and reevaluating the
	 * resources protected by those locks.
	 */
		if (!managed(p))
			break;
		physaddr = PTETOPHYS(pte);
		if (same_phys = (physaddr == p))
			break;
		if (physaddr)
			pmap_remove_range(map, v, (v + PAGE_SIZE), FALSE);
		if ((locked_pv_entry = grab_pv_head(p)) == PV_ENTRY_NULL)
			if (new_pv_entry) {
				locked_pv_entry
					= insert_pv_entry(p, new_pv_entry);
				new_pv_entry = PV_ENTRY_NULL;
			} else {
				unlock_pvh(pa_to_pvh(p));
				unlock_pmap(map);
				END_LOCKS_IN_ORDER();
				(void)splx(s);
				new_pv_entry = (pv_entry_t)zalloc(pv_list_zone);
			}
	} while (locked_pv_entry == PV_ENTRY_NULL);

	/* An infrequent but possible occurence. */
	if (new_pv_entry)
		zfree(pv_list_zone, (vm_offset_t)new_pv_entry);

	/* Fill and unlock the pv_entry, if this is a new mapping. */
	if (!same_phys) {
		if (managed(p))
			fill_pv_entry(p, locked_pv_entry, map, v);
		map->stats.resident_count++;
		if (IS_TEXT(prot))
			map->stats.resident_text++;
	}

	ASSERT((unsigned int)prot <= VM_PROT_ALL);
	scratch = (map == kernel_pmap) ?
		proto_kern_ptetab[prot] : proto_user_ptetab[prot];
	scratch.pg_pfn = alpha_btop(p);
	scratch.pg_wire = wired;

	/*
	 * Tracking of page reference and modify bits is done through
	 * the 3 fault-on bits in the pte.  If an access is made while
	 * the corresponding bit is set, a memory management fault will
	 * cause pmap_fault_on() to be invoked.  If bits are set in our
	 * "access" arg, then we will perform the necessary handling now.
	 * The tracking of reference and modify bits is only performed for
	 * pages which are "managed".
	 */
	if (managed(p)) {
		SET_fo_any(scratch);
		if ((access &= prot) & VM_PROT_READ) {
			/* perform pre-access handling to avoid a 2nd fault */
			head = pa_to_pvh(p);
			lock_pvh(head);
			set_reference_bit(head);
			CLEAR_for(scratch);
			if (access & VM_PROT_WRITE) {
				head->bit.modify = 1;
				scratch.pg_fow = 0;
			}
			if ((access & VM_PROT_EXECUTE) && scratch.pg_exec)
				scratch.pg_foe = 0;
			unlock_pvh(head);
		}
	}
	scratch.pg_asm = IS_SYS_VA(v);
	pte->quadword = scratch.quadword;
	mtpr_tbis(v);

	unlock_pmap(map);
	END_LOCKS_IN_ORDER();
	splx(s);

	if (free_pt_page3)
		put_free_ptepage(free_pt_page3);
	if (free_pt_page2)
		put_free_ptepage(free_pt_page2);
	if (prot & VM_PROT_EXECUTE)
		imb();
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
	register pmap_t	map;
	vm_offset_t	va;
{
	pt_entry_t	*pte, *pte2, *pte1;
	vm_offset_t   	pa = 0;		/* Failure return */
	unsigned	s;
	int             l1index, l2index, l3index;
	pt_entry_t      *l1_pte, *l2_pte, *l3_pte;
	vm_offset_t     phys = 0;

	INC_EXT_COUNT;

	s = splvm();
	lock_pmap(map);

	(void)pmap_pt_access(map, va, &pte1, &pte2, &pte);
	if (pte1->pg_v && pte2->pg_v && pte->pg_v)
		pa = PTETOPHYS(pte) + (va & PGOFSET);

#ifdef PTE_WALK_DEBUG

	/* just verifying the algorithm */

	l1index = LEVEL1_PT_OFFSET(va);
	l2index = LEVEL2_PT_OFFSET(va);
	l3index = LEVEL3_PT_OFFSET(va);

	l1_pte = map->level1_pt + l1index;
	if (l1_pte->pg_v) {
	  l2_pte = (pt_entry_t *)(PHYS_TO_KSEG(PTETOPHYS(l1_pte)+(8*l2index)));
	  if(l2_pte->pg_v) {
	    l3_pte = (pt_entry_t *)(PHYS_TO_KSEG(PTETOPHYS(l2_pte)+(8*l3index)));
	    if(l3_pte->pg_v) {
	      phys = PTETOPHYS(l3_pte) + (va & page_mask);
	    }
	  }
	}
	
	if (pa != phys)
	  panic("pmap_extract1 failure");

#endif
	unlock_pmap(map);
	splx(s);

	return pa;
}

/*
 *	Routine:	pmap_resident_extract
 *	Function:
 *		Extract the physical page address associated with the
 *		given virtual address and pmap.  The address includes
 *		the offset within a page.  This routine does not lock
 *		the pmap; it is intended to be called only by device
 *		drivers or copyin/copyout routines that know the
 *		buffer whose address is being translated is
 *		memory-resident.
 */
vm_offset_t
pmap_resident_extract(map, va)
	pmap_t		map;
	vm_offset_t	va;
{
	pt_entry_t	*pte;

	INC_EXT_COUNT;

	pte = pmap_pte(map, va);
	return (in_Othermap(pte) || pte->pg_v == 0)
		? (vm_offset_t)0
		: (PTETOPHYS(pte) + (va & PGOFSET));
}

/*
 *	Routine:	pmap_access
 *	Function:
 *		Returns whether there is a valid mapping for the
 *		given virtual address stored in the given physical map.
 */
boolean_t
pmap_access(map, va)
	pmap_t		map;
	vm_offset_t	va;
{
	INC_ACC_COUNT;

	return pmap_extract(map, va) ? TRUE : FALSE;
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
#endif	/* 0 */

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
pmap_collect(register pmap_t map)
{
	register pt_entry_t *pte1, *pte2, *epte1, *epte2;
	vm_offset_t l2ptpage, l3ptpage, ptpages;
	register int s, i;
	register pmap_seg_t	segp;
	register vm_offset_t start;

	if (map == PMAP_NULL ||  PAGE_SIZE != ALPHA_PGBYTES) 
		return;
	else if (map == kernel_pmap) 
		panic("pmap_collect on kernel_pmap");

	l2ptpage = l3ptpage = ptpages = (vm_offset_t) 0;

	s = splvm();
	BEGIN_LOCKS_IN_ORDER();
	lock_pmap(map);

	for (pte1 = map->level1_pt + LEVEL1_PT_OFFSET(VM_MIN_ADDRESS),
		epte1 = map->level1_pt + LEVEL1_PT_OFFSET(VM_MAX_ADDRESS);
		pte1 < epte1; pte1++) {
		
		if (pte1->quadword == 0L) 
			continue;
			
		for (pte2 = (pt_entry_t *) (PHYS_TO_KSEG(PTETOPHYS(pte1))),
			l2ptpage = (vm_offset_t) pte2, 
			epte2 = pte2 + PTES_PER_PAGE; pte2 < epte2; pte2++) {
			
			if (pte2->quadword == 0L) 
				continue;
				
			else if (pte2->pg_seg) {
				
				/*
				 * If the segment page table is invalid
				 * then this context has already relinquished
				 * it's segment resident count.
				 */

				if (!pte2->pg_v) 
					continue;

				pmap_ptptetoseg(pte2, segp);

				if (segp == (pmap_seg_t) NULL)
					panic("pmap_collect: pmap segment not valid");

				pmap_segres_lock(segp);
				segp->ps_rescnt--;
				pmap_segres_unlock(segp);
				pte2->quadword = 0L;

				/*
				 * Attempt to acquire the segment lock.
				 * If we failed to, then just continue.
				 */

				if (pmap_seg_lock_try(segp)) {
					if (!segp->ps_rescnt && 
						segp->ps_pagetable) {
						pmap_seg_free_pagetable(segp,
							&l3ptpage);
						pmap_segpt_free(l3ptpage);
						segp->ps_pagetable = (vm_offset_t) 0;
					}
					pmap_seg_unlock(segp);
				}
			} else {
				
				start = (((pte1 - map->level1_pt) + 1)
					 << (3*PGSHIFT - 6)) -
					((epte2 - pte2) << (2*PGSHIFT - 3));	
				
				if (pmap_remove_range(map, 
					start, 
					start + alpha_ptob(PTES_PER_PAGE),
					TRUE)) {
					l2ptpage = (vm_offset_t) 0;/* can't collect this l2 page */
					continue;
				}
				else {
					l3ptpage = PHYS_TO_KSEG(PTETOPHYS(pte2));	
					pte2->quadword = 0L;
				}
			}

			if (l3ptpage) {
				if (ptpages) 
					* (vm_offset_t *) l3ptpage = ptpages;
				else * (vm_offset_t *) l3ptpage = (vm_offset_t) 0;
				ptpages = l3ptpage;
				l3ptpage = (vm_offset_t) 0;
			}
		}
		if (l2ptpage) {
			pte1->quadword = 0L;
			if (ptpages) 
				* (vm_offset_t *) l2ptpage = ptpages;
			else 
				* (vm_offset_t *) l2ptpage = (vm_offset_t) 0;
			ptpages = l2ptpage;
			l2ptpage = (vm_offset_t) 0;
		}
	}

	mtpr_tbiap();	
	
	unlock_pmap(map);
	END_LOCKS_IN_ORDER();
	(void) splx(s);

	if (ptpages) 
		pmap_free_pt_list(ptpages);

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
	struct asn_state *	state;
	struct asn_data *	asn_d;

	thread_pcb(th)->pcb_ptbr = KSEG_TO_PHYS(map->level1_pt) >> PGSHIFT;

	if (pmap_maxasn <= 0)
		return;		/* Skip the ASN code. */

	if (map == kernel_pmap)
		return;		/* kernel threads own ASN 0. */

	state = &asns_on[cpu];
	if (map->map_asn[cpu] == 0) {
		if (queue_empty(&state->free_asns)) {
			/* Reuse the LRU entry of the queue. */
			asn_d = (struct asn_data *)dequeue(&state->active_asns);

			/* On an MP system, perform for each processor. */
			asn_d->owner->map_asn[cpu] = 0;
		} else
			asn_d = (struct asn_data *)dequeue(&state->free_asns);

		map->map_asn[cpu] = asn_d - &state->cpu_asn[0];
		asn_d->owner = map;
		mtpr_tbiap();	/* It would be nice to have tbiASN. */
	} else {
		/* Remove from somewhere in the active queue */
		asn_d = &state->cpu_asn[map->map_asn[cpu]];
		remqueue(&state->active_asns, asn_d);
	}
	/* Insert at the end of the active queue. */
	enqueue(&state->active_asns, asn_d);
	thread_pcb(th)->pcb_asn = map->map_asn[cpu];
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
 *	Routine:	pmap_remove_all
 *	Function:
 *		Removes this physical page from
 *		all physical maps in which it resides.
 */




kern_return_t
pmap_remove_all(phys, force)
	vm_offset_t		phys;
        boolean_t               force;
{
	pmap_t			map;
	union pv_list_head	*head;
	pv_entry_t		pve;
	long			s;
	register unsigned long	save;
	register vm_offset_t 	l3ptpage, nl3ptpage;
	int                     lw_wire_found;

	if (!managed(phys))
		return KERN_FAILURE;

	l3ptpage = (vm_offset_t) 0;
	s = splvm();
	BEGIN_LOCKS_OUT_OF_ORDER();
	head = pa_to_pvh(phys);
	lock_pvh(head);

	lw_wire_found = 0;
	pve = &head->entry;
	if (pve->pv_pmap) {
		save = head->bits.keep;
		do {
			pt_entry_t	*pte1p, *pte2p, *pte;
			register pt_entry_t *epte;
			boolean_t	other;
			pv_entry_t	temp;
			register pmap_seg_t segp;
			register vm_offset_t va;

			if (pv_isseg(pve)) {
				va = pve->pv_va;
				segp = pve->pv_seg;
				pmap_seg_lock(segp);
				va = pve->pv_va & ~PV_SEGFLAG;
				pte = pmap_segpte(segp, va);
				for (epte = pte + alpha_btop(PAGE_SIZE); 
				     pte < epte; pte++) {
				        if(pte->pg_wire)
					    panic("pmap_remove_all, page wired in seg");
					pte->quadword = 0L;
				}
				segp->ps_loadedpte--;
				if (segp->ps_rescnt)
					pmap_seg_remove_all(segp, va);
				else if (!segp->ps_loadedpte) {
					nl3ptpage = segp->ps_pagetable;
					pmap_segpt_free(nl3ptpage);
					segp->ps_pagetable = (vm_offset_t) 0;
					if (l3ptpage) {
						* (vm_offset_t *) nl3ptpage = 
							l3ptpage;
						l3ptpage = nl3ptpage;
					}
					else {
						l3ptpage = nl3ptpage;
						* (vm_offset_t *) l3ptpage = 
							(vm_offset_t) 0;
					}
				}
				pmap_seg_unlock(segp);
			}
			else {
				lock_pmap(pve->pv_pmap);
				other = pmap_pt_access(pve->pv_pmap, pve->pv_va,
						       &pte1p, &pte2p, &pte);

                                pve->pv_pmap->stats.resident_count--;
                                if (MAPS_TEXT(pte))
                                        pve->pv_pmap->stats.resident_text--;

				if(pte->pg_wire ||
				   (pve->pv_pmap->coproc_tbi &&
					 pmap_coproc_invalidate(pve->pv_pmap,
						pve->pv_va, NBPG))) {
				      if(force)
					    panic("pmap_remove_all, force, page wired");
				      ++lww_event[12];
				      lw_wire_found = 1;
				      unlock_pmap(pve->pv_pmap);
				      break;
				}
				else
				      pte->quadword = 0L;

				if (!other)
					mtpr_tbis(pve->pv_va);
				pmap_tbsync(pve->pv_pmap, pve->pv_va, NBPG);
	
				unlock_pmap(pve->pv_pmap);
			}

			if ((temp = pve->pv_next) == PV_ENTRY_NULL)
				break;
			*pve = *temp;
			ZFREE(pv_list_zone, (vm_offset_t)temp);
		} while (TRUE);

		if(!lw_wire_found)
		  pve->pv_pmap = PMAP_NULL;
		head->bits.keep = save;
	}

	unlock_pvh(head);
	END_LOCKS_OUT_OF_ORDER();
	splx(s);

	if (l3ptpage) pmap_free_pt_list(l3ptpage);
	if(lw_wire_found)
	  return KERN_FAILURE;
	else
	  return KERN_SUCCESS;
}

/*
 *	Routine:	pmap_copy_on_write
 *	Function:
 *		Remove write privileges from all
 *		physical maps for this physical page.
 */
void
pmap_copy_on_write(phys)
	vm_offset_t		phys;
{
	pmap_t			map;
	union pv_list_head	*head;
	pv_entry_t		pve;
	long			s;

	if (!managed(phys))
		return;

	s = splvm();
	BEGIN_LOCKS_OUT_OF_ORDER();
	head = pa_to_pvh(phys);
	lock_pvh(head);

	pve = &head->entry;
	if (pve->pv_pmap != PMAP_NULL) {
		do {
			pt_entry_t	*pte1p, *pte2p, *pte;
			boolean_t	other;

			/*
			 * For segments we ignore global COW operations.
			 */

			if (pv_isseg(pve)) continue;

			lock_pmap(pve->pv_pmap);
			other = pmap_pt_access(pve->pv_pmap, pve->pv_va,
					       &pte1p, &pte2p, &pte);
			pte->pg_prot &= PROT_UR;

			if (!other)
				mtpr_tbis(pve->pv_va);
			pmap_tbsync(pve->pv_pmap, pve->pv_va, NBPG);
			unlock_pmap(pve->pv_pmap);

		} while ((pve = pve->pv_next) != PV_ENTRY_NULL);
	}

	unlock_pvh(head);
	END_LOCKS_OUT_OF_ORDER();
	splx(s);
}

/*
 *	pmap_page_protect:
 *
 *	Lower the permission for all mappings to a given page.
 */
kern_return_t
pmap_page_protect(phys, prot)
	vm_offset_t	phys;
	vm_prot_t	prot;
{
	INC_PRT_COUNT;

	switch (prot) {
		case VM_PROT_READ:
		case VM_PROT_READ|VM_PROT_EXECUTE:
			pmap_copy_on_write(phys);
			break;
		case VM_PROT_ALL:
			break;
                case VM_PROT_NONE|VM_PROT_TRY:
			return (pmap_remove_all(phys, FALSE));
		default:
			pmap_remove_all(phys, TRUE);
			break;
	}
	return KERN_SUCCESS;
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
#ifndef pmap_pageable
/* See pmap.h */
pmap_pageable(map, start, end, pageable)
	pmap_t		map;
	vm_offset_t	start;
	vm_offset_t	end;
	boolean_t	pageable;
{
}
#endif	/* pmap_pageable */

/* external interface */
boolean_t
pmap_coproc_page_is_busy(pmap, start, end)
	pmap_t		pmap;
	vm_offset_t	start;
	vm_size_t	end;
{
        boolean_t       page_busy = FALSE;

	if (pmap->coproc_tbi)
	     page_busy = pmap_coproc_invalidate(pmap, start, end-start);
	return page_busy;
}


/*
 * pmap_coproc_invalidate:
 * Invalidate a virtual-to-physical mapping in a "smart" coprocessor.
 * This function assumes that the function pointer pmap->coproc_tbi is
 * non-NULL.  The current implementations of pmap->coproc_tbi always
 * return 0, signifying that the translation was invalidated.  In
 * anticipation of future developments, pmap_coproc_invalidate tries
 * to make sense of non-zero returns.
 */
boolean_t
pmap_coproc_invalidate(pmap, start, size)
	pmap_t		pmap;
	vm_offset_t	start;
	vm_size_t	size;
{
	boolean_t	page_busy = FALSE;
	vm_offset_t	va = trunc_page(start);
	vm_offset_t	limit = round_page(start + size);

	do
	        page_busy = (*pmap->coproc_tbi)(PMAP_COPROC_INVALIDATE_STLB, va);
	while (!page_busy && (va += NBPG) < limit);

	return page_busy;
}
void
pmap_coproc_exit_notify(pmap)
     pmap_t              pmap;
{
     if (pmap->coproc_tbi) {
         (*pmap->coproc_tbi)(PMAP_COPROC_EXIT, (vm_offset_t)0);
	 (pmap)->coproc_tbi = (int(*)())NULL;
     }
}

/*
 *	Set the modify bits on the specified physical page.
 */
void
pmap_set_modify(phys)
	vm_offset_t	phys;
{
	INC_SMD_COUNT;

	PMAP_SET_MODIFY(phys);
}

/*
 *	Clear the modify bits on the specified physical page.
 */
void
pmap_clear_modify(phys)
	vm_offset_t	phys;
{
	INC_CLR_COUNT;

	if (managed(phys))
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

	INC_IMD_COUNT;

	if (managed(phys))
		return modify_bit(phys);
	else
		return FALSE;
}

/*
 *	pmap_clear_reference:
 *
 *	Clear the reference bit on the specified physical page.
 */
void
pmap_clear_reference(phys)
	vm_offset_t		phys;
{
	union pv_list_head * 	head;
	register pv_entry_t	pve;
	long			s;

	INC_CRR_COUNT;
#ifdef REF_BITS
	if (!managed(phys))
		return;

	s = splvm();
	BEGIN_LOCKS_OUT_OF_ORDER();
	head = pa_to_pvh(phys);
	lock_pvh(head);

	/* Clear the bit. */
	head->bit.ref = 0;

	/*
	 * In order to detect the next reference set the fault-on bits
	 * for the PTEs which map phys.
	 */
       	for (pve = &head->entry; pve && pve->pv_pmap; pve = pve->pv_next) {
		pt_entry_t	temp;

		if (pv_isseg(pve)) {
			register vm_offset_t 	va;
			register pt_entry_t *	pte;

			pmap_seg_lock(pve->pv_seg);
			va = pve->pv_va & ~PV_SEGFLAG;
			pte = pmap_segpte(pve->pv_seg, va);
			temp.quadword = pte->quadword;
			SET_fo_any(temp);
			pte->quadword = temp.quadword;
			pmap_seg_remove_all(pve->pv_seg, va);
			pmap_seg_unlock(pve->pv_seg);
		} else {
			pt_entry_t	*pte1p, *pte2p , *pte;
			boolean_t	other;

			lock_pmap(pve->pv_pmap);
			other = pmap_pt_access(pve->pv_pmap, pve->pv_va,
					       &pte1p, &pte2p, &pte);
			temp.quadword = pte->quadword;
			SET_fo_any(temp);
			pte->quadword = temp.quadword;
			if (!other)
				mtpr_tbis(pve->pv_va);
	/*
	 * Here we don't call pmap_tbsync().  Pixelvision can't handle
	 * it, nor is it really necessary, as we haven't made the PTE
	 * invalid but merely changed its attributes.  SMP doesn't exist
	 * yet, so pmap_tbsync() only deals with graphics options.
	 */
			unlock_pmap(pve->pv_pmap);
		}
	}

	unlock_pvh(head);
	END_LOCKS_OUT_OF_ORDER();
	splx(s);
#endif	/* REF_BITS */
}

/*
 *	pmap_is_referenced:
 *
 *	Return whether or not the specified physical page is referenced
 *	by any physical maps.
 */
#ifndef pmap_is_referenced
boolean_t
pmap_is_referenced(phys)
	vm_offset_t	phys;
{
	register pv_entry_t	pve;
	pv_entry_t		prev;
	union pv_list_head	*head;
	long			s;
	unsigned long		save;
	boolean_t		other;
	pt_entry_t		*pte1, *pte2, *pte3;

	INC_IRR_COUNT;

	if (!managed(phys))
		return FALSE;

	if (reference_bit(phys))
		return TRUE;

	return FALSE;
}
#endif /* pmap_is_referenced */

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

	if (!managed(phys))
		return(FALSE);

	pv = &pa_to_pvh(phys)->entry;
	return (pv->pv_pmap == PMAP_NULL);

}

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
	if(tbop == TB_SYNC_NONE || pmapp == PMAP_NULL)
		return;

	if (in_Othermap(pmap_pte(pmapp, va)))
		mtpr_tbia();	/* Can't selectively invalidate other ASNs. */
	else {
		register vm_offset_t	addr, end;

		for (addr = va, end = va + sz - 1; addr <= end; addr += NBPG)
			mtpr_tbis(addr);
		if ((addr ^ end) < NBPG) /* Did we stop before the last page? */
			mtpr_tbis(end);
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
	vm_offset_t		kva;
	register vm_offset_t	phys;
	vm_size_t		sz;
	register vm_prot_t	prot;
	vm_tbop_t		tbop;
{

	if (!pmap_module_is_initialized)
		return KERN_FAILURE;

	if(sz == 0 || sz > (VM_MAX_KERNEL_ADDRESS - VM_MIN_KERNEL_ADDRESS))
		return KERN_INVALID_ARGUMENT;

	if(kva < VM_MIN_KERNEL_ADDRESS || kva + sz > VM_MAX_KERNEL_ADDRESS)
		return KERN_INVALID_ADDRESS;

	pmap_enter_range(kernel_pmap, alpha_trunc_page(kva),
			 alpha_trunc_page(phys), alpha_round_page(sz), prot);

	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(kernel_pmap, kva, sz);
	return KERN_SUCCESS;
}

/*
 * unload va from the kernel_pmap
 * unmap it from the TB(s) if requested
 */
void
pmap_mmu_unload(va, sz, tbop)
	vm_offset_t	va;
	vm_size_t	sz;
	vm_tbop_t	tbop;
{
	register vm_offset_t	end;
	register pt_entry_t *	pte;
	int			s;

	if (!pmap_module_is_initialized)
		return;

	if (sz == 0 || va < VM_MIN_KERNEL_ADDRESS)
		return;

	if ((end = va + sz) > VM_MAX_KERNEL_ADDRESS)
		end = VM_MAX_KERNEL_ADDRESS;

	while (va < end) {
		pte = pmap_map_pt_pages(kernel_pmap, va, &s);
		do {
			pte->quadword = 0L;
			if (tbop == TB_SYNC_LOCAL)
				mtpr_tbis(va);
			va += NBPG;
		} while (va < end && ((long)++pte & PGOFSET));
		unlock_pmap(kernel_pmap);
		(void)splx(s);
	}
	if (tbop == TB_SYNC_ALL)
		mtpr_tbia();	/* a stand-in for pmap_tbsync() */
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
	pt_entry_t	*kpte, *upte1, *upte2, *upte3, *end2, *end3;
	int		npf;
	pt_entry_t	scratch;
	vm_offset_t	va, end;
	int		s;

	if (!pmap_module_is_initialized)
		return KERN_FAILURE;

	if(upmap == PMAP_NULL || sz == 0)
		return KERN_INVALID_ARGUMENT;

	if(kva < VM_MIN_KERNEL_ADDRESS || kva + sz > VM_MAX_KERNEL_ADDRESS)
		return KERN_INVALID_ADDRESS;

	ASSERT((unsigned int)kprot <= VM_PROT_ALL);

	/* ensure that the kernel address pte's are mapped */
	for (va = alpha_trunc_page(kva), end = alpha_round_page(kva + sz);
	     			va < end; va += PAGE_SIZE) {
		pmap_map_pt_pages(kernel_pmap, va, &s);
		unlock_pmap(kernel_pmap);
                (void)splx(s);
	}

	scratch = proto_kern_ptetab[kprot];

	s = splvm();
	lock_pmap(upmap);
	(void)pmap_pt_access(upmap, uva, &upte1, &upte2, &upte3);
	kpte = pmap_pte(kernel_pmap, kva);
	for (va = alpha_trunc_page(kva), end = alpha_round_page(kva + sz);
	     va < end; ++upte1) {
		if (upte1->pg_v == 0) {
			printf ("upte1==0x%l016x  *upte1==0x%l016x\r\n",
				upte1, upte1->quadword);
			panic("pmap_dup: level1 PTE not valid");
		}
		end2 = (pt_entry_t *)(alpha_trunc_page(upte2) + NBPG);
		for (; va < end && upte2 < end2; ++upte2) {
			if (upte2->pg_v == 0) {
				printf ("upte2==0x%l016x  *upte2==0x%l016x\r\n",
					upte2, upte2->quadword);
				panic("pmap_dup: level2 PTE not valid");
			}
			end3 = (pt_entry_t *)(alpha_trunc_page(upte3) + NBPG);
			for (; va < end && upte3 < end3; ++upte3, va += NBPG) {
				if (upte3->pg_v == 0) {
				printf ("upte3==0x%l016x  *upte3==0x%l016x\r\n",
					upte3, upte3->quadword);
				panic("pmap_dup: level3 PTE not valid");
			}
				if (kprot & VM_PROT_WRITE) 
					PMAP_SET_MODIFY(PTETOPHYS(upte3));
				scratch.pg_pfn = upte3->pg_pfn;
				*kpte++ = scratch;
				mtpr_tbis(va);
			}
		}
	}
	unlock_pmap(upmap);
	(void)splx(s);
	if(tbop == TB_SYNC_ALL)
		pmap_tbsync(kernel_pmap, kva, sz);
	return KERN_SUCCESS;
}

kern_return_t
pmap_svatophys(kva, phys)
	vm_offset_t kva;
	vm_offset_t *phys;
{
	pt_entry_t *pte;

	if (IS_KSEG_VA(kva)) {
		*phys = KSEG_TO_PHYS(kva);
		return KERN_SUCCESS;
	}

	if (IS_SYS_VA(kva)) {
		pte = pmap_pte(kernel_pmap, kva);
		if(pte->pg_v) {
			*phys = alpha_ptob(pte->pg_pfn) + (kva & page_mask);
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
	return KERN_FAILURE; /* callers should use PHYS_TO_KSEG() instead */
}

void
pmap_tbsync(pmap, va, siz)
	pmap_t         pmap;
	vm_offset_t    va;
	vm_size_t      siz;
{

     /* don't call graphics co-processors
      * pmap_tbsync only deals with other cpus
      */	

#if NCPUS > 1
	panic("pmap_tbsync: MP work not done");
#endif /* NCPUS */
}

void
pmap_pagemove(from, to, size)
	register vm_offset_t	from, to;
	register vm_size_t	size;
{
	extern char		end[];/* jmfix: revisit for new memory layout */
	register pt_entry_t	*fpte, *tpte;

	if (size & PGOFSET || (char *)from < end || (char *)to < end)/* jmfix */
		panic("pmap_pagemove");
	fpte = pmap_pte(kernel_pmap, from);
	tpte = pmap_pte(kernel_pmap, to);
	while (size > 0) {
		*tpte = *fpte;
		fpte->quadword = 0L;
		mtpr_tbis(from);
		mtpr_tbis(to);
		from += NBPG;
		to += NBPG;
		size -= NBPG;
		tpte++;
		fpte++;
#if NCPUS > 1
		panic("pmap_pagemove: MP work not done");
#endif /* NCPUS */
	}
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
	bcopy(PHYS_TO_KSEG(src), dst, cnt);
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
	bcopy(src, PHYS_TO_KSEG(dst), cnt);
}

/*
 *	pmap_zero_page zeros the specified (machine independent) page.
 *	pmap_copy_page copies the specified (machine independent) pages.
 *
 */
void
pmap_zero_page(phys)
	vm_offset_t	phys;
{
	register long *quad_ptr_A, *quad_ptr_B, *quad_ptr_Z;

	INC_ZER_COUNT;

	PMAP_SET_MODIFY(phys);
	quad_ptr_A = (long *)PHYS_TO_KSEG(phys);
	quad_ptr_Z = (long *)((vm_offset_t)quad_ptr_A + PAGE_SIZE);
	
	do {
		quad_ptr_B = quad_ptr_A + 8;
		quad_ptr_A[0] = 0;
		quad_ptr_A[1] = 0;
		quad_ptr_A[2] = 0;
		quad_ptr_A[3] = 0;
		quad_ptr_A[4] = 0;
		quad_ptr_A[5] = 0;
		quad_ptr_A[6] = 0; /* SRM A.3.4 Sequential Read/Write */
		quad_ptr_A[7] = 0; /* Avoid more than 8 consecutive writes. */
		quad_ptr_A = quad_ptr_B + 8;
		quad_ptr_B[0] = 0; /* SRM A.2.5 Instruction scheduling */
		quad_ptr_B[1] = 0; /* Avoid some register "result latency" */
		quad_ptr_B[2] = 0; /* by alternating between 2 pointers. */
		quad_ptr_B[3] = 0;
		quad_ptr_B[4] = 0;
		quad_ptr_B[5] = 0;
		quad_ptr_B[6] = 0;
		quad_ptr_B[7] = 0;
	} while (quad_ptr_A < quad_ptr_Z);
}

void
pmap_copy_page(src, dst)
	vm_offset_t	src, dst;
{
	register long *quad_ptr_A, *quad_ptr_B, *quad_ptr_C, *quad_ptr_D, *quad_ptr_Z;

	register long a,b,c,d;

	INC_COP_COUNT;

	PMAP_SET_MODIFY(dst);
	quad_ptr_A = (long *)PHYS_TO_KSEG(src);
	quad_ptr_B = (long *)PHYS_TO_KSEG(dst);
	quad_ptr_Z = (long *)((vm_offset_t)quad_ptr_A + PAGE_SIZE);

	do {
		quad_ptr_C = quad_ptr_A + 8;
		quad_ptr_D = quad_ptr_B + 8;

		a = quad_ptr_A[0];
		b = quad_ptr_A[1];
		c = quad_ptr_A[2];
		d = quad_ptr_A[3];
		quad_ptr_B[0] = a;
		quad_ptr_B[1] = b;
		quad_ptr_B[2] = c;
		quad_ptr_B[3] = d;

		a = quad_ptr_A[4];
		b = quad_ptr_A[5];
		c = quad_ptr_A[6];
		d = quad_ptr_A[7];
		quad_ptr_B[4] = a;
		quad_ptr_B[5] = b;
		quad_ptr_B[6] = c;
		quad_ptr_B[7] = d;

		quad_ptr_A += 16;
		quad_ptr_B += 16;

		a = quad_ptr_C[0];
		b = quad_ptr_C[1];
		c = quad_ptr_C[2];
		d = quad_ptr_C[3];
		quad_ptr_D[0] = a;
		quad_ptr_D[1] = b;
		quad_ptr_D[2] = c;
		quad_ptr_D[3] = d;

		a = quad_ptr_C[4];
		b = quad_ptr_C[5];
		c = quad_ptr_C[6];
		d = quad_ptr_C[7];
		quad_ptr_D[4] = a;
		quad_ptr_D[5] = b;
		quad_ptr_D[6] = c;
		quad_ptr_D[7] = d;
	} while (quad_ptr_A < quad_ptr_Z);
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
	return KERN_FAILURE;
}

/*
 * pmap_fault_on:
 * A valid memory reference through a PTE may experience any of three
 * faults depending on the type of reference and the setting of fault
 * bits in the PTE.  These are fault-on-read, fault-on-write, and
 * fault-on-execute.  This function clears the triggering fault bit from
 * the PTE and captures the information which the fault represents.
 *
 * Any fault sets the reference bit for the physical page mapped by the
 * PTE.
 *
 * A write fault sets the modify bit.
 */
long
pmap_fault_on(va, access)
	register vm_offset_t	va;
	register unsigned long	access;
{
	register pmap_t		pm;
	register pt_entry_t	*pte, scratch;
	int			s;
	union pv_list_head	*head;

	pm = IS_SYS_VA(va) ? kernel_pmap : vm_map_pmap(current_task()->map);
	s = splvm();
	BEGIN_LOCKS_IN_ORDER();
	lock_pmap(pm);

	/*
	 * This code needs to provide for segment locking to gain
	 * exclusive control of shared segment PTEs.
	 */
	pte = vtopte(va);
	scratch.quadword = pte->quadword;
	CLEAR_for(scratch);		/* any access counts as a read */
	if (!IS_READ(access)) {
		if (access == MMF_IFETCH) {
			if (!scratch.pg_exec) {
				/* here if execute access is prohibited */
				unlock_pmap(pm);
				END_LOCKS_IN_ORDER();
				(void)splx(s);
				return(0L); /* cause invocation of trap() */
			}
			scratch.pg_foe = 0;	/* allow future executes */
		} else { /* access == MMF_WRITE */
			scratch.pg_fow = 0;	/* allow future writes */
		}
	}
	pte->quadword = scratch.quadword;  /* atomic write protocol for PTEs */

	/* Invalidate TB entries locally and on other processors. */
	mtpr_tbis(va);
	pmap_tbsync(pm, trunc_page(va), NBPG);

	head = pa_to_pvh(PTETOPHYS(pte));
	lock_pvh(head);
	set_reference_bit(head);	/* all accesses set REF */
	if (access == MMF_WRITE)
		head->bit.modify = 1;
	unlock_pvh(head);

	unlock_pmap(pm);
	END_LOCKS_IN_ORDER();
	(void)splx(s);
	return(1L); /* indicate exception handling was successful */
}

/* for console callbacks */

#define BOOTDEVLEN 80
extern char bootdevice[BOOTDEVLEN];	/* BOOTED_DEV console env string */
					/* array defined in sys_sysinfo.c */
/* do some console callbacks BEFORE changing context... */
perform_console_callbacks()
{
	char *status, *prom_getenv();

	status = prom_getenv("booted_dev");
	if (status) 
		strcpy(bootdevice, status);
}

vm_offset_t
pmap_mips_k0(vaddr, phys, size)	/* Obviously, vm/vm_resident.c needs fixing */
        vm_offset_t    *vaddr;
        vm_offset_t    phys;
	vm_size_t      size;
{
	return PHYS_TO_KSEG(phys);
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
	extern struct rpb_mdt *		memdsc;
	register long			pfn = alpha_btop(*phys);
	register struct rpb_cluster *	cluster = &memdsc->rpb_cluster[0];
	register long			i, start_hole, start_memory;

	start_hole = 0;
	*size = 0;
	for (i = 0; i < memdsc->rpb_numcl; ++i, ++cluster) {
		if (cluster->rpb_usage == CLUSTER_USAGE_OS) {
			start_memory = cluster->rpb_pfn;
			if (*size) {
				/* pfn is in the previous cluster. */
				*span = alpha_ptob(start_memory - start_hole);
				return KERN_SUCCESS;
			}
		} else
			continue; /* Ignore console-owned memory. */

		if (pfn >= start_hole && pfn < start_memory) {
			/* pfn is between clusters. */
			*span = alpha_ptob(start_memory - pfn);
			return(KERN_SUCCESS);
		}

		start_hole = cluster->rpb_pfn + cluster->rpb_pfncount;
		if (pfn >= start_memory && pfn < start_hole) {
			/* pfn is inside a cluster. */
			*size =  alpha_ptob(start_hole - pfn);
		}
	}

	/* pfn is in or beyond the last cluster. */
	*span = end - alpha_ptob(MAX(start_hole, pfn));
	return(KERN_SUCCESS);
}

/*
 * The start of pmap segmentation code.
 */

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
	segp->ps_pagetable = get_seg_ptepage();
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
	vm_offset_t l3ptpage;

	pmap_seg_lock(segp);
	if (segp->ps_pagetable) {
		pmap_seg_free_pagetable(segp, &l3ptpage);
		pmap_seg_unlock(segp);
		pmap_segpt_free(l3ptpage);
		put_free_ptepage(l3ptpage);
	}
	else pmap_seg_unlock(segp);
	ZFREE(pmap_seg_zone, segp);
}

pmap_seg_free_pagetable(pmap_seg_t segp, 
		vm_offset_t *l3ptpage)
{
	register pt_entry_t *pte, *lpte;
	register int i, j, s;
	
	pte = (pt_entry_t *) (segp->ps_pagetable);
	lpte = pte + PTES_PER_PAGE;
	j = alpha_btop(PAGE_SIZE);
	for (; (pte < lpte) && segp->ps_loadedpte;) 
		if (pte->pg_v) {
			segp->ps_loadedpte--;
			delete_pv_entry(PTETOPHYS(pte), (pmap_t) segp,
				(pmap_seg_size - alpha_ptob(lpte - pte)) |
				PV_SEGFLAG);	
			for (i = j; i; i--, pte++) pte->quadword = 0L;
		}
		else pte += j;

	*l3ptpage = segp->ps_pagetable;
}

static void
load_seg(register pt_entry_t *pte2,
	register pt_entry_t *pte,
	register vm_offset_t ptable,
	register boolean_t other)
{
	register pt_entry_t tmp;

	tmp = proto_sys_pte;	
	tmp.pg_seg = 1;
	tmp.pg_asm = 0;
	tmp.pg_pfn = KSEG_TO_PHYS(ptable) >> PGSHIFT;	
	pte2->quadword = tmp.quadword;
	if (!other) mtpr_tbis(pte);
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
	pt_entry_t *pte1, *pte2, *pte;
	register pt_entry_t tmp;
	register pv_entry_t pvhp, pvep;
	register int s;
	vm_offset_t pte2_page, pttable;
	register boolean_t other;

	assert((prot == VM_PROT_EXECUTE) || 
		(prot == (VM_PROT_EXECUTE|VM_PROT_READ)));

	pvep = PV_ENTRY_NULL;
	pte2_page = pttable = (vm_offset_t) 0;

retry:
	s = splvm();
	BEGIN_LOCKS_IN_ORDER();
	pmap_seg_lock(segp);
	lock_pmap(pmap);

	other = pmap_pt_access(pmap, addr, &pte1, &pte2, &pte);
	
	if (!pte1->pg_v) { 
		if (!pte2_page) { 
			unlock_pmap(pmap);
			pmap_seg_unlock(segp);
			END_LOCKS_IN_ORDER();
			(void) splx(s);
			pte2_page = get_free_ptepage();
			goto retry;	
		}
		else { 
			tmp = proto_sys_pte;
			tmp.pg_asm = 0;
			tmp.pg_pfn = KSEG_TO_PHYS(pte2_page) >> PGSHIFT;
			pte2_page = (vm_offset_t) 0;
			pte1->quadword = tmp.quadword;
			if (!other) mtpr_tbisd(pte2);
		}	
	}
	
	if (!pte2->pg_v) {
		if (segp->ps_pagetable) 
			(void) load_seg(pte2, pte, segp->ps_pagetable, other);
		else if (pttable != (vm_offset_t) 0) {
			segp->ps_pagetable = pttable;
			pmap_segpt_alloc(segp, pttable);
			(void) load_seg(pte2, pte, pttable, other);
			pttable = (vm_offset_t) 0;
		}
		else {
			unlock_pmap(pmap);
			pmap_seg_unlock(segp);
			END_LOCKS_IN_ORDER();
			(void) splx(s);
			pttable = get_seg_ptepage();
			goto retry;
		}
		pmap_segres_lock(segp);
		segp->ps_rescnt++;
		pmap_segres_unlock(segp);
	}

	pte = pmap_segpte(segp, addr);

	if (!pte->pg_v) {
		if ((pvhp = grab_pv_head(phys)) == PV_ENTRY_NULL) 
			if (pvep) {
				pvhp = insert_pv_entry(phys, pvep);
				pvep = PV_ENTRY_NULL;
			}
			else {
				unlock_pvh(pa_to_pvh(phys));
				unlock_pmap(pmap);
				pmap_seg_unlock(segp);
				END_LOCKS_IN_ORDER();
				(void) splx(s);
				pvep = (pv_entry_t) zalloc(pv_list_zone);
				goto retry;
			}
		fill_pv_entry(phys, pvhp, (pmap_t) segp,
			(addr & pmap_seg_mask) | PV_SEGFLAG);
		segp->ps_loadedpte++;
		tmp = proto_user_segpte;
		tmp.pg_pfn = alpha_btop(phys);
		pte->quadword = tmp.quadword;
		imb();
	}
	if (!other) mtpr_tbis(addr);

	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	END_LOCKS_IN_ORDER();
	(void) splx(s);
	if (pvep != PV_ENTRY_NULL) 
		zfree(pv_list_zone, (vm_offset_t) pvep);
	if (pttable) put_free_ptepage(pttable);
	if (pte2_page) put_free_ptepage(pte2_page);
	return;
}

/*
 * Load a segment into this address space.
 * We may have to allocate the level two page table.
 */

kern_return_t
pmap_seg_load(register pmap_t pmap, 
	register pmap_seg_t segp,
	register vm_offset_t addr)
{
	register int s;
	pt_entry_t *pte2, *pte1, *pte;
	register pt_entry_t tmp;
	vm_offset_t pte2_page, oldpte2_table;
	boolean_t other;
	register pv_entry_t spve, pv;

	addr &= ~pmap_seg_mask;
	pte2_page = (vm_offset_t) 0;
	oldpte2_table = (vm_offset_t) 0;
	spve = (pv_entry_t) zalloc(pv_list_zone);

	while (1) {
		s = splvm();
		BEGIN_LOCKS_IN_ORDER();
		pmap_seg_lock(segp);
		lock_pmap(pmap);
		other = pmap_pt_access(pmap, addr, &pte1, &pte2, &pte);
		if (pte1->pg_v) {
			if (pte2->pg_v) {
				if (!pte2->pg_seg) {
					pmap_remove_range(pmap, addr, 
						addr + pmap_seg_size, FALSE);
					oldpte2_table = PTETOPHYS(pte2);
					pte2->quadword = 0L;
				}
				else goto done;
			}
			break;
		}
		else if (!pte2_page) {
			unlock_pmap(pmap);
			pmap_seg_unlock(segp);
			END_LOCKS_IN_ORDER();
			(void) splx(s);
			pte2_page = get_free_ptepage();
			continue;
		}
		else {
			tmp = proto_sys_pte;
			tmp.pg_asm = 0;
			tmp.pg_pfn = KSEG_TO_PHYS(pte2_page) >> PGSHIFT;
			pte2_page = (vm_offset_t) 0;
			pte1->quadword = tmp.quadword;
			if (!other) mtpr_tbisd(pte2);
			break;
		}
	}

	if (pte2->pg_seg) goto done;
	
	pv = segp->ps_pvsegment;
	
	while (pv) {
		if (pv->pv_pmap == pmap && pv->pv_va == addr) 
			break;
		pv = pv->pv_next;
	}
	
	if (pv != PV_ENTRY_NULL) 
		goto done;
	
	segp->ps_refcnt++;
	spve->pv_pmap = pmap;
	spve->pv_va = addr;
	spve->pv_next = segp->ps_pvsegment;
	segp->ps_pvsegment = spve;
	spve = (pv_entry_t) 0;
	tmp = proto_sys_pte;
	tmp.pg_seg = 1;
	tmp.pg_asm = 0;
	if (segp->ps_pagetable) {
		tmp.pg_pfn = alpha_btop(KSEG_TO_PHYS(segp->ps_pagetable));
		pmap_segres_lock(segp);
		segp->ps_rescnt++;
		pmap_segres_unlock(segp);
	}
	else tmp.pg_v = 0;
	pte2->quadword = tmp.quadword;
done:
	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	END_LOCKS_IN_ORDER();
	(void) splx(s);
	if (spve != PV_ENTRY_NULL) 
		zfree(pv_list_zone, (vm_offset_t) spve);
	if (oldpte2_table) put_free_ptepage(PHYS_TO_KSEG(oldpte2_table));
	if (pte2_page) put_free_ptepage(pte2_page);
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
	register int s;
	pt_entry_t *pte1, *pte2, *pte;
	register pv_entry_t spv, pspv;
	register vm_offset_t pt3page;
	boolean_t other;

	spv = (pv_entry_t) 0;
	pt3page = (vm_offset_t) 0;
	addr &= ~pmap_seg_mask;

	s = splvm();
	BEGIN_LOCKS_IN_ORDER();
	pmap_seg_lock(segp);
	lock_pmap(pmap);
	other = pmap_pt_access(pmap, addr, &pte1, &pte2, &pte);
	
	/*
	 * The new pmap_collect algorithm now garbage collects
	 * level2 page tables.  This increases the cost of this
	 * routine because every object mapped in the upper level's
	 * notion of a segment must traverse the pmap segment PV list.
	 * This operation was originally optimized by inspecting the
	 * pg_seg bit in the level two pte.  A false pg_seg bit was
	 * utilized to inhibit the list traversal.
	 */
	
	pspv = PV_ENTRY_NULL;
	spv = segp->ps_pvsegment;
	while (spv) {
		if (spv->pv_pmap == pmap && spv->pv_va == addr) break;
		pspv = spv;
		spv = spv->pv_next;
	}
	if (spv == PV_ENTRY_NULL) {
			pmap_seg_unlock(segp);
			unlock_pmap(pmap);
			END_LOCKS_IN_ORDER();
			(void) splx(s);
			return;		
	}
	
	if (pspv) pspv->pv_next = spv->pv_next;
	else segp->ps_pvsegment = spv->pv_next;

	if (pte1->pg_v) {
		if (pte2->pg_v) {
			pmap_segres_lock(segp);
			segp->ps_rescnt--;
			pmap_segres_unlock(segp);
			pte2->quadword  = 0L;
			if (!other && segp->ps_loadedpte) mtpr_tbiap();
		}
		else pte2->quadword = 0L;
	}
	
	segp->ps_refcnt--;
	if (!segp->ps_rescnt && !segp->ps_loadedpte && segp->ps_pagetable) {
		pt3page = segp->ps_pagetable;
		pmap_segpt_free(pt3page);
		segp->ps_pagetable = (vm_offset_t) 0;
	}
	unlock_pmap(pmap);
	pmap_seg_unlock(segp);
	END_LOCKS_IN_ORDER();
	(void) splx(s);
	zfree(pv_list_zone, (vm_offset_t) spv);
	if (pt3page) put_free_ptepage(pt3page);
	return;
}

pmap_seg_remove_all(register pmap_seg_t segp,
	register vm_offset_t va)
{
	register int pteindex;
	pt_entry_t *pte1, *pte2, *pte;
	register pmap_t map;
	register pv_entry_t spv;
	register boolean_t other;
	register int i;

	pteindex = alpha_ptob(LEVEL3_PT_OFFSET(va));
	for (spv = segp->ps_pvsegment; spv; spv = spv->pv_next) {
		map = spv->pv_pmap;
		lock_pmap(map);
		va = spv->pv_va + pteindex;
		other = pmap_pt_access(map, va, &pte1, &pte2, &pte);
		if (pte1->pg_v && pte2->pg_v) {
			for (i = PAGE_SIZE; i; i -= ALPHA_PGBYTES, 
					va + ALPHA_PGBYTES) 
				if (!other) mtpr_tbis(va);
		}
		unlock_pmap(map);
	}
}

int
num_kernel_pages(void)
{
	register int i, j, k, count;
	register pt_entry_t *pte1, *pte2, *pte3;
	unsigned int etext_pfn, low_pfn, high_pfn;
	int othermap_index, selfmap_index;

	if (!pmap_physroot || !pmap_physetext)
		return(0);

	etext_pfn = alpha_btop(pmap_physetext);
	low_pfn = alpha_btop(avail_start);
	high_pfn = alpha_btop(avail_end - 1);
	count = 1 + (low_pfn - etext_pfn) + (physmem - high_pfn - 1);
	othermap_index = LEVEL1_PT_OFFSET(Othermap);
	selfmap_index = LEVEL1_PT_OFFSET(Selfmap);
	for (i = 0; i < PTES_PER_PAGE; i++) {
		if (i == othermap_index)
			continue;
		pte1 = (pt_entry_t *)PHYS_TO_KSEG(pmap_physroot) + i;
		if (!pte1->quadword || !pte1->pg_v)
			continue;
		for (j = 0; j < PTES_PER_PAGE; j++) {
			if (i == selfmap_index && j == othermap_index)
				continue;
			pte2 = (pt_entry_t *)PHYS_TO_KSEG(PTETOPHYS(pte1)) + j;
			if (!pte2->quadword || !pte2->pg_v)
				continue;
			for (k = 0; k < PTES_PER_PAGE; k++) {
				pte3 = (pt_entry_t *)
					PHYS_TO_KSEG(PTETOPHYS(pte2)) + k;
				if (pte3->quadword && pte3->pg_v &&
				    pte3->pg_pfn >= low_pfn &&
				    pte3->pg_pfn <= high_pfn)
					count++;
			}
		}
	}
	return(count);
}

int
get_next_page(vm_offset_t *blocks, int maxblocks)
{
	static int i = 0, j = 0, k = 0, state = 0;
	static unsigned int pfn, etext_pfn, low_pfn, high_pfn;
	static int othermap_index, selfmap_index;
	register pt_entry_t *pte1, *pte2, *pte3;
	register vm_offset_t addr;
	register int count;

	if (!pmap_physroot || !pmap_physetext || maxblocks <= 0)
		return(0);

	count = 0;

        switch (state) {
	case 0:				/* hardware rpb page */
		etext_pfn = alpha_btop(pmap_physetext);
		low_pfn = alpha_btop(avail_start);
		high_pfn = alpha_btop(avail_end - 1);
		othermap_index = LEVEL1_PT_OFFSET(Othermap);
		selfmap_index = LEVEL1_PT_OFFSET(Selfmap);
		blocks[count++] = PHYS_TO_KSEG(pmap_physhwrpb);
		pfn = etext_pfn;
		state = 1;
		if (count >= maxblocks)
			return(count);
		/* fall through */
	case 1:				/* global (static) kernel data */
		while (pfn < low_pfn) {
			blocks[count++] = PHYS_TO_KSEG(alpha_ptob(pfn));
			pfn++;
			if (count >= maxblocks)
				return(count);
		}
		state = 2;
		/* fall through */
        case 2:				/* mapped (dynamic) kernel data */
		for (; i < PTES_PER_PAGE; i++) {
			if (i == othermap_index)
				continue;
			pte1 = (pt_entry_t *)PHYS_TO_KSEG(pmap_physroot) + i;
			if (!pte1->quadword || !pte1->pg_v)
				continue;
			for (; j < PTES_PER_PAGE; j++) {
				if (i == selfmap_index && j == othermap_index)
					continue;
				pte2 = (pt_entry_t *)
					PHYS_TO_KSEG(PTETOPHYS(pte1)) + j;
				if (!pte2->quadword || !pte2->pg_v)
					continue;
				while (k < PTES_PER_PAGE) {
					pte3 = (pt_entry_t *)
						PHYS_TO_KSEG(PTETOPHYS(pte2)) +
						k++;
					if (!pte3->quadword || !pte3->pg_v ||
					    pte3->pg_pfn < low_pfn ||
					    pte3->pg_pfn > high_pfn)
						continue;
					blocks[count++] =
						PHYS_TO_KSEG(PTETOPHYS(pte3));
					if (count >= maxblocks)
						return(count);
				}
				k = 0;
			}
			j = 0;
		}
		pfn = high_pfn + 1;
                state = 3;
		/* fall through */
	case 3:				/* above avail_end (nothing yet) */
		while (pfn < physmem) {
			blocks[count++] = PHYS_TO_KSEG(alpha_ptob(pfn));
			pfn++;
			if (count >= maxblocks)
				return(count);
		}
		state = 4;
		/* fall through */
	case 4:				/* end of dump (shouldn't repeat) */
		break;
	}
	return(count);
}

void
pmap_segpt_free(pgpt)
vm_offset_t pgpt;
{
        pv_entry_t      pvk;

        pvh_to_pvk(pa_to_pvh(KSEG_TO_PHYS(pgpt)), pvk);
        if(pvk)
                pvk->pv_pmap = kernel_pmap;
        else
                panic("pmap_segpt_free: no kernel map for this page");

        return;
}

static pt_entry_t *
pmap_map_pt_pages(map, v, spl)
	pmap_t		map;
	vm_offset_t	v;
	int		*spl;	/* returns the prior IPL */
{
	pt_entry_t		scratch;
	pt_entry_t		*pte_at_level[4]; /* Waste a cell for clarity */
	register vm_offset_t	free_pt_page;
	boolean_t		other;
	vm_offset_t		pt_page;
	register pt_entry_t	**pt_pte;

	scratch = proto_sys_pte;
	free_pt_page = (vm_offset_t)0;

      start_over:

	*spl = splvm();
	lock_pmap(map);
	other = pmap_pt_access(map, v, &pte_at_level[1],
				       &pte_at_level[2],
				       &pte_at_level[3]);
	for (pt_pte = &pte_at_level[1]; pt_pte < &pte_at_level[3]; ++pt_pte)
		if (!(*pt_pte)->pg_v)
			if (pt_page = free_pt_page) {
				/* Show free_pt_page was consumed. */
				free_pt_page = (vm_offset_t)0;
				scratch.pg_pfn =
					KSEG_TO_PHYS(pt_page) >> PGSHIFT;
				scratch.pg_asm = IS_SYS_VA(v);
				**pt_pte = scratch;
				if (!other)
					mtpr_tbisd(*(pt_pte+1));
			} else {
				/* need a free_pt_page */
				unlock_pmap(map);
				(void)splx(*spl);
				free_pt_page = get_free_ptepage();
				goto start_over;
			}
	if (free_pt_page) {
		/* free_pt_page was allocated but not used. */
		put_free_ptepage(free_pt_page);
	}
	/* IPL is elevated to splvm(), and the map is locked */
	return pte_at_level[3];
}

/*
 * pmap_enter_range maps a range of contiguous virtual addresses to a
 * a range of contiguous physical addresses.  The physical addresses
 * MUST NOT be part of the managed memory pool, i.e. "managed(p)" is
 * assumed to be FALSE.  This functions only creates PTEs.  It is
 * particularly suitable for mapping I/O space.  Management of the
 * translation buffer is the responsibility of the caller.
 */

void
pmap_enter_range(map, v, p, size, prot)
	register pmap_t		map;
	register vm_offset_t	v;		/* must be page aligned */
	register vm_offset_t	p;		/* must be page aligned */
	register vm_size_t	size;		/* multiple of page size */
	vm_prot_t		prot;
{
	register long		gh_size;	/* counted in bytes */
	register long		gh;		/* PTE granularity hint */
	register pt_entry_t	*pte;
	pt_entry_t		scratch;
	int			s;

	/* Fill in a PTE template. */
	scratch = (map == kernel_pmap) ?
		proto_kern_ptetab[prot] : proto_user_ptetab[prot];
	scratch.pg_asm = IS_SYS_VA(v);
	scratch.pg_pfn = p >> PGSHIFT;
	scratch.pg_wire = 1;	/* no page table scavenging here */

	while (size >= NBPG) {
			/* Compute the highest power of 2 dividing p and v... */
		gh_size = p | v;
			/* ...and if size is smaller than that power...       */
		if (size < (-gh_size & gh_size))
			/* ...substitute the highest power less than size.    */
			gh_size = 1L << fls(size);

		/* Compute PTE granularity hint by counting powers of 8. */
		gh_size >>= PGSHIFT;
		for (gh = 0; ((gh_size & 0x7) == 0) && (gh < 3); ++gh)
			gh_size >>= 3;
		scratch.pg_gh = gh;

		/* Determine address of PTEs we'll be changing below. */
		pte = pmap_map_pt_pages(map, v, &s); /* raises IPL, locks map */

		/*
		 * Set gh_size to the number of bytes to be mapped in
		 * this pass through the loop.
		 */
		gh_size = 1L << (3 * gh + PGSHIFT);
		size -= gh_size;
		p += gh_size;
		v += gh_size;

		/* Assign all PTEs within relevant granularity hint range. */
		do {
			*pte++ = scratch;
			++scratch.pg_pfn;
		} while (gh_size -= NBPG);

		unlock_pmap(map);
		(void)splx(s);
	}
}

/* only works for the current map */
/* return num pages wired */
/* -1 returned if a fault occurs
 * -2 returned if a segment is found
 */

int
pmap_lw_wire(map, start, n_pages, buf, t)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
     u_long             *buf;
     u_long             t;
{
     long		        s;
     register pt_entry_t	*pte, *endpte;
     register pt_entry_t        s1;
     int                        ret;
  
     s = splvm();
     lock_pmap(map);

     pte = vtopte(start);
     s1 = *pte;
     endpte = pte + n_pages;
     ret = 0;
     *buf++ = t;

     for(; pte < endpte; pte++, buf++) {
         s1 = *pte;
	 if ((s1.quadword & (VALID_MASK|SEG_MASK)) 
	       == VALID_MASK) {
	     s1.quadword |= LWW_MASK;
	     *buf = alpha_ptob(s1.pg_pfn);
	     *pte = s1;
	     continue;
	   }
	 else if (pte->pg_v && pte->pg_seg) {
	     ++lww_event[13];
	     ret = -2;
	     goto done;
	   }
         ++lww_event[14];
	 ret = -3;
	 goto done;
       }
   done:
     unlock_pmap(map);
     splx(s);
     return ret;

   }
int
pmap_lw_unwire(map, start, n_pages)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
{
    /* note: if locals added or moved, must update
     * PMAPLW_UN_SPACE (the sp change) and IPLLOC_UN (the offset of s)
     * in nofault.s
     */       

     long		        s;
     register pt_entry_t	scratch;
     register pt_entry_t	*pte, *endpte;
  
     s = splvm();
     lock_pmap(map);

     pte = vtopte(start);
     scratch = *pte;
     endpte = pte + n_pages;

     for(; pte < endpte; pte++) {
         scratch.quadword = pte->quadword;
	 if ((scratch.quadword & (VALID_MASK|SEG_MASK)) 
	       == VALID_MASK) {
	     scratch.quadword &= ~LWW_MASK;
	     pte->quadword = scratch.quadword;
	     continue;
	 }
	 else
	     panic("pmap_lw_unwire");
       }

     unlock_pmap(map);
     splx(s);

   }

int
pmap_lw_unwire_aud(map, start, n_pages)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
{
    /* note: if locals added or moved, must update
     * PMAPLW_UN_SPACE (the sp change) and IPLLOC_UN (the offset of s)
     * in nofault.s
     */       

     long		        s;
     register pt_entry_t	scratch;
     register pt_entry_t	*pte, *endpte;
  
     s = splvm();
     lock_pmap(map);

     pte = vtopte(start);
     scratch = *pte;
     endpte = pte + n_pages;

     for(; pte < endpte; pte++) {
         scratch.quadword = pte->quadword;
	 if ((scratch.quadword & (VALID_MASK|SEG_MASK)) 
	       == VALID_MASK) {
	     if((scratch.quadword & LWW_MASK) == 0)
	         panic("pmap_lw_unwire_aud1");
	     scratch.quadword &= ~LWW_MASK;
	     pte->quadword = scratch.quadword;
	     continue;
	   }
	 else 
	   panic("pmap_lw_unwire_aud2");
       }

     unlock_pmap(map);
     splx(s);
     return 0;

   }
       
void
pmap_get_pfns(map, start, n_pages, buf)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
     u_long             *buf;
{
    long		        s;
    register pt_entry_t	scratch;
    register pt_entry_t	*pte, *endpte;

    s = splvm();
    lock_pmap(map);

    pte = vtopte(start);
    scratch = *pte;
    endpte = pte + n_pages;

    for(; pte < endpte; pte++) {
        scratch.quadword = pte->quadword;
        if ((scratch.quadword & (VALID_MASK|SEG_MASK)) 
	    == VALID_MASK) {
	    if(!pte->pg_wire)
	        panic("pmap_get_pfns: page not wired");
	    *buf++ = alpha_ptob(pte->pg_pfn);
	  }
	else
	    panic("pmap_get_pfns");
      }

    unlock_pmap(map);
    splx(s);

}
void
pmap_lw_set_modify(map, start, n_pages)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
{
  
    long		        s;
    register pt_entry_t	scratch;
    register pt_entry_t	*pte, *endpte;

    s = splvm();
    lock_pmap(map);

    pte = vtopte(start);
    scratch = *pte;
    endpte = pte + n_pages;

    for(; pte < endpte; pte++) {
        scratch.quadword = pte->quadword;
        if ((scratch.quadword & (VALID_MASK|SEG_MASK)) 
	    == VALID_MASK) {
	    if(!pte->pg_wire)
	        panic("pmap_get_pfns: page not wired");
	    PMAP_SET_MODIFY(alpha_ptob(pte->pg_pfn));
	}
	else
	    panic("pmap_lw_set_modify");
      }

    unlock_pmap(map);
    splx(s);

}
/* return 1 if any page not modified */
/* only used for testing pmap_lw_set_modify */
int
pmap_lw_check_modify(map, start, n_pages)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
{
  
    long		        s;
    register pt_entry_t	scratch;
    register pt_entry_t	*pte, *endpte;
    int rc;

    s = splvm();
    lock_pmap(map);

    rc = 0;
    pte = vtopte(start);
    scratch = *pte;
    endpte = pte + n_pages;

    for(; pte < endpte; pte++) {
        scratch.quadword = pte->quadword;
        if ((scratch.quadword & (VALID_MASK|SEG_MASK)) 
	    == VALID_MASK) {
	    if(!pte->pg_wire)
	        panic("pmap_get_pfns: page not wired");
	    if(!pmap_is_modified(alpha_ptob(pte->pg_pfn))) {
	         rc = 1;
		 break;
	    }
	}
	else
	    panic("pmap_lw_check_modify");
      }

    unlock_pmap(map);
    splx(s);
    return rc;
}
void
pmap_lw_clear_modify(map, start, n_pages)
     pmap_t	        map;
     vm_offset_t	start;
     int                n_pages;
{
  
    long		        s;
    register pt_entry_t	scratch;
    register pt_entry_t	*pte, *endpte;

    s = splvm();
    lock_pmap(map);

    pte = vtopte(start);
    scratch = *pte;
    endpte = pte + n_pages;

    for(; pte < endpte; pte++) {
        scratch.quadword = pte->quadword;
        if ((scratch.quadword & (VALID_MASK|SEG_MASK)) 
	    == VALID_MASK) {
	    if(!pte->pg_wire)
	        panic("pmap_get_pfns: page not wired");
	    pmap_clear_modify(alpha_ptob(pte->pg_pfn));
	}
	else
	    panic("pmap_lw_check_modify");
      }

    unlock_pmap(map);
    splx(s);
}


zone_t  vm_lw_trans_zone;
/* this should be 4 * quadword aligned */
vm_lw_trans_t  lw_trans_queue;
vm_lw_trans_t  lw_free_queue;
long lw_waiters;
u_long lw_trans_array[LW_TRANS_SIZE*(LW_TRANS_EL + 1)];
decl_simple_lock_data(,vm_lw_trans_queue_lock)


void
lw_init()
{
     vm_size_t    s;
     vm_lw_trans_t    t, last;
     u_long * u;
     int i;

     s = (vm_size_t)sizeof(struct vm_lw_trans);
     vm_lw_trans_zone = zinit(s, s * 300, PAGE_SIZE, "lw trans");
     lw_trans_queue = (vm_lw_trans_t)0;
     lw_free_queue = (vm_lw_trans_t)0;
     simple_lock_init(&vm_lw_trans_queue_lock);

     simple_lock(&vm_lw_trans_queue_lock);
     for(u = &lw_trans_array[0]; ((u_long)u & 0x1f) != 0; u++);
     lw_free_queue = (vm_lw_trans_t)u;
     for(t = (vm_lw_trans_t)(u + LW_TRANS_SIZE), last = lw_free_queue;
	 t < (vm_lw_trans_t)(u + LW_TRANS_SIZE*LW_TRANS_EL); last = t, t++)
       last->next = t;
     t->next = (vm_lw_trans_t)0;
     simple_unlock(&vm_lw_trans_queue_lock);
}

vm_offset_t
pmap_pt_info(map, va, map_ent, info_ent)
	pmap_t			map;
	register vm_offset_t	va;
	vm_map_entry_t		map_ent;
	struct tbl_mapinfo *	info_ent;
{
	int		s;
	pt_entry_t	*pte1, *pte2, *pte;

	info_ent->start_va = va;
	info_ent->mapping = UNMAPPED;

	s = splvm();
	lock_pmap(map);

	(void)pmap_pt_access(map, va, &pte1, &pte2, &pte);

	if (!pte1->pg_v || !pte2->pg_v) {	/* page table missing */
		register unsigned long	adder;

		adder = (1L << (pte1->pg_v ? 2*PGSHIFT-3 : 3*PGSHIFT-6)) - 1;
		va = (va + adder) & ~adder;
		va = MIN(map_ent->vme_end, va);
	} else if (!pte->pg_v) {		/* a range of unmapped pages */
		while((va += NBPG) < map_ent->vme_end
		      && ((long)++pte & PGOFSET) && !pte->pg_v)
			;
	} else {				/* a range of mapped pages */
		register long		i;

		info_ent->mapping = pte2->pg_seg ? SEGMENT : MAPPED;
		i = 0;
		do
			info_ent->pfn[i] = pte->pg_pfn;
		while ((va += NBPG) < map_ent->vme_end && ++i < MAPINFO_PFN_MAX
		       && ((long)++pte & PGOFSET) && pte->pg_v);
	}

	unlock_pmap(map);
	(void)splx(s);

	info_ent->end_va = va;
	return va;
}

/*
 *	Routine:	vtop
 *	Function:
 *		Map a (proc pointer, virtual address) pair to physical address.
 *		If the address could be a user or system virtual address.
 */
vm_offset_t
vtop(procp, va)
	struct  proc *procp;
	vm_offset_t	va;
{

	if (IS_KSEG_VA(va)) 		/* kernel space, unmapped */
		return(KSEG_TO_PHYS(va));

	if (IS_SEG1_VA(va)) 		/* kernel space, mapped */
		return(pmap_extract(kernel_pmap, va));

	if(procp == (struct proc *)NULL)
		panic("vtop: user address passed with null proc pointer\n");

	return( pmap_extract(procp->task->map->vm_pmap, va) );
}
