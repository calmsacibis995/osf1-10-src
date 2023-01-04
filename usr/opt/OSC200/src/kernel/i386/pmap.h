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
 *	@(#)$RCSfile: pmap.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:44 $
 */ 
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	File:	pmap.h
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Machine-dependent structures for the physical map module.
 */

#ifndef	_PMAP_MACHINE_
#define _PMAP_MACHINE_	1

#ifndef	ASSEMBLER

#include <kern/zalloc.h>
#include <kern/lock.h>
#include <i386/seg.h>
#include <mach/i386/vm_param.h>
#include <mach/vm_statistics.h>

/*
 *	80386 Page Table Entry
 */

/* 
 * There is a software "valid" bit that tells whether the page really 
 * should be valid.  The hardware valid bit might be turned off so 
 * that we can do copy-on-reference.  This only applies to entries in 
 * (second-level) page tables.  Entries in page directories just use 
 * the hardware valid bit.
 */
struct pt_entry {
	unsigned int
			valid:1,	/* hardware valid */
			prot:2,		/* protection */
			:2,		/* hardware reserved */
			ref:1,		/* hardware page referenced */
			modify:1,	/* hardware modify bit */
			:2,		/* hardware reserved;
/*			unused:3,	/* unused by 80386 hardware */
			wired:1,	/* is this page wired? */
			sw_valid:1,	/* is page really there? */
			:1,		/* bits we aren't using */
			pfn:20;		/* page frame number */
};

typedef struct pt_entry	pt_entry_t;
#define PT_ENTRY_NULL	((pt_entry_t *) 0)

#endif	ASSEMBLER

#define NPTES	(i386_ptob(1)/sizeof(pt_entry_t))
#define NPDES	(i386_ptob(1)/sizeof(pt_entry_t))

/*
 *	80386 PTE protection codes.  (To be used in conjunction with the
 *	structure above).
 */

#define I386_NO_ACCESS	0		/* no access */
#define I386_UR		2		/* user readable */
#define I386_UW		3		/* user writeable */
#define I386_KRW	1		/* kernel readable, writeable */

/*
 *	80386 pte bit definitions (to be used directly on the ptes
 *	without using the bit fields).
 */

#define I386_PTE_VALID	0x00000001
#define I386_PTE_UR	0x00000004
#define I386_PTE_UW	0x00000006
#define I386_PTE_KRW	0x00000002
#define I386_PTE_REF	0x00000020
#define I386_PTE_MOD	0x00000040
#define I386_PTE_WIRED	0x00000200
#define I386_PTE_PFN	0xfffff000

#ifndef	ASSEMBLER
typedef	long		cpu_set;	/* set of CPUs - must be <= 32 */

struct pmap {
	pt_entry_t	*cr3;		/* page directory pointer - CR3 */
	int		ref_count;	/* reference count */
	struct fakedesc	ldt[LDTSZ];	/* eventually a pointer */
	simple_lock_data_t
			lock;		/* lock on map */
	struct pmap_statistics	stats;	/* map statistics */
	cpu_set		cpus_using;	/* bitmap of cpus using pmap */
};

typedef struct pmap	*pmap_t;

#define PMAP_NULL	((pmap_t) 0)

#if	NCPUS > 1
/*
 *	List of cpus that are actively using mapped memory.  Any
 *	pmap update operation must wait for all cpus in this list.
 *	Update operations must still be queued to cpus not in this
 *	list.
 */
cpu_set		cpus_active;

/*
 *	List of cpus that are idle, but still operating, and will want
 *	to see any kernel pmap updates when they become active.
 */
cpu_set		cpus_idle;

/*
 *	Quick test for pmap update requests.
 */
boolean_t	cpu_update_needed[NCPUS];

/*
 *	External declarations for PMAP_ACTIVATE.
 */

void		process_pmap_updates();
void		pmap_update_interrupt();
extern	pmap_t	kernel_pmap;

#endif	NCPUS > 1

/*
 *	Machine dependent routines that are used only for I386.
 */

pt_entry_t	*pmap_pte();
vm_offset_t	pmap_map_bd();		/* map "back door" */
/*
 *	Macros for speed.
 */

#if	NCPUS > 1

/*
 *	For multiple CPUS, PMAP_ACTIVATE and PMAP_DEACTIVATE must manage
 *	fields to control TLB invalidation on other CPUS.
 */

#define PMAP_ACTIVATE(pmap, th, my_cpu)	{				\
	register pmap_t		tpmap = (pmap);				\
									\
	/*								\
	 *	Let pmap updates proceed while we wait for this pmap.	\
	 */								\
	i_bit_clear((my_cpu), &cpus_active);				\
									\
	/*								\
	 *	Lock the pmap to put this cpu in its active set.	\
	 *	Wait for updates here.					\
	 */								\
	simple_lock(&tpmap->lock);					\
									\
	if (tpmap == kernel_pmap) {					\
	    /*								\
	     *	Process invalidate requests for the kernel pmap.	\
	     *	Don't load the map registers.				\
	     */								\
	    if (cpu_update_needed[(my_cpu)])				\
		process_pmap_updates(kernel_pmap);			\
	}								\
	else {								\
	    register struct pcb	*pcb = thread_pcb((th));		\
									\
	    /*								\
	     *	No need to invalidate the TLB - the entire user pmap	\
	     *	will be invalidated by load_context().  Just reload	\
	     *	the map registers.					\
	     */								\
	    pcb->pcb_cr3 = tpmap->cr3;					\
	}								\
									\
	/*								\
	 *	Mark that this cpu is using the pmap.			\
	 */								\
	i_bit_set((my_cpu), &tpmap->cpus_using);			\
									\
	/*								\
	 *	Mark this cpu active - IPL will be lowered by		\
	 *	load_context().						\
	 */								\
	i_bit_set((my_cpu), &cpus_active);				\
									\
	simple_unlock(&tpmap->lock);					\
}

#define PMAP_DEACTIVATE(pmap, thread, my_cpu)	{			\
	/*								\
	 *	Mark pmap no longer in use by this cpu even if		\
	 *	pmap is locked against updates.				\
	 */								\
	i_bit_clear((my_cpu), &(pmap)->cpus_using);			\
}

#define MARK_CPU_IDLE(my_cpu)	{					\
	/*								\
	 *	Mark this cpu idle, and remove it from the active set,	\
	 *	since it is not actively using any pmap.  Signal_cpus	\
	 *	will notice that it is idle, and avoid signaling it,	\
	 *	but will queue the update request for when the cpu	\
	 *	becomes active.						\
	 */								\
	int	s = splvm();						\
	i_bit_set((my_cpu), &cpus_idle);				\
	i_bit_clear((my_cpu), &cpus_active);				\
	splx(s);							\
}

#define MARK_CPU_ACTIVE(my_cpu)	{					\
									\
	int	s = splvm();						\
	/*								\
	 *	If a kernel_pmap update was requested while this cpu	\
	 *	was idle, process it as if we got the interrupt.	\
	 *	Before doing so, remove this cpu from the idle set.	\
	 *	Since we do not grab any pmap locks while we flush	\
	 *	our TLB, another cpu may start an update operation	\
	 *	before we finish.  Removing this cpu from the idle	\
	 *	set assures that we will receive another update		\
	 *	interrupt if this happens.				\
	 */								\
	i_bit_clear((my_cpu), &cpus_idle);				\
									\
	if (cpu_update_needed[(my_cpu)])				\
	    pmap_update_interrupt();					\
									\
	/*								\
	 *	Mark that this cpu is now active.			\
	 */								\
	i_bit_set((my_cpu), &cpus_active);				\
	splx(s);							\
}

#define PMAP_CONTEXT(pmap, new_thread)

#else	NCPUS > 1

/*
 *	With only one CPU, we just have to indicate whether the pmap is
 *	in use.
 */

#define PMAP_ACTIVATE(pmap, th, my_cpu)	{				\
	register pmap_t		tpmap = (pmap);				\
									\
	if (tpmap != kernel_pmap) {					\
	    register struct pcb	*pcb = thread_pcb((th));		\
									\
	    pcb->pcb_cr3 = tpmap->cr3;					\
	}								\
	tpmap->cpus_using = TRUE;					\
}

#define PMAP_DEACTIVATE(pmap, thread, cpu)	{			\
	(pmap)->cpus_using = FALSE;					\
}

#define PMAP_CONTEXT(pmap, new_thread)

#endif	NCPUS > 1

#define pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define pmap_phys_address(frame)	((vm_offset_t) (i386_ptob(frame)))
#define pmap_phys_to_frame(phys)	((int) (i386_btop(phys)))
#define pmap_attribute(pmap,addr,siz,attr,val)	0

#define pte_to_pa(ptep) (*(int *)(ptep) & (~PTEMASK))

#endif	ASSEMBLER

#endif	_PMAP_MACHINE_
