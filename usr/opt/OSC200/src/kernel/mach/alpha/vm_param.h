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
 *	@(#)vm_param.h	9.7	(ULTRIX/OSF)	10/22/91
 */ 
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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 *	File:	alpha/vm_param.h
 *
 *	Alpha machine dependent virtual memory parameters.
 *	Most declarations are preceeded by ALPHA_ (or alpha_)
 *	because only Alpha specific code should be using
 *	them.
 *
 */

#ifndef	_MACH_ALPHA_VM_PARAM_H_
#define _MACH_ALPHA_VM_PARAM_H_

#define BYTE_SIZE	8	/* byte size in bits */

#define ALPHA_PGBYTES	8192	/* bytes per alpha page */
#define ALPHA_PGSHIFT	13	/* number of bits to shift for pages */

/*
 *	Convert bytes to pages and convert pages to bytes.
 *	No rounding is used.
 */

#define alpha_btop(x)		(((unsigned long)(x)) >> ALPHA_PGSHIFT)
#define alpha_ptob(x)		(((unsigned long)(x)) << ALPHA_PGSHIFT)

/*
 *	Round off or truncate to the nearest page.  These will work
 *	for either addresses or counts.  (i.e. 1 byte rounds to 1 page
 *	bytes.
 */

#define alpha_round_page(x)	((((unsigned long)(x)) + ALPHA_PGBYTES - 1) & \
					~(ALPHA_PGBYTES-1))
#define alpha_trunc_page(x)	(((unsigned long)(x)) & ~(ALPHA_PGBYTES-1))

/*
 * User level addressability
 */
#define VM_MIN_ADDRESS	((vm_offset_t) 0x10000)
#define VM_MAX_ADDRESS	((vm_offset_t)(~UNITY_BASE + 1))

/*
 * These are the bounds of the kernel heap.
 */

#define VM_MIN_KERNEL_ADDRESS	((vm_offset_t)0xffffffff80000000)
#define VM_MAX_KERNEL_ADDRESS	((vm_offset_t)0xffffffffff800000)

#define KERNEL_STACK_SIZE	(2*ALPHA_PGBYTES)
#define INTSTACK_SIZE		(1*ALPHA_PGBYTES) /* only used for bootstrap & kdb */

/*
 *	Conversion between ALPHA pages and VM pages
 */

#define trunc_alpha_to_vm(p)	(atop(trunc_page(alpha_ptob(p))))
#define round_alpha_to_vm(p)	(atop(round_page(alpha_ptob(p))))
#define vm_to_alpha(p)		(alpha_btop(ptoa(p)))

/*
 * On alpha, the following optimization is possible
 */
#include <machine/cpu.h>

#define map_physical_page(phys)		(vm_offset_t)((phys))
#define	unmap_physical_page(vaddr)

#endif	/* _MACH_ALPHA_VM_PARAM_H_ */
