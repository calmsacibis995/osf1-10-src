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
static char	*sccsid = "@(#)$RCSfile: pmap.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:40 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#define	USE_HDW 1
/* 
 * Mach Operating System
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
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *	(These guys wrote the Vax version)
 *
 *	I386 Physical Map management code.
 *
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

#include <cpus.h>

#include <mach/i386/vm_types.h>

#include <mach/boolean.h>
#include <kern/thread.h>
#include <kern/zalloc.h>

#include <kern/lock.h>

#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>
#include <vm/vm_page.h>
#include <vm/vm_user.h>

#include <mach/i386/vm_param.h>
#include <i386/pcb.h>

/*
 * In case we need to turn off these simple locks independent of the
 * rest of the kernel.
 */
#define lock_pmap(pmap)		simple_lock(&(pmap)->lock)
#define unlock_pmap(pmap)	simple_unlock(&(pmap)->lock)

#ifdef	M380
#define	OLIVETTICACHE	1
#endif	M380

#ifndef	OLIVETTICACHE
#define	WRITE_PTE(pte_p, pte_entry)*pte_p = pte_entry;
#define	WRITE_PTE_FAST(pte_p, pte_entry)*pte_p = pte_entry;
#else	OLIVETTICACHE

/* This gross kludgery is needed for Olivetti XP7 & XP9 boxes to get
 * around an apparent hardware bug. Other than at startup it doesn't
 * affect run-time performance very much, so we leave it in for all
 * models of M380s.
 */
extern	unsigned	*pstart();
#define CACHE_LINE	8
#define CACHE_SIZE	512
#define CACHE_PAGE	0x1000;

#define	WRITE_PTE(pte_p, pte_entry) { write_pte(pte_p, &pte_entry); }

write_pte(pte_p, pte_entry)
pt_entry_t	*pte_p, *pte_entry;
{
	unsigned long count;
	volatile unsigned long hold, *addr1, *addr2;

	if ( *(unsigned long *)pte_entry != *(unsigned long *)pte_p )
		*pte_p = *pte_entry;
	else {
		/* This isn't necessarily the optimal algorithm */
		addr1 = (unsigned long *)pstart;
		for (count = 0; count < CACHE_SIZE; count++) {
			addr2 = addr1 + CACHE_PAGE;
			hold = *addr1;		/* clear cache bank - A - */
			hold = *addr2;		/* clear cache bank - B - */
			addr1 += CACHE_LINE;
		}
	}
}

#define	WRITE_PTE_FAST(pte_p, pte_entry)*pte_p = pte_entry;

#endif	OLIVETTICACHE

/*
 *	Private data structures.
 */

/*
 *	For each vm_page_t, there is a list of all currently
 *	valid virtual mappings of that page.  An entry is
 *	a pv_entry_t; the list is the pv_table.
 */

typedef struct pv_entry {
	struct pv_entry	*next;		/* next pv_entry */
	pmap_t		pmap;		/* pmap where mapping lies */
	vm_offset_t	va;		/* virtual address for mapping */
} *pv_entry_t;

#define PV_ENTRY_NULL	((pv_entry_t) 0)
#define round_i386seg(x) ( ((x) + ( (1<<PDESHIFT) - 1 ) ) & ~( (1<<PDESHIFT) - 1 ) )
#define managed_phys(p)		((p) >= vm_first_phys && (p) < vm_last_phys)
#define PHYS_MODIFIED		0x40
#define	PHYS_REFERENCED		0x20
#define	PHYS_ATTRIBUTES		0x60

pv_entry_t	pv_head_table;		/* array of entries, one per page */
zone_t		pv_list_zone;		/* zone of pv_entry structures */

/*
 *	Each entry in the pv_head_table is locked by a bit in the
 *	pv_lock_table.  The lock bits are accessed by the physical
 *	address of the page they lock.
 */

char	*pv_lock_table;		/* pointer to array of bits */
#define pv_lock_table_size(n)	(((n)+BYTE_SIZE-1)/BYTE_SIZE)

/*
 *	First and last physical addresses that we maintain any information
 *	for.  Initialized to zero so that pmap operations done before
 *	pmap_init won't touch any non-existent structures.
 */
vm_offset_t	vm_first_phys = (vm_offset_t) 0;
vm_offset_t	vm_last_phys  = (vm_offset_t) 0;
boolean_t	pmap_initialized = FALSE;/* Has pmap_init completed? */

/*
 *	Index into pv_head table, its lock bits, and the modify bits
 *	starting at vm_first_phys.
 */

#define pa_index(pa)	(atop(pa - vm_first_phys))

#define pai_to_pvh(pai)		(&pv_head_table[pai])
#define lock_pvh_pai(pai)	(bit_lock(pai, pv_lock_table))
#define unlock_pvh_pai(pai)	(bit_unlock(pai, pv_lock_table))

/*
 *	Array of modify bits, one byte per physical page (to avoid expense
 *	of locking)
 */
char	*phys_attributes;

/*
 *	Locking and TLB invalidation
 */

/*
 *	Locking Protocols:
 *
 *	There are two structures in the pmap module that need locking:
 *	the pmaps themselves, and the per-page pv_lists (which are locked
 *	by locking the pv_lock_table entry that corresponds to the pv_head
 *	for the list in question.)  Most routines want to lock a pmap and
 *	then do operations in it that require pv_list locking -- however
 *	pmap_remove_all and pmap_copy_on_write operate on a physical page
 *	basis and want to do the locking in the reverse order, i.e. lock
 *	a pv_list and then go through all the pmaps referenced by that list.
 *	To protect against deadlock between these two cases, the pmap_lock
 *	is used.  There are three different locking protocols as a result:
 *
 *  1.  pmap operations only (pmap_extract, pmap_access, ...)  Lock only
 *		the pmap.
 *
 *  2.  pmap-based operations (pmap_enter, pmap_remove, ...)  Get a read
 *		lock on the pmap_lock (shared read), then lock the pmap
 *		and finally the pv_lists as needed [i.e. pmap lock before
 *		pv_list lock.]
 *
 *  3.  pv_list-based operations (pmap_remove_all, pmap_copy_on_write, ...)
 *		Get a write lock on the pmap_lock (exclusive write); this
 *		also guaranteees exclusive access to the pv_lists.  Lock the
 *		pmaps as needed.
 *
 *	At no time may any routine hold more than one pmap lock or more than
 *	one pv_list lock.  Because interrupt level routines can allocate
 *	mbufs and cause pmap_enter's, the pmap_lock and the lock on the
 *	kernel_pmap can only be held at splvm.
 *	
 *	The dev_addr list is protected by using a write lock on the 
 *	pmap_lock.  Lookups are expected to be cheap.
 */

#if	NCPUS > 1
/*
 *	We raise the interrupt level to splvm, to block interprocessor
 *	interrupts during pmap operations.  We must take the CPU out of
 *	the cpus_active set while interrupts are blocked.
 */
#define SPLVM(spl)	{ \
	spl = splvm(); \
	i_bit_clear(cpu_number(), &cpus_active); \
}

#define SPLX(spl)	{ \
	i_bit_set(cpu_number(), &cpus_active); \
	splx(spl); \
}

/*
 *	Lock on pmap system
 */
lock_data_t	pmap_lock;

#define READ_LOCK(spl) { \
	SPLVM(spl); \
	lock_read(&pmap_lock); \
}

#define WRITE_LOCK(spl) { \
	SPLVM(spl); \
	lock_write(&pmap_lock); \
}

#define READ_UNLOCK(spl) { \
	lock_read_done(&pmap_lock); \
	SPLX(spl); \
}

#define WRITE_UNLOCK(spl) { \
	lock_write_done(&pmap_lock); \
	SPLX(spl); \
}

#define LOCK_PVH(index)		(lock_pvh_pai(index))

#define UNLOCK_PVH(index)	(unlock_pvh_pai(index))

#define PMAP_UPDATE_TLBS(pmap, s, e) \
{ \
	cpu_set	cpu_mask = 1 << cpu_number(); \
	cpu_set	users; \
 \
	/* Since the pmap is locked, other updates are locked */ \
	/* out, and any pmap_activate has finished. */ \
 \
	/* find other cpus using the pmap */ \
	users = (pmap)->cpus_using & ~cpu_mask; \
	if (users) { \
	    /* signal them, and wait for them to finish */ \
	    /* using the pmap */ \
	    signal_cpus(users, (pmap), (s), (e)); \
	    while ((pmap)->cpus_using & cpus_active & ~cpu_mask) \
		continue; \
	} \
 \
	/* invalidate our own TLB if pmap is in use */ \
	if ((pmap)->cpus_using & cpu_mask) { \
	    INVALIDATE_TLB((s), (e)); \
	} \
}
#else	/* NCPUS > 1 */

#define SPLVM(spl)	{ spl = splvm(); }
#define SPLX(spl)	{ splx(spl); }

#define READ_LOCK(spl)		SPLVM(spl)
#define WRITE_LOCK(spl)		SPLVM(spl)
#define READ_UNLOCK(spl)	SPLX(spl)
#define WRITE_UNLOCK(spl)	SPLX(spl)

#define LOCK_PVH(index)
#define UNLOCK_PVH(index)

#define PMAP_UPDATE_TLBS(pmap, s, e) { \
	/* invalidate our own TLB if pmap is in use */ \
	/* if ((pmap)->cpus_using) */ { \
	    INVALIDATE_TLB((s), (e)); \
	} \
}

#endif	/* NCPUS > 1 */

#define MAX_TBIS_SIZE	32		/* > this -> TBIA */ /* XXX */

#define INVALIDATE_TLB(s, e) { \
	set_cr3(get_cr3()); \
}


#if	NCPUS > 1
/*
 *	Structures to keep track of pending TLB invalidations
 */

#define UPDATE_LIST_SIZE	4

struct pmap_update_item {
	pmap_t		pmap;		/* pmap to invalidate */
	vm_offset_t	start;		/* start address to invalidate */
	vm_offset_t	end;		/* end address to invalidate */
} ;

typedef	struct pmap_update_item	*pmap_update_item_t;

/*
 *	List of pmap updates.  If the list overflows,
 *	the last entry is changed to invalidate all.
 */
struct pmap_update_list {
	simple_lock_data_t	lock;
	int			count;
	struct pmap_update_item	item[UPDATE_LIST_SIZE];
} ;
typedef	struct pmap_update_list	*pmap_update_list_t;

struct pmap_update_list	cpu_update_list[NCPUS];

#endif	NCPUS > 1

/*
 *	Other useful macros.
 */
#define current_pmap()		(vm_map_pmap(current_thread()->task->map))
#define pmap_in_use(pmap, cpu)	(((pmap)->cpus_using & (1 << (cpu))) != 0)

struct pmap	kernel_pmap_store;
pmap_t		kernel_pmap;

struct zone	*pmap_zone;		/* zone of pmap structures */

int		pmap_debug = 0;		/* flag for debugging prints */
int		ptes_per_vm_page;	/* number of 80386 ptes required to map
					   one VM page. */

extern char end;
extern pt_entry_t	kpde[], kpte[];

pv_entry_t pmap_remove_range();	/* forward */
#if	NCPUS > 1
void signal_cpus();		/* forward */
#endif	NCPUS > 1

/* 
 * Instrumentation for copy-on-reference.
 * cor_readonly: number of kernel pages that the machine-independent 
 *		VM code requested be marked read-only.
 * cor_invalid: number of those pages marked hardware invalid even 
 *		though the page is really valid.
 * cor_writefault: number of "cor_invalid" pages that were made valid 
 *		with a write fault.  This includes read faults that 
 *		were upgraded to writes.
 * cor_readfault: number of "cor_invalid" pages that had to be faulted 
 *		in using a read fault.
 * cor_upgrades: number of kernel read faults that were successfully 
 *		upgraded to write faults.
 */
int	cor_readonly = 0;
int	cor_invalid = 0;
int	cor_writefault = 0;
int	cor_readfault = 0;
int	cor_upgrades = 0;

/*
 *	Assume that there are three protection codes, all using low bits.
 *	These form an index into the two i386 protection arrays.  Each 
 *	element in the array is suitable for sticking into the 
 *	protection bitfield.
 *
 *	I don't know what this CHIPBUG stuff is all about. -mdk
 */
int	user_protection_codes[8];
int	kernel_protection_codes[8];

i386_protection_init()
{
	register int	*kp, *up, prot;

	kp = kernel_protection_codes;
	up = user_protection_codes;
	for (prot = 0; prot < 8; prot++) {
		switch (prot) {
		case VM_PROT_NONE | VM_PROT_NONE | VM_PROT_NONE:
			*kp++ = I386_NO_ACCESS;
			*up++ = I386_NO_ACCESS;
			break;
		case VM_PROT_READ | VM_PROT_NONE | VM_PROT_NONE:
		case VM_PROT_READ | VM_PROT_NONE | VM_PROT_EXECUTE:
		case VM_PROT_NONE | VM_PROT_NONE | VM_PROT_EXECUTE:
#ifdef	CHIPBUG
			*kp++ = I386_KRW;
#else
			*kp++ = I386_UR;	/* CHIP BUG ? */
#endif
			*up++ = I386_UR;
			break;
		case VM_PROT_NONE | VM_PROT_WRITE | VM_PROT_NONE:
		case VM_PROT_NONE | VM_PROT_WRITE | VM_PROT_EXECUTE:
		case VM_PROT_READ | VM_PROT_WRITE | VM_PROT_NONE:
		case VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE:
#ifdef	CHIPBUG
			*kp++ = I386_KRW;
#else
			*kp++ = I386_UW;	/* CHIP BUG ? */
#endif
			*up++ = I386_UW;
			break;
		}
	}
}

/*
 *	Given a map and a machine independent protection code,
 *	set the i386 protection code in the given pte.  For kernel
 *	read-only pages, the page is marked invalid (for
 *	copy-on-reference).
 *	
 *	This routine should only be called for (second level) page 
 *	table entries, not for page directory entries.  Also, it 
 *	should not be used for bootstrapping (e.g., setting up pte's 
 *	for kernel text).
 *	
 *	It would be nice if we could put the COR instrumentation in 
 *	here, but many of the calls here are for templates, rather 
 *	than a specific pte.
 */
void
i386_protection(pmap, prot, valid, pte)
	pmap_t		pmap;
	int		prot;
	int		valid;		/* valid bit for pte */
	pt_entry_t	*pte;		/* IN/OUT */
{
	if (pmap == kernel_pmap) {
		pte->prot = kernel_protection_codes[prot];
		pte->sw_valid = valid;
		pte->valid = valid &&
			(prot & VM_PROT_WRITE) == VM_PROT_WRITE;
	} else {
		pte->prot = user_protection_codes[prot];
		pte->sw_valid = pte->valid = valid;
	}
}

/* 
 * XXX for multiprocessors, need to verify that locking is done right.  
 * 
 * We are only interested in cases where the i386 forced us to do 
 * something different than on other architectures.  That is, if the 
 * old sw_valid bit is off, the page would have been faulted anyway, 
 * even on other hardware, so don't count that fault.
 */
void
cor_stats(old, new)
	pt_entry_t *old, *new;
{
	if (new->sw_valid && !new->valid)
		cor_invalid++;
	else if (old->sw_valid && !old->valid && new->valid) {
		if (new->prot == I386_UR)
			cor_readfault++;
		else
			cor_writefault++;
	}
}

/* 
 * This is separate from cor_stats because we don't want 
 * pmap_mark_valid to affect cor_readonly.
 */
void
cor_stats2(map, prot)
	pmap_t map;
	int prot;
{
	if (map == kernel_pmap && !(prot & VM_PROT_WRITE))
		cor_readonly++;
}

#define	pmap_pde(pmap, addr)	(&(pmap)->cr3[pdenum(addr)])
/*
 *	Given an offset and a map, compute the address of the
 *	pte.  If the address is invalid with respect to the map
 *	then PT_ENTRY_NULL is returned (and the map may need to grow).
 *
 *	This is only used internally.
 */
pt_entry_t *pmap_pte(pmap, addr)
	register pmap_t		pmap;
	register vm_offset_t	addr;
{
	register pt_entry_t	*ptp;	/* ptr to page table */
	register pt_entry_t	pde;	/* page directory entry */

	if (pmap->cr3 == 0)
		return(PT_ENTRY_NULL);
	pde = *pmap_pde(pmap, addr);
	if (pde.valid == 0)
		return(PT_ENTRY_NULL);
	ptp = (pt_entry_t *)ptetokv(pde);
	return(&ptp[ptenum(addr)]);

}

#define DEBUG_PTE_PAGE	0

#if	DEBUG_PTE_PAGE
void ptep_check(ptep)
	ptep_t	ptep;
{
	register pt_entry_t	*pte, *epte;
	int			ctu, ctw;

	/* check the use and wired counts */
	if (ptep == PTE_PAGE_NULL)
		return;
	pte = pmap_pte(ptep->pmap, ptep->va);
	epte = pte + I386_PGBYTES/sizeof(pt_entry_t);
	ctu = 0;
	ctw = 0;
	while (pte < epte) {
		if (pte->pfn != 0) {
			ctu++;
			if (pte->wired)
				ctw++;
		}
		pte += ptes_per_vm_page;
	}

	if (ctu != ptep->use_count || ctw != ptep->wired_count) {
		printf("use %d wired %d - actual use %d wired %d\n",
		    	ptep->use_count, ptep->wired_count, ctu, ctw);
		panic("pte count");
	}
}
#endif	DEBUG_PTE_PAGE
/*
 *	Map memory at initialization.  The physical addresses being
 *	mapped are not managed and are never unmapped.
 *
 *	For now, VM is already on, we only need to map the
 *	specified memory.
 */
vm_offset_t pmap_map(virt, start, end, prot)
	register vm_offset_t	virt;
	register vm_offset_t	start;
	register vm_offset_t	end;
	register int		prot;
{
	pt_entry_t template;
	pt_entry_t *pte;
	pt_entry_t oldpte;

	i386_protection(kernel_pmap, prot, 1, &template);
	template.pfn = i386_btop(start);

	while (start < end) {
		pte = pmap_pte(kernel_pmap, virt);
		if (pte == PT_ENTRY_NULL)
			panic("pmap_map: Invalid kernel address\n");
		oldpte = *pte;
		WRITE_PTE_FAST(pte, template)
		template.pfn++;
		cor_stats(&oldpte, pte);
		cor_stats2(kernel_pmap, prot);
		virt += PAGE_SIZE;
		start += PAGE_SIZE;
	}
	pmap_update();
	return(virt);
}

/*
 *	Back-door routine for mapping kernel VM at initialization.  
 * 	Useful for mapping memory outside the range
 *	[vm_first_phys, vm_last_phys) (i.e., devices).
 *	Keeps a list of addresses that were mapped this way.
 *	Otherwise like pmap_map.
 */
vm_offset_t pmap_map_bd(virt, start, end, prot)
	register vm_offset_t	virt;
	register vm_offset_t	start;
	register vm_offset_t	end;
	register int		prot;
{
	pt_entry_t template;
	pt_entry_t *pte;
	pt_entry_t oldpte;

	i386_protection(kernel_pmap, prot, 1, &template);
	template.pfn = i386_btop(start);

	while (start < end) {
		pte = pmap_pte(kernel_pmap, virt);
		if (pte == PT_ENTRY_NULL)
			panic("pmap_map_bd: Invalid kernel address\n");
		oldpte = *pte;
		WRITE_PTE_FAST(pte, template)
		template.pfn++;
		cor_stats(&oldpte, pte);
		cor_stats2(kernel_pmap, prot);
		virt += PAGE_SIZE;
		start += PAGE_SIZE;
	}
	pmap_update();
	return(virt);
}

/*
 *	Bootstrap the system enough to run with virtual memory.
 *	Map the kernel's code and data, and allocate the system page table.
 *	Called with mapping OFF.  Page_size must already be set.
 *
 *	Parameters:
 *	load_start:	PA where kernel was loaded
 *	avail_start	PA of first available physical page
 *	avail_end	PA of last available physical page
 *	virtual_avail	VA of first available page (after kernel bss)
 *	virtual_end	VA of last available page (end of kernel address space)
 *
 *	&start_text	start of kernel text
 *	&etext		end of kernel text
 */
void pmap_bootstrap(load_start, av_start, av_end,
			v_avail, v_end)

	vm_offset_t	load_start;
	vm_offset_t	*av_start;	/* IN/OUT */
	vm_offset_t	*av_end;	/* IN/OUT */
	vm_offset_t	*v_avail;	/* IN/OUT */
	vm_offset_t	*v_end;	/* OUT */
{
	vm_offset_t		va, tva;
	pt_entry_t		template;
	pt_entry_t		*pde, *pte, *ptend;

	/*
	 *	Set ptes_per_vm_page for general use.
	 */
	ptes_per_vm_page = page_size / I386_PGBYTES;

	/*
	 *	Initialize protection arrays.
	 */
	i386_protection_init();

	/*
	 *	The kernel's pmap is statically allocated so we don't
	 *	have to use pmap_create, which is unlikely to work
	 *	correctly at this part of the boot sequence.
	 */

	kernel_pmap = &kernel_pmap_store;
	ldt_init(kernel_pmap->ldt, (char *)0, 0);

#if	NCPUS > 1
	lock_init(&pmap_lock, FALSE);		/* NOT a sleep lock */
#endif

	simple_lock_init(&kernel_pmap->lock);

	kernel_pmap->ref_count = 1;

	/*
	 *	Allocate the kernel page table from the front
	 *	of available physical memory.
	 */

	/*
	 *	Map low system memory, system text, system data/bss/symbols
	 */

	*v_avail = phystokv(*av_start);
	*v_end = phystokv(*av_end);
	pde = kpde;
	pde += pdenum(VM_MIN_KERNEL_ADDRESS);
	*(int *)&template = 0;
	template.valid = template.sw_valid = 1;
#ifdef	CHIPBUG
	template.prot = I386_KRW;
#else
	template.prot = I386_UW;
#endif
	pte = kpte;
	ptend = pte + NPTES;
	for (va = VM_MIN_KERNEL_ADDRESS; va < *v_end; va += I386_PGBYTES) {
		if (pte >= ptend) {
			pte = (pt_entry_t *)*v_avail;
			ptend = pte + NPTES;
			*v_avail = (vm_offset_t)ptend;
			pde++;
			pde->pfn = i386_btop((unsigned int)pte - VM_MIN_KERNEL_ADDRESS);
#ifdef	CHIPBUG
			pde->prot = I386_KRW;
#else
			pde->prot = I386_UW;
#endif
			pde->valid = 1;
		}
		WRITE_PTE_FAST(pte, template)
		pte++;
		template.pfn++;
	}
	*av_start = *v_avail - VM_MIN_KERNEL_ADDRESS;
	*(int *)&template = 0;
/*
 *	startup requires additional virtual memory (for tables, buffers, 
 *	etc.).  The kd driver may also require some of that memory to
 *	access the graphics board.
 *
	*v_end += *v_end - *v_avail;
 */
#define MOREVM	20			/* megabytes */
	*v_end += MOREVM*1024*1024;
	for (tva = va; tva < *v_end; tva += I386_PGBYTES) {
		if (pte >= ptend) {
			pte = (pt_entry_t *)*v_avail;
			ptend = pte + NPTES;
			*v_avail = (vm_offset_t)ptend;
			*av_start += I386_PGBYTES;
			pde++;
			pde->pfn = i386_btop((unsigned int)pte - VM_MIN_KERNEL_ADDRESS);
#ifdef	CHIPBUG
			pde->prot = I386_KRW;
#else
			pde->prot = I386_UW;
#endif
			pde->valid = 1;
		}
		WRITE_PTE_FAST(pte, template)
		pte++;
	}
	*v_avail = va;
/*
 *	c.f. comment above
 *
	*v_end = va + (*av_end - *av_start);
 */
	*v_end = va + (MOREVM*1024*1024);
	while (pte < ptend)
		*(int *)pte++ = 0;
/*
 *	invalidate virtual addresses at 0
 */
	*(int *)&kpde[0] = 0;
	kernel_pmap->cr3 = kpde;
	printf("Kernel virtual space from 0x%x to 0x%x.\n",
			VM_MIN_KERNEL_ADDRESS, *v_end);
	printf("Available physical space from 0x%x to 0x%x\n", *av_start, *av_end);

	/*
	 *	Once we turn on mapped mode, we need to reference the system
	 *	page table via its virtual address.  (Same for kernel_pmap).
	 */


}

/*
 *	Initialize the pmap module.
 *	Called by vm_init, to initialize any structures that the pmap
 *	system needs to map virtual memory.
 */
void pmap_init(phys_start, phys_end)
	vm_offset_t	phys_start, phys_end;
{
	register long		npages;
	register vm_offset_t	addr;
	register vm_size_t	s;

	/*
	 *	Allocate memory for the pv_head_table and its lock bits,
	 *	the modify bit array, and the pte_page table.
	 */

	npages = atop(phys_end - phys_start);
	s = (vm_size_t) (sizeof(struct pv_entry) * npages
				+ pv_lock_table_size(npages)
				+ npages);

	s = round_page(s);
	addr = (vm_offset_t) kmem_alloc(kernel_map, s);
	/*
	 *	memory was cleared by kmem-alloc
	 */

	/*
	 *	Allocate the structures first to preserve word-alignment.
	 */
	pv_head_table = (pv_entry_t) addr;
	addr = (vm_offset_t) (pv_head_table + npages);

	pv_lock_table = (char *) addr;
	addr = (vm_offset_t) (pv_lock_table + pv_lock_table_size(npages));

	phys_attributes = (char *) addr;

	/*
	 *	Create the zone of physical maps,
	 *	and of the physical-to-virtual entries.
	 */
	s = (vm_size_t) sizeof(struct pmap);
	pmap_zone = zinit(s, 400*s, 4096, FALSE, "pmap"); /* XXX */
	s = (vm_size_t) I386_PGBYTES;
	s = (vm_size_t) sizeof(struct pv_entry);
	pv_list_zone = zinit(s, 10000*s, 4096, FALSE, "pv_list"); /* XXX */

#if	NCPUS > 1
	/*
	 *	Set up the pmap request lists
	 */
	for (i = 0; i < NCPUS; i++) {
	    pmap_update_list_t	up = &cpu_update_list[i];

	    simple_lock_init(&up->lock);
	    up->count = 0;
	}
#endif	NCPUS > 1

	/*
	 *	Only now, when all of the data structures are allocated,
	 *	can we set vm_first_phys and vm_last_phys.  If we set them
	 *	too soon, the kmem_alloc above will try to use these
	 *	data structures and blow up.
	 */

	vm_first_phys = phys_start;
	vm_last_phys = phys_end;
	pmap_initialized = TRUE;
}

/*
 *	Use as much memory below 1MB as is possible.
 *	(What do those hex numbers in comments mean? -mdk)
 */
boolean_t	pmap_valid_page(p)
	vm_offset_t	p;
{
	register int ret;
	extern int cnvmem;
	extern	char *esym;
#ifdef PS2
	extern char etext, sdata;
#endif /* PS2 */

	if (p < 4096) {
		ret = FALSE;
	} else if (p < cnvmem * 1024) {	/* 0x006 - */
		ret = TRUE;
#ifdef PS2
	/* if there is any memory between the end of text and the start of
	 * data then we include it in the free memory region. This can go
	 * away when the linker is fixed to align the data on the next
	 * page boundary after the text. The extra page is to guard against accidents.
	 */
	} else if ((p > (vm_offset_t)&etext + 4096 - VM_MIN_KERNEL_ADDRESS) &&
		(p < (vm_offset_t)&sdata - 4096 - VM_MIN_KERNEL_ADDRESS)) {
		ret = TRUE;
#endif /* PS2 */
	} else if (p < (vm_offset_t)esym - VM_MIN_KERNEL_ADDRESS) { /* 0x0a0 - */
		ret = FALSE;
	} else {			/* 0x17c - */
		ret = TRUE;
	}
	return(ret);
}

boolean_t pmap_verify_free(phys)
	vm_offset_t	phys;
{
	pv_entry_t	pv_h;
	int		pai;
	int		spl;
	boolean_t	result;

	if (!pmap_initialized)
		return(TRUE);

	if (!(managed_phys(phys)))
		return(FALSE);

	WRITE_LOCK(spl);

	pai = pa_index(phys);
	pv_h = pai_to_pvh(pai);

	result = (pv_h->pmap == PMAP_NULL);
	WRITE_UNLOCK(spl);

	return(result);
}

/*
 *	Create and return a physical map.
 *
 *	If the size specified for the map
 *	is zero, the map is an actual physical
 *	map, and may be referenced by the
 *	hardware.
 *
 *	If the size specified is non-zero,
 *	the map will be used in software only, and
 *	is bounded by that size.
 */
pmap_t pmap_create(size)
	vm_size_t	size;
{
	register pmap_t			p;
	register pmap_statistics_t	stats;

	/*
	 *	A software use-only map doesn't even need a map.
	 */

	if (size != 0) {
		return(PMAP_NULL);
	}

/*
 *	Allocate a pmap struct from the pmap_zone.  Then allocate
 *	the page descriptor table from the pd_zone.
 */

	p = (pmap_t) zalloc(pmap_zone);
	if (p == PMAP_NULL) {
		panic("pmap_create: cannot allocate a pmap");
	}
	if ((p->cr3 = (pt_entry_t *)kmem_alloc(kernel_map, I386_PGBYTES)) == PT_ENTRY_NULL)
		panic("pmap_create:cannot allocate page descriptor table");

	bcopy(kpde, p->cr3, I386_PGBYTES);
	p->ref_count = 1;

	simple_lock_init(&p->lock);
	p->cpus_using = 0;

	/*
	 *	Initialize statistics.
	 */

	stats = &p->stats;
	stats->resident_count = 0;
	stats->wired_count = 0;

	return(p);
}

/*
 *	Retire the given physical map from service.
 *	Should only be called if the map contains
 *	no valid mappings.
 */

void pmap_destroy(p)
	register pmap_t	p;
{
	register pt_entry_t	*pdep;	/* page directory entry */
	register vm_offset_t	pa;
	register int		c, s;
	register pv_entry_t	pv_h;

	if (p == PMAP_NULL)
		return;

	SPLVM(s);
	lock_pmap(p);
	c = --p->ref_count;
	unlock_pmap(p);
	SPLX(s);

	if (c == 0) {
		/*
		 *	Free the memory maps, then the
		 *	pmap structure.
		 */
		for (pdep = p->cr3;
			pdep < pmap_pde(p, VM_MIN_KERNEL_ADDRESS); pdep++) {
			if (pdep->valid == 0)
				continue;
			pa = i386_ptob(pdep->pfn);
			pv_h = pai_to_pvh(pa_index(pa));
			while (pv_h != PV_ENTRY_NULL && pv_h->pmap != kernel_pmap)
				pv_h = pv_h->next;
			if (pv_h == PV_ENTRY_NULL)
				panic("pmap_destroy:can't find pt table");
			kmem_free(kernel_map, pv_h->va, PAGE_SIZE);
		}
		kmem_free(kernel_map, p->cr3, I386_PGBYTES);
		zfree(pmap_zone, (vm_offset_t) p);
	}
}

/*
 *	Add a reference to the specified pmap.
 */

void pmap_reference(p)
	register pmap_t	p;
{
	int	s;
	if (p != PMAP_NULL) {
		SPLVM(s);
		lock_pmap(p);
		p->ref_count++;
		unlock_pmap(p);
		SPLX(s);
	}
}

/*
 *	Remove a range of 80386 page-table entries.
 *	The entries given are the first (inclusive)
 *	and last (exclusive) entries for the VM pages.
 *	The virtual address is the va for the first pte.
 *
 *	The pmap must be locked.
 *	If the pmap is not the kernel pmap, the range must lie
 *	entirely within one pte-page.  This is NOT checked.
 *	Assumes that the pte-page exists.
 *
 * 	Returns a linked list of pv_entries that caller must
 *	free after dropping the locks.
 */

/* static */
pv_entry_t
pmap_remove_range(pmap, va, e)
	pmap_t			pmap;
	vm_offset_t		va;
	vm_offset_t		e;
{
	register pt_entry_t	*cpte, *lpte;
	register pv_entry_t	pv_h;
	pv_entry_t		prev, cur;
	pv_entry_t		free_pv = PV_ENTRY_NULL;
	int			pfn;
	int			num_removed, num_unwired;
	register int		i;
	int			pai;
	vm_offset_t		pa;
	vm_offset_t		n;

#if	DEBUG_PTE_PAGE
	if (pmap != kernel_pmap)
		ptep_check(get_pte_page(spte));
#endif	DEBUG_PTE_PAGE
	num_removed = 0;
	num_unwired = 0;

	while (va < e) {
	    if ((n = round_i386seg(va+1)) > e)
		n = e;
	    if ((cpte = pmap_pte(pmap, va)) == PT_ENTRY_NULL) {
		/*
		 *  Can only get here if the level 2 page
		 *  is missing.  Can't happen to kernel pmap.
		 */
		if (pmap == kernel_pmap) {
			printf("virtual address - 0x%x\n", va);
			panic("pmap_protect:pte missing from kernel pmap");
		}
		va = n;
		continue;
	    } else for (; va < n; cpte++, va+=PAGE_SIZE) {
		pfn = cpte->pfn;
		if (pfn == 0)
			continue;

		num_removed++;
		if (cpte->wired)
			num_unwired++;

		pa = i386_ptob(pfn);
		if (!(managed_phys(pa))) {
			/*
			 *	Outside range of managed physical memory.
			 */
			continue;
		}

		pai = pa_index(pa);
		LOCK_PVH(pai);

		/*
		 *	Get the modify bits.
		 */
		i = ptes_per_vm_page;
		lpte = cpte;
		do {
			if (lpte->modify) {
#if	DEBUG
				vm_page_t	m;

				m = PHYS_TO_VM_PAGE(pa);
				if (m->object->read_only) {
					printf("pte = 0x%x pmap = 0x%x object = 0x%x offset = 0x%x va = 0x%x m=0x%x\n",
						lpte, pmap, m->object, m->offset, va, m);
					panic("pmap_remove_pte_page: page modified that shouldn't be!");
					}
#endif	DEBUG
				phys_attributes[pai] = PHYS_MODIFIED;
			}
			*(int *)lpte++ = 0;
		} while (--i > 0);

		/*
		 *	Remove the mapping from the pvlist for
		 *	this physical page.
		 */
		pv_h = pai_to_pvh(pai);
		if (pv_h->pmap == PMAP_NULL) {
			panic("pmap_remove: null pv_list!");
		}
		if (pv_h->va == va && pv_h->pmap == pmap) {
			/*
			 * Header is the pv_entry.  Copy the next one
			 * to header and free the next one (we can't
			 * free the header)
			 */
			cur = pv_h->next;
			if (cur != PV_ENTRY_NULL) {
				*pv_h = *cur;
				cur->next = free_pv;
				free_pv = cur;
			}
			else {
			    	pv_h->pmap = PMAP_NULL;
			}
		} else {
			prev = pv_h;
			while ((cur = prev->next) != PV_ENTRY_NULL) {
				if (cur->va == va &&
				    cur->pmap == pmap)
					break;
				prev = cur;
			}
			if (cur == PV_ENTRY_NULL)
				panic("pmap-remove: mapping not in pv_list!");
			prev->next = cur->next;
			cur->next = free_pv;
			free_pv = cur;
		}
		UNLOCK_PVH(pai);
	    }
	}

	/*
	 *	Update the counts
	 */
	pmap->stats.resident_count -= num_removed;
	pmap->stats.wired_count -= num_unwired;
	return(free_pv);
}

/*
 *	Remove the given range of addresses
 *	from the specified map.
 *
 *	It is assumed that the start and end are properly
 *	rounded to the 80386 page size.
 */

void pmap_remove(map, s, e)
	pmap_t		map;
	vm_offset_t	s, e;
{
	pv_entry_t		free_pv, cur;
	int			spl;

	if (map == PMAP_NULL)
		return;

	READ_LOCK(spl);
	lock_pmap(map);

	/*
	 *	Invalidate the translation buffer first
	 */
	PMAP_UPDATE_TLBS(map, s, e);

	free_pv = pmap_remove_range(map, s, e);

	unlock_pmap(map);
	READ_UNLOCK(spl);

	/*
	 *      Free any pv entries that pmap_remove_range returned.
	 */

	while (free_pv != PV_ENTRY_NULL) {
		cur = free_pv;
		free_pv = free_pv->next;
		zfree(pv_list_zone, (vm_offset_t) cur);
	}
}

/*
 *	Routine:	pmap_remove_all
 *	Function:
 *		Removes this physical page from
 *		all physical maps in which it resides.
 *		Reflects back modify bits to the pager.
 */
void pmap_remove_all(phys)
	vm_offset_t	phys;
{
	pv_entry_t		pv_h, cur;
	register pt_entry_t	*pte;
	register pt_entry_t	*cpte;
	int			pai;
	register int		i;
	register vm_offset_t	va;
	register pmap_t		pmap;
	int			spl;

	if (!(managed_phys(phys)))
		/*
		 *	Not a managed page.
		 */
		return;

	/*
	 *	Lock the pmap system first, since we will be changing
	 *	several pmaps.
	 */

	WRITE_LOCK(spl);

	/*
	 *	Walk down PV list, removing all mappings.
	 *	We have to do the same work as in pmap_remove_pte_page
	 *	since that routine locks the pv_head.  We don't have
	 *	to lock the pv_head, since we have the entire pmap system.
	 */
	pai = pa_index(phys);
	pv_h = pai_to_pvh(pai);

	while ( (pmap = pv_h->pmap) != PMAP_NULL) {
		va = pv_h->va;

		lock_pmap(pmap);

		pte = pmap_pte(pmap, va);

		if (pte->sw_valid == 0)
			panic("pmap_remove_all: pte invalid");

		if (i386_ptob(pte->pfn) != phys) {
			panic("pmap_remove_all: pte doesn't point to page!");
		}

		pmap->stats.resident_count--;
		if (pte->wired) {
			panic("pmap_remove_all removing a wired page");
		}
		/*
		 *	Tell CPU using pmap to invalidate its TLB
		 */
		PMAP_UPDATE_TLBS(pmap, va, va + PAGE_SIZE);

		if ((cur = pv_h->next) != PV_ENTRY_NULL) {
			*pv_h = *cur;
			zfree(pv_list_zone, (vm_offset_t) cur);
		}
		else
			pv_h->pmap = PMAP_NULL;

		i = ptes_per_vm_page;
		cpte = pte;
		do {
			if (cpte->modify) {
#if	DEBUG
				vm_page_t m = PHYS_TO_VM_PAGE(phys);
				if (m->object->read_only) {
					printf("pte = 0x%x pmap = 0x%x object = 0x%x offset = 0x%x va=0x%x m=0x%x\n",
						pte, pmap, m->object, m->offset, va, m);
					panic("pmap_remove_all: page modified that shouldn't be!");
				}
#endif	DEBUG

				phys_attributes[pai] = PHYS_MODIFIED;
			}
			*(int *)cpte = 0;
			cpte++;
		} while (--i > 0);
		unlock_pmap(pmap);
	}

	WRITE_UNLOCK(spl);
}

/*
 *	Routine:	pmap_copy_on_write
 *	Function:
 *		Remove write privileges from all
 *		physical maps for this physical page.
 */
void pmap_copy_on_write(phys)
	vm_offset_t	phys;
{
	register pv_entry_t	pv_e;
	register pt_entry_t	*pte;
	register int		i;
	int			spl;
	pt_entry_t		oldpte;

	/*
	 *	Lock the entire pmap system, since we may be changing
	 *	several maps.
	 */
	WRITE_LOCK(spl);

	pv_e = pai_to_pvh(pa_index(phys));
	if (pv_e->pmap == PMAP_NULL) {
		WRITE_UNLOCK(spl);
		return;		/* no mappings */
	}

	/*
	 *	Run down the list of mappings to this physical page,
	 *	disabling write privileges on each one.
	 */

	while (pv_e != PV_ENTRY_NULL) {
		pmap_t		pmap;
		vm_offset_t	va;

		pmap = pv_e->pmap;
		va = pv_e->va;

		lock_pmap(pmap);

		pte = pmap_pte(pmap, va);

		if (pte == PT_ENTRY_NULL || pte->sw_valid == 0)
			panic("pmap_copy_on_write: invalid pte");

		/*
		 *	Ask cpus using pmap to invalidate their TLBs
		 */
		PMAP_UPDATE_TLBS(pmap, va, va + PAGE_SIZE);

		if (pte->prot == I386_UW) {
			/*
			 *	User write becomes user read
			 */
			i = ptes_per_vm_page;
			do {
				oldpte = *pte;
				i386_protection(pmap, VM_PROT_READ,
						(int)pte->sw_valid, pte);
				cor_stats(&oldpte, pte);
				cor_stats2(pmap, VM_PROT_READ);
				pte++;
			} while (--i > 0);
		}
		unlock_pmap(pmap);

		pv_e = pv_e->next;
	}

	WRITE_UNLOCK(spl);
}

/*
 *	Set the physical protection on the
 *	specified range of this map as requested.
 */
void pmap_protect(map, s, e, prot)
	pmap_t		map;
	vm_offset_t	s, e;
	vm_prot_t	prot;
{
	register pt_entry_t	*pte;
	vm_offset_t		n;
	int			spl;
	pt_entry_t		oldpte;

	if (map == PMAP_NULL)
		return;

	SPLVM(spl);
	lock_pmap(map);

	/*
	 *	Invalidate the translation buffer first
	 */
	PMAP_UPDATE_TLBS(map, s, e);

	while (s < e) {
		if ((n = round_i386seg(s+1)) > e)
			n = e;
		if ((pte = pmap_pte(map, s)) == PT_ENTRY_NULL) {
			/*
			 *  Can only get here if the level 2 page
			 *  is missing.  Can't happen to kernel pmap.
			 */
			if (map == kernel_pmap) {
				printf("virtual address - 0x%x\n", s);
				panic("pmap_protect:pte missing from kernel pmap");
			}
			s = n;
			continue;
		} else for (; s < n; pte++, s+=PAGE_SIZE) {
			if (pte->sw_valid) {
				oldpte = *pte;
				i386_protection(map, prot, (int)pte->sw_valid,
						pte);
				cor_stats(&oldpte, pte);
				cor_stats2(map, prot);
			}
		}
	}

	unlock_pmap(map);
	SPLX(spl);
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
void pmap_enter(pmap, v, pa, prot, wired)
	register pmap_t		pmap;
	vm_offset_t		v;
	register vm_offset_t	pa;
	vm_prot_t		prot;
	boolean_t		wired;
{
	register pt_entry_t	*pte;
	register pv_entry_t	pv_h;
	register int		i, pai;
	pv_entry_t		pv_e, free_pv = PV_ENTRY_NULL;
	pt_entry_t		template;
	int			spl;
	vm_offset_t		old_pa;
	pt_entry_t		oldpte;

if (pmap_debug) printf("pmap(%x, %x)\n", v, pa);
	if (pmap == PMAP_NULL)
		return;

	/*
	 *	Must allocate a new pvlist entry while we're unlocked;
	 *	zalloc may cause pageout (which will lock the pmap system).
	 *	If we determine we need a pvlist entry, we will unlock
	 *	and allocate one.  Then we will retry, throughing away
	 *	the allocated entry later (if we no longer need it).
	 */
	pv_e = PV_ENTRY_NULL;
	*(int *)&template = 0;
Retry:

	READ_LOCK(spl);
	lock_pmap(pmap);

	/*
	 *	Expand pmap to include this pte.  Assume that
	 *	pmap is always expanded to include enough 80386
	 *	pages to map one VM page.
	 */

	while ((pte = pmap_pte(pmap, v)) == PT_ENTRY_NULL) {
		/*
		 *	Must unlock to expand the pmap.
		 */
		unlock_pmap(pmap);
		READ_UNLOCK(spl);

		pmap_expand(pmap, v);

		READ_LOCK(spl);
		lock_pmap(pmap);
	}
	/*
	 *	Special case if the physical page is already mapped
	 *	at this address.
	 */
	old_pa = i386_ptob(pte->pfn);
	if (old_pa == pa) {
	    /*
	     *	May be changing its wired attribute or protection
	     */
	    if (wired && !pte->wired)
		pmap->stats.wired_count++;
	    else if (!wired && pte->wired)
		pmap->stats.wired_count--;

	    i386_protection(pmap, prot, 1, &template);
	    template.pfn = i386_btop(pa);
	    if (wired)
		template.wired |= 1;
	    PMAP_UPDATE_TLBS(pmap, v, v + PAGE_SIZE);
	    i = ptes_per_vm_page;
	    do {
		if (pte->modify)
		    template.modify |= 1;
		oldpte = *pte;
		WRITE_PTE(pte, template)
		cor_stats(&oldpte, pte);
		cor_stats2(pmap, prot);
		pte++;
		template.pfn++;
	    } while (--i > 0);
	}
	else {

	    /*
	     *	Remove old mapping from the PV list if necessary.
	     */
	    if (old_pa != (vm_offset_t) 0) {
		/*
		 *	Invalidate the translation buffer,
		 *	then remove the mapping.
		 */
		PMAP_UPDATE_TLBS(pmap, v, v + PAGE_SIZE);

		/*
		 *	Don't free the pte page if removing last
		 *	mapping - we will immediately replace it.
		 */
		if (pv_e == PV_ENTRY_NULL) {
			pv_e = pmap_remove_range(pmap, v, v + i386_ptob(ptes_per_vm_page));
		}
		else {
			free_pv = pmap_remove_range(pmap, v, v + i386_ptob(ptes_per_vm_page));
		}
	    }

	    if (managed_phys(pa)) {

		/*
		 *	Enter the mapping in the PV list for this
		 *	physical page.
		 */

		pai = pa_index(pa);
		LOCK_PVH(pai);
		pv_h = pai_to_pvh(pai);

		if (pv_h->pmap == PMAP_NULL) {
		    /*
		     *	No mappings yet
		     */
		    pv_h->va = v;
		    pv_h->pmap = pmap;
		    pv_h->next = PV_ENTRY_NULL;
		}
		else {
#if	DEBUG
		    {
			/* check that this mapping is not already there */
			pv_entry_t	e = pv_h;
			while (e != PV_ENTRY_NULL) {
			    if (e->pmap == pmap && e->va == v)
				panic("pmap_enter: already in pv_list");
			    e = e->next;
			}
		    }
#endif	DEBUG
		    
		    /*
		     *	Add new pv_entry after header.
		     */
		    if (pv_e == PV_ENTRY_NULL) {
			UNLOCK_PVH(pai);
			unlock_pmap(pmap);
			READ_UNLOCK(spl);
			pv_e = (pv_entry_t) zalloc(pv_list_zone);
			goto Retry;
		    }
		    pv_e->va = v;
		    pv_e->pmap = pmap;
		    pv_e->next = pv_h->next;
		    pv_h->next = pv_e;
		    /*
		     *	Remember that we used the pvlist entry.
		     */
		    pv_e = PV_ENTRY_NULL;
		}
		UNLOCK_PVH(pai);
	    }

	    /*
	     *	And count the mapping.
	     */

	    pmap->stats.resident_count++;
	    if (wired)
		pmap->stats.wired_count++;

	    /*
	     *	Build a template to speed up entering -
	     *	only the pfn changes.
	     */
	    i386_protection(pmap, prot, 1, &template);
	    template.pfn = i386_btop(pa);
	    if (wired)
	    	template.wired |= 1;
	    i = ptes_per_vm_page;
	    do {
		oldpte = *pte;
		WRITE_PTE(pte, template)
		cor_stats(&oldpte, pte);
		cor_stats2(pmap, prot);
		pte++;
		template.pfn++;
	    } while (--i > 0);
	}

	unlock_pmap(pmap);
	READ_UNLOCK(spl);

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
void pmap_change_wiring(map, v, wired)
	register pmap_t	map;
	vm_offset_t	v;
	boolean_t	wired;
{
	register pt_entry_t	*pte;
	register int		i;
	int			spl;

	/*
	 *	We must grab the pmap system lock because we may
	 *	change a pte_page queue.
	 */
	READ_LOCK(spl);
	lock_pmap(map);

	if ((pte = pmap_pte(map, v)) == PT_ENTRY_NULL)
		panic("pmap_change_wiring: pte missing");

	if (wired && !pte->wired) {
		/*
		 *	wiring down mapping
		 */
		map->stats.wired_count++;
	}
	else if (!wired && pte->wired) {
		/*
		 *	unwiring mapping
		 */
		map->stats.wired_count--;
	}

	i = ptes_per_vm_page;
	do {
		(pte++)->wired = wired;
	} while (--i > 0);

	unlock_pmap(map);
	READ_UNLOCK(spl);
}

/*
 *	Routine:	pmap_extract
 *	Function:
 *		Extract the physical page address associated
 *		with the given map/virtual_address pair.
 */

vm_offset_t pmap_extract(pmap, va)
	register pmap_t	pmap;
	vm_offset_t	va;
{
	register pt_entry_t	*pte;
	register vm_offset_t	pa;
	int			spl;

	SPLVM(spl);
	lock_pmap(pmap);
	if ((pte = pmap_pte(pmap, va)) == PT_ENTRY_NULL)
		pa = (vm_offset_t) 0;
	else
		pa = kvtophys(ptetokv(*pte)) + (va & I386_OFFMASK);
	unlock_pmap(pmap);
	SPLX(spl);
	return(pa);
}

/*
 *	Routine:	pmap_expand
 *
 *	Expands a pmap to be able to map the specified virtual address.
 *
 *	Allocates new virtual memory for the P0 or P1 portion of the
 *	pmap, then re-maps the physical pages that were in the old
 *	pmap to be in the new pmap.
 *
 *	Must be called with the pmap system and the pmap unlocked,
 *	since these must be unlocked to use vm_allocate or vm_deallocate.
 *	Thus it must be called in a loop that checks whether the map
 *	has been expanded enough.
 *	(We won't loop forever, since page tables aren't shrunk.)
 */
pmap_expand(map, v)
	register pmap_t		map;
	register vm_offset_t	v;
{
	pt_entry_t		*ptep;	/* page table */
	pt_entry_t		*pdp;	/* page directory entry */
	int			spl;

	if (map == kernel_pmap) {
	    panic("You lose: system page table too small");
	}
	/*
	 *	Allocate a VM page for the level 2 page table entries
	 */
	if ((ptep = (pt_entry_t *) kmem_alloc(kernel_map, PAGE_SIZE)) == PT_ENTRY_NULL)
		return;
	/*
	 *	See if someone else expanded us first
	 */
	if (pmap_pte(map, v) != PT_ENTRY_NULL) {
		kmem_free(kernel_map, ptep, PAGE_SIZE);
		return;
	}
	/*
	 *	Set the page directory entry for this page table
	 */
	/* shouldn't this come before the "someone else" check? -mdk */
	READ_LOCK(spl);	
	lock_pmap(map);
	pdp = pmap_pde(map, v);
	pdp->pfn = i386_btop(kvtophys(ptep));
	pdp->prot = I386_UW;
	pdp->valid = 1;
	unlock_pmap(map);
	READ_UNLOCK(spl);
	return;
}

/*
 *	Copy the range specified by src_addr/len
 *	from the source map to the range dst_addr/len
 *	in the destination map.
 *
 *	This routine is only advisory and need not do anything.
 */
void pmap_copy(dst_pmap, src_pmap, dst_addr, len, src_addr)
	pmap_t		dst_pmap;
	pmap_t		src_pmap;
	vm_offset_t	dst_addr;
	vm_size_t	len;
	vm_offset_t	src_addr;
{
#ifdef	lint
	dst_pmap++; src_pmap++; dst_addr++; len++; src_addr++;
#endif	lint
}

/*
 *	Require that all active physical maps contain no
 *	incorrect entries NOW.  [This update includes
 *	forcing updates of any address map caching.]
 *
 *	Generally used to insure that a thread about
 *	to run will see a semantically correct world.
 */
void pmap_update()
{
	set_cr3(get_cr3());	/* invalidate TLB */
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
 */
void pmap_collect(p)
	pmap_t 		p;
{
	vm_offset_t	va;
	pt_entry_t	*pdp, *ptp, *ptep;
	int		spl, wired;
	pv_entry_t	free_pv = PV_ENTRY_NULL;
	pv_entry_t	cur;

	if (p == PMAP_NULL)
		return;

	if (p == kernel_pmap)
		panic("pmap_collect on kernel_pmap");

	/*
	 *	Garbage collect map.
	 */
	READ_LOCK(spl);
	lock_pmap(p);
	PMAP_UPDATE_TLBS(p, VM_MIN_ADDRESS, VM_MAX_ADDRESS);

	for (pdp = p->cr3; pdp < pmap_pde(p, VM_MIN_KERNEL_ADDRESS); pdp++) {
		if (pdp->valid == 0)
			continue;
		{
		    register vm_offset_t pa;
		    register pv_entry_t	pv_h;

		    pa = pte_to_pa(pdp);
		    pv_h = pai_to_pvh(pa_index(pa));
		    while (pv_h != PV_ENTRY_NULL && pv_h->pmap != kernel_pmap)
			pv_h = pv_h->next;
		    if (pv_h == PV_ENTRY_NULL)
			panic("pmap_collect: pte page at 0x%x not mapped",
				pa);
		    ptp = (pt_entry_t *)pv_h->va;
		}
		wired = 0;
		for (ptep = ptp; ptep < &ptp[NPTES]; ptep++)
			if (ptep->wired) {
				wired++;
				break;
			}
		if (!wired) {
			va = pdetova(pdp - p->cr3);
			free_pv = pmap_remove_range(p, va, va + pdetova(1));
			*(int *)pdp = 0;
			unlock_pmap(p);
			READ_UNLOCK(spl);
			kmem_free(kernel_map, (vm_offset_t)ptp, PAGE_SIZE);
			while (free_pv != PV_ENTRY_NULL) {
				cur = free_pv;
				free_pv = free_pv->next;
				zfree(pv_list_zone, (vm_offset_t)cur);
			}

			READ_LOCK(spl);
			lock_pmap(p);
		}
	}
	unlock_pmap(p);
	READ_UNLOCK(spl);
	return;

}

/*
 *	Routine:	pmap_activate
 *	Function:
 *		Binds the given physical map to the given
 *		processor, and returns a hardware map description.
 */
void pmap_activate(my_pmap, th, my_cpu)
	register pmap_t	my_pmap;
	thread_t	th;
	int		my_cpu;
{
	PMAP_ACTIVATE(my_pmap, th, my_cpu);
}


/*
 *	Routine:	pmap_deactivate
 *	Function:
 *		Indicates that the given physical map is no longer
 *		in use on the specified processor.  (This is a macro
 *		in pmap.h)
 */
void pmap_deactivate(pmap, th, which_cpu)
	pmap_t		pmap;
	thread_t	th;
	int		which_cpu;
{
#ifdef	lint
	pmap++; th++; which_cpu++;
#endif	lint
	PMAP_DEACTIVATE(pmap, th, which_cpu);
}

/*
 *	Routine:	pmap_kernel
 *	Function:
 *		Returns the physical map handle for the kernel.
 */
pmap_t pmap_kernel()
{
    	return (kernel_pmap);
}

/*
 *	pmap_zero_page zeros the specified (machine independent)
 *	page.  This routine is written entirely in assembly language,
 *	see i386/phys.s.
 */
#ifdef	i386
#else	i386
pmap_zero_page(phys)
	register vm_offset_t	phys;
{
	register int	i;

	i = PAGE_SIZE / I386_PGBYTES;
	phys = i386_pfn(phys);

	while (i--)
		zero_phys(phys++);
}

/*
 *	pmap_copy_page copies the specified (machine independent)
 *	pages.  This routine is (or should be) written entirely in 
 *	assembly language, see i386/phys.[cs].
 */
pmap_copy_page(src, dst)
	vm_offset_t	src, dst;
{
	int	i;

	i = PAGE_SIZE / I386_PGBYTES;

	while (i--) {
		copy_phys(i386_pfn(src), i386_pfn(dst));
		src += I386_PGBYTES;
		dst += I386_PGBYTES;
	}
}
#endif	i386

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
pmap_pageable(pmap, start, end, pageable)
	pmap_t		pmap;
	vm_offset_t	start;
	vm_offset_t	end;
	boolean_t	pageable;
{
#ifdef	lint
	pmap++; start++; end++; pageable++;
#endif	lint
}

#ifdef	USE_HDW
/*
 *	Clear the modify bits on the specified physical page.
 */
void pmap_clear_modify(phys)
	register vm_offset_t	phys;
{
	if (managed_phys(phys))
	    update_phys_attributes(phys, 0, PHYS_MODIFIED);
}

/*
 *	pmap_is_modified:
 *
 *	Return whether or not the specified physical page is modified
 *	by any physical maps.
 *	    
 */

boolean_t pmap_is_modified(phys)
	register vm_offset_t	phys;
{
	if (managed_phys(phys))
	    return update_phys_attributes(phys, PHYS_MODIFIED, 0);
	else
	    return (FALSE);
}

/*
 *	pmap_clear_reference:
 *
 *	Clear the reference bit on the specified physical page.
 */

void pmap_clear_reference(phys)
	vm_offset_t	phys;
{
	if (managed_phys(phys))
	    update_phys_attributes(phys, 0, PHYS_REFERENCED);
}

/*
 *	pmap_is_referenced:
 *
 *	Return whether or not the specified physical page is referenced
 *	by any physical maps.
 *
 */

boolean_t pmap_is_referenced(phys)
	vm_offset_t	phys;
{
	if (managed_phys(phys)) {
	    return update_phys_attributes(phys, PHYS_REFERENCED, 0);
	} else
	    return (FALSE);
}

update_phys_attributes(pa, check, clr)
	vm_offset_t	pa;
{
	pv_entry_t	pv_h;
	int		*pte;
	vm_offset_t	va;
	pmap_t		pmap;
	register int	i;
	int		pai;

	/*
	 *	Walk down PV list, let pmap_remove_all do all the hairy
	 *	consistency checks
	 */
	pai = pa_index(pa);
	pv_h = pai_to_pvh(pai);

	if (!clr)
		if ((phys_attributes[pai] & check) == check)
			return 1;
	while ((pmap = pv_h->pmap) != PMAP_NULL) {
	    va = pv_h->va;

	    lock_pmap(pmap);

	    i = ptes_per_vm_page;
	    do {
		pte = (int *)pmap_pte(pmap, va);
		phys_attributes[pai] |= *pte & PHYS_ATTRIBUTES;
		if (!clr) {
			if ((phys_attributes[pai] & check) == check) {
				unlock_pmap(pmap);
				return 1;
			}
		} else
			*pte &= ~clr;
		va += I386_PGBYTES;
	    } while (--i > 0);

	    unlock_pmap(pmap);

	    if ((pv_h = pv_h->next) == PV_ENTRY_NULL) {
	    	break;
	    }

	}
	phys_attributes[pai] &= ~clr;
	return ((phys_attributes[pai] & check) == check);
}
#else	USE_HDW
/*
 *	Clear the modify bits on the specified physical page.
 *
 *	This does not work if the physical page is in
 *	any maps
 */

void pmap_clear_modify(phys)
	register vm_offset_t	phys;
{
	if (managed_phys(phys))
		phys_attributes[pa_index(phys)] &= ~PHYS_MODIFIED;
}

/*
 *	pmap_is_modified:
 *
 *	Return whether or not the specified physical page is modified
 *	by any physical maps.
 *
 *	XXX this does not yet return complete information if the page
 *	    is in any pmaps - pmap_clear_reference should have just
 *	    been called
 *	    
 */

boolean_t pmap_is_modified(phys)
	register vm_offset_t	phys;
{
	if (managed_phys(phys))
	    return ((boolean_t)
	    		(phys_attributes[pa_index(phys)] & PHYS_MODIFIED));
	else
	    return (FALSE);
}

/*
 *	pmap_clear_reference:
 *
 *	Clear the reference bit on the specified physical page.
 *
 *	Should examine the pv table but the quick hack is to just
 *	remove all references
 */

void pmap_clear_reference(phys)
	vm_offset_t	phys;
{
	pmap_remove_all(phys);
}

/*
 *	pmap_is_referenced:
 *
 *	Return whether or not the specified physical page is referenced
 *	by any physical maps.
 *
 *	Should examine the pv table but the quick hack removed all
 *	references in pmap_clear_reference so do nothing here
 */

boolean_t pmap_is_referenced(phys)
	vm_offset_t	phys;
{
#ifdef	lint
	phys++;
#endif	lint
	return(FALSE);
}
#endif	USE_HDW

#if	NCPUS > 1
/*
*	    TLB Coherence Code (TLB "shootdown" code)
* 
* Threads that belong to the same task share the same address space and
* hence share a pmap.  However, they  may run on distinct cpus and thus
* have distinct TLBs that cache page table entries. In order to guarantee
* the TLBs are consistent, whenever a pmap is changed, all threads that
* are active in that pmap must have their TLB updated. To keep track of
* this information, the set of cpus that are currently using a pmap is
* maintained within each pmap structure (cpus_using). Pmap_activate() and
* pmap_deactivate add and remove, respectively, a cpu from this set.
* Since the TLBs are not addressable over the bus, each processor must
* flush its own TLB; a processor that needs to invalidate another TLB
* needs to interrupt the processor that owns that TLB to signal the
* update.
* 
* Whenever a pmap is updated, the lock on that pmap is locked, and all
* cpus using the pmap are signaled to invalidate. All threads that need
* to activate a pmap must wait for the lock to clear to await any updates
* in progress before using the pmap. They must ACQUIRE the lock to add
* their cpu to the cpus_using set. An implicit assumption made
* throughout the TLB code is that all kernel code that runs at or higher
* than splvm blocks out update interrupts, and that such code does not
* touch pageable pages.
* 
* A shootdown interrupt serves another function besides signaling a
* processor to invalidate. The interrupt routine (pmap_update_interrupt)
* waits for the both the pmap lock (and the kernel pmap lock) to clear,
* preventing user code from making implicit pmap updates while the
* sending processor is performing its update. (This could happen via a
* user data write reference that turns on the modify bit in the page
* table). It must wait for any kernel updates that may have started
* concurrently with a user pmap update because the IPC code
* changes mappings.
* Spinning on the VALUES of the locks is sufficient (rather than
* having to acquire the locks) because any updates that occur subsequent
* to finding the lock unlocked will be signaled via another interrupt.
* (This assumes the interrupt is cleared before the low level interrupt code 
* calls pmap_update_interrupt()). 
* 
* The signaling processor must wait for any implicit updates in progress
* to terminate before continuing with its update. Thus it must wait for an
* acknowledgement of the interrupt from each processor for which such
* references could be made. For maintaining this information, a set
* cpus_active is used. A cpu is in this set if and only if it can 
* use a pmap. When pmap_update_interrupt() is entered, a cpu is removed from
* this set; when all such cpus are removed, it is safe to update.
* 
* Before attempting to acquire the update lock on a pmap, a cpu (A) must
* be at least at the priority of the interprocessor interrupt
* (splip<=splvm). Otherwise, A could grab a lock and be interrupted by a
* kernel update; it would spin forever in pmap_update_interrupt() trying
* to acquire the user pmap lock it had already acquired. Furthermore A
* must remove itself from cpus_active.  Otherwise, another cpu holding
* the lock (B) could be in the process of sending an update signal to A,
* and thus be waiting for A to remove itself from cpus_active. If A is
* spinning on the lock at priority this will never happen and a deadlock
* will result.
*/

/*
 *	Signal another CPU that it must flush its TLB
 */
void    signal_cpus(use_list, pmap, start, end)
	cpu_set		use_list;
	pmap_t		pmap;
	vm_offset_t	start, end;
{
	register int		which_cpu, j;
	register pmap_update_list_t	update_list_p;

	while ((which_cpu = ffs(use_list)) != 0) {
	    which_cpu -= 1;	/* convert to 0 origin */

	    update_list_p = &cpu_update_list[which_cpu];
	    simple_lock(&update_list_p->lock);

	    j = update_list_p->count;
	    if (j >= UPDATE_LIST_SIZE) {
		/*
		 *	list overflowed.  Change last item to
		 *	indicate overflow.
		 */
		update_list_p->item[UPDATE_LIST_SIZE-1].pmap  = kernel_pmap;
		update_list_p->item[UPDATE_LIST_SIZE-1].start = VM_MIN_ADDRESS;
		update_list_p->item[UPDATE_LIST_SIZE-1].end   = VM_MAX_KERNEL_ADDRESS;
	    }
	    else {
		update_list_p->item[j].pmap  = pmap;
		update_list_p->item[j].start = start;
		update_list_p->item[j].end   = end;
		update_list_p->count = j+1;
	    }
	    cpu_update_needed[which_cpu] = TRUE;
	    simple_unlock(&update_list_p->lock);

	    if ((cpus_idle & (1 << which_cpu)) == 0)
		interrupt_processor(which_cpu);
	    use_list &= ~(1 << which_cpu);
	}
}

void process_pmap_updates(my_pmap)
	register pmap_t		my_pmap;
{
	register int		my_cpu = cpu_number();
	register pmap_update_list_t	update_list_p;
	register int		j;
	register pmap_t		pmap;

	update_list_p = &cpu_update_list[my_cpu];
	simple_lock(&update_list_p->lock);

	for (j = 0; j < update_list_p->count; j++) {
	    pmap = update_list_p->item[j].pmap;
	    if (pmap == my_pmap ||
		pmap == kernel_pmap) {

		INVALIDATE_TLB(update_list_p->item[j].start,
				update_list_p->item[j].end);
	    }
	}
	update_list_p->count = 0;
	cpu_update_needed[my_cpu] = FALSE;
	simple_unlock(&update_list_p->lock);
}

/*
 *	Interrupt routine for TBIA requested from other processor.
 */
void pmap_update_interrupt()
{
	register int		my_cpu;
	register pmap_t		my_pmap;
	int			s;

	my_cpu = cpu_number();

	/*
	 *	Exit now if we're idle.  We'll pick up the update request
	 *	when we go active, and we must not put ourselves back in
	 *	the active set because we'll never process the interrupt
	 *	while we're idle (thus hanging the system).
	 */
	if (cpus_idle & (1 << my_cpu))
	    return;

	if (current_thread() == THREAD_NULL)
	    my_pmap = kernel_pmap;
	else {
	    my_pmap = current_pmap();
	    if (!pmap_in_use(my_pmap, my_cpu))
		my_pmap = kernel_pmap;
	}

	/*
	 *	Raise spl to splvm (above splip) to block out pmap_extract
	 *	from IO code (which would put this cpu back in the active
	 *	set).
	 */
	s = splvm();

	do {

	    /*
	     *	Indicate that we're not using either user or kernel
	     *	pmap.
	     */
	    i_bit_clear(my_cpu, &cpus_active);

	    /*
	     *	Wait for any pmap updates in progress, on either user
	     *	or kernel pmap.
	     */
	    while (my_pmap->lock.lock_data || kernel_pmap->lock.lock_data)
		continue;

	    process_pmap_updates(my_pmap);

	    i_bit_set(my_cpu, &cpus_active);

	    if (my_pmap != kernel_pmap) {
		mtpr(P0BR, my_pmap->p0br);
		mtpr(P0LR, my_pmap->p0lr);
		mtpr(P1BR, my_pmap->p1br);
		mtpr(P1LR, my_pmap->p1lr);
	    }
	} while (cpu_update_needed[my_cpu]);
	
	splx(s);
}
#else	NCPUS > 1
/*
 *	Dummy routine to satisfy external reference.
 */
void pmap_update_interrupt()
{
	/* should never be called. */
}
#endif	NCPUS > 1

/*
 *	pmap_page_protect:
 *
 *	Lower the permission for all mappings to a given page.
 */
void	pmap_page_protect(phys, prot)
	vm_offset_t	phys;
	vm_prot_t	prot;
{
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
 * kernacc - ??? (XXX)
 */
boolean_t
kernacc(base, len, op)
	vm_offset_t base;
	unsigned long len;
	int op;
{
	vm_offset_t end = base + len;
	pt_entry_t *pte;
	pmap_t map = current_pmap();
	
	if (map == 0)
		return(FALSE);

	base = trunc_page(base);
	while (base < end) {
		pte = pmap_pte(map, base);
		if (pte == PT_ENTRY_NULL)
			return(FALSE);
		if (pte->sw_valid == 0)
			return(FALSE);
		base += PAGE_SIZE;
	}   
	return(TRUE);
}

/*
 *	Check to see if user has write privilege.  Necessary
 *	since the 80386 ALWAYS gives the kernel write access
 *	to all pages.  Returns 0 if okay, < 0 if not.
 *	XXX - this probably needs some cleaning up. -mdk
 */
userwrite(v, l)
	vm_offset_t v;
	unsigned int l;
{
	register vm_map_t	map = current_thread()->task->map;
	register pmap_t		pmap;
	register pt_entry_t	*pte;

	if (map == 0)
		return(-1);
	if ((pmap = map->pmap) == 0)
		return(-1);
	v = trunc_page(v);
	while (l) {
		if ((pte = pmap_pte(pmap, v)) == PT_ENTRY_NULL ||
		    pte->sw_valid == 0 || 
		    ((pte->prot & I386_UW) != I386_UW)) {
			if (vm_fault(map, v,
				     VM_PROT_READ|VM_PROT_WRITE, FALSE) !=
			    KERN_SUCCESS)
			return (-1);
			
		}
		v += I386_PGBYTES;
		if (l > I386_PGBYTES)
			l -= I386_PGBYTES;
		else
			l = 0;
	}
	return(0);
}

/* 
 * Another back-door routine, this time for implementing copy on 
 * reference.  The given range of memory should be in core, but was 
 * marked invalid by the pmap code because it was read-only.  What we 
 * do here is turn on the hardware valid bit.  (This gives the kernel 
 * read-write access to the memory as far as the hardware is
 * concerned.  If there is a bug where the kernel tries to write into
 * logically read-only memory, well, that's just one type of bug that
 * we can't catch on an i386.)
 * 
 * We return FALSE here if for some reason the memory isn't in core.  
 * This could happen on a multiprocessor.  It's up to the caller to 
 * fault the memory again and try again.
 */

boolean_t
pmap_mark_valid(start)
	vm_offset_t start;		/* page that should be enabled */
{
	register pt_entry_t	*pte;
	pt_entry_t		oldpte;
	boolean_t	ok = FALSE;
	int		spl;

	start = trunc_page(start);

	SPLVM(spl);
	lock_pmap(kernel_pmap);
	/* No need to invalidate the TLB. */

	if ((pte = pmap_pte(kernel_pmap, start)) != PT_ENTRY_NULL
	    && pte->sw_valid) {
		oldpte = *pte;
		pte->valid = 1;
		pte->prot = I386_UR;	/* helps with debugging? */
		cor_stats(&oldpte, pte);
		ok = TRUE;
	}

	unlock_pmap(kernel_pmap);
	SPLX(spl);

	return(ok);
}


/* 
 * Routines that use or manipulate the dev_addr array.
 */

/* end of dev_addr routines */
