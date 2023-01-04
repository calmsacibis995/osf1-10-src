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
 *	@(#)$RCSfile: pmap.h,v $ $Revision: 1.2.4.4 $ (DEC) $Date: 1992/07/08 08:45:15 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from pmap.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*
 *	File:	pmap.h
 *
 *	MIPS version
 *	Copyright (C) 1988, 1989 Alessandro Forin
 *
 *	Machine-dependent structures for the physical map module.
 *
 *	Revision History:
 *
 * 10-Oct-91    Marian Macartney
 *      Clean up definitions from BL6.  
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_PMAP_MACHINE_
#define	_PMAP_MACHINE_	1

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_PMAP_MACHINE_H_PREEMPT_
#endif
#endif

#ifndef	ASSEMBLER
#include <mach/boolean.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <mach/machine/vm_param.h>
#include <mach/vm_statistics.h>
#include <machine/endian.h>


/*
 *	MIPS Page Table Entry.
 */

struct pt_entry {
#if	BYTE_MSF
unsigned int	pg_pfnum:20,		/* HW: physical page frame number */
		pg_n:1,			/* HW: non-cacheable bit */
		pg_m:1,			/* HW: modified (dirty) bit */
		pg_v:1,			/* HW: valid bit */
		pg_g:1,			/* HW: ignore pid bit */
		pg_wired:1,		/* SW: wired mapping */
		pg_refclu:1,		/* SW: clue to pg_v usage */
		pg_segment:1,		/* SW: segment pte */
                pg_SW:2,                /* SW: unused */
		pg_prot:3;		/* SW: Mach protection */
#else	/*BYTE_MSF*/
unsigned int	pg_prot:3,		/* SW: Mach protection */
                pg_SW:2,                /* SW: unused */
		pg_segment:1,		/* SW: segment pte */
		pg_refclu:1,		/* SW: clue to pg_v usage */
		pg_wired:1,		/* SW: wired mapping */
		pg_g:1,			/* HW: ignore pid bit */
		pg_v:1,			/* HW: valid bit */
		pg_m:1,			/* HW: modified (dirty) bit */
		pg_n:1,			/* HW: non-cacheable bit */
		pg_pfnum:20;		/* HW: physical page frame number */
#endif	/*BYTE_MSF*/
};

typedef struct pt_entry	pt_entry_t;
#define	PT_ENTRY_NULL	((pt_entry_t *) 0)

typedef union {
	pt_entry_t	pte;
	unsigned long	raw;
} pte_template;

typedef	unsigned tlbpid_t;	/* range 0..63 */

#define PTES_PER_PAGE	(MIPS_PGBYTES / sizeof(pt_entry_t))

#endif	!ASSEMBLER

#define VA_PAGEOFF	12
#define	VA_PAGEMASK	0xfffff000
#define	VA_OFFMASK	0x00000fff
#define	PG_N		0x00000800
#define	PG_M		0x00000400
#define	PG_V		0x00000200
#define	PG_G		0x00000100
#define	PG_PROT		0x00000007
#define	PG_PROTOFF	0
#define PG_WIRED	0x00000080
#define PG_REFCLU       0x00000040
#define	PG_SEGMENT	0x00000020
#define PG_WIREDOFF	7

/*
 * Back and forth between ptes and physical addresses
 */
#define PHYSTOPTE(addr)		((unsigned)(addr) & VA_PAGEMASK)
#define PTETOPHYS(ppte)		((*(int*)ppte) & VA_PAGEMASK)

/*
 * Mips has no hardware encoding of page protections
 */
#define mips_protection(pmap, prot)	(prot)

#ifndef	ASSEMBLER
#define NPCACHE 6

/*
 *	Pmap proper
 */
struct pmap {
	int		pid;		/* TLBPID when in use 		*/
	vm_offset_t	ptebase;	/* Base of pte array 		*/
	vm_offset_t l1ptepage;  	/* l1ptepage 			*/
	int		ref_count;	/* Reference count.		*/
	simple_lock_data_t	lock;	/* Lock on map.			*/
	int		ptepages_count;	/* How many we're using		*/
	vm_offset_t	attributes;	/* Cachability info 		*/
	struct {
		int	 index;		/* Rowing pointer for pte cache */
		struct pcache {
			vm_offset_t	vaddr;
			pt_entry_t	pte;
		} data[NPCACHE];
	} pcache;
	struct pmap_statistics	stats;	/* Map statistics.		*/
	int             (*coprocessor_vm_maint)();	/* N10          */
};

typedef struct pmap	*pmap_t;
#define	PMAP_NULL	((pmap_t) 0)

struct sys_space {
	vm_size_t   s_size;
	vm_offset_t *s_va;
	vm_size_t   s_cpuid;
};

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


/*
 * *coprocessor_vm_maint is a function of two arguments:  the first is a
 * command or operation token; the second is a virtual page number in
 * the process.  The purpose of the function is to synchronize the
 * invalidation of address translations with a device which uses those
 * translations.  The commands are defined below.
 *
 * The function returns NULL for failure and !NULL for success.
 * Presently, it is necessary that the function succeed.  Therefore,
 * this function stalls until the device it serves is done with the
 * address translation for the virtual page.  In the future, failure
 * would indicate that the device is busy with the page, and that page
 * is not presently available for invalidation.
 */

#define PDEVCMD_ONE	0	/* invalidate the specified translation */
#define PDEVCMD_ALL	1	/* invalidate all device translation data */

#define clear_coprocessor_vm_maint(pmap)                               \
if ((pmap)->coprocessor_vm_maint) {                                    \
	(*(pmap)->coprocessor_vm_maint)(PDEVCMD_ALL, 0);               \
	(pmap)->coprocessor_vm_maint = (int(*)())NULL;                 \
} else

#define set_coprocessor_vm_maint(p, func) if (1) {             \
	(p)->task->map->pmap->coprocessor_vm_maint = (func)    \
} else

/*
 *	Macros
 */

/*
 *	Given an offset and a map, return the virtual address of the
 *	pte that maps the address.
 */
#define	pmap_pte(map,addr) 						\
	((pt_entry_t *) ((map)->ptebase +				\
			((((unsigned)addr) >> MIPS_PGSHIFT) << 2)))

#define pmap_root_pte(addr)						\
	((pt_entry_t *) (root_kptes +					\
		      ((((unsigned)(addr) - KPTEADDR) >> MIPS_PGSHIFT) << 2)))
/*
 *	Given an offset and a map, return the k0 address of the level 1 or
 *  level 2 pte that maps that offset.
 */

#define pmap_l1pte(map,addr)							\
		((pt_entry_t *) ((map)->l1ptepage + 			\
						((((unsigned)(addr)) >> 22) << 2)))

#define pmap_l2pte(l1pte,addr)				       		\
		((pt_entry_t *)	((PHYS_TO_K0(PTETOPHYS(l1pte))) +   	\
						(((((unsigned)(addr)) << 10) >> 22) << 2)))

#define	va_to_ptindex(ADDR) (((ADDR) << 10) >> 22)

#define	pmap_segpte(SP,ADDR)						\
		((pt_entry_t *)(SP)->ps_pagetable) + va_to_ptindex(ADDR)


/*
 *	Given either a KPTE or UPTE, return the k0 address of the l1pte that
 *	maps that pte.
 */

#define pmap_root_l1pte(map,addr)						\
	((pt_entry_t *) ((map->l1ptepage)  +					\
		((((unsigned)(addr) - (map->ptebase)) >> MIPS_PGSHIFT) << 2)))

/*
 *	Check whether the pte page for a pte exists.  This is used to
 *	avoid faulting in pte pages unnecessarily.  
 */
#define pmap_pte_page_exists(map, pte) 				  	 \
	(pmap_root_l1pte(map, pte)->pg_v) 

#ifdef	KERNEL

#define PMAP_ACTIVATE(pmap, th, my_cpu)			\
	{                                               \
		if ((pmap)->pid < 0)                    \
			assign_tlbpid(pmap);	        \
		tlb_set_context(pmap);			\
	}

/*
 *	This is a sleazy definition, but given the places
 *	where PMAP_DEACTIVATE it actually used, it works.
 *	And it makes context switches faster.
 */

#define PMAP_DEACTIVATE(pmap, thread, cpu)

#define PMAP_CONTEXT(pmap, new_thread)

#define pmap_load           pmap_mmu_load
#define pmap_unload         pmap_mmu_unload
#define pmap_tb             pmap_mmu_tb
#define svatophys           pmap_svatophys
#define tbsync              pmap_tbsync

#define	ICACHE	0x1		/* flush i cache */
#define	DCACHE	0x2		/* flush d cache */
#define	BCACHE	(ICACHE|DCACHE)	/* flush both caches */

#define NV_OK               0
#define NV_NOTOK            1

#define TB_SYNC_NONE        1
#define TB_SYNC_LOCAL       2
#define TB_SYNC_ALL         4

typedef unsigned int        vm_tbop_t;

extern void pmap_pagemove();
extern void pmap_cache_flush();

#define	pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define	pmap_phys_address(frame)	((vm_offset_t) (mips_ptob(frame)))
#define pmap_phys_to_frame(phys)	((int) (mips_btop(phys)))
#define pmap_copy(dst,src,from,len,to)
#define pmap_update()
#define pmap_kernel()			kernel_pmap

/*
 *	Data structures this module exports
 */
extern	pmap_t	    kernel_pmap;	/* pointer to the kernel pmap	*/
extern	pmap_t	    active_pmap;	/* pmap for the current thread  */
extern	vm_offset_t root_kptes;		/* ptes to back up the kernel's pte */

#endif	KERNEL
#endif	!ASSEMBLER

#if	RT_PREEMPT
#ifdef	_PMAP_MACHINE_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	_PMAP_MACHINE_
