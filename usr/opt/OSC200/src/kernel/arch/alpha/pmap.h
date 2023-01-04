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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *
 *	File:	pmap.h
 *
 *	Alpha version
 *
 *	Machine-dependent structures for the physical map module.
 *
 */

#ifndef	_PMAP_MACHINE_
#define	_PMAP_MACHINE_	1

#define MMF_READ	0
#define MMF_IFETCH	-1
#define MMF_WRITE	1

#ifndef	ASSEMBLER
#include <mach/boolean.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <mach/machine/vm_param.h>
#include <mach/vm_statistics.h>
#include <machine/counters.h> 

/*
 *	Alpha Page Table Entry.
 */

union pt_entry {
	unsigned long	quadword;	/* for alignment and atomic writes */
	struct {
		unsigned int	_v:1,		/* valid */
				_for:1,		/* fault on read */
				_fow:1,		/* fault on write */
				_foe:1,		/* fault on execute */
				_asm:1,		/* address space match */
				_gh:2,		/* granularity hint */
				:1,		/* RESERVED future hardware */
				_prot:8,	/* {K|E|S|U}{R|W}E */
				_exec:1,	/* execute access flag */
				_wire:1,	/* wired */
				_seg:1,		/* segment pte entry */
		                _soft:13,	/* unused software bits */
				_pfn:32;	/* page frame number */
	}		PTE_BITFIELD;
};

#define pg_v	PTE_BITFIELD._v
#define pg_for	PTE_BITFIELD._for
#define pg_fow	PTE_BITFIELD._fow
#define pg_foe	PTE_BITFIELD._foe
#define pg_asm	PTE_BITFIELD._asm
#define pg_gh	PTE_BITFIELD._gh
#define pg_prot	PTE_BITFIELD._prot
#define pg_exec	PTE_BITFIELD._exec
#define pg_wire	PTE_BITFIELD._wire
#define pg_seg	PTE_BITFIELD._seg
#define pg_soft	PTE_BITFIELD._soft
#define pg_pfn	PTE_BITFIELD._pfn

typedef union pt_entry	pt_entry_t;
#define	PT_ENTRY_NULL	((pt_entry_t *) 0)

#define NOTaPFN	((unsigned int)0)
/* The lowest address which is not accessible at any page size. */
#define NOTaVA	((vm_offset_t)(1L<<54))

/*
 * Assumptions:
 *	1) Supervisor and executive modes are not used.
 *	2) User mode accessibility implies kernel mode accessibility.
 */
#define PROT_UW		0x33
#define PROT_KW		0x11
#define PROT_NA		0x00
#define PROT_URKW	0x13
#define PROT_UR		0x03
#define PROT_KR		0x01

/* (some) pte fields as quadword masks */
#define PTEQ_MASK_KWE	(1L << 12)
#define PTEQ_MASK_UWE	(1L << 13)
#define PTEQ_MASK_EXEC	(1L << 16)

#define PTES_PER_PAGE	(ALPHA_PGBYTES / sizeof(pt_entry_t))

#define PTETOPHYS(pte)	alpha_ptob((pte)->pg_pfn)

#define LEVEL1_PT_OFFSET(ADDR) \
	(((vm_offset_t)(ADDR) >> (3 * PGSHIFT - 6)) & (PGOFSET>>3))
#define LEVEL2_PT_OFFSET(ADDR) \
	(((vm_offset_t)(ADDR) >> (2 * PGSHIFT - 3)) & (PGOFSET>>3))
#define LEVEL3_PT_OFFSET(ADDR) (((vm_offset_t)(ADDR) >> PGSHIFT) & (PGOFSET>>3))
#define EQ_LEVEL1_VPN(XADDR, YADDR) \
	((((XADDR) ^ (YADDR)) >> (3 * PGSHIFT - 6)) == 0)
#define EQ_LEVEL2_VPN(XADDR, YADDR) \
	((((XADDR) ^ (YADDR)) >> (2 * PGSHIFT - 3)) == 0)
#define EQ_LEVEL3_VPN(XADDR, YADDR) ((((XADDR) ^ (YADDR)) >> PGSHIFT) == 0)
#define in_Othermap(PTE) \
	EQ_LEVEL1_VPN((vm_offset_t)(PTE), (vm_offset_t)Othermap)

/*
 *	Pmap proper
 */
struct pmap {
	simple_lock_data_t	lock;	   /* Lock on map.		*/
	pt_entry_t *		level1_pt; /* level 1 page table	*/
	struct pmap_statistics	stats;	   /* Map statistics.		*/
	int			ref_count; /* Reference count.		*/
	int			(*coproc_tbi)(); /* synch with PXG maps */
	unsigned short		map_asn[NCPUS];	/* address space number	*/
};

typedef struct pmap *	pmap_t;
#define	PMAP_NULL	((pmap_t) 0)

#define PMAP_MAXASN	63	/* good through EV4, pass 2 */

struct asn_state {
	struct asn_data {
		queue_chain_t	q;
		pmap_t		owner;
	} cpu_asn[PMAP_MAXASN + 1];
	queue_head_t	free_asns, active_asns;
} asns_on[NCPUS];

struct sys_space {
	vm_size_t   s_size;
	vm_offset_t *s_va;
	vm_size_t   s_cpuid;
};

struct scavenge_list {
	long		count;
	vm_offset_t	kseg_start;
};

extern struct scavenge_list scavenge_info;

/*
 *	Macros
 */

#ifdef	KERNEL

#define PMAP_ACTIVATE(pmap, th, my_cpu) pmap_activate(pmap, th, my_cpu)

#define PMAP_DEACTIVATE(pmap, thread, cpu)

#define PMAP_CONTEXT(pmap, new_thread)

#define PHYS_TO_KSEG(addr) ((vm_offset_t)(addr) + UNITY_BASE)
#define KSEG_TO_PHYS(addr) ((vm_offset_t)(addr) - UNITY_BASE)

/*
 * Enable segmentation
 */

#define	PMAP_SEGMENTATION	1

struct pmap_seg {
	simple_lock_data_t
			ps_seglock;		/* Segment lock */
	simple_lock_data_t
			ps_reslock;		/* Resident lock */
	unsigned short	ps_refcnt;		/* Segment ref count */
	unsigned short	ps_rescnt;		/* Segment resident count */
	unsigned short	ps_loadedpte;		/* Ptes in page table */
	vm_offset_t	ps_pagetable;		/* Page table */
	struct pv_entry *ps_pvsegment;		/* PV for segment */
};

typedef struct pmap_seg * pmap_seg_t;
#define	PMAP_SEG_NULL	((pmap_seg_t) 0)

#define	pmap_seg_lock(SP)	simple_lock(&(SP)->ps_seglock)
#define	pmap_seg_lock_try(SP)	simple_lock_try(&(SP)->ps_seglock)
#define	pmap_seg_unlock(SP)	simple_unlock(&(SP)->ps_seglock)

#define	pmap_segres_lock(SP)	simple_lock(&(SP)->ps_reslock)
#define	pmap_segres_unlock(SP)	simple_unlock(&(SP)->ps_reslock)

#define PMAP_COPROC_INVALIDATE_STLB  0
#define PMAP_COPROC_EXIT             1

#define PDEVCMD_ONE     PMAP_COPROC_INVALIDATE_STLB
#define PDEVCMD_ALL     PMAP_COPROC_EXIT

#define ALL_THRESHHOLD	(1<<20)

#define pmap_set_coproc_tbi(pmap, func)			\
	((pmap)->coproc_tbi = (func))

#define pmap_clear_coproc_tbi(pmap)			\
	if ((pmap)->coproc_tbi) {			\
		(*(pmap)->coproc_tbi)(PDEVCMD_ALL, 0);	\
		(pmap)->coproc_tbi = (int(*)())NULL;	\
	} else

#define pmap_load           pmap_mmu_load
#define pmap_unload         pmap_mmu_unload
#define pmap_tb             pmap_mmu_tb
#define svatophys           pmap_svatophys
#define tbsync              pmap_tbsync

#define TB_SYNC_NONE        1
#define TB_SYNC_LOCAL       2
#define TB_SYNC_ALL         4

typedef unsigned int        vm_tbop_t;

extern void pmap_pagemove();

#define pmap_pageable(p_map, sva, eva, pageable)
#define	pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define pmap_resident_text(pmap)	((pmap)->stats.resident_text)
#define pmap_resident_data(pmap)	(pmap_resident_count(pmap)	\
					 - pmap_resident_stack(pmap)	\
					 - pmap_resident_text(pmap))
#define	pmap_phys_address(frame)	((vm_offset_t) (alpha_ptob(frame)))
#define pmap_phys_to_frame(phys)	((int) (alpha_btop(phys)))
#define pmap_copy(dst,src,from,len,to)
#define pmap_update()
#define pmap_kernel()			kernel_pmap

#define REF_BITS /* affects compile of pmap.c, vm_pagelru.c, and vfs_ubc.c */
#ifndef	REF_BITS
#define pmap_is_referenced(phys)	((boolean_t)(INC_IRR_COUNT, FALSE))
#endif	/* REF_BITS */

/*
 *	Data structures this module exports
 */
extern	pmap_t	kernel_pmap;		/* pointer to the kernel pmap	*/
extern	pmap_t	active_pmap;		/* pmap for the current thread  */

#define	mtpr_tbis(vaddr)	mtpr_tbi(3, vaddr)
#define	mtpr_tbisi(vaddr)	mtpr_tbi(1, vaddr)
#define	mtpr_tbisd(vaddr)	mtpr_tbi(2, vaddr)
#define	mtpr_tbia()		mtpr_tbi(-2L)
#define	mtpr_tbiap()		mtpr_tbi(-1L)

#endif	/*KERNEL*/
#endif	/*!ASSEMBLER*/

#endif	/*_PMAP_MACHINE_ */
