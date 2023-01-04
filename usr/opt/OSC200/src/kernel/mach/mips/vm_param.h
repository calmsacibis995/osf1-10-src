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
 *	@(#)$RCSfile: vm_param.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1992/03/18 17:49:44 $
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
 * derived from vm_param.h	2.1	(ULTRIX/OSF)	12/3/90
 */

/*
 *	File:	mips/vm_param.h
 *	Author:	Alessandro Forin
 *
 *
 *	MIPS machine dependent virtual memory parameters.
 *	Most declarations are preceeded by MIPS_ (or mips_)
 *	because only Mips specific code should be using
 *	them.
 *
 */

#include <mach/mips/vm_types.h>

#ifndef	_MACH_MIPS_VM_PARAM_H_
#define _MACH_MIPS_VM_PARAM_H_

#define BYTE_SIZE	8	/* byte size in bits */

#define MIPS_PGBYTES	4096	/* bytes per mips page */
#define MIPS_PGSHIFT	12	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define mips_btop(x)		(((unsigned)(x)) >> MIPS_PGSHIFT)
#define mips_ptob(x)		(((unsigned)(x)) << MIPS_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define mips_round_page(x)	((((unsigned)(x)) + MIPS_PGBYTES - 1) & \
					~(MIPS_PGBYTES-1))
#define mips_trunc_page(x)	(((unsigned)(x)) & ~(MIPS_PGBYTES-1))

/*
 * User level addressability
 *
 * The Mips processor spec give the whole 0-8000000 range
 * to a user process.  However, there is a bug in the 3.0 rev
 * R2000 chips which can trash the context register to 0.
 * A utlbmiss can look like an access to the 0th segment 
 * since the context register gets set to 0. If anything 
 * in the 0th segment is valid, a tlbdropin will occur 
 * without a probe being done. This could cause multiple 
 * matching tlb entries which can lead to a tlb dead condition.
 * A reset is required if a tlb dead situation occurs. 
 * Rumor is that the rev 5 R2000s will fix this problem.
 * We'll wait and see......
 *
 * Meanwhile the fix is to disallow use of the lower
 * memory addresses, those that would be mapped with
 * the very first page of page table entries.
 * On an R2000 system this is 4Meg (1024 ptes per page 
 * and 1 pte maps 4096 bytes).
 */
#define VM_MIN_ADDRESS	((vm_offset_t) 0x400000)
#define VM_MAX_ADDRESS	((vm_offset_t) 0x80000000)


/*
 *	Layout of the kernel virtual address space.
 *
 * The entire kernel virtual address space (1Gb) is divided in two
 * parts.
 *
 * The upper part of the kernel vaddr is devoted to pte arrays,
 * both for the kernel's pte and the user's.
 * Even if the kernel's virtual address space is half the size of
 * the user's one, alignment restrictions in the CTXT tlb register
 * lead us to use uniform sizes.
 * The virtual space devoted to the kernel's pte is then large
 * enough to cover all the address space.  This is to cope with
 * a bug/feature of the latest R2000 chips which now invoke the
 * utlbmiss trap even when in kernel mode.
 * 
 * The lower part is for general kernel use, and is allocated 
 * bottom up (or whatever the VM system likes).
 */
#define MIPS_VIRTUAL_SPACE_SIZE	0x80000000	/* 2Gb, as in kuseg */

#define ptesize(x)	(((x)>>MIPS_PGSHIFT)<<2)
#define UPTESIZE	PTESIZE			/* 2 Meg */
#define KPTESIZE	(PTESIZE<<1)		/* 4 Meg */

#ifdef	ASSEMBLER
#define PTESIZE		ptesize(MIPS_VIRTUAL_SPACE_SIZE)
#define KPTEADDR	(0-KPTESIZE)

#else	/* ASSEMBLER */

#define PTESIZE		ptesize((unsigned)MIPS_VIRTUAL_SPACE_SIZE)
#define KPTEADDR	(unsigned)((unsigned)0-KPTESIZE)
#endif	/* ASSEMBLER */


/*
 * The kernel's pte pages are mapped in a little table
 * which is kept in the k0seg to avoid waste of tlb entries
 */
#define KPPTESIZE	ptesize(KPTESIZE)	/* 2 Kb, physical in kseg0 */
#define KPTERADDR	((unsigned)0 - KPPTESIZE)

/*
 * While the choice of the lower bound for the kernel's
 * virtual address space is dictated by the hardware,
 * the choice of the upper bound is dictated by software.
 * For (our) silly reasons we can't have the VM system manage
 * the pte pages for us, so we do it ourselves reserving
 * an arbitrary segment of the virtual space for page table use.
 */
#define MIPS_KERNEL_SPACE_SIZE	(((vm_size_t) 509 * PTESIZE ) - (4*MIPS_PGBYTES)) /* 890 meg */

#ifdef ASSEMBLER
#define VM_MIN_KERNEL_ADDRESS	( 0xc0000000 + (4*MIPS_PGBYTES))
#else
#define VM_MIN_KERNEL_ADDRESS	(((vm_offset_t) 0xc0000000) + (4*MIPS_PGBYTES))
#endif

#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t) VM_MIN_KERNEL_ADDRESS + MIPS_KERNEL_SPACE_SIZE)

#define KERNEL_STACK_SIZE	(2*MIPS_PGBYTES)
#define INTSTACK_SIZE		(1*MIPS_PGBYTES) /* only used for bootstrap & kdb */

/*
 *	Conversion between MIPS pages and VM pages
 */

#define trunc_mips_to_vm(p)	(atop(trunc_page(mips_ptob(p))))
#define round_mips_to_vm(p)	(atop(round_page(mips_ptob(p))))
#define vm_to_mips(p)		(mips_btop(ptoa(p)))

/*
 * On MIPS, the following optimization is possible
 */
#include <machine/cpu.h>

#define map_physical_page(phys)		(vm_offset_t)PHYS_TO_K0((phys))
#define	unmap_physical_page(vaddr)

#endif	/* _MACH_MIPS_VM_PARAM_H_ */
