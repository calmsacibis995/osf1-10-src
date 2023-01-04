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
 *	@(#)$RCSfile: pmap.h,v $ $Revision: 4.3.12.2 $ (DEC) $Date: 1993/09/03 19:26:48 $
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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 *	File:	vm/pmap.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr.
 *
 *	Machine address mapping definitions -- machine-independent
 *	section.  [For machine-dependent section, see "machine/pmap.h".]
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Delay inclusion of sys/preempt.h (for RT_PREEMPT) to avoid circular
 *	include file problem.
 *
 */

#ifndef	_VM_PMAP_H_
#define _VM_PMAP_H_

#include <rt_preempt.h>

#if	RT_PREEMPT
#ifndef	_SKIP_PREEMPT_H_
#define _SKIP_PREEMPT_H_
#define	_VM_PMAP_H_PREEMPT_
#endif
#endif

#include <machine/pmap.h>
#include <mach/machine/vm_types.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>

/*
 *	The following is a description of the interface to the
 *	machine-dependent "physical map" data structure.  The module
 *	must provide a "pmap_t" data type that represents the
 *	set of valid virtual-to-physical addresses for one user
 *	address space.  [The kernel address space is represented
 *	by a distinguished "pmap_t".]  The routines described manage
 *	this type, install and update virtual-to-physical mappings,
 *	and perform operations on physical addresses common to
 *	many address spaces.
 */

/*
 *	Routines used for initialization.
 */

extern
#ifdef __alpha
	vm_offset_t		/* Alpha returns physical address of 1st PCB. */
#else
	void
#endif /* __alpha */
			pmap_bootstrap();	/* Initialization,
						 * before running virtual.
						 */


extern void		pmap_init();		/* Initialization,
						 * after kernel runs
						 * in virtual memory.
						 */
extern vm_offset_t	pmap_map();		/* Enter a range of
						 * mappings.
						 */

/*
 *	Routines to manage the physical map data structure.
 */
extern pmap_t		pmap_create();		/* Create a pmap_t. */
#ifndef	pmap_kernel
extern pmap_t		pmap_kernel();		/* Return the kernel's pmap_t. */
#endif	/* pmap_kernel */
extern void		pmap_reference();	/* Gain a reference. */
extern void		pmap_destroy();		/* Release a reference. */

extern void		pmap_enter();		/* Enter a mapping */

/*
 *	Routines that operate on ranges of virtual addresses.
 */
extern void		pmap_remove();		/* Remove mappings. */
extern void		pmap_protect();		/* Change protections. */

/*
 *	Routines to set up hardware state for physical maps to be used.
 */
extern void		pmap_activate();	/* Prepare pmap_t to run
						 * on a given processor.
						 */
extern void		pmap_deactivate();	/* Release pmap_t from
						 * use on processor.
						 */


/*
 *	Routines that operate on physical addresses.
 */
extern kern_return_t	pmap_page_protect();	/* Restrict access to page. */
extern boolean_t	pmap_valid_page();	/* Is physical address
						 * valid and usable by
						 * machine-independent
						 * VM system?  Used only
						 * at startup.
						 */

/*
 *	Routines to manage reference/modify bits based on
 *	physical addresses, simulating them if not provided
 *	by the hardware.
 */
extern void		pmap_clear_reference();	/* Clear reference bit */
#ifndef	pmap_is_referenced
extern boolean_t	pmap_is_referenced();	/* Return reference bit */
#endif	/* pmap_is_referenced */
extern void		pmap_clear_modify();	/* Clear modify bit */
extern boolean_t	pmap_is_modified();	/* Return modify bit */


/*
 *	Statistics routines
 */
extern void		pmap_statistics();	/* Return statistics */

#ifndef	pmap_resident_count
extern int		pmap_resident_count();
#endif	/* pmap_resident_count */

/*
 *	Sundry required routines
 */
extern vm_offset_t	pmap_extract();		/* Return a virtual-to-physical
						 * mapping, if possible.
						 */
extern boolean_t	pmap_access();		/* Is virtual address valid? */

#ifndef	pmap_update
extern void		pmap_update();		/* Bring maps up to date */
#endif	/* pmap_update */
extern void		pmap_collect();		/* Perform garbage
						 * collection, if any
						 */

extern void		pmap_change_wiring();	/* Specify pageability */

#ifndef	pmap_phys_address
extern vm_offset_t	pmap_phys_address();	/* Transform address
						 * returned by device
						 * driver mapping function
						 * to physical address
						 * known to this module.
						 */
#endif	/* pmap_phys_address */

#ifndef	pmap_phys_to_frame
extern int		pmap_phys_to_frame();	/* Inverse of
						 * pmap_phys_address,
						 * for use by device driver
						 * mapping function in
						 * machine-independent
						 * pseudo-devices.
						 */

#endif	/* pmap_phys_to_frame */

/*
 *	Optional routines
 */
#ifndef	pmap_copy
extern void		pmap_copy();		/* Copy range of
						 * mappings, if desired.
						 */
#endif	/* pmap_copy */

/*
 * The machine/pmap.h include file must export PMAP_SEGMENTATION for
 * the upper level to have access to the low level pmap functions.
 */

#ifndef	PMAP_SEGMENTATION

#define	pmap_seg_destroy(PSEG)
#define	pmap_seg_alloc(PPSEG) 						\
	((*PPSEG) = (struct pmap_seg *) 0, KERN_INVALID_ADDRESS)
#define	pmap_seg_load(PMAP, PSEG, START)
#define	pmap_seg_unload(PMAP, PSEG, START)
#define	pmap_seg_enter(PMAP, PSEG, ADDR, PHYS, PROT)

#endif	/* !PMAP_SEGMENTATION */


extern void             tbsync();

extern void             pmap_tb();

extern kern_return_t    pmap_load();
extern void             pmap_unload();

extern kern_return_t    pmap_dup();

extern kern_return_t    pmap_map_io();

extern kern_return_t    svatophys();

/*
 *	Exported data structures
 */

extern pmap_t	kernel_pmap;			/* The kernel's map */

#if	RT_PREEMPT
#ifdef	_VM_PMAP_H_PREEMPT_
#include <sys/preempt.h>
#endif
#endif

#endif	/* _VM_PMAP_H_ */
