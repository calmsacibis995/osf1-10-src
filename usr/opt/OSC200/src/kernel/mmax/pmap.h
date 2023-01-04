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
 *	@(#)$RCSfile: pmap.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:35 $
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
 *	File:	machine/pmap.h
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr., Michael Wayne Young
 *
 *	Machine-dependent structures for the physical map module.
 *
 * H I S T O R Y
 *
 * 4-Apr-86  Jim Van Sciver (jvs) at Encore
 *	Created from Mach's VAX sources.
 */

#ifndef	_PMAP_MACHINE_
#define _PMAP_MACHINE_	1

#ifdef	_KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#endif

#if	MMAX_DPC || MMAX_XPC
#define MMUBUG 1
#endif

#ifndef	ASSEMBLER

#include <kern/macro_help.h>
#include <mach/boolean.h>
#include <kern/zalloc.h>
#include <kern/lock.h>
#include <mach/vm_param.h>
#include <mach/vm_statistics.h>

#include <mmax/mtpr.h>
#include <mmax/pte.h>

/*
 *	NS32K Page Table Entry and Page Table Base Register.
 */

#if	MMAX_XPC || MMAX_APC
struct pt_entry {
  unsigned int    valid:1,	/* Page is valid.                        */
                  prot:2,	/* Protection.                           */
                  hw_unused:3,	/* Reserved by NSC.                      */
                  cache_inv:1,	/* Cache Invalidate. (L2 pte ONLY)       */
                  refer:1,	/* Page has been referenced.             */
                  modify:1,	/* Page has been modified. (L2 pte ONLY) */
                  wired:1,	/* Is this page wired ?                  */
                  sw_unused:2,	/* Reserved for software.                */
                  pfn:20;	/* Page Frame Number.                    */
};
#endif	/* MMAX_XPC || MMAX_APC */

/*
 *	WARNING: the modify, wired, sw_unused and pfn fields cannot be
 *	be accessed by bit field operations in C (pte.field or pte->field).
 *	The chip will try to read the next byte beyond the pte producing
 *	illegal page faults!!
 */

#if	MMAX_DPC
struct pt_entry {
unsigned int	valid:1,		/* Page is valid.		*/
		prot:2,			/* Protection.			*/
		refer:1,		/* Page has been referenced.	*/
		modify:1,		/* Page has been modified.	*/
/*		unused:4,		/* Unused by ns32000 hardware.  */
		hw_unused:2,		/* Bits we aren't using.	*/
		wired:1,		/* Is this page wired?		*/
		sw_unused:1,		/* Bit we aren't using.	WARNING */
		pfn:23;			/* Page frame number. WARNING	*/
};

#endif	/* MMAX_DPC */

typedef struct pt_entry	pt_entry_t;
#define PT_ENTRY_NULL	((pt_entry_t *) 0)


/*
 *	Type breaking union for pte templates.
 */

union template {
	pt_entry_t	pte;
	long		bits;
};

typedef	union template		template_t;

#endif	/* ASSEMBLER */

/*
 *	NS32K PTE protection codes.  
 *	(To be used in conjunction with the structure above).
 */

#define NS32K_NO_ACCESS    -1	/* Flag to tell pmap to remove mapping */
#define NS32K_UR	   2	/* User readable, Kernel full access.*/
#define NS32K_UW	   3	/* User writeable,Kernel full access.*/
#define NS32K_KR	   0	/* Kernel readable, User no access.  */
#define NS32K_KW	   1	/* Kernel writeable,User no access.  */


#ifndef	ASSEMBLER

/* TLB invalidation */
#define CPU_SET_ADD(el,set)	(set[el] = TRUE)
#define CPU_SET_REMOVE(el,set)	(set[el] = FALSE)
#define IN_CPU_SET(el,set)	(set[el])
#define update_lock lock

/* test if a simple lock is locked WITHOUT locking it */
#ifdef	multimax
/*
 *  The inlined lock operations have been changed to
 *  work on a single byte rather than operating on the
 *  entire word.  Note that memory-based locks are employed
 *  for the DPC and APC (a locked lock has a value of 0xff)
 *  and ownership-based locks are used by the XPC (a locked
 *  lock has a value of 0x1 as the result of the usual sbitib $0).
 */
#define	simple_lock_locked(lp)	((lp)->lock_data & 0xff)
#endif

typedef	unsigned char 	cpu_set_t[NCPUS];

struct pmap {
	unsigned long	ptbr;		/* Physical addr of page table.	*/
	unsigned long	pte1_addr;	/* Virtual page table address.	*/
	int		ref_count;	/* Reference count.		*/
	simple_lock_data_t	lock;	/* Lock on map.			*/
	struct pmap_statistics	stats;	/* Map statistics.		*/
	cpu_set_t	cpus_using;	/* set of cpus using pmap     */
	int 		nactive;	/* number of cpus using pmap */
	simple_lock_data_t nactive_write_lock; /* write lock on nactive */
};

typedef struct pmap	*pmap_t;

#define PMAP_NULL	((pmap_t) 0)

pt_entry_t	*pmap_pte();


#ifdef	MMUBUG
/*
 *	MMU problem can cause a write fault to be reported as a read fault.
 *	As a workaround if the same read fault occurs twice in a row
 *	report it as a write fault.  Following data structure holds
 *	fault data.
 */

struct mmubug_vm_fault {
	char		*map;			/* really vm_map_t */
	unsigned	addr;
	int		type;
};

extern struct mmubug_vm_fault	last_fault[NCPUS];

/* Macros to update and invalidate last_fault data */

#define LAST_FAULT_IDENTICAL(cpu, m, t, va) \
	(last_fault[(cpu)].map == (char *)(m) && last_fault[(cpu)].type == (t) && \
	 last_fault[(cpu)].addr == (va))

#define	LAST_FAULT_MATCH(cpu, m, va) \
	(last_fault[(cpu)].map == (char *)(m) && last_fault[(cpu)].addr == (va))

#define LAST_FAULT_SAVE(cpu, m, t, va) \
MACRO_BEGIN \
	last_fault[(cpu)].map = (char *)(m); \
	last_fault[(cpu)].type = (t); \
	last_fault[(cpu)].addr = (va); \
MACRO_END

#define LAST_FAULT_INVALIDATE(cpu) \
MACRO_BEGIN \
	last_fault[(cpu)].type = 0; \
MACRO_END

#else /* MMUBUG */

#define LAST_FAULT_INVALIDATE(cpu)

#endif /* MMUBUG */

/*
 *	Macros for speed.
 */

/* PMAP_ACTIVATE assumed to be called at splhigh */

#define PMAP_ACTIVATE(pmap, th, my_cpu)					\
MACRO_BEGIN								\
	register pmap_t		tpmap = (pmap);				\
									\
	/* let pmap updates proceed while we wait for this pmap */	\
	CPU_SET_REMOVE((my_cpu), cpus_active);				\
									\
	/* wait for any pmap updates in progress */			\
	while (simple_lock_locked(&tpmap->update_lock) ||		\
	       simple_lock_locked(&kernel_pmap->update_lock) )		\
	{								\
	    continue;							\
	}								\
									\
	simple_lock(&tpmap->update_lock);				\
	if (cpu_update_needed[my_cpu])					\
	    process_pmap_updates(kernel_pmap);				\
									\
	CPU_SET_ADD(my_cpu, cpus_active);				\
									\
	simple_lock(&tpmap->nactive_write_lock);			\
	CPU_SET_ADD(my_cpu, tpmap->cpus_using);				\
	tpmap->nactive++;						\
	simple_unlock(&tpmap->nactive_write_lock);			\
									\
	thread_pcb(th)->pcb_ptbr = (struct pt_entry *)tpmap->ptbr;	\
	LAST_FAULT_INVALIDATE(my_cpu);					\
	simple_unlock(&tpmap->update_lock);				\
MACRO_END

#define PMAP_DEACTIVATE(pmap, thread, cpu)				\
MACRO_BEGIN								\
	register pmap_t		tpmap = (pmap);				\
	/* mark pmap no longer in use by this cpu */			\
	/* even if map is locked against updates */			\
	simple_lock(&tpmap->nactive_write_lock);			\
	CPU_SET_REMOVE((cpu), tpmap->cpus_using);			\
	tpmap->nactive--;						\
	simple_unlock(&tpmap->nactive_write_lock);			\
MACRO_END

/*
 *	MARK_CPU_{ACTIVE,IDLE} are assumed to be called at splhigh
 */

#define MARK_CPU_IDLE(cpu)  						\
MACRO_BEGIN								\
	CPU_SET_REMOVE(cpu, cpus_active);				\
	CPU_SET_ADD(cpu, cpus_idle);					\
MACRO_END

#define	MARK_CPU_ACTIVE(cpu)						\
MACRO_BEGIN								\
	CPU_SET_REMOVE(cpu, cpus_idle);					\
	while (simple_lock_locked(&kernel_pmap->update_lock))		\
		;  /* spin until update completes */			\
									\
	if (cpu_update_needed[cpu])					\
		process_pmap_updates(kernel_pmap);			\
									\
	CPU_SET_ADD(cpu, cpus_active);					\
MACRO_END

/*
 *	PMAP_CONTEXT is a no-op on mmax because page table base never moves.
 */

#define PMAP_CONTEXT(pmap, thread)


#ifdef	_KERNEL

lock_data_t	pmap_lock;		/* read/write lock for pmap system */
struct pmap	kernel_pmap_store;	/* the kernel pmap structure.	*/
pmap_t		kernel_pmap;		/* pointer to above.		*/

#define current_pmap()		(vm_map_pmap(current_thread()->task->map))

/*
 *	Macros to invalidate translation buffer.  Use map to figure out
 *	which buffer to invalidate.  Names blatently stolen from VAX.
 */

#ifdef MMUBUG

#if	MMAX_XPC || MMAX_APC

#define TBIS_K(vaddr) \
MACRO_BEGIN \
	register int cpuid = cpu_number(); \
\
	mtpr(MxPR_IVAR0,(vaddr)); \
	if (LAST_FAULT_MATCH(cpuid, kernel_pmap, vaddr)) \
		LAST_FAULT_INVALIDATE(cpuid); \
MACRO_END

#define TBIS_U(vaddr) \
MACRO_BEGIN \
	register int cpuid = cpu_number(); \
\
	mtpr(MxPR_IVAR1,(vaddr)); \
	if (LAST_FAULT_MATCH(cpuid, current_pmap(), vaddr)) \
		LAST_FAULT_INVALIDATE(cpuid); \
MACRO_END

#define TBIS(map,vaddr) \
MACRO_BEGIN \
	register int cpuid = cpu_number(); \
\
	if (map == kernel_pmap) \
		mtpr(MxPR_IVAR0,vaddr);\
	else \
		mtpr(MxPR_IVAR1,vaddr); \
	if (LAST_FAULT_MATCH(cpuid, map, vaddr)) \
		LAST_FAULT_INVALIDATE(cpuid); \
MACRO_END

#endif	/* MMAX_XPC */
#if	MMAX_DPC

#define TBIS(map,vaddr) \
MACRO_BEGIN \
	register int cpuid = cpu_number(); \
\
	if (map == kernel_pmap) \
		mtpr(MxPR_EIA,vaddr); \
	else \
		mtpr(MxPR_EIA, vaddr|EIA_PTB1); \
	if (LAST_FAULT_MATCH(cpuid, map, vaddr)) \
		LAST_FAULT_INVALIDATE(cpuid); \
MACRO_END

#endif	/* MMAX_DPC */

#define TBIA_K \
MACRO_BEGIN \
	mtpr(MxPR_PTB0,kernel_pmap->ptbr); \
	LAST_FAULT_INVALIDATE(cpu_number()); \
MACRO_END

#define TBIA_U \
MACRO_BEGIN \
	mtpr(MxPR_PTB1,(current_pmap())->ptbr); \
	LAST_FAULT_INVALIDATE(cpu_number()); \
MACRO_END

#define TBIA(map)	if (map == kernel_pmap) TBIA_K; \
				else TBIA_U;

#else /* MMUBUG */

/*
 *	Macros to invalidate translation buffer.  Use map to figure out
 *	which buffer to invalidate.  Names blatently stolen from VAX.
 */

#if	MMAX_XPC || MMAX_APC
#define TBIS_K(vaddr)		mtpr(MxPR_IVAR0,vaddr)
#define TBIS_U(vaddr)		mtpr(MxPR_IVAR1,vaddr)

#define TBIS(map,vaddr)	    if (map == kernel_pmap) mtpr(MxPR_IVAR0,vaddr);\
			    else mtpr(MxPR_IVAR1,vaddr);
#endif	/* MMAX_XPC || MMAX_APC */
#if	MMAX_DPC
#define TBIS(map,vaddr)	    if (map == kernel_pmap) mtpr(MxPR_EIA,vaddr);\
				else mtpr(MxPR_EIA, vaddr|EIA_PTB1);
#endif	/* MMAX_DPC */

#define TBIA_K		mtpr(MxPR_PTB0,kernel_pmap->ptbr)

#define TBIA_U		mtpr(MxPR_PTB1,(current_pmap())->ptbr); 

#define TBIA(map)	if (map == kernel_pmap) {TBIA_K;} \
				else {TBIA_U;}

#endif /* MMUBUG */

#define pmap_resident_count(pmap)	((pmap)->stats.resident_count)
#define pmap_attribute(pmap,addr,siz,attr,val)	0
vm_offset_t	pmap_phys_address();
int		pmap_phys_to_frame();

/*
 *	Set of cpus that are actively using mapped memory.  Any
 *	pmap update operation must wait for all cpus in this set.
 *	Update operations must still be queued to cpus not in this
 *	set.
 */
cpu_set_t	cpus_active;

/*
 *	Set of idle cpus.  Any cpu in this set does not get shootdown
 *	interrupts, but updates are queued for it anyway.  This is
 *	because the cpu is spinning in the idle loop and will check for
 *	updates when it exits.
 */
cpu_set_t	cpus_idle;

/*
 *	Quick test for pmap update requests.
 */
boolean_t	cpu_update_needed[NCPUS];
void		process_pmap_updates();

/*
 *	Reserved ptes for manipulating physical memory and corresponding
 *	kernel virtual addresses. The first set (1) is used only by
 *	pmap_zero_page and pmap_copy_page; these routines cannot fault.
 *	The second set (2) is used by pmap_copy_page, copy_to_phys, and
 *	copy_from_phys.  The latter two routines can fault, and result
 *	in calls to any of these routines.  Therefore any use of the
 *	second set of ptes must potentially save and restore them.
 *	phys_pte2_in_use keeps track of whether these ptes are in use
 *	(and therefore must be saved and restored.)
 */

vm_offset_t	phys_map_vaddr1;
vm_offset_t	phys_map_vaddr2;
pt_entry_t	*phys_map_pte1;
pt_entry_t	*phys_map_pte2;

boolean_t	phys_pte2_in_use[NCPUS];

#if	__GNUC__
/*
 *	Given an offset and a map, return the virtual address of the level
 *	two pte.  If the level two pte address is invalid with respect
 *      to the level one map then NULL is returned (and the map may need
 *	to grow).
 */
extern pt_entry_t __inline__ *pmap_pte(map, addr)
    register pmap_t	map;
    register vm_offset_t	addr;
{
#if	MMAX_APC || MMAX_XPC
    register pt_entry_t *entry;
        
    __asm__ volatile ("\n\
	movd	%2,r2		# save copy of address\n\
	movd	4(%1),%0	# address of pte1 entries\n\
	lshd	$-22,%2		# extract level 1 index\n\
	addr	0(%0)[%2:d],%0	# compute pte address\n\
	tbitb	$0,0(%0)	# pte valid ?\n\
	bfs	.+6		# branch if yes\n\
	movqd	$0,%0		# return 0\n\
	br	.+18\n\
	movd	4096(%0),%0	# base of second level pte table\n\
	andd	$0x3ff000,r2	# mask second level index\n\
	lshd	$-10,r2		# convert to offset\n\
	addd	r2,%0		# and add to base to get address\n"
                  : "=r" (entry) : "r" (map), "r" (addr) : "r2" );
    return(entry);
#else
    register vm_offset_t *pte1_addr;

    pte1_addr = (vm_offset_t *) map->pte1_addr + L1IDX(addr);

    if ((*pte1_addr & PG_VALID) == 0)
        return(PT_ENTRY_NULL);

    return(((pt_entry_t *) (*(pte1_addr + PTE1_ENTRIES))) + L2IDX(addr));
#endif	/* MMAX_APC||MMAX_XPC */
}
#endif	/* __GNUC__ */
#endif	/* _KERNEL */
#endif	/* ASSEMBLER */
#endif	/* _PMAP_MACHINE_ */
