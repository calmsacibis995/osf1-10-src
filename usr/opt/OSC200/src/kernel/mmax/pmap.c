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
static char	*sccsid = "@(#)$RCSfile: pmap.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:30 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 *	File:	pmap.c
 *	Author:	Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Multimax Physical Map management code.
 *	(Some routines are still dummy routines).
 *
 *  Original Author  Jim Van Sciver (jvs) at Encore
 *	Derived from Vax version.
 *
 *  Modifications    David L. Black (dlb) at Carnegie-Mellon University
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

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_kdb.h>
#include <mmax_debug.h>

#include <cpus.h>
#include <xpr_debug.h>

#include <mach/boolean.h>
#include <sys/types.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <kern/assert.h>

#include <kern/lock.h>

#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <mach/vm_prot.h>

#include <mmax/pmap.h>
#include <mach/mmax/vm_param.h>
#include <mmax/pcb.h>
#include <mmax/pte.h>
#include <mmax/mmu.h>
#include <mmax/inline_lock.h>

#if	XPR_DEBUG
#include <kern/xpr.h>
#define XPR_PMAP 0x40
#endif	XPR_DEBUG

#include <mmax/mtpr.h>

/* assume NO MMU bug */
#define	MBUG(prot, pte)

/*
 * This is for mapping the scc's and dpc's control blocks.
 */
#include <mmaxio/crqdefs.h>
#include <mmax/cpudefs.h>
#include <mmax/sccdefs.h>
#include <sys/table.h>
/*
 *	Array of per-processor interrupt vectors for tlb shootdown.
 */
board_vbxmit_t	tlb_vector[NCPUS];

extern	int	nproc;

/*
 *	kernel_pmap_store, kernel_pmap, pmap_lock --> pmap.h
 */
vm_offset_t	ptbr;			/* Physical address of kernel pmap. */
struct zone	*pmap_zone;		/* zone of pmap structures 	*/

#ifdef	multimax
/*
 * Temporary hack:  Even though kdb isn't useful on a multimax
 * and we don't bother to compile it, we still need these
 * definitions for dbmon.  One day, of course, there should be
 * a global switch so that we don't bother to build dbmon for
 * production kernels.
 */
#define	MMAX_DBMON	1
#endif

#if	MMAX_KDB || MMAX_DBMON
vm_offset_t	debug_vaddr;
pt_entry_t	*debug_pte;
#endif



/*
 *	For each vm_page_t, there is a list of all currently
 *	valid virtual mappings of that page.  An entry is
 *	a pv_entry_t; the list is the pv_table.  [Indexing on the
 *	pv_table is identical to the vm_page_array.]
 */

typedef struct pv_entry {
	struct pv_entry	*next;		/* next pv_entry */
	pmap_t		pmap;		/* pmap where mapping lies */
	vm_offset_t	va;		/* virtual address for mapping */
} *pv_entry_t;

#define PV_ENTRY_NULL	((pv_entry_t) 0)

pv_entry_t	pv_head_table;		/* array of entries, one per page */
zone_t		pv_list_zone;		/* zone of pv_entry structures */

/*
 *	Each entry in the pv_head_table is locked by a byte in the
 *	pv_lock_table.  The locks are accessed by the physical
 *	address of the page they lock.
 */

char	*pv_lock_table;		/* pointer to array of bits */
#define pv_lock_table_size(n)	(n)

/*
 * Byte locking macros for use with GNU-C. Otherwise these are
 * replaced by the inline'r
 */
#if	__GNUC__
/* A byte lock is just another case of a simple_lock */
extern __inline__ byte_lock(l)
        register char *l;
{
        _simple_lock(l);
}

extern __inline__ byte_unlock(l)
        register char *l;
{
        *l = 0;
}

#endif	/* __GNUC__ */

/*
 *	First and last physical addresses that we maintain pv and modify
 *	information for.  Initialized to zero to make sure that structures
 *	aren't touched before they are allocated.
 */

vm_offset_t	vm_first_phys = (vm_offset_t) 0;
vm_offset_t	vm_last_phys  = (vm_offset_t) 0;

boolean_t	pmap_initialized = FALSE;	/* Has pmap_init completed? */

#define pa_index(pa)		atop(pa - vm_first_phys)

#define pa_to_pvh(pa)		(&pv_head_table[pa_index(pa)])
#define lock_pvh_pa(pa)		{ byte_lock(pv_lock_table + pa_index(pa)); }
#define unlock_pvh_pa(pa)	{ byte_unlock(pv_lock_table + pa_index(pa)); }

/*
 *	Array of modify bits, one byte per physical page (to avoid expense
 *	of locking).
 */

char *pmap_modify_list;

/*
 *	Locking Protocols:
 *
 *	There are two structures in the pmap module that need locking:
 *	the pmap's themselves, and the per-page pv_lists (which are locked
 *	by locking the  pv_lock_table entry that corresponds the to pv_head
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
 */

/*
 *	lock and unlock macros for the pmap system.  Interrupts must be locked
 *	out because mbufs can be allocated at interrupt level and cause
 *	a pmap_enter.  (Fix from dbg)
 */

#define READ_LOCK(s)		s = splvm(); \
				CPU_SET_REMOVE(cpu_number(),cpus_active);\
				lock_read(&pmap_lock);
#define READ_UNLOCK(s)		lock_read_done(&pmap_lock);\
				CPU_SET_ADD(cpu_number(),cpus_active);splx(s);

#define WRITE_LOCK(s)		s = splvm();\
				CPU_SET_REMOVE(cpu_number(),cpus_active);\
				lock_write(&pmap_lock);
#define WRITE_UNLOCK(s)		lock_write_done(&pmap_lock);\
				CPU_SET_ADD(cpu_number(),cpus_active);splx(s);

/*
 *	Chip feature workaround.  We would like to access the
 *	pfn of a (pt_entry *) pte by using  	pte->pfn   .
 *	Unfortunately a pfn is the 23 high bits of a pte, and the chip
 *	insists on accessing bit fields as double words starting at the
 *	first byte of the field.  Hence bit field instructions cause a
 *	read-modify-write to the upper 3 bytes of pte plus
 *	the first byte following it.  For the last pte on a page
 *	this can cause an illegal page fault.  Hence we workaround
 *	by doing the field insert/extract by hand.  This also allows
 *	us to get/set physical addresses in a pte instead of pfn's.
 */

#define		get_physaddr(pte)	((vm_offset_t) \
		(*(template_t *) pte).bits & PG_PFNUM)
#define splip splvm

/*
 *	TLB invalidation
 */

#define pmap_in_use(pmap, cpu)	(IN_CPU_SET(cpu,(pmap)->cpus_using))

/*
 *   Invalidate the appropriate portion of the necessary TLBs.
 *   Assumes the pmap lock is held.
 */

#define INVAL_TLBS(pmap,start,end) \
{ \
 \
	int local; \
 \
	    if (pmap_in_use(pmap,cpu_number())) { \
		INVALIDATE_TLB((pmap),(start),(end)); \
		local = 1; \
	    }\
	    else\
		local = 0;\
	    /* If necessary, signal other cpus using the pmap */ \
	    if ((pmap)->nactive - local > 0)\
	    	signal_cpus((pmap),(start),(end)); \
 \
}

/* invalidate the appropriate portion of the local TLB */

#if	MMAX_XPC || MMAX_APC
/*
 *	32 entry tlb @ 4k bytes per entry.
 */

#define MAX_TBIS_SIZE	8		/* TBIA if more than 4 pages XXX */

#define INVALIDATE_TLB(pmap,start,end) { \
	if (pmap == kernel_pmap) { \
	    INVALIDATE_K_TLB(start,end); \
	} \
	else { \
	    INVALIDATE_U_TLB(start,end); \
	} \
}

#define INVALIDATE_K_TLB(start,end) { \
	if ((end) - (start) <= MAX_TBIS_SIZE*NS32K_PGBYTES) { \
	    register vm_offset_t tbi_va; \
	    for (tbi_va = (start); tbi_va < (end); tbi_va += NS32K_PGBYTES) \
	    	TBIS_K(tbi_va); \
	} \
	else { \
	    TBIA_K; \
	} \
}

#define INVALIDATE_U_TLB(start,end) { \
	if ((end) - (start) <= MAX_TBIS_SIZE*NS32K_PGBYTES) { \
	    register vm_offset_t tbi_va; \
	    for (tbi_va = (start); tbi_va < (end); tbi_va += NS32K_PGBYTES) \
	    	TBIS_U(tbi_va); \
	} \
	else { \
	    TBIA_U; \
	} \
}
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
/*
 *	16 entry tlb @ 512 bytes an entry  --> always invalidate all
 */
#define INVALIDATE_TLB(pmap,start,end) TBIA(pmap)
#define INVALIDATE_K_TLB(start,end) TBIA_K;
#define INVALIDATE_U_TLB(start,end) TBIA_U;
#endif	MMAX_DPC

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
 *	List of pmap updates.  If the buffer overflows,
 *	the last entry is changed to PMAP_NULL, which means invalidate all.
 */
struct pmap_update_list {
	simple_lock_data_t	lock;
	int			count;
	struct pmap_update_item	item[UPDATE_LIST_SIZE];
} ;

typedef	struct pmap_update_list	*pmap_update_list_t;

struct pmap_update_list	cpu_update_list[NCPUS];

/*
 *	Count shootdown interrupts saved.
 */

int	shoot_int_saved;

/*
 *	Useful constants:
 */

int		vm_page_ptes;	/* number of ptes needed to map a vm page */
vm_offset_t	pte_page_space; /* Amount of vm mapped by a vm page of ptes */
int		pte_page_space_mask;  /* mask to align to pte_page_space */
vm_offset_t	vm_page_bytes;	/* number of bytes needed to map a vm page */

/*
 *	For allocation of memory for first level ptes.
 */
vm_offset_t		pte1_free_list;
simple_lock_data_t	pte1_free_lock;
#if	MMAX_DPC
boolean_t		pte1_doing_alloc;
boolean_t		pte1_wakeup_needed;
#endif	MMAX_DPC

/*
 *	Assume that there are three protection codes, all using low bits.
 */

/*
 *	Initialize arrays that convert machine-independent protections to
 *	ns32000 protections.  Note that write access implies
 *	full access.  There are no write only pages.  Also note that
 *	the kernel always has at least read access.
 */

int	user_protection_codes[8];
int	kernel_protection_codes[8];

ns32k_protection_init()
{
	register int *kp, *up, prot;

	kp = kernel_protection_codes;
	up = user_protection_codes;
	for (prot = 0; prot < 8; prot++) {
		switch (prot) {
		/*
		 *	EXECUTE is equivalent to NONE because ns32k
		 *	does not support EXECUTE-only protection.
		 *	Also, WRITE without READ is NONE because ns32k
		 *	must grant READ to grant WRITE.
		 */
		case VM_PROT_NONE | VM_PROT_NONE | VM_PROT_NONE:
		case VM_PROT_NONE | VM_PROT_NONE | VM_PROT_EXECUTE:
		case VM_PROT_NONE | VM_PROT_WRITE | VM_PROT_NONE:
		case VM_PROT_NONE | VM_PROT_WRITE | VM_PROT_EXECUTE:
			*kp++ = NS32K_NO_ACCESS;
			*up++ = NS32K_NO_ACCESS;
			break;
		case VM_PROT_READ | VM_PROT_NONE | VM_PROT_NONE:
		case VM_PROT_READ | VM_PROT_NONE | VM_PROT_EXECUTE:
			*kp++ = NS32K_KR;
			*up++ = NS32K_UR;
			break;
		case VM_PROT_READ | VM_PROT_WRITE | VM_PROT_NONE:
		case VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE:
			*kp++ = NS32K_KW;
			*up++ = NS32K_UW;
			break;
		}
	}
}

/*
 *	Convert a map and a machine independent protection code to
 *	a ns32000 protection code.
 */

#define ns32k_protection(map,prot) \
	(((map) == kernel_pmap) ? kernel_protection_codes[prot] : \
				  user_protection_codes[prot])


/*
 *	Initialize the kernel's physical map.
 *
 *	This routine is only called at boot.  It is used to map a range of
 *	physical addresses into the kernel's virtual address space.  Memory
 *	management is either on or off.
 *
 *	This routine uses pmap_pte to return the address of the level two
 *	pte or NULL if the level one pte is invalid.  In the latter case
 *	the level one pte is set up to point to the next available group
 *	of level two ptes.  The kernel level two page tables immediately
 *	follow the level one page table.
 */
vm_offset_t pmap_map(virt, start, end, prot)
    vm_offset_t	virt;			/* Starting virtual address.	*/
    vm_offset_t	start;			/* Starting physical address.	*/
    vm_offset_t	end;			/* Ending physical address.	*/
    int		prot;			/* Machine indep. protection.	*/
{
    pt_entry_t	*pte;
    int		num_ns32k_pages;
    template_t	pte_template;
    int		kprot;

    kprot = ns32k_protection(kernel_pmap, prot);
    if (kprot == NS32K_NO_ACCESS) panic("pmap_map: Bad protection");

    pte_template.bits = PHYSTOPTE(start)|(kprot << PG_PROTOFF)| PG_R | PG_V;

    num_ns32k_pages = ns32k_btop(ns32k_round_page(end - start));
    for ( ; num_ns32k_pages > 0; num_ns32k_pages--, virt += NS32K_PGBYTES,
	pte_template.bits += NS32K_PGBYTES) {

	if ((pte = pmap_pte(kernel_pmap, virt)) == PT_ENTRY_NULL) {
	    printf("virtual address = 0x%x\n", virt);
	    panic("pmap_map: pte missing from kernel pmap!");
	}

	/*
	 *   Point the level two entry to the physical page and increment
	 *   the virtual and physical addresses by the National page size.
	 */
	*pte = pte_template.pte;

    }
    return(virt);
}

/*
 * pmap_populate_kernel_level1()
 *	Fill out kernel level-1 page-table.
 *
 */


/*
 * Number of 'extra' L1 and L2 entries we'll need to map in the SCC and other
 *  stuff in high memory if the full virtual address space was not specified
 *  (which it isn't!).
 */
#define EXTRA_L2	((0xffffffff - SCCMEM_BASE+1 +(MAPPED_PER_PTE2-1)) / MAPPED_PER_PTE2)
#define EXTRA_L1	((EXTRA_L2 + PTE2_ENTRIES-1) / PTE2_ENTRIES)

/*
 * Arguments:
 *	ptbr		- Address of Level 1 PTE
 *	num_level1	- Number of level1 entries we'll validate.
 *
 * Fill in the kernel level one page table,  only validate
 *  those entries which we'll actually use.
 */

static
pmap_populate_kernel_level1(ptbr, level2_map, num_level1)
vm_offset_t		ptbr;
int			num_level1;
{
	register pt_entry_t 	*pte1;

	/*
	 * Fill out kernel level-1 page-table, and virtual pointers to the
	 * level-2 mapping pages.
	 */

	for(pte1 = (pt_entry_t *) ptbr; pte1 < (pt_entry_t *) ptbr + num_level1;
	    pte1++, level2_map += NS32K_PGBYTES) {

		*(unsigned *) pte1 = PHYSTOPTE(level2_map) |
			PG_V | PG_R | PG_M | (NS32K_KW << PG_PROTOFF);

		*((vm_offset_t *) pte1 + PTE1_ENTRIES) =
			level2_map + VM_MIN_KERNEL_ADDRESS;
	}
}

/*
 *	Bootstrap the system enough to run with virtual memory.
 *	Map the kernel's code and data, and allocate the system page table.
 *	Called with mapping OFF.
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
void pmap_bootstrap(avail_start, avail_end, virtual_avail, virtual_end)

	vm_offset_t	*avail_start;	/* IN/OUT */
	vm_offset_t	*avail_end;	/* IN/OUT */
	vm_offset_t	*virtual_avail;	/* OUT */
	vm_offset_t	*virtual_end;	/* IN/OUT */
{
	long			map_size;
	vm_offset_t		i, vaddr;
	vm_offset_t		*pte1;
	vm_offset_t		s_text, e_text;
	int			num_level2;
	vm_offset_t		sccpage;
	vm_offset_t		*sccpte1;

	extern int		start_text, etext;

	/*
	 *	Initialize protection code arrays and useful constants.
	 */
	ns32k_protection_init();
	shoot_int_saved = 0;
#if	MACH
	pte1_free_list = (vm_offset_t) 0;
	simple_lock_init(&pte1_free_lock);
#if	MMAX_DPC
	pte1_doing_alloc = FALSE;
	pte1_wakeup_needed = FALSE;
#endif	MMAX_DPC
#endif	MACH
	vm_page_ptes = ns32k_btop(PAGE_SIZE);
	vm_page_bytes = vm_page_ptes * sizeof(pt_entry_t);
	pte_page_space =
		(vm_offset_t)ns32k_ptob(PAGE_SIZE/sizeof(pt_entry_t));
	pte_page_space_mask = ~(pte_page_space - 1);


	/*
	 *	The kernel's pmap is statically allocated so we don't
	 *	have to use pmap_create, which is unlikely to work
	 *	correctly at this part of the boot sequence.
	 */

	kernel_pmap = &kernel_pmap_store;

	lock_init(&pmap_lock, FALSE);
	simple_lock_init(&kernel_pmap->lock);

	/*
	 *	Allocate the kernel page table from the front of available
	 *	physical memory.  The level one map must start on a 1K (DPC)
	 *	or 4K (APC) boundary.  The level two map starts immediately
	 *      after the level one entries.
	 */

	if ((*avail_start & ~VA_PTBMASK) != 0)
	    *avail_start += PTE1_SIZE;
	*avail_start &= VA_PTBMASK;

	ptbr = kernel_pmap->pte1_addr = kernel_pmap->ptbr = *avail_start;
	/*
	 *	Figure out maximum kernel address, then figure out how
	 *	many level 2 PTE's we'll need.  Kernel virtual space is:
	 *		- at least twice physical memory
	 *		- at least VM_MIN_KERNEL_SPACE
	 *		- limited by VM_MAX_KERNEL_ADDRESS
	 */
	*virtual_end = VM_MIN_KERNEL_ADDRESS + 2*(*avail_end);
	if (*virtual_end < VM_MIN_KERNEL_SPACE)
	    *virtual_end = VM_MIN_KERNEL_SPACE;
	if (*virtual_end > VM_MAX_KERNEL_ADDRESS)
	    *virtual_end = VM_MAX_KERNEL_ADDRESS;

	num_level2 = ((*virtual_end+MAPPED_PER_PTE2-1) / MAPPED_PER_PTE2);

	/*
	 * Map size is large enough to hold two full L1 Page tables,
	 *  whatever we need (given the maximum kernel virtual address)
	 *  for  the L2 page tables, plus estra pages for the L2
	 *  entries which map the SCC (and other stuff in high memory).
	 */
	map_size = 2 * PTE1_SIZE + num_level2 * sizeof(struct pte);

	/*
	 * Add in extra L2 pages and Round to page boundary
	 */
	map_size = ((map_size + VA_OFFMASK) + BYTES_PER_PTE2*(EXTRA_L1*PTE2_ENTRIES)) & VA_PT2MASK;
	*avail_start += map_size;
	sccpage = *avail_start - BYTES_PER_PTE2*(EXTRA_L1*PTE2_ENTRIES);

	kernel_pmap->ref_count = 1;
	blkclr(ptbr, map_size);

	/*
	 * Populate the level 1 page table
	 */
	pmap_populate_kernel_level1(ptbr, ptbr + 2*PTE1_SIZE, (num_level2+PTE2_ENTRIES-1) / PTE2_ENTRIES);

	/*
	 *	Map low system memory, system text, system data/bss/symbols
	 *
	 *	&start_text and &etext can't have arithmetic done on them
	 *	directly because they must be physical addresses.
	 */

	s_text = (vm_offset_t) &start_text;
	e_text = (vm_offset_t) &etext;
	s_text = round_page(s_text);
	e_text = trunc_page(e_text);
	vaddr = pmap_map(VM_MIN_KERNEL_ADDRESS, (vm_offset_t) 0, s_text,
						VM_PROT_WRITE|VM_PROT_READ);
	vaddr = pmap_map(vaddr, s_text, e_text, VM_PROT_READ);
	vaddr = pmap_map(vaddr, e_text, ptbr, VM_PROT_WRITE|VM_PROT_READ);


	/*
	 *	Map system page table.
	 */

	*avail_start = round_page(*avail_start);
	*virtual_avail = pmap_map(vaddr, ptbr, *avail_start,
				    VM_PROT_WRITE|VM_PROT_READ);

	printf("Kernel virtual space from 0x%x to 0x%x.\n",
			VM_MIN_KERNEL_ADDRESS, *virtual_end);


	/*
	 *	Allocate virtual space in the system map to be used for
	 *	manipulating physical memory.  (Two VM pages are allocated
	 *	per CPU.)  Then make sure that level two maps are allocated
	 *	for this space and remember the pte address.
	 */
	phys_map_vaddr1 = *virtual_avail;
	*virtual_avail += PAGE_SIZE*NCPUS;
	phys_map_vaddr2 = *virtual_avail;
	*virtual_avail += PAGE_SIZE*NCPUS;

#if	MMAX_KDB || MMAX_DBMON
	/* also give the debugger a page for the same purpose */
	debug_vaddr = *virtual_avail;
	*virtual_avail += PAGE_SIZE;
#endif

	phys_map_pte1 = pmap_pte(kernel_pmap, phys_map_vaddr1);
	phys_map_pte2 = pmap_pte(kernel_pmap, phys_map_vaddr2);
#if	MMAX_KDB || MMAX_DBMON
	debug_pte = pmap_pte(kernel_pmap, debug_vaddr);
#endif
	*virtual_avail = round_page(*virtual_avail);

	/*
	 *	Initialize the in_use arrays for the physical map ptes.
	 */
	for(i = 0; i < NCPUS; i++) {
		phys_pte2_in_use[i] = FALSE;
	}

	/*	Once mapped mode is turned on, we will need to reference
	 *	the system page tables via virtual addresses.  So virtual
	 *	addresses are derived for the level two page tables and
	 *	the page table base register.  We've been using physical
	 *	addresses up till now to make the subroutines work.
	 */

	for (pte1 = (vm_offset_t *)kernel_pmap->pte1_addr + PTE1_ENTRIES;
	     pte1 < (vm_offset_t *)kernel_pmap->pte1_addr + 2*PTE1_ENTRIES;
	     pte1++) {

	    if (*pte1)
		*pte1 += VM_MIN_KERNEL_ADDRESS;
	}
	kernel_pmap->pte1_addr = vaddr;
	kernel_pmap = (pmap_t)(((vm_offset_t) kernel_pmap) +
				VM_MIN_KERNEL_ADDRESS);


	/*	Map in the SCC's and DPC's control/status blocks.  This has
	 *	to be done now so that dbmon (and kernel debugging) will work
	 *	after memory management has been turned on.  Note that the
	 *	virtual address is limited to 24 bits (DPC), 32 bits (APC).
	 */

#if	MMAX_XPC || MMAX_APC
#define VA_LENGTH_MASK  0xffffffff
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_DPC
#define VA_LENGTH_MASK	0x00ffffff
#endif	MMAX_DPC
	/*
	 * Find L1 page which will hold the SCC stuff.
	 *  If it already points to a page of L2's, fine, otherwise
	 *  use 'sccpage'.
	 * Note: We presume that SCCMEM_BASE contains the lowest address
	 *  within this area that we are interested int.
	 */
	sccpte1 = (vm_offset_t *) ptbr + L1IDX((SCCMEM_BASE & VA_LENGTH_MASK));
	if((*sccpte1 & PG_V) == 0)
		pmap_populate_kernel_level1(sccpte1, sccpage, EXTRA_L1);

	pmap_map((SCCMEM_BASE & VA_LENGTH_MASK), SCCMEM_BASE, SCCMEM_TOP,
		VM_PROT_WRITE|VM_PROT_READ);
	pmap_map((SCCREG_BASE & VA_LENGTH_MASK), SCCREG_BASE, SCCREG_TOP,
		VM_PROT_WRITE|VM_PROT_READ);
#if	MMAX_XPC
	pmap_map((XPCREG_BASE & VA_LENGTH_MASK), XPCREG_BASE, XPCREG_TOP,
		 VM_PROT_WRITE|VM_PROT_READ);
	pmap_map((XPCROM_BASE & VA_LENGTH_MASK), XPCROM_BASE, XPCROM_TOP,
		VM_PROT_WRITE|VM_PROT_READ);
#endif	MMAX_XPC
#if	MMAX_APC
	pmap_map((APCREG_BASE & VA_LENGTH_MASK), APCREG_BASE, APCREG_TOP,
		 VM_PROT_WRITE|VM_PROT_READ);
	pmap_map((APCROM_BASE & VA_LENGTH_MASK), APCROM_BASE, APCROM_TOP,
		VM_PROT_WRITE|VM_PROT_READ);
#endif	MMAX_APC
#if	MMAX_DPC
	pmap_map((DPCREG_BASE & VA_LENGTH_MASK), DPCREG_BASE, DPCREG_TOP,
		VM_PROT_WRITE|VM_PROT_READ);
	pmap_map((DPCROM_BASE & VA_LENGTH_MASK), DPCROM_BASE, DPCROM_TOP,
		VM_PROT_WRITE|VM_PROT_READ);
#endif	MMAX_DPC
	/*
	 * Return to locore; locore grabs ptb0 value from ptbr and
	 * will then turn on memory management.
	 */
	return;
}

/*
 *	Initialize the pmap module.
 *	Called by vm_init, to initialize any structures that the pmap
 *	system needs to map virtual memory.
 */
void pmap_init(phys_start, phys_end)
	vm_offset_t	phys_start, phys_end;
{
	long			npages;
    	vm_offset_t		addr;
	vm_size_t		s;
	extern vm_size_t	mem_size;
	pmap_update_list_t	ulistp;
	int i;

	/*
	 *	Allocate memory for the pv_head_table, its lock bits,
	 *	and the modify bit array.
	 */

	npages = atop(phys_end - phys_start);
	s = (vm_size_t) (sizeof(struct pv_entry) * npages +
				pv_lock_table_size(npages) +
				npages);
	s = round_page(s);
	addr = (vm_offset_t) kmem_alloc(kernel_map, s);
	if (addr == 0)
		panic("pmap_init: cannot allocate pv_head_table");
	blkclr((caddr_t) addr, s);

	pv_head_table = (pv_entry_t) addr;

	addr = ((vm_offset_t)(pv_head_table)) +
		(npages * sizeof(struct pv_entry));
	pv_lock_table = (char *) addr;

	addr = ((vm_offset_t)(pv_lock_table)) + pv_lock_table_size(npages);
	pmap_modify_list = (char *) addr;

	/*
	 *	Create the zones of physical maps,
	 *	physical-to-virtual entries, and level 1 maps;
	 */
	s = (vm_size_t) sizeof(struct pmap);
	pmap_zone = zinit(s, (nproc+100)*s, 8192, "pmap");
	s = (vm_size_t) sizeof(struct pv_entry);
	pv_list_zone = zinit(s, 100000*s, 8192, "pv_list"); /* XXX */

	/*
	 *   init update list locks & pointers and active table entry
	 *   for each cpu
	 */
	
	for (i = 0, ulistp = cpu_update_list; i < NCPUS; i++,ulistp++) {
	    simple_lock_init(&ulistp->lock);
	    ulistp->count = 0;
	    cpus_active[i] = FALSE;
	    cpus_idle[i] = FALSE;
	}
	/*
	 *	Initialize vectors for tlb invalidation.
	 */
	 init_tlb_vectors();
#if	MMAX_XPC || MMAX_APC
	/*
	 *	Protect optimization in pmap_remove_range that assumes
	 *	PAGE_SIZE is 8k.
	 */
	if (PAGE_SIZE != 8192)
		panic("Wrong page size, fix pmap_remove_range");
#endif	MMAX_XPC || MMAX_APC
	/*
	 *	Only now that all of the data structures are allocated,
	 *	is it safe to set vm_first_phys and vm_last_phys.
	 *	Otherwise the kmem_alloc above will try to use them.
	 */
	vm_first_phys = phys_start;
	vm_last_phys = phys_end;

	pmap_initialized = TRUE;
}

/*
 *	pte1_alloc:
 *
 *	Allocate the first level page table.  This is twice the size
 *	of the table used by the hardware to allow it to be mirrored
 *	in software.  Hardware uses the first half for physical addresses;
 *	corresponding virtual addresses are in the second half.
 *
 *	This is done here in order to guarantee alignment; the new zone
 *	package no longer makes this guarantee.
 */
vm_offset_t pte1_alloc()
{
	vm_offset_t	result;

	simple_lock(&pte1_free_lock);
#if	MMAX_XPC || MMAX_APC
	/*
	 *	Hardware page table is 4k.  Just kmem_alloc the required
	 *	double-size table.  This works for a page size of 4k or
	 *	more (table and duplicate only need to be contiguous
	 *	in virtual space).
	 */
	if ((result = pte1_free_list) == (vm_offset_t) 0) {
	   simple_unlock(&pte1_free_lock);
	   result = kmem_alloc(user_pt_map, 2*PTE1_SIZE);
	   if (result == (vm_offset_t)0)
	   	panic("pte1_alloc");

	   return(result);
	}
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_DPC
	/*
	 *	Grab a page and break it into pieces. (DPC only).
	 */
	while ((result = pte1_free_list) == (vm_offset_t) 0) {
	    vm_offset_t		newmem, size;

	    /*
	     *	No free tables left.  Check to make sure someone else
	     *	isn't already doing the required allocation.
	     */
	    if (pte1_doing_alloc) {
		/*
		 *	Wait for allocation to finish.
		 */
		pte1_wakeup_needed = TRUE;
		thread_sleep((int)&pte1_free_list, &pte1_free_lock, FALSE);
	    }
	    else {
		/*
		 *	Have to allocate them ourselves.  Can't hold lock
		 *	across call to kmem_alloc.
		 */
		pte1_doing_alloc = TRUE;
		simple_unlock(&pte1_free_lock);

		size = PAGE_SIZE;
		newmem = (vm_offset_t)kmem_alloc(user_pt_map, size);
		if (newmem == (vm_offset_t)0)
	    		panic("pte1_alloc: no memory");

		simple_lock(&pte1_free_lock);
		pte1_doing_alloc = FALSE;

		/*
		 *	Divide up the new memory and insert it into the
		 *	free list.
		 */
		while (size > 0) {
		    *(vm_offset_t *)newmem = pte1_free_list;
		    pte1_free_list = newmem;
		    size -= 2*PTE1_SIZE;
		    newmem += 2*PTE1_SIZE;
		}

		/*
		 *	And wakeup anyone waiting for this.
		 */
		if (pte1_wakeup_needed) {
			pte1_wakeup_needed = FALSE;
			thread_wakeup(&pte1_free_list);
		}
	    }
	}
#endif	MMAX_DPC

	/*
	 *	Remove table from free list and zero out the pointer.
	 *	Resulting table is entirely zero.
	 */
	pte1_free_list = *(vm_offset_t *)result;
	*(vm_offset_t *)result = 0;
	simple_unlock(&pte1_free_lock);

	return(result);
}

/*
 *	pte1_free:
 *
 *	Free a first level page table.  Zero it here so that all free
 *	tables are clean except for the free list pointer.
 */
pte1_free(table)
vm_offset_t	table;
{
	simple_lock(&pte1_free_lock);
	*(vm_offset_t *)table = pte1_free_list;
	pte1_free_list = table;
	simple_unlock(&pte1_free_lock);
}

/*
 *	Create and return a physical map.
 *
 *	If the size specified for the map is zero, the map is an actual
 *	physical map, and may be referenced by the hardware.  In this
 *	case, space for the level one page table is allocated and cleared.
 *
 *	If the size specified is non-zero, the map will be used in software
 *	only, and is bounded by that size.
 */
pmap_t pmap_create(size)
	vm_size_t	size;
{
	register pmap_t			p;
	register pmap_statistics_t	stats;
	register int i;

	/*
	 *	A software use-only map doesn't even need a map.
	 */
	if (size != 0) {
		return(PMAP_NULL);
	}

	/*
	 *	Allocate a pmap structure from the pmap_zone.  Then allocate
	 *	the level one map.  Can't use a zone for this because
	 *	it may not get the alignment right.
	 */
	p = (pmap_t) zalloc(pmap_zone);
	if (p == PMAP_NULL) {
		panic("pmap_create: cannot allocate a pmap");
	}
	p->pte1_addr = pte1_alloc();
	p->ptbr = PHYSTOPTE(pmap_extract(kernel_pmap, p->pte1_addr));
	p->ref_count = 1;
	simple_lock_init(&p->lock);

	for (i=0; i < NCPUS; i++)
	  p->cpus_using[i] = FALSE;
	
	p -> nactive = 0;
	simple_lock_init(&p->nactive_write_lock);

	/*
	 *	Initialize statistics.
	 */

	stats = &p->stats;
	stats->resident_count = 0;
	stats->wired_count = 0;

	return(p);
}

/*
 *	Retire the given physical map from service.  Should
 *	only be called if the map contains no valid mappings.
 */

void pmap_destroy(map)
    pmap_t	map;
{
    vm_size_t	size;
    vm_offset_t	addr;
    register vm_offset_t *pte1_addr;
    register vm_offset_t *pte2_addr;
    vm_offset_t *pte;
    int		c,i;
    unsigned	s;

    if (map != PMAP_NULL) {

	if (map == kernel_pmap)
		s = splvm();
	else
		s = splip();

	CPU_SET_REMOVE(cpu_number(),cpus_active);
	simple_lock(&map->lock);
	c = --map->ref_count;
	simple_unlock(&map->lock);
	CPU_SET_ADD(cpu_number(),cpus_active);
	splx(s);

	if (c == 0) {
	    /*
	     *	Free the level two and level one memory maps,
	     *	then the pmap structure.
	     */

	    for (pte1_addr = (vm_offset_t *)map->pte1_addr;
		 pte1_addr < (vm_offset_t *)(map->pte1_addr + PTE1_SIZE);
		 pte1_addr += vm_page_ptes ) {

		if (*pte1_addr & PG_VALID) {
		    /*
		     *  Found a level 2 page.  Free it; nothing
		     *	should be using these mappings.
		     */
		    pte2_addr = (vm_offset_t *)(*(pte1_addr + PTE1_ENTRIES));
		    kmem_free(user_pt_map, pte2_addr, PAGE_SIZE);

		    /*
		     * Also clean out level 1 mappings.
		     */
		    bzero(pte1_addr, vm_page_bytes);
		    bzero(pte1_addr + PTE1_ENTRIES, vm_page_bytes);
		}
	    }
	    pte1_free(map->pte1_addr);
	    zfree(pmap_zone, map);
	}
    }
}

/*
 *	Add a reference to the specified pmap.
 */

void pmap_reference(p)
	pmap_t	p;
{
	unsigned s;

	if (p != PMAP_NULL) {
		if ( p == kernel_pmap)
		      s = splvm();
	        else
		      s = splip();

		CPU_SET_REMOVE(cpu_number(),cpus_active);
		simple_lock(&p->lock);
		p->ref_count++;
		simple_unlock(&p->lock);
		CPU_SET_ADD(cpu_number(),cpus_active);
		splx(s);
	}
}

/*
 *	Given an offset and a map, return the virtual address of the level
 *	two pte.  If the level two pte address is invalid with respect
 *      to the level one map then NULL is returned (and the map may need
 *	to grow).  This routine is now handled by inline.
 */

#if	0
pt_entry_t *pmap_pte(map, addr)
    pmap_t	map;
    vm_offset_t	addr;
{
    register vm_offset_t *pte1_addr;

    pte1_addr = (vm_offset_t *) map->pte1_addr + L1IDX(addr);

    if ((*pte1_addr & PG_VALID) == 0)
	return(PT_ENTRY_NULL);

    return(((pt_entry_t *) (*(pte1_addr + PTE1_ENTRIES))) + L2IDX(addr));
}
#endif	0

/*
 *	Remove a range of level two page-table entries.  The given
 *	addresses are the first (inclusive) and last (exclusive)
 *	addresses for the VM pages.  Returns a linked list of pv_entries
 *	that caller must free after dropping interrupts.
 *
 *	The pmap must be locked.
 */

pv_entry_t
pmap_remove_range(pmap, s, e)
    pmap_t	pmap;
    vm_offset_t	s, e;
{
    register pt_entry_t		*spte, *cpte;
    pt_entry_t			*epte;
    register pv_entry_t		pv_h, prev, cur;
    pv_entry_t			free_pv = PV_ENTRY_NULL;
    vm_offset_t			pa;
    boolean_t			invalidate_done = FALSE;
#if	MMAX_DPC
    register pt_entry		*mpte, *lpte;
#endif	MMAX_DPC

    while (s < e) {

	/*
	 *	Skip a bunch if level 2 page is missing.
	 */
	if ((spte = pmap_pte(pmap, s)) == PT_ENTRY_NULL) {
	    /*
	     *	Can only get here if the level 2 page is missing.
	     *	Can't happen to kernel pmap.  For user pmap
	     *	set s so that it is mapped by the start of the next
	     *	level 2 page.
	     */
	    if (pmap == kernel_pmap) {
		printf("virtual address = %d\n", s);
		panic("pmap_remove_range: pte missing from kernel pmap!");
	    }

	    s &= pte_page_space_mask;
	    s += pte_page_space;
	    continue;
	}


	/*	Find the last (non-inclusive) level 2 pte, making sure all
	 *	of the inclusive ptes are in the save level 1 group as the
	 *	starting pte.  For user maps, that group is a vm page.
	 *	For the kernel map, all ptes are contiguous.
	 */
	epte =  spte + ns32k_btop(e-s);

	if (pmap != kernel_pmap) {
	    register pt_entry_t		*pg_pte;

	    pg_pte = (pt_entry_t *)((int)trunc_page(spte) + PAGE_SIZE);
	    if (pg_pte < epte)
		epte = pg_pte;
	}

	/*
	 *	Scan the range, unmapping any pages found.
	 */
	for (cpte=spte; cpte<epte; cpte += vm_page_ptes, s += PAGE_SIZE) {
		/*
		 *	Continue if no page here.
		 */
		if ((*(unsigned *)cpte & PG_V) == 0) {
			continue;
		}
		
		/*
		 *	Invalidate tlbs at most once.
		 */
		if (!invalidate_done) {
			INVAL_TLBS(pmap, s, e);
			invalidate_done = TRUE;
		}

		pmap->stats.resident_count--;
		if (*(unsigned *)cpte & PG_WIRED)
		    pmap->stats.wired_count--;

		/*
		 *	Check if bookkeeping needed for this address.
		 */
		pa = get_physaddr(cpte);
		if (pa < vm_first_phys || pa >= vm_last_phys) {
			continue;
		}

		/*
		 *	Save modify bits.
		 */
#if	MMAX_XPC || MMAX_APC
		/*
		 * Can't resist optimizing this -- 4k hardware page size
		 * and 8k vm page size means we look at only 2 ptes.
		 * pmap_init will panic if vm page size is wrong.
		 */
		if ((*(unsigned *)cpte | *(unsigned *)(cpte + 1)) & PG_M) {
			pmap_modify_list[pa_index(pa)] = 1;
		}
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_DPC
		lpte = cpte + vm_page_ptes;
		for(mpte = cpte; mpte < lpte; mpte++) {
			if (*(unsigned *)mpte & PG_M) {
				pmap_modify_list[pa_index(pa)] = 1;
				break;
			}
		}
#endif	MMAX_DPC

		/*
		 *	Remove corresponding pv_list entry.
		 */
		lock_pvh_pa(pa);
		pv_h = pa_to_pvh(pa);
		if (pv_h->pmap == PMAP_NULL) {
			panic("pmap_remove: null pv_list!");
		}
		if (pv_h->va == s && pv_h->pmap == pmap) {
			/*
			 *	Header is the pv_entry.  Copy the next one
			 *	to header and free the next one (we can't
			 *	free the header)
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
		}
		else {
			prev = pv_h;
			while ((cur = prev->next) != PV_ENTRY_NULL) {
				if (cur->va == s && cur->pmap == pmap)
					break;
				prev = cur;
			}
			if (cur == PV_ENTRY_NULL)
				panic("pmap_remove: mapping not in pv_list!");
			prev->next = cur->next;
			cur->next = free_pv;
			free_pv = cur;
		}
		unlock_pvh_pa(pa);
	}

	/*
	 *	Zero the individual ptes.
	 */

	bzero((caddr_t) spte, (epte-spte)*sizeof(pt_entry_t));
    }

    return(free_pv);
}

/*
 *	Remove the given range of addresses from the specified map.  It is
 *	assumed that start and end are rounded to the VM page size.
 */

void pmap_remove(map, start, end)
	pmap_t		map;
	vm_offset_t	start, end;
{
	unsigned	s;
	pv_entry_t	free_pv, cur;


	if (map == PMAP_NULL)
		return;

	READ_LOCK(s);
	simple_lock(&map->lock);

	/* Translation buffer is cleared inside pmap_remove_range */

	free_pv = pmap_remove_range(map, start, end);

	simple_unlock(&map->lock);
	READ_UNLOCK(s);

	/*
	 *	Free any pv entries that pmap_remove_range returned.
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
    register pv_entry_t	pv_h;
    pv_entry_t	cur, free_pv;
    register pt_entry_t	*pte, *end_pte;
    vm_offset_t	va;
    register pmap_t pmap;
    int		pai;
    unsigned	s;

    if (phys < vm_first_phys || phys >= vm_last_phys) {
	/*
	 *	No pv information for this page -- this should never happen.
	 */
	return;
    }

    free_pv = PV_ENTRY_NULL;
    /*
     *	Lock the pmap system first, since we will be changing
     *	several pmaps.
     */

    WRITE_LOCK(s);

    /*
     *	Get the physical page structure since we may be using
     *	it a lot.
     */

    pai = pa_index(phys);

    /*
     *	Walk down PV list, removing all mappings.
     *	We have to do the same work as in pmap_remove_range
     *	since that routine locks the pv_head.  We don't have
     *	to lock the pv_head, since we have the entire pmap system.
     */

    pv_h = pa_to_pvh(phys);
    while ( (pmap = pv_h->pmap) != PMAP_NULL) {

	simple_lock(&pmap->lock);
	va = pv_h->va;

	pte = pmap_pte(pmap, va);
	if (pte == PT_ENTRY_NULL) {
	    panic("pmap_remove_all: pte in list but not in map");
	}

	/*
	 *	Page must be mapped, because we have a pv_list entry.
	 */
	INVAL_TLBS(pmap,va,va+PAGE_SIZE);

	pmap->stats.resident_count--;
	if (*(unsigned *)pte & PG_WIRED)
	    pmap->stats.wired_count--;


	if ((cur = pv_h->next) != PV_ENTRY_NULL) {
	    *pv_h = *cur;
	    cur->next = free_pv;
	    free_pv = cur;
	}
	else
	    pv_h->pmap = PMAP_NULL;

#if	MMAX_XPC || MMAX_APC
	/*
	 *	Optimized assuming 8k vm page size.
	 */
	end_pte = pte + 1;
	if ((*(unsigned *)pte | *(unsigned *)end_pte) & PG_M) {
		pmap_modify_list[pai] = 1;
	}
	*(unsigned *)pte = 0;
	*(unsigned *)end_pte = 0;
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_DPC
	for (end_pte = pte + vm_page_ptes ; pte < end_pte ; pte++) {

	    if (*(unsigned *)pte & PG_M)
		pmap_modify_list[pai] = 1;

	    *(unsigned *)pte = 0;
	}
#endif	MMAX_DPC
	simple_unlock(&pmap->lock);
    }
    WRITE_UNLOCK(s);

    /*
     *	Really free free pv entries now.
     */

    while (free_pv != PV_ENTRY_NULL) {
	cur = free_pv;
	free_pv = free_pv->next;
	zfree(pv_list_zone, (vm_offset_t) cur);
    }

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
    register pmap_t	pmap;
    register pt_entry_t	*pte, *end_pte;
    unsigned 	s;

    /*
     *	Lock the entire pmap system, since we may be changing several maps.
     */
    WRITE_LOCK(s);


    /*
     *	Run down the list of mappings to this physical page,
     *	disabling write privileges on each one.
     */
    pv_e = pa_to_pvh(phys);

    if (pv_e->pmap != PMAP_NULL)
	do {
		/*
		 *	Lock this pmap and invalidate the TB's.
		 *	Page must be mapped because we have a pv_list entry.
		 */
		pmap = pv_e->pmap;	
		simple_lock(&pmap->lock);

		/*
		 *	If there is a mapping, change the protections.
		 */
		if ((pte = pmap_pte(pmap, pv_e->va)) == PT_ENTRY_NULL)
		    panic("pmap_copy_on_write: pte in list, but not map.");

		if( ((*(unsigned *)pte & PG_PROT) >> PG_PROTOFF)
		    == NS32K_UW) {
			INVAL_TLBS(pmap,pv_e->va,pv_e->va+PAGE_SIZE);
			for (end_pte = pte + vm_page_ptes;
			    pte < end_pte; pte++) {
				*(unsigned *)pte &= ~PG_PROT;
				*(unsigned *)pte |= (NS32K_UR << PG_PROTOFF);
			}
		} else if(
		    ((*(unsigned *)pte & PG_PROT) >> PG_PROTOFF)
		    == NS32K_KW) {
			INVAL_TLBS(pmap,pv_e->va,pv_e->va+PAGE_SIZE);
			for (end_pte = pte + vm_page_ptes;
		    	    pte < end_pte; pte++) {
				*(unsigned *)pte &= ~PG_PROT;
				*(unsigned *)pte |= (NS32K_KR << PG_PROTOFF);
			}
		}
		/*
		 *	Continue with next pv_entry.
		 */
		simple_unlock(&pmap->lock);
		pv_e = pv_e->next;

	} while (pv_e != PV_ENTRY_NULL);

    WRITE_UNLOCK(s);
}

/*
 *	Set the physical protection on the
 *	specified range of this map as requested.
 */
void pmap_protect(map, start, end, prot)
    register pmap_t map;
    register vm_offset_t start;
    vm_offset_t	end;
    vm_prot_t	prot;
{
    register pt_entry_t	*pte, *end_pte;
    register int 	vp;
    unsigned s;
    boolean_t	invalidate_done = FALSE;

    if (map == PMAP_NULL)
	return;

    /*
     *	Translate to ns32k protection.  Remove mapping if
     *	protection combination not supported.
     */

    if ((vp = ns32k_protection(map, prot)) == NS32K_NO_ACCESS) {
	pmap_remove(map,start,end);
	return;
    }

    vp = vp << PG_PROTOFF;

    if (map == kernel_pmap)
	  s = splvm();
    else
	  s = splip();
    CPU_SET_REMOVE(cpu_number(),cpus_active);
    simple_lock(&map->lock);

    while(start < end) {

	if ((pte = pmap_pte(map, start)) == PT_ENTRY_NULL) {
	    /*
	     *	Can only get here if the level 2 page is missing.
	     *	Can't happen to kernel pmap.  For user pmaps
	     *	set start so that it is mapped by the start of the next
	     *	level 2 page.
	     */
	    if (map == kernel_pmap) {
		printf("virtual address = %d\n", start);
		panic("pmap_protect: pte missing from kernel pmap!");
	    }

	    start &= pte_page_space_mask;
	    start += pte_page_space;
	}
	else {

	    /*
	     *	Found ptes.  Only change protection if there is a mapping
	     *  and protection is being reduced.
	     */
	    if (((*(unsigned *)pte & PG_V) != 0) &&
		((*(unsigned *)pte & PG_PROT) > vp)) {
		/*
		 *	Invalidate translation buffer at most once.
		 */
		if (!invalidate_done) {
			INVAL_TLBS(map,start,end);
			invalidate_done = TRUE;
		}
		for (end_pte = pte + vm_page_ptes; pte < end_pte; pte++) {
			*(unsigned *)pte &= ~PG_PROT;
			*(unsigned *)pte |= vp;
			MBUG(prot, pte);
		}
	    }

	    start += PAGE_SIZE;
	}
    }
    simple_unlock(&map->lock);
    CPU_SET_ADD(cpu_number(),cpus_active);
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
void pmap_enter(map, v, p, prot, wired)
    register pmap_t map;
    vm_offset_t	v;
    vm_offset_t	p;
    vm_prot_t	prot;
    boolean_t	wired;
{
    register pt_entry_t	*pte, *end_pte;
    register pv_entry_t	pv_h;
    int 	vp;
    pt_entry_t	*p_pte;
    register pv_entry_t	pv_e;
    pv_entry_t	free_pv = PV_ENTRY_NULL;
    vm_offset_t	phys,ptaddr;
    unsigned	s;
    template_t	pte_template;

    if (map == PMAP_NULL)
	    return;

    if ((vp = ns32k_protection(map, prot)) == NS32K_NO_ACCESS) {
	pmap_remove(map, v, v + PAGE_SIZE);
	return;
    }

    /*
     *	Must allocate a new pv_list entry while we're unlocked;
     *	zalloc may cause pageout (which will lock the pmap system).
     *	Most of the time we'll just throw this pv_list entry away.
     *	The perils of multi-processors...
     */
    pv_e = PV_ENTRY_NULL;
Retry:
    READ_LOCK(s);
    simple_lock(&map->lock);

    /*
     *	Expand pmap to include this pte.  Assume that
     *	pmap is always expanded to include enough
     *	pages to map one VM page.
     */
    while ((pte = pmap_pte(map, v)) == PT_ENTRY_NULL) {
	    if (map == kernel_pmap) {
		    printf("virtual address = %d\n", v);
		    panic("pmap_enter: pte missing from kernel pmap!");
	    }
	    else {
		    simple_unlock(&map->lock);
		    READ_UNLOCK(s);
		    pmap_expand(map, v);
		    READ_LOCK(s);
		    simple_lock(&map->lock);
	    }
    }

    if (pv_head_table != PV_ENTRY_NULL) {

	/*
	 *	Enter the mapping in the PV list for this physical page.
	 *	If there is already a mapping, remove the old one first.
	 *	(If it's the same physical page, it's already in the PV list.)
	 */

	ptaddr = get_physaddr(pte);
	if (ptaddr != p) {
	    if (ptaddr != 0) {
		/*
		 *	Translation buffer cleared inside pmap_remove_range.
		 * 	At most one pv_entry can be returned.
		 */
		if (pv_e == PV_ENTRY_NULL) {
		    pv_e = pmap_remove_range(map, v, (v + PAGE_SIZE));
		}
		else {
		    free_pv = pmap_remove_range(map, v, (v + PAGE_SIZE));
		}
	    }

	    /*
	     *	Enter into pv list if page is one we keep info for.
	     */
	    if (p >= vm_first_phys && p < vm_last_phys) {

		lock_pvh_pa(p);
		pv_h = pa_to_pvh(p);

		if (pv_h->pmap == PMAP_NULL) {
		    /*
		     *	No mappings yet
		     */
		    pv_h->va = v;
		    pv_h->pmap = map;
		    pv_h->next = PV_ENTRY_NULL;
		}
		else {
		    /*
		     *	Add new pv_entry after header.
		     */
		    if (pv_e == PV_ENTRY_NULL) {
			unlock_pvh_pa(p);
			simple_unlock(&map->lock);
			READ_UNLOCK(s);
			pv_e = (pv_entry_t) zalloc(pv_list_zone);
			goto Retry;
		    }
		    pv_e->va = v;
		    pv_e->pmap = map;
		    pv_e->next = pv_h->next;
		    pv_h->next = pv_e;
		    /*
		     *	Remember that we used the pv_list entry.
		     */
		    pv_e = PV_ENTRY_NULL;
		}
	        unlock_pvh_pa(p);
	    }

	    map->stats.resident_count++;
	    if (wired)
		    map->stats.wired_count++;
	}
    }

    /*
     *	Enter the mapping in each NS32000 pte.
     */

    if (*(unsigned *)pte & PG_V) {
	/*
	 *	Replacing a valid mapping - preserve the
	 *	modify and referenced bits.  Could only be changing
	 *	protection or wired bits here.  Invalidate the TLB, assuming
	 *	it's the currently active pmap.
	 */
	INVAL_TLBS(map,v,v+PAGE_SIZE);

	if (((*(unsigned *)pte & PG_PROT) >> PG_PROTOFF) != vp) {
	    if (((*(unsigned *)pte & PG_WIRED) >> PG_WIREDOFF) != wired ) {
		/*
		 *	Changing both protection and wiring.
		 */
		pte_template.bits = (vp << PG_PROTOFF)|(wired << PG_WIREDOFF);
		for ( end_pte = pte + vm_page_ptes; pte < end_pte; pte++) {
			*(unsigned *)pte &= ~(PG_PROT | PG_WIRED);
			*(unsigned *)pte |= pte_template.bits;
			MBUG(prot, pte);
		}
	    }
	    else {
		/*
		 *	Changing protection field
		 */
		pte_template.bits = (vp << PG_PROTOFF);
		for ( end_pte = pte + vm_page_ptes; pte < end_pte; pte++) {
			*(unsigned *)pte &= ~PG_PROT;
			*(unsigned *)pte |= pte_template.bits;
			MBUG(prot, pte);
		}
	    }
	}
	else if (((*(unsigned *)pte & PG_WIRED) >> PG_WIREDOFF) != wired) {
	    /*
	     *	Changing wired bit
	     */
	    pte_template.bits = (wired << PG_WIREDOFF);
	    for ( end_pte = pte + vm_page_ptes; pte < end_pte; pte++) {
			*(unsigned *)pte &= ~PG_WIRED;
 			*(unsigned *)pte |= pte_template.bits;
	    }
	}
    }
    else {
	/*
	 *	Not a valid mapping - don't have to invalidate the TLB.
	 *	Set up a template to use in making the pte's.
	 */
	pte_template.bits = PHYSTOPTE(p)|(vp << PG_PROTOFF)|
		((wired << PG_WIREDOFF) & PG_WIRED)| PG_R | PG_V;
	/* MMU BUG -- don't trust modify bit */
	MBUG(prot, &pte_template.pte);

	/*
	 *	Now use the template to set up all the pte's.
	 */
	for ( end_pte = pte + vm_page_ptes ; pte < end_pte; pte++,
		pte_template.bits += NS32K_PGBYTES)
			*pte = pte_template.pte;
    }

    simple_unlock(&map->lock);
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
void pmap_change_wiring(map, v, wired)
	register pmap_t	map;
	vm_offset_t	v;
	boolean_t	wired;
{
	register pt_entry_t	*pte, *end_pte;
	unsigned		s;

	if (map == kernel_pmap)
	      s = splvm();
	else
	      s = splip();
	CPU_SET_REMOVE(cpu_number(),cpus_active);
	simple_lock(&map->lock);

	if ((pte = pmap_pte(map, v)) == PT_ENTRY_NULL)
		panic("pmap_change_wiring: pte missing");

	if (*(unsigned *)pte & PG_V) {
		INVAL_TLBS(map,v,v+PAGE_SIZE);
	}

	for(end_pte = pte + vm_page_ptes; pte < end_pte ; pte++) {
		*(unsigned *)pte &= ~PG_WIRED;
		*(unsigned *)pte |= (wired << PG_WIREDOFF);
	}

	simple_unlock(&map->lock);
	CPU_SET_ADD(cpu_number(),cpus_active);
	splx(s);
}

/*
 *	Routine:	pmap_extract
 *	Function:
 *		Extract the physical page address associated with the given
 *		map/virtual_address pair.  The address includes the offset
 *		within a page.  This routine may not be called by device
 *		drivers at interrupt level -- use pmap_resident_extract.
 */

vm_offset_t pmap_extract(pmap, va)
    register pmap_t		pmap;
    vm_offset_t	va;
{
    pt_entry_t		*pte;
    unsigned	pa;
    unsigned		s;

    if (pmap == kernel_pmap)
	  s = splvm();
    else
	  s = splip();
    CPU_SET_REMOVE(cpu_number(),cpus_active);
    simple_lock(&pmap->lock);

    if (((pte = pmap_pte(pmap, va)) == PT_ENTRY_NULL) || (pte->valid == 0))
	pa = 0;
    else
	pa = get_physaddr(pte) + (va & VA_OFFMASK);

    simple_unlock(&pmap->lock);
    CPU_SET_ADD(cpu_number(),cpus_active);
    splx(s);

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

vm_offset_t pmap_resident_extract(pmap, va)
pmap_t		pmap;
vm_offset_t	va;
{
    pt_entry_t		*pte;

    if (((pte = pmap_pte(pmap, va)) == PT_ENTRY_NULL) || (pte->valid == 0))
	return(0);
    else
	return(get_physaddr(pte) + (va & VA_OFFMASK));
}

/*
 *	Routine:	pmap_access
 *	Function:
 *		Returns whether there is a valid mapping for the
 *		given virtual address stored in the given physical map.
 */

boolean_t pmap_access(pmap, va)
	register pmap_t		pmap;
	vm_offset_t	va;
{
    	pt_entry_t	*pte;
	boolean_t	ok;
	unsigned	s;

        if (pmap == kernel_pmap)
	      s = splvm();
	else
	      s = splip();
	CPU_SET_REMOVE(cpu_number(),cpus_active);
	simple_lock(&pmap->lock);

	if ((pte = pmap_pte(pmap, va)) == PT_ENTRY_NULL)
		ok = FALSE;
	else
	        ok = ( (*(unsigned *)pte & PG_V) == PG_V);

	simple_unlock(&pmap->lock);
	CPU_SET_ADD(cpu_number(),cpus_active);
	splx(s);

	return (ok);
}

/*
 *	Routine:	pmap_expand
 *
 *	Expands a pmap to be able to map the specified virtual address.
 *	Must be called with the pmap module and the pmap unlocked.
 *	This routine must expand the map to handle PAGE_SIZE bytes.
 */
pmap_expand(map, va)
	pmap_t	map;
	vm_offset_t	va;
{
	register pt_entry_t 	*pte1;
	pt_entry_t	*pte2;
	register pt_entry_t	*end_pte;
	register unsigned 	*pte1_va;
	unsigned	pte2_phys;
	register unsigned	pte2_va;
	register unsigned	pte1_offset;
	template_t	pte_template;
	unsigned s;

	/*
	 *	Allocate a VM page for the level two map and get the
	 *	physical and kernel virtual addresses for the map.  Just
	 *	return on kmem_alloc failure (else user can reliably crash
	 *	system by allocating more than his address space.)
	 */
	pte2_va = (vm_offset_t)kmem_alloc(user_pt_map, PAGE_SIZE);
	if (pte2_va == 0)
		return;
	pte2_phys = pmap_extract(kernel_pmap, pte2_va);

	/* if map expanded during above kmem_alloc, just get out */

	s = splvm();
	CPU_SET_REMOVE(cpu_number(),cpus_active);
	simple_lock(&map->lock);

	if (pmap_pte(map,va) != PT_ENTRY_NULL) {

		simple_unlock(&map->lock);
		CPU_SET_ADD(cpu_number(),cpus_active);
		splx(s);

		kmem_free(user_pt_map,pte2_va,PAGE_SIZE);
		return;
	}

	/*	Get the level one offset for these level two entries.
	 *	This is used in the following loop.
	 */
	pte1_offset  = L1IDX(va);
	pte1_offset &= ~(vm_page_ptes - 1);

	/*
	 *	Loop through each entry in this VM page of level two tables,
	 *	entering the physical address of the level two map in the real
	 *	level one page table entry.  Then enter the virtual address
	 *	of the level two page table in the level one virtual list.
	 *	The modify and reference bits are set here to get a
	 *	performance gain.  The MMU does not have to redo the
	 *	translation when it modifies or refers to the Level 2 page
	 *	tables.
	 *
	 *	Note that the level one protection is always set to user
	 *	write.  Read-only protection is enforced by setting
	 *	the level two page table entry to the appropriate value.
	 */

	/*
	 *	Set up a template to use for setting the ptes.
	 */
	pte_template.bits = PHYSTOPTE(pte2_phys)|(NS32K_UW << PG_PROTOFF)|
		PG_R | PG_M | PG_V ;

	/*
	 *	Now set up the ptes and the shadow table
	 */
	pte1 = (pt_entry_t *)map->pte1_addr + pte1_offset;
	pte1_va = (unsigned *)pte1 + PTE1_ENTRIES;

	for (end_pte = pte1 + vm_page_ptes; pte1 < end_pte;
	    pte1++, pte1_va++, pte2_va += NS32K_PGBYTES,
	    pte_template.bits += NS32K_PGBYTES) {

		*pte1 = pte_template.pte;
		*pte1_va = pte2_va;
	}

	simple_unlock(&map->lock);
	CPU_SET_ADD(cpu_number(),cpus_active);
	splx(s);

	/*
	 *	Unlike the vax we don't have to context switch here because
	 *	our level 1 page table never moves.
	 */
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
void pmap_copy(dst_pmap, src_pmap, dst_addr, len, src_addr)
	pmap_t		dst_pmap;
	pmap_t		src_pmap;
	vm_offset_t	dst_addr;
	vm_size_t	len;
	vm_offset_t	src_addr;
{
#ifdef	lint
	pmap_copy(dst_pmap, src_pmap, dst_addr, len, src_addr);
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
	/* TBIA both kernel and user spaces */

	TBIA_K;
	if (pmap_in_use(current_pmap(),cpu_number() )) {
		TBIA_U;
	}
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
void pmap_collect(pmap)
	pmap_t 		pmap;
{
	vm_offset_t	*pte1_addr, *pte2_addr, pte2_page;
	vm_offset_t	mapped_addr, *end_addr1, *end_addr2;
	boolean_t	found_wired;
	unsigned	s;
	pv_entry_t	free_pv = PV_ENTRY_NULL;
	pv_entry_t	cur;

	if (pmap == PMAP_NULL)
		return;

	if (pmap == kernel_pmap)
		panic("pmap_collect on kernel_pmap");

	/*
	 *	Lock down map and invalidate any TLB entries.
	 */
	READ_LOCK(s);
	simple_lock(&pmap->lock);

	/*
	 *	Scan pmap, removing any level 2 page that contains no wired
	 *	mappings.  pte_page_space is the amount of virtual
	 *	space mapped by one level 2 page.  vm_page_bytes is
	 *	the number of level 1 bytes needed to map a level 2 page.
	 */
	
	/*
	 *	Outside loop - for every possible level 2 page.
	 */
	end_addr1 = (vm_offset_t *)(pmap->pte1_addr) + PTE1_ENTRIES;

	for (pte1_addr = (vm_offset_t *)pmap->pte1_addr,
		mapped_addr = VM_MIN_ADDRESS; pte1_addr < end_addr1 ;
		pte1_addr += vm_page_ptes, mapped_addr += pte_page_space ) {

		if (*pte1_addr & PG_VALID) {
			/*
			 *	Found a level2 page; look for wired mappings.
			 */
			pte2_page = *(pte1_addr + PTE1_ENTRIES);
		  	found_wired = FALSE;

			/*
			 *	Inside loop - is any mapping in this
			 *	level 2 page wired ?
			 */
			end_addr2 =(vm_offset_t *)(pte2_page + PAGE_SIZE);

			for (pte2_addr = (vm_offset_t *)pte2_page;
				pte2_addr < end_addr2 ;
				pte2_addr += vm_page_ptes) {

				if (*pte2_addr & PG_WIRED) {
					found_wired = TRUE;
					break;
				}
			}

			if (!found_wired) {
				/*
				 *	No wired mappings, so remove all
				 *	mappings from this page, and all
				 *	references to it from the level 1 map.
				 */
			
				free_pv = pmap_remove_range(pmap, mapped_addr,
					mapped_addr + pte_page_space);
				bzero(pte1_addr, vm_page_bytes);
				bzero(pte1_addr + PTE1_ENTRIES,
					vm_page_bytes);
				/*
				 *	Now free the page, but must drop
				 *	locks while doing this.
				 */
				simple_unlock(&pmap->lock);
				READ_UNLOCK(s);
				kmem_free(user_pt_map, pte2_page, PAGE_SIZE);
				while (free_pv != PV_ENTRY_NULL) {
					cur = free_pv;
					free_pv = free_pv->next;
					zfree(pv_list_zone, (vm_offset_t)cur);
				}

				READ_LOCK(s);
				simple_lock(&pmap->lock);
			}
		}
	}

	simple_unlock(&pmap->lock);
	READ_UNLOCK(s);
}

/*
 *	Routine:	pmap_activate
 *	Function:
 *		Binds the given physical map to the given processor.
 */
void pmap_activate(pmap, th, cpu)
	pmap_t		pmap;
	thread_t	th;
	int		cpu;
{
	PMAP_ACTIVATE(pmap, th, cpu);
}

/*
 *	Routine:	pmap_deactivate
 *	Function:
 *		Indicates that the given physical map is no longer
 *		in use on the specified processor.
 */
void pmap_deactivate(pmap, th, cpu)
	pmap_t		pmap;
	thread_t	th;
	int		cpu;
{
	PMAP_DEACTIVATE(pmap, th, cpu);
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
 *	pmap_zero_page zeros the specified (machine independent) page.
 *	pmap_copy_page copies the specified (machine independent) pages.
 *
 *	These routines are in phys.c.
 */

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
}

/*
 *	Clear the modify bits on the specified physical page.
 *	Assumes that page is not mapped.
 */

void
pmap_clear_modify(phys)
	register vm_offset_t	phys;
{
	if (phys >= vm_first_phys && phys < vm_last_phys)
		pmap_modify_list[pa_index(phys)] = 0;
}

/*
 *	pmap_clear_reference:
 *
 *	Clear the reference bit on the specified physical page.
 *	For the MMAX, unmap the page to avoid scanning the pv_list
 *	here and in pmap_is_referenced, pmap_is_modified.
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
 *	Only called after pmap_clear_reference if the page has not been
 *	reactivated.  Always returns FALSE, because reference to unmapped
 *	page (e.g. from pmap_clear_reference) causes a page fault that
 *	reactivates the page.
 */

boolean_t pmap_is_referenced(phys)
	vm_offset_t	phys;
{
#ifdef	lint
	phys++;
#endif	lint
	return(FALSE);
}

/*
 *	pmap_is_modified:
 *
 *	Return whether or not the specified physical page is modified
 *	by any physical maps.
 *
 *	Assumes page is not mapped -- i.e. pmap_clear_reference has
 *	just been called.
 */

boolean_t pmap_is_modified(phys)
	register vm_offset_t	phys;
{
	if (phys >= vm_first_phys && phys < vm_last_phys)
		return((boolean_t)pmap_modify_list[pa_index(phys)]);
	else
		return(FALSE);
}

/*
 *	pmap_set_modify:
 *
 *	Inform pmap module that the specified physical page has been
 *	modified via a back-door mechanism.
 */

void
pmap_set_modify(phys)
	register vm_offset_t	phys;
{
	if (phys >= vm_first_phys && phys < vm_last_phys)
		pmap_modify_list[pa_index(phys)] = 1;
}

#if	0
/*
 * kmem no longer uses kernacc.
 */
/*
 *	kernacc is used by the kmem code to check if kernel memory can
 *	be accessed.
 */

#define K_READ	0
#define K_WRITE	1

kernacc(address, len, type)
vm_offset_t	address;
unsigned	len;
int		type;
{
	register pt_entry_t		*spte,*epte;
	vm_offset_t		start,end;

	start = (vm_offset_t)ns32k_trunc_page(address);
	end = (vm_offset_t)ns32k_round_page(address + len);

	/*
	 *	If no pte for address, panic.
	 */
	if ((spte = pmap_pte(kernel_pmap, start)) == PT_ENTRY_NULL) {
	    printf("virtual address = %d\n", start);
	    panic("pmap_enter: pte missing from kernel pmap!");
	}

	/*
	 *	Find the last level 2 pte.
	 */
	epte =  spte + ns32k_btop(end-start);

	/*
	 *	Check each pte in this group.
	 */
	for (; spte < epte; spte++) {
	    if (( (*(unsigned *)spte & PG_V) == 0) ||
		((type == K_WRITE) &&
		 ( ((*(unsigned *)spte & PG_PROT)>>PG_PROTOFF)
				       == NS32K_KR) ))
			return(0);	/* failure */
	}
	return(1);	/* success */
}
#endif	0

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
*
* The code here is adapted from the vax version. Because (vm_page_size)/
* (machine page size) is large compared to the number of entries in the
* TLB, an update consists of flushing the entire user and/or kernel half
* of the TLB.  However, much of the code for queuing individual ranges of
* addresses has been retained from the vax version in case vm_page_size
* is altered in the future. The original interrupt code from the vax
* version spun on acquiring both the user and kernel locks, unlocking the
* user lock if the kernel lock could not be acquired. This continuous
* locking-unlocking by a large number of processors could result in
* excessive bus activity, which on the Sequent could cause saturation of
* the slic.  Instead, we spin on the VALUES of these locks.  This causes
* only local cache references,  and allows the TLB flushes on each
* processor to continue in parallel once the updating processor releases
* the lock.
*
* Since locking is expensive on the Sequent, sets are byte
* vectors rather than bit vectors; this allows elements to be added and
* removed without having to acquire another lock. The tradeoff is that we need
* to keep a count of the number of threads active in a pmap to avoid
* the overhead of calling signal_cpus() when only a local TLB update
* is required.
*/

int shootdowns = 0;			/* count shootdown vectors sent */
/*
 *	Interrupts already disabled on multimax.
 */
#if	MMAX_IDEBUG
#define TLB_shootdown_interrupt(cpu) \
	shootdowns++; \
	ASSERT(!icu_ints_on() && ints_on()); \
	send_vector(&(tlb_vector[cpu]));
#else	MMAX_IDEBUG
#define TLB_shootdown_interrupt(cpu)	(shootdowns++, \
					 send_vector(&(tlb_vector[cpu])))
#endif	MMAX_IDEBUG

/*
 *	Signal another CPU that it must flush its TLB
 */

signal_cpus(pmap, start, end)
	register pmap_t		pmap;
	vm_offset_t	start, end;
{
	register int		which_cpu, j;
	register pmap_update_list_t	update_list_p;
#if	MMAX_XPC || MMAX_APC
	register pmap_update_item_t	update_item_p;
#endif	MMAX_XPC || MMAX_APC
	int mycpu = cpu_number();
	int	shoot_cpus[NCPUS];
	register int shoot_count;
	boolean_t	update_in_progress;
#if	XPR_DEBUG
	int	old_timestamp = XPR_TIMESTAMP;
	int	new_timestamp;
#endif	XPR_DEBUG


	shoot_count = 0;
	for (which_cpu = 0; which_cpu < NCPUS; which_cpu++) {

		if (!(IN_CPU_SET(which_cpu, pmap->cpus_using)))
			continue;

		if (which_cpu == mycpu)
			continue;

	    	update_list_p = &(cpu_update_list[which_cpu]);
	    	simple_lock(&update_list_p->lock);

	    	j = update_list_p->count;
		if (j >= UPDATE_LIST_SIZE) {
		    /*
		     *	Overflow.  Change last item to PMAP_NULL to
		     *	indicate this.
		     */
		    update_list_p->item[UPDATE_LIST_SIZE - 1].pmap =
			PMAP_NULL;
		}
		else {
#if	MMAX_XPC || MMAX_APC
		    update_item_p = &(update_list_p->item[j]);
		    update_item_p->pmap = pmap;
		    update_item_p->start = start;
		    update_item_p->end = end;
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_DPC
		    /*
		     *	ns32082 tlb is too small to bother with ranges.
		     */
		    update_list_p->item[j].pmap  = pmap;
#endif	MMAX_DPC
		    update_list_p->count += 1;
		}

		update_in_progress = cpu_update_needed[which_cpu];
	    	cpu_update_needed[which_cpu] = TRUE;
	    	simple_unlock(&update_list_p->lock);
		/*	
		 *	Idle cpus don't get interrupts.  This works because:
		 *
		 *	1.  Idle cpus can only be 'using' the kernel pmap,
		 *	    hence can only be involved in kernel pmap updates.
		 *	2.  Before going non-idle an idle cpu waits for
		 *	    kernel pmap updates to complete, and then performs
		 *	    any queued tlb flushes.
		 *	3.  We have the kernel pmap lock if this is a kernel
		 *	    pmap update.  The idle cpu cannot go active
		 *	    before we drop it.
		 */
		if (!IN_CPU_SET(which_cpu, cpus_idle)) {
			/*
			 *	No interrupt if one has been send already.
			 */
			if (update_in_progress) {
				shoot_int_saved++;
			}
			else {
				shoot_cpus[shoot_count++] = which_cpu;
			}
		}
	}

	if (shoot_count == 0) {
	      /* no update interrupts are needed */
	      return;
	}

	/*
	 *	Send the update interrupts.
	 *
	 */
	for (j = 0; j < shoot_count ; j++) {
		TLB_shootdown_interrupt(shoot_cpus[j]);
	}

	/*
	 *	Wait for all interrupted cpus to respond to the interrupt
	 *	or stop using the pmap.
	 */
	for ( j = 0; j < shoot_count; j++) {
		which_cpu = shoot_cpus[j];
    		while ( IN_CPU_SET(which_cpu, cpus_active) &&
			IN_CPU_SET(which_cpu, pmap->cpus_using) )
			;	/* wait for response */
	}
#if	XPR_DEBUG
	new_timestamp = XPR_TIMESTAMP;
	XPR(XPR_PMAP, (0,shoot_count,(end - start),(pmap == kernel_pmap),
		(new_timestamp - old_timestamp)));
#endif	XPR_DEBUG
}

void process_pmap_updates(my_pmap)
register pmap_t		my_pmap;
{
	int		my_cpu = cpu_number();
	register pmap_update_list_t	update_list_p;
	register pmap_update_item_t	update_item_p;
	register int		j;
	register pmap_t		pmap;

	update_list_p = &(cpu_update_list[my_cpu]);
	update_item_p = &(update_list_p->item[0]);
	simple_lock(&update_list_p->lock);

	for ( j = 0; j < update_list_p->count; j++, update_item_p++) {
	    pmap = update_item_p->pmap;
	    if (pmap == kernel_pmap) {
	        INVALIDATE_K_TLB(update_item_p->start, update_item_p->end);
	    }
	    else if (pmap == my_pmap) {
	        INVALIDATE_U_TLB(update_item_p->start, update_item_p->end);
	    }
	    else if (pmap == PMAP_NULL) {
		/*
		 *	Update list overflowed.  Invalidate everything.
		 */
	    	TBIA_K;
		if (my_pmap != kernel_pmap) {
			TBIA_U;
		}
	    }
	}
	update_list_p->count = 0;
	cpu_update_needed[my_cpu] = FALSE;
	simple_unlock(&update_list_p->lock);
}

/*
 *	Interrupt routine for TLB invalidate requested from other processor.
 */


void pmap_update_interrupt()
{
	register int		my_cpu;
	register pmap_t		pmap;
	/*
	 *	Multimax interrupt locks out other interrupts. Don't have
	 *	to fiddle with spl here as a result.
	 */
#if	XPR_DEBUG
	int	old_timestamp = XPR_TIMESTAMP;
	int	new_timestamp;
#endif	XPR_DEBUG

	my_cpu = cpu_number();

	/*
	 *	idle cpus don't process updates here.
	 */
	if (IN_CPU_SET(my_cpu, cpus_idle))
		return;

	if (current_thread() == THREAD_NULL)
	    pmap = kernel_pmap;
	else {
	    pmap = current_pmap();
	    if (!pmap_in_use(pmap, my_cpu))
		pmap = kernel_pmap;
	}

	while (cpu_update_needed[my_cpu]) {
	
	    /*
	     *	Indicate that we are not using either user or kernel
	     *	pmap.
	     */

	    CPU_SET_REMOVE(my_cpu, cpus_active);

	    /*
	     *	Wait for any pmap updates in progress, on both this
	     *	pmap and the kernel pmap (if distinct).  Must do this
	     *	because update list may contain items for both kernel
	     *	and user pmaps.
	     */

	    while (simple_lock_locked(&pmap->update_lock) ||
		simple_lock_locked(&kernel_pmap->update_lock)) {
			continue;	/* spin*/
	    }
	    process_pmap_updates(pmap);

	    CPU_SET_ADD(my_cpu, cpus_active);

	}

#if	XPR_DEBUG
	/*  Only do on five cpus (5,13,21,29,37) to avoid bashing locks */
	if ((my_cpu & 5) == 5) {
		new_timestamp = XPR_TIMESTAMP;
		XPR(XPR_PMAP, (1,(new_timestamp - old_timestamp)));
	}
#endif	XPR_DEBUG
}

/*
 *	Multimax dynamically allocates interrupt vectors.  Must set them up.
 */
init_tlb_vectors()
{
	board_vbxmit_t	vec;
	int		thiscpu;

	/*
	 *	Allocate interrupt vector and make it directed(slave).
	 */
	if (alloc_vector(&vec, pmap_update_interrupt, 0, INTR_OTHER))
		panic("init_tlb_vectors");
	vec.f.v_class = SLAVE_CLASS;
	
	/*
	 *	Now set up the vectors for each processor.
	 */
#define CPU_TO_SLOT(cpu)	((cpu & 0x3c) >> 2)
#define CPU_TO_DEV(cpu)		(cpu & 0x3)
	for (thiscpu = 0; thiscpu < NCPUS; thiscpu++) {
		vec.f.v_slot = CPU_TO_SLOT(thiscpu);
		vec.f.v_device = CPU_TO_DEV(thiscpu);
		tlb_vector[thiscpu] = vec;
	}
}

vm_offset_t	pmap_phys_address(frame)
	int		frame;
{
	return(ptoa(frame));
}

int	pmap_phys_to_frame(phys)
	vm_offset_t	phys;
{
	return(atop(phys));
}

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
 *	The physical address space is dense ... there are no holes.
 *	All addresses provided to vm_page_startup() are valid.
 */
boolean_t	pmap_valid_page(p)
	vm_offset_t	p;
{
#ifdef	lint
	p++;
#endif	lint
	return(TRUE);
}

/*
 *	Check whether a page is really free.
 */
boolean_t	pmap_verify_free(phys)
	vm_offset_t	phys;
{
	pv_entry_t	pv_h;
	int		spl;
	boolean_t	result;

	if (!pmap_initialized)
		return(TRUE);

	if (phys < vm_first_phys || phys >= vm_last_phys)
		return(FALSE);

	WRITE_LOCK(spl);

	pv_h = pa_to_pvh(phys);

	result = (pv_h->pmap == PMAP_NULL);
	WRITE_UNLOCK(spl);

	return(result);
}
