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
 * @(#)$RCSfile: vm_perf.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/08/25 16:46:11 $
 */
#ifndef	__VM_PERF__
#define	__VM_PERF__ 1

typedef	unsigned long vpf_t;

struct vm_perf {
						/* Rates and Total */
#define	vpf_first_rat	vpf_pagefaults
	vpf_t vpf_pagefaults;			/* All page faults */
	vpf_t vpf_kpagefaults;			/* Kernel page faults */
	vpf_t vpf_cowfaults;			/* Cow part of all faults */
	vpf_t vpf_cowsteals;			/* Optimizied COW faults */
	vpf_t vpf_zfod;				/* User zero filled on demand */
	vpf_t vpf_kzfod;			/* Kernel  "  " */
	vpf_t vpf_pgiowrites;			/* Page I/O writes */
	vpf_t vpf_pgwrites;			/* Page writes */
	vpf_t vpf_pgioreads;			/* Page I/O reads */
	vpf_t vpf_pgreads;			/* Page reads */
	vpf_t vpf_swapreclaims;			/* Swap write reclaims */
	vpf_t vpf_taskswapouts;			/* Task swapouts */
	vpf_t vpf_taskswapins;			/* Task swapins */
	vpf_t vpf_vplmsteal;			/* VP low mem steal */
	vpf_t vpf_vplmstealwins;		/* VP low mem steal wins */
	vpf_t vpf_vpseqdrain;			/* VP sequentail drains */
	vpf_t vpf_ubchit;			/* LRUed UBC pages */
	vpf_t vpf_ubcalloc;			/* Memory demand of ubc */
	vpf_t vpf_ubcpushes;			/* UBC pages pushed */
	vpf_t vpf_ubcpagepushes;		/* UBC pages pushes */
	vpf_t vpf_ubcdirtywra;			/* UBC dirty write aheads */
	vpf_t vpf_ubcreclaim;			/* UBC page reclaims */
	vpf_t vpf_reactivate;			/* Pageout system reactivate */
#define	vpf_last_rat	vpf_reactivate
						/* Average and Current */
#define	vpf_first_aac	vpf_allocatedpages
	vpf_t vpf_allocatedpages;		/* Total pages allocated */
	vpf_t vpf_wiredpages;			/* Wired pages */
	vpf_t vpf_ubcpages;			/* UBC pages */
	vpf_t vpf_freepages;			/* Free VM pages */
#define	vpf_last_aac	vpf_freepages
						/* Totals */
	vpf_t vpf_swapspace;			/* Free swap space */
};

#ifdef	KERNEL

extern struct vm_perf vm_perf;		/* Counts */
					/* Rates or Average and Summary */
extern struct vm_perf vm_perfcomp; 		
					/* Current value or running total */
extern struct vm_perf vm_perfsum;	

#if	UNIX_LOCKS
extern	simple_lock_data_t vm_perf_lock;
#endif	/* UNIX_LOCKS */

#define vpf_load(FIELD)		vm_perf.vpf_/**/FIELD
#define vpf_cload(FIELD)	vm_perfcomp.vpf_/**/FIELD
#define vpf_sload(FIELD)	vm_perfsum.vpf_/**/FIELD

#define vpf_lload(FIELD) {						\
	usimple_lock(&vm_perf_lock);					\
	vpf_load(FIELD);						\
	usimple_unlock(&vm_perf_lock);					\
}

#define	vpf_store(FIELD,COUNT)	vm_perf.vpf_/**/FIELD = (COUNT)		
#define	vpf_cstore(FIELD,COUNT)	vm_perfcomp.vpf_/**/FIELD = (COUNT)		
#define	vpf_sstore(FIELD,COUNT)	vm_perfsum.vpf_/**/FIELD = (COUNT)		

#define	vpf_lstore(FIELD,COUNT) {					\
	usimple_lock(&vm_perf_lock);					\
	vpf_store(FIELD,COUNT);						\
	usimple_unlock(&vm_perf_lock);					\
}

#define	vpf_add(FIELD,COUNT) vm_perf.vpf_/**/FIELD += (COUNT)

#define	vpf_ladd(FIELD,COUNT) {						\
	usimple_lock(&vm_perf_lock);					\
	vpf_add(FIELD,COUNT);						\
	usimple_unlock(&vm_perf_lock);					\
}

#define vpf_subtract(FIELD,COUNT) vm_perf.vpf_/**/FIELD -= (COUNT)

#define	vpf_lsubtract(FIELD,COUNT) {					\
	usimple_lock(&vm_perf_lock);					\
	vpf_subtract(FIELD,COUNT);					\
	usimple_unlock(&vm_perf_lock);					\
}


#endif	/* KERNEL */

#endif	/* !__VM_PERF__ */
